+++
schema_version = 1
id = "empty-arrays-011"
key = "heap-contract"
area = "empty-arrays"
status = "open"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["empty-arrays-009", "empty-arrays-010"]
+++
# State and assert the heap size contract

## Outcome

Heap's Doxygen states that every requested allocation size is greater than zero and that no null pointer reaches memcpy, memmove, or memset, rrealloc no longer accepts a null result for size zero, and a Debug-only assertion in Heap::ralloc and Heap::rrealloc enforces the size contract.

## Context

This lands the contract after every known site is fixed. Write it on Heap::alloc, realloc (the new count m is positive; the old count n may be zero), copy, ralloc, and rrealloc in gecode/support/heap.hpp, next to the wording the first stage gave Space::alloc and MemoryManager::alloc; remove the `|| s == 0` allowance in Heap::rrealloc (heap.hpp:411); and add the assertion with no Release cost. Add a changelog.in entry under 7.0.0: code that asks the heap for zero bytes now fails an assertion in Debug builds.

## Boundaries

- No branch in Release builds, and the heap does not start accepting zero.

## Done when

- [ ] The assertion is committed, and check-standard passes in build/full.
- [ ] check-standard in a GECODE_SANITIZER=undefined build reports no nonnull or null-pointer finding.
- [ ] The Doxygen and changelog entries describe the contract.

## Validation

- check-standard in build/full and in a GECODE_SANITIZER=undefined build.
- uv run misc/check.py all --build-dir build/full --jobs N as the area's last task.
