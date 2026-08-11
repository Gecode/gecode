+++
schema_version = 1
id = "word-009"
key = "fixed-structure"
area = "word"
status = "open"
blocked_by = ["word-002"]
+++
# Add fixed structural word operations

## Outcome

Direct posting supports extraction, concatenation, repetition, zero extension, and sign extension with explicit and checked result widths.

## Boundaries

- Do not add shifts or rotations in this task.

## Done when

- [ ] Masked equality-style propagators implement every fixed structural operation without per-bit Boolean expansion.
- [ ] Width-changing contracts, invalid dimensions, constants, aliases, failure, subsumption, cloning, and recomputation are covered.
- [ ] Tests state and verify the claimed propagation property of each operation.

## Validation

- Run assigned-value oracle and partial-domain soundness tests through the shared WordVar framework.
- Run first-bit, last-bit, full-width slice, repetition, sign-bit, invalid-dimension, alias, and clone tests.
