+++
schema_version = 1
id = "open-search-045"
key = "puzzle-manhattan-values"
area = "open-search"
status = "open"
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

- [ ] Both open and fixed models expose -branching min and -branching manhattan; the latter chooses a legal next blank position minimizing resulting board Manhattan distance, with deterministic ties.
- [ ] Explain the fixed-model filter and compare node/failure/propagation statistics for both orders on identical bounded cases; all reported incumbents replay to the goal and final optima agree.
- [ ] Normal parent and child builds and focused checks pass; parent source commit is incorporated into the child and user edits are preserved.

## Validation

- Build the parent and child fifteen-puzzle targets normally. Use bounded BFS/path-replay checks for both branching options and open/fixed models, including BAB and child A*/CABS.
- Measure deterministic statistics on a small fixed set with single-thread execution; bound total measurement solver time to 120 seconds, do not infer runtime gains from node counts.
- Review domain iteration, board assignment prerequisites, cloning/recomputation, default behavior, whitespace, and stack ancestry.
