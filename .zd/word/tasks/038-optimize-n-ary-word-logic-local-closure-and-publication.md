+++
schema_version = 1
id = "word-038"
key = "optimize-nary-logic-publication"
area = "word"
status = "done"
blocked_by = []
+++
# Optimize n-ary word logic local closure and publication

## Outcome

Native n-ary AND, OR, and XOR reach the same fixpoint while avoiding unchanged and intermediate WordView publications.

## Boundaries

- Keep the public API, normalization, actor count, propagation strength, aliases, and lifecycle unchanged.
- Do not change binary Logic::Table or add advisors, finer events, a new test harness, or durable benchmark infrastructure.
- Use existing test/word coverage; add a normal focused regression only if the changed path is not already exercised.

## Done when

- [x] N-ary logic computes its local fixpoint over temporary masks and publishes only actually changed views once, or a measured investigation shows that an alternative smaller change is preferable.
- [x] All existing Word Logic, Conditional, MiniModel logic, and TestFramework tests pass with unchanged semantics, aliases, cloning, recomputation, and subsumption.
- [x] A temporary Release benchmark and sample profile compare the exact baseline and candidate for assigned, partial, and search workloads across AND, OR, and XOR; the retained implementation has a useful repeatable result.

## Validation

- Build the focused Release Word library and gecode-test target.
- Run the registered Word Logic, Conditional, MiniModel logic, and TestFramework filters.
- Run a temporary exact-baseline Release benchmark/profile and git diff --check.

## Result

Optimized native n-ary AND/OR local mask closure and changed-only publication, with a smaller changed-only XOR path, preserving propagation and lifecycle.

Validation:

- Focused Release Word Logic, Conditional, MiniModel logic, and TestFramework tests passed.
- Exact baseline/candidate benchmarks preserved semantic and search parity and measured repeatable gains up to 21 percent.
- Independent verification and git diff --check passed.
