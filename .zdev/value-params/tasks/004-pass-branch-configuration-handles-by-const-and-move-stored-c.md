+++
schema_version = 1
id = "value-params-004"
key = "branch-configs"
area = "value-params"
status = "done"
complexity = "advanced"
afk = true
blocked_by = []
+++
# Pass branch configuration handles by const& and move stored callbacks

## Outcome

The kernel branch configuration classes and the int, bool, set, and float branch configuration factories and constructors take Rnd, AFC, Action, and CHB handles and read-only merit functions by const&, and move branch tables and merit, value, and commit functions into the configuration objects they build, so the inventory reports no diagnostic for G3, G4, G5, and G6.

## Context

Repair groups G3 (kernel branch configurations, 27 parameters), then G4 (int and bool, 120), G5 (set, 73), and G6 (float, 71) from .zdev/value-params/background/value-param-inventory.md. Do G3 first: G4-G6 move their value and commit functions into IntValBranch(v, c) and its siblings, which pass them to ValBranch(BranchVal v, BranchCommit c) (gecode/kernel/branch/val.hpp:80); that constructor copies vf(v), cf(c) today and must become vf(std::move(v)), cf(std::move(c)) for the moves to reach storage. The VarBranch constructors already move into _tbl and _mf (gecode/kernel/branch/var.hpp:119-150), so INT_VAR_*/BOOL_VAR_*/SET_VAR_*/FLOAT_VAR_* move tbl and merit functions into the returned object (`return IntVarBranch(..., std::move(tbl))`). Follow the inventory sections "Other stored values" (bullets Branch configuration factories, VarBranch setters, Action/CHB merit functions, and TieBreak and tie_break), "Repair groups", "Verification hooks", and the per-parameter tables G3-G6. Delicate cases: (1) Rnd/AFC/Action/CHB have no move, so they become const& everywhere, including the *_RND/*_AFC_*/*_ACTION_*/*_CHB_* factories that build them into the returned object and the VarBranch/ValBranch constructors that copy them into members; these are the const& keepers the verifier checks by name. (2) The VarBranch setters afc(), action(), and chb() (var.hpp:178, 190, 202; declared :101, :105, :109) copy-assign handles and become const&. (3) The Action/CHB merit chain (IntAction/BoolAction/IntCHB/BoolCHB and the set/float forms -> Action::init -> Storage, gecode/kernel/branch/action.hpp:272, 354 and chb.hpp:257, 344) only calls the merit function to seed values, so the whole chain (20 parameters) becomes const& although the check says "consider moving" for its outer layers; change the declarations in gecode/int.hh, set.hh, float.hh, and action.hpp:58/:108 and chb.hpp too. (4) TieBreak(VarBranch a0, ...) (tiebreak.hpp:65, declared :42) and the three tie_break templates (:70, :77, :84; declared :54, :57, :60) keep all 13 parameters by value and move them, for example `TieBreak<VarBranch> ab(std::move(a), std::move(b))`; the configuration classes declare no copy operations, so their implicit moves move the std::function members. Rules from the brief and task value-params-002: an owned or reference-counted parameter becomes const& unless the callee keeps the value and the type has a real move constructor; then it stays by value and is moved on every path to its storage. A std::move that binds a copying const& constructor does not count as a move: SharedHandle types (IntSet, DFA, TupleSet, SharedArray, TraceFilter), the *Args arrays, FloatVal, Rnd, and the AFC/Action/CHB handles have no real move, so they become const& and the member copy (or copy assignment) from the const& is the only copy. Shallow handles matched by the inventory's Proposed AllowedTypes (variables, views, Home, groups, Pos, Reify, VarArray/ViewArray, open variable lists, VarType) stay by value. A virtual override changes together with its base declaration. Change every other declaration the inventory row lists. Within a forwarding chain the treatment follows the final destination (inventory "What the diagnostics mean here", items 3-6). Public header signature changes are accepted for Gecode 7: they change the ABI but not source compatibility. Do not change ownership or lifetime. Verify the storage paths by source inspection; no new permanent test is needed (brief, Testing).

## Boundaries

- Brancher posting (G7, G8: the branch(...) entry points, post_*_brancher, BrancherFilter/BrancherPrint, and moving IntVarBranch vars into TieBreak in gecode/int/branch.cpp), SharedData (G2), and FlatZinc's IntBoolVarBranch (G13) belong to later children.
- Do not edit .clang-tidy, docs/code-style.md, or changelog.in; task value-params-003 enables the check, documents the rule, and writes the 7.0.0 changelog entry about the changed public signatures. Do not edit anything under .zdev.
- The parent task value-params-002 keeps the area's final checks: the full inventory rerun with no diagnostic outside AllowedTypes, both builds with ctest, and `uv run misc/check.py all --build-dir build/full --jobs 12`.
- Keep behaviour, ownership, and lifetimes unchanged, and add no permanent test or allocator instrumentation. Throwaway probes stay under build/.
- Shared top-level headers (gecode/int.hh, set.hh, float.hh, kernel.hh, minimodel.hh, kernel/core.hpp, and the module .hh files) are edited by several children in turn; the blocked_by chain keeps the children serial. Change only the declarations of this child's groups in them.
- Initial task-owned path allocation: ["gecode/kernel/branch/action.hpp","gecode/kernel/branch/chb.hpp","gecode/kernel/branch/tiebreak.hpp","gecode/kernel/branch/val.hpp","gecode/kernel/branch/var.hpp","gecode/int/branch/action.cpp","gecode/int/branch/assign.hpp","gecode/int/branch/chb.cpp","gecode/int/branch/val.hpp","gecode/int/branch/var.hpp","gecode/int.hh","gecode/set/branch/action.cpp","gecode/set/branch/assign.hpp","gecode/set/branch/chb.cpp","gecode/set/branch/val.hpp","gecode/set/branch/var.hpp","gecode/set.hh","gecode/float/branch/action.cpp","gecode/float/branch/assign.hpp","gecode/float/branch/chb.cpp","gecode/float/branch/val.hpp","gecode/float/branch/var.hpp","gecode/float.hh"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [x] The inventory reproduction rerun in build/zdev-audit/value-params-branch-configs/ reports no diagnostic for G3, G4, G5, and G6: its rows.tsv has no row in these groups (group column, matched by group number), it has no row (compared by file, function, and parameter) that the baseline build/zdev-audit/value-params/rows.tsv lacks, and checks.txt is empty.
- [x] The result lists every parameter changed to const& whose callee keeps a copy, by function, parameter, and file:line (at least the inventory's Verification hooks rows for G3-G6), with the member or object it initializes and how the copy is made (copy constructor or copy assignment from the const&).
- [x] The result shows, with source references, that every by-value-and-move parameter of G3-G6 reaches its storage through a move constructor (the implicit move constructors of IntVarBranch, BoolVarBranch, SetVarBranch, FloatVarBranch, the *ValBranch and *Assign classes, and TieBreak, and std::function's move into VarBranch::_tbl and _mf and ValBranch::vf and cf), and that no std::move binds a copying const& constructor.
- [x] build/full and build/local build, and ctest passes in both.

## Validation

- Build build/full (Debug, configured as in the inventory's Reproduce step 1: experimental, CBS, fault injection, audit, and Gist) and the default Release configuration build/local, and run ctest in both.
- In build/full, run `bin/gecode-test -test Branch -tag standard`, `-test LDSB -tag standard`, and `-test NoGoods -tag standard`.
- Run `uv run misc/check.py format-check --build-dir build/full`.
- Rerun the inventory reproduction (Reproduce steps 2 and 3 in .zdev/value-params/background/value-param-inventory.md) into a fresh directory build/zdev-audit/value-params-branch-configs/ so the baseline in build/zdev-audit/value-params/ stays intact: `sh build/zdev-audit/value-params-branch-configs/reproduce.sh build/full 10`.

## Result

G3-G6: Rnd/AFC/Action/CHB handles, VarBranch setters and the Action/CHB merit chain by const&; branch tables, merit, value and commit callbacks and TieBreak arguments by value and moved into std::function and configuration storage

Validation:

- build/full (Debug, experimental, CBS, fault injection, audit, Gist) and build/local (Release) build without warnings; ctest 8/8 and 7/7
- gecode-test -tag standard: Branch 17/17, LDSB 38/38, NoGoods 161/161
- format-check clean
- Scoped inventory rerun: 417 parameters, no G3-G6 row, no new row versus baseline, checks.txt empty
- Copy-count probe: callback copies 2/1/4/8/8/4/0/0 -> 1/0/2/2/2/0/0/0
- Independent Codex verifier: pass
