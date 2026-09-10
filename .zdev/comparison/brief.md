# Non-mutating solution comparison

## Objective

Give Gecode 7 a model-defined, non-mutating way to compare solution quality,
and use it wherever best-solution search arbitrates between independently
produced incumbents. A custom objective should work in parallel BAB and PBS
without posting constraints or propagating either solution to decide which
one to retain.

This area is isolated on `feature/comparison`, based on `main` at
`6b7de57b0414fe2f8af3513743c9835840a3c019`. Its records have `pull-request`
ownership: review them on the branch and remove them with `zdev cleanup squash`
before squash merge.

This brief is a design proposal. The requested exploration creates the area
and records the recommendations; it does not authorize implementation tasks.
Gecode 7 may impose new comparison requirements on optimization spaces; a
legacy arbitration fallback is not required. Each space defines when it has
enough information to compare. How to report insufficient information, float
behavior, and partial-order handling remain proposals pending discussion.

## Boundaries

The proposed implementation PR includes the Space contract, standard objective
classes, FlatZinc optimization, incumbent arbitration in BAB/PBS/RBS, documentation,
and focused regression coverage. Include the small driver change needed to keep
the existing Gist display-comparison overload visible.

Retain `constrain()` for pruning, recomputation, and restarts. Do not replace it
with a comparison of domain bounds. Preserve the existing search ownership,
synchronization, stopping, and restart protocols except where comparison
outcomes must reach the caller.

Exclude implementation of diverse-solution search, Pareto-front enumeration,
a new search engine,
objective extraction or serialization, a comparator registry,
heterogeneous-model conversion, changing
the C++ language requirement, and a performance benchmark campaign. No slices
or tasks are needed for this exploration.

Include limited advance planning for diverse-solution and Pareto search so this
interface leaves those uses possible. Their implementation and concrete APIs
remain separate work; see Future search below.

## Terms and proposed interface

An **incumbent** is a feasible solution retained as the reference for future
search. **Better** is relative to the model's objective, so a larger integer is
better for maximization. **Equivalent** means equivalent quality according to
the model, not equal assignments. Interchangeable improvement restrictions are
an additional requirement of the current single-incumbent optimization engines.
**Incomparable** is a known result under a partial ordering; it does not mean
that an objective is unassigned or comparison has not been implemented.

Recommend one virtual member of `Space`, with receiver-relative results:

```cpp
enum SpaceComparison {
  SC_BETTER,
  SC_EQUIVALENT,
  SC_WORSE,
  SC_INCOMPARABLE
};

virtual SpaceComparison compare(const Space& other) const;
```

Recommend keeping the result restricted to ordering outcomes. Missing comparison
support and insufficient information to compare are errors, not ordering results. The
readiness decision is still open; this is the recommended interface if the
precondition is adopted. The exact enum spelling is provisional. This uses the
current C++17 baseline and needs neither an additional capability virtual nor a
new objective base class. A non-pure default that reports unsupported use can
leave satisfaction-only spaces unaffected; optimization spaces must provide
the comparison, usually through a convenience base.

The method reads objective data only. It must not call `status()`, post a
constraint, clone either argument, alter domains or branchers, or change shared
model state. Small temporary allocations, such as existing `IntVarArgs` cost
accessors, need not be prohibited. The public method need not be `noexcept`;
normal valid comparisons must not throw.

The generic contract is that the operands contain enough information for the
individual space to establish the comparison it reports. There is no generic
assignment test, requirement that every objective component have a value, or
requirement to call `status()`. Comparability can depend on the pair: a space
may know how it relates to one operand but lack information for another.
Reported relations must remain sound under the model's interpretation.

For example, scalar integer convenience classes naturally compare assigned
cost values using `val()`. A lexicographic model can establish a strict relation
from a decisive prefix without knowing later components. A custom model may
compare a derived property or prove a relation from domains. These are model
contracts, not restrictions imposed by `Space` or a generic search preflight.
Do not conflate lack of enough information with genuine Pareto incomparability.

Search invokes comparison on its stable solution spaces. Direct callers must
respect the individual space's documented comparison domain; the generic
interface does not require a completed search or fully assigned space. A space
being comparable does not by itself make it a valid incumbent: external
incumbents must also satisfy the engine's solution contract. Likewise,
`constrain()` retains its own model-defined requirements on its argument.

Recommend treating insufficient information for the requested comparison as
a precondition violation, reported using the appropriate model error convention.
For scalar integer helpers, the existing unassigned-value exception fits.
The alternative is a distinct `SC_UNDETERMINED` result, allowing a caller to
recover when the model cannot establish the relation. The current arbitration
sites would still need an error policy for this result. The reporting choice
is open; model ownership of comparability is settled.

All participating assets must use the same objective direction, component
interpretation, and pruning policy. Built-in comparisons should diagnose
incompatible objective families or lexicographic dimensions using the project's
existing error conventions; a successful cast alone cannot prove model
compatibility. Distinct model classes may cooperate through an intentionally
shared objective interface, but automatic conversion is outside this PR.

For exact optimization, define `better(a,b)` by `a.compare(b) == SC_BETTER`.
It must be irreflexive and transitive, reverse to `SC_WORSE`, and agree with
`constrain(b)` on objective-complete feasible solutions. Equivalence must be
transitive and preserve comparisons with every third solution. For the existing
single-incumbent engines, every pair must be better, worse, or equivalent.

There is also a search requirement: if `a` replaces `b`, the solutions admitted
by the new improvement restriction must be a subset of those admitted by the
old one. Equivalent incumbents must admit the same future improvements. This
is what makes accumulated cuts, local incumbents, and portfolio exhaustion
compatible. A pairwise ranking alone cannot establish this property.

## Future search

Keep pairwise comparison separate from the policy for retaining solutions and
restricting future search. The comparison method must not assume that its
argument is the sole incumbent, consult an engine-owned solution history, or
encode whether a solution should be retained in a particular archive.

For diverse-solution search, a future engine needs restrictions against all
previous accepted solutions. Repeated calls to a model-specific restriction may
suffice when they accumulate conjunctively; recomputed nodes and portfolio assets
must receive every relevant restriction, not just the latest one. Concurrent
candidates also need to be checked against the accepted history, including
each other, before both are returned. The archive and its acceptance policy
belong to that search design. Pairwise objective ordering does not answer
whether a point is sufficiently different from another, and equal objective
quality does not mean duplicate solutions.

For Pareto search, a future engine can compare a candidate with each retained
frontier member, discard dominated candidates, and remove members dominated by
the candidate. Its pruning must preserve the rest of the frontier. Equivalent
quality need not imply keeping only one assignment; that is an engine policy.
These uses motivate retaining the distinction between equivalence and
incomparability, without adding history arguments, distance results, or an
archive abstraction to this PR.

The nested-cut and equivalent-cut requirements above apply to the present
single-incumbent engines, not to every future consumer of comparison. Before
finalizing the interface, check that direct pairwise calls can support these
uses and that no global assignment or single-incumbent assumption has entered
the Space contract. Do not implement either future engine here.

## Search behavior

Use the same orientation everywhere: compare the incoming solution against the
retained incumbent. Install the first valid incumbent; thereafter replace only
for `SC_BETTER`. Keep the existing incumbent for `SC_EQUIVALENT` and `SC_WORSE`.
This deliberately removes the current replacement of equal solutions at some
call sites; it does not promise a deterministic parallel solution sequence.

| Boundary | Proposed change |
| --- | --- |
| Parallel BAB `solution()` | Compare candidate with global incumbent before cloning, broadcasting, or enqueuing it; never constrain the completed candidate as a comparison probe. |
| Sequential and parallel BAB `constrain()` | Compare an incoming external incumbent before replacing the retained solution; keep the existing pruning of current spaces and recomputation marks. |
| Parallel PBS `CollectBest::add()` and `constrain()` | Select the winner without modifying the retained solution; update `reporter` only when installing a new incumbent. |
| RBS `constrain()` | Compare with `last` before updating it and forwarding the new bound to the master and child engine. |
| Sequential BAB `next()` and sequential PBS scheduling | Keep their existing control flow: sequential BAB already searches under the incumbent cut; sequential PBS forwards incumbents to its assets. |
| Path recomputation, stolen work, worker `better()`, restart `master()` | Continue calling `constrain()` on search spaces. These are pruning operations, not solution arbitration. |

Preserve accepted-solution queue order and ownership. Audit queued parallel BAB
solutions when accepting an external incumbent: no queued result returned after
the update may violate the new incumbent contract. Filtering those pending
solutions is in scope if required; changing queue policy otherwise is not.

`SC_INCOMPARABLE` is expressible for direct model use and a future Pareto engine,
but existing best-solution engines must report unsupported use. Do not silently
treat it as a tie, choose an arbitrary winner, or claim Pareto completeness.
Partial orders can support finding a single maximal solution in a deliberately
designed dominance-chain search, but that is not the current PBS asset protocol.

Missing or unsupported comparison must produce a caller-visible diagnostic,
not normal exhaustion, a hang, or termination of a detached worker thread.
Do not add a capability protocol or routine self-comparison merely to discover
whether a required override exists. Parallel reporting must stop/wake safely
and deliver the error at
the controlling call after workers are quiescent. Nested PBS/RBS/BAB composition
must carry it through worker boundaries. Limit this work to comparison failures;
do not turn it into general exception-safety refactoring.

## Standard objectives and migration

Implement direct scalar and lexicographic integer comparison in all four
MiniModel integer optimization bases, plus integer optimization in
`FlatZincSpace`. Compare values with relational operators, not subtraction.
Respect lexicographic order and existing vector-length semantics; validate
compatible dimensions rather than accidentally comparing different objectives.
Models overriding an inherited `constrain()` with a different objective must
also override its inherited comparison.

Settled Gecode 7 direction: optimization spaces may be required to supply
comparison; no legacy clone/constrain arbitration fallback is needed. The
ordinary integer minimization and maximization cases inherit it from the existing
convenience classes. Custom optimization models implement the same contract.
DFS and other satisfaction-only paths do not require it. Document the migration
and ordinary ABI impact of adding a virtual function.

The ordinary sequential BAB loop need not call comparison merely to enforce
this requirement: it already searches under the incumbent cut. Preserving old
non-ordering BAB uses is not a design constraint. Treat `cartesian-heart` as
evidence of a separate diverse-solution use, not a reason for an unavailable
comparison result or a compatibility path. Its all-previous-solutions search
semantics are outside this PR.

### Floating-point recommendation

Separate objective ranking from the improvement step. For the existing float
bases, propose ranking the upper cost bound for minimization and the lower cost
bound for maximization, matching the incumbent endpoint used in their cuts.
Keep the step in `constrain()`, with a common step policy across assets. Compare
assigned float variables using Gecode's meaning of assigned, which can include
an interval between adjacent representable values.

This is a bound-quality ordering, not a claim to know the exact real value
inside an interval. It intentionally allows retaining a better concurrent
candidate whose improvement is smaller than the step. Equivalence uses equal
ranking keys; being within a step is not equivalence. The latter would be
non-transitive. Approximate pruning needs the subset property above, but need
not admit every solution ranked better.

Before implementing this choice, reconcile it explicitly with FlatZinc's
interval-valued float threshold, strict relations, rounding, and exact threshold
boundaries. In particular, equal upper bounds do not necessarily give equivalent
FlatZinc minimization cuts when the lower bounds differ. Its comparator needs a
cut-compatible key; do not copy the MiniModel endpoint rule without resolving
this difference. Preserve the posted improvement constraints unless the user
approves a change. If the step must instead hold between reported solutions, this
ordering alone is insufficient: a separate admission policy or a bounded legacy
float path must be chosen. Do not quietly redefine comparison as a tolerance
test. Float behavior is a material open decision, not an implementation detail.

## Open questions

1. When a space lacks enough information for a requested comparison, should it
   report a precondition violation or return a separate undetermined result?
   Recommend the precondition for the present search consumers. In either case,
   the individual space decides what information suffices; assignment is not
   a generic requirement.
2. Adopt float bound ranking with step-based pruning, allowing improvements
   smaller than the step among concurrent results, or preserve the reporting
   step through an additional policy? Recommend the former, subject to focused
   interval-boundary checks.
3. Confirm the proposed partial-order result with rejection in existing engines.
   A Pareto-front engine is separate work; if it is required here, reshape the
   area before splitting implementation tasks.

The Gecode 7 requirement, model-defined comparability, and advance planning
without implementation of diverse-solution search are settled.
The choices listed here remain recommendations, not settled user decisions.

## Testing

For this exploration: existing structural checks only. No production code or
tests change, and no build is needed to validate a planning artifact.

For implementation: focused coverage in the existing search test suite, then
the existing search regressions. The observable risks justify these cases:

- Direct scalar, lexicographic, and custom-objective comparisons: direction,
  equal quality, model-defined insufficient information, and unchanged inputs.
  Include a valid model comparison with unassigned variables, such as a
  decisive lexicographic prefix, to guard against generic assignment checks.
- Better/equal/worse external incumbent updates, including an older incumbent
  arriving late and pending parallel results. Returned results must not regress.
- A small custom non-scalar objective through BAB, PBS, and a portfolio with an
  RBS/BAB asset. Check improvement and the known optimum without requiring a
  fixed thread schedule or solution count. Reuse existing engine test patterns.
- Missing comparison and an incomparable result must fail cleanly in a direct
  and a nested parallel path. A focused termination check is necessary because
  the current thread runner has no exception forwarding.
- If float ranking is accepted: zero/nonzero step, improvement smaller than the
  step, the strict threshold, and tight non-singleton intervals in MiniModel
  and FlatZinc. Check the agreed ranking/pruning distinction explicitly.
- Compile the ordering and Gist display overloads together in a representative
  script; a model overriding one overload must not hide the other from Gist.

Keep ordinary sequential optimization covered by existing tests. Do not
multiply all new cases across the existing
large recomputation/thread matrix. Do not add a harness, general property-testing
layer, timing assertions, or benchmarks merely to justify cheap comparison.

## Validation and completion

The shaping deliverable is this brief, its indexed research note, and a passing
`zdev check comparison --format json`. No tasks are imported during exploration.

Implementation is complete when every incumbent arbitration site above uses
the agreed contract, standard objectives and migration behavior are covered,
unsupported outcomes reach callers safely, and the focused plus existing search
checks pass. Include documentation of objective readiness, ties, incomparability,
and `compare()`/`constrain()` consistency.

The repository provides CMake's `gecode-test` target and a `-test` prefix filter.
For example, configure a build under `build/comparison` with `BUILD_TESTING=ON`,
build `gecode-test`, then run `build/comparison/gecode-test -test '^Search::'
-iter 2 -threads 1`. The final flag serializes test runners; individual tests
still exercise parallel search. Retain normal variable features and run the
relevant FlatZinc checks if its comparison changes. Use the existing build/CI
setup for optional dependencies; do not invent a second test configuration.

## Background

- [Comparison and search analysis](background/comparison-and-search.md): source
  map, the limits of constrain-based comparison, ordering/cut requirements,
  Pareto and float counterexamples, interface alternatives, and implementation
  seams. This brief remains the authoritative proposal.
