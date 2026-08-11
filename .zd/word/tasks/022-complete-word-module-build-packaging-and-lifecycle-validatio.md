+++
schema_version = 1
id = "word-022"
key = "release-engineering"
area = "word"
status = "open"
blocked_by = ["word-020", "word-021"]
+++
# Complete word module build packaging and lifecycle validation

## Outcome

GecodeWord builds, disables, installs, exports, and is consumed correctly across supported CMake and Autoconf/Make configurations, with complete lifecycle hardening for the released surface.

## Boundaries

- Do not add FlatZinc, MiniZinc, or another frontend.

## Done when

- [ ] Shared and static builds cover word enabled and disabled configurations and preserve aggregate component dependencies.
- [ ] Installed downstream consumers link the word component and aggregate targets through both supported build-system contracts.
- [ ] Generated-file, visibility, representative platform, source archive, and package-content checks include GecodeWord correctly.
- [ ] Sanitizer and fault-injection coverage exercises WordVar, propagator, brancher, MiniModel-owned, and heap-backed clone/disposal paths.
- [ ] The full focused word suite and required regression suites pass.

## Validation

- Run the shared/static, enabled/disabled CMake and Autoconf/Make matrix.
- Run installed downstream consumer tests and generated-source checks.
- Run focused sanitizer and fault-injection suites.
- Scan source and binary packages and run the full focused word regression suite.
