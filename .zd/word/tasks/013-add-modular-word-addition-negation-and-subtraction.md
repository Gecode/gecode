+++
schema_version = 1
id = "word-013"
key = "linear-arithmetic"
area = "word"
status = "done"
blocked_by = ["word-005", "word-010", "word-004"]
+++
# Add modular word addition negation and subtraction

## Outcome

Direct posting supports addition, unary negation, and subtraction modulo 2^width using word-level carry state or decomposition.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not add multiplication or division.
- Do not replace the decomposed carry design with a fused adder without measurement.

## Done when

- [x] Addition propagates through the full carry vector, and negation and subtraction preserve wraparound semantics.
- [x] Constants, aliases, failure, fixpoint, subsumption, cloning, and recomputation are covered.
- [x] Intermediate variables remain ordinary searchable Gecode variables without leaking through the public result API.

## Validation

- Run exhaustive assigned and partial-domain oracle tests at small widths.
- Run carry-chain, zero, maximum, wraparound, alias, and lifecycle tests.
- Compare with a Boolean decomposition and record allocation and propagation counters.

## Result

Added modular Word add, neg, and sub posting APIs using Wang-style carry decomposition over ordinary hidden WordVar intermediates.

Validation:

- Focused Word::Arithmetic and Word::TestFramework tests pass for assigned/partial domains, carry/wraparound, constants, aliases, failure, cloning, Boolean full-adder parity, counters, replay recomputation, and subsumption; independent verifier PASS.
