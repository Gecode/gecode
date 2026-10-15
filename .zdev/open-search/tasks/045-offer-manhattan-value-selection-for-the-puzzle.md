+++
schema_version = 1
id = "open-search-045"
key = "puzzle-manhattan-values"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
blocked_by = []
+++
# Offer Manhattan value selection for the puzzle

## Outcome

Both puzzle models offer optional Manhattan move ordering, with measured search-statistic comparisons.

## Context

The user asks what the fixed-model branch filter does and whether Manhattan value selection changes statistics. Use ordinary INT_VAL callbacks, keep numerical minimum as the default, and keep value ordering separate from A*/CABS guidance. Apply the common example changes on feature/open-cp before refreshing feature/open-search.

## Boundaries

- Keep this example simple; no custom search engine or brancher, new permanent tests, or local sanitizer work.
- Preserve parent uncommitted neighbor-reversion and open-route edits. Coordinator owns parent selective integration, stack refresh, and publication.

## Done when

- [x] Both open and fixed models expose -branching min and -branching manhattan; the latter chooses a legal next blank position minimizing resulting board Manhattan distance, with deterministic ties.
- [x] Explain the fixed-model filter and compare node/failure/propagation statistics for both orders on identical bounded cases; all reported incumbents replay to the goal and final optima agree.
- [x] Normal parent and child builds and focused checks pass; parent source commit is incorporated into the child and user edits are preserved.

## Validation

- Build the parent and child fifteen-puzzle targets normally. Use bounded BFS/path-replay checks for both branching options and open/fixed models, including BAB and child A*/CABS.
- Measure deterministic statistics on a small fixed set with single-thread execution; bound total measurement solver time to 120 seconds, do not infer runtime gains from node counts.
- Review domain iteration, board assignment prerequisites, cloning/recomputation, default behavior, whitespace, and stack ancestry.

## Result

Added optional Manhattan value selection to both puzzle models through ordinary INT_VAL callbacks, with numerical minimum unchanged as default. Parent commit 0d80c51e0b is incorporated into the search branch.

Validation:

- Both normal builds and 96 BFS/path replay checks passed; independent verification passed at W5519716bf265e4d8. Parent user edits are preserved; whitespace and selected clang-tidy checks passed.
- On scramble 12, seed 1, limit 12, one thread: nodes min to Manhattan are BAB 5426 to 11, A* 33 to 29, CABS 33 to 19 in both models. Open BAB failures 2712 to 2 and propagations 606170 to 3127.
- The 24-run comparison used 18.891 seconds of solver time. On scramble 18, seed 3, limit 18, CABS nodes increased 43176 to 50669 and A* changed 7985 to 7981; four BAB runs stopped without solutions at 3500ms. All 20 completed runs agreed on final optima, and every incumbent replayed to the goal. No permanent tests or local sanitizer work.
