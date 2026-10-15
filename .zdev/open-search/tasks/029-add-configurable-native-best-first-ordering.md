+++
schema_version = 1
id = "open-search-029"
key = "rank-first-best-first"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-024", "open-search-025"]
+++
# Add configurable native best-first ordering

## Outcome

Ordinary Spaces can use bound-first or rank-first best-first exploration through shared native search machinery, with ranking independent of sound pruning and completion.

## Context

AStar currently compares bound, then rank, then insertion order in gecode/search/astar.cpp. HeuristicGuidance already separates bound and rank. Read the brief's Additional engine work section, search.hh, astar.cpp, its seq header and native search tests. Tasks 024 and 025 provide the shared descriptor registry and original-Choice continuation. Hansen and Zhou's Anytime Heuristic Search (CPKB 2007-hansen-anytime-heuristic-search, fulltext lines 140–176) motivates separating inadmissible exploration from admissible pruning.

## Boundaries

- Keep the existing AStar API and its default ordering compatible. Choose the smallest native API that exposes ordering through a normal Search::Builder; do not introduce a universal policy framework or require Model, portfolio, open lists, or new Space virtual methods.
- Reuse the existing frontier implementation, shared descriptor registry and Choice continuation. Preserve ownership, incumbent updates, stops/resume, reset and optional StatePruning contracts.
- Rank is lower-first and may be inadmissible. Bound retains its sense-aware pruning/proof contract. Define deterministic ties and missing-rank behavior; use bound-first fallback when no rank is supplied.
- Provide greedy h and weighted g+w*h examples only for additive minimization with explicit g and h and checked arithmetic. Do not reinterpret a CP total bound as remaining h. Dynamic weights, ARA* repair/rekeying, first-goal optimality and approximation-ratio guarantees are outside this task.

## Done when

- [x] A documented native builder exposes bound-first and rank-first exploration without duplicating the engine, and existing AStar callers retain their behavior.
- [x] Ordering, sound incumbent pruning and completion remain separate: misleading rank may change the first incumbent but cannot cause an incorrect optimum or completion claim for either objective sense.
- [x] A small ordinary-Space example demonstrates greedy and fixed weighted ranking; arbitrary ranking and optional descriptor pruning remain composable.
- [x] Documentation explains callback lifetime, ties, fallback, numeric requirements and when completion proves optimality, using existing native contracts where they already suffice.

## Validation

- Build the affected search library and example. Add focused semantic coverage showing an actual exploration-order difference and a misleading first goal followed by the optimum; cover both objective senses without a full option matrix.
- Run existing AStar/state-pruning, lifecycle, stop/resume and relevant native builder/portfolio tests. Reuse these tests rather than adding callback-wiring tests or a performance campaign.

## Result

Added native configurable best-first ordering through the existing AStar worker, preserving legacy defaults and independent admissible-bound pruning; demonstrated greedy and fixed weighted ranking on an ordinary Space.

Validation:

- Verified open-search-029 against snapshot Wa9e80630fff49a1b at HEAD 01928ba0870add826081661d50e7828c64cefa67. Reviewed the whole attributed delta, public APIs and callers, shared frontier comparator, bound-only pruning, continuation/reset ownership, optional descriptor pruning, documentation and example arithmetic. Existing AStar signatures/default ordering remain compatible; rank-first changes exploration without changing sound completion. Independently passed Release configure/build, all six focused native selections, full check, inherited open-list/regular/Model portfolio selection, both native examples and all four inherited examples, native linkage inspection and git diff --check. Greedy example returned 10 then 5; fixed weighted ranking returned 5; both proved completion. Reviewed tracked contents, index, HEAD and untracked example blob still match the frozen snapshot. Validation intentionally updated ignored build artifacts and /private/tmp verification logs; no task-owned source or record writes occurred. Baseline validation was historical evidence rather than independently recreated. Autotools, Windows and sanitizer configurations were not exercised; no new ownership boundary required conditional ASan, and no benchmark campaign ran.
