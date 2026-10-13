+++
schema_version = 1
id = "open-search-032"
key = "experimental-preview"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
blocked_by = []
+++
# Gate heuristic search as an experimental preview and separate experimental changelog entries

## Outcome

Gecode users explicitly opt into the open-search engine preview, and generated release notes distinguish experimental changes.

## Context

Read the Experimental preview section of brief.md. CBS supplies the existing default-off configure/CMake/config-header pattern. Native engine declarations are in gecode/search.hh with AStar/best-first, CABS and Column source registrations in CMake and Make. Changelog metadata is documented in changelog.in and consumed by misc/genchangelog.py, gentxtchangelog.py, gencurrentchangelog.py and tidy.py. Preserve shared native prerequisites and existing stable search interfaces.

## Boundaries

- Work only on feature/open-search; preserve parent, backup and Model integration branches.
- Keep normal namespaces and independent native search; no new engines, compatibility framework, kernel layout variation or benchmark campaign.
- Use existing generator fixture and search test infrastructure; gate only feature-specific checks.

## Done when

- [x] CMake GECODE_ENABLE_EXPERIMENTAL and Autoconf --enable-experimental default off and generate matching GECODE_HAS_EXPERIMENTAL availability. All AStar, best-first, CABS and Column APIs/implementations are available only when enabled; default ordinary search still builds and runs.
- [x] Public documentation, build instructions and engine release entries clearly identify the experimental 6.5.0 preview and its API stability policy.
- [x] Experimental metadata accepts exactly true/false and defaults false; all three generated changelog formats separate true entries per release while preserving module/type ordering and unchanged historical presentation when none are marked. Invalid values fail validation.
- [x] Applicable tracked generated configuration/changelog outputs are regenerated and existing builds/tests remain usable in both configurations.

## Validation

- Extend existing tidy/generator fixtures for true, false, omitted, invalid and separate rendering across all three generators; run tidy checks and inspect generated current-release sections.
- Run CMake Release default-OFF and explicit-ON builds with existing check targets; focused native engine tests/examples enabled, ordinary DFS/BAB disabled-mode smoke and compile-failure check for preview API when disabled.
- Verify Autoconf/Make OFF/ON configuration, generated macro, source/example registration and relevant build smoke; no broad TSPTW performance rerun.

## Result

Added a default-off experimental search preview in CMake and Autoconf, with generated availability, gated native APIs/implementations/examples/tests, public documentation and an enabled CI configuration. Added strict optional Experimental changelog metadata and separated experimental sections in all three generators. Shared native contracts remain independent of the flag. Independently verified.

Validation:

- CMake OFF check 470/470; ON check 474/474; focused preview tests 4/4. Autoconf OFF/ON search-library builds, ordinary OFF consumers, enabled native consumers and disabled-API negative compiles passed. Native examples and tiny TSPTW checker passed. Tidy true/false/omitted/invalid and rendering fixtures passed; all historical unmarked generator output is byte-identical. Tracked configure regenerated. Independent verification passed at unchanged snapshot W25adf51e7f0f1991. Remote CI was not run.
