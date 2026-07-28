+++
schema_version = 1
id = "parallel-search-worker-control-008"
proposal_key = "validate-performance-semantics"
proposal_hash = "sha256:fe3b85209ea22917059887563e54603a796b8c6e81d7ba8d2d7962ef59be4477"
area = "parallel-search-worker-control"
kind = "implementation"
interaction = "hitl"
priority = "normal"
status = "done"
blocked_by = ["parallel-search-worker-control-007"]
+++
# Benchmark overhead and portfolio reallocation

## Source

The area done condition requires negligible unchanged-limit overhead and evidence that dynamic limits are useful for portfolio resource allocation.

## What to build

Benchmark the unchanged-limit fast path, resize response latency, resident parked-thread cost, and a representative portfolio that reallocates a fixed active-worker budget among assets. Publish the results, document user-visible semantics and caveats, and obtain a readiness decision against an explicit overhead threshold.

## Non-goals

- Claiming universal portfolio speedup.
- Designing a production allocation heuristic.

## Acceptance criteria

- [x] Benchmarks compare baseline parallel DFS and BAB against controls fixed at capacity.
- [x] Resize response latency is measured for cheap-node and expensive-propagation workloads.
- [x] A portfolio experiment demonstrates bounded active concurrency while reallocating workers among assets.
- [x] Documentation explains fixed maximum capacity, parked threads, asynchronous convergence, the 1..maximum range, and nondeterministic search order.
- [x] A human review records whether measured overhead and behavior are acceptable for release.

## Validation

- Run the recorded benchmark commands with repeated samples and retain summarized results.
- Review the published semantics and benchmark threshold before accepting the area.

## Reviewed decision

The user approved the feature for Gecode 7 release on 2026-07-24 after
reviewing the retained `worker-control-release-20260724-final` results and the
documented cooperative semantics.

The approved release gate is a median paired fixed-capacity overhead no greater
than 3% and a deterministic-bootstrap 95% confidence-interval upper bound no
greater than 5%, separately for DFS and BAB. The retained run passed:

- DFS: -0.896% median, 95% CI [-3.873%, 3.885%].
- BAB: -0.045% median, 95% CI [-0.224%, 0.129%].

The review also accepts the measured cooperative behavior: grow p95 below
20 microseconds in both workloads, shrink p95 approximately 7.5 milliseconds,
a median 368 KiB process peak-RSS delta for three parked resident workers, and
30 of 30 PBS trials respecting a fixed four-worker active budget.

This approval does not establish a universal resize-latency or memory limit,
prescribe a PBS allocation policy, or claim universal portfolio speedup.
