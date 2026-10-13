+++
schema_version = 1
id = "open-search-022"
key = "compose-model-portfolio-heuristics"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-021"]
+++
# Compose explicit Model portfolio recipes with native heuristic builders

## Outcome

An owning Model portfolio can run explicitly configured native heuristic recipes alongside BAB through the same builder composition boundary.

## Context

gecode/model/portfolio.cpp:2153 limits CompleteEngine to DFS/BAB and make_engine():2444–2463 constructs that closed set. Ordinary-Space portfolios already compose A*/CABS/Column builders with safe incumbent adoption and local resource-stop handling. Use the builder/Model ownership boundary introduced by compose-model-native-builders and existing authored-root recipes; read ModelEngineAsset restriction/rebuild and capacity protocols before integration.

## Boundaries

- Start with explicit scalar-integer authored-root recipes. Keep existing portfolio defaults and scheduling policies; no automatic heuristic portfolio selection.
- Preserve restriction and incumbent delivery at safe asset-thread boundaries and authored branchers. Reject unsupported restart/parallel combinations rather than silently altering the engine.
- Do not add a second scheduler or Model-specific bound/rank/pruning implementation.

## Done when

- [x] Explicit A*, CABS and Column recipes reuse native builders, guidance and pruning through the Model portfolio.
- [x] A width/frontier-stopped heuristic asset contributes no coverage proof, while a BAB peer can finish and establish the result.
- [x] Incumbents, supported restrictions/rebuilds, worker parking and recipe lifetimes preserve the existing Model portfolio contracts.

## Validation

- Extend the existing bounded Model portfolio journey with a configured heuristic recipe and BAB, an external improvement, and capped incomplete termination followed by BAB completion. Reuse focused native parking and Model restriction/rebuild checks; add only regressions needed at the new adapter boundary.

## Result

Added explicit owning Model portfolio recipes using native builders, authored roots and existing asset protocols; local caps permit BAB peer completion and ordinary incumbent delivery preserves native frontiers.

Validation:

- Independent whole-task PASS at W28f384ad4dbf4430; both snapshot comparisons equal.
- Release builds, twelve focused Model/native/worker selections, inherited open-list/extensional checks, check target, quickstart and four open examples passed; git diff --check clean.
- Worked CABS width-one example returned -2 Incomplete alone and -7 Optimal with BAB; capped Column also remained incomplete alone and allowed BAB completion.
