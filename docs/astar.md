# Native A* and best-first search

This API is part of the experimental 6.5.0 preview, disabled by default.
Build the library with `-DGECODE_ENABLE_EXPERIMENTAL=ON` (CMake) or
`--enable-experimental` (Autoconf). `GECODE_HAS_EXPERIMENTAL` in the generated
configuration header reports availability. Preview APIs may change or be
removed in future feature releases; the correctness contracts below still apply.

`AStar<T>` searches an ordinary `Space` subclass with its existing branching,
`compare`, and `constrain` methods. It returns caller-owned, strictly improving
solutions. The native API needs only the search and integer libraries.
[The closed example](../examples/astar.cpp) compares A*, CABS, and BAB on the same bounded model with objectives
4, -2, -7, and 1. Continuing A* proves -7 optimal.
[CABS guidance](cabs.md) explains the widening and resource-limited runs.

```cpp
Search::HeuristicGuidance guidance(Search::HeuristicGuidance::MINIMIZE,
  [](const Space& s) -> std::int64_t {
    return static_cast<const MySpace&>(s).objective.min();
  });
AStar<MySpace> search(root, guidance);
while (std::unique_ptr<MySpace> solution{search.next()}) {
  // Read or retain the improvement; the caller owns it.
}
if (search.stopped()) {
  // Raise or reset the stop limit and call next() again to resume.
} else {
  // The best accepted incumbent is optimal; without one, the problem is infeasible.
}
```

The bound must be optimistic **total** objective value on a propagated stable
space: a lower bound for `MINIMIZE`, an upper bound for `MAXIMIZE`, and the exact
objective on every solution. A transition model supplies its own prefix plus
remaining-cost bound. The engine adds no cost and compares signed 64-bit values
directly. The bound, `compare`, and strict-improvement `constrain` must describe
the same compatible scalar integer objective. The caller must justify the bound
and keep callback arithmetic in range.

A* selects the best total bound first. An optional third guidance argument
supplies a signed 64-bit rank; lower ranks break equal-bound ties, followed by
insertion order. Rank never prunes. A generated goal can be returned before a
better queued node is expanded, so the first result need not be optimal.
Only natural exhaustion after repeated `next()` calls proves the best accepted
incumbent optimal, including an adopted external incumbent. It proves
infeasibility only if no incumbent was accepted. Returning no new solution after
external adoption therefore does not imply infeasibility.

`bestfirst<T>` exposes a fixed exploration order through the same native
engine and ordinary `Search::Builder`:

```cpp
std::unique_ptr<Search::Builder> builder(
  bestfirst<MySpace>(guidance, Search::RANK_FIRST, options, pruning));
std::unique_ptr<Search::Engine> engine((*builder)(root));
while (std::unique_ptr<MySpace> solution{
         static_cast<MySpace*>(engine->next())}) {
  // Keep searching after the first incumbent.
}
```

`Search::BOUND_FIRST` compares optimistic total bound, lower rank, then insertion
order, preserving `AStar<T>` and `astar<T>` behavior. `Search::RANK_FIRST`
compares lower rank, optimistic total bound, then insertion order. Bounds follow
the objective sense; ranks are lower-first for either sense. A missing rank
callback gives every node rank zero, so rank-first falls back to bound-first.
The engine caches both values when admitting a stable branching node. Ordering
is fixed for the engine, including `reset`; there is no rekeying.

Rank can be arbitrary or inadmissible. It selects the next pending branching
Space, while sound total bounds and the compatible incumbent restriction prune.
Generated solutions are still published immediately. Neither ordering guarantees
that the first goal is optimal or has the best rank. Natural finite-tree
exhaustion with `stopped()==false` proves the best accepted incumbent optimal,
or infeasibility when none exists, provided the bound, comparison and optional
state-pruning contracts preserve coverage. Best-first uses the callback lifetime,
numeric, ownership, stop, exception and descriptor contracts below. The native
factory is `Search::bestfirstengine(root, guidance, ordering, options, pruning)`;
`Search::astarengine` retains its signature and selects bound-first.

The [additive example](../examples/best-first.cpp) minimizes two explicit stages.
The first-stage costs are 0 and 9, with final-arc costs `[5,6]` and `[1,2]`.
After a route decision, `g` is its prefix cost and `h` is its cheapest remaining
arc. At the root `g=0` and `h=5`; at a solution `g` is the full cost and `h=0`.
The admissible total bound is `g+h` throughout.

Greedy rank `h` prefers the second route and publishes 10, then 5. Fixed weight
2 with rank `g+2*h` prefers the first route and publishes 5. Both runs continue
to prove the independently enumerated optimum 5. The example checks addition
and multiplication before evaluating them and rejects signed 64-bit overflow.
These ranks use actual remaining costs; a CP total objective bound is not a
remaining-cost estimate. The fixed weight supplies exploration order only,
with no first-goal optimality or approximation-ratio guarantee. Dynamic weights
and weighted-search repair are outside this API.

Guidance is copied into engines and builders. Callbacks read the space passed to
them without posting constraints. Captured dependencies must outlive every
engine and builder using them; do not capture another space's local variables.
If guidance, comparison, or a model operation throws during search, destroy the
engine before using it again. Constructor failure leaves the supplied root with
the caller. After successful construction, `Options::clone=false` transfers the
root to the engine; the default clones it and leaves the supplied root with the
caller.

Builders and external incumbents use the native engine interface:

```cpp
std::unique_ptr<Search::Builder> builder(astar<MySpace>(guidance, options));
std::unique_ptr<Search::Engine> engine((*builder)(root));
engine->constrain(external_solution);
while (std::unique_ptr<Space> solution{engine->next()}) {
  // Each returned solution strictly improves the accepted incumbent.
}
```

Equivalent or worse external solutions do not replace the incumbent;
incomparable ones raise `Search::Incomparable`. Each accepted incumbent has an
independent owned copy. Queued parents
and saved choices stay unchanged while alternatives remain. The engine commits
each original alternative on an owned child, cloning the selected parent for
earlier alternatives and transferring it for the final one. It then applies a
newer improvement restriction if the child has not inherited it.
Publication and interruption preserve all remaining n-ary alternatives and
interrupted brancher preparation.

The engine stores a cloned frontier and a selected parent, with no hard memory
guarantee. `c_d` and `a_d` do not enable replay or recomputation. No-good extraction
returns the existing empty result. Ordinary stop objects preserve continuation;
capacity-one `WorkerControl` supports pause/resume. A* rejects expanded thread
counts other than one and rejects tracers explicitly. Statistics depth counts
committed search edges, independently of objective value.

Optional state pruning is a separate, copied `Search::StatePruning` value.
Tree search is the default. An extractor returns `std::optional<Descriptor>`:
`std::nullopt` leaves a node unregistered, while a descriptor owns a cached
`std::size_t hash`, a `std::vector<std::int64_t> key`, and a vector `label`.
Hashes select buckets; exact key equality is required before comparing labels.
The relation reports `EQUIVALENT`, `DOMINATES`, `DOMINATED`, or `INCOMPARABLE`
relative to its first argument. Equal labels keep one representative.

The model author must justify every omitted historical fact and guarantee a
consistent, transitive relation preserving at-least-as-good feasible completions.
It must also preserve progress toward a solution under repeated pruning,
including residual brancher/search stages. A sufficient argument shows that
every completion of a suppressed node has an at-least-as-good completion from
its representative with no more remaining search transitions. This follows the
progress condition in Kuroiwa and Beck's
[Definition 14](https://doi.org/10.1016/j.artint.2026.104506).
Prefix objective cost belongs in this comparison; it cannot be inferred from
the optimistic total bound. Earlier time at greater cost may be incomparable.
Partially assigned CP stores need a sufficient key or declined extraction.
Callbacks read only the supplied stable Space, and captured dependencies follow
the guidance lifetime rules. Providing just one callback raises
`Search::UninitializedStatePruning` before root ownership transfers.

For example, consider the finite chain `root -> A -> B -> goal`, where `A` and
`B` have identical variable domains and prefix costs but different brancher
stages. A domain-only key with equal labels is unsafe: A* and column search
retain `A`'s descriptor after expanding it, reject `B` as equivalent, and lose
the only route to the goal. `A` needs two remaining commits; `B` needs one.
For this chain, adding the exact brancher stage to the key distinguishes the
residual problems and preserves progress. More generally, include enough stage
and residual state to justify the relation, or return `std::nullopt` wherever
that information is unavailable. Search depth alone need not be sufficient.
The TSPTW example's remaining-visit set already distinguishes successive visit
stages.

The [native TSPTW transition model](../examples/tsptw-transition.hh) supplies
a worked declaration for its own residual rules:

```cpp
Search::StatePruning pruning = TSPTW::Transition::pareto_pruning();
AStar<TSPTW::Transition> search(root, TSPTW::Transition::guidance(),
                                options, pruning);
std::unique_ptr<Search::Builder> builder(
  astar<TSPTW::Transition>(TSPTW::Transition::guidance(), options, pruning));
// The native factory also accepts pruning as its final argument.
```

A* retains nondominated descriptor values after expansion, without extra Space
clones. A better label enters the frontier before replacing dominated metadata,
even when an older representative already expanded. Admissible inconsistent
bounds therefore remain supported. Older pending Spaces and saved Choice
cursors remain intact and can do redundant work. Reset clears the metadata.
An optional `descriptor_limit` (the declaration's final argument, zero for
unlimited) clears registry metadata when admitting another label would exceed
it. Forgetting never deletes pending work or reports a stop; it only reduces
pruning. An empty declaration performs no extraction, descriptor allocation,
or registry construction.

`Search::Statistics::state_pruned` counts rejected equivalent/dominated arrivals.
`state_descriptors` and `state_bytes` are peaks, aggregated by maximum when
statistics are combined. Bytes estimate descriptor objects and their owned
key/label vector capacities; bucket/container capacity, optional wrappers and
allocator overhead are excluded. They measure neither frontier Spaces nor
process memory, and a descriptor-count ceiling is not a byte limit. Disabled
pruning leaves all three fields zero. Pruning and forgetting preserve the
existing solution ownership, stop and proof rules.

With the repository CMake build, run
`cmake --build build/open-search --target astar` and
`build/open-search/bin/astar`. Installed consumers can use
`find_package(Gecode CONFIG REQUIRED COMPONENTS search int)` and link
`Gecode::gecodesearch` plus `Gecode::gecodeint`.
Build and run the additive example with
`cmake --build build/open-search --target best-first` and
`build/open-search/bin/best-first`; it links the same native libraries.

[Native column search](column.md) uses the same ownership, guidance and state
pruning declarations with retained depth queues. Its optional rank orders each
queue independently of bound, and full cycles increase the expansion quota.
Quota growth zero selects fixed-quota CBFS.
