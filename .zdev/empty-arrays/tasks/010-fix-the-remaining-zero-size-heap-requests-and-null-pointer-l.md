+++
schema_version = 1
id = "empty-arrays-010"
key = "other-sites"
area = "empty-arrays"
status = "open"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["empty-arrays-008"]
+++
# Fix the remaining zero-size heap requests and null-pointer library calls

## Outcome

Every site in background/heap-zero-size.md other than the Action and CHB element buffers is fixed, with the evidence its kind requires.

## Context

Follow background/heap-zero-size.md; this task owns every site in it except the Action and CHB element buffers. The known groups are DFA::init (gecode/int/extensional/dfa.cpp), TupleSet(0) (gecode/int/extensional/tuple-set.cpp), IntSet from an empty initializer list, and the Archive copy constructor and assignment (gecode/kernel/archive.cpp:44-53); the audit adds the rest. Fix each at its site without changing documented results. For the evidence, follow the brief's Testing paragraphs: a heap-request site gets a test that fails with the scratch assertion before the fix (reuse an existing test where one reaches it, otherwise add one next to the closest existing test of that type); a null-pointer site gets a UBSan-reached test; a Region site gets a source-trace record. If the record is too large for one reviewable change, split through the derived-work contract by module.

## Boundaries

- If a site needs a user decision (for example a new documented exception), escalate it to the user before changing it. The task does not complete while any assigned site is unfixed, because the heap-contract task relies on every site being fixed.

## Done when

- [ ] Every site the record assigns to this task is fixed with its evidence; none is left open.
- [ ] The tests that reach the fixed heap-request sites pass with the scratch assertion, and the UBSan run of those tests reports nothing for the fixed sites.

## Validation

- The focused tests for each fixed site in build/full with the scratch assertion, and in a GECODE_SANITIZER=undefined build.
- check-standard in build/full.
