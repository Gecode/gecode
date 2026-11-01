+++
schema_version = 1
id = "empty-arrays-001"
key = "no-overlap"
area = "empty-arrays"
status = "open"
afk = true
priority = "high"
blocked_by = []
+++
# Post nothing for empty no_overlap

## Outcome

Every no_overlap overload in gecode/int/no-overlap.cpp returns without posting or allocating when given zero boxes, after it has checked the argument sizes. The allocator's sizes-greater-than-zero contract is documented in Doxygen.

## Context

The brief has the reproduction: `no_overlap(home, IntVarArgs(), IntArgs(), IntVarArgs(), IntArgs())` aborts in a Debug build with `Assertion failed: (sz > 0)` at gecode/kernel/memory/manager.hpp:289. It comes through `Space::alloc<T>(0)` (gecode/kernel/core.hpp) into `MemoryManager::alloc`. The fixed-dimension overloads allocate ManBox/OptBox arrays with home.alloc(n_boxes). The flexible-dimension overloads delegate to the fixed-dimension ones when all widths and heights are assigned, which includes the empty case, and otherwise allocate their own box arrays. The existing tests are in test/int/no-overlap.cpp and use Test::Int::Test with arity computed from the box count.

## Boundaries

- Do not make the allocator accept zero sizes.
- Keep the size-mismatch checks and their exceptions in front of the new empty-input return.

## Done when

- [ ] Posting each no_overlap overload with empty arrays in a Debug build gives a space whose status() is SS_SOLVED and does not abort.
- [ ] A regression test next to the existing no_overlap tests covers the empty case of each overload and fails before the fix in a Debug build.
- [ ] Doxygen comments on Space::alloc, Space::ralloc, and MemoryManager::alloc state that sizes must be greater than zero.

## Validation

- In build/full (Debug, audit): run the new regression test by its exact name before and after the fix, then `bin/gecode-test -test NoOverlap -tag standard`.
- Build the default Release configuration and run the same tests.
