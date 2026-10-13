+++
schema_version = 1
id = "open-search-008"
key = "native-state-pruning"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-007"]
+++
# Add optional exact-state and Pareto pruning to A* and CABS

## Outcome

Native A*/CABS accept an optional model-justified descriptor/relation and prune equivalent or dominated work safely, demonstrated on TSPTW Pareto labels.

## Context

Read the brief's complete state-pruning rules and ../background/next-search-and-benchmarks.md Guidance, equality, dominance, and merging. Start from gecode/search.hh, seq/astar.hh, seq/cabs.hh and their pending-node/Choice ownership. The worked transition Space now supplies sufficient U/i and t/g; cost cannot be inferred from a total bound.

## Boundaries

- Add no Space virtual method, mandatory registry, Model/portfolio prerequisite, relaxed CP-store merge or generic transition-dominance framework.
- Descriptors own values and may decline pruning at a node where sufficient state is unavailable. Hashes are buckets; exact equality is required. Disabled policy keeps ordinary tree behavior.

## Done when

- [x] The native declaration extracts owned exact keys/labels and distinguishes equivalence, dominance and incomparability. A documented TSPTW declaration groups (U,i) and compares both t and g; ordinary partial CP nodes can remain unregistered.
- [x] A* admits fresh dominating work and reopens better labels after earlier expansion, with incompatible/obsolete pending entries handled safely. Equal representatives retain one route.
- [x] CABS compares only live next-layer candidates, removes registry entries when candidates leave the beam, and clears between layers/passes. A beam-discarded dominator cannot suppress surviving work or erase truncation.
- [x] Collision, incomparable cost/time, better-label reopening with inconsistent admissible bounds, equivalent representatives, beam removal and registry forgetting preserve tiny optima. Returned tours remain independently owned and valid.
- [x] Document retention/forgetting and resource behavior; only forgetting registry information is allowed to lose pruning. Expose the minimal counts/storage evidence needed for the benchmark, without retaining duplicate Space clones.

## Validation

- Add focused semantic cases in existing native search fixtures that would lose an optimum under an incorrect key/registry policy, including forced hash collisions.
- Run TSPTW pruning off/on against enumeration; validate all publications with the checker.
- Run focused search/completion/worker-control/portfolio checks, full check and native ASan -threads 1; show no descriptor/registry work when disabled.

## Result

Added optional native exact-state and Pareto pruning with A* reopening, CABS live-candidate descriptors, metadata-only forgetting, and default-off TSPTW guidance.

Validation:

- Independent whole-task review passed on unchanged snapshot Wa0bd57435da19d5f.
- Release and Model-disabled ASan builds, native semantic/lifecycle/portfolio selections, full repository check, native examples, and no-Model linkage inspection passed.
- Pruning off/on matched tiny enumeration and every returned tour passed independent checking; bounded canonical validation produced 22 proofs and 14 honest stops under five-second caps.
