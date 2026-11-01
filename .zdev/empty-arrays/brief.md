# Empty-array posting

## Objective

Every public posting function that accepts arrays handles empty arrays in
Debug and Release builds. It either posts nothing, because the constraint is
trivially satisfied, or fails or throws exactly as documented. No call path
reaches a zero-byte space allocation.

Reproduction at the area base, in a Debug build (`build/full`):

```cpp
IntVarArgs x, y; IntArgs w, h;
no_overlap(home, x, w, y, h);   // Assertion failed: (sz > 0), manager.hpp:289
```

`Space::alloc<T>(n)` forwards `sizeof(T) * n` to `MemoryManager::alloc`, which
asserts `sz > 0`. Release builds skip the assertion and hand out a zero-sized
block. That happens to work, but it relies on unspecified allocator behaviour.

Success:

- `no_overlap` with empty arrays posts nothing in every overload in
  `gecode/int/no-overlap.cpp`.
- An audit lists every public posting function in the int, set, float, and
  minimodel modules whose path can reach `alloc`, `ralloc`, `Region::alloc`, or
  a `ViewArray`/`SharedArray` allocation with a size of zero. Every listed path
  is fixed.
- The allocator contract (sizes greater than zero) is written in the Doxygen
  comments of `Space::alloc`/`ralloc` and `MemoryManager::alloc`.

## Boundaries and non-goals

- Owned: posting functions and their propagator post helpers in `gecode/int`,
  `gecode/set`, `gecode/float`, and `gecode/minimodel`; the Doxygen comments
  for the allocation functions in `gecode/kernel`; the matching tests; and
  `changelog.in`.
- Do not make the allocator accept zero sizes, and add no branch to the
  allocation fast path.
- Do not change behaviour for non-empty input. Where the documented result for
  empty input is an exception, as for `element` with an empty array, keep it.
- FlatZinc and the instance reader only need checking. Fix them in this area
  only if they reach a failing posting path with input that a model can
  produce.

## Terms and settled decisions

- Fix at the posting site. Gecode's propagator rules already say "post
  nothing when the constraint is already decided", and an empty `no_overlap`,
  `distinct`, or `linear` sum is decided at posting time. Keeping the
  allocator's `sz > 0` assertion keeps it catching real bugs.
- `ViewArray` and `SharedArray` constructed with size zero must not allocate.
  Check their constructors before changing callers that rely on them.

## Testing

Focused coverage. The bug is an abort in Debug builds on a specific input
shape, so each fixed posting family needs one regression test.

- Put each test next to the posting family's existing test, using its fixture
  (`Test::Int::Test` and its relatives). Where a fixture cannot express arity
  zero, add a small `Test::Base` test that posts on an empty `Space`, calls
  `status()`, and checks the documented result (`SS_SOLVED`, `SS_FAILED`, or
  an exception).
- Where the original path aborts or gives a wrong result, its test fails
  before the fix in a Debug build. Release passing before the fix is expected.
- A zero-size `Region` or heap request that already gives the correct result
  cannot fail a test, because `Region::ralloc` has no size assertion. Record
  the before and after allocation sizes from a source trace in the audit
  instead. Do not add allocator instrumentation only to make a test fail.
- Run focused tests with `-test <pattern>`, then `check-standard` in Debug.

## Background

- [Empty-array posting audit](background/empty-array-audit.md): which public
  posting functions were checked for empty arrays, how, and what each does
  now; the FlatZinc and instance-reader findings; and the zero-size requests
  recorded but not fixed.

## Validation

- Debug build with audit enabled (`build/full` configuration) and `ctest`.
- `uv run misc/check.py all --build-dir <debug-full-dir> --jobs N` before the
  last commit.
