+++
schema_version = 1
id = "open-search-038"
key = "fifteen-puzzle-deferred-goal"
area = "open-search"
status = "open"
complexity = "routine"
afk = true
blocked_by = []
+++
# Carry deferred goal checks into the heuristic puzzle example

## Outcome

The open fifteen-puzzle model constructs boards only as needed and detects goals after assignment, while BAB, A*, and CABS keep their existing search configuration.

## Context

The MPG comparison found that imposing the initial Manhattan distance on open path length materializes unassigned boards early. The canonical source at /Users/zayenz/.codex/worktrees/bba6/MPG/rst/examples/src/fifteen-puzzle.cpp now uses OpenPuzzle::wait_goal(i) in the constructor and transition. The feature/open-cp thread is applying this correction to the BAB-only base; wait for its committed update, then refresh this branch onto it and resolve any overlap while preserving this branch's heuristic extension. Read the current example and Gecode wait(IntVarArgs, callback) implementation for callback and cloning behavior.

## Boundaries

- Use the upstream deferred goal correction: wait for board assignment, capture only its index, use the callback's supplied home, then fix length at a goal or require one successor. Keep the fixed model unchanged.
- Preserve BAB as default, both heuristic options/adapters and experimental guards, and the Manhattan total bounds. Remove only the open model's initial Manhattan length constraint.
- Do not adopt the diagnostic NoGPI patch, change kernel metadata allocation/tracing/group control, edit MPG source/results, or run another performance campaign. Add no permanent tests or example assertions.

## Done when

- [ ] The constructor and each materialized transition use clone-safe deferred goal checks, and the open model has no initial Manhattan length constraint.
- [ ] The example still supports both representations with BAB, A*, and CABS, and builds/runs BAB with experimental search disabled.
- [ ] Small solution, already solved, and insufficient-limit cases behave correctly; printed solutions contain legal slides ending at the goal.

## Validation

- Build fifteen-puzzle in the existing experimental ON, OFF, and ASan/UBSan configurations. Check source against the CP correction and unchanged heuristic extension.
- Run small representative open/fixed BAB/A*/CABS cases through exhaustion, including solved starts and insufficient move limits; replay printed paths externally. Check default completion and sanitizer diagnostics.
- Confirm the deferred open model only requires a successor after an assigned non-goal board; use a temporary root inspection if needed, without changing the example's output or adding a permanent fixture.
