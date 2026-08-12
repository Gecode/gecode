+++
schema_version = 1
id = "word-039"
key = "optimize-native-multiplication-closure"
area = "word"
status = "done"
blocked_by = ["word-038"]
+++
# Optimize native word multiplication local closure

## Outcome

Native multiplication reaches its existing prefix and inverse propagation fixpoint with less repeated view publication and rescanning.

## Boundaries

- Preserve the public API, one-actor shape, declared propagation contract, aliases, and lifecycle.
- Do not replace multiplication with a decomposition, strengthen its contract opportunistically, or add durable benchmark machinery.
- Use existing test/word arithmetic coverage; add only a normal focused regression for an otherwise uncovered optimized path.

## Done when

- [x] Multiplication performs prefix, forced-zero, and inverse reasoning over local masks to closure and publishes changed views once, or a measured smaller refactor removes the identified repeated work.
- [x] Existing assigned, partial, inverse, alias, clone, recomputation, subsumption, MiniModel, and TestFramework coverage passes with the same propagation results.
- [x] A temporary Release baseline/candidate benchmark and sample profile show whether the retained change usefully reduces wall time or narrowing work without semantic or search regression.

## Validation

- Build the focused Release Word library and gecode-test target.
- Run registered Word Arithmetic multiplication/lifecycle, MiniModel arithmetic, and TestFramework filters.
- Run a temporary exact-baseline Release benchmark/profile and git diff --check.

## Result

Optimized native multiplication to reach its existing prefix and inverse closure over local masks and publish changed views once.

Validation:

- Focused Release multiplication, arithmetic lifecycle, MiniModel arithmetic, and TestFramework tests passed.
- Exact baseline benchmarks preserved all semantic and search counters and improved assigned, partial, and inverse workloads by 4 to 8 percent.
- Independent verification and git diff --check passed.
