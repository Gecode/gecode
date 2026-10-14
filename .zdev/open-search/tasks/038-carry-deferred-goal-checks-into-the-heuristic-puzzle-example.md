+++
schema_version = 1
id = "open-search-038"
key = "fifteen-puzzle-deferred-goal"
area = "open-search"
status = "done"
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

- [x] The constructor and each materialized transition use clone-safe deferred goal checks, and the open model has no initial Manhattan length constraint.
- [x] The example still supports both representations with BAB, A*, and CABS, and builds/runs BAB with experimental search disabled.
- [x] Small solution, already solved, and insufficient-limit cases behave correctly; printed solutions contain legal slides ending at the goal.

## Validation

- Build fifteen-puzzle in the existing experimental ON, OFF, and ASan/UBSan configurations. Check source against the CP correction and unchanged heuristic extension.
- Run small representative open/fixed BAB/A*/CABS cases through exhaustion, including solved starts and insufficient move limits; replay printed paths externally. Check default completion and sanitizer diagnostics.
- Confirm the deferred open model only requires a successor after an assigned non-goal board; use a temporary root inspection if needed, without changing the example's output or adding a permanent fixture.

## Result

Carried CP deferred-goal correction 81fff38e8e into the heuristic branch by managed rebase. Constructor and transition wait for assigned boards with index-only callbacks using the supplied space. Fixed constraints and heuristic configuration are preserved. Independent verification passed without required findings.

Validation:

- Built fifteen-puzzle in Release experimental ON and OFF and Debug ASan/UBSan; all passed. Selected clang-tidy core/C++ analyzers and git diff --check passed.
- Independent verifier ran 42 small cases through exhaustion across available engines and both representations, including solved and insufficient-limit inputs. Replayed every printed path; no sanitizer diagnostics.
- Temporary Release inspector at scramble=20, limit=32, seed=3 confirmed 32 tile variables, two path positions, depth 0 and bound 16; the stable clone matched. The solved start with limit 0 had one board and bound 0; a non-goal start with limit 0 failed.
- Known usability regression: open BAB default reached a 10-second Release cutoff without a solution; fixed default finished at 12 moves in 38 ms. Upstream observed the open default exceeding 30 seconds in Debug. Default usability remains a separate follow-up.
- Compared against the saved branch tip and CP correction: only 18 lines in fifteen-puzzle source changed; fixed model, bounds, experimental guards and adapters preserved. Kernel unchanged; NoGPI excluded. Snapshot W5b38b585ce5fa57b remained equal.
