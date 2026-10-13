+++
schema_version = 1
id = "open-search-005"
key = "tsptw-data-checker"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = []
+++
# Establish exact TSPTW data and independent checking

## Outcome

A small canonical TSPTW corpus, reusable native input reader, and independent checker establish identical semantics for every formulation and reference solver.

## Context

Read the brief's Next bundle data/measurement rules and ../background/next-search-and-benchmarks.md TSPTW data and comparison. Start from examples/tsp.cpp, misc/ benchmark conventions, the publisher's canonical matrices and check_solution.cpp. Use a small examples/data/tsptw correctness set and a simple acquisition/selection manifest for larger cases; exact paths are implementation choices.

## Boundaries

- Preserve canonical matrix values by exact scaling; no coordinate reconstruction, hidden rounding, double service, makespan objective, or new data-management framework.
- Use fixed departure zero and return-depot window; reject unsupported start-ready or integer-limit inputs explicitly. Tiny generated cases are separate from canonical families.

## Done when

- [x] The reader validates dimensions, nonnegative arcs, windows, exact decimal scaling and Gecode arithmetic ranges; canonical travel-cost and service-folded conventions are documented.
- [x] A standalone checker recomputes every visit, waiting, return, coverage and objective independently of model/search internals, accepts a valid tour and rejects a duplicate, omission, late customer, late return and incorrect cost.
- [x] Tiny enumerated optima/infeasibility cover waiting, asymmetric/nonmetric travel, decimal input and folded service. Published tours are checked under the same convention; best-known values are distinguished from proof.
- [x] A reproducible manifest selects a tiny correctness set, 12 development and 24 disjoint held-out cases across Dumas, GendreauDumasExtended, SolomonPotvinBengio, AFG and marked window variants before any tuning, with source URLs and content hashes. Larger acquisition is explicit; routine tests run offline.

## Validation

- Run the reader/checker on tiny positive and negative cases and independently enumerate tiny tours.
- Acquire the selected public data and verify at least one published tour per selected canonical family against the publisher's conventions. Check the manifest's disjoint split and integer limits.
- No broad new test harness; focused parser/checker behavior and CLI smoke checks are sufficient.

## Result

Established exact native TSPTW reading, independent checking, offline tiny cases and frozen development/held-out data.

Validation:

- C++ reader build; offline 9-case and full 45-case validation passed with exact parser agreement and input/tour rejection checks.
- Acquisition/hash validation passed for 36 selected cases; independent enumeration confirmed five tiny results and publisher checker accepted four canonical-family tours.
- Fresh independent whole-task verifier passed against unchanged W598a61b451c35a74, including the leading-dot zero regression.
