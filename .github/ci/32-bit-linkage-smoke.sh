#!/usr/bin/env bash
set -euxo pipefail

source_dir="${1:?usage: 32-bit-linkage-smoke.sh <source-dir> <work-dir>}"
work_dir="${2:?usage: 32-bit-linkage-smoke.sh <source-dir> <work-dir>}"
build_dir="$work_dir/build"
install_dir="$work_dir/install"
consumer_dir="$work_dir/consumer"

rm -rf "$work_dir"
mkdir -p "$work_dir"

cmake -S "$source_dir" -B "$build_dir" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS=-m32 \
  -DCMAKE_CXX_FLAGS=-m32 \
  -DGECODE_BUILD_SHARED=OFF \
  -DGECODE_BUILD_STATIC=ON \
  -DGECODE_ENABLE_EXAMPLES=OFF \
  -DGECODE_ENABLE_FLATZINC=OFF \
  -DGECODE_ENABLE_DRIVER=OFF \
  -DGECODE_ENABLE_MINIMODEL=OFF \
  -DGECODE_ENABLE_FLOAT_VARS=OFF \
  -DGECODE_ENABLE_SET_VARS=OFF \
  -DGECODE_ENABLE_INT_VARS=OFF \
  -DGECODE_ENABLE_CPPROFILER=OFF \
  -DGECODE_ENABLE_GIST=OFF \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_MPFR=OFF \
  -DGECODE_INSTALL=ON
cmake --build "$build_dir" -j2
cmake --install "$build_dir" --prefix "$install_dir"

mkdir -p "$consumer_dir"
cat > "$consumer_dir/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(gecode_32_bit_consumer LANGUAGES CXX)

find_package(Gecode CONFIG REQUIRED COMPONENTS search)
add_executable(gecode-32-bit-stop-consumer main.cpp)
target_link_libraries(gecode-32-bit-stop-consumer PRIVATE Gecode::gecodesearch)
EOF
cat > "$consumer_dir/main.cpp" <<'EOF'
#include <gecode/search.hh>

int main(void) {
  Gecode::Search::Statistics statistics;
  Gecode::Search::Options options;

  Gecode::Search::NodeStop node(1);
  Gecode::Search::NodeStop node_copy(node);
  node_copy = node;
  node.limit(2);
  (void) node.stop(statistics, options);

  Gecode::Search::FailStop fail(1);
  Gecode::Search::FailStop fail_copy(fail);
  fail_copy = fail;
  fail.limit(2);
  (void) fail.stop(statistics, options);

  Gecode::Search::TimeStop time(1000.0);
  Gecode::Search::TimeStop time_copy(time);
  time_copy = time;
  time.reset();
  (void) time.stop(statistics, options);

  Gecode::Search::RestartStop restart(1);
  Gecode::Search::RestartStop restart_copy(restart);
  restart_copy = restart;
  restart.limit(2);
  (void) restart.stop(statistics, options);

  return 0;
}
EOF

cmake -S "$consumer_dir" -B "$consumer_dir/build" \
  -DCMAKE_CXX_FLAGS=-m32 \
  -DCMAKE_PREFIX_PATH="$install_dir"
cmake --build "$consumer_dir/build"
binary_type="$(file "$consumer_dir/build/gecode-32-bit-stop-consumer")"
printf '%s\n' "$binary_type"
binary_type="${binary_type#*: }"
case "$binary_type" in
  *"32-bit"*|*"Intel 80386"*) ;;
  *)
    echo "expected a 32-bit consumer binary, got: $binary_type" >&2
    exit 1
    ;;
esac
