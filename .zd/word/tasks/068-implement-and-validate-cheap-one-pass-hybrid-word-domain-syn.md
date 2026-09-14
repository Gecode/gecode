+++
schema_version = 1
id = "word-068"
key = "optimize-hybrid-word-sync"
area = "word"
status = "done"
blocked_by = []
+++
# Implement and validate cheap one-pass hybrid Word-domain synchronization

## Outcome

Produce a production-shaped hybrid domain primitive combining a Word cube with one numeric interval, with synchronization proven correct and optimized before any end-to-end performance measurement.

## Context

Task word-067 investigated a more general dual-interval design and over-weighted the 48-to-80-byte state increase. The revised spike stores one pair of WordValue endpoints and an ordering tag: unsigned order or signed two's-complement rank order using value XOR sign_bit. Propagators batch local cube and bound changes and explicitly synchronize once before publishing. Synchronization canonicalizes the endpoints to admitted cube members, fixes the common endpoint prefix in the cube, and emits one combined modification event.

## Boundaries

- Keep the prototype outside the shared production tree; update only the Word brief with durable conclusions.
- Do not store simultaneous unsigned and signed intervals. A variable has one interval ordering; modular Word operations remain independent of that ordering.
- Do not leave WordVarImp observably dirty across propagator returns, scheduling, cloning, branching, tracing, or status calls.
- Do not use heap allocation, advisors, persistent caches, or per-value enumeration.
- Do not begin end-to-end workload benchmarking until the correctness and synchronization-cost gates pass.

## Done when

- [x] A simple reference implementation and optimized implementation represent cube intersect interval for both unsigned and signed-rank order.
- [x] Synchronization is one-pass and idempotent: canonical endpoints remain admitted after common-prefix cube narrowing, so no local fixpoint loop is needed.
- [x] Masked successor and predecessor use bounded bit operations and carry or borrow logic rather than scanning represented values.
- [x] Exhaustive small-width tests compare the optimized implementation with concrete enumeration across domains, narrowing orders, empty intersections, assignment, signed zero crossing, and repeated synchronization.
- [x] Targeted width-32 and width-64 tests cover sign boundaries, endpoint overflow hazards, and maximal masks.
- [x] Batching several cube and bound updates followed by one synchronization is equivalent to eager synchronization after every update.
- [x] The proposed Word bound modification event and delta distinguish cube changes, bound-only changes, assignment, and failure with one notification.
- [x] Generated-code inspection, operation counters, and focused microbenchmarks quantify no-change, cube-only, bound-only, and mutually refining synchronization paths.
- [x] The Word brief records the selected algorithm, invariants, measured synchronization cost, and any remaining integration risks.

## Validation

- Run exhaustive oracle comparisons and randomized width-32 and width-64 update sequences.
- Run the synchronization microbenchmarks in Release mode with preserved temporary commands and raw output.
- Run zd check word and git diff --check.
- Confirm no production, API, test, example, or build file changed.

## Result

Implemented and independently verified a one-pass cube-and-interval synchronization primitive with unsigned and signed-rank ordering, constant-width bit operations, and explicit batched publication semantics.

Validation:

- Independent enumerating-reference checks passed for 280,782 domains, 1,169,550 exhaustive batched sequences, 800,000 randomized width-32/64 sequences, and seven width-64 boundary cases.
- Seven-trial Release microbenchmarks and ARM64 assembly inspection confirmed bounded no-loop synchronization at roughly 5-8 ns and a 48.9 ns eager versus 8.5 ns batched path with identical checksums.
- Independent Spec and Standards verification passed; zd check word and git diff --check passed with no production/API/test/example/build change.
