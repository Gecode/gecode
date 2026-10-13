# Native progressive column search

This API is part of the experimental 6.5.0 preview, disabled by default.
Build the library with `-DGECODE_ENABLE_EXPERIMENTAL=ON` (CMake) or
`--enable-experimental` (Autoconf). `GECODE_HAS_EXPERIMENTAL` in the generated
configuration header reports availability. Preview APIs may change or be
removed in future feature releases; the correctness contracts below still apply.

`Column<T>` searches ordinary Spaces with sequential depth queues. It requires
no Model declaration or compilation. The Space supplies `compare` and strict
`constrain`, and `Search::HeuristicGuidance` supplies an optimistic total bound:
a lower bound for minimization or upper bound for maximization, exact on a
solution. The engine adds no prefix cost. Guidance follows the [A* callback
contract](astar.md).

```cpp
Search::Options options;
options.threads = 1;
Search::ColumnOptions quotas(1, 1); // initial quota, growth per full cycle
Column<MySpace> search(root, guidance, options, quotas, pruning);
while (MySpace* solution = search.next()) {
  // Each returned solution is owned by the caller and strictly improves.
  delete solution;
}
// !search.stopped() proves the best accepted incumbent optimal,
// including external adoption, or infeasibility if none was accepted.
```

The optional `pruning` argument defaults to `Search::StatePruning()`. The native
`Search::columnengine(root, guidance, options, quotas, pruning)` and ordinary
`column<MySpace>(guidance, options, quotas, pruning)` builder use the same
configuration. Builders copy callbacks and quotas. Root ownership follows
`options.clone`: the default preserves the caller's root; false transfers it
after configuration validation. Returned solutions and external incumbents
are independently owned clones. `constrain(incumbent)` adopts a compatible,
strictly improving incumbent and ignores equivalent or worse values.

A cycle visits successive committed depths from zero. At each depth it selects
up to the positive quota of parents, finishes every original n-ary Choice, and
then advances. Newly generated deeper queues participate in that same cycle.
Queued alternatives remain available for later cycles. Publication preserves
cycle depth, quota usage, selected parent and alternative cursor. Ordinary
stops and capacity-one worker parking preserve the same continuation, including
interrupted preparation. Skipping bound-pruned queued entries consumes no
expansion quota. A new cycle starts only after all depths have been visited;
it adds `quota_growth` with saturation at the largest unsigned-long value.

Defaults are `initial_quota=1`, `quota_growth=1`, and `frontier_limit=0`.
`Search::ColumnOptions(1, 0)` selects fixed-quota CBFS. The quota schedules
expansion; it never truncates a queue. Within each depth, supplied rank orders
lower first independently of objective bound. Without rank, the most optimistic
bound comes first in the objective direction. Ties use insertion order. Rank
never prunes. A misleading early goal can precede a better one, so continue
search to natural exhaustion for a proof.

An optional state registry uses owned descriptors, exact keys after hash
bucket selection, and sound equivalence/dominance relations. It admits fresh
improving labels even after an earlier representative expanded, including with
inconsistent admissible bounds. Replaced metadata does not delete old pending
Spaces or their Choice cursors. A descriptor ceiling forgets metadata alone;
it may increase repeated work but preserves coverage. Omitted pruning creates
no registry or descriptors. `state_pruned`, `state_descriptors` and `state_bytes`
follow [A*'s accounting](astar.md); registry labels must include prefix objective
cost and every residual fact relevant to feasibility and brancher/search progress.
The [A* pruning contract](astar.md) explains why identical domains and costs at
different stages are insufficient, and when to include stage state or decline
extraction.

The frontier cap counts queued Spaces plus the selected unfinished parent.
A fully expanded parent is retired before its last child is admitted, so a
unary chain fits cap one. Incumbents, caller-owned solutions, temporary children
and brancher trials are excluded. If an otherwise admissible child cannot fit,
search stops incompletely and stays stopped until `Search::Engine::reset`.
An ordinary stop can resume.

This is a Space count, without a byte or process-RSS guarantee. Each retained
node owns a stable Space; the engine reuses each selected parent for its final
alternative. Descriptor vectors and queue/container capacity add storage.
`c_d` and `a_d` do not enable recomputation. No-good extraction is empty.
Only one expanded thread and no tracer are supported, with explicit exceptions
for unsupported settings and zero initial quota. If a callback, comparison or
model operation throws, destroy the engine before using it again.

Build `astar` to run progressive and fixed quotas against the same misleading
rank model used by A*/BAB/CABS. The [TSPTW example](../examples/data/tsptw/README.md)
accepts `--engine column`, `--initial-quota`, `--quota-growth`, and
`--frontier-limit` with closed, transition and eager formulations. Its defaults
remain prefix bounds and residual propagation off. Installed consumers use
`find_package(Gecode CONFIG REQUIRED COMPONENTS search int)` and link
`Gecode::gecodesearch` and `Gecode::gecodeint`.
