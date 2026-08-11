+++
schema_version = 1
id = "word-014"
key = "multiply"
area = "word"
status = "done"
blocked_by = ["word-013", "word-010", "word-011"]
+++
# Add modular word multiplication

## Outcome

Direct posting supports multiplication modulo 2^width through conditional shifted terms or a measured stronger word-level alternative.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not add division or unconditional per-bit Boolean expansion.

## Done when

- [x] Multiplication implements wraparound semantics for assigned and partial domains.
- [x] Zero, one, maximum values, powers of two, constants, aliases, failure, cloning, and recomputation are covered.
- [x] Any implementation stronger than the approved decomposition is supported by measurements and keeps word-level state.

## Validation

- Run exhaustive assigned oracle and partial-domain soundness tests at small widths.
- Run Boolean parity, lifecycle, allocation, and propagation guardrail tests.

## Result

Added modular Word multiplication through an all-Word schoolbook decomposition of conditional shifted terms folded with modular addition.

Validation:

- Focused multiplication and Word test-framework tests pass for assigned/partial domains, width one, overflow, constants, aliases, failure, cloning, independent Boolean parity, counters, replay recomputation, and subsumption; independent verifier PASS.
