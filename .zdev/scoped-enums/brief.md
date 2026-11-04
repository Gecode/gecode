# Scoped enums and descriptive constants

## Objective

Public enums whose values should not mix with integers or with other enums
become `enum class` in Gecode 7. Abbreviated constants whose meaning is
unclear get clearer names. The user approves the inventory before any API
changes.

Success:

- An inventory covers every unscoped public enum in the installed headers. For
  each it records the decision (scope it or keep it unscoped) and the reason,
  plus the arithmetic and bitwise operations that callers rely on. Examples of
  such operations are `IPL_BASIC | IPL_ADVANCED`, `ExecStatus` comparisons,
  and enums used as array indices.
- The approved enums are scoped. All uses in `gecode/`, `examples/`, `test/`,
  and `tools/` compile, including the output of the generators
  (`misc/genvarimp.py` and the FlatZinc grammar actions).
- Approved constant renames are applied with no compatibility aliases, as with
  the earlier `IRT_LE` → `IRT_LT` rename.
- `docs/code-style.md` states when new enums are scoped. The changelog lists
  every changed enum and constant.

## Boundaries and non-goals

- Owned: public and internal enum declarations, their uses, the generators,
  `docs/code-style.md`, and `changelog.in`.
- `ModEvent`, `PropCond`, and `ModEventDelta` are `int` typedefs with `ME_*`
  and `PC_*` constants. They stay integers, because they are combined
  arithmetically.
- The UPPER_CASE vocabulary is settled (see `docs/breaking-7.md`, Decisions).
  `ES_FIX`, `IRT_EQ`, `ME_INT_VAL`, and `INT_VAR_SIZE_MIN()` keep their
  spelling unless the user approves a specific rename.
- Leave the enums in third-party code, and the internal enums that are
  already `enum class`, unchanged.

## Terms and settled decisions

- Scoped public enums keep their constant spellings through `using enum`
  (C++20) at the scope of the enum:
  `enum class IntRelType { IRT_EQ, ... }; using enum IntRelType;`. Existing
  code such as `rel(home, x, IRT_EQ, y)` still compiles. Implicit conversion
  to `int` and mixing with other enums stop. The user approved this spelling.
- The first task is a read-only inventory. For each enum it recommends whether
  to scope it, based on how its values are used. No enum is changed before the
  user approves the inventory.

## Open questions

The user decides this after the inventory.

- Which constants count as unclear? Candidates come from the inventory, such
  as single-letter or numeric-suffix names. The user approves a concrete list.

## Testing

Existing checks only. The change is a compile-time change with no change to
behaviour. The compiler, the integrity suite, and the installed-package
consumer fixtures cover it.

- Add a static check that implicit conversion to `int` is
  rejected, but only if a later regression there would be silent.
  `static_assert(!std::is_convertible_v<IntRelType, int>)` in an existing test
  file is enough.

## Background

- [Enum inventory](background/enum-inventory.md): every unscoped enum in the
  installed headers, with how the header set was derived, a compiler probe
  per enum that lists the operations scoping blocks and their call sites,
  the operators and explicit conversions each needs, and a scope-or-keep
  recommendation with its reason. It covers ExecStatus, SpaceStatus,
  IntPropLevel's bitwise use, the FlatZinc grammar, and the genvarimp
  output, lists candidate constant renames for task 002, and records the
  commands and scripts that reproduce every count.

## Validation

- `uv run misc/check.py all --build-dir <debug-full-dir> --jobs N`, including
  regeneration through `.github/ci/generated-source-freshness.sh` when a
  generator changes.
- This area touches the API broadly. Implement it after the
  `config-header`, `empty-arrays`, `value-params`, and `boost` work is
  committed, so that the inventory covers their code.
