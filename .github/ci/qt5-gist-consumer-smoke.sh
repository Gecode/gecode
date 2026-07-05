#!/usr/bin/env bash
set -euxo pipefail

prefix="${1:?usage: qt5-gist-consumer-smoke.sh <install-prefix>}"
work="$RUNNER_TEMP/gecode-qt5-gist-consumer"
rm -rf "$work"
mkdir -p "$work"

cat > "$work/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(qt5_gist_consumer LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS gist)
add_executable(qt5_gist_consumer main.cpp)
target_link_libraries(qt5_gist_consumer PRIVATE Gecode::gecodegist)
get_target_property(gecodegist_links Gecode::gecodegist_shared INTERFACE_LINK_LIBRARIES)
if(NOT gecodegist_links MATCHES "Qt5::Widgets")
  message(FATAL_ERROR "Expected Gecode::gecodegist_shared to link Qt5::Widgets, got: ${gecodegist_links}")
endif()
EOF
cat > "$work/main.cpp" <<'EOF'
#include <gecode/gist.hh>
int main(void) { return 0; }
EOF

cmake -S "$work" -B "$work/build" -DCMAKE_PREFIX_PATH="$prefix"
cmake --build "$work/build" -j4
