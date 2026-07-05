#!/usr/bin/env bash
set -euxo pipefail

prefix="${1:?usage: qt-gist-consumer-smoke.sh <install-prefix>}"
work="$RUNNER_TEMP/gecode-qt-gist-consumer"
rm -rf "$work"
mkdir -p "$work"

cat > "$work/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(qt_gist_consumer LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS gist flatzinc)
add_executable(qt_gist_consumer main.cpp)
target_link_libraries(qt_gist_consumer PRIVATE Gecode::gecodegist Gecode::gecodeflatzinc)
get_target_property(gecodeflatzinc_links Gecode::gecodeflatzinc_shared INTERFACE_LINK_LIBRARIES)
if(NOT gecodeflatzinc_links MATCHES "Qt6::Core")
  message(FATAL_ERROR "Expected Gecode::gecodeflatzinc_shared to link Qt6::Core, got: ${gecodeflatzinc_links}")
endif()
EOF
cat > "$work/main.cpp" <<'EOF'
#include <gecode/gist.hh>
#include <gecode/flatzinc.hh>
int main(void) { return 0; }
EOF

cmake -S "$work" -B "$work/build" -DCMAKE_PREFIX_PATH="$prefix"
cmake --build "$work/build" -j4
