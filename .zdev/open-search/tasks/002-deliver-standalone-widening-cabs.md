+++
schema_version = 1
id = "open-search-002"
key = "native-cabs"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-001"]
+++
# Deliver standalone widening CABS

## Outcome

Ordinary Space users obtain improving integer solutions through sequential layer beams that widen until complete, independently of Model and portfolio, with honest width/resource termination.

## Context

Read Initial search contract and Native, Model, and portfolio integration in ../brief.md, plus CABS proof/code references in ../background/didp-and-tsptw.md. Reuse native A* guidance, ownership, preparation, and builder conventions. Start with gecode/search.hh, the new A* implementation, search/support.hh, and test/search.cpp. CABS must recover width-discarded alternatives and deeper goals; a narrow empty beam cannot prove infeasibility.

## Boundaries

- Use native Choice commits as layers, width one followed by checked doubling, cloned states, and the same scalar objective scope.
- No graph dominance, parallel beam, learned ranking, global dual-bound API, or portfolio policy rewrite in this task.
- Use the native API without requiring Model compilation, portfolio configuration, or open lists.

## Done when

- [x] A native engine and builder run CABS with optional heuristic ranking and admissible bound pruning; equal priorities use deterministic ties. The same capacity-one pause/resume protocol works without losing beam/pass state.
- [x] An immutable root and owned incumbent survive widening passes. Increasing width regenerates omitted work; external incumbent adoption affects remaining work and subsequent passes without mutating the stored restart root. Reuse the native preparation/stop-resume protocol.
- [x] Terminal nodes at different depths and publication during a pass do not lose improving alternatives. A pass reports complete only when all improving alternatives were exhausted with sound pruning.
- [x] Maximum width, live-frontier node limit, ordinary stop objects, and width-growth overflow end incompletely with stopped reporting; node limits are documented as node quotas rather than hard process-memory limits.
- [x] Focused tests show width-one loss of an optimum and loss of all solutions, recovery on widening, variable-depth goals, min/max behavior, and width/resource termination without false proof.
- [x] A standalone bounded closed Space example compares A*, CABS, and BAB under the same formulation; documentation explains their result and resource semantics.

## Validation

- Build/run focused native CABS tests plus the standalone example, including brancher-preparation interruption/resumption using the existing completion fixture; compare tiny optimum/infeasibility results with exhaustive/BAB references.
- Run the existing check target and inherited open-list lifecycle/portfolio public-contract checks. Record a small reproducible width/frontier/time comparison; do not tune defaults or claim benchmark gains yet.

## Result

Delivered standalone sequential CABS with bounded beams, widening, incumbent sharing, resumable preparation, and honest resource stops.

Validation:

- Independent verifier passed snapshot W478094cb560bd1c9 with no findings and unchanged checkout.
- Release and AddressSanitizer focused CABS/AStar/completion/worker-control tests and native comparison passed.
- Repository check, inherited open-list/regular/portfolio checks, and four open examples passed; native target has no Model dependency.
