+++
schema_version = 1
id = "empty-arrays-007"
key = "open-distinct"
area = "empty-arrays"
status = "open"
complexity = "standard"
afk = true
blocked_by = []
+++
# Skip empty buffers in open-list distinct

## Outcome

distinct(Home, OpenIntVarList, IntPropLevel) makes no zero-size Region request when the list has no active positions, and posts nothing when the list is closed and empty, with unchanged results for every other list.

## Context

Found by the empty-arrays-002 audit rework. distinct(Home, OpenIntVarList, IntPropLevel) (gecode/int/distinct.cpp) posts Int::Distinct::Open<IntView>. Its propagate() (gecode/int/distinct/open.hpp, around the `Region region; const int capacity = active_count;` block) allocates `current` and `next` with region.alloc<int>(capacity) twice. With no active positions capacity is 0, so each propagation makes two zero-size Region requests. This happens for an open list created with maximum length 0, for an unbounded list before any position exists (the usual initial state of every open list), and with IPL_DOM. The result is already correct (SS_SOLVED), and Debug does not abort because Region::ralloc has no size assertion. For a list with maximum length 0 the propagator is still posted although the constraint is decided.

Fix: skip the Region buffers when there is nothing to process (capacity 0), keeping the propagator's later behaviour for lists that grow. Post nothing at posting when the list is already closed and empty, if the open-list API exposes that cheaply; check how other open-list posting functions (rel, precede, sequence, extensional, min, max in the int module) decide closed lists and follow the same idiom. Keep the propagator subscribed and correct for an open list that is still empty, since positions can be added later.

Testing follows the brief (focused coverage). The existing open-list distinct tests are in test/int/open-var-list.cpp; add a small Test::Base `Empty`-style test there (or the file's own fixture if it can express it) covering an open list with maximum length 0 (solved, nothing posted), an unbounded list before any position (solved, then extended with positions that must be distinct, including a failing duplicate), and IPL_DOM. Because the original path already gives the correct result, the test passes before the fix; record the zero-size request counts before (2 per propagation) and after (0) from a fresh instrumented build instead, and add no allocator instrumentation to tracked code or tests.

The throwaway driver (build/zdev-probe/probe.cpp, probes.inc) already has open-list distinct probes; build/zdev-instr-006 is HEAD plus the three-site zero-size instrumentation and build/zdev-probe/probe-instr-006 links against it. To check the fix, archive the edited tree into a fresh copy with the same patch (as build/zdev-instr-006 was made) and rerun the open-list distinct probes; never put the patch in tracked code. Append a short section for this child to build/zdev-audit/empty-array-audit.md in the format of the earlier child sections.

## Boundaries

- Fix at the posting site or its propagator: decide the empty constraint at posting where it is decided, and otherwise skip the zero-size buffers. Keep existing argument checks and GECODE_POST first.
- Do not change the allocator or the kernel, and add no branch to the allocation fast path.
- Do not change behaviour for non-empty input or for open lists that grow after posting.
- Do not edit changelog.in or anything under .zdev. The parent task empty-arrays-002 owns the audit document, the changelog entries and the final check.py all.
- Initial task-owned path allocation: ["gecode/int/distinct/open.hpp","gecode/int/distinct.cpp","test/int/open-var-list.cpp"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [ ] The open-list distinct probes (maximum length 0, unbounded before any position, IPL_DOM) give the same results as before in build/full and show no zero-size request in a fresh instrumented build.
- [ ] A focused test in test/int/open-var-list.cpp documents the empty-list results: a closed empty list is solved with nothing posted, and an empty open list still enforces distinctness once positions are added.
- [ ] build/zdev-audit/empty-array-audit.md has a section for this child with method, results and zero-size request counts before and after.

## Validation

- In build/full (Debug, audit): build gecode-test, run the new test by exact name, then `bin/gecode-test -threads 0 -test OpenVarList -tag standard` (check the actual test-name prefix in test/int/open-var-list.cpp) and `-threads 0 -test Int::Distinct -tag standard`.
- Rerun the open-list distinct probes against build/full and against a fresh instrumented build of the edited tree.
- Release build in build/local and the same selections.
- Run `uv run misc/check.py format-check --build-dir build/full` and `uv run misc/check.py lint --build-dir build/full --jobs 12`.
