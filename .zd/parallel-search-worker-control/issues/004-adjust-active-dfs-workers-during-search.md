+++
schema_version = 1
id = "parallel-search-worker-control-004"
proposal_key = "implement-dfs-worker-parking"
proposal_hash = "sha256:efd63252dca0a96ad3e690d867be967dd7d48c0c07972726790c55ef6d954972"
area = "parallel-search-worker-control"
kind = "implementation"
interaction = "afk"
priority = "high"
status = "done"
blocked_by = ["parallel-search-worker-control-003"]
+++
# Adjust active DFS workers during search

## Source

The approved scheduler design requires a first complete search-engine slice proving execution-slot migration and completeness.

## What to build

Implement cooperative execution-slot acquisition, shrink, growth, parking, and owner reactivation in the shared parallel machinery and connect it to DFS. Deliver an externally adjustable DFS whose fixed worker identities and search state survive repeated limit changes while `next()` is running.

## Non-goals

- BAB support.
- Physical thread retirement or path migration.

## Acceptance criteria

- [x] At most the requested number of DFS workers execute exploration after cooperative convergence.
- [x] A parked worker retaining the last unexplored current branch is eventually allowed to run.
- [x] Repeated shrink and growth preserve complete enumeration with no missing or duplicate solutions.
- [x] Stop, reset, no-good extraction, statistics, tracing, and destruction complete with parked DFS workers.
- [x] The unchanged-limit path avoids contended synchronization per search node.

## Validation

- Run focused DFS resize sequences including 4→1→3→1 while another thread is in next().
- Run existing DFS, no-good, reset, stop, and tracing tests.
