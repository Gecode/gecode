+++
schema_version = 1
id = "open-search-020"
key = "separate-model-bound-rank"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = []
+++
# Compose Model bounds and ranking with independent dependencies

## Outcome

Separately authored Model bound and ranking helpers compose without sharing or rewriting their dependency indexes, and export the existing native guidance values.

## Context

gecode/model/runtime.hh:705–707 takes one dependency list for both callbacks, model.cpp:2591–2593 rejects a second declaration, and runtime.cpp:1522–1526 gives both callbacks the same index context. A bound helper using index zero for X and an independent rank helper using index zero for Y cannot be directly composed. Native HeuristicGuidance already has independently replaceable callbacks. Read Model declaration, retention/remapping and compile/runtime tests before choosing the smallest separate declaration API.

## Boundaries

- Keep the combined heuristic_guidance call as a compatible convenience; define conflicts between combined and separate declarations clearly.
- Retain only the union of explicitly declared dependencies while preserving each callback's own indexing. Export the unchanged native HeuristicGuidance; no new generic policy framework or automatic heuristic inference.

## Done when

- [x] A bound and a rank with different index-zero dependencies can be authored independently, compiled together and evaluated correctly on clones.
- [x] Bound-only, rank-only with the safe objective default, and the existing combined call retain their documented behavior and lifetime guarantees.
- [x] Conflicting declarations fail clearly and leave the Model declaration state consistent.

## Validation

- Extend the existing Model guidance fixtures with one independent-dependency composition witness and conflicting declaration coverage; reuse O0–O3 retention, remapping and artifact-lifetime checks. Demonstrate reuse with native A*/CABS/Column without duplicating their ordering tests.

## Result

Added independent heuristic_bound and heuristic_rank declarations, callback-specific dependency contexts and union retention while preserving atomic combined convenience and unchanged native guidance.

Validation:

- Baseline and final build and all four Model heuristic fixtures passed. O0-O3 remapping, independent indexes, clone and artifact lifetime, open-list permissions, transactional conflicts and native AStar/CABS/Column reuse were independently verified. Diff check passed and review context remained unchanged.
