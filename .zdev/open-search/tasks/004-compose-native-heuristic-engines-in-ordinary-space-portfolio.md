+++
schema_version = 1
id = "open-search-004"
key = "portfolio-integration"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-002"]
+++
# Compose native heuristic engines in ordinary-Space portfolios

## Outcome

An ordinary Space portfolio combines A* or CABS with an existing exact engine, shares improving incumbents, and reports proof or interruption correctly without requiring Model.

## Context

Read Native, Model, and portfolio integration in ../brief.md and ../background/integration.md. Start with gecode/search/portfolio.hpp SpacePortfolioSearchFactory and complete recipes, search/portfolio.cpp SpaceRecipeCompleteAsset, search/support.hh WorkerToEngine, and test/search.cpp (ordinary-Space portfolio and worker-control fixtures). Reuse native builders and the existing adapter lifecycle. Natural recipe exhaustion is a coverage claim; a width-limited beam cannot certify closure. Model conveniences can produce the same builders later. SpaceRecipeCompleteAsset currently inherits a no-op adopt(), so forward accepted external incumbents to best-engine constrain() at a safe engine boundary. Its stopped() currently promotes intrinsic engine stops to a whole-portfolio stop; use the existing coverage-free asset End handling for CABS-local width/frontier limits while preserving shared stop semantics.

## Boundaries

- Use existing ordinary-Space portfolio recipes/adapters and capacity-one native engines; add no scheduler, portfolio policy, Model prerequisite, or parallel heuristic engine.
- Retain direct standalone construction and the same native guidance/comparison contract.

## Done when

- [x] A documented ordinary closed Space example configures A* and CABS builders through SpacePortfolioSearchFactory alongside BAB, without Model declarations or compilation.
- [x] At a safe engine boundary, accepted external incumbents reach best-engine constrain(). Owning publications, shared stop propagation, and capacity-one parking/resumption work for both heuristic assets without losing unvisited alternatives.
- [x] Only genuine complete exhaustion contributes a coverage proof. CABS-local width/frontier stops end that asset coverage-free and let other eligible complete assets continue; a shared ordinary/global stop still stops the portfolio. If no asset proves closure the result remains incomplete.
- [x] Focused existing-suite checks compare standalone and portfolio optima and exercise external incumbent sharing and parked/resumed work. Force CABS-local resource termination before BAB closure and verify BAB can still certify optimum/infeasibility; separately verify shared stop terminates incompletely.

## Validation

- Build/run the focused ordinary-Space portfolio tests and example with independently checkable tiny optima; reuse existing worker-control/lifecycle fixtures.
- Run the existing check target and relevant inherited portfolio public-contract tests. Verify the native example and its portfolio variant use the same Space and guidance.

## Result

Ordinary-Space A*/CABS recipes queue owned accepted incumbents at safe engine boundaries. Local CABS limits release the asset without proof; BAB can complete optimality or infeasibility. The native example and docs show the same standalone Space and guidance in portfolio recipes.

Validation:

- Independent verifier passed all task conditions at unchanged snapshot W6e53afa60e6e1aa6.
- Release focused and inherited selections, full check, native portfolio example/linkage and four inherited open examples passed.
- ASan build, full focused selection with -threads 1 and native example passed. Automatic parallel test harness -threads 0 overflowed its stack during root propagation before asset construction; log preserved.
