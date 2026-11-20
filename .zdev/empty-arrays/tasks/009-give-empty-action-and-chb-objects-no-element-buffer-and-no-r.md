+++
schema_version = 1
id = "empty-arrays-009"
key = "action-chb"
area = "empty-arrays"
status = "done"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = []
+++
# Give empty Action and CHB objects no element buffer and no recorder

## Outcome

Action and CHB objects over an empty array, and branching with ACTION or CHB merits on an empty array, request and free no element buffer and post no recorder propagator, while staying initialized with size zero and copyable; an empty Action still accepts a decay change.

## Context

Action::Storage (gecode/kernel/branch/action.hpp:272, destructor gecode/kernel/branch/action.cpp:39) and CHB::Storage (gecode/kernel/branch/chb.hpp:257, destructor gecode/kernel/branch/chb.cpp:39) call heap.alloc with x.size() and heap.free with n; Action::init and CHB::init post their Recorder unconditionally (chb.hpp:339-347 and the matching Action code). The brief's Terms settle the result: the shared Storage object is still allocated (it is what makes the handle initialized, and Action keeps its decay there, action.hpp:344), but its element buffer is not. Guard the free as well as the allocation: under GECODE_PEAKHEAP, Heap::rfree measures the block, and MSVC's _msize rejects a null pointer. Check every member that reads the buffer (size(), operator[], update, decay, rescaling) for n = 0. The empty-array audit lists the posting paths (IntAction, BoolAction, SetAction, FloatAction, the CHB classes, branch with ACTION or CHB merits, and open lists with a maximum length of 0) and their existing tests in test/branch.cpp and the open-list tests.

## Boundaries

- Non-empty behaviour and decay semantics stay unchanged.

## Done when

- [x] With the heap-audit task's scratch assertion applied locally (or the same s > 0 check if that task has not run yet), tests for empty Action and CHB objects and for ACTION/CHB branching on empty arrays pass, and they fail without this fix.
- [x] A test checks that an empty Action and an empty CHB are initialized, report size 0, can be copied, and that posting them adds no propagator, and that an empty Action accepts a decay change (CHB has no public decay API).

## Validation

- bin/gecode-test -test Branch:: and the open-list tests in build/full, then check-standard in build/full.

## Result

Empty Action and CHB keep their shared Storage but allocate and free no element buffer and post no recorder; EmptyStatistics tests for Int, Bool, Set, Float branching and the open list; changelog entry.

Validation:

- build/full: Branch:: 21/21, OpenList 61/61, check-standard 6808/6808; new tests fail without the fix under the scratch s > 0 check; verifier: EmptyStatistics 5/5, OpenList 61/61
