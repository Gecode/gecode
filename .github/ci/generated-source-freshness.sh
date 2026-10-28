#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -gt 1 ]; then
  echo "usage: $0 [--check|--update]" >&2
  exit 2
fi
mode=check
case "${1:-}" in
  ""|--check) ;;
  --update) mode=update ;;
  *)
    echo "usage: $0 [--check|--update]" >&2
    exit 2
    ;;
esac

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

prepend_brew_bin() {
  formula=$1
  if ! command -v brew >/dev/null 2>&1; then
    return
  fi
  prefix=$(brew --prefix "$formula" 2>/dev/null || true)
  if [ -n "$prefix" ] && [ -d "$prefix/bin" ]; then
    PATH="$prefix/bin:$PATH"
  fi
}

# Homebrew keeps these tools keg-only on macOS. Make the same helper
# work from a developer shell without requiring them to edit PATH manually.
prepend_brew_bin bison
prepend_brew_bin flex
prepend_brew_bin llvm
prepend_brew_bin llvm@22
export PATH

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
    echo "Install the required generator, then run 'bash .github/ci/generated-source-freshness.sh --update'." >&2
    exit 1
  fi
}

require_version bison "bison (GNU Bison) 3.8.2"
require_version flex "flex 2.6.4"
if ! command -v uv >/dev/null 2>&1; then
  echo "Required generator runner is unavailable: uv" >&2
  exit 1
fi

vis_files=(
  ./gecode/int/var-imp/int.vis
  ./gecode/int/var-imp/bool.vis
  ./gecode/set/var-imp/set.vis
  ./gecode/float/var-imp/float.vis
)

# Copy generator inputs with their source-relative paths. Generated outputs stay
# isolated until --update explicitly copies them back to the source tree.
for input in .clang-format misc/genvarimp.py "${vis_files[@]}" \
    gecode/flatzinc/parser.yxx gecode/flatzinc/lexer.lxx; do
  mkdir -p "$source_copy/$(dirname "$input")"
  cp "$repo_root/$input" "$source_copy/$input"
done
mkdir -p "$source_copy/gecode/kernel"

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
  if [ "$mode" = update ]; then
    cp "$source_copy/$file" "$repo_root/$file"
    echo "Updated generated source: $file"
    continue
  fi
  stale=1
  echo "Generated source is stale: $file" >&2
  diff -u \
    --label "tracked/$file" \
    --label "regenerated/$file" \
    "$repo_root/$file" "$source_copy/$file" >&2 || true
done

if [ "$mode" = update ]; then
  echo "Generated source regeneration completed"
  exit 0
fi

if [ "$stale" -ne 0 ]; then
  echo "Regenerate with GNU Bison 3.8.2, GNU Flex 2.6.4, clang-format 22, and uv, then commit the reviewed outputs." >&2
  echo "Run 'bash .github/ci/generated-source-freshness.sh --update' to use the canonical generator workflow." >&2
  exit 1
fi

echo "Generated source freshness check passed"
