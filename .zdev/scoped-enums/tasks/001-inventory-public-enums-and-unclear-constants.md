+++
schema_version = 1
id = "scoped-enums-001"
key = "inventory"
area = "scoped-enums"
status = "done"
complexity = "advanced"
afk = true
blocked_by = ["config-header-003", "empty-arrays-002", "value-params-003", "boost-005", "empty-arrays-011"]
+++
# Inventory public enums and unclear constants

## Outcome

An inventory of every unscoped enum in the installed headers records, for each one, how its values are used and a recommendation to scope it or keep it unscoped. It also lists candidate constant renames for names whose meaning is unclear.

## Context

The brief settles the spelling (`enum class` plus `using enum` at the enclosing scope) and what stays integer (ModEvent, PropCond, ModEventDelta). This task waits for the other areas' terminal tasks so that the inventory covers their code. Derive the header set from the install rules in CMakeLists.txt (`install(DIRECTORY gecode ...)` and the other install rules), including the generated config and variable headers and the installed test-component headers. Exclude third-party code and upstream-generated scaffolding, as the brief requires. Record why each discovered enum is public or out of scope. For each enum, record the operations that an enum class would block, with call sites: implicit conversion to int, arithmetic, bitwise combination (IntPropLevel is combined by callers and masked as an integer in gecode/int/ipl.hpp), use as an array index, and use in the FlatZinc grammar actions or misc/genvarimp.py output. Same-enum comparison and switch work unchanged with enum class. For each enum to scope, propose the operator signatures and result types to add, and the conversions that become explicit casts. ExecStatus and SpaceStatus in gecode/kernel/core.hpp need care.

## Boundaries

- Read-only for source. Commit only the background file and the brief index.

## Done when

- [x] .zdev/scoped-enums/background/enum-inventory.md has one row per unscoped public enum with its location, the blocked operations and their call sites, the proposed operator signatures and results, the conversions that become explicit, and the recommendation. It also has a separate list of candidate constant renames with reasons, and the brief indexes it.

## Validation

- The commands used to find enums and count uses are recorded so the counts can be reproduced.

## Result

background/enum-inventory.md inventories the 80 unscoped enum declarations in the 636 installed headers: 77 named enums, each probed by scoping it and recompiling every dependent TU, plus 2 anonymous holders and Bison's yytokentype out of scope. It recommends scoping 68 (53 compile unchanged, 15 with local fixes) and keeping 9 bit-set enums, including TraceEvent. ExecStatus and SpaceStatus can be scoped; IntPropLevel needs an unsigned underlying type and |, &, |= operators; two enums inside class templates need static constexpr aliases instead of using enum. It lists candidate constant renames with reasons and use counts, and findings outside the enum question (test/float.cpp:722, val-commit.hpp:130, test/set/mm-set.cpp str(srt0), possible MSVC IntPropLevels signedness). The brief indexes it.

Validation:

- Installed header set from cmake --install equals the set derived from the install rules (636). grep and clang-query agree on all 80 declarations over 490 TUs.
- 77 per-enum compile probes plus patched re-probes; reproduction commands, scripts and patches embedded in the document.
- Fresh verifier (gpt-6.1-sol high) FAIL with three findings (ev_t excluded, reproduction inputs missing, rename counts); one correction; focused recheck PASS.
