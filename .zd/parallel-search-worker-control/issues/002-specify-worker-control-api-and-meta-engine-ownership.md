+++
schema_version = 1
id = "parallel-search-worker-control-002"
proposal_key = "research-control-api-meta-engines"
proposal_hash = "sha256:c66cb9eaaf6a669e23e89d4a6e3a09c17fbdb63cb0f544bfb1039b71ece8fb12"
area = "parallel-search-worker-control"
kind = "research"
interaction = "hitl"
priority = "high"
status = "done"
blocked_by = []
+++
# Specify worker-control API and meta-engine ownership

## Source

The approved plan calls for an external asynchronous control handle that survives copied Options, RBS restarts, and per-asset PBS construction without exposing internal slave engines.

## What to build

Specify the public worker-control API, ownership and lifetime contract, invalid-limit behavior, non-threaded behavior, observability, and transport through Options, builders, RBS, and PBS assets. Include a minimal user-facing example for portfolio resource reallocation and a reviewed decision on whether effective-active counts are exposed.

## Non-goals

- Selecting a portfolio allocation policy.
- Implementing zero-worker pause semantics.

## Acceptance criteria

- [x] The API keeps Options::threads as immutable maximum capacity and defines asynchronous updates in the range 1..maximum.
- [x] The lifetime contract prevents dangling control state through Options copies, RBS rebuilds, PBS builders, and engine destruction.
- [x] Per-asset PBS control is possible without adding allocation policy to PBS.
- [x] Behavior for invalid limits, absent controls, and builds without thread support is explicit.

## Validation

- Run `zd research check` on the research result against Search::Options, Search::Engine, Base, RBS, PBS, and builder sources.
- Review the proposed public names and ownership contract before implementation.

## Reviewed decision

The user approved this design on 2026-07-24.

Add an opaque, copyable value handle:

```cpp
namespace Gecode { namespace Search {
  class GECODE_SEARCH_EXPORT WorkerControl {
  public:
    WorkerControl(void) noexcept;
    explicit WorkerControl(unsigned int requested);
    explicit operator bool(void) const noexcept;
    unsigned int requested(void) const noexcept;
    void request(unsigned int workers);
    unsigned int capacity(void) const noexcept;
  };
}}
```

`Search::Options` stores `WorkerControl worker_control` by value.
`Options::threads`, after expansion, remains the immutable construction-time
capacity. An engaged handle owns hidden reference-counted state shared by the
caller, copied Options, builders, meta-engines, and the attached logical
engine. The state contains the atomic requested limit and wakeup generation;
`request()` never dereferences an engine.

One public control identity binds to one logical engine. Constructing a second
unrelated engine from copied Options containing the same bound control is an
error. RBS owns the logical binding and transports an internal attachment
generation to replacement child engines, so restart does not create a second
binding. Engine destruction retains the shared state until parked workers have
been woken and joined, then detaches it. External copies remain memory-safe;
later requests update inert state and cannot call a destroyed engine.

`WorkerControl(n)` creates unbound state with an initial positive request.
Binding publishes the expanded immutable capacity and rejects a request above
it. `request(0)`, construction with zero, and post-bind requests above capacity
are errors. Requesting through an empty handle is an error; empty queries return
zero and its boolean conversion is false. Binding and requesting serialize in
the shared state so their race either installs an in-range request or reports
the range error.

An empty `Options::worker_control` preserves current fixed-at-maximum behavior.
In a build without thread support, ordinary engine selection and diagnostics
remain unchanged. A sequential engine may bind only at capacity one;
`request(1)` is a no-op and other limits are invalid.

PBS transports controls but implements no allocation policy. Explicit
multi-asset PBS obtains one control from each asset builder's Options. Controls
for builders discarded by outer PBS admission remain unbound. A single engaged
outer control is rejected for homogeneous multi-asset PBS because broadcasting
one identity into several logical engines would make ownership and capacity
ambiguous; users who need controlled homogeneous assets construct explicit
builders.

Version one exposes `requested()` and `capacity()`, but not an
effective-active count. Effective activity is transient during cooperative
shrink, can be zero while `next()` is not exploring, and changes between
observation and policy action. Tests instead use internal generation-tagged
high-water and convergence instrumentation. A public acknowledgement or
effective-active API can be added later if portfolio measurements establish a
concrete need.

Minimal per-asset portfolio use:

```cpp
Search::WorkerControl left(4);
Search::WorkerControl right(1);

Search::Options lo;
lo.threads = 4;
lo.worker_control = left;

Search::Options ro;
ro.threads = 4;
ro.worker_control = right;

SEBs assets { dfs<Model>(lo), dfs<Model>(ro) };
Search::Options po;
po.threads = 2; // Admit both assets; this is not an allocation policy.
PBS<Model,DFS> portfolio(root, assets, po);

Model* solution = nullptr;
std::thread searching([&] { solution = portfolio.next(); });
left.request(1);
right.request(4);
searching.join();
```

The retained source-backed API and ownership design is
`.zd/parallel-search-worker-control/reports/research-745d63ceea1af3022abcf4405052e172a2da339f13db8d04fc2683211aca81a0.md`
from run `research-2ce17fa397e6`.
