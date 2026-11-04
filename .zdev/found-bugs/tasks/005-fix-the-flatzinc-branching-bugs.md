+++
schema_version = 1
id = "found-bugs-005"
key = "flatzinc"
area = "found-bugs"
status = "open"
complexity = "standard"
afk = true
priority = "high"
blocked_by = ["found-bugs-004"]
+++
# Fix the FlatZinc branching bugs

## Outcome

FlatZinc honours AFC and action decay, set value annotations see the relation names, and AuxVarBrancher's copying is confirmed or fixed.

## Context

IntBoolVarBranch(Select, double d) : VarBranch<IntVar>(d) (gecode/flatzinc/branch.hpp:36-37) resolves to VarBranch(Rnd) through Rnd(unsigned), so decay is ignored; a probe confirmed it (build/zdev-probe-007/decay.cpp, if still present). annotation_to_set_val_branch takes r0 and r1 by value (gecode/flatzinc/flatzinc.cpp:670), so the relation names it sets never reach the caller at :1269 and :1284. AuxVarBrancher's cloning constructor (flatzinc.cpp:137) copies only `done`: check whether the members are set elsewhere and fix it if not. Consider making Rnd(unsigned) explicit if that is the root cause and the change is small.

## Done when

- [ ] Each is fixed or shown not to be a bug, with a Test::FlatZinc test where possible.

## Validation

- -test FlatZinc in build/full.
