#!/usr/bin/env bash
set -euxo pipefail

usage="usage: cmake-consumer-smoke.sh <install-prefix> <expected-include-dir> [core|mpfr]"
prefix="${1:?$usage}"
expected_include="${2:?$usage}"
mode="${3:-mpfr}"
case "$mode" in
  core)
    expect_mpfr=OFF
    ;;
  mpfr)
    expect_mpfr=ON
    ;;
  *)
    echo "$usage" >&2
    exit 2
    ;;
esac
work="$RUNNER_TEMP/gecode-consumers"
rm -rf "$work"
mkdir -p "$work/a" "$work/b" "$work/c" "$work/d/cmake" "$work/e/cmake" "$work/e/stale" \
  "$work/mpfr-prefix/lib/cmake/MPFR"

cat > "$work/a/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_a LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED)
message(STATUS "Gecode_VERSION=${Gecode_VERSION}")
message(STATUS "Gecode_INCLUDE_DIRS=${Gecode_INCLUDE_DIRS}")
set(EXPECTED_GECODE_INCLUDE_DIR "" CACHE PATH "Expected Gecode include directory")
if(NOT Gecode_INCLUDE_DIRS STREQUAL EXPECTED_GECODE_INCLUDE_DIR)
  message(FATAL_ERROR
    "Expected Gecode_INCLUDE_DIRS=${EXPECTED_GECODE_INCLUDE_DIR}, got: ${Gecode_INCLUDE_DIRS}")
endif()
set(EXPECT_MPFR OFF CACHE BOOL "Expect Gecode package to require MPFR")
if(EXPECT_MPFR)
  if(NOT TARGET MPFR::MPFR)
    message(FATAL_ERROR "Expected MPFR::MPFR from the fake MPFR config package")
  endif()
  get_target_property(mpfr_config_marker MPFR::MPFR INTERFACE_COMPILE_DEFINITIONS)
  if(NOT mpfr_config_marker)
    set(mpfr_config_marker)
  endif()
  if(NOT "GECODE_SMOKE_FAKE_MPFR_CONFIG" IN_LIST mpfr_config_marker)
    message(FATAL_ERROR "Expected fake MPFR config package, got marker: ${mpfr_config_marker}")
  endif()
endif()
add_executable(consumer_a main.cpp)
target_link_libraries(consumer_a PRIVATE Gecode::gecode)
EOF
cat > "$work/a/main.cpp" <<'EOF'
#include <gecode/support/config.hpp>
int main(void) { return 0; }
EOF

cat > "$work/b/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_b LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS driver)
add_executable(consumer_b main.cpp)
target_link_libraries(consumer_b PRIVATE Gecode::gecodedriver)
EOF
cat > "$work/b/main.cpp" <<'EOF'
#include <gecode/driver.hh>
int main(void) {
  Gecode::Options options("consumer_b");
  return options.model();
}
EOF

cat > "$work/c/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_c LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS gecodedriver)
add_executable(consumer_c main.cpp)
target_link_libraries(consumer_c PRIVATE Gecode::gecodedriver)
EOF
cat > "$work/c/main.cpp" <<'EOF'
#include <gecode/driver.hh>
int main(void) {
  Gecode::Options options("consumer_c");
  return options.model();
}
EOF

cat > "$work/d/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_d LANGUAGES CXX)
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
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
target_link_libraries(consumer_d PRIVATE Gecode::gecode)
EOF
cat > "$work/d/main.cpp" <<'EOF'
#include <gecode/support/config.hpp>
#ifndef GECODE_HAS_MPFR
#error "Expected installed Gecode package to enable MPFR"
#endif
#include <gecode/float.hh>
int main(void) {
  Gecode::Float::Rounding rounding;
  return rounding.exp_down(1.0) > 0.0 ? 0 : 1;
}
EOF
cat > "$work/d/cmake/FindMPFR.cmake" <<'EOF'
set(MPFR_FOUND TRUE)
set(MPFR_INCLUDE_DIRS "/bad/consumer/mpfr/include")
set(MPFR_LIBRARIES "/bad/consumer/mpfr/lib/libmpfr.a")
EOF

cat > "$work/e/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_e LANGUAGES CXX)
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/stale")
find_package(BadDep CONFIG REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/cmake" NO_DEFAULT_PATH)
find_package(Gecode CONFIG REQUIRED)
if(NOT TARGET MPFR::MPFR)
  message(FATAL_ERROR "Expected Gecode package to recover MPFR::MPFR after stale dependency lookup")
endif()
add_executable(consumer_e main.cpp)
target_link_libraries(consumer_e PRIVATE Gecode::gecode)
EOF
cat > "$work/e/main.cpp" <<'EOF'
#include <gecode/support/config.hpp>
#ifndef GECODE_HAS_MPFR
#error "Expected installed Gecode package to enable MPFR"
#endif
#include <gecode/float.hh>
int main(void) {
  Gecode::Float::Rounding rounding;
  return rounding.log_down(2.0) >= 0.0 ? 0 : 1;
}
EOF
cat > "$work/e/cmake/BadDepConfig.cmake" <<'EOF'
include(CMakeFindDependencyMacro)
find_dependency(MPFR)
EOF
cat > "$work/e/stale/FindMPFR.cmake" <<'EOF'
set(MPFR_FOUND TRUE)
set(MPFR_INCLUDE_DIRS "/bad/prior/mpfr/include")
set(MPFR_LIBRARIES "/bad/prior/mpfr/lib/libmpfr.a")
EOF

cat > "$work/mpfr-prefix/lib/cmake/MPFR/MPFRConfig.cmake" <<'EOF'
get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
if(NOT TARGET MPFR::MPFR)
  add_library(MPFR::MPFR INTERFACE IMPORTED)
endif()
set_target_properties(MPFR::MPFR PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS GECODE_SMOKE_FAKE_MPFR_CONFIG)
set(MPFR_FOUND TRUE)
EOF

cmake_prefix_path="$prefix"
if [ "$mode" = "mpfr" ]; then
  cmake_prefix_path="$work/mpfr-prefix;$prefix"
fi

cmake -S "$work/a" -B "$work/a/build" \
  -DCMAKE_PREFIX_PATH="$cmake_prefix_path" \
  -DEXPECTED_GECODE_INCLUDE_DIR="$expected_include" \
  -DEXPECT_MPFR="$expect_mpfr" 2>&1 | tee "$work/a/configure.log"
cmake --build "$work/a/build" -j4

cmake -S "$work/b" -B "$work/b/build" -DCMAKE_PREFIX_PATH="$prefix"
cmake --build "$work/b/build" -j4

cmake -S "$work/c" -B "$work/c/build" -DCMAKE_PREFIX_PATH="$prefix"
cmake --build "$work/c/build" -j4

if [ "$mode" = "mpfr" ]; then
  cmake -S "$work/d" -B "$work/d/build" -DCMAKE_PREFIX_PATH="$prefix"
  cmake --build "$work/d/build" -j4

  cmake -S "$work/e" -B "$work/e/build" -DCMAKE_PREFIX_PATH="$prefix"
  cmake --build "$work/e/build" -j4
fi

grep -q "Gecode_VERSION=" "$work/a/configure.log"
grep -q "Gecode_INCLUDE_DIRS=$expected_include" "$work/a/configure.log"
if [ "$mode" = "mpfr" ]; then
  grep -q '^#define GECODE_HAS_MPFR /\*\*/' "$expected_include/gecode/support/config.hpp"
fi
"$prefix/bin/fzn-gecode" --help
test -x "$prefix/bin/mzn-gecode"
! grep -q "/usr/local" "$prefix/bin/mzn-gecode"
