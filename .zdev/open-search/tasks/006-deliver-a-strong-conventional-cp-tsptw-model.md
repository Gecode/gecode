+++
schema_version = 1
id = "open-search-006"
key = "tsptw-closed-cp"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-005"]
+++
# Deliver a strong conventional CP TSPTW model

## Outcome

An ordinary native Space solves canonical TSPTW with costed-circuit and time propagation, provides valid improving solutions, and serves as a serious CP baseline.

## Context

Read the brief's TSPTW conventions and ../background/next-search-and-benchmarks.md closed model discussion. examples/tsp.cpp already posts a costed circuit and regret-based branching; its reversal symmetry break cannot be copied for asymmetric/windowed tours. Reuse the input/checker and existing native example/build patterns.

## Boundaries

- Use existing Gecode globals, propagation and BAB; keep Model optional and add no new routing global.
- Do not post time precedence around a cyclic single depot time: separate start and return treatment. Avoid redundant branching over arbitrary waiting schedules.

## Done when

- [x] The native executable reads the shared data, enforces one complete tour and all windows/return, minimizes the supplied arc sum, and emits checker-compatible tours and honest proof/stop status.
- [x] Costed-circuit propagation, sound unavailable-arc filtering and customer timing are active; branching is explicit and a feasible seed is independently checked before use.
- [x] BAB and existing native A*/CABS run on the same Space with propagated total-objective guidance; engine comparisons preserve formulation and branching.
- [x] Tiny exhaustive cases and checked canonical cases agree on optimum/infeasibility, including asymmetric tours that the inherited reversal restriction would remove.

## Validation

- Build/run the new native target with BAB/A*/CABS on the correctness set and independently check every returned tour.
- Run existing native search/portfolio lifecycle selections and the repository check target for touched interfaces; inspect linkage to confirm no Model requirement.
- Use existing semantic test seams; do not add an options matrix.

## Result

Added a native costed-circuit TSPTW baseline with timing propagation, checked seeds, and BAB/A*/CABS comparisons.

Validation:

- Independent whole-task review passed on unchanged snapshot W96474f0503f57f45.
- Release build, reader/checker validator, five native lifecycle/search selections, and repository check target passed.
- All 12 canonical engine runs proved matching objectives; Model-disabled ASan tiny, seed, and stop journeys passed; linkage has no Model dependency.
