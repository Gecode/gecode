+++
schema_version = 1
id = "word-018"
key = "minimodel-structure"
area = "word"
status = "done"
blocked_by = ["word-017", "word-009", "word-010", "word-012"]
+++
# Add structural MiniModel word expressions

## Outcome

MiniModel covers extraction, concatenation, repetition, extension, shifts, and rotations using the direct structural posting API and settled naming conventions.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Use C++ operators only where their fixed-width meaning is unambiguous.
- Use snake_case functions for multiword structural operations.

## Done when

- [x] Structural nodes preserve and validate result widths before lowering.
- [x] Every supported structural operation lowers through the same direct API covered by propagator tests.
- [x] Invalid expression shapes fail with the documented exception rather than truncating or extending silently.

## Validation

- Run direct-posting/MiniModel parity across structural edge cases and result widths.
- Run invalid-expression, node-copy, clone, and recomputation tests.

## Result

Extended WordExpr with validated structural nodes for extract, concat, repeat, extension, shifts, and rotations, lowering through the direct Word API.

Validation:

- Focused Word MiniModel and TestFramework tests pass for all structural forms, edge amounts/result widths, valid partial parity, invalid shapes, copy/assignment, clone, replay, and subsumption; independent verifier PASS.
