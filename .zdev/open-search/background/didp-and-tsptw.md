# DIDP search and a TSPTW starting model

Read this before settling search semantics, state pruning, or evaluation. This
is a planning synthesis, not a new benchmark result. CPKB's complete concepts,
sources, and fulltext index was checked fresh on 2026-10-07. The principal topic
and source pages were reviewed in the KB on that date. Primary paper passages
and the didp-rs code ranges below were also read for this plan.

## Search semantics that transfer to Gecode

DIDP exposes a residual state, transitions, terminal cases, state constraints,
resource preferences, and admissible remaining-cost bounds. A* can order by a
bound on total solution cost. Resource dominance must also account for the cost
of reaching the state. A good ranking score alone cannot justify deleting a
candidate. These are mathematical model obligations, not properties inferred
from a variable or callback name.

Kuroiwa and Beck's original formulation supplies the modeling starting point:
[ICAPS 2023 foundational paper](https://doi.org/10.1609/icaps.v33i1.27200).
Its original cost-algebraic A* returns an optimal first solution under its stated
assumptions. That guarantee should not be transferred automatically to arbitrary
CP ranking callbacks, custom objectives, or broader state models.

The [2023 anytime paper](https://doi.org/10.1609/icaps.v33i1.27201), section
"Complete Anytime Beam Search" and Algorithm 2, restarts a layered beam with
widths 1, 2, 4, ... and retains the incumbent. It makes beam loss explicit.
A narrow run can miss an optimum or all solutions. Early return after an
improvement also leaves work to recover, even if no width truncation occurred.
This suggests explicit tracking of unsearched improving alternatives in the
engine, rather than deriving proof from frontier emptiness alone.

The [2026 formal treatment](https://arxiv.org/abs/2401.13883v5), Algorithms 2–3
and Theorems 16–20, establishes termination for finite, acyclic models under
its cost assumptions and separates a complete iteration from early solution
return. Its global dual bound includes the bounds of discarded beam states.
A bound over only surviving states can be too high. Increasing beam width alone
does not make an unbounded cyclic model terminate; cross-layer handling requires
additional assumptions. Use finite bounded examples for the first Gecode slice.

For ordinary CP, queued nodes represent domains and propagator state, rather
than necessarily concrete DP states. One engine should support tree search
without requiring an equivalence key. Optional graph pruning requires a
sufficient residual-state contract. Do not use a hash value as state identity
without equality handling, or merge states merely because selected output
variables match. Whether inconsistent admissible estimates require reopening
must be settled alongside the precise graph-pruning contract.

## Inspected implementation reference

The KB registers didp-rs v0.11.1 at
`59a5330537962bcc2f93c3914cf5fd9ee361b23d`. The following exact ranges were read
from the local checkout at that commit:

- [`Cabs::search_inner`, cabs.rs, 178–273](https://github.com/domain-independent-dp/didp-rs/blob/59a5330537962bcc2f93c3914cf5fd9ee361b23d/dypdl-heuristic-search/src/search_algorithm/cabs.rs#L178-L273): incumbent/bound updates, width growth, and separate width/time termination.
- [`beam_search`, beam_search.rs, 237–336](https://github.com/domain-independent-dp/didp-rs/blob/59a5330537962bcc2f93c3914cf5fd9ee361b23d/dypdl-heuristic-search/src/search_algorithm/beam_search.rs#L237-L336): tracks truncated/replaced beam nodes, their bounds, and proof separately. The registry is cleared between layers unless all-layer retention is selected.

These are design references for mechanisms, not timings or proof that a Gecode
port would have identical behavior. The papers use separate historical versions.
RPID's programmable model interface is another useful API comparison; its
registered v0.4.0 source reading path is in the KB system page below. RPID code
was not independently reviewed during this setup.

## TSPTW transition model

Use a state `(U, i, t)`: unvisited customers, current location, and current
service/arrival time after permitted waiting. Keep prefix travel cost `g`
separate from `t`; waiting increases time but not the travel objective.

A visit to `j in U` is applicable when `t + travel[i,j] <= due[j]`, with valid
input windows. It removes `j`, sets the location to `j`, advances time to
`max(t + travel[i,j], ready[j])`, and adds `travel[i,j]` to cost. At `U = {}`,
include the return-to-depot travel cost, either as a terminal cost or explicit
return transition. Decide depot-window and service-duration conventions from
the chosen data before implementing; keep the checker and all baselines equal.
The fundamental [2023 paper](https://doi.org/10.1609/icaps.v33i1.27200),
"Example: TSPTW", gives this residual formulation.

For equal `(U,i)`, label `(t1,g1)` can dominate `(t2,g2)` when both
`t1 <= t2` and `g1 <= g2`, provided the formulation permits waiting and earlier
arrival cannot harm future choices. Earlier arrival reached at higher cost is
not sufficient: retain the Pareto-incomparable labels. Path output belongs to
reconstruction, while all history that affects future constraints belongs in
the state/equivalence contract.

Start with a zero remaining-cost bound to check semantics. Then compare cheap
minimum incoming/outgoing travel bounds: every completion must enter every
remaining customer and the depot, and leave the current location and remaining
customers. Each sum is a lower bound when its candidate arcs include every
possible completion; their maximum can be stronger. Account for the root and
terminal cases explicitly. For reachability pruning on nonmetric data, use
shortest-path travel lower bounds; direct travel can overestimate reachability
when intermediate visits are possible. Do not assume symmetry or a triangle
inequality from the name TSPTW.

The open formulation should grow a visit/action list lazily and keep the
residual state clone-local. The current Model open-list transition callback is
called when a position is materialized; this is not automatically one committed
visit. Determine where assigned visit effects occur and how they relate to
CABS layers. TSPTW has a finite visit count, so using open construction here
means avoiding eager decision/state allocation, not inventing an unbounded tour.
An automaton for every subset would defeat the intended state representation.

## Propagation and measurement

[Marijnissen et al., DIDP with constraint propagation](https://arxiv.org/abs/2603.16648v2)
adds residual CP inference inside model functions while retaining A* and CABS.
It tests failures, filters next actions, and combines DP/CP lower bounds by
maximum. Its TSPTW propagation model is a scheduling relaxation. It reports
benefits for tightly constrained cases and overhead on looser cases. The
publisher text uses one pass per state and reports that fixpoint propagation
was too costly for that setup; Gecode's ordinary `status()` fixpoint behavior
makes this a policy question to measure, not a setting to copy.

The [RPID paper](https://doi.org/10.4230/LIPIcs.CP.2025.23) is a lead for
compiled state/transition functions and stronger native bounds.
The KB source page reports that a stronger 1-tree bound reduces expansions but
hurts TSPTW runtime/coverage in that experiment. Begin with inexpensive bounds,
then measure evaluation time and state storage before adding stronger ones.

Compare engines on one formulation first. Compare open transition and closed
CP formulations separately. Include loose, tight, and infeasible windows,
several sizes, and at least one existing planning and one closed-model example.
Use tiny exhaustive references for correctness; use development and held-out
benchmark subsets for tuning. Report wall time, quality over time, proof status,
peak memory, expansions, and relevant propagation/cloning costs. Paper results
motivate the experiments; they do not establish a Gecode speedup.

## CPKB reading paths

Resolve these paths under `/Users/zayenz/lab/knowledge/cpkb`. Start with the
concept pages, then the relevant source and fulltext passages. Keep the KB
read-only for this work.

- `wiki/topics/domain-independent-dynamic-programming.md`
- `wiki/topics/domain-independent-dynamic-programming/modelling-and-dominance.md`
- `wiki/topics/domain-independent-dynamic-programming/search-algorithms.md`
- `wiki/topics/domain-independent-dynamic-programming/results-and-comparisons.md`
- `wiki/topics/search/complete-anytime-beam-search.md`
- `wiki/systems/didp-rs.md` and `wiki/systems/rpid.md`
- `wiki/sources/2023-kuroiwa-domain-independent-dynamic-programming-generic-s.md`
- `wiki/sources/2023-kuroiwa-solving-domain-independent-dynamic-programming-p.md`
- `wiki/sources/2026-kuroiwa-domain-independent-dynamic-programming.md`
- `wiki/sources/2025-kuroiwa-rpid-rust-programmable-interface-for-domain-inde.md`
- `wiki/sources/2026-marijnissen-domain-independent-dynamic-programming-with-cons.md`

Canonical bibliographic fields and original extracted passages live at
`sources/derived/papers/<paper-id>/metadata.yaml` and `fulltext.md`.
The didp-rs YAML TSPTW example is a useful formulation comparison at
`sources/code/didp-rs/didp-yaml/examples/tsptw/tsptw-domain.yaml`; it has an
explicit return action. Check its travel-matrix assumptions before reusing its
reachability condition.
