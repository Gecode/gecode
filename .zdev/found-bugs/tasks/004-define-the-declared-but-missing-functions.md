+++
schema_version = 1
id = "found-bugs-004"
key = "missing-definitions"
area = "found-bugs"
status = "open"
complexity = "standard"
afk = true
priority = "high"
blocked_by = ["found-bugs-003"]
+++
# Define the declared but missing functions

## Outcome

The assign overloads with TieBreak and FloatValArgs(std::initializer_list) link.

## Context

gecode/int.hh:5823 and :5843 declare assign with TieBreak<IntVarBranch> and TieBreak<BoolVarBranch>. gecode/int/branch.cpp:145 defines one as `branch` and :258 has mismatched types, so callers fail to link, and an orphan may collide with a real branch overload. gecode/float.hh:1035 declares FloatValArgs(std::initializer_list<FloatVal>) with no definition in gecode/float/array.hpp. Look for other declared-but-undefined exported functions of the same kind.

## Done when

- [ ] All these declarations have matching definitions, and a test calls each.

## Validation

- Focused branch and float tests in build/full.
