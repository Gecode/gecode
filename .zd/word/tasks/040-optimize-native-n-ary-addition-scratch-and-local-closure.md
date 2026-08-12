+++
schema_version = 1
id = "word-040"
key = "optimize-nary-addition-scratch"
area = "word"
status = "open"
blocked_by = ["word-039"]
+++
# Optimize native n-ary addition scratch and local closure

## Outcome

Native n-ary addition preserves its bounded-carry propagation while reducing per-pass allocation, view reads, and intermediate publication.

## Boundaries

- Preserve the public API, one-actor shape, bounded-carry contract, duplicate/result aliases, lifecycle, and honest width-times-arity cost.
- Start with moving Region and scratch allocation out of the local fixpoint loop; retain broader local-mask refactoring only when it remains conventional and measured.
- Do not add advisors, a finer event model, a new testing framework, or durable benchmarks.

## Done when

- [ ] Scratch storage is reused across local fixpoint passes and the actor avoids avoidable repeated view publication, without weakening the existing carry reasoning.
- [ ] Existing n-ary-add assigned, partial soundness, identity, alias, width-64, failure, clone, recomputation, subsumption, arithmetic regression, and TestFramework coverage passes.
- [ ] A temporary Release benchmark/profile covers arity 4 and 8 assigned, partial, and search workloads and justifies every retained production change.

## Validation

- Build the focused Release Word library and gecode-test target.
- Run registered Word NaryAdd, Arithmetic Lifecycle, full Arithmetic, and TestFramework filters.
- Run a temporary exact-baseline Release benchmark/profile and git diff --check.
