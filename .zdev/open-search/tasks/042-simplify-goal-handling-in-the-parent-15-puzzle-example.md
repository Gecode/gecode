+++
schema_version = 1
id = "open-search-042"
key = "simplify-open-puzzle-goals"
area = "open-search"
status = "open"
complexity = "standard"
afk = true
blocked_by = []
+++
# Simplify goal handling in the parent 15-puzzle example

## Outcome

The feature/open-cp 15-puzzle example explains goal detection directly in its transition, with simple readable code; feature/open-search incorporates the same correction.

## Context

The user finds wait_goal and its surrounding force_goal/check_bound/constrain choreography unnecessarily complicated. They want the transition to check the new board and close the path on the goal. The shared model belongs on feature/open-cp. Read examples/fifteen-puzzle.cpp in both branches and the open-list transition/materialization contract: materialization precedes assignment. The user clarified that the open model should resemble the fixed model, with the same slide and goal constraints and variables introduced as the list grows. This supersedes the deferred-goal scheduling from tasks038/039; preserving their two-board root is not a requirement. Coordinate parent source commit and child rebase; preserve the existing parent open-route edit.

## Boundaries

- Change the shared puzzle model on feature/open-cp first. Preserve fixed-model semantics, CLI, BAB default, known_length, and child heuristic adapters. No kernel or open-list API changes.
- Use the fixed model's shared slide and goal constraints directly in the transition; remove the wait_goal/force_goal machinery. Preserve goal termination, move-cap infeasibility and admissible Manhattan bounds. Let ordinary constraints determine how much of the list must be constructed.
- Use bounded example runs and temporary independent checks, not new permanent test fixtures or a benchmark campaign. Coordination owns commits, stack refresh and publication.

## Done when

- [ ] The new-board goal decision is clear in the transition and redundant goal-handling helpers are removed; start-at-goal and cap cases remain correct.
- [ ] Independent bounded checks confirm valid optimal paths and infeasible bounds for parent BAB and child BAB/A*/CABS on open/fixed models; default open BAB remains practical.
- [ ] Parent source change is committed, the child remains stacked on it, and the unrelated parent edit is unchanged.

## Validation

- Build the affected examples on parent and child, including a representative ASan/UBSan run using existing settings where available.
- Use a temporary independent BFS/path replay checker for a small solved/unsolved/bounded matrix, plus the default seed under a short timeout. Check incumbent cuts and construction for a move cap larger than the solution.
- Review against Gecode/zdev coding guidance, inspect branch deltas and run git diff --check.
