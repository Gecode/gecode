+++
schema_version = 1
id = "parallel-search-worker-control-005"
proposal_key = "implement-bab-worker-parking"
proposal_hash = "sha256:1a74728e45547d018a7d3a2b2fd96f53f914886084ec319f64ef6a4b4f198e4d"
area = "parallel-search-worker-control"
kind = "implementation"
interaction = "afk"
priority = "high"
status = "done"
blocked_by = ["parallel-search-worker-control-004"]
+++
# Adjust active BAB workers while preserving incumbents

## Source

BAB workers add incumbent clones, constrain propagation, and recomputation marks to the parked-worker state already proven for DFS.

## What to build

Extend adjustable execution slots to parallel BAB. Ensure parked workers retain and receive incumbent state correctly, external constrain remains safe, and repeated resizing cannot lose optimal solutions or violate BAB recomputation invariants.

## Non-goals

- Changing incumbent selection or BAB search policy.
- Guaranteeing deterministic incumbent order.

## Acceptance criteria

- [x] Repeated shrink and growth preserve the proven optimum across representative BAB models.
- [x] New incumbents safely constrain active and parked workers.
- [x] External constrain remains correct while workers are parked or transitioning.
- [x] Reset, statistics, tracing, stop, and destruction remain safe for resized BAB engines.
- [x] Existing BAB behavior is unchanged when the requested limit equals capacity.

## Validation

- Run focused BAB optimality and external-constrain tests over repeated resize sequences.
- Run existing BAB and parallel-search regression tests.
