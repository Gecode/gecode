+++
schema_version = 1
id = "open-search-028"
key = "cheap-tsptw-deadline-filter"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-018"]
+++
# Evaluate a cheap TSPTW residual deadline filter

## Outcome

TSPTW has a sound cheap inference option and a bounded development comparison showing when it is useful between OFF and full unary inference.

## Context

Transition::refresh_inference in examples/tsptw-transition.hh:291–319 offers OFF or a fresh ResidualSchedule and full fixpoint on each committed successor. Individual shortest-path deadline bounds already appear in ResidualSchedule:275–280. Normal held-out summaries report roughly 80% of BAB/CABS search time in inference, with killed runs unmeasured. Marijnissen et al., Domain-Independent Dynamic Programming with Constraint Propagation (doi:10.1609/icaps.v36i1.42826), equations 28–30, use a shortest-path residual dead-end test (CPKB paper 2026-marijnissen-domain-independent-dynamic-programming-with-cons, fulltext 475–504).

## Boundaries

- Add an explicit necessary customer-deadline/depot-return check using shortest paths; keep nonmetric/asymmetric data sound. Do not change Gecode's fixpoint policy.
- Preserve frozen configurations, held-out observations and defaults. Keep bound, ranking, dominance and inference choices independent; compare native engines on the same formulation.
- Before running, select six existing tiny/development cases covering loose, tight and infeasible behavior; use the existing tiny infeasible fixtures as controls. Compare OFF/cheap/UNARY with BAB/CABS, two repetitions and a two-second cap: 144 nominal solver seconds. Cap total launch-through-exit solver work including validation at 300 seconds; report builds separately. No held-out or larger campaign.
- Use fresh task-local admission/accounting for one shared 300-second allowance across implementation and independent verification, recording solver validation, failed/interrupted launches and cleanup. The historical 7200-second ledger and phase reserves do not apply. Reuse the existing runner/capture and saved-record format with the smallest necessary budget extension; preserve historical interpretation.

## Done when

- [x] The cheap mode preserves all feasible completions and uses no residual unary Space.
- [x] The small fixed-case comparison reports checked solutions/proofs, inference time, node counts, stops and missing measurements without claiming a universal speedup.
- [x] Results recommend whether to keep the option and whether a separately scoped incremental unary or fixed-width continuation study is justified; defaults remain unchanged.

## Validation

- Extend the existing residual-completion enumeration with the cheap filter and the nonmetric counterexample, plus waiting/return boundaries already represented in fixtures. Run only the declared bounded comparison using the repaired runner; preserve partial failures and stop when the budget is spent.

## Result

Added sound CHEAP residual deadline inference without a unary Space and completed the fixed 72-row OFF/CHEAP/UNARY BAB/CABS comparison. Retain the explicit option; defaults remain unchanged.

Validation:

- Reviewed all eight source/report paths and the attributed task clarification against the original clean baseline and admitted frozen snapshot. CHEAP uses sound shortest-path customer/return bounds for asymmetric, nonmetric, folded-service and waiting semantics, constructs no residual unary Space, and preserves independent bound/ranking/pruning choices, defaults, frozen configuration arguments, historical ledger interpretation, manifest and held-out reports. Required verification passed: prebuild; cmake check target with 25-second cap; Search::TSPTW::Transition with 3-second cap; inherited open-list, regular/NFA, portfolio and search selection with 15-second cap; four inherited default examples with 1-second caps; six independently checked CHEAP BAB/CABS waiting/late-customer/late-return checks with 2-second caps; 12 Python benchmark tests; git diff --check. Rechecked all 72 saved terminal rows, 476 terminal publications and 3 partial publications; confirmed 49 summaries and 23 timeouts, proof consistency, input hashes, resume/rejection evidence and exact offline regenerated core-report equality. Recommendations and missing-measurement limitations match the evidence. Verification charged 26.873740248091053 seconds, including the reaped sandbox-interrupted attempt; final shared usage is 197.83128824434243 of 300 seconds, with no pending launches. Verification build took 0.11097958299797028 seconds separately. Validation wrote only ignored build evidence and the authorized external ledger; no source, report or .zdev validation writes occurred.
- Verifier evidence: work_context_snapshot: Wcafb228c60862ab9. Shared task budget final charge 197.83128824434243/300 seconds: implementation170.95754799625138, verification26.873740248091053; no pending launches. Builds implementation12.273696000047494 and verification0.11097958299797028 seconds separately.
