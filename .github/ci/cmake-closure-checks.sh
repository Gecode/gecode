#!/usr/bin/env bash
set -euxo pipefail

cmake -S "$GITHUB_WORKSPACE" -B "$RUNNER_TEMP/gecode-closure-minimodel" \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_GIST=OFF \
  -DGECODE_ENABLE_MINIMODEL=ON \
  -DGECODE_ENABLE_INT_VARS=OFF
grep -q 'GECODE_ENABLE_INT_VARS:BOOL=ON' "$RUNNER_TEMP/gecode-closure-minimodel/CMakeCache.txt"

cmake -S "$GITHUB_WORKSPACE" -B "$RUNNER_TEMP/gecode-closure-examples" \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_GIST=OFF \
  -DGECODE_ENABLE_EXAMPLES=ON \
  -DGECODE_ENABLE_SET_VARS=OFF
grep -q 'GECODE_ENABLE_SET_VARS:BOOL=ON' "$RUNNER_TEMP/gecode-closure-examples/CMakeCache.txt"

cmake -S "$GITHUB_WORKSPACE" -B "$RUNNER_TEMP/gecode-no-cpprofiler" \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_GIST=OFF \
  -DGECODE_ENABLE_CPPROFILER=OFF \
  -DGECODE_ENABLE_MPFR=OFF \
  -DGECODE_ENABLE_EXAMPLES=OFF
! grep -q -- '--cp-profiler' "$RUNNER_TEMP/gecode-no-cpprofiler/tools/flatzinc/gecode.msc"
