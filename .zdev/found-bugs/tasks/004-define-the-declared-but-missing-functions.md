+++
schema_version = 1
id = "found-bugs-004"
key = "missing-definitions"
area = "found-bugs"
status = "done"
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

- [x] All these declarations have matching definitions, and a test calls each.

## Validation

- Focused branch and float tests in build/full.

## Result

Define the assign TieBreak overloads and the FloatValArgs initializer-list constructor, plus the same kind of gaps an audit found, and make plain assign honour its variable selection for all four variable types.

Validation:

- gecode-test -threads 8: Assign 37/37, LDSB 40/40, Set::Var 9/9, Float::Linear::InitializerList 1/1, -iter 2 -fixprob 1 -tag check 546/546
- Set and Float VarSelection tests fail with ViewSelNone restored; job-shop ft06 still proves makespan 55; tidy and format pass
