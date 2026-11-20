+++
schema_version = 1
id = "scoped-enums-003"
key = "scope-enums"
area = "scoped-enums"
status = "done"
complexity = "advanced"
afk = true
blocked_by = ["scoped-enums-002"]
+++
# Scope the approved enums

## Outcome

Each approved enum is an enum class with `using enum` at its enclosing scope and only the operators that the inventory found to be needed. All code in gecode/, examples/, test/, and tools/, including generator output, compiles.

## Context

Work from the approved list in the brief and background/enum-inventory.md. If the list is large, split this task before editing into compiling groups through the derived-work contract, following any groups recorded in the brief. Regenerate through the CMake regenerate target if misc/genvarimp.py or the grammar changes.

## Boundaries

- Do not rename constants in this task. That is the next task.

## Done when

- [x] Every approved enum is scoped with the approved operator contract, and existing spellings such as IRT_EQ still compile through using enum.
- [x] test/ has a static_assert that a scoped public enum, for example IntRelType, does not convert to int.
- [x] docs/code-style.md says when new enums are scoped, and changelog.in lists the scoped enums under 7.0.0.

## Validation

- Build build/full and the default Release configuration and run ctest.
- .github/ci/generated-source-freshness.sh when a generator changed.

## Result

All 68 approved enums are scoped with using enum or static constexpr aliases; IntPropLevel alone gains operators, IPL_BITS_ becomes a private constexpr width, and ev_t becomes EventType; the 9 kept enums are unchanged.

Validation:

- check.py all exit 0 (format, tidy, freshness, lint 489/489, CTest 8/8, packages, Doxygen with empty doxygen.log); build/full and build/local 0 errors, CTest 8/8 and 7/7; gecode-test -list unchanged
- GCC 15.2 and Clang 21.1 on erebor: build and ctest 7/7, GCC warnings identical to HEAD; gecode-test -threads 8 -iter 2 -fixprob 1 -tag check 546/546; MSVC, MinGW, -m32, Qt 5 are CI-only and not yet run
