+++
schema_version = 1
id = "empty-arrays-004"
key = "ldsb-relax"
area = "empty-arrays"
status = "open"
complexity = "advanced"
afk = true
blocked_by = []
+++
# Branch with empty symmetries and relax empty arrays

## Outcome

LDSB branching with int, Bool and set variables ignores symmetry handles that describe no variables or values and allocates no zero-size symmetry array when posting or cloning, and the int, set and float relax functions return without work for empty variable arrays. Neither family aborts in Debug on empty input.

## Context

Part of the empty-arrays-002 inventory (branching and relax). Both families abort in build/full on empty input.

A3 LDSB branch with symmetries. gecode/int/ldsb.cpp:285,323,417,467 and gecode/set/ldsb.cpp:149,186 call home.alloc<SymmetryImp*>(syms.size()), which is a zero-size Space request for Symmetries(). create_int_sym and create_bool_sym (gecode/int/ldsb.cpp:148-245) and create_set_sym in gecode/set/ldsb.cpp make zero-size requests for symmetries over no variables or values: variable_symmetry([]), variable_symmetry(x, []), value_symmetry([]), *_sequence_symmetry([], k), values_reflect(0, -1), and rows_interchange or columns_interchange of a 0x0 matrix. ValueSymmetryImp asserts n>0 (gecode/int/ldsb/sym-imp.hpp:116). The LDSBBrancher copy constructor makes another home.alloc(_nsyms) at gecode/int/ldsb/brancher.hpp:114, so the fix must also hold after cloning.
Fix: add one shared predicate in Int::LDSB (declared in gecode/int/ldsb.hh) that tells whether a symmetry handle describes any variables or values, and use it in both gecode/int/ldsb.cpp and gecode/set/ldsb.cpp to drop empty handles. Allocate the SymmetryImp array only when at least one symmetry remains (otherwise nullptr with a count of 0), and make the LDSBBrancher copy constructor allocate _syms only when _nsyms>0. Keep the value-selection checks and LDSBBadValueSelection exactly as they are: the probe `branch(y,Symmetries) SPLIT` must still throw. Cover the plain and tie-break branch overloads for int, Bool and set variables.

A4 relax. gecode/int/relax.cpp:64,74, gecode/set/relax.cpp:61 and gecode/float/relax.cpp:56 call Search::relax (gecode/search/relax.hh:59). With empty x the bitset is all set, so it calls ax.clear(r(0)), an out-of-bounds assertion in gecode/support/bitset-base.hpp:546.
Fix: each of the four public relax functions returns after its argument checks when x is empty. FlatZinc LNS does not call Gecode::relax (gecode/flatzinc/flatzinc.cpp:1148-1170, 2238-2248).

Out of scope, recorded by the parent only: Search::relax calls home.failed() where home.fail() is meant, so a failed equality does not fail the space for non-empty input; the kernel Action/CHB storage makes heap zero requests (gecode/kernel/branch/action.hpp:273, chb.hpp:258) for ACTION/CHB merits on empty arrays.

Testing follows the brief (focused coverage). Test::Int::Test and its relatives cannot express arity 0 (CpltAssignment::has_more reads dsv[0], test/int.hpp:55), so each family gets a Test::Base `Empty` test next to its existing tests, modelled on Int::NoOverlap::Empty (test/int/no-overlap.cpp:269-301): an empty fixture Space, the public posting call, and status() compared with the documented result (SS_SOLVED, SS_FAILED, or the documented exception). For a family that aborts, the test must abort before the fix in build/full (Debug with audit; exit 134 is accepted, as in empty-arrays-001) and pass after it. For a family whose result is already correct but which makes a zero-size Region or heap request, the test documents the result and passes before the fix; record the before and after request sizes in the audit draft instead, and add no allocator instrumentation to tracked code or tests.

Inventory artefacts from the parent's probe run are ignored files in the area checkout (/Users/zayenz/.codex/worktrees/716f/gecode): build/zdev-probe/probe.cpp and probes.inc post every array-taking overload with empty arrays, each in a forked child; results are in build/zdev-probe/out.txt, and `./probe '<name substring>'` reruns matching probes. build/zdev-instr/src is `git archive HEAD` with Region::ralloc, Heap::ralloc and MemoryManager::alloc patched to print a backtrace on size 0; it is built in build/zdev-instr/b and linked by ./probe-instr, and per-probe zero-size sites are in build/zdev-probe/instr-report.txt. To check a fix, archive the edited tree into a fresh copy with the same three-site patch; never put that patch in tracked code.

Append a section for this child's families to build/zdev-audit/empty-array-audit.md (an untracked draft; create the directory if missing). For each posting function give the method (driver, source trace, or both), the result before, the result after, and the zero-size request sizes before and after. The parent assembles the final audit from this draft.

No relax test exists yet; test/search.cpp is the home for Search::Relax::Empty.

## Boundaries

- Fix at the posting site or its propagator post helper: decide the empty constraint at posting and post nothing, or fail. Keep every existing argument check, GECODE_POST and documented exception in front of the new code, as 5f9a8e0e71 does for no_overlap.
- Do not change the allocator or the kernel: do not let it accept zero sizes, and add no branch to the allocation fast path. ViewArray and SharedArray already skip allocation for n==0 (gecode/kernel/data/array.hpp:306,323,1091-1112,1322; gecode/kernel/data/shared-array.hpp:187), so their callers need no change.
- Do not change behaviour for non-empty input.
- Do not edit changelog.in or anything under .zdev. The parent task empty-arrays-002 owns the audit document in .zdev/empty-arrays/background, the 7.0.0 changelog entry and the final check.py all.
- Keep LDSB value-selection checks and the LDSBBadValueSelection exception unchanged.
- Do not change Search::relax in gecode/search/relax.hh or the kernel Action/CHB storage; the parent records them.
- Initial task-owned path allocation: ["gecode/int/ldsb.cpp","gecode/int/ldsb.hh","gecode/int/ldsb/brancher.hpp","gecode/set/ldsb.cpp","gecode/int/relax.cpp","gecode/set/relax.cpp","gecode/float/relax.cpp","test/ldsb.cpp","test/search.cpp"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [ ] The LDSB probes (int, Bool and set branch with symmetries, including `set branch(sv,Symmetries())`, Symmetries(), values_reflect(0,-1), rows_interchange of a 0x0 matrix) and the int, set and float relax probes give no abort and no zero-size Space request in build/full and in a fresh instrumented build, including after a clone; `branch(y,Symmetries) SPLIT` still throws.
- [ ] New Test::Base tests LDSB::Empty* (test/ldsb.cpp) for int, Bool and set branching and Search::Relax::Empty (test/search.cpp) for all four relax functions clone and search, abort before the fix in build/full and pass after it.
- [ ] build/zdev-audit/empty-array-audit.md has a section for this child's families listing each posting function, how it was checked (driver, source trace, or both), its result before and after, and the zero-size request sizes before and after.

## Validation

- In build/full (Debug, audit): build gecode-test and run the new LDSB::Empty* and Search::Relax::Empty tests by exact name before the fix (expect exit 134) and after it.
- Rerun the LDSB and relax probes against build/full and against a fresh instrumented build of the edited tree.
- `bin/gecode-test -test LDSB` and `bin/gecode-test -test Relax` in build/full, then `bin/gecode-test -test LDSB -tag standard`.
- Run `uv run misc/check.py format-check --build-dir build/full` and the lint check (`uv run misc/check.py lint --build-dir build/full --jobs 12`) for the touched files.
