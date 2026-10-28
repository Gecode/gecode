# Gecode 7 modernization work

This branch makes CMake the only build system, establishes checked formatting
and naming conventions, and moves the code to C++23 with Clang lint checks.
Each stage is committed locally after focused validation.

## Stages

- [ ] Remove Autoconf, maintained Makefiles, their helpers and CI cases.
      Preserve CMake builds, installation, and generated-source maintenance.
- [ ] Establish clang-format rules from representative Gecode code. Compare
      two- and four-space indentation, update EditorConfig, remove modelines,
      and format maintained sources. Preserve third-party source conventions.
- [ ] Require C++23 through exported CMake targets. Use broadly implemented
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
- Naming scope and indentation choice are being resolved before bulk edits.

## Other modernization ideas

- Replace bespoke configuration-template processing with CMake's native
  `configure_file` / `#cmakedefine`, checking feature-macro compatibility.
- Remove Flex/Bison installation from CI jobs that only consume generated code.
- Audit shipped Boost compatibility headers against the new compiler baseline.
- Consider scoped enums and descriptive constants after naming stabilizes.
- Revisit the `forceinline` portability macro and obsolete `(void)` parameter lists.
