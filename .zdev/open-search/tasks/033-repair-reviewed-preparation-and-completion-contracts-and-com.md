+++
schema_version = 1
id = "open-search-033"
key = "repair-reviewed-search-contracts"
area = "open-search"
status = "done"
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
- Target the experimental Gecode 6.5 preview. Do not depend on feature/comparison, which targets Gecode 7. Remove the extracted Gecode 7 comparison API from this branch if native scalar guidance suffices; retain existing Gecode 6.x ordinary-model compatibility and record the comparison assessment.

## Done when

- [x] Later dependent decisions and incumbent restrictions cannot reuse an invalid completion witness or publish infeasible solutions.
- [x] Nested completion obeys enclosing stops, resumes correctly and accounts for its actual search statistics.
- [x] Successful preparation has an explicit enforced status contract and cannot cause null-choice dereference on failure or completion.
- [x] CABS replaces repeated dominated-victim erasures with one compaction while preserving descriptor accounting and search coverage.
- [x] Completion state transitions are readable, focused regressions cover the exhibited defects, and native incumbent acceptance no longer requires the Gecode 7 Space comparison API. Record any separate legacy ordinary-search limitation honestly.

## Validation

- Add focused behavior regressions in the existing test/search.cpp fixtures for the three reproduced defects, including ordinary/native and nested resumption paths as applicable.
- Build and run existing CMake check in experimental ON and OFF configurations; run the existing native AStar/CABS/Column/TSPTW and completion checks and tiny native examples.
- Run focused affected tests under current ASan+UBSan using the project sanitizer option when the toolchain permits; report any concrete unavailable tooling. Check diff whitespace on touched source. No new test harness or broad benchmark campaign.

## Result

Repaired completion invalidation, nested stop/statistics and prepared terminal handling across ordinary/native engines; compacted CABS victims once. Removed extracted Gecode 7 comparison API: native exact scalar guidance supports the Gecode 6.5 experimental preview. Documented legacy ordinary hidden-completion admission limitation.

Validation:

- Independent review passed against unchanged snapshot W229fed27ab5740d6; experimental ON 479/479 and OFF 475/475.
- Focused main-thread checks 16/16 and fresh Debug ASan+UBSan 16/16 passed without diagnostics; all three new regressions failed before fixes.
- Tiny native examples, independently checked TSPTW formulations and engines, rebuilt CADDS witness, and diff whitespace passed. clang-format and clang-tidy unavailable.
