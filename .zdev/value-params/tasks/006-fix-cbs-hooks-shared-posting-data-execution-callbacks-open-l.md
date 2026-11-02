+++
schema_version = 1
id = "value-params-006"
key = "posting-leftovers"
area = "value-params"
status = "done"
complexity = "advanced"
afk = true
blocked_by = ["value-params-005"]
+++
# Fix CBS hooks, shared posting data, execution callbacks, open lists, and tracing

## Outcome

The CBS virtual hooks and their overrides take their callbacks by const&, shared posting data and trace filters are passed by const&, wait, when, function-branch, and open-list callbacks reach storage through moves, and the inventory reports no diagnostic for G1, G9, G10, G11, and G12.

## Context

Repair groups G1 (CBS virtual hooks, 4 parameters), G9 (posting with shared data, 48), G10 (execution callbacks, 22), G11 (open variable lists, 6), and G12 (tracing, 9) from .zdev/value-params/background/value-param-inventory.md. G10 and G11 depend on the SharedData(Data&&) path added by the previous child. Follow "Delicate cases" (DFA through OpenRegular; Stored callbacks and SharedData for UnaryWait, NaryWait, When, and FunctionBranch), the "Other stored values" bullets Open variable lists, Count's generic VY, and CBS virtual hooks, "Repair groups", "Verification hooks", and the per-parameter tables G1, G9, G10, G11, and G12. G1: Propagator::solution_distribution(Space&, SendMarginal) const and domain_size_sum(InDecision, unsigned int&, unsigned int&) const (gecode/kernel/core.hpp:1259, 1269; defined :3781, :3784) change together with all six overrides in Distinct::Val, Bnd, and Dom (gecode/int/distinct.hh:75, 77, 165, 167, 291, 293; gecode/int/distinct/val.hpp:160, 166, bnd.hpp:462, 468, dom.hpp:80, 86), which the check does not report, and the helpers cbs_distinct and cbs_size (distinct/cbs.hpp:135, 211; declared distinct.hh:99, 102). Callers in gecode/int/branch/cbs.hpp pass a lambda or a local std::function, which bind to const&. CBS code is compiled only with GECODE_ENABLE_CBS (on in build/full, off in build/local). G9: DFA through OpenRegular becomes const DFA& in all three layers (extensional-regular.cpp:56, 63; extensional/open.hpp:296 and the constructor at :108) and both gecode/int.hh declarations; the constructor reads dfa0 again after copying it into the member, so never move it. Count's VY (EqInt/GqInt/LqInt/EqView/GqView/LqView, gecode/int/count/*.hpp, declared count.hh) becomes const VY&, and the explicit specializations is_int_set(IntSet) and isval(IntSet) (count/rel.hpp:50, 62) change with their primary templates (rel.hpp:44, 57; count.hh:54, 57). IntSharedArray, IntSet, Rnd, IntArgs, and Matrix's A become const&; the const& keepers are OpenRegular dfa0, ValueSequenceSymmetryObject vs, and Matrix a. G10: wait, when, and function-branch callbacks stay by value and move into UnaryWait, NaryWait (gecode/kernel/propagator/wait.hpp:108, 171), When (gecode/int/exec/when.hpp:37), and FunctionBranch (gecode/kernel/branch/function.cpp:88) as c(std::move(c0)) and so on; each post calls the callback directly when the view is assigned and otherwise constructs, so the paths are exclusive and moving on the constructing path is safe. G11: OpenIntVarList::factory(Domain d) (gecode/int/open-var-list.cpp:40) checks !d and then captures d; capture it with [d = std::move(d)] after the check, and move d and t once through the constructors at :52-:65. G12: TraceFilter is a SharedHandle, so it becomes const& down to the ViewTraceRecorder and TraceRecorder members (gecode/kernel/trace/recorder.hpp:260, 390), the const& keepers for this group. Rules from the brief and task value-params-002: an owned or reference-counted parameter becomes const& unless the callee keeps the value and the type has a real move constructor; then it stays by value and is moved on every path to its storage. A std::move that binds a copying const& constructor does not count as a move: SharedHandle types (IntSet, DFA, TupleSet, SharedArray, TraceFilter), the *Args arrays, FloatVal, Rnd, and the AFC/Action/CHB handles have no real move, so they become const& and the member copy (or copy assignment) from the const& is the only copy. Shallow handles matched by the inventory's Proposed AllowedTypes (variables, views, Home, groups, Pos, Reify, VarArray/ViewArray, open variable lists, VarType) stay by value. A virtual override changes together with its base declaration. Change every other declaration the inventory row lists. Within a forwarding chain the treatment follows the final destination (inventory "What the diagnostics mean here", items 3-6). Public header signature changes are accepted for Gecode 7: they change the ABI but not source compatibility. Do not change ownership or lifetime. Verify the storage paths by source inspection; no new permanent test is needed (brief, Testing).

## Boundaries

- Leave the CBS callers in gecode/int/branch/cbs.hpp unchanged unless a const& signature forces a change.
- FlatZinc, Gist, search, tests, examples (G13-G15), and FloatVal (G16) belong to later children.
- Do not edit .clang-tidy, docs/code-style.md, or changelog.in; task value-params-003 enables the check, documents the rule, and writes the 7.0.0 changelog entry about the changed public signatures. Do not edit anything under .zdev.
- The parent task value-params-002 keeps the area's final checks: the full inventory rerun with no diagnostic outside AllowedTypes, both builds with ctest, and `uv run misc/check.py all --build-dir build/full --jobs 12`.
- Keep behaviour, ownership, and lifetimes unchanged, and add no permanent test or allocator instrumentation. Throwaway probes stay under build/.
- Shared top-level headers (gecode/int.hh, set.hh, float.hh, kernel.hh, minimodel.hh, kernel/core.hpp, and the module .hh files) are edited by several children in turn; the blocked_by chain keeps the children serial. Change only the declarations of this child's groups in them.
- Initial task-owned path allocation: ["gecode/kernel/core.hpp","gecode/int/distinct.hh","gecode/int/distinct/cbs.hpp","gecode/int/distinct/val.hpp","gecode/int/distinct/bnd.hpp","gecode/int/distinct/dom.hpp","gecode/float/relax.cpp","gecode/int/count/int-eq.hpp","gecode/int/count/int-gq.hpp","gecode/int/count/int-lq.hpp","gecode/int/count/rel.hpp","gecode/int/count/view-eq.hpp","gecode/int/count/view-gq.hpp","gecode/int/count/view-lq.hpp","gecode/int/count.hh","gecode/int/element.cpp","gecode/int/extensional-regular.cpp","gecode/int/extensional/open.hpp","gecode/int/ldsb/sym-obj.cpp","gecode/int/relax.cpp","gecode/int/sequence/int.hpp","gecode/int/sequence/view.hpp","gecode/int/sequence.hh","gecode/minimodel/aliases.hpp","gecode/minimodel/matrix.hpp","gecode/minimodel.hh","gecode/set/int.cpp","gecode/float/exec.cpp","gecode/int/exec.cpp","gecode/int/exec/when.hpp","gecode/int/exec.hh","gecode/kernel/branch/function.cpp","gecode/kernel/propagator/wait.hpp","gecode/set/exec.cpp","gecode/kernel.hh","gecode/int/open-var-list.cpp","gecode/float/trace.cpp","gecode/int/trace.cpp","gecode/kernel/trace/general.cpp","gecode/kernel/trace/recorder.hpp","gecode/set/trace.cpp"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [x] The G1 base virtuals, all six Distinct overrides, and the two CBS helpers take SendMarginal and InDecision by const&, and build/full (CBS on) compiles them.
- [x] The inventory reproduction rerun in build/zdev-audit/value-params-posting-leftovers/ reports no diagnostic for G1, G9, G10, G11, and G12: its rows.tsv has no row in these groups (group column, matched by group number), it has no row (compared by file, function, and parameter) that the baseline build/zdev-audit/value-params/rows.tsv lacks, and checks.txt is empty.
- [x] The result lists every parameter changed to const& whose callee keeps a copy, by function, parameter, and file:line (at least the inventory's Verification hooks rows for G9 and G12), with the member or object it initializes and how the copy is made (copy constructor or copy assignment from the const&).
- [x] The result shows, with source references, that every by-value-and-move parameter of G10 and G11 reaches its storage through a move constructor (std::function's move constructor at each layer, SDO(Data&&) in UnaryWait, NaryWait, When, FunctionBranch, and OpenVarList<Var>::Object, and the factory lambda's init-capture), and that no std::move binds a copying const& constructor.
- [x] build/full and build/local build, and ctest passes in both.

## Validation

- Build build/full (Debug, configured as in the inventory's Reproduce step 1: experimental, CBS, fault injection, audit, and Gist) and the default Release configuration build/local, and run ctest in both.
- Because callback storage changes, run `cmake --build build/full --target check-fault`.
- In build/full, run `bin/gecode-test -tag standard` with `-test Int::Branch::CBS`, `-test Int::Distinct`, `-test Int::Wait`, `-test Int::When`, `-test Int::Count`, `-test Int::Extensional`, `-test Int::Sequence`, `-test Int::OpenList`, `-test LDSB`, and `-test Search::RBS`.
- Run `uv run misc/check.py format-check --build-dir build/full`.
- Rerun the inventory reproduction (Reproduce steps 2 and 3 in .zdev/value-params/background/value-param-inventory.md) into a fresh directory build/zdev-audit/value-params-posting-leftovers/ so the baseline in build/zdev-audit/value-params/ stays intact: `sh build/zdev-audit/value-params-posting-leftovers/reproduce.sh build/full 10`.

## Result

CBS hooks, their six Distinct overrides and helpers take SendMarginal and InDecision by const&; DFA, IntSet, IntSharedArray, Rnd, IntArgs, Count's VY, Sequence's Val, Matrix's A and TraceFilter are const& down to their member copies (OpenRegular, ValueSequenceSymmetryObject, Matrix, Count IntBase/ViewBase, Sequence, trace recorders); wait, when, function-branch and open-list callbacks move through every layer into SharedData(Data&&) (inventory groups G1, G9-G12)

Validation:

- build/full (Debug, experimental, CBS, fault injection, audit, Gist), gecode-test and build/local (Release) built with 0 warnings; ctest 8/8 and 7/7; check-fault 21/21
- Standard-tag focused tests: Int::Branch::CBS, Int::Distinct, Int::Wait, Int::When, Int::Count, Int::Extensional, Int::Sequence, Int::OpenList, LDSB, Search::RBS, Set::Wait, Set::Int::Weights, Int::Element::Matrix, Search::Relax all pass
- format-check with clang-format 22 and clang-tidy 22 with the repository .clang-tidy on 24 changed TUs: clean
- Scoped inventory rerun: 159 parameters (G13 56, G14 7, G15 43, G16 53), no G1 or G9-G12 row, no new keys versus baseline, empty checks.txt
- Copy-count probe, lvalue callbacks: wait 4 to 1, when 8 to 2, branch 4 to 1, OpenIntVarList domain and transition 5 to 2; rvalues 0
- Independent Codex verifier: pass, no findings
