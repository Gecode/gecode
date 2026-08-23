+++
schema_version = 1
id = "word-007"
key = "retire-bounded-disequality"
area = "word"
status = "open"
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

- [ ] Already disjoint bounded intervals post no disequality actor.
- [ ] A bound-only update that makes the intervals disjoint subsumes an existing actor.
- [ ] Compact, mixed-kind, assigned, alias, clone, and replay behavior remains unchanged.

## Validation

- Run focused Word relation tests and the new actor-retirement regression.
- Run TestFramework.
