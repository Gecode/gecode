+++
schema_version = 1
id = "open-search-039"
key = "fifteen-puzzle-assigned-pruning"
area = "open-search"
status = "done"
complexity = "routine"
afk = true
blocked_by = []
+++
# Restore pruning in the deferred open puzzle model

## Outcome

The open puzzle rejects assigned boards that cannot reach the goal within the move cap and constrains an already materialized final board to the goal, restoring practical BAB defaults without eager future boards.

## Context

Task038 carried deferred goal checks forward, but the open BAB default exceeded a 10-second Release cutoff. The current canonical source /Users/zayenz/.codex/worktrees/bba6/MPG/rst/examples/src/fifteen-puzzle.cpp adds check_bound, final_goal and hooks in wait_goal/constrain. Its SHA256 is 1f0905c45f97addcfef060806b959063cac831ea45bd79410d76c0e3d0e12a92. Independent source review found no required corrections, including clone safety, immediate upper-bound updates and BAB replay paths. The CP thread is adapting and validating this ordinary correction first; wait for its commit, refresh this branch onto it, then verify the heuristic extension. The serial measurement run has completed, so focused builds may resume.

## Boundaries

- Prune only assigned boards with the admissible depth-plus-Manhattan bound against the current maximum moves. Preserve deferred goal checks and avoid imposing a Manhattan minimum on open-list length.
- Force the cap board to goal when first materialized or when an incumbent cut makes an already materialized board the cap. Preserve index-only callbacks using their supplied space, fixed-model constraints, BAB default, A*/CABS adapters and experimental guards.
- Keep kernel metadata, NoGPI and compact/rolling-board variants outside this task. Do not modify MPG source/results or run another performance campaign. Add no permanent tests or embedded example assertions.

## Done when

- [x] The current ordinary CP correction is incorporated, including assigned-board pruning and both cap-goal hooks, with the heuristic extension preserved.
- [x] The root for scramble20/limit32/seed3 still constructs two boards; goal and insufficient-limit cases remain correct across available engines and representations.
- [x] The open BAB default completes within the prior 10-second Release check, and the example builds/runs with experimental search enabled and disabled.

## Validation

- Build fifteen-puzzle in existing Release experimental ON/OFF and Debug ASan/UBSan configurations; inspect the actual CP delta and unchanged native/kernel/fixed-model code.
- Run small open/fixed BAB/A*/CABS cases through exhaustion, including improving incumbents, solved starts and insufficient limits; replay printed paths externally and check sanitizer diagnostics. Check both Release defaults with a 10-second cutoff.
- Use temporary root/clone inspection to confirm two boards at scramble=20, limit=32, seed=3 and cap-goal behavior after an incumbent cut on an already materialized successor. Keep inspection outside the example and do not create a permanent fixture.

## Result

Carried CP pruning correction fc26a2a8fc into the heuristic branch. Assigned-board budget checks and both cap-goal hooks restore the open BAB default while preserving deferred construction, the fixed model and A*/CABS configuration. Resolved the rebase overlap with one shared depth query and the existing heuristic bound. Independent verification passed without findings.

Validation:

- Release experimental ON/OFF and Debug ASan/UBSan fifteen-puzzle builds passed. Selected clang-analyzer checks passed with actual Release flags; established Gecode formatting preserved because no project formatter configuration exists.
- Independent verifier ran 44 Release and 30 sanitized cases through exhaustion across both representations and available engines. Printed paths replayed legally; incumbents strictly improved; solved and insufficient-limit cases passed. Sanitizer stderr was empty.
- ON/OFF open defaults returned 12 moves in 9–10 ms under a 10-second cutoff; fixed defaults completed in 46–47 ms. This resolves the previous default slowdown on the ordinary kernel.
- Sanitized root/clone probe at scramble=20, limit=32, seed=3 confirmed two boards and bound 16; clone growth left the root unchanged. First-cap materialization solved; a real three-move incumbent forced an already materialized cap before propagation. Pending-board and assigned-blank-prefix cases correctly failed the impossible two-move restriction.
- The source delta is confined to 39 lines in the example; fixed/shared rules, heuristic adapters, bounds and guards, and kernel are unchanged. NoGPI and compact experiments are excluded. Snapshot Wfb14909dd8e16dad remained equal; git diff whitespace checks passed.
