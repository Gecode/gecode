+++
schema_version = 1
id = "empty-arrays-008"
key = "heap-audit"
area = "empty-arrays"
status = "open"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Find zero-size heap requests and null-pointer library calls

## Outcome

A record lists every zero-size heap request, zero-size reallocation, and null pointer passed to memcpy, memmove, or memset that a bounded static audit, the test suites with a scratch heap assertion, or a UBSan run finds, with the site, the input that reaches it, and the proposed fix.

## Context

Read the brief's second stage, its Terms, and its Testing paragraphs on the three kinds of evidence. The empty-array audit's 'Recorded, not fixed' section (background/empty-array-audit.md) and the brief name the known sites: the element buffers of kernel Action and CHB storage (gecode/kernel/branch/action.hpp, chb.hpp), DFA::init and TupleSet(0), the empty-list IntSet, and copying or assigning an empty Archive (gecode/kernel/archive.cpp:44-53). Static audit first, bounded to the wrappers and their callers: every caller of Heap::alloc, realloc, copy, ralloc, and rrealloc and of Support::allocator.alloc, realloc, and memcpy (gecode/support/heap.hpp, allocator.hpp), the SharedArray, ViewArray, ArgArray, and IntSet constructors, Region's heap fallback, and every direct memcpy, memmove, memset, malloc, or realloc call outside gecode/third-party. For each, record whether a zero size or a null pointer can reach it and from which public input. Then runtime discovery: apply a Debug-only check (s > 0) to Heap::ralloc and Heap::rrealloc in a scratch patch that logs the call site instead of aborting, and run check-standard in a Debug build with experimental, CBS, and Gist; separately configure with GECODE_SANITIZER=undefined and run check-standard, recording every nonnull or null-pointer report (zero reports is a valid, recorded result). For each site record the source location, the call path, a minimal input, whether an existing test reaches it, the kind of evidence (heap assertion, UBSan, or Region source trace), and a fix in the brief's style (no zero request; documented exceptions kept). Write .zdev/empty-arrays/background/heap-zero-size.md and index it in the brief. Do not propose separate tasks: the other-sites task owns every site except the Action and CHB element buffers.

## Boundaries

- No tracked source changes; the scratch assertion and builds stay under build/.

## Done when

- [ ] .zdev/empty-arrays/background/heap-zero-size.md lists every site found by the static audit and the runtime runs with its call path, minimal input, existing test, evidence kind, and proposed fix, plus the exact scratch patch and commands, and the brief indexes it.
- [ ] The record lists the static audit's scope (functions and directories covered) so it can be checked for completeness.

## Validation

- Rerun the recorded commands for one heap-assertion site and confirm it reproduces; if the UBSan run found a site, rerun it too.
