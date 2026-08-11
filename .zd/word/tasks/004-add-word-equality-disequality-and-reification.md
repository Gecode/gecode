+++
schema_version = 1
id = "word-004"
key = "relations"
area = "word"
status = "open"
blocked_by = ["word-002"]
+++
# Add word equality disequality and reification

## Outcome

Direct posting supports WordVar equality and disequality with variables, explicit-width constants, and every applicable Gecode reification mode.

## Boundaries

- Do not add signed or unsigned ordering relations.

## Done when

- [ ] Equality and disequality propagators or strength-preserving rewrites implement the settled lo/hi semantics and reject width mismatches.
- [ ] All reification modes, aliases, constants, failure, subsumption, cloning, and recomputation are covered.
- [ ] Tests record the direct/rewrite/decomposition shape and the consistency property they enforce.

## Validation

- Run assigned-value oracle and exhaustive partial-domain soundness tests through the shared WordVar framework.
- Run reification truth-table, width-mismatch, alias, clone, and recomputation tests.
