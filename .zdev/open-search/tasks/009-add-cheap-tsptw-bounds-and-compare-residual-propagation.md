+++
schema_version = 1
id = "open-search-009"
key = "tsptw-bounds-propagation"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-006", "open-search-007"]
+++
# Add cheap TSPTW bounds and compare residual propagation

## Outcome

Checked inexpensive bounds and optional residual CP inference strengthen TSPTW without changing feasibility, objective or transition-layer meaning.

## Context

Read ../background/didp-and-tsptw.md bound and propagation discussions and the brief's next-bundle rules. Start with the two worked models and current guidance callbacks. Direct travel is not a path lower bound on nonmetric input; terminal cases and return must be handled explicitly. Existing unary/scheduling and costed-circuit facilities are candidates for a residual relaxation.

## Boundaries

- Keep bound evaluation separate from beam ranking. Add no 1-tree or new propagator before measurements.
- Residual inference is an optional independently switchable relaxation run through normal Gecode status() fixpoints; do not copy Pumpkin's one-pass policy or silently change the sufficient state contract.

## Done when

- [x] Transition guidance supplies checked g plus the maximum of valid minimum incoming/outgoing residual bounds, including root/terminal return conventions; closed CP guidance uses its propagated total objective without double-counting g.
- [x] Nonmetric reachability uses sound shortest-path lower bounds. Optional residual scheduling/CP inference can reject dead residual problems or filter actions only through justified relaxation consequences.
- [x] Every retained stronger bound and filter is cross-checked on enumerated residual completions, including waiting, nonmetric travel and unequal time/cost labels.
- [x] Bound strength and residual propagation have independent switches. A bounded development comparison records evaluation/propagation time, expansions, elapsed time and storage; a slower option may remain explicit rather than become a default.

## Validation

- Compare baseline, cheap bounds, and residual propagation on tiny exhaustive cases; checker validates all returned tours.
- Use at most 15 minutes of solver wall budget for the initial development ablation, record exact settings and inputs, and make no held-out tuning decision.
- Run existing touched propagation/search/open-list selections and check; no broad bound-option matrix.

## Result

Added checked minimum-arc total bounds and independent residual unary inference, preserving native transition semantics and defaults; retained a bounded development comparison.

Validation:

- Independent PASS at We368f449ded56b48; exact snapshot comparison unchanged.
- Release and Model-disabled ASan builds; exhaustive residual suffix and offline tour checks; search, unary, open-list, check, Model/portfolio and inherited open-example regressions passed.
- Verified all 18 development comparison settings, input/binary hashes, checked incumbents and termination/timing/storage caveats; 33.855 seconds external capture, no held-out outcomes.
