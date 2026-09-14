+++
schema_version = 1
id = "word-007"
key = "retire-bounded-disequality"
area = "word"
status = "done"
complexity = "routine"
blocked_by = []
+++
# Retire bounded disequality on interval disjointness

## Outcome

Homogeneous bounded Word disequality detects interval disjointness at posting and after bound-only updates, subsuming redundant actors promptly.

## Context

post_bound_rel routes WRT_NQ to the generic bit-only Nq actor. It checks cube disjointness but does not use the existing bounded interval-disjoint test or subscribe to bound changes, so an entailed actor can survive cloning and recomputation.

## Boundaries

- Change entailment and scheduling only; preserve accepted solutions and assigned-value exclusion.
- Keep the generic bit-only actor for compact and mixed-kind disequality.
- Do not broaden this task into relation refactoring.

## Done when

- [x] Already disjoint bounded intervals post no disequality actor.
- [x] A bound-only update that makes the intervals disjoint subsumes an existing actor.
- [x] Compact, mixed-kind, assigned, alias, clone, and replay behavior remains unchanged.

## Validation

- Run focused Word relation tests and the new actor-retirement regression.
- Run TestFramework.

## Result

Added bounds-aware retirement for homogeneous bounded Word disequality while preserving generic compact and mixed-kind behavior.

Validation:

- Independent focused and exhaustive Word relation lifecycle tests passed five iterations, including posting-time and bound-only actor retirement.
- Compact, mixed-kind, assigned, alias, clone, DFS replay, and TestFramework checks passed; production code compiled warning-clean and diff checks passed.
