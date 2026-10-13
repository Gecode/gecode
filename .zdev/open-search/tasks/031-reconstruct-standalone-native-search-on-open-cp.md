+++
schema_version = 1
id = "open-search-031"
key = "split-native-branch"
area = "open-search"
status = "open"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = []
+++
# Reconstruct standalone native search on open-cp

## Outcome

feature/open-search contains the implemented native heuristic search and benchmarks on feature/open-cp without Model or portfolio dependencies.

## Context

Read the approved branch split section of brief.md. The backup branch preserves the complete implementation. Extract native AStar, configurable best-first ordering, CABS and Column engines, guidance, optional state pruning, native examples and TSPTW tooling, and only necessary generic kernel/search contracts. Preserve newer open-cp worker-control and open-list fixes. Existing task records describe completed combined-branch work.

## Boundaries

- Do not modify backup or parent branches, Model or portfolio implementations, or zdev records as a source worker.
- Preserve implemented native behavior and orthogonal ranking, bounds and state pruning; no new engine or performance campaign.
- Use existing tests and build systems; adapt focused semantic coverage only where extraction changes contracts.

## Done when

- [ ] Native AStar, configurable best-first ordering, CABS and Column engines, plus native examples/benchmarks, build and run on the open-cp base.
- [ ] Model and portfolio source, build targets and dependencies are absent from native search additions.
- [ ] Minimal generic prerequisite contracts preserve ordinary Space use and newer base behavior.

## Validation

- Fresh Release build and full existing check suite.
- Focused search, open-list, regular, comparison/continuation/worker-control checks as applicable; native examples and tiny TSPTW correctness.
- Inspect extracted diff against the backup and open-cp base for omitted behavior or unrelated dependencies.
