$ErrorActionPreference = "Stop"

function Assert-NativeCommandSucceeded {
  param(
    [int] $ExitCode,
    [string] $Description
  )
  if ($ExitCode -ne 0) {
    throw "$Description failed with exit code $ExitCode."
  }
}

$prefix = $args[0]
if (-not $prefix) {
  throw "usage: cmake-consumer-smoke.ps1 <install-prefix>"
}

$work = Join-Path $env:RUNNER_TEMP "gecode-consumers-win"
if (Test-Path $work) { Remove-Item -Recurse -Force $work }
New-Item -ItemType Directory -Force -Path `
  (Join-Path $work "a"), `
  (Join-Path $work "b"), `
  (Join-Path $work "c"), `
  (Join-Path $work "manual") | Out-Null

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
project(manual_consumer LANGUAGES CXX)
set(GECODE_PREFIX "" CACHE PATH "Installed Gecode prefix")
find_path(GECODE_INCLUDE_DIR
  NAMES gecode/support/config.hpp
  PATHS "${GECODE_PREFIX}/include"
  NO_DEFAULT_PATH
  REQUIRED)
find_library(GECODE_SUPPORT_LIBRARY
  NAMES gecodesupport
  PATHS "${GECODE_PREFIX}/lib"
  NO_DEFAULT_PATH
  REQUIRED)
add_executable(manual_consumer main.cpp)
target_include_directories(manual_consumer PRIVATE "${GECODE_INCLUDE_DIR}")
target_link_libraries(manual_consumer PRIVATE "${GECODE_SUPPORT_LIBRARY}")
'@ | Set-Content -Encoding utf8 (Join-Path $work "manual\CMakeLists.txt")
@'
#include <gecode/support.hh>
#ifndef GECODE_NO_AUTOLINK
#error "CMake-installed headers must disable legacy MSVC auto-linking"
#endif
int main(void) {
  (void) Gecode::Support::hwrnd();
  return 0;
}
'@ | Set-Content -Encoding utf8 (Join-Path $work "manual\main.cpp")

cmake -S (Join-Path $work "a") -B (Join-Path $work "a\build") -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$prefix" 2>&1 | Tee-Object -FilePath (Join-Path $work "a\configure.log")
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_a configure"
cmake --build (Join-Path $work "a\build") --config $env:BUILD_TYPE
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_a build"

cmake -S (Join-Path $work "b") -B (Join-Path $work "b\build") -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$prefix"
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_b configure"
cmake --build (Join-Path $work "b\build") --config $env:BUILD_TYPE
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_b build"

cmake -S (Join-Path $work "c") -B (Join-Path $work "c\build") -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$prefix"
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_c configure"
cmake --build (Join-Path $work "c\build") --config $env:BUILD_TYPE
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_c build"

# Exercise the installed headers and library without loading the CMake package
# or inheriting compile definitions from an exported target.
cmake -S (Join-Path $work "manual") -B (Join-Path $work "manual\build") -G "Visual Studio 17 2022" -A x64 "-DGECODE_PREFIX=$prefix"
Assert-NativeCommandSucceeded $LASTEXITCODE "manual_consumer configure"
cmake --build (Join-Path $work "manual\build") --config $env:BUILD_TYPE
Assert-NativeCommandSucceeded $LASTEXITCODE "manual_consumer build"

if (-not (Select-String -Path (Join-Path $work "a\configure.log") -Pattern "Gecode_VERSION=" -Quiet)) {
  throw "Gecode_VERSION was not reported during consumer configure."
}
