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
  "$work/f" "$work/g" "$work/h" "$work/i" "$work/j" "$work/k" \
  "$work/mpfr-prefix/lib/cmake/MPFR"

cat > "$work/a/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_a LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED)
if(NOT Gecode_LIBRARIES STREQUAL "Gecode::gecode")
  message(FATAL_ERROR "Expected aggregate Gecode_LIBRARIES, got: ${Gecode_LIBRARIES}")
endif()
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
if(NOT Gecode_LIBRARIES STREQUAL "Gecode::gecodedriver")
  message(FATAL_ERROR "Expected driver-only Gecode_LIBRARIES, got: ${Gecode_LIBRARIES}")
endif()
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
if(NOT Gecode_LIBRARIES STREQUAL "Gecode::gecodedriver")
  message(FATAL_ERROR "Expected canonical library for legacy component, got: ${Gecode_LIBRARIES}")
endif()
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
if(NOT Gecode_LIBRARIES STREQUAL "Gecode::gecodefloat")
  message(FATAL_ERROR "Expected float-only Gecode_LIBRARIES, got: ${Gecode_LIBRARIES}")
endif()
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
target_link_libraries(consumer_d PRIVATE ${Gecode_LIBRARIES})
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

cat > "$work/f/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_f LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS support)
if(NOT Gecode_LIBRARIES STREQUAL "Gecode::gecodesupport")
  message(FATAL_ERROR "Expected support-only Gecode_LIBRARIES, got: ${Gecode_LIBRARIES}")
endif()
add_executable(consumer_f main.cpp)
target_link_libraries(consumer_f PRIVATE ${Gecode_LIBRARIES})
EOF
cat > "$work/f/main.cpp" <<'EOF'
#include <gecode/support.hh>
int main(void) { return Gecode::Support::Thread::npu() > 0 ? 0 : 1; }
EOF

cat > "$work/g/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_g LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS gecodesupport)
if(NOT Gecode_LIBRARIES STREQUAL "Gecode::gecodesupport")
  message(FATAL_ERROR "Expected canonical library for legacy component, got: ${Gecode_LIBRARIES}")
endif()
add_executable(consumer_g main.cpp)
target_link_libraries(consumer_g PRIVATE ${Gecode_LIBRARIES})
EOF
cat > "$work/g/main.cpp" <<'EOF'
#include <gecode/support.hh>
int main(void) { return 0; }
EOF

cat > "$work/h/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_h LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS unknown_component)
EOF

cat > "$work/i/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_i LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS float)
EOF

cat > "$work/j/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_j LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS gist)
if(NOT Gecode_LIBRARIES STREQUAL "Gecode::gecodegist")
  message(FATAL_ERROR "Expected gist-only Gecode_LIBRARIES, got: ${Gecode_LIBRARIES}")
endif()
add_executable(consumer_j main.cpp)
target_link_libraries(consumer_j PRIVATE ${Gecode_LIBRARIES})
EOF
cat > "$work/j/main.cpp" <<'EOF'
#include <gecode/gist.hh>
int main(void) { return 0; }
EOF

cat > "$work/k/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(consumer_k LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS support)
find_package(Gecode CONFIG REQUIRED COMPONENTS float)
if(NOT TARGET MPFR::MPFR)
  message(FATAL_ERROR "Expected the second component request to resolve MPFR")
endif()
if(NOT Gecode_LIBRARIES STREQUAL "Gecode::gecodefloat")
  message(FATAL_ERROR "Expected the second request to replace Gecode_LIBRARIES, got: ${Gecode_LIBRARIES}")
endif()
add_executable(consumer_k main.cpp)
target_link_libraries(consumer_k PRIVATE ${Gecode_LIBRARIES})
EOF
cat > "$work/k/main.cpp" <<'EOF'
#include <gecode/float.hh>
int main(void) { Gecode::Float::Rounding rounding; return rounding.sqrt_down(4.0) == 2.0 ? 0 : 1; }
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

# A support-only consumer must not discover dependencies belonging exclusively
# to Float, FlatZinc, or Gist, even when those targets exist in the package.
cmake -S "$work/f" -B "$work/f/build" \
  -DCMAKE_PREFIX_PATH="$prefix" \
  -DCMAKE_DISABLE_FIND_PACKAGE_MPFR=TRUE \
  -DCMAKE_DISABLE_FIND_PACKAGE_Qt5=TRUE \
  -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=TRUE
cmake --build "$work/f/build" -j4

cmake -S "$work/g" -B "$work/g/build" \
  -DCMAKE_PREFIX_PATH="$prefix" \
  -DCMAKE_DISABLE_FIND_PACKAGE_MPFR=TRUE \
  -DCMAKE_DISABLE_FIND_PACKAGE_Qt5=TRUE \
  -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=TRUE
cmake --build "$work/g/build" -j4

if cmake -S "$work/h" -B "$work/h/build" -DCMAKE_PREFIX_PATH="$prefix"; then
  echo "Unknown required Gecode component unexpectedly succeeded" >&2
  exit 1
fi

if [ "$mode" = "mpfr" ]; then
  cmake -S "$work/d" -B "$work/d/build" -DCMAKE_PREFIX_PATH="$prefix"
  cmake --build "$work/d/build" -j4

  cmake -S "$work/e" -B "$work/e/build" -DCMAKE_PREFIX_PATH="$prefix"
  cmake --build "$work/e/build" -j4

  cmake -S "$work/k" -B "$work/k/build" -DCMAKE_PREFIX_PATH="$prefix"
  cmake --build "$work/k/build" -j4

  if cmake -S "$work/i" -B "$work/i/build" \
      -DCMAKE_PREFIX_PATH="$prefix" \
      -DCMAKE_DISABLE_FIND_PACKAGE_MPFR=TRUE; then
    echo "Float component unexpectedly succeeded without MPFR" >&2
    exit 1
  fi
fi

targets_file="$(find "$prefix" -name GecodeTargets.cmake -print -quit)"
if grep -q 'add_library(Gecode::gecodegist ' "$targets_file"; then
  if cmake -S "$work/j" -B "$work/j/no-qt-build" \
      -DCMAKE_PREFIX_PATH="$prefix" \
      -DCMAKE_DISABLE_FIND_PACKAGE_Qt5=TRUE \
      -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=TRUE; then
    echo "Gist component unexpectedly succeeded without Qt" >&2
    exit 1
  fi
  cmake -S "$work/j" -B "$work/j/build" -DCMAKE_PREFIX_PATH="$prefix"
  cmake --build "$work/j/build" -j4
else
  if cmake -S "$work/j" -B "$work/j/missing-build" -DCMAKE_PREFIX_PATH="$prefix"; then
    echo "Missing required Gist component unexpectedly succeeded" >&2
    exit 1
  fi
fi

grep -q "Gecode_VERSION=" "$work/a/configure.log"
grep -q "Gecode_INCLUDE_DIRS=$expected_include" "$work/a/configure.log"
grep -q '^#define GECODE_NO_AUTOLINK 1$' "$expected_include/gecode/support/config.hpp"
if [ "$mode" = "mpfr" ]; then
  grep -q '^#define GECODE_HAS_MPFR /\*\*/' "$expected_include/gecode/support/config.hpp"
fi
"$prefix/bin/fzn-gecode" --help
test -x "$prefix/bin/mzn-gecode"
! grep -q "/usr/local" "$prefix/bin/mzn-gecode"
