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
  /^Name:[[:space:]]*Int$/ { print "Name:           CustomOne"; next }
  /^Namespace:[[:space:]]*Gecode::Int$/ { print "Namespace:      Gecode::CustomOne"; next }
  /^Ifdef:[[:space:]]*GECODE_HAS_INT_VARS$/ { next }
  { print }
' gecode/int/var-imp/int.vis > custom-one.vis

awk '
  /^Name:[[:space:]]*Int$/ { print "Name:           CustomTwo"; next }
  /^Namespace:[[:space:]]*Gecode::Int$/ { print "Namespace:      Gecode::CustomTwo"; next }
  /^Ifdef:[[:space:]]*GECODE_HAS_INT_VARS$/ { next }
  { print }
' gecode/int/var-imp/int.vis > custom-two.vis

CFLAGS="-fPIC" \
CXXFLAGS="-std=c++11 -fPIC -DGECODE_MEMORY_ALIGNMENT=16" \
  dash ./configure \
    --with-vis="$PWD/custom-one.vis,$PWD/custom-two.vis" \
    --disable-cpprofiler \
    --disable-examples \
    --disable-flatzinc \
    --disable-float-vars \
    --disable-gist \
    --disable-qt 2>&1 | tee configure.log

! grep -Fq "unexpected operator" configure.log
grep -Eq '^export DLLSUFFIX[[:space:]]*=[[:space:]]*\.' Makefile
grep -Fq "VIS = \$(top_srcdir)/gecode/int/var-imp/int.vis \$(top_srcdir)/gecode/int/var-imp/bool.vis \$(top_srcdir)/gecode/set/var-imp/set.vis \$(top_srcdir)/gecode/float/var-imp/float.vis $PWD/custom-one.vis $PWD/custom-two.vis" Makefile

rm -f gecode/kernel/var-type.hpp gecode/kernel/var-imp.hpp
make -j4
grep -q "CustomOneVarImpConf" gecode/kernel/var-type.hpp
grep -q "CustomTwoVarImpConf" gecode/kernel/var-type.hpp
