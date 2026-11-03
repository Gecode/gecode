# Empty-array posting

## Objective

Every public posting function that accepts arrays handles empty arrays in
Debug and Release builds. It either posts nothing, because the constraint is
trivially satisfied, or fails or throws exactly as documented. No call path
reaches a zero-byte space allocation.

Second stage, decided by the user on 2026-11-03: the heap gets the same
contract as space memory. Every requested allocation size passed to the
heap (`Heap::ralloc`, `Heap::rrealloc`, and the typed `alloc` and the new
count `m` of `realloc`) is greater than zero, and a Debug-only assertion in
`Heap::ralloc` and `Heap::rrealloc` enforces it. A typed `realloc` from an
old count of zero to a positive new count stays valid; open-list growth does
this. No code path passes a null pointer to `memcpy`, `memmove`, or
`memset`, whatever the length. The known zero-size heap requests (the
element buffers of kernel `Action` and `CHB` storage on empty arrays, DFA
and `TupleSet(0)`, the empty-list `IntSet`, and copying or assigning an
empty `Archive`, gecode/kernel/archive.cpp:44-53) and any others the static
audit, the assertion, or UBSan finds are fixed at their sites first, so the
assertion lands with every suite passing. `Heap::rrealloc` currently accepts
a null result when the size is zero (heap.hpp:411); the contract removes that
allowance.

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

Second-stage ownership: the `Heap` interface and its Doxygen in
`gecode/support/heap.hpp`, the kernel `Action` and `CHB` storage and
recorders in `gecode/kernel/branch/`, and any library or tool site that
makes a zero-size heap request or passes a null pointer with a zero length
to a C library function, with matching tests and `changelog.in`.

## Terms and settled decisions

- Standards compliance (user, 2026-11-03): rely on nothing that the C or
  C++ standard leaves implementation-defined or undefined. C++23 takes the C
  library from C17:
  - `malloc(0)` is implementation-defined: it may return a null pointer,
    which `Heap::ralloc` would turn into a spurious `MemoryExhausted`.
  - `realloc(p, 0)` with a non-null `p` is implementation-defined in C17 and
    undefined in C23, and `realloc(nullptr, 0)` behaves like `malloc(0)`. So
    every zero-size reallocation is avoided, whatever the pointer.
  - Passing a null pointer to `memcpy`, `memmove`, or `memset` is undefined
    even with length zero in C17 (C2y relaxes this; Gecode does not rely on
    it).
- Heap contract, the user's option 2 of 2026-11-03: sizes greater than zero,
  checked by a Debug-only assertion. The heap does not learn to accept zero
  (that would hide bugs), and Release builds get no extra branch.
- Empty `Action` and `CHB` (the user's decision, same date): an action or CHB
  object over an empty array stays initialized with size zero. Its shared
  `Storage` object is still allocated as today (a non-zero request, and what
  "initialized" means for a `SharedHandle`), but it requests no element
  buffer, frees none, and posts no recorder propagator. Copying, size, and
  decay keep working.

- Fix at the posting site. Gecode's propagator rules already say "post
  nothing when the constraint is already decided", and an empty `no_overlap`,
  `distinct`, or `linear` sum is decided at posting time. Keeping the
  allocator's `sz > 0` assertion keeps it catching real bugs.
- `ViewArray` constructed with size zero must not allocate, and a
  zero-length `SharedArray` allocates no element buffer while keeping its
  initialized shared object (`new SAO(0)`, shared-array.hpp:247). Check their
  constructors before changing callers that rely on them.

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
- First stage: a zero-size `Region` request that already gives the correct
  result cannot fail a test, because `Region::ralloc` has no size assertion.
  Record the before and after allocation sizes from a source trace in the
  audit instead. (The second stage below uses a scratch heap assertion for
  heap requests.)
- Run focused tests with `-test <pattern>`, then `check-standard` in Debug.

## Background

- [Empty-array posting audit](background/empty-array-audit.md): which public
  posting functions were checked for empty arrays, how, and what each does
  now; the FlatZinc and instance-reader findings; and the zero-size requests
  recorded but not fixed.

Second stage: once the heap assertion lands, every Debug suite run checks
the contract. The evidence differs by kind of site:

- A zero-size heap request needs a test that reaches it, so that the
  assertion fails without the fix; reuse an existing test where one already
  reaches the site. Before the assertion lands, show this with the assertion
  applied locally.
- A null pointer passed to a C library function is shown by a UBSan run
  (`GECODE_SANITIZER=undefined`) with a test that reaches it. A UBSan run
  that finds nothing is recorded as such.
- A zero-size `Region` request (for example in DFA construction) never
  reaches the heap and cannot fail a test; as in the first stage, record the
  before and after sizes from a source trace.

## Validation

- Debug build with audit enabled (`build/full` configuration) and `ctest`.
- `uv run misc/check.py all --build-dir <debug-full-dir> --jobs N` before the
  last commit.
