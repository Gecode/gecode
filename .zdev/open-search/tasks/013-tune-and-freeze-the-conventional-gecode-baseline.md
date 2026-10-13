+++
schema_version = 1
id = "open-search-013"
key = "tsptw-cp-baseline-tuning"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-006", "open-search-012"]
+++
# Tune and freeze the conventional Gecode baseline

## Outcome

Development measurements select a strong conventional CP baseline using existing propagation, branching and search facilities, then freeze it before held-out evaluation.

## Context

Read the brief's development budget and conventional-versus-transition distinction. Start with the closed model's costed circuit/time constraints, examples/tsp.cpp regret branching, native BAB/restart options and existing ordinary-Space LNS recipe seams. This task makes best Gecode can offer a measured comparison rather than a deliberately weak straw baseline.

## Boundaries

- At most six named candidate configurations, including the initial BAB baseline; no new routing global or engine.
- Use development cases only and at most 45 minutes solver wall budget, two repetitions at 15 seconds/case. No tuning on held-out outcomes.

## Done when

- [x] The candidates justify propagation level and at most a small set of branching choices; include an existing restart or LNS recipe when it can be configured soundly and adds a meaningful alternative within the six-candidate cap.
- [x] Each candidate uses the same objective/data/checker and reports its resources. Local LNS completion never becomes a global proof; exact peers retain proof responsibility.
- [x] Paired development results include validity, time/quality/proof and RSS. Freeze the selected closed CP formulation/branching/BAB configuration and, if useful, an explicit anytime recipe; retaining the initial baseline is a valid measured outcome.
- [x] Document the actual comparison, counterproductive variants and the frozen baseline command so later engine/formulation results can be interpreted.

## Validation

- Run the bounded named development matrix, check all incumbents and replay analysis from saved records.
- Run tiny correctness and existing native portfolio/restart checks for touched configuration changes; no new tests are expected for tuning-only settings.

## Result

Measured five conventional CP candidates and froze DOM/size BAB. Across 24 paired development samples it produced 20 feasible runs and 18 proofs versus initial regret BAB 16/12; the AFG regression is documented. Restart did not improve coverage, so no anytime recipe is recommended.

Validation:

- Independent whole-task PASS against W3b9ba286a0e3b921. Release build, existing runner/TSPTW/RBS/portfolio checks and bounded five-candidate tiny, stop, resume, mismatch, interruption and offline-analysis checks passed. All 120 campaign records and 806 incumbents were independently rechecked; completed resume preserved all 360 raw files and launched no samples. Campaign wall was 792.229 seconds; tuning and compilation before review used 896.421 seconds. Charging the entire verifier interval a conservative 600 seconds gives 1496.421 seconds within the 2700-second tuning ceiling; exact duration of its unwrapped correctness command is unavailable. No held-out collection or second tuning matrix. Sampled RSS and unavailable frontier/clone/isolated phase metrics remain explicit.
