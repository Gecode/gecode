+++
schema_version = 1
id = "word-010"
key = "fixed-shifts"
area = "word"
status = "done"
blocked_by = ["word-002"]
+++
# Add constant word shifts and rotations

## Outcome

Direct posting supports constant logical and arithmetic shifts and constant left and right rotations with fixed-width semantics.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not add word-valued shift amounts or variable rotations.

## Done when

- [x] Constant shift and rotation propagators implement masked word-level updates and settled out-of-range semantics.
- [x] Constants, aliases, failure, subsumption, cloning, recomputation, and widths 1 and 64 are covered.
- [x] Tests state and verify the claimed propagation property.

## Validation

- Run assigned-value oracle and partial-domain tests through the shared WordVar framework.
- Run shift and rotation amounts 0, width minus one, width, and greater than width, plus alias and lifecycle tests.

## Result

Added constant logical left/right shifts, arithmetic right shift, and left/right rotations with fixed-width SMT-LIB shift semantics and modulo rotations.

Validation:

- Independent verification passed focused Word::Structure::Shift and Word::TestFramework implementation tests plus forward/backward mask, out-of-range, width 1/64, constant, alias, subsumption, clone, naming, and git diff review.
