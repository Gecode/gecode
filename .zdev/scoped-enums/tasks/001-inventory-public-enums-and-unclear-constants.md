+++
schema_version = 1
id = "scoped-enums-001"
key = "inventory"
area = "scoped-enums"
status = "open"
complexity = "advanced"
afk = true
blocked_by = ["config-header-003", "empty-arrays-002", "value-params-003", "boost-005"]
+++
# Inventory public enums and unclear constants

## Outcome

An inventory of every unscoped enum in the installed headers records, for each one, how its values are used and a recommendation to scope it or keep it unscoped. It also lists candidate constant renames for names whose meaning is unclear.

## Context

The brief settles the spelling (`enum class` plus `using enum` at the enclosing scope) and what stays integer (ModEvent, PropCond, ModEventDelta). This task waits for the other areas' terminal tasks so that the inventory covers their code. Derive the header set from the install rules in CMakeLists.txt (`install(DIRECTORY gecode ...)` and the other install rules), including the generated config and variable headers and the installed test-component headers. Exclude third-party code and upstream-generated scaffolding, as the brief requires. Record why each discovered enum is public or out of scope. For each enum, record the operations that an enum class would block, with call sites: implicit conversion to int, arithmetic, bitwise combination (IntPropLevel is combined by callers and masked as an integer in gecode/int/ipl.hpp), use as an array index, and use in the FlatZinc grammar actions or misc/genvarimp.py output. Same-enum comparison and switch work unchanged with enum class. For each enum to scope, propose the operator signatures and result types to add, and the conversions that become explicit casts. ExecStatus and SpaceStatus in gecode/kernel/core.hpp need care.

## Boundaries

- Read-only for source. Commit only the background file and the brief index.

## Done when

- [ ] .zdev/scoped-enums/background/enum-inventory.md has one row per unscoped public enum with its location, the blocked operations and their call sites, the proposed operator signatures and results, the conversions that become explicit, and the recommendation. It also has a separate list of candidate constant renames with reasons, and the brief indexes it.

## Validation

- The commands used to find enums and count uses are recorded so the counts can be reproduced.
