+++
schema_version = 1
id = "word-012"
key = "variable-shifts"
area = "word"
status = "open"
blocked_by = ["word-010", "word-008", "word-011"]
+++
# Add shifts by a word variable

## Outcome

A WordVar shift amount can control each supported variable shift while the implementation retains word-level state.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not eagerly channel every amount bit to Boolean variables.
- Do not add variable rotations unless the area brief is amended.

## Done when

- [ ] Variable shifts implement the settled semantics for partial value and amount domains through a staged or word-level decomposition.
- [ ] Amount and value aliases, failure, fixpoint, subsumption, cloning, and recomputation are covered.
- [ ] The implementation records its propagation property and avoids unconditional bit blasting.

## Validation

- Run assigned and partial-domain oracle tests for amounts 0, width minus one, width, and larger.
- Run small-width differential Boolean models and focused lifecycle tests.
