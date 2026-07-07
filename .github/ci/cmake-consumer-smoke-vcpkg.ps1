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
New-Item -ItemType Directory -Force -Path `
  (Join-Path $work "a"), `
  (Join-Path $work "b"), `
  (Join-Path $work "c"), `
  (Join-Path $work "d"), `
  $manifest | Out-Null

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

@'
cmake_minimum_required(VERSION 3.21)
project(consumer_d LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS float)
if(NOT TARGET MPFR::MPFR)
  message(FATAL_ERROR "Expected Gecode package to provide MPFR::MPFR")
endif()
if(TARGET Gecode::gecodefloat_shared)
  get_target_property(gecodefloat_links Gecode::gecodefloat_shared INTERFACE_LINK_LIBRARIES)
elseif(TARGET Gecode::gecodefloat_static)
  get_target_property(gecodefloat_links Gecode::gecodefloat_static INTERFACE_LINK_LIBRARIES)
else()
  message(FATAL_ERROR "Expected an exported gecodefloat library target")
endif()
if(NOT gecodefloat_links MATCHES "MPFR::MPFR")
  message(FATAL_ERROR "Expected gecodefloat to link MPFR::MPFR, got: ${gecodefloat_links}")
endif()
add_executable(consumer_d main.cpp)
target_link_libraries(consumer_d PRIVATE Gecode::gecodefloat)
'@ | Set-Content -Encoding utf8 (Join-Path $work "d\CMakeLists.txt")
@'
#include <gecode/support/config.hpp>
#ifndef GECODE_HAS_MPFR
#error "Expected installed Gecode package to enable MPFR"
#endif
#include <gecode/float.hh>
int main(void) {
  Gecode::Float::Rounding rounding;
  return rounding.exp_down(1.0) > 0.0 ? 0 : 1;
}
'@ | Set-Content -Encoding utf8 (Join-Path $work "d\main.cpp")

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

cmake -S (Join-Path $work "d") -B (Join-Path $work "d\build") -G "Visual Studio 17 2022" -A x64 `
  "-DCMAKE_TOOLCHAIN_FILE=$toolchain" `
  "-DVCPKG_MANIFEST_MODE=ON" `
  "-DVCPKG_MANIFEST_DIR=$manifest" `
  "-DVCPKG_INSTALLED_DIR=$consumerVcpkgInstalled" `
  "-DVCPKG_TARGET_TRIPLET=x64-windows" `
  "-DCMAKE_PREFIX_PATH=$prefix"
cmake --build (Join-Path $work "d\build") --config $env:BUILD_TYPE

if (-not (Select-String -Path (Join-Path $work "a\configure.log") -Pattern "Gecode_VERSION=" -Quiet)) {
  throw "Gecode_VERSION was not reported during consumer configure."
}
