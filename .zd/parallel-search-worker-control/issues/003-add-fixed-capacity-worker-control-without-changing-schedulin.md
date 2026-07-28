+++
schema_version = 1
id = "parallel-search-worker-control-003"
proposal_key = "implement-control-surface"
proposal_hash = "sha256:eeae7c325a677adf94b38ac7823985182c6cafa6d10bfcecd3c59c290dfb58bd"
area = "parallel-search-worker-control"
kind = "implementation"
interaction = "afk"
priority = "high"
status = "done"
blocked_by = ["parallel-search-worker-control-001", "parallel-search-worker-control-002"]
+++
# Add fixed-capacity worker control without changing scheduling

## Source

The two approved design studies establish the public contract and scheduler integration boundary.

## What to build

Add the external worker-control type and Options integration end to end while retaining current fixed-worker execution when the requested limit equals capacity or no control is supplied. Validate construction, copying, bounds, notifications, and non-threaded compatibility without yet parking DFS or BAB workers.

## Non-goals

- Parking workers.
- Changing DFS or BAB search order.

## Acceptance criteria

- [x] Callers can configure a fixed maximum and an initial requested limit through the approved public API.
- [x] Concurrent limit updates use defined synchronization and never mutate copied Options::threads.
- [x] Existing callers without a control object retain current behavior.
- [x] Invalid-limit and non-threaded behaviors match the approved API contract.
- [x] Focused tests cover construction, copying, concurrent updates, and lifetime boundaries.

## Validation

- Build the search library and run the focused worker-control API tests.
- Run existing search tests with no control object and with limit equal to capacity.
