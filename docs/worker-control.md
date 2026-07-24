# Adjustable parallel-search workers

## Purpose

`Search::WorkerControl` lets an external scheduler change how many workers a
running DFS or BAB engine may use. Its main use is a portfolio that has a fixed
thread budget but wants to move that budget among independently useful search
engines as conditions change.

The engine still starts with a fixed maximum:

```cpp
Gecode::Search::Options options;
options.threads = 8; // Resident worker capacity

Gecode::Search::WorkerControl control(2); // Initially request two workers
options.worker_control = control;

Gecode::DFS<MySpace> engine(root, options);

// Safe from another thread while engine.next() is running.
control.request(6);
control.request(1);
```

The `threads` option is expanded once and becomes the engine's immutable
capacity. A request must be between one and that capacity, inclusive. Zero is
not a pause operation: at least one worker remains available to make progress.

## Asynchronous semantics

`request` is thread-safe and non-blocking. It publishes a desired worker count
and wakes parked workers when necessary; it does not wait for the engine to
reach that count.

Changes take effect cooperatively at scheduler boundaries. A grow request can
make parked workers eligible immediately. A shrink request does not interrupt
a worker in the middle of a search action: excess workers finish their current
action and park before beginning another. Consequently, the old and new
requests may overlap briefly.

Requests affect scheduling, not search correctness. DFS still enumerates the
same solution set and BAB still returns the same optimum. Parallel exploration
order, solution order, node counts, failure counts, and the time at which a
request becomes visible are deliberately nondeterministic.

Shrinking parks resident operating-system threads. It does not destroy their
thread objects or discard their engine-local search state. Growing wakes those
threads again. Applications should therefore choose capacity as a real
resource maximum: parked workers retain stacks and other per-worker state.

## Handle lifetime and ownership

A control is a copyable shared-identity handle. Copies made before or after
engine construction publish to the same request state:

```cpp
Gecode::Search::WorkerControl portfolio_control(4);
Gecode::Search::Options options;
options.threads = 8;
options.worker_control = portfolio_control;

Gecode::DFS<MySpace> engine(root, options);
auto scheduler_control = portfolio_control;
scheduler_control.request(3);
```

An empty default-constructed handle means that worker adjustment is disabled.
Calling `request` on an empty handle raises
`Search::UninitializedWorkerControl`.

One shared identity can bind to one logical leaf engine once. It cannot be
shared by two simultaneously constructed DFS/BAB engines, reused for a
replacement engine after destruction, or attached directly to an enclosing
meta-engine. Such reuse raises `Search::WorkerControlInUse`. Construct a new
control for each leaf-engine lifetime.

Destroying an engine safely detaches its state, but it does not make that
identity reusable. A copied handle may outlive the engine; further in-range
requests are harmless and cannot access destroyed scheduler state.

## Meta-search

Restart-based search transports one leaf control through restart construction
and reset. The same underlying DFS or BAB leaf engine remains the adjustment
target across episodes.

Portfolio-based search does not divide a global thread budget. Give each PBS
asset its own control in the corresponding sequential-engine builder options:

```cpp
constexpr unsigned int budget = 8;

Gecode::Search::WorkerControl asset_a(6);
Gecode::Search::WorkerControl asset_b(2);

Gecode::Search::Options a;
a.threads = budget;
a.worker_control = asset_a;

Gecode::Search::Options b;
b.threads = budget;
b.worker_control = asset_b;

// Construct the PBS assets from builders carrying a and b.

// Later, preserve the external invariant sum(requests) <= budget.
asset_a.request(2);
asset_b.request(6);
```

The portfolio controller must enforce its own active-worker budget. Make the
decrease before the increase if even a brief oversubscription is unacceptable;
because resizing is cooperative, applications needing a strict observed
handoff must also provide their own acknowledgement or accounting layer.
Gecode intentionally exposes the scheduling seam without embedding a
portfolio allocation policy.

## Benchmarking and release gate

The opt-in CMake target `gecode-worker-control-benchmark` and
`misc/benchmark-worker-control.py` measure:

- paired DFS and BAB steady-state overhead when control remains fixed at
  capacity;
- cooperative shrink and grow latency for cheap and propagation-heavy work;
- process CPU and peak-RSS differences with three of four resident workers
  parked; and
- a two-asset PBS reallocation sequence under a fixed four-worker budget.

Build and run the retained suite with:

```sh
cmake --build build --target gecode-worker-control-benchmark
uv run --script misc/benchmark-worker-control.py run \
  --name worker-control-release --build-dir build
uv run --script misc/benchmark-worker-control.py analyze \
  --name worker-control-release
uv run --script misc/benchmark-worker-control.py report \
  --name worker-control-release
uv run --script misc/benchmark-worker-control.py plot \
  --name worker-control-release
```

Raw JSON, stdout, stderr, provenance, analysis, Markdown, LaTeX, PNG, and SVG
artifacts are stored under `results/<name>/`. That directory is ignored by
Git so measurements are not mistaken for portable golden data.

The provisional unchanged-limit release gate is, for both DFS and BAB, median
paired overhead no greater than 3% and the deterministic-bootstrap 95%
confidence-interval upper bound no greater than 5%. Latency and parked-memory
measurements are reported rather than assigned universal limits: acceptable
values depend on propagation granularity, platform, stack configuration, and
the portfolio's reallocation timescale. A human release decision is required
after reviewing the retained results.

### Retained Gecode 7 measurement

The retained `worker-control-release-20260724-final` run at commit
`6e2e8a707771bf540b03c809cf3422d4d006b49b` executed 590 child cases on
Apple Silicon with Apple Clang 21.0.0:

| Measurement | Result |
| --- | ---: |
| DFS fixed-capacity median overhead; bootstrap 95% CI | -0.896%; [-3.873%, 3.885%] |
| BAB fixed-capacity median overhead; bootstrap 95% CI | -0.045%; [-0.224%, 0.129%] |
| Cheap grow latency, p50 / p95 | 12.688 µs / 17.966 µs |
| Propagation-heavy grow latency, p50 / p95 | 12.500 µs / 19.664 µs |
| Cheap shrink latency, p50 / p95 | 7.488 ms / 7.532 ms |
| Propagation-heavy shrink latency, p50 / p95 | 7.493 ms / 7.518 ms |
| Median peak-RSS delta, four resident workers with three parked | 368 KiB |
| Median CPU/wall ratio, one worker / three of four parked | 1.000 / 1.459 |
| PBS maximum summed admitted high-water mark / budget | 4 / 4 |

Both unchanged-limit overhead gates passed, all child cases completed, and all
30 PBS samples respected the active-worker bound. Negative point estimates are
measurement noise, not a claim that the control improves fixed-capacity
performance. Shrink latency reflects cooperative completion of current work;
grow latency reflects waking already resident workers. Peak RSS is a
process-level comparison and not an exact per-thread stack-reservation
measurement.

The automated result is **provisional pass**. The human release decision
remains pending until these results and their platform-specific limitations
have been reviewed.
