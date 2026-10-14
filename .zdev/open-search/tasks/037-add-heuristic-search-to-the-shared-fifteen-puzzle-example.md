+++
schema_version = 1
id = "open-search-037"
key = "fifteen-puzzle-search"
area = "open-search"
status = "open"
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

- [ ] Both model representations support bab, astar, and cabs through Script::run with experimental search enabled.
- [ ] The example builds and runs BAB with experimental search disabled, using a promptly completing default.
- [ ] The extension preserves upstream model behavior and uses known_length() consistently, including the existing TSPTW caller.

## Validation

- Build the example with experimental search enabled and disabled using the existing project configuration.
- Run small open/fixed cases with BAB, A*, and CABS, including an already solved start and a bounded infeasible case; inspect results and stop/exhaustion reporting. No fresh performance campaign is required.
- Build the existing TSPTW example after updating its renamed query; check the diff against the CP base and canonical MPG source for accidental model changes.
