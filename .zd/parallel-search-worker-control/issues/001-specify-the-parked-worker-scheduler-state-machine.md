+++
schema_version = 1
id = "parallel-search-worker-control-001"
proposal_key = "research-scheduler-state-machine"
proposal_hash = "sha256:a213e80a64bbb6be9e03ec9a3452cf28824ba638fcede691d673009502fc2947"
area = "parallel-search-worker-control"
kind = "research"
interaction = "hitl"
priority = "high"
status = "done"
blocked_by = []
+++
# Specify the parked-worker scheduler state machine

## Source

The current parallel engine equates n_busy with search exhaustion, while the approved design requires parked workers to retain unexplored search without executing.

## What to build

Produce a source-backed concurrency design for fixed-capacity worker parking. Define worker states, execution-slot ownership, slot migration, lock ordering, wakeup rules, and progress arguments across work stealing, solution blocking, stop, reset, no-good extraction, and termination. End with a reviewed recommendation precise enough to implement in the shared parallel engine.

## Non-goals

- Implementing the scheduler.
- Supporting zero active workers or changing physical worker capacity.

## Acceptance criteria

- [x] The design distinguishes ownership of unexplored search from permission to execute and states how n_busy retains its exhaustion meaning.
- [x] The design proves that a parked worker holding the remaining current branch is eventually reactivated.
- [x] The design specifies wakeups and lock ordering for C_WORK, C_WAIT, C_RESET, and C_TERMINATE without lost wakeups or barrier deadlocks.
- [x] The design compares viable waiting primitives and records the chosen primitive with steady-state cost and portability rationale.

## Validation

- Run `zd research check` on the research result against the current parallel engine sources.
- Review the state-transition table against DFS and BAB worker loops and record the human decision.

## Reviewed decision

The user approved this design on 2026-07-24.

Logical search ownership and permission to execute are independent state axes.
A worker is logically an owner while its current Space or path retains work,
pending while it has not finished its empty-state transition, and idle only
after reporting exhaustion. Independently, it either holds an execution-slot
lease or does not. Parking removes only the lease: it does not set the worker
idle, clear its Space or path, discard statistics or BAB state, or decrement
`n_busy`.

`n_busy` remains the exhaustion counter. It counts workers that have not
declared themselves idle, including parked owners. Consequently,
`n_busy == 0` retains its existing meaning: all construction-time workers have
accounted for their local work.

| Current condition | Boundary action | Result |
| --- | --- | --- |
| Slot holder, requested limit reduced | Finish the current bounded search action, then relinquish its lease if the scheduler has excess leases | Parked owner or parked idle worker |
| Parked eligible worker, requested limit increased | Acquire a lease under the scheduler mutex | Active owner or active thief |
| Active thief steals work | Keep the lease and become an owner | Active owner |
| Active thief cannot steal while `n_busy > 0` | Hand its lease to the next parked non-idle worker, preferring owners and using round-robin order | Parked owner becomes active |
| Owner exhausts local work | Report idle under the existing search lock, then hand off its lease if logical work remains elsewhere | Active or parked idle worker |
| `C_RESET` or `C_TERMINATE` | Bypass slot admission and wake every worker | Every fixed barrier member acknowledges |

Slot reconciliation occurs at the top of a `C_WORK` iteration before acquiring
the worker mutex. A lease authorizes one existing bounded loop action; resizing
never interrupts propagation or recomputation. Scheduler state is protected by
one scheduler mutex. That mutex is never nested with worker, search, reset,
termination, or wait locks. Event waits and signals occur after releasing it.
Updates to `n_busy` finish under the existing search lock before any scheduler
handoff.

Progress uses an owner-prioritized round-robin queue. While `C_WORK` remains
active, the requested limit is at least one, and `n_busy > 0`, the scheduler
maintains at least one lease or recorded handoff. An empty lease holder either
steals work or passes the lease through the finite queue. Therefore a parked
worker that owns the final non-stealable current branch is eventually
reactivated.

Use one existing sticky `Support::Event` per worker. Its single-waiter contract
matches one event per fixed worker, retains signals issued before waiting, and
permits targeted wakeups. A shared Event is invalid because it supports only
one waiter. A new multi-waiter primitive enlarges the support-layer scope, and
a direct condition variable would duplicate Gecode's conditional thread
abstraction and require a new predicate-mutex protocol. Fairness comes from the
explicit round-robin queue, not from the mutex or event.

During reset, after all start acknowledgements and while workers are blocked at
the existing barrier, rebuild leases deterministically from the latest
requested limit, including worker zero. `C_RESET` and `C_TERMINATE` wake every
parked worker before waiting for acknowledgements. `C_WAIT` may leave already
parked workers quiescent; a limit update during `C_WAIT` cannot admit search
because the event predicate rechecks the command.

The retained source-backed design is
`.zd/parallel-search-worker-control/reports/research-b6fa4905bc0eb2eac389c760842629cbb00d1e80cf415f3c569bc8f3cf953002.md`
from run `research-0c3224aa047f`.
