# Search extensions and the next TSPTW study

Read before planning additional engines, state pruning, or TSPTW measurements.
This is a design recommendation, not a Gecode performance result. On
2026-10-07 CPKB's concepts, sources, and fulltext index was fresh. Its DIDP
algorithm, modelling/dominance, CABS, didp-rs, and RPID pages informed this
comparison; original implementation and paper passages were checked below.

## Which engines to add

[Kuroiwa and Beck's anytime study](https://doi.org/10.1609/icaps.v33i1.27201)
compares DFBnB, CBFS, ACPS, APPS, DBDFS, and CABS. Their different storage and
expansion policies are useful choices, rather than six mandatory ports.

| Candidate | Fit for Gecode | Recommendation for this bundle |
| --- | --- | --- |
| Guided DFBnB | BAB already supplies exact depth-first optimization and recomputation. Model-specific value ordering can provide a useful baseline. DIDP DFBnB additionally retains a dominance registry, so its memory claim differs. | Use BAB with deliberate branching; defer a separate bound-ordered successor engine until it adds measured value. |
| CBFS / ACPS | Depth queues retain deferred alternatives. A progressive quota changes the balance between diving and revisiting shallow choices, without beam restarts. Cloned Spaces make the retained frontier expensive. | Implement one column-search engine: quota one with no growth is CBFS; the default grows quota by one each cycle. Keep the admissible bound separate from ranking. |
| APPS | Packs and suspend lists offer another anytime order, with much of the same retained-Space cost as column search. | Defer until column-search results reveal a distinct unmet need. |
| DBDFS / discrepancy search | Gecode already has LDS and brancher preferences. A discrepancy band over complete transition alternatives is not automatically the same as its binary decision discrepancy. | Use existing LDS only where its semantics fit; defer a new engine. |
| Weighted A* | Current A* uses rank only for equal bounds; this cannot implement weighted expansion. A total CP bound does not expose a unique prefix/remaining decomposition. | Defer. Any later weighted/greedy expansion must use a separate rank and retain admissible pruning; it must specify whether it returns bounded-suboptimal or eventually exact results. |
| BSS, BULB, BUDDS | Fixed-width stacks avoid CABS's widening frontier, at the cost of regeneration. This matters when every node is a Space. | Investigate BSS first; do not implement all three before demonstrating sound regeneration and useful storage savings. |
| LNBS | Path-segment repair is particularly relevant to TSPTW and existing portfolio LNS recipes. | Investigate a small suffix-compatible repair journey; defer a new universal LNBS engine or bandit policy. |
| Parallel, GPU, learned ranking, relaxed DD search | Require additional concurrency, representation, inference, or relaxation contracts. | Outside this bundle. |

The [DIDPPy solver-selection guide](https://didppy.readthedocs.io/en/stable/solver-selection.html)
recommends CABS generally and identifies ACPS/APPS and LNBS as useful alternatives
with different proof, quality, and memory tradeoffs. This is guidance for its
models, not evidence of a Gecode speedup. Its
[ACPS API](https://didppy.readthedocs.io/en/stable/_autosummary/didppy.ACPS.html)
exposes initial quota, growth, maximum quota, and reset. We need only initial
quota and checked growth, with a retained-frontier resource cap; do not copy
the entire options surface. A finite quota never discards pending alternatives.

The [CP 2026 linear-memory paper](https://doi.org/10.4230/LIPIcs.CP.2026.12)
uses a border state and total bound/lexicographic order to regenerate omitted
successors (local fulltext lines 244–280). Its O(Bd) bound counts stored nodes,
with fixed width B and depth d, not bytes. Generic CP domains, weakly monotonic
propagation, new incumbent cuts, and changed choice enumerations need their own
coverage argument. A retained original Choice cursor and stable source clone
may offer a Gecode alternative to the paper's concrete-state border. The study
must include ties, n-ary alternatives, variable-depth goals, and resumption.

## LNBS repair study

[Large Neighborhood Beam Search](https://doi.org/10.4230/LIPIcs.CP.2023.23)
retains a prefix and suffix while repairing the intervening path. Successful
suffix rollout requires executable suffix transitions and a valid complete
solution, rather than the original boundary resource values. A changed arrival
time or prefix cost can therefore be allowed when the suffix remains feasible.
The complete adaptive algorithm also searches the full path eventually; a
single local repair does not supply that global proof.

Read CPKB's
`wiki/sources/2023-kuroiwa-large-neighborhood-beam-search-for-domain-indepe.md`
and its original
`sources/derived/papers/2023-kuroiwa-large-neighborhood-beam-search-for-domain-indepe/fulltext.md`,
especially suffix rollout, before the study. Start from
`SpacePortfolioLnsRecipe` in `gecode/search/portfolio.hpp` for the existing
neighborhood construction and inner-search lifecycle. The proposed study uses
one fixed segment rule and checks the whole repaired tour; adaptive neighborhood
selection and production LNBS integration require a later decision.

## Guidance, equality, dominance, and merging

Keep `Search::HeuristicGuidance` as the read-only total-bound/rank contract.
For a closed CP model use its propagated objective minimum/maximum. For an
additive transition model explicitly calculate g+h; do not add g to a bound
that already includes it. Cache expensive estimates once per admitted stable
node where useful, without imposing state functions on all models.

Optional state pruning has a separate declaration. Extract a small owned
descriptor from the stable Space: an exactly comparable grouping key and the
labels needed for a cost-aware relation. Hashing only selects a bucket; key
equality must resolve collisions. The relation distinguishes equivalence,
dominance in either direction, and incomparability. Missing declaration means
ordinary tree search, with no registry or descriptor allocation. The model
author is responsible for sufficient state and a sound relation; an integer
resource name or a common output prefix proves neither.

For TSPTW group by unvisited set U and current vertex i. Retain Pareto labels
(t,g): t1<=t2 AND g1<=g2 can justify pruning label 2 when waiting is allowed.
Earlier time at greater cost remains incomparable. The path is retained for
output, but history affecting future constraints must also enter the descriptor.
An ordinary CP model with partially assigned successors does not acquire this
state key merely because it is also a TSPTW formulation.

The registry owns descriptors, not extra Space clones. A* and column search
must admit a new dominating representative before invalidating old pending
work, and reopen when a better label is reached after expansion; admissible
inconsistent bounds remain legal. Initially CABS compares only live next-layer
candidates, with registry entries removed when their representative leaves
that candidate beam and cleared between layers and passes. A representative
discarded by a beam cannot justify deleting the surviving route to the same
residual problem. Record beam loss independently of sound label pruning.
Keep one representative for equivalent labels; never delete both. Forgetting
registry information may cause repeated work but cannot discard search work.

This is exact representative retention and Pareto pruning. It does not merge
two CP stores into a relaxed state. Relaxed decision-diagram merging requires
a separate relaxation operator and bound argument and is deferred. Likewise,
[transition dominance](https://doi.org/10.4230/LIPIcs.CP.2025.5) concerns choices
at one state, including immediate cost, rather than registry labels. Keep any
TSPTW-specific valid rule in its transition brancher; do not add a generic
SCC/rule language in this increment.

Model may declare descriptor dependencies using its existing guidance retention
and read-context conventions. It resolves them in each evaluated clone and
exports the same native callbacks; no Model prerequisite, Space virtual
method, general analyzer, or portfolio registry is needed. Existing objective
and functional-dependency metadata can support explicit branching suggestions,
but cannot automatically infer an admissible estimate or a sufficient state key.

## Inspected implementation references

The registered didp-rs checkout is v0.11.1,
`59a5330537962bcc2f93c3914cf5fd9ee361b23d`. This research read:

- [`StateRegistry::remove_dominated`, lines 171–235](https://github.com/domain-independent-dp/didp-rs/blob/59a5330537962bcc2f93c3914cf5fd9ee361b23d/dypdl-heuristic-search/src/search_algorithm/data_structure/state_registry.rs#L171-L235): residual preference and prefix cost are both compared.
- [`Acps::search_next`, lines 270–420](https://github.com/domain-independent-dp/didp-rs/blob/59a5330537962bcc2f93c3914cf5fd9ee361b23d/dypdl-heuristic-search/src/search_algorithm/acps.rs#L270-L420): depth queues, retained successors, resumable solution return, and progressive cycles.
- [`beam_search`, lines 136–230 and 310–332](https://github.com/domain-independent-dp/didp-rs/blob/59a5330537962bcc2f93c3914cf5fd9ee361b23d/dypdl-heuristic-search/src/search_algorithm/beam_search.rs#L136-L230): layer registry handling and loss/proof distinction.

RPID v0.4.0's
[`Dominance` interface, lines 205–233](https://github.com/domain-independent-dp/rpid/blob/81e14e055cc231705fe988f58c52b55386e424f9/src/dp.rs#L205-L233)
separates a hashable key from a partial comparison. Its default equal-key
equivalence is an assumption we should make explicit rather than silently
apply to resource labels.

## TSPTW data and comparison

Use the [López-Ibáñez/Blum canonical matrices](https://lopez-ibanez.eu/tsptw-instances),
starting with Dumas, GendreauDumasExtended, SolomonPotvinBengio, and AFG. Preserve
matrix decimals by exact integer scaling. Do not regenerate distances from
coordinates or add service twice when it is already included in an arc.
Minimize the sum of the supplied arc values, including return; waiting affects
feasibility but not this objective. Use fixed departure at time zero, waiting
to customer ready times, and the depot window on return, matching the collection's
checker. Nonzero start-depot ready times are rejected in this initial adapter.
The publisher's `check_solution.cpp` lines 256–287 were read to establish these
conventions. Nonnegative asymmetric and nonmetric matrices remain supported.
Checked scaling and Gecode integer limits are input constraints, not silent
rounding or saturation.

Keep a small checked-in correctness set and a deterministic acquisition manifest
for a larger development/held-out selection. Independently check published
tours and distinguish a best-known cost from a proven optimum. Include tight,
loose, and infeasible cases. Tiny nonmetric, service-folded, waiting, return-window,
and decimal cases expose semantic errors; a few deterministic window variants
extend workload diversity and are reported separately from canonical data.

[Soulignac's 2026 benchmark analysis](https://arxiv.org/abs/2512.01064v2)
warns that many large classical instances are easy for makespan/duration
objectives and identifies harder loose-window cases. Its conclusion does not
prove the same result for travel-cost optimization. It does justify avoiding
size alone as a difficulty measure and keeping objective conventions explicit.
Do not compare travel-cost runs with makespan best-known values.

The closed CP baseline starts from `examples/tsp.cpp`'s costed circuit, adds
customer times and separate start/return depot handling, and uses strong
existing propagation and deliberate branching. Its reversal symmetry break is
not valid for general asymmetric/windowed tours. BAB, then a bounded comparison
of propagation/branching and existing LNS facilities, supplies a serious Gecode
baseline. Do not hold it artificially to the transition model's brancher.

The transition Space uses finite clone-local (U,i,t,g), a lazily grown native
open visit list, and a problem-specific n-ary brancher. A commit means one
assigned visit, with an explicit final return. Materialization is not a visit.
Use the existing prepare protocol when adding variables before a choice. An
eager path-variable control with the same transition decisions isolates list
storage; the conventional CP model is a separate formulation comparison.

Measure native Gecode against a pinned DIDPPy transition formulation using the
same exact inputs and checker. Start with cheap incoming/outgoing bounds;
stronger scheduling relaxations must repay their propagation/clone costs.
Record elapsed time including startup/preprocessing, quality over time, proof
and stop status, peak process RSS, frontier/descriptor counts, and sampled
clone footprint. Space count is not DP-state byte cost. Use a small named,
resumable runner following existing `misc/` benchmark conventions, without
importing the unrelated FlatZinc collection pipeline wholesale.
