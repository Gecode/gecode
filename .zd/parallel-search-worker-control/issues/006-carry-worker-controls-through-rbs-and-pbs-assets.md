+++
schema_version = 1
id = "parallel-search-worker-control-006"
proposal_key = "integrate-rbs-pbs-controls"
proposal_hash = "sha256:92ab15f51912a8aac85bda5f35903d0603984bb91df8546baf610829d710833f"
area = "parallel-search-worker-control"
kind = "implementation"
interaction = "afk"
priority = "high"
status = "done"
blocked_by = ["parallel-search-worker-control-004", "parallel-search-worker-control-005"]
+++
# Carry worker controls through RBS and PBS assets

## Source

The motivating use case reallocates workers among portfolio assets, while RBS and PBS currently copy or rewrite engine options during construction.

## What to build

Carry the approved control handle through RBS restarts and PBS engine builders so each portfolio asset can be resized independently. Preserve portfolio solution and stop handshakes when inner DFS or BAB workers are parked, and document that PBS does not choose the allocation policy.

## Non-goals

- Adding an automatic allocation algorithm to PBS.
- Allowing an asset limit of zero.

## Acceptance criteria

- [x] An external controller can independently change active-worker limits for multiple PBS assets.
- [x] RBS restarts retain the same control and current requested limit.
- [x] A PBS solution or stop request cannot leave a parked asset blocking the portfolio completion handshake.
- [x] PBS and RBS destruction remains safe after repeated resizing.
- [x] A focused portfolio test reallocates a fixed active-worker budget among assets.

## Validation

- Run focused RBS restart and PBS per-asset allocation tests for DFS and BAB assets.
- Run existing sequential and parallel portfolio tests.
