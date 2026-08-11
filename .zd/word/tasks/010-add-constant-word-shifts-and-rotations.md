+++
schema_version = 1
id = "word-010"
key = "fixed-shifts"
area = "word"
status = "open"
blocked_by = ["word-002"]
+++
# Add constant word shifts and rotations

## Outcome

Direct posting supports constant logical and arithmetic shifts and constant left and right rotations with fixed-width semantics.

## Boundaries

- Do not add word-valued shift amounts or variable rotations.

## Done when

- [ ] Constant shift and rotation propagators implement masked word-level updates and settled out-of-range semantics.
- [ ] Constants, aliases, failure, subsumption, cloning, recomputation, and widths 1 and 64 are covered.
- [ ] Tests state and verify the claimed propagation property.

## Validation

- Run assigned-value oracle and partial-domain tests through the shared WordVar framework.
- Run shift and rotation amounts 0, width minus one, width, and greater than width, plus alias and lifecycle tests.
