+++
schema_version = 1
id = "word-022"
key = "release-engineering"
area = "word"
status = "done"
blocked_by = ["word-020", "word-021"]
+++
# Complete word module build packaging and lifecycle validation

## Outcome

GecodeWord builds, disables, installs, exports, and is consumed correctly across supported CMake and Autoconf/Make configurations, with complete lifecycle hardening for the released surface.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not add FlatZinc, MiniZinc, or another frontend.

## Done when

- [x] Shared and static builds cover word enabled and disabled configurations and preserve aggregate component dependencies.
- [x] Installed downstream consumers link the word component and aggregate targets through both supported build-system contracts.
- [x] Generated-file, visibility, representative platform, source archive, and package-content checks include GecodeWord correctly.
- [x] Sanitizer and fault-injection coverage exercises WordVar, propagator, brancher, MiniModel-owned, and heap-backed clone/disposal paths.
- [x] The full focused word suite and required regression suites pass.

## Validation

- Run the shared/static, enabled/disabled CMake and Autoconf/Make matrix.
- Run installed downstream consumer tests and generated-source checks.
- Run focused sanitizer and fault-injection suites.
- Scan source and binary packages and run the full focused word regression suite.

## Result

Completed Word release engineering with installed component and aggregate consumers, shared/static enabled/disabled coverage, guarded fault-injection lifecycle cases, and stable clone tests.

Validation:

- CMake and Autoconf shared/static enabled/disabled matrices, installed consumers, full Word and fault suites, ASan+UBSan focused faults, generated/visibility/archive/package checks, representative regressions, and independent verifier PASS.
