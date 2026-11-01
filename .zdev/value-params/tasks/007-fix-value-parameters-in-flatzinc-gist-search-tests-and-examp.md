+++
schema_version = 1
id = "value-params-007"
key = "flatzinc-gist-tests"
area = "value-params"
status = "open"
complexity = "advanced"
afk = true
blocked_by = ["value-params-006"]
+++
# Fix value parameters in FlatZinc, Gist, search, tests, and examples

## Outcome

FlatZinc (including the grammar and its regenerated parser), Gist, the CPProfiler tracer, the tests, and the examples pass owned values by const& or move them into storage, and the inventory reports no diagnostic for G13, G14, and G15.

## Context

Repair groups G13 (FlatZinc, 56 parameters), G14 (Gist and search, 7), and G15 (tests and examples, 43) from .zdev/value-params/background/value-param-inventory.md. G13 uses the signatures of G4-G8 and G15 mirrors library signatures, so this child runs after the library children (G15 last among the library groups; G16's float tests are in G16 and in disjoint files). Follow the "Other stored values" bullets FlatZinc, FlatZinc parser, and Tests, "Repair groups", "Verification hooks", and the per-parameter tables G13, G14, and G15. G13: AuxVarBrancher (gecode/flatzinc/flatzinc.cpp:113) stores all eight selectors and its post (:246) forwards them, so both stay by value and move; BranchInformationO(..., v0) (:286) and BlackBoxContext::fail(std::exception_ptr e) (gecode/flatzinc/blackbox-propagator.cpp:143) move. The IntBoolVarBranch constructors and int_bool_var_* factories (gecode/flatzinc/branch.hpp) and BlackBoxBounds sub_int0 and sub_float0 keep handles without a move, so they take const& (the verification-hook keepers). The three const std::string parameters in flatzinc.cpp become const std::string&. ParserState::output(std::string x, AST::Node* n) (gecode/flatzinc/parser.hh:258) stays by value and uses emplace_back(std::move(x), n). The three parser.tab.cpp parameters come from gecode/flatzinc/parser.yxx:98, :176, and :200 and only read id, so they become const std::string&: edit the grammar, never parser.tab.cpp, and build the CMake `regenerate` target with Homebrew Bison 3.8 and Flex 2.6 first on PATH (the macOS system Bison is too old), then check with .github/ci/generated-source-freshness.sh. G14: Gist comparator name strings become const&, and the CPProfiler tracer name moves. G15: fixture constructors move strings, vectors, shared_ptrs, and callbacks. Delicate case: NoGoods(TestTags, ValBranch vb0, ...) (test/nogoods.cpp:250) reads Model::val(vb0) in its Base initializer before members are initialized; keep that read and write vb(std::move(vb0)). Labels that test/ldsb.cpp and test/int/gcc.cpp forward to a base constructor become const&, and TupleSetTest ts0 (test/int/extensional.cpp:1194) is a const& keeper. Rules from the brief and task value-params-002: an owned or reference-counted parameter becomes const& unless the callee keeps the value and the type has a real move constructor; then it stays by value and is moved on every path to its storage. A std::move that binds a copying const& constructor does not count as a move: SharedHandle types (IntSet, DFA, TupleSet, SharedArray, TraceFilter), the *Args arrays, FloatVal, Rnd, and the AFC/Action/CHB handles have no real move, so they become const& and the member copy (or copy assignment) from the const& is the only copy. Shallow handles matched by the inventory's Proposed AllowedTypes (variables, views, Home, groups, Pos, Reify, VarArray/ViewArray, open variable lists, VarType) stay by value. A virtual override changes together with its base declaration. Change every other declaration the inventory row lists. Within a forwarding chain the treatment follows the final destination (inventory "What the diagnostics mean here", items 3-6). Public header signature changes are accepted for Gecode 7: they change the ABI but not source compatibility. Do not change ownership or lifetime. Verify the storage paths by source inspection; no new permanent test is needed (brief, Testing).

## Boundaries

- Change the generated parser only by regenerating it from parser.yxx; keep the lexer unchanged unless regeneration rewrites it.
- Do not change test behaviour, coverage, or test names.
- Do not edit .clang-tidy, docs/code-style.md, or changelog.in; task value-params-003 enables the check, documents the rule, and writes the 7.0.0 changelog entry about the changed public signatures. Do not edit anything under .zdev.
- The parent task value-params-002 keeps the area's final checks: the full inventory rerun with no diagnostic outside AllowedTypes, both builds with ctest, and `uv run misc/check.py all --build-dir build/full --jobs 12`.
- Keep behaviour, ownership, and lifetimes unchanged, and add no permanent test or allocator instrumentation. Throwaway probes stay under build/.
- Shared top-level headers (gecode/int.hh, set.hh, float.hh, kernel.hh, minimodel.hh, kernel/core.hpp, and the module .hh files) are edited by several children in turn; the blocked_by chain keeps the children serial. Change only the declarations of this child's groups in them.
- Initial task-owned path allocation: ["gecode/flatzinc/blackbox-propagator.cpp","gecode/flatzinc/blackbox.hh","gecode/flatzinc/branch.cpp","gecode/flatzinc/branch.hpp","gecode/flatzinc/branch.hh","gecode/flatzinc/flatzinc.cpp","gecode/flatzinc/parser.hh","gecode/flatzinc/parser.yxx","gecode/flatzinc/parser.tab.cpp","gecode/flatzinc/parser.tab.hpp","gecode/gist/gist.hpp","gecode/gist.hh","gecode/search/cpprofiler/tracer.cpp","gecode/search.hh","examples/job-shop.cpp","examples/pentominoes.cpp","test/branch/int.cpp","test/flatzinc.cpp","test/flatzinc.hh","test/instance.cpp","test/int/extensional.cpp","test/int/gcc.cpp","test/int/min-distance.cpp","test/int/open-var-list.cpp","test/ldsb.cpp","test/nogoods.cpp","test/search.cpp","test/test.cpp","test/test.hh"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [ ] parser.yxx carries the const std::string& changes, and the regenerated parser sources are fresh according to .github/ci/generated-source-freshness.sh.
- [ ] The inventory reproduction rerun in build/zdev-audit/value-params-flatzinc-gist-tests/ reports no diagnostic for G13, G14, and G15: its rows.tsv has no row in these groups (group column, matched by group number), it has no row (compared by file, function, and parameter) that the baseline build/zdev-audit/value-params/rows.tsv lacks, and checks.txt is empty.
- [ ] The result lists every parameter changed to const& whose callee keeps a copy, by function, parameter, and file:line (at least the inventory's Verification hooks rows for G13 and G15), with the member or object it initializes and how the copy is made (copy constructor or copy assignment from the const&).
- [ ] The result shows, with source references, that every by-value-and-move parameter of G13, G14, and G15 reaches its storage through a move constructor (the implicit move constructors of the configuration classes and TieBreak, and the move constructors of std::string, std::vector, std::shared_ptr, std::exception_ptr, and std::function), and that no std::move binds a copying const& constructor.
- [ ] build/full and build/local build, and ctest passes in both.

## Validation

- Build build/full (Debug, configured as in the inventory's Reproduce step 1: experimental, CBS, fault injection, audit, and Gist) and the default Release configuration build/local, and run ctest in both.
- Build the `regenerate` target with Homebrew Bison 3.8 and Flex 2.6 on PATH and run .github/ci/generated-source-freshness.sh.
- In build/full, run `bin/gecode-test -tag standard` with `-test FlatZinc`, `-test Instance`, `-test NoGoods`, `-test LDSB`, `-test Int::Extensional`, `-test Int::GCC`, `-test Int::MinDistance`, `-test Int::OpenList`, and `-test Search`. Confirm the Gist translation units and the touched examples build in build/full.
- Run `uv run misc/check.py format-check --build-dir build/full`.
- Rerun the inventory reproduction (Reproduce steps 2 and 3 in .zdev/value-params/background/value-param-inventory.md) into a fresh directory build/zdev-audit/value-params-flatzinc-gist-tests/ so the baseline in build/zdev-audit/value-params/ stays intact: `sh build/zdev-audit/value-params-flatzinc-gist-tests/reproduce.sh build/full 10`.
