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
  (Join-Path $work "flatzinc"), `
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
if(NOT TARGET Gecode::gecodedriver OR
   NOT Gecode_LIBRARIES STREQUAL "Gecode::gecodedriver")
  message(FATAL_ERROR "Expected canonical library for legacy component, got: ${Gecode_LIBRARIES}")
endif()
'@ | Set-Content -Encoding utf8 (Join-Path $work "c\CMakeLists.txt")

@'
cmake_minimum_required(VERSION 3.21)
project(consumer_flatzinc LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS flatzinc)
add_executable(consumer_flatzinc main.cpp)
target_link_libraries(consumer_flatzinc PRIVATE Gecode::gecodeflatzinc)
'@ | Set-Content -Encoding utf8 (Join-Path $work "flatzinc\CMakeLists.txt")
@'
#include <gecode/flatzinc.hh>
#include <iostream>
#include <memory>
#include <sstream>

bool check(const char* name, const char* model, bool expect_failed = false) {
  std::istringstream source(model);
  std::ostringstream errors;
  Gecode::FlatZinc::Printer printer;
  std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> space(
    Gecode::FlatZinc::parse(source, printer, errors));
  if (!space || (space->status() == Gecode::SS_FAILED) != expect_failed) {
    std::cerr << name << ": unexpected parse or propagation result\n"
              << errors.str();
    return false;
  }
  return true;
}

int main() {
  // Each registry family lives in its own object file of the library.
  bool ok = check("integer constraints",
    "var 0..0:x; constraint set_in(x,{1}); solve satisfy;", true);
  ok = check("scheduling constraints",
    "constraint fzn_cumulatives([0,0],[2,2],[1,1],1); solve satisfy;",
    true) && ok;
#ifdef GECODE_HAS_SET_VARS
  ok = check("set constraints",
    "var set of 1..2:s; constraint set_eq(s,{1}); "
    "constraint fzn_set_card(s,2); solve satisfy;", true) && ok;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
  ok = check("float constraints",
    "var 0.0..1.0:x; constraint fzn_float_eq(x,2.0); solve satisfy;",
    true) && ok;
#endif
  return ok ? 0 : 1;
}
'@ | Set-Content -Encoding utf8 (Join-Path $work "flatzinc\main.cpp")

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

# Parsing must reach every enabled registry family, including from static
# libraries, and the registry's private header must not be installed.
if (Test-Path (Join-Path $prefix "include\gecode\flatzinc\registry-internal.hh")) {
  throw "The private FlatZinc registry header was installed."
}
cmake -S (Join-Path $work "flatzinc") -B (Join-Path $work "flatzinc\build") -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$prefix"
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_flatzinc configure"
cmake --build (Join-Path $work "flatzinc\build") --config $env:BUILD_TYPE
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_flatzinc build"
$env:PATH = "$(Join-Path $prefix 'bin');$env:PATH"
& (Join-Path $work "flatzinc\build\$env:BUILD_TYPE\consumer_flatzinc.exe")
Assert-NativeCommandSucceeded $LASTEXITCODE "consumer_flatzinc run"

# Exercise the installed headers and library without loading the CMake package
# or inheriting compile definitions from an exported target.
cmake -S (Join-Path $work "manual") -B (Join-Path $work "manual\build") -G "Visual Studio 17 2022" -A x64 "-DGECODE_PREFIX=$prefix"
Assert-NativeCommandSucceeded $LASTEXITCODE "manual_consumer configure"
cmake --build (Join-Path $work "manual\build") --config $env:BUILD_TYPE
Assert-NativeCommandSucceeded $LASTEXITCODE "manual_consumer build"

if (-not (Select-String -Path (Join-Path $work "a\configure.log") -Pattern "Gecode_VERSION=" -Quiet)) {
  throw "Gecode_VERSION was not reported during consumer configure."
}
