+++
schema_version = 1
id = "word-012"
key = "variable-shifts"
area = "word"
status = "done"
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

- [x] Variable shifts implement the settled semantics for partial value and amount domains through a staged or word-level decomposition.
- [x] Amount and value aliases, failure, fixpoint, subsumption, cloning, and recomputation are covered.
- [x] The implementation records its propagation property and avoids unconditional bit blasting.

## Validation

- Run assigned and partial-domain oracle tests for amounts 0, width minus one, width, and larger.
- Run small-width differential Boolean models and focused lifecycle tests.

## Result

Implemented direct word-level variable shifts with SMT-LIB overshift semantics, bounded cube-hull propagation, fixed-amount rewrite, alias proxies, and honest linear cost.

Validation:

- Focused VariableShift and TestFramework tests pass, including independent width-3 Boolean parity, partial domains, failure, aliases, subsumption, cloning, and replay-based recomputation; independent verifier PASS.
