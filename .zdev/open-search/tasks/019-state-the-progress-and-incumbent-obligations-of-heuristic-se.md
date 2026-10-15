+++
schema_version = 1
id = "open-search-019"
key = "clarify-pruning-and-proof-contracts"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = []
+++
# State the progress and incumbent obligations of heuristic search

## Outcome

Native and Model users can supply sound state pruning and interpret exhaustion correctly after external incumbent adoption.

## Context

gecode/search.hh:1199–1203 and docs/astar.md require preservation of residual feasibility/cost but omit progress toward a solution. In a finite root→A→B→goal chain, A/B can have identical domains/costs but different brancher stages; a persistent ancestor descriptor can suppress B. Kuroiwa and Beck, Domain-Independent Dynamic Programming (doi:10.1016/j.artint.2026.104506), Definition 14 explicitly requires an at-least-as-good completion with no more transitions (CPKB paper 2026-kuroiwa-domain-independent-dynamic-programming, fulltext lines 574–606). TSPTW's remaining-visit key already preserves progress. Separately docs/model/README.md:2501–2503 overlooks an accepted external incumbent, while test/model/runtime.cpp:17683–17692 already covers exhaustion without a new publication.

## Boundaries

- Treat pruning as an expert-supplied soundness obligation; do not infer stage, dominance or admissibility automatically or add mandatory descriptor fields.
- Preserve the existing native engine behavior and scalar finite-tree scope.

## Done when

- [x] The native and Model pruning contracts explicitly cover residual brancher/search progress, with a sufficient-stage-key or declined-extraction example and the unsafe domain-only counterexample.
- [x] All affected proof instructions refer to the best accepted incumbent, including external adoption; no newly returned solution does not imply infeasibility when an incumbent was adopted.

## Validation

- Independently check the analytical finite-stage counterexample and sufficient-key explanation against the persistent registry code and paper Definition 14. Reuse the existing graph and external-incumbent fixtures as evidence; no new automated tests are expected for this documentation/contract clarification.

## Result

Native and Model contracts preserve residual brancher progress, show the unsafe domain-only chain and stage-key or declined-extraction remedy, and interpret proof using the best accepted incumbent including external adoption.

Validation:

- Independent analytical/source verification checked persistent registries, DIDP Definition 14, graph/history/reopening fixtures and all three Model incumbent-adoption cases. Diff check passed; documentation-only change required no new tests.
