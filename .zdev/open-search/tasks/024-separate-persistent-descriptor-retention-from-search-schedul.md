+++
schema_version = 1
id = "open-search-024"
key = "share-persistent-state-registry"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = []
+++
# Separate persistent descriptor retention from search scheduling

## Outcome

A* and Column share one small internal value-only registry, while each engine continues to own its search frontier and scheduling.

## Context

gecode/search/astar.cpp:110–152 and column.cpp:117–168 duplicate exact-key lookup, label comparison, dominated-label removal, forgetting and descriptor accounting; their seq headers duplicate storage. These are the same persistent-descriptor semantics with two real callers. CABS instead ties descriptors to live candidates and must keep that lifetime rule.

## Boundaries

- The registry owns descriptors and comparison/forgetting behavior, never Spaces, Choices, queue order or depth.
- Keep the engine sequence explicit: extract, check subsumption, admit frontier work, then retain its descriptor. Replacing metadata must not remove old pending work.
- Keep CABS's live-candidate storage separate; do not create a registry with algorithm modes or a universal search base class.

## Done when

- [x] Both A*/Column use one implementation for persistent registry semantics, limits and current storage accounting.
- [x] Engines retain responsibility for rejection counts and peak statistics; pruning disabled incurs no registry/extraction work.
- [x] Reset, better-label reopening, exact-key collision handling and metadata forgetting preserve current results.

## Validation

- Run the existing state-pruning, A*, Column and lifecycle cases before and after the behavior-preserving extraction. No new tests are expected unless a previously untested changed contract is identified.

## Result

Extracted a shared value-only StateRegistry for A*/Column; frontier admission, scheduling, rejection and peak statistics stay with each engine, and CABS storage remains separate.

Validation:

- Independent whole-task PASS at Wb77139e403543c13; both exact comparisons equal.
- Existing six native/state-pruning/lifecycle cases passed before and after extraction. Independent final Release and ASAN builds/cases/examples, repository check, inherited tests and four open examples passed; packaging includes new header; git diff --check clean.
