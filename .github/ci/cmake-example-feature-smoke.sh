#!/usr/bin/env bash
set -euo pipefail

source_dir="${GITHUB_WORKSPACE:-$(pwd)}"
build_root="${RUNNER_TEMP:-/tmp}/gecode-example-features"

rm -rf "$build_root"

cmake -S "$source_dir" -B "$build_root/int" -G Ninja \
  -DGECODE_ENABLE_EXAMPLES=ON \
  -DGECODE_ENABLE_SET_VARS=OFF \
  -DGECODE_ENABLE_FLOAT_VARS=OFF \
  -DGECODE_ENABLE_FLATZINC=OFF \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_GIST=OFF

grep -q '^GECODE_ENABLE_SET_VARS:BOOL=OFF$' "$build_root/int/CMakeCache.txt"
grep -q '^GECODE_ENABLE_FLOAT_VARS:BOOL=OFF$' "$build_root/int/CMakeCache.txt"
cmake --build "$build_root/int" --target help > "$build_root/int-targets.txt"
grep -q '^tsp:' "$build_root/int-targets.txt"
! grep -q '^crew:' "$build_root/int-targets.txt"
! grep -q '^cartesian-heart:' "$build_root/int-targets.txt"
! grep -q '^archimedean-spiral:' "$build_root/int-targets.txt"
cmake --build "$build_root/int" --target tsp

cmake -S "$source_dir" -B "$build_root/optional" -G Ninja \
  -DGECODE_ENABLE_EXAMPLES=ON \
  -DGECODE_ENABLE_SET_VARS=ON \
  -DGECODE_ENABLE_FLOAT_VARS=ON \
  -DGECODE_ENABLE_MPFR=OFF \
  -DGECODE_ENABLE_FLATZINC=OFF \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_GIST=OFF

cmake --build "$build_root/optional" --target help > "$build_root/optional-targets.txt"
grep -q '^crew:' "$build_root/optional-targets.txt"
grep -q '^cartesian-heart:' "$build_root/optional-targets.txt"
! grep -q '^archimedean-spiral:' "$build_root/optional-targets.txt"
