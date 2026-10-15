+++
schema_version = 1
id = "open-search-044"
key = "split-puzzle-goal-implications"
area = "open-search"
status = "open"
complexity = "routine"
afk = true
blocked_by = []
+++
# Split puzzle goal implications with a shared BoolVar

## Outcome

The shared parent post_goal helper posts two clear equivalent implications and computes its board goal indicator once as a BoolVar.

## Context

User explicitly approved the globally equivalent split: (length == states) implies solved; solved implies (length <= states), with solved materialized once as BoolVar from the existing tile conjunction. Unlike !solved implies length>states, this leaves unused fixed-model suffix boards unconstrained. Local research showed stronger propagation across the optional-state boundary. Apply to feature/open-cp first, refresh feature/open-search, and push both. Parent checkout has unrelated uncommitted neighbor-reversion and open-route edits; preserve them byte-for-byte and commit only the goal helper.

## Boundaries

- Only post_goal in examples/fifteen-puzzle.cpp plus task records/stack anchor. Preserve IntArgs in committed code, fixed/open semantics, neighbor order and heuristic adapters.
- No new permanent tests or local sanitizer builds/runs; normal example builds, bounded independent checks and source review suffice.
- Coordinator owns selective parent staging, commits, stack refresh and publication.

## Done when

- [ ] post_goal computes one BoolVar named solved and posts the two approved implications, preserving optional suffix semantics.
- [ ] Both example versions build and bounded checks confirm valid optimal solutions, solved starts and infeasible caps; parent user edits stay unchanged.
- [ ] Parent source commit is incorporated into the child; the verified stack is ready for the requested lease-protected publication.

## Validation

- Build parent and child puzzle examples normally. Use existing temporary BFS/path replay checks or representative existing cases across open/fixed and BAB/A*/CABS; no sanitizer work.
- Review the exact goal-helper change, BoolExpr sharing, optional suffix behavior, ancestry, whitespace and byte-identical parent user edits.
