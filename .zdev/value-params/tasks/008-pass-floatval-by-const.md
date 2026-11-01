+++
schema_version = 1
id = "value-params-008"
key = "floatval"
area = "value-params"
status = "open"
complexity = "standard"
afk = true
blocked_by = ["value-params-007"]
+++
# Pass FloatVal by const&

## Outcome

FloatVal parameters in the float propagators, views, posting functions, minimodel, and float tests are passed by const FloatVal&, and the inventory reports no diagnostic for G16.

## Context

Repair group G16 (FloatVal, 53 parameters: gecode/float 39, gecode/minimodel 6, test 8) from .zdev/value-params/background/value-param-inventory.md. The brief defines a shallow handle as a pointer-sized copy with no reference count or allocation. FloatVal is two doubles (16 bytes), so it is not a shallow handle and this child is in scope by default. The inventory's "Policy exception for the user: FloatVal" offers allowing FloatVal as a user decision. If the user adopts that exception before this child starts, this child is cancelled and `;^Gecode::FloatVal$` is appended to AllowedTypes in task value-params-003 instead. No other child depends on this one, which is why it runs last. Follow that section, "Repair groups", "Verification hooks", and the per-parameter table G16. FloatVal declares a copy constructor and no move constructor (gecode/float/val.hpp:55), so every row becomes const FloatVal& and no row moves. Where the callee keeps the value, the member copy from the const& is the only copy; the keepers are Lin c0, ReEqFloat c0, ReLqFloat c0, ReLeFloat c0, NqFloat _c, ScaleView d, DomArgs args, LinFloatExpr a (two constructors), LinFloatExpr::Node::fill m, LinFloatRel r and l, and the test's Val c0. DomArgs (gecode/minimodel/dom.cpp:105) reaches storage through std::forward in an initializer; follow its row, whose note the inventory's Validation section corrected. G16's test rows (test/float.hh, float.hpp, float.cpp, float/basic.cpp, float/dom.cpp) are disjoint from G15's files. Rules from the brief and task value-params-002: an owned or reference-counted parameter becomes const& unless the callee keeps the value and the type has a real move constructor; then it stays by value and is moved on every path to its storage. A std::move that binds a copying const& constructor does not count as a move: SharedHandle types (IntSet, DFA, TupleSet, SharedArray, TraceFilter), the *Args arrays, FloatVal, Rnd, and the AFC/Action/CHB handles have no real move, so they become const& and the member copy (or copy assignment) from the const& is the only copy. Shallow handles matched by the inventory's Proposed AllowedTypes (variables, views, Home, groups, Pos, Reify, VarArray/ViewArray, open variable lists, VarType) stay by value. A virtual override changes together with its base declaration. Change every other declaration the inventory row lists. Within a forwarding chain the treatment follows the final destination (inventory "What the diagnostics mean here", items 3-6). Public header signature changes are accepted for Gecode 7: they change the ABI but not source compatibility. Do not change ownership or lifetime. Verify the storage paths by source inspection; no new permanent test is needed (brief, Testing).

## Boundaries

- Do not add FloatVal to AllowedTypes or edit .clang-tidy here; that is the alternative to this child, and it belongs to task value-params-003.
- Do not add std::move for FloatVal.
- Do not edit .clang-tidy, docs/code-style.md, or changelog.in; task value-params-003 enables the check, documents the rule, and writes the 7.0.0 changelog entry about the changed public signatures. Do not edit anything under .zdev.
- The parent task value-params-002 keeps the area's final checks: the full inventory rerun with no diagnostic outside AllowedTypes, both builds with ctest, and `uv run misc/check.py all --build-dir build/full --jobs 12`.
- Keep behaviour, ownership, and lifetimes unchanged, and add no permanent test or allocator instrumentation. Throwaway probes stay under build/.
- Shared top-level headers (gecode/int.hh, set.hh, float.hh, kernel.hh, minimodel.hh, kernel/core.hpp, and the module .hh files) are edited by several children in turn; the blocked_by chain keeps the children serial. Change only the declarations of this child's groups in them.
- Initial task-owned path allocation: ["gecode/float/dom.cpp","gecode/float/linear.cpp","gecode/float/linear.hh","gecode/float/linear/nary.hpp","gecode/float/linear/post.cpp","gecode/float/rel.cpp","gecode/float/rel.hh","gecode/float/rel/eq.hpp","gecode/float/rel/lq-le.hpp","gecode/float/rel/nq.hpp","gecode/float/view.hpp","gecode/float/view/float.hpp","gecode/float/view/minus.hpp","gecode/float/view/offset.hpp","gecode/float/view/rel-test.hpp","gecode/float/view/scale.hpp","gecode/minimodel/dom.cpp","gecode/minimodel/float-expr.cpp","gecode/minimodel/float-rel.hpp","test/float.cpp","test/float.hh","test/float.hpp","test/float/basic.cpp","test/float/dom.cpp"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [ ] The inventory reproduction rerun in build/zdev-audit/value-params-floatval/ reports no diagnostic for G16: its rows.tsv has no row in these groups (group column, matched by group number), it has no row (compared by file, function, and parameter) that the baseline build/zdev-audit/value-params/rows.tsv lacks, and checks.txt is empty.
- [ ] The result lists every parameter changed to const& whose callee keeps a copy, by function, parameter, and file:line (at least the inventory's Verification hooks rows for G16), with the member or object it initializes and how the copy is made (copy constructor or copy assignment from the const&).
- [ ] No G16 parameter stays by value or is moved, and build/full and build/local build with ctest passing in both.

## Validation

- Build build/full (Debug, configured as in the inventory's Reproduce step 1: experimental, CBS, fault injection, audit, and Gist) and the default Release configuration build/local, and run ctest in both.
- In build/full, run `bin/gecode-test -test Float -tag standard`.
- Run `uv run misc/check.py format-check --build-dir build/full`.
- Rerun the inventory reproduction (Reproduce steps 2 and 3 in .zdev/value-params/background/value-param-inventory.md) into a fresh directory build/zdev-audit/value-params-floatval/ so the baseline in build/zdev-audit/value-params/ stays intact: `sh build/zdev-audit/value-params-floatval/reproduce.sh build/full 10`.
