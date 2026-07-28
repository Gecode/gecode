# Externally adjustable parallel search workers

Allow an external portfolio controller to asynchronously adjust active DFS and BAB search workers within a fixed construction-time maximum while preserving completeness, optimization correctness, lifecycle safety, and low steady-state overhead.

## Done condition

A caller can construct a parallel DFS or BAB engine with an immutable maximum worker capacity and an externally owned control handle, change the requested active-worker limit from another thread while `next()` is running, and observe the engine continue with no more than that many workers executing search. Repeated shrink and growth preserve DFS solution completeness and BAB optimality. Reset, restart, portfolio stopping, tracing, statistics, no-good extraction, and destruction remain safe. The unchanged-limit path has measured and documented negligible overhead.

## Boundaries and non-goals

- `Search::Options::threads` remains the construction-time maximum and fixes the worker array, worker identities, and reset and termination barrier membership.
- The runtime limit is an integer in `1..maximum`. Supporting zero active workers is out of scope because a fully parked PBS asset cannot observe portfolio stop requests without another wakeup mechanism.
- Parked workers retain their current space, path, statistics, and BAB incumbent state. They may keep an operating-system thread blocked, but they do not consume a search execution slot.
- Growing beyond the construction-time maximum, shrinking the physical worker array, returning search workers to the global thread pool, and migrating paths between worker objects are out of scope.
- A limit change is asynchronous and takes effect at safe exploration boundaries. It does not interrupt propagation or recomputation.
- Completeness and optimization correctness are required. Sequential search order, deterministic solution order, and stable node counts are not promised after resizing.
- The first version targets parallel DFS and BAB. RBS and PBS must preserve and transport the control handle, but they do not choose allocation policy.

## Settled decisions

- Use a fixed-capacity worker pool and park workers rather than create and destroy workers dynamically.
- Separate logical ownership of unexplored search from permission to execute. A parked worker can remain logically busy.
- Make the external update operation thread-safe, nonblocking, and usable while another thread is blocked in `next()`.
- Keep the maximum immutable for the engine lifetime.
- Require execution-slot migration: when active workers exhaust local work while parked workers still own work, a parked owner must be allowed to run.
- Wake parked workers for reset, termination, and other engine-wide lifecycle transitions.
- Control individual PBS assets through handles supplied in their search-engine builder options; PBS itself does not implement an allocation policy.

## Open questions

- Which synchronization design gives a correct wakeup protocol with the lowest steady-state cost: a new multi-waiter support primitive, per-worker events coordinated by the engine, or a narrowly scoped standard-library condition variable?
- What exact state machine should relate `n_busy`, execution slots, idle workers searching for stealable work, parked workers that own a current branch, and global engine commands?
- Should the public type expose only the requested limit, or also observable capacity and effective-active counts for portfolio feedback and tests?
- What ownership and lifetime contract should the control handle use so copied `Options`, RBS rebuilds, PBS builders, and engine destruction cannot retain a dangling control object?
- What benchmark threshold constitutes negligible unchanged-limit overhead, and which examples best represent frequent nodes, expensive propagation, and portfolio use?

## Ordered work

1. Specify the scheduler state machine, lock ordering, wakeup rules, and progress argument against the current DFS and BAB loops. Resolve how reset, stop, solution blocking, termination, and work stealing interact with parked workers.
2. Specify the public control API and its ownership through `Options`, engine construction, RBS, and PBS asset builders. Record error handling for invalid limits and behavior without thread support.
3. Add the control surface with a fixed-limit compatibility path and focused API tests before changing scheduling behavior.
4. Implement execution-slot acquisition, cooperative shrink and growth, and owner reactivation in the shared parallel engine, then connect DFS with completeness tests.
5. Extend the behavior to BAB, including incumbent propagation, external `constrain`, and optimality tests across repeated resizes.
6. Verify RBS restart and per-asset PBS integration, including stop propagation and destruction while workers are parked.
7. Stress lifecycle and race behavior under repeated concurrent limit changes, sanitizer builds, tracing, reset, no-goods, and destruction.
8. Benchmark the unchanged-limit fast path and a representative dynamically allocated portfolio, document semantics, and decide whether the feature is ready for general use.

## Validation

- DFS enumeration returns every expected solution exactly once across repeated shrink and growth sequences.
- BAB returns a proven optimum across the same sequences and remains correct when incumbents arrive while workers are parked.
- The observed number of workers executing search never exceeds the requested limit after cooperative convergence.
- A parked owner eventually executes when it holds the remaining search, so `next()` neither reports false exhaustion nor waits forever.
- Reset, RBS restart, PBS stop propagation, no-good extraction, statistics, tracing, and destruction complete without deadlock or use-after-free.
- ThreadSanitizer reports no races in repeated concurrent control updates; ASan and UBSan remain clean.
- Non-threaded builds retain their current behavior and diagnostics.
- Benchmarks quantify unchanged-limit overhead, resize response latency, resident-thread cost, and portfolio-level resource reallocation behavior.
