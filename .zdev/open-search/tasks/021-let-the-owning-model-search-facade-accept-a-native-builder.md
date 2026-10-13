+++
schema_version = 1
id = "open-search-021"
key = "compose-model-native-builders"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = []
+++
# Let the owning Model search facade accept a native builder

## Outcome

Model root/output ownership composes with an already configured native Search::Builder, while the existing engine-selection constructor remains convenient.

## Context

ModelHeuristicSearch in gecode/model/runtime.hh:1292–1307 and runtime.cpp:1554–1578 currently selects through an enum, reconstructs guidance/pruning and carries options for unrelated engines. CompiledModel already exports native guidance and pruning, and Search::Builder exposes options() and best(). A builder-taking path lets one compiled artifact vary ranking, pruning and engine configuration without adding another Model switch.

## Boundaries

- Reuse Search::Builder and the existing ownership conventions; preserve the convenience constructor by delegating through the same construction path.
- The new path consumes the builder, sets its clone option to true for the facade's temporary owned root, and uses its stop option for root construction. Stop objects remain borrowed for the search lifetime. Accept compatible best-search builders, including BAB, within the facade's scalar-integer objective scope; reject null or non-best builders before creating the root. ModelSpace compatibility remains the caller's obligation.
- Do not make native engines depend on Model or introduce a second Model search-policy hierarchy.

## Done when

- [x] A caller can pass a configured native builder and reuse one compiled Model with changed/disabled ranking or pruning.
- [x] The old convenience call uses the same ownership-safe path, and metadata/rendering remain valid for the documented lifetime.
- [x] Construction failure, null/unsuitable builder and capped incomplete termination have clear behavior without leaking or double-owning roots.

## Validation

- Extend existing Model facade tests with configured native heuristic builders and BAB, one lifetime/construction-failure case and a capped run. Check the consumed builder/root ownership and borrowed stop behavior. Reuse existing native semantic checks; no duplicate engine matrix.

## Result

Added a consuming native-builder Model facade; preserved caller configuration and delegated enum convenience through the same ownership-safe path.

Validation:

- Independent review PASS at Wcd81d673bb35ab96; both completion comparisons equal.
- Release builds, focused Model/AStar/CABS/Column tests, check target, heuristic quickstart and four inherited open examples passed; git diff --check clean.
