+++
schema_version = 1
id = "word-004"
key = "relations"
area = "word"
status = "done"
blocked_by = ["word-002"]
+++
# Add word equality disequality and reification

## Outcome

Direct posting supports WordVar equality and disequality with variables, explicit-width constants, and every applicable Gecode reification mode.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not add signed or unsigned ordering relations.

## Done when

- [x] Equality and disequality propagators or strength-preserving rewrites implement the settled lo/hi semantics and reject width mismatches.
- [x] All reification modes, aliases, constants, failure, subsumption, cloning, and recomputation are covered.
- [x] Tests record the direct/rewrite/decomposition shape and the consistency property they enforce.

## Validation

- Run assigned-value oracle and exhaustive partial-domain soundness tests through the shared WordVar framework.
- Run reification truth-table, width-mismatch, alias, clone, and recomputation tests.

## Result

Added direct WordVar equality and disequality with explicit-width constants and all Gecode reification modes using conventional word-level propagators and rewrites.

Validation:

- Independent verification passed source and actor-lifecycle review; focused Word::Rel and Word::TestFramework tests passed; shared reification truth tables, aliases, partial-domain soundness, cloning, width mismatches, and git diff checks passed.
