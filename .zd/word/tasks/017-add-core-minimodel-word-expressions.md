+++
schema_version = 1
id = "word-017"
key = "minimodel-core"
area = "word"
status = "done"
blocked_by = ["word-003", "word-005", "word-004", "word-008", "word-011"]
+++
# Add core MiniModel word expressions

## Outcome

MiniModel can construct and lower WordExpr values for logic, relations, reification, mixed Boolean operations, and conditionals through the tested direct posting API.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not add structural or arithmetic expression nodes yet.
- Do not create an implicit full Boolean representation of a word.

## Done when

- [x] WordExpr nodes preserve explicit widths and reject invalid combinations before lowering.
- [x] Logical, relational, reified, mixed Boolean, and conditional expressions lower through the direct posting functions.
- [x] Expression nodes copy safely and follow the settled CamelCase type and snake_case multiword naming rules.

## Validation

- Compare direct-posting and MiniModel widths, propagation outcomes, and solution sets.
- Run node-copy, invalid-width, mixed Boolean, clone, and recomputation tests.

## Result

Added guarded refcounted WordExpr MiniModel nodes for logic, signed/unsigned relations, bit Bool expressions, reification, and Bool/word conditionals with construction-time width checks.

Validation:

- Focused Word::MiniModel and TestFramework tests pass for direct parity, partial propagation, node copy/assignment, invalid widths/index, positive/negated relation and bit atoms, clone, replay, and subsumption; CMake and Make MiniModel-to-Word closure verified; independent verifier PASS.
