+++
schema_version = 1
id = "found-bugs-001"
key = "test-harness"
area = "found-bugs"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = []
+++
# Fix the float and set test harness bugs

## Outcome

The float reification test compares where it assigned, set expression tests name their relation, and the MaybeType comments are correct.

## Context

test/float.cpp:722 reads `(sol = MT_FALSE)`, so the branch never runs and `sol` is overwritten for the RM_PMI checks below. Check the sibling reification tests and test/int.cpp and test/set.cpp for the same slip. Fixing it may expose a real failure: diagnose that rather than weakening the check. test/set/mm-set.cpp:151 and :278 call `str(srt0)` on a SetRelType through Test::Int::Test, so the relation prints as an integer; use Test::Set::SetTest::str, which renames those tests. test/float.hh:64-65 has MT_FALSE and MT_TRUE comments swapped and written `//<`, which Doxygen ignores.

## Done when

- [x] test/float.cpp compares `sol` and the corrected branch runs and passes.
- [x] mm-set test names show the relation's name.
- [x] The MaybeType comments are correct Doxygen.

## Validation

- Build gecode-test and run -test Float:: and -test MiniModel::SetExpr in build/full.

## Result

test/float.cpp:722 compares sol instead of assigning it, so the branch runs and an uncertain MT_MAYBE stays uncertain for the later RM_PMI checks. mm-set test names show the relation through Set::SetTest::str. The MaybeType comments are correct Doxygen. No same slip exists elsewhere in the harnesses.

Validation:

- Float:: 1566/1566 and MiniModel::SetExpr 12280/12280 in build/full; a temporary mutation made Float::Rel fail 44/126, proving the branch runs.
- Fresh verifier (gpt-6.1-sol high) PASS with no findings.
