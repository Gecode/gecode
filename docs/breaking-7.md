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
- [x] Rename identifiers to snake_case and review concatenated and unclear
      names. Update declarations, uses, generators, examples and documentation.
- [x] Enable a useful clang-tidy baseline and resolve diagnostics, including
      experimental configurations and optional modules where available.
- [x] Provide simple formatting and extended-check commands and CI coverage.

## Validation

Use clean CMake builds under `build/`, the existing integrity suite, installed
package consumers, generated-source freshness, and the repository tidy check.
Run formatting and lint checks on maintained code; generated files follow the
same rules through their generators. Broader suites follow changes that can
affect runtime behaviour. Platform-specific checks remain in CI.

## Decisions

- Work stays local; pushing and opening pull requests require explicit approval.
- Version and ABI metadata remain at the branch baseline. Assign the new
  release version and ABI before publishing this breaking release.
- C++ uses four-space indentation and LLVM 22 formatting; see `code-style.md`.
- Functions, variables, and members use snake_case; types retain PascalCase,
  enum constants retain UPPER_CASE, and external API overrides keep their names.

## Other modernization ideas

- Replace bespoke configuration-template processing with CMake's native
  `configure_file` / `#cmakedefine`, checking feature-macro compatibility.
- Audit shipped Boost compatibility headers against the new compiler baseline.
- Consider scoped enums and descriptive constants after naming stabilizes.
- Revisit the `forceinline` portability macro and obsolete `(void)` parameter lists.
- Audit callback and argument-array copying with `performance-unnecessary-value-param`;
  distinguish owned data from cheap variable/view handles.
- Compare custom bit helpers with C++20 bit operations before replacing them.
- Audit empty-array posting paths, including `no_overlap`, for zero-byte
  allocations rejected by the Debug allocator.

## Verified checkpoints

- CMake-only build: 536 integrity tests passed; generator freshness and
  changelog checks passed. Commit `d5b07b083b`.
- C++23 requirement: exported targets require C++23; strict-mode core build
  passed. Commit `fe7d10b566`.
- LLVM 22 formatting: 1,128 maintained C/C++ files formatted; 1,123 indentation
  headers removed. Formatting and generated freshness passed. C++23 build with
  experimental search, CBS, fault injection, audit, Qt and Gist passed all
  539 selected integrity tests.
- API naming and focused Clang diagnostic fixes: full optional-feature Debug
  build passed. Commit `46b6934132`. The final LLVM 22 lint pass checked all
  479 configured translation units, owned grammar actions, and the parser
  header without diagnostics.
- The full extended command passed in the optional-feature Debug build:
  563 integrity cases, 21 fault-injection cases, all eight CTest entries, both
  installed-package fixtures and their lints, generated freshness, changelog
  checks, and Doxygen 1.18 documentation.
- The full standard and sweep stress suites remain explicit long-running
  targets. Windows and Qt5 branches were reviewed but not executed locally.
