+++
schema_version = 1
id = "word-073"
key = "bounded-relations"
area = "word"
status = "done"
blocked_by = ["word-072"]
+++
# Migrate Word relations to optional numeric bounds

## Outcome

Equality and unsigned/signed order relations, including reification, exploit matching bounded domains and conventionally fall back to cube propagation otherwise.

## Context

Implement stage three of the word-070 operator matrix using typed Word views and ordinary Gecode actor lifecycle.

## Boundaries

- Do not inspect other propagators or add global order-cycle analysis.
- Mixed, cube, or opposite-kind operands retain the current cube actor for the complete relation.

## Done when

- [x] Direct and reified equality/order dispatch is correct for homogeneous unsigned, homogeneous signed, constants, aliases, and results.
- [x] All relation and reification modes preserve exact semantics under cube and mixed-kind fallback.
- [x] Transactional actors publish each distinct variable once and use the weakest sound Word propagation condition.
- [x] Ordinary relation lifecycle, exhaustive small-width semantics, width 64, clone, replay, failure, and subsumption tests pass.

## Validation

- Run full focused Word Rel and TestFramework suites.
- Compare bounded order behavior with IntVar inside safe limits and preserve the degenerative cycle microbenchmark.
- Run zd check word and git diff --check.

## Result

Added native bounded equality and unsigned/signed ordering with reification, constants, and complete cube/mixed fallback.

Validation:

- Release Word Rel x10 passed
- Full Word suite passed
- Bounded cycle and Int comparison recorded
- Independent Spec and Standards verification PASS
- zd check word and git diff --check passed
