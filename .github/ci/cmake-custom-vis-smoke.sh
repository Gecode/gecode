#!/usr/bin/env bash
set -euxo pipefail

repo_root="$(git rev-parse --show-toplevel)"
temp_root="${RUNNER_TEMP:-${TMPDIR:-/tmp}}"
work_parent=""
fixture_dir=""

cleanup() {
  if [[ -n "$fixture_dir" &&
        "$fixture_dir" == "$repo_root"/.cmake-custom-vis-smoke.* ]]; then
    rm -rf "$fixture_dir"
  fi
  if [[ -n "$work_parent" &&
        "$work_parent" == "$temp_root"/gecode-cmake-custom-vis-smoke.* ]]; then
    rm -rf "$work_parent"
  fi
}
trap cleanup EXIT

work_parent="$(mktemp -d "$temp_root/gecode-cmake-custom-vis-smoke.XXXXXX")"
fixture_dir="$(mktemp -d "$repo_root/.cmake-custom-vis-smoke.XXXXXX")"
fixture_relative="${fixture_dir#"$repo_root"/}"
build_dir="$work_parent/build"
mkdir -p "$fixture_dir/custom vis"

awk '
  /^Name:[[:space:]]*Int$/ { print "Name:           CustomOne"; next }
  /^Namespace:[[:space:]]*Gecode::Int$/ { print "Namespace:      Gecode::CustomOne"; next }
  /^Ifdef:[[:space:]]*GECODE_HAS_INT_VARS$/ { next }
  { print }
' "$repo_root/gecode/int/var-imp/int.vis" > "$fixture_dir/custom vis/custom one.vis"

awk '
  /^Name:[[:space:]]*Int$/ { print "Name:           CustomTwo"; next }
  /^Namespace:[[:space:]]*Gecode::Int$/ { print "Namespace:      Gecode::CustomTwo"; next }
  /^Ifdef:[[:space:]]*GECODE_HAS_INT_VARS$/ { next }
  { print }
' "$repo_root/gecode/int/var-imp/int.vis" > "$fixture_dir/custom-two.vis"

source_type_hash="$(cmake -E sha256sum "$repo_root/gecode/kernel/var-type.hpp")"
source_imp_hash="$(cmake -E sha256sum "$repo_root/gecode/kernel/var-imp.hpp")"

cmake -S "$repo_root" -B "$build_dir" -G Ninja \
  -DGECODE_WITH_VIS="$fixture_relative/custom vis/custom one.vis,$fixture_dir/custom-two.vis" \
  -DGECODE_REGENERATE_VARIMP=OFF \
  -DGECODE_ENABLE_EXAMPLES=OFF \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_GIST=OFF \
  -DBUILD_TESTING=OFF

grep -q '^GECODE_REGENERATE_VARIMP:BOOL=OFF$' "$build_dir/CMakeCache.txt"
cmake --build "$build_dir" --target gecode-varimp-gen -v

type_header="$build_dir/gecode/kernel/var-type.hpp"
imp_header="$build_dir/gecode/kernel/var-imp.hpp"
grep -q 'CustomOneVarImpConf' "$type_header"
grep -q 'CustomTwoVarImpConf' "$type_header"
grep -q 'CustomOneVarImp' "$imp_header"
grep -q 'CustomTwoVarImp' "$imp_header"

custom_one_line="$(grep -n -m1 'class CustomOneVarImpConf' "$type_header" | cut -d: -f1)"
custom_two_line="$(grep -n -m1 'class CustomTwoVarImpConf' "$type_header" | cut -d: -f1)"
int_line="$(grep -n -m1 'class IntVarImpConf' "$type_header" | cut -d: -f1)"
bool_line="$(grep -n -m1 'class BoolVarImpConf' "$type_header" | cut -d: -f1)"
set_line="$(grep -n -m1 'class SetVarImpConf' "$type_header" | cut -d: -f1)"
float_line="$(grep -n -m1 'class FloatVarImpConf' "$type_header" | cut -d: -f1)"
test "$int_line" -lt "$bool_line"
test "$bool_line" -lt "$set_line"
test "$set_line" -lt "$float_line"
test "$float_line" -lt "$custom_one_line"
test "$custom_one_line" -lt "$custom_two_line"

mtime() {
  stat -f '%m' "$1" 2>/dev/null || stat -c '%Y' "$1"
}
type_mtime="$(mtime "$type_header")"
imp_mtime="$(mtime "$imp_header")"
cmake -E sleep 1
printf '\n' >> "$fixture_dir/custom vis/custom one.vis"
cmake --build "$build_dir" --target gecode-varimp-gen -v 2>&1 | tee "$work_parent/rebuild.log"
test "$(mtime "$type_header")" -gt "$type_mtime"
test "$(mtime "$imp_header")" -gt "$imp_mtime"
grep -q 'GenerateVarImp.cmake' "$work_parent/rebuild.log"

# CMake list syntax is supported in addition to the command-line comma form.
cmake -S "$repo_root" -B "$work_parent/list-build" -G Ninja \
  -DGECODE_WITH_VIS="$fixture_relative/custom vis/custom one.vis;$fixture_dir/custom-two.vis" \
  -DGECODE_ENABLE_EXAMPLES=OFF \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_GIST=OFF \
  -DBUILD_TESTING=OFF
cmake --build "$work_parent/list-build" --target gecode-varimp-gen

if cmake -S "$repo_root" -B "$work_parent/missing-build" -G Ninja \
    -DGECODE_WITH_VIS="$fixture_dir/does-not-exist.vis" \
    -DGECODE_ENABLE_EXAMPLES=OFF -DBUILD_TESTING=OFF \
    >"$work_parent/missing.log" 2>&1; then
  echo "Missing GECODE_WITH_VIS file unexpectedly configured" >&2
  exit 1
fi
grep -q "does not exist" "$work_parent/missing.log"
grep -q "does-not-exist.vis" "$work_parent/missing.log"

if cmake -S "$repo_root" -B "$work_parent/directory-build" -G Ninja \
    -DGECODE_WITH_VIS="$fixture_dir/custom vis" \
    -DGECODE_ENABLE_EXAMPLES=OFF -DBUILD_TESTING=OFF \
    >"$work_parent/directory.log" 2>&1; then
  echo "Directory GECODE_WITH_VIS entry unexpectedly configured" >&2
  exit 1
fi
grep -q "directory; expected a .vis file" "$work_parent/directory.log"

test "$source_type_hash" = "$(cmake -E sha256sum "$repo_root/gecode/kernel/var-type.hpp")"
test "$source_imp_hash" = "$(cmake -E sha256sum "$repo_root/gecode/kernel/var-imp.hpp")"

# Clearing the custom list restores the user's cached OFF choice and makes the
# generation target a no-op in the same build tree.
cmake -S "$repo_root" -B "$build_dir" -G Ninja \
  -DGECODE_WITH_VIS= \
  -DGECODE_REGENERATE_VARIMP=OFF \
  -DGECODE_ENABLE_EXAMPLES=OFF \
  -DGECODE_ENABLE_QT=OFF \
  -DGECODE_ENABLE_GIST=OFF \
  -DBUILD_TESTING=OFF
grep -q '^GECODE_REGENERATE_VARIMP:BOOL=OFF$' "$build_dir/CMakeCache.txt"
rm -f "$type_header" "$imp_header"
cmake --build "$build_dir" --target gecode-varimp-gen -v \
  >"$work_parent/cleared.log" 2>&1
test ! -e "$type_header"
test ! -e "$imp_header"
grep -q 'no work to do' "$work_parent/cleared.log"
