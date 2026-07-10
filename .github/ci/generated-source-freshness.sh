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
autoconf_build_root=
workspace=
pending_signal=
cleanup() {
  if [ -n "$autoconf_build_root" ]; then
    rm -rf -- "$autoconf_build_root"
  fi
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

autoconf_version=2.72
autoconf_expected="autoconf (GNU Autoconf) $autoconf_version"
autoconf_sha256=afb181a76e1ee72832f6581c0eddf8df032b83e2e0239ef79ebedc4467d92d6e

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

# Homebrew keeps bison, flex, and m4 keg-only on macOS. Make the same helper
# work from a developer shell without requiring them to edit PATH manually.
prepend_brew_bin m4
prepend_brew_bin bison
prepend_brew_bin flex
export PATH

autoconf_is_expected() {
  command -v autoconf >/dev/null 2>&1 &&
    [ "$(autoconf --version | sed -n '1p')" = "$autoconf_expected" ]
}

ensure_autoconf() {
  if autoconf_is_expected; then
    return
  fi

  if [ -n "${GECODE_GENERATED_TOOLS_DIR:-}" ]; then
    tools_root=$GECODE_GENERATED_TOOLS_DIR
  elif [ -n "${RUNNER_TEMP:-}" ]; then
    tools_root="$RUNNER_TEMP/gecode-generated-tools"
  else
    tools_root="${XDG_CACHE_HOME:-$HOME/.cache}/gecode/generated-tools"
  fi
  install_root="$tools_root/autoconf-$autoconf_version"

  if [ -x "$install_root/bin/autoconf" ]; then
    PATH="$install_root/bin:$PATH"
    export PATH
    if autoconf_is_expected; then
      return
    fi
  fi

  for tool in curl make tar; do
    if ! command -v "$tool" >/dev/null 2>&1; then
      echo "Cannot bootstrap Autoconf $autoconf_version: missing $tool." >&2
      exit 1
    fi
  done
  if ! command -v shasum >/dev/null 2>&1 &&
     ! command -v sha256sum >/dev/null 2>&1; then
    echo "Cannot bootstrap Autoconf $autoconf_version: no SHA-256 checker is available." >&2
    exit 1
  fi
  if ! command -v m4 >/dev/null 2>&1; then
    echo "Cannot bootstrap Autoconf $autoconf_version: GNU m4 is unavailable." >&2
    exit 1
  fi

  mkdir -p "$tools_root"
  autoconf_build_root=$(mktemp -d "$temp_root/gecode-autoconf-build.XXXXXX")
  archive="$autoconf_build_root/autoconf-$autoconf_version.tar.gz"
  curl --fail --silent --show-error --location --retry 3 \
    --output "$archive" \
    "https://ftp.gnu.org/gnu/autoconf/autoconf-$autoconf_version.tar.gz"
  if command -v shasum >/dev/null 2>&1; then
    printf '%s  %s\n' "$autoconf_sha256" "$archive" | shasum -a 256 -c -
  else
    printf '%s  %s\n' "$autoconf_sha256" "$archive" | sha256sum -c -
  fi
  tar -xzf "$archive" -C "$autoconf_build_root"
  rm -rf -- "$install_root"
  (
    cd "$autoconf_build_root/autoconf-$autoconf_version"
    ./configure --prefix="$install_root" >/dev/null
    make -j2 >/dev/null
    make install >/dev/null
  )
  PATH="$install_root/bin:$PATH"
  export PATH
}

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
    echo "Install the required generator, then run 'make regenerate'." >&2
    exit 1
  fi
}

ensure_autoconf
require_version autoconf "$autoconf_expected"
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
  echo "Regenerate with Autoconf 2.72, GNU Bison 3.8.2, GNU Flex 2.6.4, and uv, then commit the reviewed outputs." >&2
  echo "Run 'make regenerate' to use the canonical generator workflow." >&2
  exit 1
fi

echo "Generated source freshness check passed"
