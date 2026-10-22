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

  # Shared explicit ends and a zero-duration task with excessive usage.
  # Exercise the MiniZinc dispatch as well as all three explicit-end bindings.
  result=$("$prefix/bin/mzn-gecode" - <<'MZN'
include "gecode.mzn";
include "cumulative.mzn";
array[1..3] of var 0..2: d;
array[1..3] of var 1..100: r;
var 1..3: b;
array[1..3] of var 0..3: e;
constraint d = [2,0,1] /\ r = [2,100,1];
constraint forall(i in 1..3)(e[i] = [0,1,0][i] + d[i]);
constraint fzn_cumulative([0,1,0],d,r,b) :: gecode_full_propagation;
constraint gecode_unary_flex([0,2],[2,1],[e[1],3]);
constraint gecode_cumulative_flex([0,0],[2,1],[e[1],e[3]],[2,1],b);
constraint gecode_cumulatives_flex([0,0],[2,2],[e[1],e[1]],
  [3,-2],[0,0],[1],true);
% Keep usages variable so this call uses the alternate-machine encoding.
array[1..2] of var 1..2: zero_r;
var 1..3: zero_b;
constraint sum(zero_r) = 3;
constraint fzn_cumulative([0,0],[0,2],zero_r,zero_b);
solve minimize b + zero_b;
output ["capacity=",show(b)," empty-capacity=",show(zero_b),"\n"];
MZN
  )
  [[ "$result" == *"capacity=3 empty-capacity=1"* ]]

fi
