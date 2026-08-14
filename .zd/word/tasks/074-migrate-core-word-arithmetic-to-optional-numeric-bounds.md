+++
schema_version = 1
id = "word-074"
key = "bounded-arithmetic"
area = "word"
status = "done"
blocked_by = ["word-073"]
+++
# Migrate core Word arithmetic to optional numeric bounds

## Outcome

Binary add, subtract, negate, multiply, carry/borrow, and overflow exploit non-wrapping signed or unsigned bounds while retaining native modular cube consistency.

## Context

Implement stage four individually and retain each bounded enhancement only when exact workloads demonstrate useful propagation or runtime benefit.

## Boundaries

- Keep actor count and public modular semantics unchanged.
- Do not weaken existing cube propagation or reopen NaryAdd without separate evidence.
- Wrapping cases use local cube fallback rather than unsound interval arithmetic.

## Done when

- [x] Each retained arithmetic actor closes cube and numeric rules locally, groups aliases, and synchronizes each distinct view once.
- [x] Non-wrapping and wrapping boundaries, signed extrema, aliases, carry/borrow/overflow, width 1/64, clone, replay, and subsumption are exact.
- [x] Shared multiplication/order and arithmetic-chain workloads show material search or runtime gains while ALU, Speck, MD5, and cube controls show no material regression.
- [x] Rejected bounded actor variants are removed and documented rather than retained speculatively.

## Validation

- Run full focused Word Arithmetic, lifecycle, MiniModel arithmetic, and TestFramework suites.
- Run exact baseline/candidate arithmetic matrices with counters, checksums, timings, clone distance, and profiles.
- Run zd check word and git diff --check.

## Result

Added selectively routed bounded arithmetic actors for signed/unsigned Add, Sub, Mult, signed Neg, carry/borrow, and overflow, with transactional alias-aware cube-and-range closure and cube fallback for unsupported wrapping regimes.

Validation:

- Release Arithmetic and Overflow suites, MiniModel ArithmeticLifecycle, TestFramework, warning build, 1728 core and 432 flag oracle roots, exact baseline/candidate benchmarks and controls, independent verification, zd check word, and git diff --check all pass.
