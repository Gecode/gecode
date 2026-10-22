#!/usr/bin/env bash
set -euxo pipefail

prefix="${1:?usage: flatzinc-tools-smoke.sh <install-prefix>}"
"$prefix/bin/fzn-gecode" --help
test -x "$prefix/bin/mzn-gecode"
! grep -q "/usr/local" "$prefix/bin/mzn-gecode"

# Exercise the installed solver and its own library through the launcher.
if command -v minizinc >/dev/null 2>&1; then
  result=$("$prefix/bin/mzn-gecode" - <<'MZN'
include "gecode.mzn";
var 1..12: g;
constraint gecode_gcd(12,18,g);
solve satisfy;
output ["gcd=", show(g), "\n"];
MZN
  )
  [[ "$result" == *"gcd=6"* ]]
fi
