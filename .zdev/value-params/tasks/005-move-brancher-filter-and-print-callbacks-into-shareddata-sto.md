+++
schema_version = 1
id = "value-params-005"
key = "brancher-posting"
area = "value-params"
status = "open"
complexity = "advanced"
afk = true
blocked_by = ["value-params-004"]
+++
# Move brancher filter and print callbacks into SharedData storage

## Outcome

SharedData accepts its data by move, branch filter and print callbacks reach BrancherFilter, BrancherPrint, and OpenListBrancher storage through moves on every posting path, read-only configurations are passed by const&, and the inventory reports no diagnostic for G7 and G8.

## Context

Repair groups G2 (SharedData rvalue sink, no parameters), G7 (kernel and int brancher posting, 111 parameters), and G8 (set and float brancher posting, 58) from .zdev/value-params/background/value-param-inventory.md, in that order: G7 and G8 depend on G2, and G8 reuses the kernel layers G7 changes. Follow the inventory section "Stored callbacks and SharedData" (its receiver table and the paragraphs after it), the "TieBreak and tie_break" bullet under "Other stored values", "Repair groups", "Verification hooks", and the per-parameter tables G7 and G8. G2: SharedData<Data> (gecode/kernel/data/shared-data.hpp) has only SharedData(const Data&) and SDO(const Data&) with a `const Data d` member, so every store copies. Add SharedData(Data&& d) : SharedHandle(new SDO(std::move(d))) and SDO(Data&& d0) : d(std::move(d0)) beside them; then ValSelFunction, ValCommitFunction, MeritFunction, ViewSelChooseTbl, and OpenVarList<Var>::Object move from their temporaries without other changes. G7: filter and print callbacks stay by value and move on every path down to BrancherFilter (gecode/kernel/branch/filter.hpp:91), BrancherPrint (print.hpp:94), and OpenListBrancher (gecode/int/branch/open.hpp:58). Delicate cases: BrancherFilter tests `if (!bf)` after f(bf) and BrancherPrint tests `if (!vvp)` after p(vvp); after moving, test the stored value (`if (!f())`, `if (!p())`) or test before the initializer. BrancherNoFilter and BrancherNoPrint (filter.hpp:129, print.hpp:133) only assert emptiness and take const&. The `if (bf) ... else ...` dispatch in post_view_value_brancher, post_view_values_brancher, post_open_brancher, post_ldsb_brancher, and post_ldsb_set_brancher passes the callback once per branch, so moving is safe. *ValBranch/*Assign and posting functions that only read a TieBreak take const&; the public branch(Home, ..., IntVarBranch vars, ...) builds TieBreak<IntVarBranch>(vars) (gecode/int/branch.cpp:309, 322), so vars moves there (and the BoolVarBranch form likewise). If 111 parameters are too many for one review, the inventory's G7a/G7b split (kernel and int layers, then the public entry points) is a commit order inside this child, not separate tasks. G8 applies the same treatment to set and float. Rules from the brief and task value-params-002: an owned or reference-counted parameter becomes const& unless the callee keeps the value and the type has a real move constructor; then it stays by value and is moved on every path to its storage. A std::move that binds a copying const& constructor does not count as a move: SharedHandle types (IntSet, DFA, TupleSet, SharedArray, TraceFilter), the *Args arrays, FloatVal, Rnd, and the AFC/Action/CHB handles have no real move, so they become const& and the member copy (or copy assignment) from the const& is the only copy. Shallow handles matched by the inventory's Proposed AllowedTypes (variables, views, Home, groups, Pos, Reify, VarArray/ViewArray, open variable lists, VarType) stay by value. A virtual override changes together with its base declaration. Change every other declaration the inventory row lists. Within a forwarding chain the treatment follows the final destination (inventory "What the diagnostics mean here", items 3-6). Public header signature changes are accepted for Gecode 7: they change the ABI but not source compatibility. Do not change ownership or lifetime. Verify the storage paths by source inspection; no new permanent test is needed (brief, Testing).

## Boundaries

- Keep the existing SharedData(const Data&) and SDO(const Data&) constructors and SharedData's copy semantics; only add the rvalue path.
- The execution callbacks (wait, when, function branch: G10) and open variable lists (G11) also end in SharedData but belong to the next child.
- Do not edit .clang-tidy, docs/code-style.md, or changelog.in; task value-params-003 enables the check, documents the rule, and writes the 7.0.0 changelog entry about the changed public signatures. Do not edit anything under .zdev.
- The parent task value-params-002 keeps the area's final checks: the full inventory rerun with no diagnostic outside AllowedTypes, both builds with ctest, and `uv run misc/check.py all --build-dir build/full --jobs 12`.
- Keep behaviour, ownership, and lifetimes unchanged, and add no permanent test or allocator instrumentation. Throwaway probes stay under build/.
- Shared top-level headers (gecode/int.hh, set.hh, float.hh, kernel.hh, minimodel.hh, kernel/core.hpp, and the module .hh files) are edited by several children in turn; the blocked_by chain keeps the children serial. Change only the declarations of this child's groups in them.
- Initial task-owned path allocation: ["gecode/kernel/data/shared-data.hpp","gecode/kernel/branch/filter.hpp","gecode/kernel/branch/print.hpp","gecode/kernel/branch/view-val.hpp","gecode/kernel/branch/view.hpp","gecode/int/branch.cpp","gecode/int/branch.hpp","gecode/int/branch/open.hpp","gecode/int/branch/view-values.hpp","gecode/int/ldsb.cpp","gecode/int/ldsb/brancher.hpp","gecode/int/branch.hh","gecode/int/ldsb.hh","gecode/float/branch.cpp","gecode/float/branch.hpp","gecode/set/branch.cpp","gecode/set/branch.hpp","gecode/set/ldsb.cpp","gecode/set/ldsb/brancher.hpp","gecode/set/ldsb.hh"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [ ] gecode/kernel/data/shared-data.hpp has SharedData(Data&&) and SDO(Data&&) that move the data into the SDO member, and the receivers BrancherFilter, BrancherPrint, OpenListBrancher, ValSelFunction, ValCommitFunction, MeritFunction, and ViewSelChooseTbl reach SDO through it.
- [ ] BrancherFilter and BrancherPrint no longer read their parameter after moving from it.
- [ ] The inventory reproduction rerun in build/zdev-audit/value-params-brancher-posting/ reports no diagnostic for G7 and G8: its rows.tsv has no row in these groups (group column, matched by group number), it has no row (compared by file, function, and parameter) that the baseline build/zdev-audit/value-params/rows.tsv lacks, and checks.txt is empty.
- [ ] The result lists every parameter changed to const& whose callee keeps a copy, by function, parameter, and file:line (at least the inventory's Verification hooks rows for G7 and G8 (the inventory lists none; report any found)), with the member or object it initializes and how the copy is made (copy constructor or copy assignment from the const&).
- [ ] The result shows, with source references, that every by-value-and-move parameter of G7 and G8 reaches its storage through a move constructor (std::function's move constructor at each layer and SDO(Data&&) at each receiver, traced from each public branch(...) and assign(...) entry point), and that no std::move binds a copying const& constructor.
- [ ] build/full and build/local build, and ctest passes in both.

## Validation

- Build build/full (Debug, configured as in the inventory's Reproduce step 1: experimental, CBS, fault injection, audit, and Gist) and the default Release configuration build/local, and run ctest in both.
- Because SharedData construction changes, run `cmake --build build/full --target check-fault`.
- In build/full, run `bin/gecode-test -test Branch -tag standard`, `-test LDSB -tag standard`, `-test Int::OpenList -tag standard`, and `-test NoGoods -tag standard`.
- Run `uv run misc/check.py format-check --build-dir build/full`.
- Rerun the inventory reproduction (Reproduce steps 2 and 3 in .zdev/value-params/background/value-param-inventory.md) into a fresh directory build/zdev-audit/value-params-brancher-posting/ so the baseline in build/zdev-audit/value-params/ stays intact: `sh build/zdev-audit/value-params-brancher-posting/reproduce.sh build/full 10`.
- Optional (brief, Testing): rerun the throwaway counted callback probe from 9396a93c24 (build/callback-ownership-probe.cpp) and confirm callback copy counts do not regress. Do not commit it.
