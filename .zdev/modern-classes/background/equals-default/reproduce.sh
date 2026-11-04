#!/bin/sh
# Usage (from the source root): sh .zdev/modern-classes/background/equals-default/reproduce.sh BUILD_DIR [JOBS]
# Rewrites the special members of task modern-classes-003 on a tree at
# 0ecc0550d2 (or a rebase of it). BUILD_DIR is a configured and built CMake
# tree with the options of the inventory's build/full, so that clang-tidy and
# libclang see the same code as misc/check.py lint. Needs LLVM 22, uv, and
# Bison 3.8 and Flex 2.6 (Homebrew paths below).
set -e
D=.zdev/modern-classes/background/equals-default
B=$(cd "$1" && pwd)
J=${2:-10}
OUT=$B/equals-default
PATH=/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:/opt/homebrew/opt/llvm@22/bin:$PATH
export PATH
# The generated variable headers change through their generator.
git apply "$D/genvarimp.patch"
cmake --build "$B" --target regenerate
# Members that stay hand-written get NOLINT before the fix run.
python3 "$D/nolint.py" "$D/keep.tsv"
rm -rf "$OUT"
python3 "$D/run.py" "$B" "$OUT" "$J" equals
mv "$OUT"/fixes-*.yaml "$OUT/fixes/"
clang-apply-replacements "$OUT/fixes"
# The fix leaves out-of-line = default definitions; move them into the class.
python3 "$D/relocate.py" "$B" "$J"
uv run misc/check.py format
# Manual candidates, .clang-tidy, docs/code-style.md, and changelog.in.
git apply "$D/manual.patch"
uv run misc/check.py format
