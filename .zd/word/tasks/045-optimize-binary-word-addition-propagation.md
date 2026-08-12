+++
schema_version = 1
id = "word-045"
key = "optimize-binary-add"
area = "word"
status = "open"
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

- [ ] The Add actor avoids the measured redundant work and preserves exact support projection for every retained bit value.
- [ ] Normal test/word Add and Arithmetic lifecycle coverage passes, with a compact standard regression added only if the changed path was not already exercised.
- [ ] A focused exact-baseline Release comparison on assigned, partial, inverse-search, and ARX workloads records semantic parity and a useful repeatable performance tradeoff.

## Validation

- Build GecodeWord and gecode-test in Release mode and run focused Add, Arithmetic Lifecycle, and Word TestFramework filters.
- Run temporary exact-baseline Add and ARX benchmarks and sampling profiles, recording actors, propagations, search statistics, wall time, and best-effort RSS.
- Run git diff --check.
