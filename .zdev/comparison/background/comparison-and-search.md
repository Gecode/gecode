# Comparison and best-solution search

This note records the analysis behind the [comparison brief](../brief.md).
Repository observations refer to `main` at
`6b7de57b0414fe2f8af3513743c9835840a3c019`. Symbol names are the primary source
anchors; line numbers may move during implementation.

## What is missing today?

[`Space::constrain()`](../../../gecode/kernel/core.hpp) specifies a restriction
to solutions better than an incumbent. The default implementation does nothing
in [`core.cpp`](../../../gecode/kernel/core.cpp). It is a mutation of a search
space, not a query about two solutions.

The implementation already uses it for both purposes:

| Source and symbol | Current arbitration |
| --- | --- |
| [`search/par/bab.hpp`](../../../gecode/search/par/bab.hpp), `BAB::solution` | Constrain the candidate by the incumbent; discard it if propagation fails. |
| Same file, `BAB::constrain` | Constrain the retained incumbent by the incoming one; retain the old one if propagation does not fail. |
| [`search/seq/bab.hpp`](../../../gecode/search/seq/bab.hpp), `BAB::constrain` | The same reverse-direction test, charging propagation through the worker statistics. |
| [`search/par/pbs.hpp`](../../../gecode/search/par/pbs.hpp), `CollectBest::add` / `constrain` | Constrain the retained incumbent by the candidate; replace it if propagation fails. |
| [`search/seq/rbs.cpp`](../../../gecode/search/seq/rbs.cpp), `RBS::constrain` | Apply the reverse-direction test to `last`, then update master and child engine. |

For a strict total objective on fully determined costs, the forward test accepts
strict improvements. The reverse test also accepts ties: the old solution cannot
be strictly better than an equal candidate. Thus current arbitration is not
uniform even for simple objectives.

With a custom constraint, propagation need not decide satisfiability. Posting
may add auxiliaries or branchers, change the supposedly completed solution, or
leave an undecided relation. Two clone/constrain/status probes do not repair
that generally: failure is useful evidence, but non-failure is not a proof of
the converse ordering. Full search could decide some such questions, at a cost
and with a contract quite different from comparing objective values.

The expected gain is removal of this posting/propagation work during arbitration,
including work done under a central mutex. It does not remove solution ownership
clones, worker incumbent copies, pruning, or recomputation. No measured speedup
is claimed.

## Not every constrain method defines an order

[`car-sequencing.cpp`](../../../examples/car-sequencing.cpp),
`CarSequencing::constrain`, is an ordinary scalar objective: reduce `nstall`.
The balanced objective in [`test/search.cpp`](../../../test/search.cpp),
`HasSolutions::constrain`, compares an absolute difference of sums. Both have
simple read-only comparisons without requiring one particular cost variable
representation.

[`cartesian-heart.cpp`](../../../examples/cartesian-heart.cpp),
`CartesianHeart::constrain`, seeks a next point sufficiently separated in selected
directions. Its disjunction allows either direction of movement in the second
coordinate; sufficiently separated points can satisfy both directed tests.
This is not an asymmetric improvement order, yet the example uses sequential BAB.
It represents a different search contract: diversity must account for all
previous solutions, not merely the latest incumbent. Gecode 7 may impose new
requirements on optimization spaces, so preserving this use through an
unavailable comparison result or legacy arbitration path is not required.

Other limits follow from the same distinction: diversity enumeration, an
incumbent-dependent neighborhood, changing objective weights, or a policy that
depends on search history may not have a fixed transitive ranking. Some can be
reformulated with fixed policy state; the engine must not infer that reformulation.

## Non-fixed spaces

[`Space::status()`](../../../gecode/kernel/core.cpp) returns `SS_SOLVED` when
no brancher has alternatives left. It does not inspect every variable for
assignment. This is why a generic solved-space check cannot validate cost data.
Calling it inside a const comparison would also introduce propagation.

The individual space defines what information suffices to establish a relation.
Even requiring all objective components to be assigned would be too strong as
a generic rule: a lexicographic model can establish a strict ordering from a
decisive prefix, regardless of the remaining components. A derived model property
can likewise be comparable without a stored, assigned objective variable.
Comparability may depend on the pair, not on a unary readiness flag.

For scalar integer convenience classes, the existing value accessor is a natural
implementation:
[`IntVar::val()`](../../../gecode/int/var/int.hpp) explicitly throws
`Int::ValOfUnassignedVar` when its variable is unassigned. Integer convenience
`constrain()` methods use `val()` on the incumbent objective. Scalar comparison
can use it on both costs. This is that class's contract, not an assignment
requirement on all implementations of `Space::compare()`.

An explicit undetermined result is a viable alternative for a caller intending
to inspect partial objectives and recover without an exception. In the existing
search consumers, however, inability to establish a required comparison is
still an error:
discarding it may lose a valid improvement, accepting it cannot establish
betterness, and calling `status()` to resolve it violates the comparison
contract. Without such a caller, this extra result adds no useful search
behavior. How to report insufficient information remains an open decision;
a model-defined precondition is the recommendation. A generic assignment or
`SS_SOLVED` test must not stand in for the model's judgment.

Comparing arbitrary domains asks a different question. For example, minimization
domains `[1,4]` and `[3,6]` overlap: comparing minima ranks bounds, not the eventual
solutions. Some disjoint domains admit a guaranteed relation, and an individual
model may report that relation when sound under its contract. No generic domain
comparison machinery is needed. Distinguish insufficient information from true
incomparability and leave propagation-based pruning in `constrain()`.

## Advance planning for other search policies

The compatibility check for this PR is whether the pairwise interface remains
usable when the engine retains more than one solution. It should: `compare(a,b)`
describes a model-defined relation, not whether either argument is the one
global incumbent. The current single-incumbent engines impose additional cut
nesting requirements; those must not be presented as requirements of all future
search policies.

In diverse-solution search, each accepted solution can contribute a restriction
against accepting similar future solutions. If these restrictions accumulate
conjunctively, repeated model-specific calls can express them without a batch
interface. What changes is engine state: an archive of relevant solutions or
restrictions, replay during recomputation, and delivery of all restrictions to
assets. A worker that has seen only the latest accepted point is not generally
up to date. Two concurrently produced points may each differ from the old archive
but fail the diversity requirement with respect to each other, so acceptance
also needs coordination against the current archive.

Ordering cannot substitute for a diversity relation: two spaces with equivalent
cost can be very different solutions. Conversely, adding distance or history to
the ordering return type would force unrelated policies together. Leave a future
diversity admission/restriction contract separate; no new methods are justified
in this PR merely to reserve names.

A Pareto engine can reuse pairwise comparison against its frontier, with its own
rules for dominated, incomparable, and equivalent candidates. It also needs
frontier-preserving pruning and explicit output semantics when a later solution
dominates an earlier one. Objective equivalence does not itself choose whether
to retain one or many assignments. This is why the comparison result and the
engine's retention policy should remain distinct.

These are design checks, not implementation commitments. They justify keeping
comparison pairwise, model-defined, and independent of solution-history storage;
they do not require a new archive class or engine now.

## Ordering and nested improvement restrictions

A total ordering of objective equivalence classes is the useful common case:
scalar, lexicographic, fixed weighted sums, and fixed hierarchical criteria.
Two different assignments can be equivalent. Replacing an incumbent only on a
strict improvement avoids equal-result churn, but equivalence must also mean
that the two incumbent cuts are interchangeable.

Let `F(b)` denote the set of feasible objective-complete solutions allowed by
the restriction posted by `constrain(b)`. For exact ordinary optimization,
`F(b) = {x | x is better than b}`. Transitivity then gives
`F(a) subset F(b)` whenever `a` is better than `b`.

The search code depends on this nesting: workers can have older cuts, accept
new cuts, and retain local incumbents. An asset that has already explored or
pruned a region cannot recover it merely because a new comparison method exists.
For stepped or otherwise approximate optimization, the equality above can be
relaxed, but cut nesting and a clearly stated weaker optimality promise remain
necessary.

The four ordering outcomes follow the familiar partial-order distinction between
less, equivalent, greater, and unordered in the
[C++ comparison specification](https://eel.is/c++draft/cmp.partialord).
Gecode's direction should be objective-relative (`BETTER` / `WORSE`) rather than
numeric. The project's CMake library targets currently require `cxx_std_17`, so
using `std::partial_ordering` would unnecessarily couple this change to a language
upgrade. A small enum is adequate. Undetermined objective data and an absent
comparison implementation are outside this ordering; the current recommendation
reports them as errors rather than adding a fifth relation.

## Why incomparability is more than a collector policy

For two minimization objectives, `(1,5)` and `(5,1)` are incomparable under
Pareto dominance. Neither is an equal-quality substitute for the other. A full
Pareto engine needs an incumbent set, cuts excluding dominated regions without
excluding other frontier points, and a policy for withdrawing previously emitted
points if a later point dominates them. A comparator is useful infrastructure
but does not provide any of this.

Even a promise of finding just one maximal solution needs care with the existing
parallel portfolio protocol. Consider four feasible points:

| Point | Objectives | Role |
| --- | --- | --- |
| A | `(6,6)` | An old shared incumbent. |
| B | `(5,1)` | One asset's new local incumbent. |
| C | `(1,5)` | The concurrently retained global incumbent. |
| D | `(0,4)` | A feasible improvement of C that does not dominate B. |

B and C both dominate A but not each other. An asset already constrained to
dominate B can exclude D. Adding the cut for C does not restore D. If the
collector discards B as an incomparable candidate and that asset subsequently
exhausts, its exhaustion does not prove C maximal. In
[`Par::PBS::report`](../../../gecode/search/par/pbs.hpp), an exhausted asset
stops the current run; `next()` can then return no solution without reporting a
limit stop. Treating incomparability as a tie would therefore overstate what
the portfolio has established.

A purpose-built dominance-chain engine can return one maximal point without
enumerating a frontier. That possibility does not justify silently applying
the existing total-objective portfolio protocol to partial orders. Recommend
representing incomparability and rejecting it in these engines for this PR.

## Float steps expose a second policy

The six objective bases in
[`minimodel/optimize.cpp`](../../../gecode/minimodel/optimize.cpp) include
integer scalar and lexicographic minimization/maximization, and two float bases.
The float cuts use `best.cost().max() - step` for minimization and
`best.cost().min() + step` for maximization, with strict relations.

[`FloatVarImp::assigned()`](../../../gecode/float/var-imp/float.hpp) delegates
to [`FloatVal::tight()`](../../../gecode/float/val.hpp), which accepts a
singleton or adjacent representable endpoints. An assigned float is not always
an exact real singleton. Using an undocumented midpoint would introduce a new
policy without respecting the current endpoint-based bounds.

With exact singleton costs, let an incumbent cost be 10, a candidate cost be
9.5, and step be 1. The candidate is numerically better, but it does not satisfy
the incumbent's `< 9` improvement restriction. The current forward BAB probe
rejects it. The current reverse PBS probe constrains 10 to `< 8.5`, fails, and
accepts 9.5. There is already a policy difference between these paths; it cannot
be preserved by one uniform better/equal/worse test.

Treating values within a step as equivalent does not solve this. With step 1,
costs 0 and 0.75 would be equivalent, as would 0.75 and 1.5, but 0 and 1.5 would
not. Equivalence in a weak ordering must be transitive; see the
[C++ strict weak order requirements](https://eel.is/c++draft/concept.strictweakorder).
Such a tolerance relation must not be advertised as ordinary equality.

Recommend comparing conservative endpoint quality independently of the step,
while keeping step-based pruning. A concurrent sub-step improvement can then
be retained, and applying its threshold still tightens future search under a
common step policy. This makes ranking uniform, but changes which results may
be reported. Preserve a reporting step only through an explicitly separate
admission policy, if that behavior is required.

FlatZinc is a required follow-through, not an integer-only afterthought:
[`FlatZincSpace::constrain()`](../../../gecode/flatzinc/flatzinc.cpp) uses an
interval-valued `val() +/- step` threshold for floats, rather than exactly the
MiniModel scalar endpoint expression. Its rounding and boundary behavior need
focused checks before the recommendation becomes an implementation decision.
The relevant relation posting is in
[`float/rel.cpp`](../../../gecode/float/rel.cpp); strict relations combine
a bound restriction with disequality. Do not rewrite them as an assumed
non-strict minimum-step comparison.

There is a concrete reason not to copy the MiniModel key mechanically:
[`NqFloat::post()`](../../../gecode/float/rel/nq.hpp) rejects an assigned
interval overlapping the threshold interval. For minimization at step zero,
an incumbent `[l,u]` with adjacent endpoints excludes the candidate singleton
`[l,l]` in FlatZinc, whereas a singleton incumbent `[u,u]` admits it. Both
incumbents have the same upper bound. Thus upper-bound equality alone would
claim equivalence between different FlatZinc cuts. The implementation must choose
a key compatible with that cut or obtain agreement to align the float policies.
This is an unresolved detail within the float decision, not evidence against a
model-defined comparison method.

## Interface alternatives and compatibility

| Alternative | Assessment |
| --- | --- |
| `bool better(const Space&) const` | Enough for one total-objective acceptance decision, but cannot distinguish equivalent from incomparable without more calls or methods. |
| Three ordering values only | The smallest total-objective interface, but cannot report the partial-order distinction raised in the proposal. |
| Four ordering values plus an undetermined result | Useful only if callers deliberately compare partial objectives and need a recoverable outcome; existing search would still have to reject such incumbents. |
| Four ordering values with model-defined comparability as a precondition | Recommended: results describe valid comparisons; missing support or insufficient information follows error conventions. A non-pure throwing default can leave satisfaction models unaffected. |
| A separate capability virtual or optimization base | Adds a second piece of model configuration, or moves a Space-level search operation behind another abstraction. Not needed solely for this change. |
| Extract a serialized or type-erased objective key | Useful for distributed search, but unnecessary machinery for two in-process spaces. |
| Infer comparison by clone/constrain/status | Potentially expensive and undecided; no compatibility requirement justifies it for this Gecode 7 change. |

A virtual added to `Space` changes the ABI. A non-pure default avoids source
breakage in every satisfaction model, but cannot magically provide a meaningful
comparison for old custom optimization models. The settled Gecode 7 direction
permits requiring comparison on optimization spaces. Most models inherit scalar
integer comparison from the existing convenience classes; custom objectives
must implement it. No legacy concurrent fallback is needed. This leaves the
readiness contract as a separate choice rather than conflating it with migration.

An inherited comparison also deserves attention: an application may derive from
`IntMinimizeSpace` but override `constrain()` to optimize a different criterion.
It must override comparison as well; the engine cannot validate arbitrary
logical consistency. State this alongside the existing obligation to provide
a sound improvement restriction.

## Integration details worth retaining

- [`seq/bab.hpp`](../../../gecode/search/seq/bab.hpp), `BAB::next`, already
  obtains solutions under the incumbent restriction. Adding comparison to every
  sequential solution is unnecessary for ordinary execution. Not calling it
  there does not require a separate compatibility policy for optimization models.
- [`seq/path.hpp`](../../../gecode/search/seq/path.hpp) and
  [`par/path.hpp`](../../../gecode/search/par/path.hpp) apply the incumbent at
  the recomputation mark. Those calls must remain constraints.
- `Par::BAB::Worker::better` clones the incumbent and constrains current work.
  Const comparison does not make clone/update internals safe for concurrent
  mutation. Preserve the existing synchronization around ownership and cloning.
- `Par::PBS::CollectBest` retains a solution and a `reporter` pointer. Its `get`
  clones that solution; a rejected equal or worse candidate must not overwrite
  `reporter` or trigger a new propagation broadcast.
- [`par/engine.hpp`](../../../gecode/search/par/engine.hpp), `Engine::next`,
  returns already queued solutions before resuming work. External incumbent
  updates need a deliberate check of that pending-result path.
- [`support/thread/thread.cpp`](../../../gecode/support/thread/thread.cpp)
  calls `Runnable::run()` on detached threads without a catch boundary.
  `Par::BAB` and `Par::PBS` also hold manually managed locks around arbitration.
  A default throwing comparison inserted at these points is not a complete
  implementation. Comparison errors need safe reporting and nesting behavior.
- [`driver.hh`](../../../gecode/driver.hh), `Driver::ScriptBase`, already
  declares `compare(const Space&, std::ostream&) const` for Gist display.
  Bring the inherited one-argument overload into scope with a suitable using
  declaration. [`flatzinc.hh`](../../../gecode/flatzinc.hh) has display
  overloads too; retain their behavior and overload visibility. Virtual search
  calls through `Space` must dispatch to the objective comparison. Custom scripts
  overriding the new one-argument method may themselves need `using
  Script::compare` (or their actual base name):
  [`Gist::VarComparator<S>`](../../../gecode/gist/gist.hpp) calls the display
  overload through `S`, where a new derived overload would otherwise hide it.

## Existing validation seams

[`test/search.cpp`](../../../test/search.cpp) has BAB, RBS, PBS, and mixed
search-builder portfolio tests, with recomputation distances, stop/resume, and
thread choices already represented. Extend selected patterns instead of
duplicating the whole matrix. The balanced objective is a useful non-scalar
example; tests that only check a final optimum need focused companions for
tie handling, external updates, and unsupported outcomes.

[`CMakeLists.txt`](../../../CMakeLists.txt) defines `gecode-test`, a CTest
build fixture, and the normal check arguments. Tests are named `Search::...`;
[`test/test.cpp`](../../../test/test.cpp) treats `-test '^Search::'` as a
prefix filter. The `-threads` runner option is separate from search-thread
options inside each test. No new validation framework is needed.
