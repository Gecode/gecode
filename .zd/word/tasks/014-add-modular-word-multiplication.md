+++
schema_version = 1
id = "word-014"
key = "multiply"
area = "word"
status = "open"
blocked_by = ["word-013", "word-010", "word-011"]
+++
# Add modular word multiplication

## Outcome

Direct posting supports multiplication modulo 2^width through conditional shifted terms or a measured stronger word-level alternative.

## Boundaries

- Do not add division or unconditional per-bit Boolean expansion.

## Done when

- [ ] Multiplication implements wraparound semantics for assigned and partial domains.
- [ ] Zero, one, maximum values, powers of two, constants, aliases, failure, cloning, and recomputation are covered.
- [ ] Any implementation stronger than the approved decomposition is supported by measurements and keeps word-level state.

## Validation

- Run exhaustive assigned oracle and partial-domain soundness tests at small widths.
- Run Boolean parity, lifecycle, allocation, and propagation guardrail tests.
