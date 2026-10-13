# Native complete anytime beam search

This API is part of the experimental 6.5.0 preview, disabled by default.
Build the library with `-DGECODE_ENABLE_EXPERIMENTAL=ON` (CMake) or
`--enable-experimental` (Autoconf). `GECODE_HAS_EXPERIMENTAL` in the generated
configuration header reports availability. Preview APIs may change or be
removed in future feature releases; the correctness contracts below still apply.

`CABS<T>` searches an ordinary `Space` subclass with its existing branching,
`compare`, and strict-improvement `constrain` methods. It needs no Model,
portfolio, or open list. Like [A*](astar.md), it returns caller-owned, strictly
improving scalar integer solutions through `next()`.

```cpp
Search::HeuristicGuidance guidance(Search::HeuristicGuidance::MINIMIZE,
  [](const Space& s) -> std::int64_t {
    return static_cast<const MySpace&>(s).objective.min();
  });
CABS<MySpace> search(root, guidance);
while (std::unique_ptr<MySpace> solution{search.next()}) {
  // Read or retain this improvement; the caller owns it.
}
if (search.stopped()) {
  // Search has no proof. Ordinary stop limits can be raised to resume.
} else {
  // The best accepted incumbent is optimal; without one, the model is infeasible.
}
```

The bound is an optimistic **total** objective value on a stable propagated
space: lower for minimization, upper for maximization, and exact on solutions.
The engine adds no prefix cost. Bounds, comparison, and constraints must agree
on the same scalar integer objective. Values are signed 64-bit integers and
compared directly; callback arithmetic must stay in range. Callbacks read only
the supplied space, must not post constraints, and must not capture another
space's local variables. Captured dependencies must outlive engines and builders.

An optional third guidance argument supplies a rank. Lower rank selects the
beam, independently of the bound. Without rank, the beam prefers the optimistic
bound in the objective direction. Equal priorities use generation order. Rank
never justifies sound pruning.

Each native `Choice` commit advances one layer. The engine starts at width one,
considers every generated terminal node, and retains at most that many unresolved
next-layer candidates. A shallow goal can be returned before deeper improvements;
publication preserves the remaining parents, alternatives, and preparation.
After all retained nodes and original alternatives in a pass finish, CABS can
prove optimality when every width-discarded bound excludes strict improvement
of the accepted incumbent. It keeps the minimum discarded bound for minimization
or maximum for maximization across every layer, independently of rank. This
also applies when a discarded bound equals the incumbent objective. If a bound
still permits improvement, or no incumbent exists after width loss, CABS doubles
the width and restarts from an unchanged stable root, retaining the incumbent.
A pass with no width loss can prove optimality or infeasibility. Finite search
trees are required for the widening completeness claim.
Natural exhaustion proves optimality of the best accepted incumbent, including
external adoption, or infeasibility if no incumbent was accepted. No newly
returned solution after adoption does not imply infeasibility.

Beam-specific limits are separate from ordinary search options:

```cpp
Search::Options options;
options.clone = false;
Search::CABSOptions limits(/*maximum width=*/16, /*frontier nodes=*/1000);
CABS<MySpace> search(new MySpace, guidance, options, limits);

std::unique_ptr<Search::Builder> builder(cabs<MySpace>(guidance, options, limits));
std::unique_ptr<Search::Engine> engine((*builder)(new MySpace));
engine->constrain(external_solution);
while (std::unique_ptr<Space> solution{engine->next()}) {
  // Each solution improves the accepted incumbent, including external ones.
}
```

Zero means unlimited for either limit. Pass widths are 1, 2, 4, and so on; a
maximum of 3 permits widths 1 and 2. A complete pass may prove its result at the
maximum width. When an incomplete pass needs a wider restart, exceeding the
maximum or overflowing width growth reports `stopped()`. Frontier exhaustion
also reports a stop. These intrinsic stops remain stopped on repeated `next()`
calls. `Search::Engine::reset(new_root)` starts fresh, clearing the stop,
incumbent, pending work, and statistics; it takes ownership of the new root.

The frontier quota counts retained pending search spaces: unexpanded entries
in the current layer, next-layer candidates, and a selected parent with remaining
alternatives. Moving a parent between these owners does not consume an extra
slot, and replacing a candidate uses its existing slot. The quota excludes the
immutable restart root, owned incumbent, caller-owned returned solutions, a
temporary successor, and hidden brancher trial spaces. It is a **node quota**,
without a byte or process-memory guarantee.

Ordinary stop objects retain continuation. Raise or reset their limits and call
`next()` again to continue the same pass. Capacity-one `WorkerControl` supports
parking and resuming without reporting exhaustion. CABS rejects expanded thread
counts other than one and rejects tracers before taking root ownership.

The default `Options::clone=true` keeps the supplied root with the caller;
`clone=false` transfers it after successful construction. Returned solutions and
the retained incumbent have independent ownership. Equivalent or worse external
incumbents are ignored, and incomparable ones raise `Search::Incomparable`.
Cuts affect committed children, leaving saved source choices and the restart root
unchanged. Already inherited cuts are not repeated, so retained completion
branchers can finish. If a callback, comparison, or model operation throws,
destroy the engine before using it again.

The engine stores cloned states and reuses each selected parent for its final
alternative. `c_d` and `a_d` do not enable replay or recomputation. No-good
extraction returns the existing empty result. Statistics
count depth as committed search edges and restarts as widening passes.

CABS accepts the same optional [state-pruning declaration](astar.md) as A*,
with the same expert obligation to preserve feasibility, cost and residual
brancher/search progress, owned keys/labels and exact collision checks:

```cpp
auto pruning = TSPTW::Transition::pareto_pruning();
CABS<TSPTW::Transition> search(root, TSPTW::Transition::guidance(),
                              options, limits, pruning);
std::unique_ptr<Search::Builder> builder(
  cabs<TSPTW::Transition>(TSPTW::Transition::guidance(), options, limits, pruning));
```

Descriptors belong only to retained **next-layer candidates**. CABS compares
neither selected/current-layer nodes nor previously expanded nodes. A live
equivalent or dominating candidate can reject an arrival. A new dominator takes
one victim's retained slot before removing any other victims, even if its beam
rank is worse. Sound replacement does not lose coverage. An arrival discarded
by ranking registers nothing; an evicted candidate loses its descriptor.
Descriptors are cleared as candidates become the current layer, between passes
and on reset. A beam-discarded representative therefore cannot suppress a later
surviving route. Earlier width-discarded bounds remain in the pass aggregate;
sound label pruning does not erase them. Completion still requires processing
all retained work and bounding out every width-discarded alternative.

`descriptor_limit` forgets all candidate descriptors if their count exceeds the
ceiling, preserving the candidates themselves. It is independent of width and
frontier limits and never reports a stop. Empty policy performs no extraction,
descriptor allocation or registry work. `state_pruned` counts rejected arrivals
and removed dominated live candidates, separately from failures and beam loss.
`state_descriptors`/`state_bytes` report retained descriptor peaks using the
[A* storage estimate](astar.md); all three are zero when pruning is disabled.
The existing stop, proof and independently owned solution contracts still apply.

Build and run the [closed example](../examples/astar.cpp):

```sh
cmake --build build/open-search --target astar
build/open-search/bin/astar
```

The example runs A*, BAB, and CABS on one bounded formulation. A misleading rank
makes capped width one return 4 and -2, then stop without a proof. Width two, width four, and
unrestricted widening recover -7 and prove it optimal. A frontier quota of one
stops after 4. The example prints elapsed milliseconds, nodes, and restarts for
fixed width/frontier settings. Its tiny timings illustrate the reporting and do
not establish performance gains or justify default tuning. It links only the
search and integer libraries; installed consumers can link `Gecode::gecodesearch`
and `Gecode::gecodeint`.
