+++
schema_version = 1
id = "scoped-enums-003"
key = "scope-enums"
area = "scoped-enums"
status = "open"
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

- [ ] Every approved enum is scoped with the approved operator contract, and existing spellings such as IRT_EQ still compile through using enum.
- [ ] test/ has a static_assert that a scoped public enum, for example IntRelType, does not convert to int.
- [ ] docs/code-style.md says when new enums are scoped, and changelog.in lists the scoped enums under 7.0.0.

## Validation

- Build build/full and the default Release configuration and run ctest.
- .github/ci/generated-source-freshness.sh when a generator changed.
