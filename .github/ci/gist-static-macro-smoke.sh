#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
temp_root="${RUNNER_TEMP:-${TMPDIR:-/tmp}}"
work="$(mktemp -d "$temp_root/gecode-gist-static-macro.XXXXXX")"
trap 'rm -rf -- "$work"' EXIT

mkdir -p "$work/stubs/gecode/gist" "$work/stubs/gecode"
touch \
  "$work/stubs/gecode/kernel.hh" \
  "$work/stubs/gecode/search.hh" \
  "$work/stubs/gecode/int.hh" \
  "$work/stubs/gecode/set.hh" \
  "$work/stubs/gecode/float.hh" \
  "$work/stubs/gecode/gist/treecanvas.hh" \
  "$work/stubs/gecode/gist/nodestats.hh"

cxx="${CXX:-c++}"

dump_macros() {
  header=$1
  platform=$2
  shift 2
  "$cxx" -E -dM -x c++ \
    -I"$work/stubs" -I"$repo_root" \
    "$platform" -DGECODE_NO_AUTOLINK "$@" \
    -include "$header" /dev/null
}

check_header() {
  header=$1
  platform=$2

  legacy="$(dump_macros "$header" "$platform" -DGIST_STATIC_LIBS)"
  grep -Eq '^#define GECODE_GIST_EXPORT[[:space:]]*$' <<<"$legacy"
  ! grep -Eq '^#define GECODE_STATIC_LIBS([[:space:]]|$)' <<<"$legacy"

  current="$(dump_macros "$header" "$platform" -DGECODE_STATIC_LIBS)"
  grep -Eq '^#define GECODE_GIST_EXPORT[[:space:]]*$' <<<"$current"
  ! grep -Eq '^#define GIST_STATIC_LIBS([[:space:]]|$)' <<<"$current"

  shared="$(dump_macros "$header" "$platform")"
  grep -Eq '^#define GECODE_GIST_EXPORT __declspec\([[:space:]]*dllimport[[:space:]]*\)$' \
    <<<"$shared"

  building="$(dump_macros "$header" "$platform" -DGECODE_BUILD_GIST)"
  grep -Eq '^#define GECODE_GIST_EXPORT __declspec\([[:space:]]*dllexport[[:space:]]*\)$' \
    <<<"$building"
}

for header in gecode/gist.hh gecode/gist/qtgist.hh; do
  check_header "$header" -D__MINGW32__
  check_header "$header" -D_MSC_VER=1930
done
