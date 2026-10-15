+++
schema_version = 1
id = "open-search-037"
key = "fifteen-puzzle-search"
area = "open-search"
status = "done"
complexity = "routine"
afk = true
blocked_by = []
+++
# Add heuristic search to the shared fifteen-puzzle example

## Outcome

The upstream fifteen-puzzle example supports BAB, A*, and CABS for its open and fixed representations, using the same Manhattan total bound.

## Context

The MPG thread supplied a working reference implementation and coordinated a BAB-only base in feature/open-cp. Wait for its relayed commit, refresh this branch onto that parent, then extend examples/fifteen-puzzle.cpp. Read ../../_notes/fifteen-puzzle.md for the exact source path, model invariants, and separate memory follow-up. The serial measurement campaign has finished. Preserve upstream documentation, size checks, registrations, and limit=12 default. Use known_length() after the upstream rename; the existing downstream TSPTW caller also needs adaptation.

## Boundaries

- Keep the open/fixed model design and standard INT_VAL_MIN branching. Add no oracle, embedded optimum assertions, benchmark framework, or new permanent tests.
- Use the same minimizing total Manhattan bound for both heuristic engines and default unrestricted CABS widening. Guard experimental registrations and adapters; BAB must work with experimental search disabled.
- Do not modify MPG source/results or change the kernel's shared propagator metadata. Keep the GPI concern as a separate follow-up.

## Done when

- [x] Both model representations support bab, astar, and cabs through Script::run with experimental search enabled.
- [x] The example builds and runs BAB with experimental search disabled, using a promptly completing default.
- [x] The extension preserves upstream model behavior and uses known_length() consistently, including the existing TSPTW caller.

## Validation

- Build the example with experimental search enabled and disabled using the existing project configuration.
- Run small open/fixed cases with BAB, A*, and CABS, including an already solved start and a bounded infeasible case; inspect results and stop/exhaustion reporting. No fresh performance campaign is required.
- Build the existing TSPTW example after updating its renamed query; check the diff against the CP base and canonical MPG source for accidental model changes.

## Result

Added A* and CABS to the shared open/fixed fifteen-puzzle example with the same Manhattan total bound; kept BAB as default. Refreshed onto CP commit 60325e4801 and adapted the TSPTW query to known_length(). Independent verification passed without findings.

Validation:

- Release experimental ON: cmake --build build/open-search-split --target fifteen-puzzle tsptw -j2 passed.
- Release experimental OFF: cmake --build build/open-search-experimental-off --target fifteen-puzzle -j2 passed; help exposes only BAB and the default completes.
- ASan/UBSan: build/open-search-033-asan-ubsan fifteen-puzzle build and representative runs passed without diagnostics.
- Independent verifier ran 36 small normal/sanitized open/fixed BAB/A*/CABS cases through exhaustion, including solved and infeasible starts, and replayed every printed solution for legal slides and the goal. Defaults completed at 12 moves in all three configurations.
- Compared with CP base and canonical MPG source: model behavior, documentation, size checks, registrations and move-limit default preserved. git diff --check passed; verifier snapshot W4e57d393d7d88659 comparison remained equal.
