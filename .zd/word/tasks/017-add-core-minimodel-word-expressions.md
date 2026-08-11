+++
schema_version = 1
id = "word-017"
key = "minimodel-core"
area = "word"
status = "open"
blocked_by = ["word-003", "word-005", "word-004", "word-008", "word-011"]
+++
# Add core MiniModel word expressions

## Outcome

MiniModel can construct and lower WordExpr values for logic, relations, reification, mixed Boolean operations, and conditionals through the tested direct posting API.

## Boundaries

- Do not add structural or arithmetic expression nodes yet.
- Do not create an implicit full Boolean representation of a word.

## Done when

- [ ] WordExpr nodes preserve explicit widths and reject invalid combinations before lowering.
- [ ] Logical, relational, reified, mixed Boolean, and conditional expressions lower through the direct posting functions.
- [ ] Expression nodes copy safely and follow the settled CamelCase type and snake_case multiword naming rules.

## Validation

- Compare direct-posting and MiniModel widths, propagation outcomes, and solution sets.
- Run node-copy, invalid-width, mixed Boolean, clone, and recomputation tests.
