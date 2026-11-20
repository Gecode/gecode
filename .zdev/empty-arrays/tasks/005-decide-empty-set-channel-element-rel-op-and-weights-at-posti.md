+++
schema_version = 1
id = "empty-arrays-005"
key = "set-posting"
area = "empty-arrays"
status = "done"
complexity = "standard"
afk = true
blocked_by = []
+++
# Decide empty set channel, element, rel-op and weights at posting

## Outcome

The set channel_sorted, channel with a Boolean array, element, n-ary rel with a constant set, and weights posting functions decide empty input at posting and post nothing or fail, with no Debug abort and no zero-size Space or Region request, and with unchanged results for non-empty input.

## Context

Part of the empty-arrays-002 inventory (set module). Three families abort in build/full; four give correct results but make zero-size requests.

Aborts:
A5 channel_sorted: gecode/set/channel/sorted.hpp:61 posts ChannelSorted with empty xs, which indexes xs[0] during propagation (abort in status()). FlatZinc gecode_set_channel_sorted reaches it (gecode/flatzinc/registry-set.cpp:417). Fix: ChannelSorted::post returns after its cardinality constraints when xs is empty.
A6 channel(BoolVarArgs, SetVar): gecode/set/channel/bool.hpp:114 runs y.intersect(0, -1), which asserts ma>=mi in integerset.hpp:363. FlatZinc link_set_to_booleans with idx 0 reaches it (gecode/flatzinc/registry-set.cpp:227). Fix: ChannelBool::post forces y empty and posts nothing for empty x.
A7 element(SOT_UNION or SOT_DUNION, IntSetArgs or IntArgs, ...): ElementUnionConst::post (gecode/set/element/unionConst.hpp:89, constructor :44) makes a Space alloc<IntSet>(0), reached from gecode/set/element.cpp:104-120, and its propagate makes a Region alloc(0) (:110). Fix: handle n==0 in the post helper: the index set is empty and the union result is empty; post nothing.

Correct result, zero-size Region request (record sizes in the audit):
B5 element with SetVarArgs or IntVarArgs and empty x: ElementUnion::post (gecode/set/element/union.hpp:85), ElementIntersection::post (gecode/set/element/inter.hpp:88) and ElementDisjoint::post (gecode/set/element/disjoint.hpp:56) post propagators whose propagate makes a Region alloc(0). This covers minimodel range() and FlatZinc range (gecode/flatzinc/registry-set.cpp:336). Fix: each post helper handles n==0 and posts nothing: index set empty, union result empty, intersection result equal to the universe.
B6 rel(op, x[], IntSet z, y): UnionN::post(x, z, y) (gecode/set/rel-op/union.hpp:234) and PartitionN::post(x, z, y) (gecode/set/rel-op/partition.hpp:92) post n-ary propagators with empty x, and propagate makes a Region alloc(0). The variants without z already decide in a `case 0` (union.hpp:218, partition.hpp:78, inter.hpp:212). Fix: following that precedent, the z variants constrain y equal to z for empty x and post nothing.
B7 weights with empty elements: Weights::post (gecode/set/int/weights.hpp:164) makes a Region alloc(0). FlatZinc weights reaches it. Fix: with no elements, force x empty and y=0 and post nothing.

Already correct and left alone: set dom, cardinality, rel(op,x,y), rel(op,intx,y), sequence(y,x), atmost_one, channel(int x, set y), channel(set, set), precede, element INTER over constants (fails: universe), wait, branch/assign, trace. Set sequence(x) and set element(x,y,z) throw TooFewArguments; keep that.

Testing follows the brief (focused coverage). Test::Int::Test and its relatives cannot express arity 0 (CpltAssignment::has_more reads dsv[0], test/int.hpp:55), so each family gets a Test::Base `Empty` test next to its existing tests, modelled on Int::NoOverlap::Empty (test/int/no-overlap.cpp:269-301): an empty fixture Space, the public posting call, and status() compared with the documented result (SS_SOLVED, SS_FAILED, or the documented exception). For a family that aborts, the test must abort before the fix in build/full (Debug with audit; exit 134 is accepted, as in empty-arrays-001) and pass after it. For a family whose result is already correct but which makes a zero-size Region or heap request, the test documents the result and passes before the fix; record the before and after request sizes in the audit draft instead, and add no allocator instrumentation to tracked code or tests.

Inventory artefacts from the parent's probe run are ignored files in the area checkout (/Users/zayenz/.codex/worktrees/716f/gecode): build/zdev-probe/probe.cpp and probes.inc post every array-taking overload with empty arrays, each in a forked child; results are in build/zdev-probe/out.txt, and `./probe '<name substring>'` reruns matching probes. build/zdev-instr/src is `git archive HEAD` with Region::ralloc, Heap::ralloc and MemoryManager::alloc patched to print a backtrace on size 0; it is built in build/zdev-instr/b and linked by ./probe-instr, and per-probe zero-size sites are in build/zdev-probe/instr-report.txt. To check a fix, archive the edited tree into a fresh copy with the same three-site patch; never put that patch in tracked code.

Append a section for this child's families to build/zdev-audit/empty-array-audit.md (an untracked draft; create the directory if missing). For each posting function give the method (driver, source trace, or both), the result before, the result after, and the zero-size request sizes before and after. The parent assembles the final audit from this draft.

## Boundaries

- Fix at the posting site or its propagator post helper: decide the empty constraint at posting and post nothing, or fail. Keep every existing argument check, GECODE_POST and documented exception in front of the new code, as 5f9a8e0e71 does for no_overlap.
- Do not change the allocator or the kernel: do not let it accept zero sizes, and add no branch to the allocation fast path. ViewArray and SharedArray already skip allocation for n==0 (gecode/kernel/data/array.hpp:306,323,1091-1112,1322; gecode/kernel/data/shared-array.hpp:187), so their callers need no change.
- Do not change behaviour for non-empty input.
- Do not edit changelog.in or anything under .zdev. The parent task empty-arrays-002 owns the audit document in .zdev/empty-arrays/background, the 7.0.0 changelog entry and the final check.py all.
- Keep the existing results of the set channel, element, rel and weights probes and `mm range` in build/zdev-probe/out.txt where they do not abort; only the aborts and the zero-size requests go away. The `set branch` and `set relax` probes belong to the ldsb-relax child.
- Initial task-owned path allocation: ["gecode/set/channel/sorted.hpp","gecode/set/channel/bool.hpp","gecode/set/element/union.hpp","gecode/set/element/inter.hpp","gecode/set/element/disjoint.hpp","gecode/set/element/unionConst.hpp","gecode/set/rel-op/union.hpp","gecode/set/rel-op/partition.hpp","gecode/set/int/weights.hpp","test/set/channel.cpp","test/set/element.cpp","test/set/rel-op-const.cpp","test/set/int.cpp"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [x] The `set channel_sorted`, `set channel(bool x,y)`, `set element(...)`, `set rel(...,z,y)`, `set weights` and `mm range` probes give no abort in build/full and no zero-size request in a fresh instrumented build. Probes that report SS_SOLVED, SS_FAILED or an exception in build/zdev-probe/out.txt keep that result; the six that exit 134 now give the documented result (channel_sorted and channel with a Boolean array post nothing with y empty, element UNION/DUNION over constant sets posts nothing with the result empty).
- [x] New Test::Base Empty tests for channel_sorted and channel with a Boolean array (test/set/channel.cpp) and for element SOT_UNION/SOT_DUNION over constant sets (test/set/element.cpp) abort before the fix in build/full and pass after it.
- [x] New Test::Base Empty tests document the empty-input results of element over variable arrays (test/set/element.cpp), rel with a constant set (test/set/rel-op-const.cpp) and weights (test/set/int.cpp).
- [x] build/zdev-audit/empty-array-audit.md has a section for this child's families listing each posting function, how it was checked (driver, source trace, or both), its result before and after, and the zero-size request sizes before and after.

## Validation

- In build/full (Debug, audit): build gecode-test and run each new Empty test by exact name before the fix (the abort families expect exit 134) and after it.
- Rerun the `set ...` and `mm range` probes against build/full and against a fresh instrumented build of the edited tree.
- `bin/gecode-test -test Set::Channel`, `-test Set::Element`, `-test Set::RelOp` and `-test Set::Int` in build/full, then each with `-tag standard`.
- Run `uv run misc/check.py format-check --build-dir build/full` and the lint check (`uv run misc/check.py lint --build-dir build/full --jobs 12`) for the touched files.

## Result

Set channel_sorted, channel with a Boolean array, element (union, union over constants, intersection, disjoint), n-ary union and partition with a constant set, and weights decide empty input at posting and post nothing

Validation:

- Set::Channel::Sorted::Empty, Set::Channel::Bool::Empty and Set::Element::Const::Empty abort (exit 134) before the fix in a Debug audit build; all six new Empty tests pass after it
- build/full and build/local: Set::Channel, Set::Element, Set::RelOp and Set::Int standard selections pass
- Probes: only the six aborting results changed (to SS_SOLVED); fresh instrumented build shows no zero-size request on these families
- format-check and lint (LLVM 22) passed
- Independent Codex verifier: pass, no findings
