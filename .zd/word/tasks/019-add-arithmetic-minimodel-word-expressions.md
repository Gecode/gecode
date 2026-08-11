+++
schema_version = 1
id = "word-019"
key = "minimodel-arithmetic"
area = "word"
status = "open"
blocked_by = ["word-017", "word-013", "word-014", "word-016"]
+++
# Add arithmetic MiniModel word expressions

## Outcome

MiniModel covers word addition, negation, subtraction, multiplication, division, remainder, and modulus and preserves WordSemantics through expression nodes and lowering.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not add a second WordSemantics policy.

## Done when

- [ ] Arithmetic expression nodes preserve widths and the selected WordSemantics policy through composed expressions.
- [ ] Every supported arithmetic expression lowers through the corresponding direct posting API.
- [ ] Invalid width combinations and unsupported policy values fail before posting.

## Validation

- Run exhaustive assigned small-width direct-posting/MiniModel parity tests.
- Run policy-retention, composed-expression, invalid-width, node-copy, clone, and recomputation tests.
