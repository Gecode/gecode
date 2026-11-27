#!/usr/bin/env bash
# Build a support-only package with scoped mimalloc, check that neither library
# exports allocator symbols, and run the installed coexistence consumers.
set -euxo pipefail

source_dir="${1:?usage: allocator-smoke.sh <source-dir> <work-dir>}"
work_dir="${2:?usage: allocator-smoke.sh <source-dir> <work-dir>}"
build_type="${BUILD_TYPE:-Release}"
build="$work_dir/build"
install="$work_dir/install"

rm -rf "$work_dir"
mkdir -p "$work_dir"

cmake -S "$source_dir" -B "$build" \
  -DCMAKE_BUILD_TYPE="$build_type" \
  -DGECODE_BUILD_SHARED=ON -DGECODE_BUILD_STATIC=ON \
  -DGECODE_ENABLE_MIMALLOC=ON \
  -DGECODE_ENABLE_GCC_VISIBILITY=OFF \
  -DBUILD_TESTING=OFF -DGECODE_ENABLE_EXAMPLES=OFF \
  -DGECODE_ENABLE_SEARCH=OFF -DGECODE_ENABLE_INT_VARS=OFF \
  -DGECODE_ENABLE_SET_VARS=OFF -DGECODE_ENABLE_FLOAT_VARS=OFF \
  -DGECODE_ENABLE_MINIMODEL=OFF -DGECODE_ENABLE_DRIVER=OFF \
  -DGECODE_ENABLE_FLATZINC=OFF -DGECODE_ENABLE_QT=OFF -DGECODE_ENABLE_GIST=OFF
cmake --build "$build" --parallel 4
cmake --install "$build" --prefix "$install"

nm -D --defined-only "$install/lib/libgecodesupport.so" > "$work_dir/exports.txt"
if grep -E '[[:space:]](_*mi_[[:alnum:]_]+|gecode_+mi_[[:alnum:]_]+|malloc|calloc|realloc|free|aligned_alloc|posix_memalign)$' \
    "$work_dir/exports.txt"; then
  echo "Allocator implementation or malloc replacement exported by gecodesupport" >&2
  exit 1
fi
nm -g --defined-only "$install/lib/libgecodesupport.a" > "$work_dir/symbols.txt"
if grep -E '[[:space:]](_*mi_[[:alnum:]_]+|malloc|calloc|realloc|free|aligned_alloc|posix_memalign)$' \
    "$work_dir/symbols.txt"; then
  echo "Unprefixed allocator symbol in static gecodesupport" >&2
  exit 1
fi
grep -q '[[:space:]]gecode_mi_malloc$' "$work_dir/symbols.txt"

# Unsupported allocator combinations must fail at configure time.
if cmake -S "$source_dir" -B "$work_dir/mimalloc-custom" \
    -DGECODE_ENABLE_MIMALLOC=ON -DGECODE_ENABLE_ALLOCATOR=OFF > "$work_dir/mimalloc-custom.log" 2>&1; then
  exit 1
fi
grep -q 'requires GECODE_ENABLE_ALLOCATOR=ON' "$work_dir/mimalloc-custom.log"
if cmake -S "$source_dir" -B "$work_dir/mimalloc-sanitizer" \
    -DGECODE_ENABLE_MIMALLOC=ON -DGECODE_SANITIZER=address > "$work_dir/mimalloc-sanitizer.log" 2>&1; then
  exit 1
fi
grep -q 'does not support GECODE_SANITIZER' "$work_dir/mimalloc-sanitizer.log"

for variant in shared static; do
  consumer="$work_dir/consumer-$variant"
  cmake -S "$source_dir/test/package/allocator" -B "$consumer" \
    -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_PREFIX_PATH="$install" \
    -DGECODE_LINK_VARIANT="$variant" -DGECODE_EXPECT_MIMALLOC=ON \
    -DGECODE_HOST_MIMALLOC_OVERRIDE=ON \
    -DMIMALLOC_SOURCE_DIR="$build/_deps/gecode_mimalloc-src"
  cmake --build "$consumer" --parallel 4
  ctest --test-dir "$consumer" --output-on-failure
done
test -f "$install/share/licenses/gecode/mimalloc-LICENSE"
test ! -f "$install/include/mimalloc.h"
