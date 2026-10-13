+++
schema_version = 1
id = "open-search-027"
key = "cabs-discarded-bound-completion"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = []
+++
# Close CABS passes when discarded work is bounded out

## Outcome

CABS can prove completion after a truncated pass when every width-discarded alternative is unable to improve the accepted incumbent.

## Context

gecode/search/cabs.cpp:148–155 records only truncated, and advance():213–219 always widens or stops afterward. At width one, keeping A(bound 0), dropping B(bound 20), then finding cost 10 under A permits a proof once retained work finishes. Kuroiwa and Beck's DIDP Theorem 20 accounts for discarded-state bounds (doi:10.1016/j.artint.2026.104506; CPKB paper 2026-kuroiwa-domain-independent-dynamic-programming, fulltext 1871–1903). This is a new capability, not a defect in the existing conservative contract.

## Boundaries

- Keep proof accounting independent of beam ranking and retain only an optimistic aggregate for width-discarded work.
- Check the shortcut only after all retained work and original alternatives in the pass are processed. Frontier-cap loss and ordinary interruption remain incomplete unless separately proven under an explicit contract.
- No global dual-bound reporting API, weighted A*, new engine family or changed defaults.

## Done when

- [x] Rejected and evicted width candidates contribute sound directional bounds, including arbitrary ranking and inconsistent admissible estimates.
- [x] A completed pass closes before widening/width-limit termination when its discarded work cannot improve the incumbent.
- [x] If any dropped bound still permits improvement, widening and capped-stop behavior remain conservative; reset/pass accounting and external incumbents are correct.

## Validation

- Extend existing CABS fixtures with minimization/maximization proof shortcuts, an opposite-ranking case and a discarded improving branch that must prevent proof. Reuse interrupted-pass and dominance tests to check that pending work is never omitted.

## Result

CABS proves completed truncated passes when the directional aggregate of actual width-discarded bounds cannot improve the accepted incumbent; unresolved loss remains conservative.

Validation:

- Independent whole-task PASS at W184b370fc65c9fc8; both exact comparisons equal.
- Baseline/final Release tests and new finite-graph proofs, Release/ASAN builds/tests/examples, repository check and inherited open integration passed; git diff --check clean.
- Tests cover min/max reject/evict equality, opposite ranking/inconsistent admissible bounds/multiple layers, original alternatives, interruption/resume, external optimum, and reset after cap/widening.
