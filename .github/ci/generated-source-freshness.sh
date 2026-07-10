#!/usr/bin/env bash
set -euo pipefail

repo_root="${GECODE_SOURCE_ROOT:-$(git rev-parse --show-toplevel)}"
temp_root="${RUNNER_TEMP:-${TMPDIR:-/tmp}}"

if [ ! -d "$temp_root" ]; then
  echo "Temporary root is not a directory: $temp_root" >&2
  exit 1
fi
if [ ! -d "$repo_root/.git" ] && [ ! -f "$repo_root/.git" ]; then
  echo "Source root is not a Git worktree: $repo_root" >&2
  exit 1
fi

cleanup_root=
workspace=
pending_signal=
cleanup() {
  if [ -n "$cleanup_root" ]; then
    rm -rf -- "$cleanup_root"
  fi
}
trap cleanup EXIT

# Defer signals while ownership moves from an unowned candidate name to a
# successfully created directory.  On collision, no cleanup path is recorded;
# on success, a pending signal is handled only after cleanup owns the path.
trap 'pending_signal=HUP' HUP
trap 'pending_signal=INT' INT
trap 'pending_signal=TERM' TERM

umask 077
for attempt in {1..100}; do
  candidate="$temp_root/gecode-generated-check.${BASHPID:-$$}.$RANDOM.$RANDOM.$attempt"
  if mkdir -m 700 "$candidate"; then
    cleanup_root=$candidate
    break
  fi
  if [ -n "$pending_signal" ]; then
    exit 1
  fi
done
if [ -z "$cleanup_root" ]; then
  echo "Could not create a private generated-file workspace below $temp_root" >&2
  exit 1
fi
if [ -n "$pending_signal" ]; then
  exit 1
fi
trap 'exit 1' HUP INT TERM
workspace=$(mktemp -d "$cleanup_root/work.XXXXXX")
source_copy="$workspace/source"
mkdir "$source_copy"

require_version() {
  tool=$1
  expected=$2
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "Required generator is unavailable: $tool ($expected required)" >&2
    exit 1
  fi
  actual=$($tool --version | sed -n '1p')
  if [ "$actual" != "$expected" ]; then
    echo "Unsupported $tool version." >&2
    echo "  required: $expected" >&2
    echo "  found:    $actual" >&2
    echo "Use the pinned generator version or regenerate and review all tracked outputs intentionally." >&2
    exit 1
  fi
}

require_version autoconf "autoconf (GNU Autoconf) 2.72"
require_version bison "bison (GNU Bison) 3.8.2"
require_version flex "flex 2.6.4"
if ! command -v uv >/dev/null 2>&1; then
  echo "Required generator runner is unavailable: uv" >&2
  exit 1
fi

# Copy the current tracked worktree content, including local tracked changes,
# without copying build products or allowing generators to touch the source.
git -C "$repo_root" ls-files -z |
  tar -C "$repo_root" --null -T - -cf - |
  tar -C "$source_copy" -xf -

vis_files=(
  ./gecode/int/var-imp/int.vis
  ./gecode/int/var-imp/bool.vis
  ./gecode/set/var-imp/set.vis
  ./gecode/float/var-imp/float.vis
)

normalize_generated_whitespace() {
  output=$1
  awk '
    {
      sub(/[ \t\r]+$/, "")
      line[NR] = $0
    }
    END {
      last = NR
      while ((last > 0) && (line[last] == ""))
        last--
      for (i = 1; i <= last; i++)
        print line[i]
    }
  ' "$output" > "$output.normalized"
  mv "$output.normalized" "$output"
}

(
  cd "$source_copy"
  # Ignore any autom4te cache so output matches a clean source release.
  autoconf --force
  # Autoconf 2.72 appends blank lines after the generated script body; source
  # releases retain exactly one final newline.
  normalize_generated_whitespace configure
  uv run --script ./misc/genvarimp.py -typehpp "${vis_files[@]}" \
    > gecode/kernel/var-type.hpp
  uv run --script ./misc/genvarimp.py -header "${vis_files[@]}" \
    > gecode/kernel/var-imp.hpp
  bison -t -o gecode/flatzinc/parser.tab.cpp -d \
    ./gecode/flatzinc/parser.yxx
  # Source releases keep generated Bison files free of whitespace-only lines
  # and excess blank lines at EOF.  Bison 3.8.2 preserves indentation from an
  # otherwise blank grammar line and appends a blank line to its .cpp output.
  for output in \
      gecode/flatzinc/parser.tab.cpp \
      gecode/flatzinc/parser.tab.hpp; do
    normalize_generated_whitespace "$output"
  done
  flex -ogecode/flatzinc/lexer.yy.cpp ./gecode/flatzinc/lexer.lxx
)

generated_files=(
  configure
  gecode/kernel/var-type.hpp
  gecode/kernel/var-imp.hpp
  gecode/flatzinc/parser.tab.cpp
  gecode/flatzinc/parser.tab.hpp
  gecode/flatzinc/lexer.yy.cpp
)

stale=0
for file in "${generated_files[@]}"; do
  if cmp -s "$repo_root/$file" "$source_copy/$file"; then
    continue
  fi
  stale=1
  echo "Generated source is stale: $file" >&2
  diff -u \
    --label "tracked/$file" \
    --label "regenerated/$file" \
    "$repo_root/$file" "$source_copy/$file" >&2 || true
done

if [ "$stale" -ne 0 ]; then
  echo "Regenerate with Autoconf 2.72, GNU Bison 3.8.2, GNU Flex 2.6.4, and uv, then commit the reviewed outputs." >&2
  exit 1
fi

echo "Generated source freshness check passed"
