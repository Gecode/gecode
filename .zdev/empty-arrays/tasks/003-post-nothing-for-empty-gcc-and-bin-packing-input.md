+++
schema_version = 1
id = "empty-arrays-003"
key = "gcc-binpack"
area = "empty-arrays"
status = "open"
complexity = "standard"
afk = true
blocked_by = []
+++
# Post nothing for empty GCC and bin-packing input

## Outcome

Every global cardinality count overload and both bin_packing overloads handle empty arrays in Debug and Release without aborting or making a zero-size request: count posts nothing for empty x after forcing every count to 0, and fails for non-empty x with no values; bin_packing with no items constrains every load to 0 and posts nothing.

## Context

Part of the empty-arrays-002 inventory (int globals). Both families abort in build/full on empty input.

A1 GCC count. All 7 count overloads that take cardinalities (probes `count(x,c[]) gcc`, `count(x,IntSetArgs) gcc`, `count(x,c[],v[]) gcc`, its dom and bnd variants, `count(x,IntSetArgs,v[]) gcc`, `count(x,IntSet,v[]) gcc`, all EXIT 134 in out.txt) funnel into two functions in gecode/int/gcc.cpp: the IntVarArgs-cardinality one at :101 (GECODE_POST at :111) and the IntSetArgs-cardinality one at :143 (GECODE_POST at :157). The abort is the out-of-bounds &k[0] passed to quicksort at gecode/int/gcc/post.hpp:62, followed by a Region alloc(0) at :82. It happens for empty v whether x is empty or not. With empty x and non-empty c there is still a Region zero request in post_side_constraints/is_distinct. FlatZinc global_cardinality* reaches these paths (gecode/flatzinc/registry.cpp:1099-1193) and needs no frontend change.
Fix: in both funnels, after the existing checks and GECODE_POST, handle the empty cases before the propagator post helper. For empty x, force every count to 0 (a variable cardinality is constrained to 0 through its CardView; a constant cardinality set fails unless it contains 0) and post nothing. For non-empty x with empty v, fail, since the documentation says no other value occurs (gecode/int.hh:2210-2278).

A2 bin_packing. Pack::post (gecode/int/bin-packing/propagate.cpp:365) sorts &bs[0] at :367 with no items, which is out of bounds. Single-dimensional bin_packing (gecode/int/bin-packing.cpp:58) and the multi-dimensional overload (:107) reach it whenever b is empty. With n==0 the multi-dimensional overload also builds the ConflictGraph and runs maximum_clique (:117), which makes Region zero requests. FlatZinc bin_packing_load reaches it (gecode/flatzinc/registry.cpp:1094).
Fix: Pack::post skips the sort for an empty item array and uses its existing no-items branch (:383), which constrains the loads to 0. The multi-dimensional overload returns IntSet::empty for n==0 after the per-dimension posts, before building the conflict graph.

Recorded, not fixed here: bin_packing(home, d, ...) with d==0 divides by zero at gecode/int/bin-packing.cpp:72; it is not an allocation and needs a semantic choice, so leave it to the parent's audit.

Testing follows the brief (focused coverage). Test::Int::Test and its relatives cannot express arity 0 (CpltAssignment::has_more reads dsv[0], test/int.hpp:55), so each family gets a Test::Base `Empty` test next to its existing tests, modelled on Int::NoOverlap::Empty (test/int/no-overlap.cpp:269-301): an empty fixture Space, the public posting call, and status() compared with the documented result (SS_SOLVED, SS_FAILED, or the documented exception). For a family that aborts, the test must abort before the fix in build/full (Debug with audit; exit 134 is accepted, as in empty-arrays-001) and pass after it. For a family whose result is already correct but which makes a zero-size Region or heap request, the test documents the result and passes before the fix; record the before and after request sizes in the audit draft instead, and add no allocator instrumentation to tracked code or tests.

Inventory artefacts from the parent's probe run are ignored files in the area checkout (/Users/zayenz/.codex/worktrees/716f/gecode): build/zdev-probe/probe.cpp and probes.inc post every array-taking overload with empty arrays, each in a forked child; results are in build/zdev-probe/out.txt, and `./probe '<name substring>'` reruns matching probes. build/zdev-instr/src is `git archive HEAD` with Region::ralloc, Heap::ralloc and MemoryManager::alloc patched to print a backtrace on size 0; it is built in build/zdev-instr/b and linked by ./probe-instr, and per-probe zero-size sites are in build/zdev-probe/instr-report.txt. To check a fix, archive the edited tree into a fresh copy with the same three-site patch; never put that patch in tracked code.

Append a section for this child's families to build/zdev-audit/empty-array-audit.md (an untracked draft; create the directory if missing). For each posting function give the method (driver, source trace, or both), the result before, the result after, and the zero-size request sizes before and after. The parent assembles the final audit from this draft.

## Boundaries

- Fix at the posting site or its propagator post helper: decide the empty constraint at posting and post nothing, or fail. Keep every existing argument check, GECODE_POST and documented exception in front of the new code, as 5f9a8e0e71 does for no_overlap.
- Do not change the allocator or the kernel: do not let it accept zero sizes, and add no branch to the allocation fast path. ViewArray and SharedArray already skip allocation for n==0 (gecode/kernel/data/array.hpp:306,323,1091-1112,1322; gecode/kernel/data/shared-array.hpp:187), so their callers need no change.
- Do not change behaviour for non-empty input.
- Do not edit changelog.in or anything under .zdev. The parent task empty-arrays-002 owns the audit document in .zdev/empty-arrays/background, the 7.0.0 changelog entry and the final check.py all.
- Leave the bin_packing d==0 division by zero as it is; the parent records it.
- Initial task-owned path allocation: ["gecode/int/gcc.cpp","gecode/int/bin-packing.cpp","gecode/int/bin-packing/propagate.cpp","test/int/gcc.cpp","test/int/bin-packing.cpp"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [ ] The GCC probes (`count(...) gcc*` and `count(x=[y],c[],v[])`) and the bin_packing probes give SS_SOLVED or SS_FAILED as documented in build/full, with no abort and no zero-size request in a fresh instrumented build.
- [ ] New Test::Base tests Int::GCC::Empty (test/int/gcc.cpp) and Int::BinPacking::Empty (test/int/bin-packing.cpp) cover every overload's empty case, abort before the fix in build/full and pass after it.
- [ ] build/zdev-audit/empty-array-audit.md has a section for this child's families listing each posting function, how it was checked (driver, source trace, or both), its result before and after, and the zero-size request sizes before and after.

## Validation

- In build/full (Debug, audit): build gecode-test and run Int::GCC::Empty and Int::BinPacking::Empty by exact name before the fix (expect exit 134) and after it.
- Rerun the GCC and bin_packing probes against build/full and against a fresh instrumented build of the edited tree.
- `bin/gecode-test -test Int::GCC -tag standard` and `bin/gecode-test -test Int::BinPacking -tag standard` in build/full.
- Run `uv run misc/check.py format-check --build-dir build/full` and the lint check (`uv run misc/check.py lint --build-dir build/full --jobs 12`) for the touched files.
