# Gecode 7 modernization work

This branch makes CMake the only build system, establishes checked formatting
and naming conventions, and moves the code to C++23 with Clang lint checks.
Each stage is committed locally after focused validation.

## Stages

- [x] Remove Autoconf, maintained Makefiles, their helpers and CI cases.
      Preserve CMake builds, installation, and generated-source maintenance.
- [x] Establish clang-format rules from representative Gecode code. Compare
      two- and four-space indentation, update EditorConfig, remove modelines,
      and format maintained sources. Preserve third-party source conventions.
- [x] Require C++23 through exported CMake targets. Use broadly implemented
      language and library facilities; keep platform CI coverage.
- [ ] Rename identifiers to snake_case and review concatenated and unclear
      names. Update declarations, uses, generators, examples and documentation.
- [ ] Enable a useful clang-tidy baseline and resolve diagnostics, including
      experimental configurations and optional modules where available.
- [ ] Provide simple formatting and extended-check commands and CI coverage.

## Validation

Use clean CMake builds under `build/`, the existing integrity suite, installed
package consumers, generated-source freshness, and the repository tidy check.
Run formatting and lint checks on maintained code; generated files follow the
same rules through their generators. Broader suites follow changes that can
affect runtime behaviour. Platform-specific checks remain in CI.

## Decisions

- Work stays local; pushing and opening pull requests require explicit approval.
- Keep the current version/ABI values during build-system removal. Revisit the
  release metadata with the breaking API changes.
- C++ uses four-space indentation and LLVM 22 formatting; see `code-style.md`.
- Naming scope is being resolved before bulk identifier changes.

## Other modernization ideas

- Replace bespoke configuration-template processing with CMake's native
  `configure_file` / `#cmakedefine`, checking feature-macro compatibility.
- Remove Flex/Bison installation from CI jobs that only consume generated code.
- Audit shipped Boost compatibility headers against the new compiler baseline.
- Consider scoped enums and descriptive constants after naming stabilizes.
- Revisit the `forceinline` portability macro and obsolete `(void)` parameter lists.

## Verified checkpoints

- CMake-only build: 536 integrity tests passed; generator freshness and
  changelog checks passed. Commit `d5b07b083b`.
- C++23 requirement: exported targets require C++23; strict-mode core build
  passed. Commit `fe7d10b566`.
- LLVM 22 formatting: 1,128 maintained C/C++ files formatted; 1,123 indentation
  headers removed. Formatting and generated freshness passed. C++23 build with
  experimental search, CBS, fault injection, audit, Qt and Gist passed all
  539 selected integrity tests.
- Initial clang-tidy diagnostics are being resolved; the baseline is not yet
  clean. Naming and full extended-check validation remain in progress.
