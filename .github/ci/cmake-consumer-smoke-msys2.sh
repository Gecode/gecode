#!/usr/bin/env bash
set -euxo pipefail

prefix="${1:?usage: cmake-consumer-smoke-msys2.sh <install-prefix>}"
work_root="${RUNNER_TEMP:-${TMPDIR:-/tmp}}"
work="$work_root/gecode-consumer-msys2"

rm -rf "$work"
mkdir -p "$work"

cat > "$work/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(msys2_static_consumer LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED)
if(NOT TARGET Gecode::gecode)
  message(FATAL_ERROR "Expected installed package to export Gecode::gecode")
endif()
add_executable(msys2_static_consumer main.cpp)
target_link_libraries(msys2_static_consumer PRIVATE Gecode::gecode)
EOF

cat > "$work/main.cpp" <<'EOF'
#include <gecode/int.hh>
#include <gecode/support/config.hpp>

int main(void) {
  Gecode::IntArgs values(1);
  values[0] = 0;
  return values[0];
}
EOF

cmake -S "$work" -B "$work/build" -G Ninja -DCMAKE_PREFIX_PATH="$prefix"
cmake --build "$work/build"
