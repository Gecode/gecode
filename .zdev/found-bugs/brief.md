# Bugs found during the Gecode 7 areas

## Objective

The user asked on 2026-11-04 to fix the bugs that earlier areas found and
recorded. Each task fixes one related group, adds the smallest test that fails
without the fix where the framework can express it, and records in its result
why a fix has no test when it cannot.

## Boundaries and non-goals

- Owned: the code each task names, its tests, Doxygen beside it, and
  `changelog.in` for user-visible changes.
- Scoping enums and renaming constants belong to `scoped-enums`. Do not start
  them here. The `IntPropLevels` signedness that a Microsoft ABI build may show
  is fixed there by the unsigned underlying type.
- Version and ABI numbers stay as they are.

## Terms and settled decisions

- `BlackBoxProcessSession`'s `GECODE_HAS_THREADS` guard is not a bug (user,
  2026-11-04): one cannot link code built with and without threads.
- `gecode-test` staying out of `ALL`, and the 26 cloning constructors kept with
  `NOLINT`, are not bugs and are not touched.
- A new argument check throws an existing exception type and is documented
  beside the declaration.

## Testing

Focused coverage. Use the fixture of the most similar existing test, and run a
focused selection, then `check-standard` in proportion to the change.

## Validation

- `uv run misc/check.py all --build-dir build/full --jobs 10` before the last
  commit of the area.
