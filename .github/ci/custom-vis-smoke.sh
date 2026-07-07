#!/usr/bin/env bash
set -euxo pipefail

repo_root="$(git rev-parse --show-toplevel)"
work_parent="${RUNNER_TEMP:-${TMPDIR:-/tmp}}/gecode-custom-vis-smoke"
work="$work_parent/worktree"

cleanup() {
  git -C "$repo_root" worktree remove --force "$work" >/dev/null 2>&1 || rm -rf "$work"
}
trap cleanup EXIT

rm -rf "$work_parent"
mkdir -p "$work_parent"
git -C "$repo_root" worktree prune
git -C "$repo_root" worktree add --detach "$work" HEAD

cd "$work"
touch configure

awk '
  /^Name:[[:space:]]*Int$/ { print "Name:           Custom"; next }
  /^Namespace:[[:space:]]*Gecode::Int$/ { print "Namespace:      Gecode::Custom"; next }
  /^Ifdef:[[:space:]]*GECODE_HAS_INT_VARS$/ { next }
  { print }
' gecode/int/var-imp/int.vis > custom.vis

CFLAGS="-fPIC" \
CXXFLAGS="-std=c++11 -fPIC -DGECODE_MEMORY_ALIGNMENT=16" \
  ./configure \
    --with-vis="$PWD/custom.vis" \
    --disable-cpprofiler \
    --disable-examples \
    --disable-flatzinc \
    --disable-float-vars \
    --disable-gist \
    --disable-qt

rm -f gecode/kernel/var-type.hpp gecode/kernel/var-imp.hpp
make -j4
grep -q "CustomVarImpConf" gecode/kernel/var-type.hpp
