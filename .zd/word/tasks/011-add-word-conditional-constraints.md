+++
schema_version = 1
id = "word-011"
key = "conditional"
area = "word"
status = "open"
blocked_by = ["word-005", "word-004"]
+++
# Add word conditional constraints

## Outcome

Direct posting supports the settled Boolean-controlled and word-mask-controlled if-then-else operations without exposing bit arrays.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Use a word-level direct actor or word-level decomposition; do not add MiniModel syntax yet.

## Done when

- [ ] Conditionals propagate among controls, branches, and results for fixed and partial domains.
- [ ] Equal branches, aliases, constants, failure, subsumption, cloning, and recomputation are covered.
- [ ] The implementation records its direct or decomposed shape and claimed propagation property.

## Validation

- Run exhaustive assigned and partial-domain oracle tests through the shared WordVar framework.
- Run fixed/unknown control, equal-branch, alias, failure, clone, and recomputation tests.
