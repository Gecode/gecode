+++
schema_version = 1
id = "word-013"
key = "linear-arithmetic"
area = "word"
status = "open"
blocked_by = ["word-005", "word-010", "word-004"]
+++
# Add modular word addition negation and subtraction

## Outcome

Direct posting supports addition, unary negation, and subtraction modulo 2^width using word-level carry state or decomposition.

## Boundaries

- Do not add multiplication or division.
- Do not replace the decomposed carry design with a fused adder without measurement.

## Done when

- [ ] Addition propagates through the full carry vector, and negation and subtraction preserve wraparound semantics.
- [ ] Constants, aliases, failure, fixpoint, subsumption, cloning, and recomputation are covered.
- [ ] Intermediate variables remain ordinary searchable Gecode variables without leaking through the public result API.

## Validation

- Run exhaustive assigned and partial-domain oracle tests at small widths.
- Run carry-chain, zero, maximum, wraparound, alias, and lifecycle tests.
- Compare with a Boolean decomposition and record allocation and propagation counters.
