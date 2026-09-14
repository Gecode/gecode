+++
schema_version = 1
id = "word-045"
key = "optimize-binary-add"
area = "word"
status = "done"
blocked_by = ["word-044"]
+++
# Optimize binary Word addition propagation

## Outcome

The native binary Word Add actor performs less repeated transition, support, and kernel-publication work while retaining its exact bit-consistency contract.

## Boundaries

- Keep the public APIs, one-actor shape, propagation result, aliases, subscriptions, cost class, and lifecycle unchanged.
- Prefer a small local-mask, cached-transition, or changed-only-publication refactor over advisors, new events, generated tables, or a second actor.
- Keep benchmark drivers and profiles temporary.

## Done when

- [x] The Add actor avoids the measured redundant work and preserves exact support projection for every retained bit value.
- [x] Normal test/word Add and Arithmetic lifecycle coverage passes, with a compact standard regression added only if the changed path was not already exercised.
- [x] A focused exact-baseline Release comparison on assigned, partial, inverse-search, and ARX workloads records semantic parity and a useful repeatable performance tradeoff.

## Validation

- Build GecodeWord and gecode-test in Release mode and run focused Add, Arithmetic Lifecycle, and Word TestFramework filters.
- Run temporary exact-baseline Add and ARX benchmarks and sampling profiles, recording actors, propagations, search statistics, wall time, and best-effort RSS.
- Run git diff --check.

## Result

Optimized and independently verified binary Word Add tuple filtering and changed-only publication without changing bit consistency.

Validation:

- Release Add, Arithmetic Lifecycle, and Word TestFramework filters passed.
- Exact-baseline assigned, partial, inverse, and ARX benchmarks preserved semantic/search counters and improved median runtime by roughly 7-13%.
- zdev checks and git diff --check passed.
