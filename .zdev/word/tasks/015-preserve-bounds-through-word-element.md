+++
schema_version = 1
id = "word-015"
key = "add-bounded-word-element"
area = "word"
status = "done"
complexity = "advanced"
blocked_by = ["word-004"]
+++
# Preserve bounds through Word element

## Outcome

Homogeneous bounded Word element propagation uses candidate interval information to prune indices and publish both cube and ranked hulls.

## Context

Word element currently lowers candidates and result to WordView, subscribes only to bit events, and computes only the cube hull. It cannot remove an index whose candidate interval is disjoint from the result. A previous advisor-based incremental scan was flat or slower and is not the design for this task.

## Boundaries

- Use one ordinary candidate scan to combine cube and interval reasoning; do not revive advisor-based incremental scanning.
- Preserve duplicate candidates, index holes, aliases, cancellation, cloning, recomputation, and compact fallback.
- Use homogeneous signed or unsigned bounded actors only where ordering interpretation matches.
- Do not add a general table constraint.

## Done when

- [x] Interval disjointness removes unsupported indices and remaining candidates publish sound cube and ranked hulls to the result.
- [x] Result narrowing removes candidates using cube-plus-range disjointness without losing supported values.
- [x] Small-width oracle coverage includes duplicates, aliases, sparse indices, signed and unsigned kinds, cloning, and replay.
- [x] A lookup or register-file portion of the realistic model demonstrates useful propagation or search value.

## Validation

- Run focused Word Element tests and the small-width oracle.
- Run interleaved Release comparisons on the realistic mixed lookup model and compact controls.
- Run TestFramework.

## Result

Preserve unsigned and signed bounded domains through Word Element using one scan for index support and cube/ranked hulls.

Validation:

- Independent verifier passed exhaustive bounded Element oracle, lifecycle/alias/fallback audit, focused Element, and anchored Word::TestFramework.
- Natural register-file comparison preserved 16 solutions/checksum 1972 while bounded pruned two root indices and reduced 35 nodes/2 failures/127 propagations to 31/0/12; exact-parent compact counters matched.
- Twenty rotated/reversed Release trials, README/runner review, diff checks, artifact hygiene, and immutable context comparison passed.
