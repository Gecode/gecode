# Parameter passing audit

## Objective

Parameters are passed by value only when copying is cheap or the callee keeps
the copy. clang-tidy's `performance-unnecessary-value-param` is enabled in
`.clang-tidy`, so the policy stays enforced.

Commit `9396a93c24` on the base branch already cut branch-callback copies
from three to one and open-variable-list callback copies from five to two. It
enabled the other performance checks, but not this one.

Success:

- `performance-unnecessary-value-param` is in `.clang-tidy`. Its
  `AllowedTypes` option lists the shallow handle types that stay passed by
  value. `uv run misc/check.py lint` passes.
- Owned or reference-counted values, such as `std::function` callbacks,
  `SharedArray`/`IntSharedArray`, `IntSet`, `TupleSet`, `DFA`, `SharedHandle`
  subclasses, branch configuration objects, `Search::Options`, and strings,
  are passed by `const&`. When the callee stores the value, it takes it by
  value and moves it into place.
- The rule and the `AllowedTypes` list are explained in
  `docs/code-style.md`, next to the other performance-check notes.

## Boundaries and non-goals

- Owned: function signatures and their definitions in `gecode/`, `examples/`,
  `test/`, and `tools/`; `.clang-tidy`; `docs/code-style.md`; and
  `changelog.in`.
- Variable and view handles (`IntVar`, `BoolVar`, `SetVar`, `FloatVar`,
  `IntView` and the other views, `Home`, `Space&`-style handles) stay passed by
  value. They are one or two pointers wide, and Gecode code assumes value
  semantics for them. `AllowedTypes` encodes this.
- Do not change ownership or lifetime. If a parameter is moved, stored, or
  needs its own copy, keep it by value.
- `Args` arrays (`IntArgs`, `IntVarArgs`, ...) are already passed by `const&`
  in the public API. Do not touch them unless the check reports one.
- Signature changes in public headers are accepted for Gecode 7. They change
  the ABI but not source compatibility for callers.
- Do not enable `performance-enum-size`, `performance-noexcept-*`, or other
  checks that `docs/code-style.md` reserves for separate review.

## Terms and settled decisions

- "Shallow handle" means a type whose copy is a pointer-sized copy with no
  reference count or allocation. Variables, views, and `Home` are shallow
  handles. `SharedArray`, `IntSet`, and the other `SharedHandle` types are
  not: copying one touches an atomic reference count.
- Overriding virtual functions follow their base declaration. Change the base
  declaration and all overrides together, or leave all of them.

## Testing

Existing checks only. The change keeps behaviour. The risks are a dangling
reference where a by-value parameter used to protect a lifetime, and missed
moves, and the integrity suite and lint cover both.

- When a parameter changes to `const&` and the callee keeps it beyond the
  call, the callee must store a copy. Review each such case. The verifier
  checks them by name.
- Optionally rerun the counted callback probe used for `9396a93c24` (a
  throwaway copy-counting callback under `build/`) to confirm the counts do
  not regress. Do not commit it.

## Background

- [Value-parameter inventory](background/value-param-inventory.md): the
  clang-tidy command and `--config` that measure the check, the proposed
  `AllowedTypes` value, counts per module, one row per reported parameter
  with its treatment, the traced DFA and `SharedData` cases, and the repair
  groups with their order.
- [Value-parameter results](background/value-param-results.md): the final
  record after the repair groups. It lists every parameter changed to
  `const&` that the callee keeps and how it copies it, every by-value
  parameter with its move, and the inventory rerun showing no allowed
  diagnostics.

## Validation

- `uv run misc/check.py lint --build-dir <dir>` with the check enabled, on a
  build configured with experimental, CBS, and Gist so that optional code is
  linted.
- `uv run misc/check.py all --build-dir <debug-full-dir> --jobs N` before the
  last commit.
