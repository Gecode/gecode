+++
schema_version = 1
id = "open-search-007"
key = "tsptw-open-transitions"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-005"]
+++
# Deliver open-list TSPTW transitions and an eager control

## Outcome

A finite ordinary Space expresses DIDP-style TSPTW transitions with a native open visit list and clone-local residual state, plus an eager path-variable control with identical decisions.

## Context

Read the brief's transition rules, ../background/didp-and-tsptw.md and ../background/next-search-and-benchmarks.md. Begin at gecode/int/open-var-list.hpp, native brancher prepare/choice/commit contracts, test/int/open-var-list.cpp and test/search.cpp completion fixtures. Consult the older feature/open-cp-model:models/open-puzzle.cpp only for the concrete move-brancher pattern.

## Boundaries

- Use existing native open variables and a problem-specific brancher, with no Model prerequisite, generic transition language, subset automaton or engine understanding of list horizons.
- The eager control changes only path-variable allocation, not transition applicability, ordering, propagation policy or search engine.

## Done when

- [x] Stable nodes represent concrete clone-local (U,i,t,g); a n-ary Choice commit assigns exactly one applicable visit and updates state once. Materialization uses prepare where required and is not a committed transition.
- [x] A final return action enforces the depot window and contributes cost exactly once; terminal output and bound equal full tour cost. The initial valid total bound is prefix cost at nonsolutions.
- [x] BAB/A*/CABS find the same tiny optima/infeasibility as enumeration and checker. Partial publication, clone independence, inherited cuts and parked/resumed original alternatives preserve unvisited work.
- [x] Open-list allocation grows with the committed prefix. The eager control reserves path variables upfront and emits the same tours; report basic Space/allocation sizes without claiming compact DP representation.

## Validation

- Build/run both representations on the offline correctness set, check every tour, and compare against tiny exhaustive enumeration.
- Exercise materialized-but-unassigned positions and cloned prefixes in existing open-list/completion tests. Run focused native search, worker-control and open-list selections plus check.
- Use the existing native ASan build with -threads 1 for new ownership paths.

## Result

Added clone-local native TSPTW transitions with lazy open-list and eager path allocation, archived replay, checked seeds, and explicit depot return.

Validation:

- Independent whole-task review passed on unchanged snapshot Wf2f02ad91f4651bd.
- Release and Model-disabled ASan builds, focused transition/search/completion/pause/open-list selections, reader/checker validation, linkage inspection, and repository check passed.
- Tiny enumeration and lazy/eager tour sequences agree; canonical runs produced 22 proofs and 14 honest stops under five-second caps; all 45 retained/acquired inputs passed reader checking.
