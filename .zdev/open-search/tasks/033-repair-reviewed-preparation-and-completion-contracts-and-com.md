+++
schema_version = 1
id = "open-search-033"
key = "repair-reviewed-search-contracts"
area = "open-search"
status = "open"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Repair reviewed preparation and completion contracts and compact CABS victims

## Outcome

Native and ordinary search respect completion validity, nested stop/statistics and preparation outcomes; CABS removes dominated candidates with one compaction.

## Context

The approved Astra review found three reproducible defects: a cached existential completion survives later dependent branching/cuts and yields false solutions; nested completion falls through ordinary DFS choice construction with default stop/statistics; successful preparation can leave a failed space whose null choice is dereferenced. Begin with gecode/search/completion.hh, seq/choice-expansion.hh, kernel brancher lifecycle, ordinary DFS/BAB and test/search.cpp. Reuse the existing fixtures. The user approved applying these findings and the local CABS compaction. The review and four small reproducers are available in /private/tmp/open-search-astra-review-20261013.md and /private/tmp/open-search-*-probe.cpp in this session.

## Boundaries

- Preserve native independence from Model, portfolio and open lists, and preserve separate ranking, admissible bounds and optional pruning.
- Fix the lifecycle behavior rather than merely documenting known false solutions away; use the smallest coherent Gecode-compatible protocol. Do not introduce a universal search policy framework.
- Keep existing supported search/completion composition sound; inspect ordinary search and replay paths affected by the hooks. Do not modify feature/comparison or sibling worktrees.
- Assess feature/comparison as the common non-mutating objective comparison seam; retain compatibility and record a concise recommendation without importing unrelated branch changes.

## Done when

- [ ] Later dependent decisions and incumbent restrictions cannot reuse an invalid completion witness or publish infeasible solutions.
- [ ] Nested completion obeys enclosing stops, resumes correctly and accounts for its actual search statistics.
- [ ] Successful preparation has an explicit enforced status contract and cannot cause null-choice dereference on failure or completion.
- [ ] CABS replaces repeated dominated-victim erasures with one compaction while preserving descriptor accounting and search coverage.
- [ ] Completion state transitions are readable, focused regressions cover the exhibited defects, and comparison integration recommendation identifies current overlap and remaining dependencies.

## Validation

- Add focused behavior regressions in the existing test/search.cpp fixtures for the three reproduced defects, including ordinary/native and nested resumption paths as applicable.
- Build and run existing CMake check in experimental ON and OFF configurations; run the existing native AStar/CABS/Column/TSPTW and completion checks and tiny native examples.
- Run focused affected tests under current ASan+UBSan using the project sanitizer option when the toolchain permits; report any concrete unavailable tooling. Check diff whitespace on touched source. No new test harness or broad benchmark campaign.
