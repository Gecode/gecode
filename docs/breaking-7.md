# Gecode 7 modernization work

This branch makes CMake the only build system, establishes checked formatting
and naming conventions, and moves the code to C++23 with Clang lint checks.
Each stage is committed locally after focused validation.

## Stages

- [x] Remove Autoconf, maintained Makefiles, their helpers and CI cases.
      Preserve CMake builds, installation, and generated-source maintenance.
- [x] Establish clang-format rules from representative Gecode code. Compare
      two- and four-space indentation, update EditorConfig, remove modelines,
      and format maintained sources.
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
- C++ uses four-space indentation, a 120-column limit, and LLVM 22 formatting;
  see `code-style.md`.
- Functions, variables, and members use snake_case; types retain PascalCase,
  enum constants retain UPPER_CASE, and external API overrides keep their names.
- Constants, and functions that stand for constant values, keep UPPER_CASE:
  `ME_*`, `PC_*`, `ES_SUBSUMED` and the other status helpers, and branching
  selectors such as `INT_VAR_SIZE_MIN()`. They form one vocabulary with enum
  constants such as `ES_FIX` and `IRT_EQ`; lowercasing them made propagator
  code mix both spellings and changed every model without a readability gain.
- Public enums are `enum class`, and `using enum` keeps their constants usable
  without qualification. Abbreviated constants have descriptive names, with no
  compatibility aliases; see `code-style.md`.

## Other modernization ideas

- Replace bespoke configuration-template processing with CMake's native
  `configure_file` / `#cmakedefine`, checking feature-macro compatibility.
- Revisit the `forceinline` portability macro and obsolete `(void)` parameter lists.
- Continue auditing callback, branch-configuration, and shared-handle copying
  with `performance-unnecessary-value-param`; distinguish owned data and
  atomic reference counts from shallow variable/view handles.
- Compare custom bit helpers with C++20 bit operations before replacing them.
- Audit empty-array posting paths, including `no_overlap`, for zero-byte
  allocations rejected by the Debug allocator.

## Verified checkpoints

The branch was rebased onto the 6.5.0 release. Formatting was rerun on the
release tree rather than replayed, and the later commits were applied as diffs
between formatted trees. Release code that is new since the original branch
(the instance reader, rewritten examples) was moved to the renamed API and the
lint profile.

- Each commit builds and passes the integrity suite.
- The full extended command passed in a Debug build with experimental search,
  CBS, fault injection, audit, Qt and Gist: formatting of 1,138 files, the
  changelog and generated-source checks, lint of 486 translation units with
  the grammar actions and parser header, all eight CTest entries including the
  fault suite, both installed-package fixtures, and Doxygen 1.18.
- The full standard and sweep stress suites remain explicit long-running
  targets. Windows and Qt5 branches were reviewed but not executed locally.
- Before the rebase, counted callback probes showed branch callback copies
  reduced from three to one and open-variable-list callback copies from five
  to two, with unchanged example output.
