# Adjustable parallel-search workers

## Purpose

`Search::WorkerControl` lets an external scheduler change how many workers a
running DFS or BAB engine may use. A portfolio with a fixed thread budget can
move workers among its engines at runtime.

The engine still starts with a fixed maximum:

```cpp
Gecode::Search::Options options;
options.threads = 8; // Resident worker capacity

Gecode::Search::WorkerControl control(2); // Initially request two workers
options.worker_control = control;

Gecode::DFS<MySpace> engine(root, options);

// Safe from another thread while engine.next() is running.
control.request(6);
control.request(0); // Pause
control.request(1); // Resume
```

Gecode resolves the `threads` option when it constructs the engine. The result
is the fixed worker capacity. A request must be between zero and that capacity,
inclusive. Zero pauses the engine without discarding its search state.

## Asynchronous semantics

`request` is thread-safe and non-blocking. It publishes a desired worker count
and wakes parked workers when necessary; it does not wait for the engine to
reach that count.

Changes take effect cooperatively at scheduler boundaries. A grow request makes
parked workers eligible immediately. A shrink request cannot interrupt a
worker in the middle of a search action. Excess workers finish that action and
park before beginning another, so the engine may briefly use more workers than
requested. Once a request for zero has taken effect, `next` remains blocked
until a positive request resumes the engine.

`TimeStop` follows the same cooperative rule. The engine checks stop objects at
search boundaries; there is no timer thread to wake a paused engine. If a
`TimeStop` expires while the worker request is zero, `next` remains blocked.
After the engine resumes, it observes the expired stop at a normal stop check.

Requests affect scheduling, not search correctness. DFS still enumerates the
same solution set and BAB still returns the same optimum. Parallel exploration
order, solution order, node counts, failure counts, and the time at which a
request becomes visible remain nondeterministic.

Shrinking parks resident operating-system threads. It does not destroy their
thread objects or discard their engine-local search state. Growing wakes those
threads again. Set the capacity to the largest allocation the engine may
receive. Parked workers retain their stacks and other per-worker state.

## Handle lifetime and ownership

A control is a copyable handle with shared identity. Copies made before or
after engine construction publish to the same request state:

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

A shared identity can be bound to only one logical leaf engine. It cannot be
shared by two DFS/BAB engines, reused for a replacement engine after
destruction, or attached directly to an enclosing meta-engine. These uses raise
`Search::WorkerControlInUse`. Construct a new control for each leaf-engine
lifetime.

Destroying an engine safely detaches its state, but it does not make that
identity reusable. A copied handle may outlive the engine; further in-range
requests are harmless and cannot access destroyed scheduler state.

## Meta-search

Restart-based search keeps one leaf control through construction and reset.
The same DFS or BAB leaf engine remains the adjustment target across restarts.

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
resizing is cooperative, so a controller that must measure the handoff needs
its own acknowledgement or accounting. Gecode does not choose an allocation
policy.

PBS completes a `next` round only after every active asset has reported. An
asset at zero still observes the internal stop used to close the round. It
reports without doing more search, and its worker request remains zero. A
solution from another asset returns while the asset stays paused. If every
active asset is paused, `next` blocks until at least one resumes.
