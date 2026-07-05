$ErrorActionPreference = "Stop"

$prefix = $args[0]
if (-not $prefix) {
  throw "usage: cmake-consumer-smoke-vcpkg.ps1 <install-prefix>"
}

$work = Join-Path $env:RUNNER_TEMP "gecode-consumers-win-vcpkg"
$toolchain = "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
$manifest = Join-Path $work "manifest"
$consumerVcpkgInstalled = Join-Path $work "vcpkg_installed"
if (Test-Path $work) { Remove-Item -Recurse -Force $work }
New-Item -ItemType Directory -Force -Path (Join-Path $work "a"), (Join-Path $work "b"), (Join-Path $work "c"), $manifest | Out-Null

$baseline = (Get-Content "$env:GITHUB_WORKSPACE\vcpkg.json" | ConvertFrom-Json).'builtin-baseline'
@"
{
  "name": "gecode-consumer-smoke",
  "version-string": "0",
  "builtin-baseline": "$baseline",
  "dependencies": [
    "mpfr"
  ]
}
"@ | Set-Content -Encoding utf8 (Join-Path $manifest "vcpkg.json")

@'
cmake_minimum_required(VERSION 3.21)
project(consumer_a LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED)
message(STATUS "Gecode_VERSION=${Gecode_VERSION}")
add_executable(consumer_a main.cpp)
target_link_libraries(consumer_a PRIVATE Gecode::gecode)
'@ | Set-Content -Encoding utf8 (Join-Path $work "a\CMakeLists.txt")
@'
#include <gecode/support/config.hpp>
int main(void) { return 0; }
'@ | Set-Content -Encoding utf8 (Join-Path $work "a\main.cpp")

@'
cmake_minimum_required(VERSION 3.21)
project(consumer_b LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS driver)
add_executable(consumer_b main.cpp)
target_link_libraries(consumer_b PRIVATE Gecode::gecodedriver)
'@ | Set-Content -Encoding utf8 (Join-Path $work "b\CMakeLists.txt")
@'
#include <gecode/driver.hh>
int main(void) {
  Gecode::Options options("consumer_b");
  return options.model();
}
'@ | Set-Content -Encoding utf8 (Join-Path $work "b\main.cpp")

@'
cmake_minimum_required(VERSION 3.21)
project(consumer_c LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS gecodedriver)
add_executable(consumer_c main.cpp)
target_link_libraries(consumer_c PRIVATE Gecode::gecodedriver)
'@ | Set-Content -Encoding utf8 (Join-Path $work "c\CMakeLists.txt")
@'
#include <gecode/driver.hh>
int main(void) {
  Gecode::Options options("consumer_c");
  return options.model();
}
'@ | Set-Content -Encoding utf8 (Join-Path $work "c\main.cpp")

cmake -S (Join-Path $work "a") -B (Join-Path $work "a\build") -G "Visual Studio 17 2022" -A x64 `
  "-DCMAKE_TOOLCHAIN_FILE=$toolchain" `
  "-DVCPKG_MANIFEST_MODE=ON" `
  "-DVCPKG_MANIFEST_DIR=$manifest" `
  "-DVCPKG_INSTALLED_DIR=$consumerVcpkgInstalled" `
  "-DVCPKG_TARGET_TRIPLET=x64-windows" `
  "-DCMAKE_PREFIX_PATH=$prefix" 2>&1 | Tee-Object -FilePath (Join-Path $work "a\configure.log")
if ($LASTEXITCODE -ne 0) {
  throw "consumer_a configure failed."
}
cmake --build (Join-Path $work "a\build") --config $env:BUILD_TYPE

cmake -S (Join-Path $work "b") -B (Join-Path $work "b\build") -G "Visual Studio 17 2022" -A x64 `
  "-DCMAKE_TOOLCHAIN_FILE=$toolchain" `
  "-DVCPKG_MANIFEST_MODE=ON" `
  "-DVCPKG_MANIFEST_DIR=$manifest" `
  "-DVCPKG_INSTALLED_DIR=$consumerVcpkgInstalled" `
  "-DVCPKG_TARGET_TRIPLET=x64-windows" `
  "-DCMAKE_PREFIX_PATH=$prefix"
cmake --build (Join-Path $work "b\build") --config $env:BUILD_TYPE

cmake -S (Join-Path $work "c") -B (Join-Path $work "c\build") -G "Visual Studio 17 2022" -A x64 `
  "-DCMAKE_TOOLCHAIN_FILE=$toolchain" `
  "-DVCPKG_MANIFEST_MODE=ON" `
  "-DVCPKG_MANIFEST_DIR=$manifest" `
  "-DVCPKG_INSTALLED_DIR=$consumerVcpkgInstalled" `
  "-DVCPKG_TARGET_TRIPLET=x64-windows" `
  "-DCMAKE_PREFIX_PATH=$prefix"
cmake --build (Join-Path $work "c\build") --config $env:BUILD_TYPE

if (-not (Select-String -Path (Join-Path $work "a\configure.log") -Pattern "Gecode_VERSION=" -Quiet)) {
  throw "Gecode_VERSION was not reported during consumer configure."
}
