+++
schema_version = 1
id = "open-search-010"
key = "native-column-search"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-006", "open-search-008"]
+++
# Deliver native progressive column search with CBFS mode

## Outcome

An ordinary-Space sequential column-search engine offers progressive ACPS and fixed-quota CBFS behavior with retained alternatives, optional native state pruning and honest optimization proofs.

## Context

Read the brief's column-search contract and ../background/next-search-and-benchmarks.md algorithm comparison and inspected ACPS reference. Start with native A*/CABS expansion/prepare/ownership and WorkerToEngine adapters, now using the optional descriptor declaration. Reuse actual mechanisms only; depth queues/cycle state belong to this engine, not a universal policy framework.

## Boundaries

- Keep normal Space, scalar min/max, owning improving next(), native builders and capacity-one worker control. Model and portfolio remain optional.
- Default initial quota 1 grows by 1 each full cycle; growth 0 is fixed-quota CBFS. Quotas schedule expansion rather than truncate queues. A retained-frontier resource cap must stop incompletely.

## Done when

- [x] A public native engine/builder expands up to the positive quota from successive depth queues, admits deeper successors during the cycle, and retains all deferred work. Queue ordering uses lower-first rank when supplied, otherwise optimistic bound; only bound/dominance prune.
- [x] Cycle/depth/quota and original n-ary Choice cursor survive solution publication, ordinary stops and parking. Checked growth saturates safely; resource caps latch incomplete status.
- [x] Optional equality/dominance works with fresh-label reopening; policy omission performs tree search. Ownership and external incumbents agree with existing heuristic engine contracts.
- [x] Closed CP and transition TSPTW executables expose this engine and fixed-quota mode using the same formulations/guidance. A misleading early goal still yields the optimum on natural exhaustion.
- [x] Focused semantic fixtures cover min/max, finite no-solution exhaustion, mixed goal depth, quota cycling, interrupted n-ary expansion and frontier-cap termination; unsupported parallel/tracer settings receive explicit diagnostics.

## Validation

- Build/run native examples and TSPTW correctness cases, compare fixed and progressive quotas with BAB/A*/CABS, and independently check all tours.
- Run native search/completion/worker-control/portfolio selections, check and native ASan -threads 1.
- Verify direct native linkage and public installed builder use without Model.

## Result

Added native progressive Column search and fixed-quota CBFS, preserving full cycles, choices, pruning and incomplete cap status; exposed ordinary builders and TSPTW controls.

Validation:

- Independent PASS at W79937a33b1892d77 with exact unchanged snapshot.
- Release and Model-disabled ASan builds; native quota/min-max/ownership/reopening/completion/parking/portfolio checks; exhaustive checked TSPTW fixed/progressive cases and bounded canonical validation passed.
- Repository check, inherited Model/open-list/regular selections and four open examples passed. Both installed ordinary-Space consumers run Column and column builders with search/int/kernel/support linkage and no Model/driver dependency.
