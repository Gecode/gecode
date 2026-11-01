+++
schema_version = 1
id = "empty-arrays-006"
key = "linear-buffers"
area = "empty-arrays"
status = "done"
complexity = "standard"
afk = true
blocked_by = []
+++
# Skip empty term buffers in linear posting and minimodel expressions

## Outcome

The int, Bool and float linear posting functions and the minimodel LinIntExpr and LinFloatExpr posting make no zero-size Region request for empty sums and still decide them at posting, and the minimodel BElementExpr and ArithNonLinFloatExpr constructors allocate nothing for zero operands and still throw the documented TooFewArguments.

## Context

Part of the empty-arrays-002 inventory (linear sums and minimodel expressions). Every family here already gives the correct result; the fix removes zero-size Region or heap requests, so the audit records the sizes.

B1 int linear: gecode/int/linear-int.cpp:43,55,69,83 (the coefficient variants allocate Term arrays with alloc(x.size())) and gecode/int/linear-bool.cpp:45,60,78,96,128,143,160,177 (all 8 overloads alloc(n)). Linear::post already decides n==0 (gecode/int/linear/int-post.cpp:243,561; gecode/int/linear/bool-post.cpp:462,585).
B2 float linear: gecode/float/linear.cpp:44,57,72,87. Float post_linear decides n==0 (gecode/float/linear/post.cpp:205).
B3 minimodel linear expressions: LinIntExpr::post (gecode/minimodel/int-expr.cpp:184-345) and LinFloatExpr::post (gecode/minimodel/float-expr.cpp:146-230) make Region requests sized n_int, n_bool and n_float, which are 0 for empty sums such as rel(home, sum(x) == c), sum(IntArgs) and Matrix sums.
Fix for B1-B3: skip the term buffer allocation for an empty sum and let the existing n==0 decision decide and post nothing; reuse that logic rather than re-deriving it.

B4 expression constructors make heap zero requests before the documented TooFewArguments throw: BElementExpr (gecode/minimodel/bool-expr.cpp:669, element(BoolVarArgs, LinIntExpr)) and ArithNonLinFloatExpr (gecode/minimodel/float-arith.cpp:74,76, float min and max). Precedent: ArithNonLinIntExpr::allocate returns nullptr for n==0 (gecode/minimodel/int-arith.cpp:71). Fix: allocate nothing for zero operands and keep the throw at posting. FlatZinc empty int/float lin_* constraints reach B1/B2 and need no frontend change.

Testing follows the brief (focused coverage). Test::Int::Test and its relatives cannot express arity 0 (CpltAssignment::has_more reads dsv[0], test/int.hpp:55), so each family gets a Test::Base `Empty` test next to its existing tests, modelled on Int::NoOverlap::Empty (test/int/no-overlap.cpp:269-301): an empty fixture Space, the public posting call, and status() compared with the documented result (SS_SOLVED, SS_FAILED, or the documented exception). For a family that aborts, the test must abort before the fix in build/full (Debug with audit; exit 134 is accepted, as in empty-arrays-001) and pass after it. For a family whose result is already correct but which makes a zero-size Region or heap request, the test documents the result and passes before the fix; record the before and after request sizes in the audit draft instead, and add no allocator instrumentation to tracked code or tests.

Inventory artefacts from the parent's probe run are ignored files in the area checkout (/Users/zayenz/.codex/worktrees/716f/gecode): build/zdev-probe/probe.cpp and probes.inc post every array-taking overload with empty arrays, each in a forked child; results are in build/zdev-probe/out.txt, and `./probe '<name substring>'` reruns matching probes. build/zdev-instr/src is `git archive HEAD` with Region::ralloc, Heap::ralloc and MemoryManager::alloc patched to print a backtrace on size 0; it is built in build/zdev-instr/b and linked by ./probe-instr, and per-probe zero-size sites are in build/zdev-probe/instr-report.txt. To check a fix, archive the edited tree into a fresh copy with the same three-site patch; never put that patch in tracked code.

Append a section for this child's families to build/zdev-audit/empty-array-audit.md (an untracked draft; create the directory if missing). For each posting function give the method (driver, source trace, or both), the result before, the result after, and the zero-size request sizes before and after. The parent assembles the final audit from this draft.

## Boundaries

- Fix at the posting site or its propagator post helper: decide the empty constraint at posting and post nothing, or fail. Keep every existing argument check, GECODE_POST and documented exception in front of the new code, as 5f9a8e0e71 does for no_overlap.
- Do not change the allocator or the kernel: do not let it accept zero sizes, and add no branch to the allocation fast path. ViewArray and SharedArray already skip allocation for n==0 (gecode/kernel/data/array.hpp:306,323,1091-1112,1322; gecode/kernel/data/shared-array.hpp:187), so their callers need no change.
- Do not change behaviour for non-empty input.
- Do not edit changelog.in or anything under .zdev. The parent task empty-arrays-002 owns the audit document in .zdev/empty-arrays/background, the 7.0.0 changelog entry and the final check.py all.
- Keep the documented TooFewArguments exceptions for minimodel element over Boolean arrays and float min and max.
- Initial task-owned path allocation: ["gecode/int/linear-int.cpp","gecode/int/linear-bool.cpp","gecode/float/linear.cpp","gecode/minimodel/int-expr.cpp","gecode/minimodel/float-expr.cpp","gecode/minimodel/bool-expr.cpp","gecode/minimodel/float-arith.cpp","test/int/linear.cpp","test/float/linear.cpp","test/int/mm-lin.cpp","test/float/mm-lin.cpp","test/int/mm-bool.cpp","test/float/arithmetic.cpp"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [x] The linear*, float linear*, `mm rel(sum..)`, `mm float ...`, `mm expr(element(bool x,y))` and `mm float expr(min/max)` probes keep their results from build/zdev-probe/out.txt and show no zero-size request in a fresh instrumented build.
- [x] New Test::Base Empty tests next to the existing tests (test/int/linear.cpp, test/float/linear.cpp, test/int/mm-lin.cpp, test/float/mm-lin.cpp, test/int/mm-bool.cpp, test/float/arithmetic.cpp) document the decided results and the kept exceptions.
- [x] build/zdev-audit/empty-array-audit.md has a section for this child's families listing each posting function, how it was checked (driver, source trace, or both), its result before and after, and the zero-size request sizes before and after.

## Validation

- In build/full (Debug, audit): build gecode-test and run each new Empty test by exact name.
- Rerun the family's probes against build/full and against a fresh instrumented build of the edited tree, and record the before (0-byte Region or heap request) and after (none) sizes.
- `bin/gecode-test -test Int::Linear`, `-test Float::Linear` and `-test MiniModel` in build/full, then each with `-tag standard`.
- Run `uv run misc/check.py format-check --build-dir build/full` and the lint check (`uv run misc/check.py lint --build-dir build/full --jobs 12`) for the touched files.

## Result

Int, Bool and float linear posting and the LinIntExpr and LinFloatExpr relations allocate term buffers only for non-empty sums; BElementExpr and float min/max expressions allocate nothing for zero operands and still throw TooFewArguments

Validation:

- Six new Empty tests (int, float linear; int, float minimodel linear relations; Boolean element; float min/max) pass in build/full and build/local
- build/full and build/local standard: Int::Linear 126/126, Float::Linear 3/3, MiniModel 3544/3544; Release untagged Int::Linear, Float::Linear and MiniModel pass
- 46 probes: identical results; fresh instrumented build shows no zero-size request (before: one on each of 32 probes)
- format-check and lint (LLVM 22) passed
- Independent Codex verifier: pass, no findings
