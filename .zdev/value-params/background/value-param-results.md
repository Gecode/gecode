# value-params final results (task value-params-002)

HEAD `cae26c0d61` on `feature/breaking-7`. Children: 004 `1f4a7e2dab` (G3-G6),
005 `1c0d3c4b5a` (G2, G7, G8), 006 `6d3fc2253a` (G1, G9-G12),
007 `5d0bc37735` (G13-G15), 008 `cae26c0d61` (G16). Baseline rows:
`build/zdev-audit/value-params/rows.tsv` (708 rows at `f91fbadafc`). Every
HEAD `file:line` below was looked up in the HEAD source, not copied from the
baseline: the row's overload is matched by its ordinal among same-named
signatures that declare the parameter. The mapping is
`build/vp002-scripts/mapped.json`.

## Method for the kept-parameter lists (A1-A4 and "const& other")

The lists come from a script pass over the diff plus source inspection. Every
script is in `build/vp002-scripts/`; run them from the repository root.

1. `python3 build/vp002-scripts/diff_params.py build/vp002-scripts/diff.json`
   reads `git diff -U0 02c2ff74f3 HEAD -- gecode test examples tools` and
   lists every parameter that a `+` line declares as `const T& name` while the
   hunk's `-` lines declare `name` without `&`: 652 signatures. A looser
   pattern (`const <anything>& name`) over the same diff adds 12 more, the
   multi-line `const typename BranchTraits<...>::Merit& bm` parameters in
   gecode/kernel/branch/action.hpp and chb.hpp. These are appended in
   `diff_all.json`.
2. `python3 build/vp002-scripts/uses.py build/vp002-scripts/diff_all.json
   build/vp002-scripts/uses.json` drops declarations and keeps the 362
   definitions. For each it extracts the constructor initializer list and the
   body, with comments and literals blanked, and records every use of the
   parameter name: member access, argument of call `f`, assignment, return,
   or other.
3. The 362 definitions are matched to the 708 baseline rows
   (`build/vp002-scripts/mapped.json`, HEAD lines) by HEAD line and name. 344
   match a baseline `const&` row. The other 2 baseline `const&` rows are the
   unnamed parameters of the G1 base declarations. Of the 18 unmatched
   definitions, 3 are the generator source of baseline rows
   (gecode/flatzinc/parser.yxx:98, :176 and :200 generate parser.tab.cpp:133,
   :211 and :235), and 15 are outside the baseline (A4 and the last table
   below).
4. Every use that passes the parameter on was followed, by hand, through each
   callee until the value is dropped or stored. That includes by-value hops
   (where the hop itself copies), the `branch`/`assign` overloads, propagator
   `post` functions, and handle-sharing operators such as `REG::operator|`.
   A trace also ends where only a value computed from the parameter survives;
   such a row is "const& other", and its Use cell says "derived".
   The decisions are written out in `build/vp002-scripts/classify.py`, which
   asserts that every baseline `const&` row has one and writes the tables
   below (`a-sections.md`, `counts.md`).

A parameter counts as **kept** when, after the call, a live object holds a
copy of the parameter itself, a copy of one of its members or elements, or a
handle that shares its representation (a SharedHandle copy, or a
REG/LinFloatExpr/AST node that references or copies it). Values *computed*
from the parameter are derived results, not keeps, and carry no lifetime or
copy risk. This covers formatted strings, arithmetic products such as
`m * a`, set intersections, automata or graphs built from its contents, and
state indices. Domain updates, comparisons, stream output and scalars computed
from it are also not kept.

## Validation outcomes

- Tools: Homebrew LLVM 22.1.8 (clang-tidy, clang-format, run-clang-tidy), bison
  3.8.2, flex 2.6.4, Doxygen 1.18.0.
- Builds: `cmake --build build/full` plus `--target gecode-test` gave "ninja: no
  work to do" (rc 0; build/vp002-full-build.log). `cmake --build build/local`
  plus `--target gecode-test` gave rc 0 with 0 warnings
  (build/vp002-local-build.log).
- Inventory (done-condition 1): `sh build/zdev-audit/value-params-final/reproduce.sh
  build/full 10` with the default AllowedTypes over all compile_commands
  entries (build/vp002-inventory.log, rc 0). Every pass log reports `exit 0`.
  - `no-allowed raw 54588 unique 1176`
  - `allowed raw 0 unique 0`
  - `allowed-floatval raw 0 unique 0`
  - `unique parameters 0 fix-its 0 with other declarations 0 checks 0`
  - `checks.txt` is empty and `rows.tsv` has only its header line.
- ctest: build/full 8/8 passed (build/vp002-full-ctest.log), build/local 7/7
  passed (build/vp002-local-ctest.log).
- `uv run misc/check.py all --build-dir build/full --jobs 12`: `check-all rc=0` (build/vp002-check-all.log).
  Format check of 1137 files, "Changelog tidy check passed", "Generated source
  freshness check passed", the build, lint (clang-tidy on 486 of 490 entries,
  no warnings), ctest 8/8, the installed-package checks (allocator 1/1 and
  public-test-component 1/1, each linted), and the doc target all passed.
  Lint uses the repository .clang-tidy, which does not enable
  performance-unnecessary-value-param yet (task 003), so the inventory above
  is the acceptance check for this rule.
- No failures, so none needed attributing against 02c2ff74f3, and no fixes
  were made. `git status --short` is clean.

## Counts per group (baseline rows by treatment at HEAD)

A1 means the callee keeps a copy itself, or returns an object that holds one.
A2 means a base or delegated constructor keeps it. A3 means the parameter is
forwarded, through any chain of calls, to a function that keeps a copy. A4
lists changed parameters outside the baseline rows that are kept; they are
counted separately. "const& other" covers parameters that nothing keeps; a
Use cell that says "derived" names a value computed from the parameter.
"by value + move" means the parameter stays by value and is moved. The final
inventory reports 0 rows in every group.

| Group | Baseline rows | A1 kept | A2 base-ctor | A3 forwarded | const& other | by value + move | A4 (outside baseline) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| G1 | 4 | 0 | 0 | 0 | 4 | 0 | 0 |
| G3 | 27 | 8 | 0 | 0 | 4 | 15 | 0 |
| G4 | 120 | 24 | 12 | 0 | 8 | 76 | 0 |
| G5 | 73 | 17 | 6 | 0 | 4 | 46 | 0 |
| G6 | 71 | 15 | 6 | 0 | 4 | 46 | 0 |
| G7 | 111 | 0 | 0 | 30 | 6 | 75 | 0 |
| G8 | 58 | 0 | 0 | 18 | 2 | 38 | 0 |
| G9 | 48 | 3 | 6 | 16 | 23 | 0 | 4 |
| G10 | 22 | 0 | 0 | 0 | 0 | 22 | 0 |
| G11 | 6 | 0 | 0 | 0 | 0 | 6 | 0 |
| G12 | 9 | 2 | 0 | 7 | 0 | 0 | 0 |
| G13 | 56 | 21 | 0 | 10 | 5 | 20 | 0 |
| G14 | 7 | 0 | 1 | 0 | 5 | 1 | 0 |
| G15 | 43 | 3 | 3 | 11 | 9 | 17 | 0 |
| G16 | 53 | 12 | 3 | 17 | 21 | 0 | 1 |
| Total | 708 | 105 | 37 | 109 | 95 | 362 | 5 |

708 baseline rows: 346 `const&` (105 + 37 + 109 + 95) and 362 by value plus
move. With the 5 A4 entries, 713 parameters are classified. Ten more changed
parameters outside the baseline are not kept; they are listed after the
"const& other" table and not counted.

## A. Kept `const&` parameters

The kept types are `SharedHandle`s (Rnd, AFC/Action/CHB and their
Int/Bool/Set/Float subclasses, TraceFilter, DFA, IntSet, TupleSet,
SharedArray/IntSharedArray), `ArgArray`s (IntArgs, the Matrix `A`),
FloatVal, REG, std::string, and the branching configurations (`*ValBranch`,
`*Assign`, `TieBreak<*VarBranch>`), whose members are copied. None of the
handle types has a move that is cheaper than its copy, so the keeper
copy-constructs or copy-assigns from the `const&`, and that copy is the only
one; the caller's object is never copied for the call itself. Where a path
crosses a by-value parameter, the table says so: that hop makes the copy from
the `const&`. A branching configuration is not stored whole: the selectors
copy its `Rnd` handle or its std::function members through by-value
accessors. No keeper holds a reference or pointer to the argument (each stores
by value or shares a reference-counted representation), so no lifetime
changed.

### A1. The callee itself keeps the copy

| Group | Function | Parameter | Type | HEAD parameter | Copy site | How the copy is made |
| --- | --- | --- | --- | --- | --- | --- |
| G3 | `ValBranch::ValBranch<Var>` | `r0` | `Rnd` | gecode/kernel/branch/val.hpp:74 | gecode/kernel/branch/val.hpp:74 | copy-constructs `r`; the only copy |
| G3 | `VarBranch::VarBranch<Var>` | `a` | `AFC` | gecode/kernel/branch/var.hpp:125 | gecode/kernel/branch/var.hpp:125 | copy-constructs `_afc`; the only copy |
| G3 | `VarBranch::VarBranch<Var>` | `a` | `Action` | gecode/kernel/branch/var.hpp:131 | gecode/kernel/branch/var.hpp:131 | copy-constructs `_act`; the only copy |
| G3 | `VarBranch::VarBranch<Var>` | `c` | `CHB` | gecode/kernel/branch/var.hpp:137 | gecode/kernel/branch/var.hpp:137 | copy-constructs `_chb`; the only copy |
| G3 | `VarBranch::VarBranch<Var>` | `r` | `Rnd` | gecode/kernel/branch/var.hpp:143 | gecode/kernel/branch/var.hpp:143 | copy-constructs `_rnd`; the only copy |
| G3 | `VarBranch::afc` | `a` | `AFC` | gecode/kernel/branch/var.hpp:178 | gecode/kernel/branch/var.hpp:179 | copy-assigns `_afc`; the only copy |
| G3 | `VarBranch::action` | `a` | `Action` | gecode/kernel/branch/var.hpp:190 | gecode/kernel/branch/var.hpp:191 | copy-assigns `_act`; the only copy |
| G3 | `VarBranch::chb` | `chb` | `CHB` | gecode/kernel/branch/var.hpp:202 | gecode/kernel/branch/var.hpp:203 | copy-assigns `_chb`; the only copy |
| G4 | `INT_ASSIGN_RND` | `r` | `Rnd` | gecode/int/branch/assign.hpp:68 | gecode/int/branch/assign.hpp:69 | returns a `IntAssign` from its `const&` constructor; chain: IntAssign(r) gecode/int/branch/assign.hpp:40 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G4 | `BOOL_ASSIGN_RND` | `r` | `Rnd` | gecode/int/branch/assign.hpp:105 | gecode/int/branch/assign.hpp:106 | returns a `BoolAssign` from its `const&` constructor; chain: BoolAssign(r) gecode/int/branch/assign.hpp:82 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G4 | `INT_VAL_RND` | `r` | `Rnd` | gecode/int/branch/val.hpp:68 | gecode/int/branch/val.hpp:69 | returns a `IntValBranch` from its `const&` constructor; chain: IntValBranch(r) gecode/int/branch/val.hpp:40 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G4 | `BOOL_VAL_RND` | `r` | `Rnd` | gecode/int/branch/val.hpp:135 | gecode/int/branch/val.hpp:136 | returns a `BoolValBranch` from its `const&` constructor; chain: BoolValBranch(r) gecode/int/branch/val.hpp:112 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G4 | `INT_VAR_RND` | `r` | `Rnd` | gecode/int/branch/var.hpp:108 | gecode/int/branch/var.hpp:109 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(r) gecode/int/branch/var.hpp:46 → VarBranch(r) gecode/kernel/branch/var.hpp:143 → copy-constructs `_rnd` at gecode/kernel/branch/var.hpp:143; the only copy |
| G4 | `INT_VAR_AFC_MIN` | `a` | `IntAFC` | gecode/int/branch/var.hpp:138 | gecode/int/branch/var.hpp:139 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(a) gecode/int/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G4 | `INT_VAR_AFC_MAX` | `a` | `IntAFC` | gecode/int/branch/var.hpp:148 | gecode/int/branch/var.hpp:149 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(a) gecode/int/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G4 | `INT_VAR_ACTION_MIN` | `a` | `IntAction` | gecode/int/branch/var.hpp:158 | gecode/int/branch/var.hpp:159 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(a) gecode/int/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G4 | `INT_VAR_ACTION_MAX` | `a` | `IntAction` | gecode/int/branch/var.hpp:168 | gecode/int/branch/var.hpp:169 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(a) gecode/int/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G4 | `INT_VAR_CHB_MIN` | `c` | `IntCHB` | gecode/int/branch/var.hpp:173 | gecode/int/branch/var.hpp:174 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(c) gecode/int/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G4 | `INT_VAR_CHB_MAX` | `c` | `IntCHB` | gecode/int/branch/var.hpp:183 | gecode/int/branch/var.hpp:184 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(c) gecode/int/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G4 | `INT_VAR_AFC_SIZE_MIN` | `a` | `IntAFC` | gecode/int/branch/var.hpp:238 | gecode/int/branch/var.hpp:239 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(a) gecode/int/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G4 | `INT_VAR_AFC_SIZE_MAX` | `a` | `IntAFC` | gecode/int/branch/var.hpp:248 | gecode/int/branch/var.hpp:249 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(a) gecode/int/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G4 | `INT_VAR_ACTION_SIZE_MIN` | `a` | `IntAction` | gecode/int/branch/var.hpp:258 | gecode/int/branch/var.hpp:259 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(a) gecode/int/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G4 | `INT_VAR_ACTION_SIZE_MAX` | `a` | `IntAction` | gecode/int/branch/var.hpp:268 | gecode/int/branch/var.hpp:269 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(a) gecode/int/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G4 | `INT_VAR_CHB_SIZE_MIN` | `c` | `IntCHB` | gecode/int/branch/var.hpp:273 | gecode/int/branch/var.hpp:274 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(c) gecode/int/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G4 | `INT_VAR_CHB_SIZE_MAX` | `c` | `IntCHB` | gecode/int/branch/var.hpp:283 | gecode/int/branch/var.hpp:284 | returns a `IntVarBranch` from its `const&` constructor; chain: IntVarBranch(c) gecode/int/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G4 | `BOOL_VAR_RND` | `r` | `Rnd` | gecode/int/branch/var.hpp:377 | gecode/int/branch/var.hpp:378 | returns a `BoolVarBranch` from its `const&` constructor; chain: BoolVarBranch(r) gecode/int/branch/var.hpp:321 → VarBranch(r) gecode/kernel/branch/var.hpp:143 → copy-constructs `_rnd` at gecode/kernel/branch/var.hpp:143; the only copy |
| G4 | `BOOL_VAR_AFC_MIN` | `a` | `BoolAFC` | gecode/int/branch/var.hpp:407 | gecode/int/branch/var.hpp:408 | returns a `BoolVarBranch` from its `const&` constructor; chain: BoolVarBranch(a) gecode/int/branch/var.hpp:329 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G4 | `BOOL_VAR_AFC_MAX` | `a` | `BoolAFC` | gecode/int/branch/var.hpp:417 | gecode/int/branch/var.hpp:418 | returns a `BoolVarBranch` from its `const&` constructor; chain: BoolVarBranch(a) gecode/int/branch/var.hpp:329 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G4 | `BOOL_VAR_ACTION_MIN` | `a` | `BoolAction` | gecode/int/branch/var.hpp:427 | gecode/int/branch/var.hpp:428 | returns a `BoolVarBranch` from its `const&` constructor; chain: BoolVarBranch(a) gecode/int/branch/var.hpp:333 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G4 | `BOOL_VAR_ACTION_MAX` | `a` | `BoolAction` | gecode/int/branch/var.hpp:437 | gecode/int/branch/var.hpp:438 | returns a `BoolVarBranch` from its `const&` constructor; chain: BoolVarBranch(a) gecode/int/branch/var.hpp:333 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G4 | `BOOL_VAR_CHB_MIN` | `c` | `BoolCHB` | gecode/int/branch/var.hpp:442 | gecode/int/branch/var.hpp:443 | returns a `BoolVarBranch` from its `const&` constructor; chain: BoolVarBranch(c) gecode/int/branch/var.hpp:337 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G4 | `BOOL_VAR_CHB_MAX` | `c` | `BoolCHB` | gecode/int/branch/var.hpp:452 | gecode/int/branch/var.hpp:453 | returns a `BoolVarBranch` from its `const&` constructor; chain: BoolVarBranch(c) gecode/int/branch/var.hpp:337 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G5 | `SET_ASSIGN_RND_INC` | `r` | `Rnd` | gecode/set/branch/assign.hpp:83 | gecode/set/branch/assign.hpp:84 | returns a `SetAssign` from its `const&` constructor; chain: SetAssign(r) gecode/set/branch/assign.hpp:40 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G5 | `SET_ASSIGN_RND_EXC` | `r` | `Rnd` | gecode/set/branch/assign.hpp:88 | gecode/set/branch/assign.hpp:89 | returns a `SetAssign` from its `const&` constructor; chain: SetAssign(r) gecode/set/branch/assign.hpp:40 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G5 | `SET_VAL_RND_INC` | `r` | `Rnd` | gecode/set/branch/val.hpp:83 | gecode/set/branch/val.hpp:84 | returns a `SetValBranch` from its `const&` constructor; chain: SetValBranch(r) gecode/set/branch/val.hpp:40 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G5 | `SET_VAL_RND_EXC` | `r` | `Rnd` | gecode/set/branch/val.hpp:88 | gecode/set/branch/val.hpp:89 | returns a `SetValBranch` from its `const&` constructor; chain: SetValBranch(r) gecode/set/branch/val.hpp:40 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G5 | `SET_VAR_RND` | `r` | `Rnd` | gecode/set/branch/var.hpp:108 | gecode/set/branch/var.hpp:109 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(r) gecode/set/branch/var.hpp:46 → VarBranch(r) gecode/kernel/branch/var.hpp:143 → copy-constructs `_rnd` at gecode/kernel/branch/var.hpp:143; the only copy |
| G5 | `SET_VAR_AFC_MIN` | `a` | `SetAFC` | gecode/set/branch/var.hpp:138 | gecode/set/branch/var.hpp:139 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(a) gecode/set/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G5 | `SET_VAR_AFC_MAX` | `a` | `SetAFC` | gecode/set/branch/var.hpp:148 | gecode/set/branch/var.hpp:149 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(a) gecode/set/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G5 | `SET_VAR_ACTION_MIN` | `a` | `SetAction` | gecode/set/branch/var.hpp:158 | gecode/set/branch/var.hpp:159 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(a) gecode/set/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G5 | `SET_VAR_ACTION_MAX` | `a` | `SetAction` | gecode/set/branch/var.hpp:168 | gecode/set/branch/var.hpp:169 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(a) gecode/set/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G5 | `SET_VAR_CHB_MIN` | `c` | `SetCHB` | gecode/set/branch/var.hpp:178 | gecode/set/branch/var.hpp:179 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(c) gecode/set/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G5 | `SET_VAR_CHB_MAX` | `c` | `SetCHB` | gecode/set/branch/var.hpp:188 | gecode/set/branch/var.hpp:189 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(c) gecode/set/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G5 | `SET_VAR_AFC_SIZE_MIN` | `a` | `SetAFC` | gecode/set/branch/var.hpp:238 | gecode/set/branch/var.hpp:239 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(a) gecode/set/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G5 | `SET_VAR_AFC_SIZE_MAX` | `a` | `SetAFC` | gecode/set/branch/var.hpp:248 | gecode/set/branch/var.hpp:249 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(a) gecode/set/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G5 | `SET_VAR_ACTION_SIZE_MIN` | `a` | `SetAction` | gecode/set/branch/var.hpp:258 | gecode/set/branch/var.hpp:259 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(a) gecode/set/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G5 | `SET_VAR_ACTION_SIZE_MAX` | `a` | `SetAction` | gecode/set/branch/var.hpp:268 | gecode/set/branch/var.hpp:269 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(a) gecode/set/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G5 | `SET_VAR_CHB_SIZE_MIN` | `c` | `SetCHB` | gecode/set/branch/var.hpp:278 | gecode/set/branch/var.hpp:279 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(c) gecode/set/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G5 | `SET_VAR_CHB_SIZE_MAX` | `c` | `SetCHB` | gecode/set/branch/var.hpp:288 | gecode/set/branch/var.hpp:289 | returns a `SetVarBranch` from its `const&` constructor; chain: SetVarBranch(c) gecode/set/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G6 | `FLOAT_ASSIGN_RND` | `r` | `Rnd` | gecode/float/branch/assign.hpp:63 | gecode/float/branch/assign.hpp:64 | returns a `FloatAssign` from its `const&` constructor; chain: FloatAssign(r) gecode/float/branch/assign.hpp:40 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G6 | `FLOAT_VAL_SPLIT_RND` | `r` | `Rnd` | gecode/float/branch/val.hpp:63 | gecode/float/branch/val.hpp:64 | returns a `FloatValBranch` from its `const&` constructor; chain: FloatValBranch(r) gecode/float/branch/val.hpp:40 → ValBranch(r0) gecode/kernel/branch/val.hpp:74 → copy-constructs `r` at gecode/kernel/branch/val.hpp:74; the only copy |
| G6 | `FLOAT_VAR_RND` | `r` | `Rnd` | gecode/float/branch/var.hpp:118 | gecode/float/branch/var.hpp:119 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(r) gecode/float/branch/var.hpp:46 → VarBranch(r) gecode/kernel/branch/var.hpp:143 → copy-constructs `_rnd` at gecode/kernel/branch/var.hpp:143; the only copy |
| G6 | `FLOAT_VAR_AFC_MIN` | `a` | `FloatAFC` | gecode/float/branch/var.hpp:138 | gecode/float/branch/var.hpp:139 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(a) gecode/float/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G6 | `FLOAT_VAR_AFC_MAX` | `a` | `FloatAFC` | gecode/float/branch/var.hpp:148 | gecode/float/branch/var.hpp:149 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(a) gecode/float/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G6 | `FLOAT_VAR_ACTION_MIN` | `a` | `FloatAction` | gecode/float/branch/var.hpp:158 | gecode/float/branch/var.hpp:159 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(a) gecode/float/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G6 | `FLOAT_VAR_ACTION_MAX` | `a` | `FloatAction` | gecode/float/branch/var.hpp:168 | gecode/float/branch/var.hpp:169 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(a) gecode/float/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G6 | `FLOAT_VAR_CHB_MIN` | `c` | `FloatCHB` | gecode/float/branch/var.hpp:178 | gecode/float/branch/var.hpp:179 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(c) gecode/float/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G6 | `FLOAT_VAR_CHB_MAX` | `c` | `FloatCHB` | gecode/float/branch/var.hpp:188 | gecode/float/branch/var.hpp:189 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(c) gecode/float/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G6 | `FLOAT_VAR_AFC_SIZE_MIN` | `a` | `FloatAFC` | gecode/float/branch/var.hpp:238 | gecode/float/branch/var.hpp:239 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(a) gecode/float/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G6 | `FLOAT_VAR_AFC_SIZE_MAX` | `a` | `FloatAFC` | gecode/float/branch/var.hpp:248 | gecode/float/branch/var.hpp:249 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(a) gecode/float/branch/var.hpp:54 → VarBranch(a) gecode/kernel/branch/var.hpp:125 → copy-constructs `_afc` at gecode/kernel/branch/var.hpp:125; the only copy |
| G6 | `FLOAT_VAR_ACTION_SIZE_MIN` | `a` | `FloatAction` | gecode/float/branch/var.hpp:258 | gecode/float/branch/var.hpp:259 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(a) gecode/float/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G6 | `FLOAT_VAR_ACTION_SIZE_MAX` | `a` | `FloatAction` | gecode/float/branch/var.hpp:268 | gecode/float/branch/var.hpp:269 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(a) gecode/float/branch/var.hpp:58 → VarBranch(a) gecode/kernel/branch/var.hpp:131 → copy-constructs `_act` at gecode/kernel/branch/var.hpp:131; the only copy |
| G6 | `FLOAT_VAR_CHB_SIZE_MIN` | `c` | `FloatCHB` | gecode/float/branch/var.hpp:278 | gecode/float/branch/var.hpp:279 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(c) gecode/float/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G6 | `FLOAT_VAR_CHB_SIZE_MAX` | `c` | `FloatCHB` | gecode/float/branch/var.hpp:288 | gecode/float/branch/var.hpp:289 | returns a `FloatVarBranch` from its `const&` constructor; chain: FloatVarBranch(c) gecode/float/branch/var.hpp:62 → VarBranch(c) gecode/kernel/branch/var.hpp:137 → copy-constructs `_chb` at gecode/kernel/branch/var.hpp:137; the only copy |
| G9 | `OpenRegular::OpenRegular<View, pc>` | `dfa0` | `DFA` | gecode/int/extensional/open.hpp:108 | gecode/int/extensional/open.hpp:109 | copy-constructs `dfa`; the only copy |
| G9 | `ValueSequenceSymmetryObject::ValueSequenceSymmetryObject` | `vs` | `IntArgs` | gecode/int/ldsb/sym-obj.cpp:97 | gecode/int/ldsb/sym-obj.cpp:97 | copy-constructs `values`; the only copy |
| G9 | `Matrix::Matrix<type-parameter-0-0>` | `a` | `Gecode::BoolVarArgs / Gecode::IntVarArgs / Gecode::SetVarArgs / Gecode::IntArgs / Gecode::ArgArray<Gecode::IntSet>` | gecode/minimodel/matrix.hpp:128 | gecode/minimodel/matrix.hpp:129 | copy-constructs `_a`; the only copy |
| G12 | `ViewTraceRecorder::ViewTraceRecorder<View>` | `tf0` | `TraceFilter` | gecode/kernel/trace/recorder.hpp:260 | gecode/kernel/trace/recorder.hpp:262 | copy-constructs `tf`; the only copy |
| G12 | `TraceRecorder::TraceRecorder` | `tf0` | `TraceFilter` | gecode/kernel/trace/recorder.hpp:390 | gecode/kernel/trace/recorder.hpp:391 | copy-constructs `tf`; the only copy |
| G13 | `BlackBoxBounds::BlackBoxBounds` | `sub_int0` | `SharedArray<bool>` | gecode/flatzinc/blackbox-propagator.cpp:287 | gecode/flatzinc/blackbox-propagator.cpp:296 | copy-constructs `sub_int`; the only copy |
| G13 | `BlackBoxBounds::BlackBoxBounds` | `sub_float0` | `SharedArray<bool>` | gecode/flatzinc/blackbox-propagator.cpp:289 | gecode/flatzinc/blackbox-propagator.cpp:298 | copy-constructs `sub_float`; the only copy |
| G13 | `IntBoolVarBranch::IntBoolVarBranch` | `i` | `IntAFC` | gecode/flatzinc/branch.hpp:40 | gecode/flatzinc/branch.hpp:41 | copy-constructs `int_afc_data`; the only copy |
| G13 | `IntBoolVarBranch::IntBoolVarBranch` | `b` | `BoolAFC` | gecode/flatzinc/branch.hpp:40 | gecode/flatzinc/branch.hpp:41 | copy-constructs `bool_afc_data`; the only copy |
| G13 | `IntBoolVarBranch::IntBoolVarBranch` | `i` | `IntAction` | gecode/flatzinc/branch.hpp:44 | gecode/flatzinc/branch.hpp:45 | copy-constructs `int_action_data`; the only copy |
| G13 | `IntBoolVarBranch::IntBoolVarBranch` | `b` | `BoolAction` | gecode/flatzinc/branch.hpp:44 | gecode/flatzinc/branch.hpp:45 | copy-constructs `bool_action_data`; the only copy |
| G13 | `IntBoolVarBranch::IntBoolVarBranch` | `i` | `IntCHB` | gecode/flatzinc/branch.hpp:48 | gecode/flatzinc/branch.hpp:49 | copy-constructs `int_chb_data`; the only copy |
| G13 | `IntBoolVarBranch::IntBoolVarBranch` | `b` | `BoolCHB` | gecode/flatzinc/branch.hpp:48 | gecode/flatzinc/branch.hpp:49 | copy-constructs `bool_chb_data`; the only copy |
| G13 | `int_bool_var_afc_max` | `ia` | `IntAFC` | gecode/flatzinc/branch.hpp:115 | gecode/flatzinc/branch.hpp:116 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_afc_max` | `ba` | `BoolAFC` | gecode/flatzinc/branch.hpp:115 | gecode/flatzinc/branch.hpp:116 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_action_max` | `ia` | `IntAction` | gecode/flatzinc/branch.hpp:123 | gecode/flatzinc/branch.hpp:124 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_action_max` | `ba` | `BoolAction` | gecode/flatzinc/branch.hpp:123 | gecode/flatzinc/branch.hpp:124 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_chb_max` | `ic` | `IntCHB` | gecode/flatzinc/branch.hpp:131 | gecode/flatzinc/branch.hpp:132 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_chb_max` | `bc` | `BoolCHB` | gecode/flatzinc/branch.hpp:131 | gecode/flatzinc/branch.hpp:132 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_afc_size_max` | `ia` | `IntAFC` | gecode/flatzinc/branch.hpp:139 | gecode/flatzinc/branch.hpp:140 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_afc_size_max` | `ba` | `BoolAFC` | gecode/flatzinc/branch.hpp:139 | gecode/flatzinc/branch.hpp:140 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_action_size_max` | `ia` | `IntAction` | gecode/flatzinc/branch.hpp:147 | gecode/flatzinc/branch.hpp:148 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_action_size_max` | `ba` | `BoolAction` | gecode/flatzinc/branch.hpp:147 | gecode/flatzinc/branch.hpp:148 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_chb_size_max` | `ic` | `IntCHB` | gecode/flatzinc/branch.hpp:155 | gecode/flatzinc/branch.hpp:156 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `int_bool_var_chb_size_max` | `bc` | `BoolCHB` | gecode/flatzinc/branch.hpp:155 | gecode/flatzinc/branch.hpp:156 | returns a `IntBoolVarBranch` from its `const&` constructor; the only copy |
| G13 | `get_var_ref_arg` | `id` | `string` | gecode/flatzinc/parser.tab.cpp:211 | gecode/flatzinc/parser.tab.cpp:215, :218-221, :227 | generated from `get_var_ref_arg` at gecode/flatzinc/parser.yxx:176. When `annotation` is set, `n = id` (:215) copy-assigns a local, and the returned variable nodes (:218-221) copy-construct `n` into `AST::Var::n` (gecode/flatzinc/ast.hh:204); `new AST::Atom(id)` (:227) copy-constructs `id` into `Atom::id` (gecode/flatzinc/ast.hh:316). Source lines parser.yxx:180, :183-186, :192 |
| G15 | `Pentominoes::tile_reg` | `mark` | `REG` | examples/pentominoes.cpp:300 | examples/pentominoes.cpp:303, :307 | `REG color[] = {other, mark}` copy-constructs a `REG`, which shares `mark`'s expression node and increments its use count (gecode/minimodel/reg.cpp:219-220); `res += color[which]` (:307) links that node into the returned `REG` (reg.cpp:349-350). An ownership-preserving handle copy |
| G15 | `Pentominoes::tile_reg` | `eol` | `REG` | examples/pentominoes.cpp:300 | examples/pentominoes.cpp:301, :310, :313 | `REG oe = other \| eol` (:301) makes an OR node whose second kid is `eol`'s expression, with its use count incremented (gecode/minimodel/reg.cpp:297-298); `res += oe(...)` and `res += *oe + oe` (:310, :313) link `oe` into the returned `REG`. An ownership-preserving handle copy |
| G15 | `TupleSetTest::TupleSetTest` | `ts0` | `Gecode::TupleSet` | test/int/extensional.cpp:1194 | test/int/extensional.cpp:1198 | copy-constructs `ts`; the only copy |
| G16 | `Lin::Lin<P, N, pc>` | `c0` | `FloatVal` | gecode/float/linear/nary.hpp:43 | gecode/float/linear/nary.hpp:44 | copy-constructs `c`; the only copy |
| G16 | `ReEqFloat::ReEqFloat<View, CtrlView, rm>` | `c0` | `FloatVal` | gecode/float/rel/eq.hpp:272 | gecode/float/rel/eq.hpp:273 | copy-constructs `c`; the only copy |
| G16 | `ReLqFloat::ReLqFloat<View, CtrlView, rm>` | `c0` | `FloatVal` | gecode/float/rel/lq-le.hpp:226 | gecode/float/rel/lq-le.hpp:227 | copy-constructs `c`; the only copy |
| G16 | `ReLeFloat::ReLeFloat<View, CtrlView, rm>` | `c0` | `FloatVal` | gecode/float/rel/lq-le.hpp:313 | gecode/float/rel/lq-le.hpp:314 | copy-constructs `c`; the only copy |
| G16 | `NqFloat::NqFloat<View>` | `_c` | `FloatVal` | gecode/float/rel/nq.hpp:86 | gecode/float/rel/nq.hpp:87 | copy-constructs `c`; the only copy |
| G16 | `ScaleView::ScaleView` | `d` | `FloatVal` | gecode/float/view/scale.hpp:44 | gecode/float/view/scale.hpp:45 | copy-constructs `a`; the only copy |
| G16 | `DomArgs::DomArgs<Args...>` | `args` | `Gecode::FloatVal` | gecode/minimodel/dom.cpp:104 | gecode/minimodel/dom.cpp:104 | copy-constructs `_args`; the only copy |
| G16 | `LinFloatExpr::LinFloatExpr` | `a` | `FloatVal` | gecode/minimodel/float-expr.cpp:257 | gecode/minimodel/float-expr.cpp:261 | copy-assigns `n->a`; the only copy |
| G16 | `LinFloatExpr::LinFloatExpr` | `a` | `FloatVal` | gecode/minimodel/float-expr.cpp:311 | gecode/minimodel/float-expr.cpp:317 | copy-assigns `n->a`; the only copy |
| G16 | `LinFloatExpr::Node::fill` | `m` | `FloatVal` | gecode/minimodel/float-expr.cpp:345 | gecode/minimodel/float-expr.cpp:358 | copy-assigns `tf->a = m` on the NT_NONLIN path, the only direct copy; the NT_CONST and NT_VAR paths store products (`d += m * c` at :349, `tf->a = m * a` at :353), and recursive calls pass `m` or a new product by `const&` |
| G16 | `LinFloatRel::LinFloatRel` | `l` | `FloatVal` | gecode/minimodel/float-rel.hpp:53 | gecode/minimodel/float-rel.hpp:54; gecode/minimodel/float-expr.cpp:459-460, :308 | `e(l - r)` calls `operator-(const FloatVal&, const LinFloatExpr&)`, which makes an `NT_SUB` node whose `n->c = c` copy-assigns `l` itself; the only copy |
| G16 | `Val::Val` | `c0` | `Gecode::FloatVal` | test/float/dom.cpp:57 | test/float/dom.cpp:58 | copy-constructs `c`; the only copy |

### A2. Forwarded to a base or delegated constructor, which keeps the copy

| Group | Function | Parameter | Type | HEAD parameter | Receiving constructor |
| --- | --- | --- | --- | --- | --- |
| G4 | `IntAssign::IntAssign` | `r` | `Rnd` | gecode/int/branch/assign.hpp:40 | ValBranch<IntVar> (copies from its `const&`) |
| G4 | `BoolAssign::BoolAssign` | `r` | `Rnd` | gecode/int/branch/assign.hpp:82 | ValBranch<BoolVar> (copies from its `const&`) |
| G4 | `BoolValBranch::BoolValBranch` | `r` | `Rnd` | gecode/int/branch/val.hpp:112 | ValBranch<BoolVar> (copies from its `const&`) |
| G4 | `IntValBranch::IntValBranch` | `r` | `Rnd` | gecode/int/branch/val.hpp:40 | ValBranch<IntVar> (copies from its `const&`) |
| G4 | `BoolVarBranch::BoolVarBranch` | `r` | `Rnd` | gecode/int/branch/var.hpp:321 | VarBranch<BoolVar> (copies from its `const&`) |
| G4 | `BoolVarBranch::BoolVarBranch` | `a` | `BoolAFC` | gecode/int/branch/var.hpp:329 | VarBranch<BoolVar> (copies from its `const&`) |
| G4 | `BoolVarBranch::BoolVarBranch` | `a` | `BoolAction` | gecode/int/branch/var.hpp:333 | VarBranch<BoolVar> (copies from its `const&`) |
| G4 | `BoolVarBranch::BoolVarBranch` | `c` | `BoolCHB` | gecode/int/branch/var.hpp:337 | VarBranch<BoolVar> (copies from its `const&`) |
| G4 | `IntVarBranch::IntVarBranch` | `r` | `Rnd` | gecode/int/branch/var.hpp:46 | VarBranch<IntVar> (copies from its `const&`) |
| G4 | `IntVarBranch::IntVarBranch` | `a` | `IntAFC` | gecode/int/branch/var.hpp:54 | VarBranch<IntVar> (copies from its `const&`) |
| G4 | `IntVarBranch::IntVarBranch` | `a` | `IntAction` | gecode/int/branch/var.hpp:58 | VarBranch<IntVar> (copies from its `const&`) |
| G4 | `IntVarBranch::IntVarBranch` | `c` | `IntCHB` | gecode/int/branch/var.hpp:62 | VarBranch<IntVar> (copies from its `const&`) |
| G5 | `SetAssign::SetAssign` | `r` | `Rnd` | gecode/set/branch/assign.hpp:40 | ValBranch<SetVar> (copies from its `const&`) |
| G5 | `SetValBranch::SetValBranch` | `r` | `Rnd` | gecode/set/branch/val.hpp:40 | ValBranch<SetVar> (copies from its `const&`) |
| G5 | `SetVarBranch::SetVarBranch` | `r` | `Rnd` | gecode/set/branch/var.hpp:46 | VarBranch<SetVar> (copies from its `const&`) |
| G5 | `SetVarBranch::SetVarBranch` | `a` | `SetAFC` | gecode/set/branch/var.hpp:54 | VarBranch<SetVar> (copies from its `const&`) |
| G5 | `SetVarBranch::SetVarBranch` | `a` | `SetAction` | gecode/set/branch/var.hpp:58 | VarBranch<SetVar> (copies from its `const&`) |
| G5 | `SetVarBranch::SetVarBranch` | `c` | `SetCHB` | gecode/set/branch/var.hpp:62 | VarBranch<SetVar> (copies from its `const&`) |
| G6 | `FloatAssign::FloatAssign` | `r` | `Rnd` | gecode/float/branch/assign.hpp:40 | ValBranch<FloatVar> (copies from its `const&`) |
| G6 | `FloatValBranch::FloatValBranch` | `r` | `Rnd` | gecode/float/branch/val.hpp:40 | ValBranch<FloatVar> (copies from its `const&`) |
| G6 | `FloatVarBranch::FloatVarBranch` | `r` | `Rnd` | gecode/float/branch/var.hpp:46 | VarBranch<FloatVar> (copies from its `const&`) |
| G6 | `FloatVarBranch::FloatVarBranch` | `a` | `FloatAFC` | gecode/float/branch/var.hpp:54 | VarBranch<FloatVar> (copies from its `const&`) |
| G6 | `FloatVarBranch::FloatVarBranch` | `a` | `FloatAction` | gecode/float/branch/var.hpp:58 | VarBranch<FloatVar> (copies from its `const&`) |
| G6 | `FloatVarBranch::FloatVarBranch` | `c` | `FloatCHB` | gecode/float/branch/var.hpp:62 | VarBranch<FloatVar> (copies from its `const&`) |
| G9 | `EqInt::EqInt<VX, VY>` | `y` | `Gecode::IntSet` | gecode/int/count/int-eq.hpp:37 | IntBase<VX, VY> (copies from its `const&`) |
| G9 | `GqInt::GqInt<VX, VY>` | `y` | `Gecode::IntSet` | gecode/int/count/int-gq.hpp:37 | IntBase<VX, VY> (copies from its `const&`) |
| G9 | `LqInt::LqInt<VX, VY>` | `y` | `Gecode::IntSet` | gecode/int/count/int-lq.hpp:37 | IntBase<VX, VY> (copies from its `const&`) |
| G9 | `EqView::EqView<VX, VY, VZ, shr, dom>` | `y` | `Gecode::IntSet` | gecode/int/count/view-eq.hpp:37 | ViewBase<VX, VY, VZ> (copies from its `const&`) |
| G9 | `GqView::GqView<VX, VY, VZ, shr, dom>` | `y` | `Gecode::IntSet` | gecode/int/count/view-gq.hpp:37 | ViewBase<VX, VY, VZ> (copies from its `const&`) |
| G9 | `LqView::LqView<VX, VY, VZ, shr>` | `y` | `Gecode::IntSet` | gecode/int/count/view-lq.hpp:37 | ViewBase<VX, VY, VZ> (copies from its `const&`) |
| G14 | `VarComparator::VarComparator<S>` | `name` | `std::string` | gecode/gist/gist.hpp:36 | TextOutput (copies from its `const&`) |
| G15 | `TupleSetTest::TupleSetTest` | `d0` | `Gecode::IntSet` | test/int/extensional.cpp:1194 | Test (copies from its `const&`) |
| G15 | `RandomTupleSetTest::RandomTupleSetTest` | `d0` | `Gecode::IntSet` | test/int/extensional.cpp:1236 | TupleSetTest (copies from its `const&`) |
| G15 | `RandomTupleSetTest::RandomTupleSetTest` | `ts0` | `Gecode::TupleSet` | test/int/extensional.cpp:1236 | TupleSetTest (copies from its `const&`) |
| G16 | `Eq::Eq<P, N>` | `c` | `FloatVal` | gecode/float/linear/nary.hpp:122 | Lin<P, N, PC_FLOAT_BND> (copies from its `const&`) |
| G16 | `Lq::Lq<P, N>` | `c` | `FloatVal` | gecode/float/linear/nary.hpp:254 | Lin<P, N, PC_FLOAT_BND> (copies from its `const&`) |
| G16 | `Basic::Basic` | `v` | `Gecode::FloatVal` | test/float/basic.cpp:56 | Test (copies from its `const&`) |

### A3. Forwarded, through any chain of calls, to a function that keeps a copy

The kept copy is of the parameter, of one of its members or elements, or a handle sharing its representation. "Forwarded to" names the first hop and its HEAD call line; by-value hops are marked.

| Group | Function | Parameter | Type | HEAD parameter | Forwarded to | Copy site | How the copy is made |
| --- | --- | --- | --- | --- | --- | --- | --- |
| G7 | `branch` | `vals` | `IntValBranch` | gecode/int/branch.cpp:38 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :54 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `IntValBranch` | gecode/int/branch.cpp:61 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :77, :91, :105, :119 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `IntValBranch` | gecode/int/branch.cpp:126 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :129 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `assign` | `vals` | `IntAssign` | gecode/int/branch.cpp:133 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :140 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `IntAssign` | gecode/int/branch.cpp:145 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :161, :166, :171, :176 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `assign` | `ia` | `IntAssign` | gecode/int/branch.cpp:183 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :186 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `BoolValBranch` | gecode/int/branch.cpp:190 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :198 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `BoolValBranch` | gecode/int/branch.cpp:203 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :219, :222 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `BoolValBranch` | gecode/int/branch.cpp:239 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :242 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `assign` | `vals` | `BoolAssign` | gecode/int/branch.cpp:246 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :253 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `assign` | `vals` | `BoolAssign` | gecode/int/branch.cpp:258 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :274, :277 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `assign` | `ba` | `BoolAssign` | gecode/int/branch.cpp:294 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :297 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `IntValBranch` | gecode/int/branch.cpp:311 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :313 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vars` | `TieBreak<IntVarBranch>` | gecode/int/branch.cpp:317 | Int::Branch::post_open_brancher at :320 (gecode/int/branch/open.hpp:320) | gecode/int/branch/open.hpp:322; gecode/int/branch/view-sel.cpp:38/:175; gecode/kernel/branch/merit.hpp:201, :246, :273, :298; gecode/kernel/branch/view-sel.hpp:457, :689; gecode/int/branch/merit.hpp:99, :125, :150 | `VarBranch selectors[4] = {vars.a, vars.b, vars.c, vars.d}` copy-constructs the four configurations (their Rnd/AFC/Action/CHB handles and BranchTbl/MeritFunction std::functions). `OpenListBrancher::post` (gecode/int/branch/open.hpp:247) passes each to `view_selection`, whose selectors copy `vb.rnd()`, `vb.tbl()`, `vb.merit()`, `vb.afc()`, `vb.action()` or `vb.chb()` through by-value accessors (gecode/kernel/branch/var.hpp:154-212) into their members, which the brancher keeps |
| G7 | `branch` | `vals` | `IntValBranch` | gecode/int/branch.cpp:317 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :320 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `BoolValBranch` | gecode/int/branch.cpp:325 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :327 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vars` | `TieBreak<BoolVarBranch>` | gecode/int/branch.cpp:331 | Int::Branch::post_open_brancher at :334 (gecode/int/branch/open.hpp:320) | gecode/int/branch/open.hpp:322; gecode/int/branch/view-sel.cpp:38/:175; gecode/kernel/branch/merit.hpp:201, :246, :273, :298; gecode/kernel/branch/view-sel.hpp:457, :689; gecode/int/branch/merit.hpp:99, :125, :150 | `VarBranch selectors[4] = {vars.a, vars.b, vars.c, vars.d}` copy-constructs the four configurations (their Rnd/AFC/Action/CHB handles and BranchTbl/MeritFunction std::functions). `OpenListBrancher::post` (gecode/int/branch/open.hpp:247) passes each to `view_selection`, whose selectors copy `vb.rnd()`, `vb.tbl()`, `vb.merit()`, `vb.afc()`, `vb.action()` or `vb.chb()` through by-value accessors (gecode/kernel/branch/var.hpp:154-212) into their members, which the brancher keeps |
| G7 | `branch` | `vals` | `BoolValBranch` | gecode/int/branch.cpp:331 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :334 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `IntValBranch` | gecode/int/branch.hpp:36 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :37 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `BoolValBranch` | gecode/int/branch.hpp:40 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :41 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `assign` | `vals` | `IntAssign` | gecode/int/branch.hpp:45 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :46 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `assign` | `vals` | `BoolAssign` | gecode/int/branch.hpp:49 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :50 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `open_valselcommit` | `vals` | `IntValBranch` | gecode/int/branch/open.hpp:290 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :294 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `open_valselcommit` | `vals` | `BoolValBranch` | gecode/int/branch/open.hpp:299 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :301 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `post_open_brancher` | `vars` | `TieBreak<Gecode::BoolVarBranch> / TieBreak<Gecode::IntVarBranch>` | gecode/int/branch/open.hpp:320 | local copies at :322, then post_open_brancher<View, n> → OpenListBrancher::post | gecode/int/branch/open.hpp:322; gecode/int/branch/view-sel.cpp:38/:175; gecode/kernel/branch/merit.hpp:201, :246, :273, :298; gecode/kernel/branch/view-sel.hpp:457, :689; gecode/int/branch/merit.hpp:99, :125, :150 | `VarBranch selectors[4] = {vars.a, vars.b, vars.c, vars.d}` copy-constructs the four configurations (their Rnd/AFC/Action/CHB handles and BranchTbl/MeritFunction std::functions). `OpenListBrancher::post` (gecode/int/branch/open.hpp:247) passes each to `view_selection`, whose selectors copy `vb.rnd()`, `vb.tbl()`, `vb.merit()`, `vb.afc()`, `vb.action()` or `vb.chb()` through by-value accessors (gecode/kernel/branch/var.hpp:154-212) into their members, which the brancher keeps |
| G7 | `post_open_brancher` | `vals` | `Gecode::BoolValBranch / Gecode::IntValBranch` | gecode/int/branch/open.hpp:320 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :336 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `IntValBranch` | gecode/int/ldsb.cpp:289 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :324 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `IntValBranch` | gecode/int/ldsb.cpp:330 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :346, :378, :400, :422 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `BoolValBranch` | gecode/int/ldsb.cpp:430 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :462 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G7 | `branch` | `vals` | `BoolValBranch` | gecode/int/ldsb.cpp:468 | Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :484, :513 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `FloatValBranch` | gecode/float/branch.cpp:38 | Float::Branch::value_selection_commit (gecode/float/branch/val-sel-commit.cpp:38/:58); calls at :47 | gecode/float/branch/val-sel.hpp:81; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `FloatValBranch` | gecode/float/branch.cpp:51 | Float::Branch::value_selection_commit (gecode/float/branch/val-sel-commit.cpp:38/:58); calls at :67, :70 | gecode/float/branch/val-sel.hpp:81; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `FloatValBranch` | gecode/float/branch.cpp:92 | the `const&` `branch`/`assign` overload → Float::Branch::value_selection_commit (gecode/float/branch/val-sel-commit.cpp:38/:58); calls at :95 | gecode/float/branch/val-sel.hpp:81; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `assign` | `vals` | `FloatAssign` | gecode/float/branch.cpp:99 | Float::Branch::value_selection_commit (gecode/float/branch/val-sel-commit.cpp:38/:58); calls at :107 | gecode/float/branch/val-sel.hpp:81; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `assign` | `vals` | `FloatAssign` | gecode/float/branch.cpp:111 | Float::Branch::value_selection_commit (gecode/float/branch/val-sel-commit.cpp:38/:58); calls at :127, :130 | gecode/float/branch/val-sel.hpp:81; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `assign` | `vals` | `FloatAssign` | gecode/float/branch.cpp:152 | the `const&` `branch`/`assign` overload → Float::Branch::value_selection_commit (gecode/float/branch/val-sel-commit.cpp:38/:58); calls at :155 | gecode/float/branch/val-sel.hpp:81; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `FloatValBranch` | gecode/float/branch.hpp:36 | the `const&` `branch`/`assign` overload → Float::Branch::value_selection_commit (gecode/float/branch/val-sel-commit.cpp:38/:58); calls at :37 | gecode/float/branch/val-sel.hpp:81; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `assign` | `vals` | `FloatAssign` | gecode/float/branch.hpp:41 | the `const&` `branch`/`assign` overload → Float::Branch::value_selection_commit (gecode/float/branch/val-sel-commit.cpp:38/:58); calls at :42 | gecode/float/branch/val-sel.hpp:81; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `SetValBranch` | gecode/set/branch.cpp:38 | Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :46 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `SetValBranch` | gecode/set/branch.cpp:51 | Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :67, :70 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `SetValBranch` | gecode/set/branch.cpp:87 | the `const&` `branch`/`assign` overload → Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :90 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `assign` | `vals` | `SetAssign` | gecode/set/branch.cpp:94 | Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :101 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `assign` | `vals` | `SetAssign` | gecode/set/branch.cpp:106 | Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :122, :125 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `assign` | `vars` | `SetAssign` | gecode/set/branch.cpp:142 | the `const&` `branch`/`assign` overload → Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :145 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `SetValBranch` | gecode/set/branch.hpp:36 | the `const&` `branch`/`assign` overload → Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :37 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `assign` | `vals` | `SetAssign` | gecode/set/branch.hpp:41 | the `const&` `branch`/`assign` overload → Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :42 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `SetValBranch` | gecode/set/ldsb.cpp:152 | Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :172 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G8 | `branch` | `vals` | `SetValBranch` | gecode/set/ldsb.cpp:177 | Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :193, :207 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G9 | `EqInt::post` | `y` | `Gecode::IntSet` | gecode/int/count/int-eq.hpp:42 | holds, post_false, post_true, EqInt<VX, VY> | gecode/int/count/int-base.hpp:38 | the `EqInt` constructor passes `y` to `IntBase`, whose `y(y0)` copy-constructs the member |
| G9 | `GqInt::post` | `y` | `Gecode::IntSet` | gecode/int/count/int-gq.hpp:42 | holds, post_true, GqInt<VX, VY> | gecode/int/count/int-base.hpp:38 | as `EqInt::post`, through `GqInt` |
| G9 | `LqInt::post` | `y` | `Gecode::IntSet` | gecode/int/count/int-lq.hpp:42 | holds, post_false, LqInt<VX, VY> | gecode/int/count/int-base.hpp:38 | as `EqInt::post`, through `LqInt` |
| G9 | `EqView::post` | `y` | `Gecode::IntSet` | gecode/int/count/view-eq.hpp:42 | isval, EqInt<VX, VY>::post, sharing, EqView<VX, VY, VZ, true, dom>, EqView<VX, VY, VZ, false, dom> | gecode/int/count/view-base.hpp:38; gecode/int/count/int-base.hpp:38 | the `EqView` constructor passes `y` to `ViewBase`, whose `y(y0)` copy-constructs the member; the `EqInt::post` path copies at int-base.hpp:38 |
| G9 | `GqView::post` | `y` | `Gecode::IntSet` | gecode/int/count/view-gq.hpp:42 | isval, GqInt<VX, VY>::post, sharing, GqView<VX, VY, VZ, true, dom>, GqView<VX, VY, VZ, false, dom> | gecode/int/count/view-base.hpp:38; gecode/int/count/int-base.hpp:38 | as `EqView::post`, through `GqView` or `GqInt::post` |
| G9 | `LqView::post` | `y` | `Gecode::IntSet` | gecode/int/count/view-lq.hpp:42 | LqInt<VX, VY>::post, sharing, LqView<VX, VY, VZ, true>, LqView<VX, VY, VZ, false> | gecode/int/count/view-base.hpp:38; gecode/int/count/int-base.hpp:38 | as `EqView::post`, through `LqView` or `LqInt::post` |
| G9 | `element` | `a` | `IntSharedArray` | gecode/int/element.cpp:282 | `element(Home, IntSharedArray c, IntVar x0, IntVar x1, IntPropLevel)` at gecode/int/element.cpp:38, called at :287 | gecode/int/element/int.hpp:149 | the by-value parameter `IntSharedArray c` of the callee copy-constructs from the `const&` (a reference count increment); `post_int` (gecode/int/element/int.hpp:400) and `Int::post` (:168) pass it by reference to `new Int(home, c, x0, x1)` (:174), whose `c(c0)` copy-constructs the member (gecode/int/element.hh:163). The propagator is made only when neither view is assigned |
| G9 | `element` | `a` | `IntSharedArray` | gecode/int/element.cpp:291 | `element(Home, IntSharedArray c, IntVar x0, IntVar x1, IntPropLevel)` at gecode/int/element.cpp:38, called at :297 | gecode/int/element/int.hpp:149 | the by-value parameter `IntSharedArray c` of the callee copy-constructs from the `const&` (a reference count increment); `post_int` (gecode/int/element/int.hpp:400) and `Int::post` (:168) pass it by reference to `new Int(home, c, x0, x1)` (:174), whose `c(c0)` copy-constructs the member (gecode/int/element.hh:163). The propagator is made only when neither view is assigned |
| G9 | `element` | `a` | `IntSharedArray` | gecode/int/element.cpp:301 | `element(Home, IntSharedArray c, IntVar x0, BoolVar x1, IntPropLevel)` at gecode/int/element.cpp:65, called at :306 | gecode/int/element/int.hpp:149 | the by-value parameter `IntSharedArray c` of the callee copy-constructs from the `const&` (a reference count increment); `post_int` (gecode/int/element/int.hpp:400) and `Int::post` (:168) pass it by reference to `new Int(home, c, x0, x1)` (:174), whose `c(c0)` copy-constructs the member (gecode/int/element.hh:163). The propagator is made only when neither view is assigned |
| G9 | `element` | `a` | `IntSharedArray` | gecode/int/element.cpp:310 | `element(Home, IntSharedArray c, IntVar x0, BoolVar x1, IntPropLevel)` at gecode/int/element.cpp:65, called at :316 | gecode/int/element/int.hpp:149 | the by-value parameter `IntSharedArray c` of the callee copy-constructs from the `const&` (a reference count increment); `post_int` (gecode/int/element/int.hpp:400) and `Int::post` (:168) pass it by reference to `new Int(home, c, x0, x1)` (:174), whose `c(c0)` copy-constructs the member (gecode/int/element.hh:163). The propagator is made only when neither view is assigned |
| G9 | `extensional` | `dfa` | `DFA` | gecode/int/extensional-regular.cpp:56 | Int::Extensional::OpenRegular<Int::IntView, Int::PC_INT_DOM>::post | gecode/int/extensional/open.hpp:109 | `OpenRegular::post` makes `new OpenRegular`, whose `dfa(dfa0)` copy-constructs the member |
| G9 | `extensional` | `dfa` | `DFA` | gecode/int/extensional-regular.cpp:63 | Int::Extensional::OpenRegular<Int::BoolView, Int::PC_BOOL_VAL>::post | gecode/int/extensional/open.hpp:109 | as above, for Bool views |
| G9 | `OpenRegular::post` | `dfa` | `DFA` | gecode/int/extensional/open.hpp:296 | copies into local symbols; OpenRegular | gecode/int/extensional/open.hpp:109 | `new OpenRegular(...)` copy-constructs `dfa(dfa0)`; the local `symbols` only iterates it |
| G9 | `Sequence::post` | `s` | `Gecode::IntSet` | gecode/int/sequence/int.hpp:129 | check, Sequence<View, Val>, p->vvsamax.propagate, p->vvsamin.propagate | gecode/int/sequence/int.hpp:42 | `new Sequence(...)` copy-constructs `s(s0)`; `vvsamax`/`vvsamin` take `const Val&` and only read it |
| G9 | `weights` | `elements` | `IntSharedArray` | gecode/set/int.cpp:260 | Set::Int::Weights<Set::SetView>::post at :262 (gecode/set/int/weights.hpp:159) | gecode/set/int/weights.hpp:143 | `Weights::post` makes `new Weights(home, elements, weights, x, y)` (gecode/set/int/weights.hpp:176), whose `elements(elements0)` copy-constructs the `SharedArray<int>` member (a reference count increment) |
| G9 | `weights` | `weights` | `IntSharedArray` | gecode/set/int.cpp:260 | Set::Int::Weights<Set::SetView>::post at :262 (gecode/set/int/weights.hpp:159) | gecode/set/int/weights.hpp:143 | `Weights::post` makes `new Weights(home, elements, weights, x, y)` (gecode/set/int/weights.hpp:176), whose `weights(weights0)` copy-constructs the `SharedArray<int>` member (a reference count increment) |
| G12 | `trace` | `tf` | `TraceFilter` | gecode/float/trace.cpp:38 | FloatTraceRecorder::post at :42 (`ViewTraceRecorder<View>::post`) | gecode/kernel/trace/recorder.hpp:262 | `ViewTraceRecorder::post` (gecode/kernel/trace/recorder.hpp:281) makes `new ViewTraceRecorder`, whose `tf(tf0)` copy-constructs the `TraceFilter` member |
| G12 | `trace` | `tf` | `TraceFilter` | gecode/int/trace.cpp:38 | IntTraceRecorder::post at :42 (`ViewTraceRecorder<View>::post`) | gecode/kernel/trace/recorder.hpp:262 | `ViewTraceRecorder::post` (gecode/kernel/trace/recorder.hpp:281) makes `new ViewTraceRecorder`, whose `tf(tf0)` copy-constructs the `TraceFilter` member |
| G12 | `trace` | `tf` | `TraceFilter` | gecode/int/trace.cpp:46 | BoolTraceRecorder::post at :50 (`ViewTraceRecorder<View>::post`) | gecode/kernel/trace/recorder.hpp:262 | `ViewTraceRecorder::post` (gecode/kernel/trace/recorder.hpp:281) makes `new ViewTraceRecorder`, whose `tf(tf0)` copy-constructs the `TraceFilter` member |
| G12 | `trace` | `tf` | `TraceFilter` | gecode/kernel/trace/general.cpp:38 | TraceRecorder::post at :42 | gecode/kernel/trace/recorder.hpp:391 | `TraceRecorder::post` (gecode/kernel/trace/recorder.hpp:397) makes `new TraceRecorder`, whose `tf(tf0)` copy-constructs the `TraceFilter` member |
| G12 | `ViewTraceRecorder::post` | `tf` | `TraceFilter` | gecode/kernel/trace/recorder.hpp:281 | ViewTraceRecorder | gecode/kernel/trace/recorder.hpp:262 | `new ViewTraceRecorder(...)` copy-constructs `tf(tf0)` |
| G12 | `TraceRecorder::post` | `tf` | `TraceFilter` | gecode/kernel/trace/recorder.hpp:397 | TraceRecorder | gecode/kernel/trace/recorder.hpp:391 | `new TraceRecorder(...)` copy-constructs `tf(tf0)` |
| G12 | `trace` | `tf` | `TraceFilter` | gecode/set/trace.cpp:38 | SetTraceRecorder::post at :42 (`ViewTraceRecorder<View>::post`) | gecode/kernel/trace/recorder.hpp:262 | `ViewTraceRecorder::post` (gecode/kernel/trace/recorder.hpp:281) makes `new ViewTraceRecorder`, whose `tf(tf0)` copy-constructs the `TraceFilter` member |
| G13 | `branch` | `vals` | `IntValBranch` | gecode/flatzinc/branch.cpp:112 | Int::Branch::value_selection_commit at :118 and :119 (through `i2b`, gecode/flatzinc/branch.hpp:395) | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116; gecode/flatzinc/branch.hpp:407 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members. `i2b` returns `BOOL_VAL_RND(ivb.rnd())`, a BoolValBranch holding a copy of the `Rnd` |
| G13 | `annotation_to_int_var_branch` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:405 | INT_VAR_RND | gecode/kernel/branch/var.hpp:143 | the factory returns a `*VarBranch` whose `VarBranch(const Rnd&)` copy-constructs `_rnd` |
| G13 | `annotation_to_int_val_branch` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:454 | INT_VAL_RND | gecode/kernel/branch/val.hpp:74 | the factory returns a value configuration whose `ValBranch(const Rnd&)` copy-constructs `r` |
| G13 | `annotation_to_int_assign` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:515 | INT_ASSIGN_RND | gecode/kernel/branch/val.hpp:74 | the factory returns a value configuration whose `ValBranch(const Rnd&)` copy-constructs `r` |
| G13 | `annotation_to_bool_var_branch` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:534 | BOOL_VAR_RND | gecode/kernel/branch/var.hpp:143 | the factory returns a `*VarBranch` whose `VarBranch(const Rnd&)` copy-constructs `_rnd` |
| G13 | `annotation_to_bool_val_branch` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:559 | BOOL_VAL_RND | gecode/kernel/branch/val.hpp:74 | the factory returns a value configuration whose `ValBranch(const Rnd&)` copy-constructs `r` |
| G13 | `annotation_to_bool_assign` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:620 | BOOL_ASSIGN_RND | gecode/kernel/branch/val.hpp:74 | the factory returns a value configuration whose `ValBranch(const Rnd&)` copy-constructs `r` |
| G13 | `annotation_to_set_var_branch` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:638 | SET_VAR_RND | gecode/kernel/branch/var.hpp:143 | the factory returns a `*VarBranch` whose `VarBranch(const Rnd&)` copy-constructs `_rnd` |
| G13 | `annotation_to_float_var_branch` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:712 | FLOAT_VAR_RND | gecode/kernel/branch/var.hpp:143 | the factory returns a `*VarBranch` whose `VarBranch(const Rnd&)` copy-constructs `_rnd` |
| G13 | `add_domain_constraint` | `id` | `std::string` | gecode/flatzinc/parser.tab.cpp:235 | new ConExpr | gecode/flatzinc/conexpr.hh:62 | generated from `add_domain_constraint` at gecode/flatzinc/parser.yxx:200; `new ConExpr(id, ...)` copy-constructs `id(id0)` (ConExpr's constructor was already `const&` at 02c2ff74f3) |
| G15 | `JobShopProbe::branch` | `r` | `Rnd` | examples/job-shop.cpp:459 | INT_VAR_RND | gecode/kernel/branch/var.hpp:143 | the factory returns a `*VarBranch` whose `VarBranch(const Rnd&)` copy-constructs `_rnd` |
| G15 | `Pentominoes::get_constraint` | `mark` | `REG` | examples/pentominoes.cpp:320 | tile_reg at :329 (A1 row for its `mark`) | examples/pentominoes.cpp:303, :307 | as in `tile_reg` (A1); `res = res \| tile_reg(...)` (:329) links the returned `REG` into `res` (gecode/minimodel/reg.cpp:288-299), which is returned. An ownership-preserving handle copy |
| G15 | `Pentominoes::get_constraint` | `other` | `REG` | examples/pentominoes.cpp:320 | tile_reg at :329, by value | examples/pentominoes.cpp:301, :303 | the by-value hop copy-constructs `tile_reg`'s `REG other` from the `const&` (use count increment, gecode/minimodel/reg.cpp:219-220); `other \| eol` and `REG color[] = {other, mark}` link it into the returned `REG`, which `res = res \| tile_reg(...)` keeps. An ownership-preserving handle copy |
| G15 | `Pentominoes::get_constraint` | `eol` | `REG` | examples/pentominoes.cpp:320 | tile_reg at :329 (A1 row for its `eol`) | examples/pentominoes.cpp:301, :310, :313 | as in `tile_reg` (A1); `res = res \| tile_reg(...)` (:329) links the returned `REG` into `res` (gecode/minimodel/reg.cpp:288-299), which is returned. An ownership-preserving handle copy |
| G15 | `FilteredRndTiesSpace::FilteredRndTiesSpace` | `r` | `Gecode::Rnd` | test/branch/int.cpp:49 | Gecode::INT_VAR_RND | gecode/kernel/branch/var.hpp:143 | the factory returns a `*VarBranch` whose `VarBranch(const Rnd&)` copy-constructs `_rnd` |
| G15 | `OpenRegularSpace::OpenRegularSpace` | `d` | `Gecode::DFA` | test/int/extensional.cpp:836 | Gecode::extensional at :837 (open list, gecode/int/extensional-regular.cpp:56, A3) | gecode/int/extensional/open.hpp:109 | `OpenRegular::post` (gecode/int/extensional/open.hpp:296) makes `new OpenRegular`, whose `dfa(dfa0)` copy-constructs the `DFA` member (a reference count increment) |
| G15 | `SelectorSpace::SelectorSpace` | `vals` | `Gecode::IntValBranch` | test/int/open-var-list.cpp:167 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :172 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G15 | `SelectorChoiceSpace::SelectorChoiceSpace` | `vals` | `Gecode::IntValBranch` | test/int/open-var-list.cpp:274 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :279 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G15 | `EmptyInt::post_branching` | `vals` | `IntValBranch` | test/ldsb.cpp:1784 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :1790, :1792 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G15 | `Queens::Queens` | `ivb` | `IntValBranch` | test/nogoods.cpp:61 | the `const&` `branch`/`assign` overload → Int::Branch::value_selection_commit (gecode/int/branch/val-sel-commit.cpp:38/:69/:91/:111); calls at :75, :78 | gecode/int/branch/val-sel.hpp:94; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G15 | `Hamming::Hamming` | `svb` | `SetValBranch` | test/nogoods.cpp:150 | the `const&` `branch`/`assign` overload → Set::Branch::value_selection_commit (gecode/set/branch/val-sel-commit.cpp:38/:68); calls at :169, :172 | gecode/set/branch/val-sel.hpp:93; gecode/kernel/branch/val-sel.hpp:112; gecode/kernel/branch/val-commit.hpp:116 | `ValSelCommit(home, vb)` (gecode/kernel/branch/val-sel-commit.hpp:142) builds the selectors from the `const&`. For a RND selection `r(vb.rnd())` copy-initializes the `Rnd` member from the accessor's returned copy; for VAL_COMMIT `v(vb.val())` and `c(vb.commit())` move the returned std::function copies into `SharedData` (gecode/kernel/data/shared-data.hpp:89). Other selections copy nothing. gecode/kernel/branch/val.hpp:85 (`rnd()`), :91 (`val()`), :97 (`commit()`) return copies of the members |
| G16 | `dom` | `n` | `FloatVal` | gecode/float/dom.cpp:82 | Limits::check, Rel::ReEqFloat<FloatView, Int::BoolView, RM_EQV>::post, Rel::ReEqFloat<FloatView, Int::BoolView, RM_IMP>::post, Rel::ReEqFloat<FloatView, Int::BoolView, RM_PMI>::post | gecode/float/rel/eq.hpp:273 | the reified posts make `ReEqFloat`, whose `c(c0)` copy-constructs the member |
| G16 | `linear` | `c` | `FloatVal` | gecode/float/linear.cpp:40 | Float::Linear::post at :50 (by value, gecode/float/linear/post.cpp:254) | gecode/float/rel/nq.hpp:87; gecode/float/linear/nary.hpp:44 | the by-value hop `Linear::post(..., FloatVal c)` (gecode/float/linear/post.cpp:254) copies the `FloatVal`. For FRT_NQ/LT/GR it passes `c` unchanged to `rel(home, extend(...), frt, c)` (:264), whose `NqFloat::post` makes `new NqFloat` with `c(_c)` copy-constructing the member. For FRT_EQ/LQ/GQ, `post_nary` (:103) → `Eq/Lq<View, View>::post` → `Lin` constructor `c(c0)` (nary.hpp:44) stores `c` itself, or a value derived from it when `post_linear` (:117) subtracts assigned terms or negates it for FRT_GQ |
| G16 | `linear` | `c` | `FloatVal` | gecode/float/linear.cpp:54 | Float::Linear::post at :64 (`const&` with Reify, gecode/float/linear/post.cpp:275) | gecode/float/rel/eq.hpp:273; gecode/float/rel/lq-le.hpp:227, :314 | `Linear::post(..., const FloatVal& c, Reify r)` (gecode/float/linear/post.cpp:275) calls the reified Float `rel` (gecode/float/rel.cpp:178, A3), whose `ReEqFloat/ReLqFloat/ReLeFloat::post` make propagators that copy-construct `c(c0)` |
| G16 | `linear` | `c` | `FloatVal` | gecode/float/linear.cpp:68 | Float::Linear::post at :80 (by value, gecode/float/linear/post.cpp:254) | gecode/float/rel/nq.hpp:87; gecode/float/linear/nary.hpp:44 | the by-value hop `Linear::post(..., FloatVal c)` (gecode/float/linear/post.cpp:254) copies the `FloatVal`. For FRT_NQ/LT/GR it passes `c` unchanged to `rel(home, extend(...), frt, c)` (:264), whose `NqFloat::post` makes `new NqFloat` with `c(_c)` copy-constructing the member. For FRT_EQ/LQ/GQ, `post_nary` (:103) → `Eq/Lq<View, View>::post` → `Lin` constructor `c(c0)` (nary.hpp:44) stores `c` itself, or a value derived from it when `post_linear` (:117) subtracts assigned terms or negates it for FRT_GQ |
| G16 | `linear` | `c` | `FloatVal` | gecode/float/linear.cpp:84 | Float::Linear::post at :96 (`const&` with Reify, gecode/float/linear/post.cpp:275) | gecode/float/rel/eq.hpp:273; gecode/float/rel/lq-le.hpp:227, :314 | `Linear::post(..., const FloatVal& c, Reify r)` (gecode/float/linear/post.cpp:275) calls the reified Float `rel` (gecode/float/rel.cpp:178, A3), whose `ReEqFloat/ReLqFloat/ReLeFloat::post` make propagators that copy-construct `c(c0)` |
| G16 | `Eq::post` | `c` | `FloatVal` | gecode/float/linear/nary.hpp:127 | Eq<P, N> | gecode/float/linear/nary.hpp:44 | `new Eq` → `Lin` constructor `c(c0)` copy-constructs the member |
| G16 | `Lq::post` | `c` | `FloatVal` | gecode/float/linear/nary.hpp:259 | Lq<P, N> | gecode/float/linear/nary.hpp:44 | `new Lq` → `Lin` constructor `c(c0)` copy-constructs the member |
| G16 | `post_nary` | `c` | `FloatVal` | gecode/float/linear/post.cpp:103 | Eq<View, View>::post, Lq<View, View>::post | gecode/float/linear/nary.hpp:44 | `Eq/Lq::post` → `Lin` constructor `c(c0)` copy-constructs the member |
| G16 | `post` | `c` | `FloatVal` | gecode/float/linear/post.cpp:275 | reified Float `rel` at :277 (gecode/float/rel.cpp:178) | gecode/float/rel/eq.hpp:273; gecode/float/rel/lq-le.hpp:227, :314 | the reified `rel` posts `ReEqFloat/ReLqFloat/ReLeFloat`, whose constructors copy-construct `c(c0)` |
| G16 | `rel` | `n` | `FloatVal` | gecode/float/rel.cpp:42 | Limits::check, x.eq, Rel::NqFloat<FloatView>::post, x.lq, x.gq | gecode/float/rel/nq.hpp:87 | for FRT_NQ, `Rel::NqFloat<FloatView>::post` (gecode/float/rel/nq.hpp:91) makes `new NqFloat`, whose `c(_c)` copy-constructs the `FloatVal` member; other relations only update the domain |
| G16 | `rel` | `n` | `FloatVal` | gecode/float/rel.cpp:178 | Limits::check, Rel::ReEqFloat<FloatView, Int::BoolView, RM_EQV>::post, Rel::ReEqFloat<FloatView, Int::BoolView, RM_IMP>::post, Rel::ReEqFloat<FloatView, Int::BoolView, RM_PMI>::post, Rel::ReEqFloat<FloatView, Int::NegBoolView, RM_EQV>::post, Rel::ReEqFloat<FloatView, Int::NegBoolView, RM_PMI>::post, Rel::ReEqFloat<FloatView, Int::NegBoolView, RM_IMP>::post, Rel::ReLqFloat<FloatView, Int::BoolView, RM_EQV>::post, Rel::ReLqFloat<FloatView, Int::BoolView, RM_IMP>::post, Rel::ReLqFloat<FloatView, Int::BoolView, RM_PMI>::post, Rel::ReLeFloat<FloatView, Int::BoolView, RM_EQV>::post, Rel::ReLeFloat<FloatView, Int::BoolView, RM_IMP>::post, Rel::ReLeFloat<FloatView, Int::BoolView, RM_PMI>::post, Rel::ReLeFloat<FloatView, Int::NegBoolView, RM_EQV>::post, Rel::ReLeFloat<FloatView, Int::NegBoolView, RM_PMI>::post, Rel::ReLeFloat<FloatView, Int::NegBoolView, RM_IMP>::post, Rel::ReLqFloat<FloatView, Int::NegBoolView, RM_EQV>::post, Rel::ReLqFloat<FloatView, Int::NegBoolView, RM_PMI>::post, Rel::ReLqFloat<FloatView, Int::NegBoolView, RM_IMP>::post | gecode/float/rel/eq.hpp:273; gecode/float/rel/lq-le.hpp:227, :314 | the reified posts make `ReEqFloat/ReLqFloat/ReLeFloat`, whose `c(c0)` copy-constructs the member |
| G16 | `rel` | `c` | `FloatVal` | gecode/float/rel.cpp:285 | Limits::check, xi.eq, Rel::NqFloat<FloatView>::post, xi.lq, xi.gq | gecode/float/rel/nq.hpp:87 | for FRT_NQ, `Rel::NqFloat<FloatView>::post` (gecode/float/rel/nq.hpp:91) makes `new NqFloat`, whose `c(_c)` copy-constructs the `FloatVal` member; other relations only update the domain |
| G16 | `ReEqFloat::post` | `c` | `FloatVal` | gecode/float/rel/eq.hpp:277 | x.eq, overlap, ReEqFloat | gecode/float/rel/eq.hpp:273 | `new ReEqFloat(...)` copy-constructs `c(c0)` |
| G16 | `ReLqFloat::post` | `c` | `FloatVal` | gecode/float/rel/lq-le.hpp:231 | ReLqFloat<View, CtrlView, rm>, rtest_lq | gecode/float/rel/lq-le.hpp:227 | `new ReLqFloat(...)` copy-constructs `c(c0)` |
| G16 | `ReLeFloat::post` | `c` | `FloatVal` | gecode/float/rel/lq-le.hpp:318 | ReLeFloat<View, CtrlView, rm>, rtest_lt | gecode/float/rel/lq-le.hpp:314 | `new ReLeFloat(...)` copy-constructs `c(c0)` |
| G16 | `NqFloat::post` | `c` | `FloatVal` | gecode/float/rel/nq.hpp:91 | overlap, NqFloat<View> | gecode/float/rel/nq.hpp:87 | `new NqFloat(...)` copy-constructs `c(_c)` |
| G16 | `TestSpace::rel` | `n` | `Gecode::FloatVal` | test/float.cpp:222 | Gecode::rel at :247 (gecode/float/rel.cpp:42) | gecode/float/rel/nq.hpp:87 | for FRT_NQ, `Rel::NqFloat<FloatView>::post` (gecode/float/rel/nq.hpp:91) makes `new NqFloat`, whose `c(_c)` copy-constructs the `FloatVal` member; other relations only update the domain |

### A4. Keepers outside the baseline rows

| Group | Function | Parameter | Type | HEAD parameter | Copy site | How the copy is made |
| --- | --- | --- | --- | --- | --- | --- |
| G9 | `Count::IntBase::IntBase<VX, VY>` | `y0` | `VY` (IntSet when not a view) | gecode/int/count/int-base.hpp:37 | gecode/int/count/int-base.hpp:38 | copy-constructs member `y` (gecode/int/count.hh:143); reached from `EqInt/GqInt/LqInt` (A2) and their `post` (A3) |
| G9 | `Count::ViewBase::ViewBase<VX, VY, VZ>` | `y0` | `VY` (IntSet when not a view) | gecode/int/count/view-base.hpp:37 | gecode/int/count/view-base.hpp:38 | copy-constructs member `y` (gecode/int/count.hh:272); reached from `EqView/GqView/LqView` (A2) and their `post` (A3) |
| G9 | `Sequence::Sequence<View, Val>` | `s0` | `Val` (IntSet) | gecode/int/sequence/int.hpp:41 | gecode/int/sequence/int.hpp:42 | copy-constructs member `s`; `vvsamax`/`vvsamin` take `const Val&` and only read it; reached from `Sequence::post` (A3) |
| G9 | `Matrix::Matrix<A>` (square) | `a` | `A` (an Args array) | gecode/minimodel/matrix.hpp:136 | gecode/minimodel/matrix.hpp:137 | copy-constructs `_a` (the rectangular constructor at :128 is in A1) |
| G16 | `DomArgs::DomArgs<IntVar, Args...>` (IntVar specialization) | `args...` | `const Args&...` | gecode/minimodel/dom.cpp:114 | gecode/minimodel/dom.cpp:114 | copy-constructs the elements of the `std::tuple<IntVar, Args...> _args` member (dom.cpp:96), including the IntSet of `dom(const IntVar&, const IntSet&)` (dom.cpp:157-158); `x` stays by value |

### const& other: changed parameters that nothing keeps

| Group | Function | Parameter | Type | HEAD parameter | Use |
| --- | --- | --- | --- | --- | --- |
| G1 | `Propagator::solution_distribution` | `#2` | `SendMarginal` | ? | unnamed and unused in the base virtual declaration |
| G1 | `Propagator::domain_size_sum` | `#1` | `InDecision` | ? | unnamed and unused in the base virtual declaration |
| G1 | `cbs_distinct` | `send` | `Propagator::SendMarginal` | gecode/int/distinct/cbs.hpp:135 | called only |
| G1 | `cbs_size` | `in` | `Propagator::InDecision` | gecode/int/distinct/cbs.hpp:211 | called only |
| G3 | `Action::Storage::Storage` | `bm` | `typename BranchTraits<typename Gecode::Int::BoolView::VarType>::Merit` | gecode/kernel/branch/action.hpp:273 | called only (gecode/kernel/branch/action.hpp:278) |
| G3 | `Action::init` | `bm` | `typename BranchTraits<typename Gecode::Int::BoolView::VarType>::Merit` | gecode/kernel/branch/action.hpp:355 | passed to `Storage`, which only calls it |
| G3 | `CHB::Storage::Storage` | `bm` | `typename BranchTraits<typename Gecode::Int::BoolView::VarType>::Merit` | gecode/kernel/branch/chb.hpp:257 | called only (gecode/kernel/branch/chb.hpp:263) |
| G3 | `CHB::init` | `bm` | `typename BranchTraits<typename Gecode::Int::BoolView::VarType>::Merit` | gecode/kernel/branch/chb.hpp:344 | passed to `Storage`, which only calls it |
| G4 | `IntAction::IntAction` | `bm` | `IntBranchMerit` | gecode/int/branch/action.cpp:37 | passed to `Action::init`, whose `Storage` only calls it |
| G4 | `IntAction::init` | `bm` | `IntBranchMerit` | gecode/int/branch/action.cpp:43 | passed to `Action::init`, whose `Storage` only calls it |
| G4 | `BoolAction::BoolAction` | `bm` | `BoolBranchMerit` | gecode/int/branch/action.cpp:48 | passed to `Action::init`, whose `Storage` only calls it |
| G4 | `BoolAction::init` | `bm` | `BoolBranchMerit` | gecode/int/branch/action.cpp:54 | passed to `Action::init`, whose `Storage` only calls it |
| G4 | `IntCHB::IntCHB` | `bm` | `IntBranchMerit` | gecode/int/branch/chb.cpp:37 | passed to `CHB::init`, whose `Storage` only calls it |
| G4 | `IntCHB::init` | `bm` | `IntBranchMerit` | gecode/int/branch/chb.cpp:43 | passed to `CHB::init`, whose `Storage` only calls it |
| G4 | `BoolCHB::BoolCHB` | `bm` | `BoolBranchMerit` | gecode/int/branch/chb.cpp:48 | passed to `CHB::init`, whose `Storage` only calls it |
| G4 | `BoolCHB::init` | `bm` | `BoolBranchMerit` | gecode/int/branch/chb.cpp:54 | passed to `CHB::init`, whose `Storage` only calls it |
| G5 | `SetAction::SetAction` | `bm` | `SetBranchMerit` | gecode/set/branch/action.cpp:37 | passed to `Action::init`, whose `Storage` only calls it |
| G5 | `SetAction::init` | `bm` | `SetBranchMerit` | gecode/set/branch/action.cpp:43 | passed to `Action::init`, whose `Storage` only calls it |
| G5 | `SetCHB::SetCHB` | `bm` | `SetBranchMerit` | gecode/set/branch/chb.cpp:37 | passed to `CHB::init`, whose `Storage` only calls it |
| G5 | `SetCHB::init` | `bm` | `SetBranchMerit` | gecode/set/branch/chb.cpp:43 | passed to `CHB::init`, whose `Storage` only calls it |
| G6 | `FloatAction::FloatAction` | `bm` | `FloatBranchMerit` | gecode/float/branch/action.cpp:37 | passed to `Action::init`, whose `Storage` only calls it |
| G6 | `FloatAction::init` | `bm` | `FloatBranchMerit` | gecode/float/branch/action.cpp:43 | passed to `Action::init`, whose `Storage` only calls it |
| G6 | `FloatCHB::FloatCHB` | `bm` | `FloatBranchMerit` | gecode/float/branch/chb.cpp:37 | passed to `CHB::init`, whose `Storage` only calls it |
| G6 | `FloatCHB::init` | `bm` | `FloatBranchMerit` | gecode/float/branch/chb.cpp:43 | passed to `CHB::init`, whose `Storage` only calls it |
| G7 | `assign` | `vars` | `IntVarBranch` | gecode/int/branch.cpp:133 | passed to `ViewSelNone`, whose constructor stores nothing (gecode/kernel/branch/view-sel.hpp:400-401) |
| G7 | `assign` | `vars` | `BoolVarBranch` | gecode/int/branch.cpp:246 | passed to `ViewSelNone`, whose constructor stores nothing (gecode/kernel/branch/view-sel.hpp:400-401) |
| G7 | `check_open_selector` | `vars` | `IntVarBranch` | gecode/int/branch/open.hpp:255 | `select()` read only |
| G7 | `check_open_selector` | `vars` | `BoolVarBranch` | gecode/int/branch/open.hpp:274 | `select()` read only |
| G7 | `BrancherNoFilter::BrancherNoFilter<View>` | `bf` | `BranchFilter<Var>` | gecode/kernel/branch/filter.hpp:131 | `BrancherNoFilter` asserts the callback is empty and stores nothing; so does the new `&&` overload at :138 |
| G7 | `BrancherNoPrint::BrancherNoPrint<View, Val>` | `vvp` | `VarValPrint<Var, int>` | gecode/kernel/branch/print.hpp:135 | `BrancherNoPrint` asserts the callback is empty and stores nothing; so does the new `&&` overload at :142 |
| G8 | `assign` | `vars` | `FloatVarBranch` | gecode/float/branch.cpp:99 | passed to `ViewSelNone`, whose constructor stores nothing (gecode/kernel/branch/view-sel.hpp:400-401) |
| G8 | `assign` | `vars` | `SetVarBranch` | gecode/set/branch.cpp:94 | passed to `ViewSelNone`, whose constructor stores nothing (gecode/kernel/branch/view-sel.hpp:400-401) |
| G9 | `relax` | `r` | `Rnd` | gecode/float/relax.cpp:56 | by-value hop into `Search::relax` (gecode/search/relax.hh:47), which only draws numbers; that copy ends with the call |
| G9 | `is_int_set` | `y` | `IntSet` | gecode/int/count/rel.hpp:50 | unused (`(void)y`) |
| G9 | `isval` | `y` | `IntSet` | gecode/int/count/rel.hpp:62 | unused (`(void)y`) |
| G9 | `extensional` | `dfa` | `DFA` | gecode/int/extensional-regular.cpp:38 | `post_lgp` → `LayeredGraph::post` reads the DFA; LayeredGraph has no DFA member. Derived: the transition state indices copied into `Edge` arrays (gecode/int/extensional/layered-graph.hpp:292-308) |
| G9 | `extensional` | `dfa` | `DFA` | gecode/int/extensional-regular.cpp:47 | `post_lgp` → `LayeredGraph::post` reads the DFA; LayeredGraph has no DFA member. Derived: the transition state indices copied into `Edge` arrays (gecode/int/extensional/layered-graph.hpp:292-308) |
| G9 | `relax` | `r` | `Rnd` | gecode/int/relax.cpp:64 | by-value hop into `Search::relax` (gecode/search/relax.hh:47), which only draws numbers; that copy ends with the call |
| G9 | `relax` | `r` | `Rnd` | gecode/int/relax.cpp:76 | by-value hop into `Search::relax` (gecode/search/relax.hh:47), which only draws numbers; that copy ends with the call |
| G9 | `Sequence::check` | `s` | `Gecode::IntSet` | gecode/int/sequence/int.hpp:105 | membership tests (`includes`/`excludes`) |
| G9 | `ViewValSupport::alternative_not_possible` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:165 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupport::s_not_possible` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:172 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupport::init` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:179 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupport::shaved` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:214 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupport::schedule_conclusion` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:223 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupport::conclude` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:254 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupport::pushup` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:289 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupport::advise` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:334 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupport::propagate` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:372 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupportArray::ViewValSupportArray<View, Val, iss>` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:408 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupportArray::propagate` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:461 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `ViewValSupportArray::advise` | `s` | `Gecode::IntSet` | gecode/int/sequence/view.hpp:470 | ViewValSupport helpers take `const Val&` and only read it |
| G9 | `values` | `y` | `IntSet` | gecode/minimodel/aliases.hpp:122 | `dom(IntVarArgs, IntSet)` intersects domains through `IntSetRanges` (gecode/int/dom.cpp:94-103); `size()` read |
| G9 | `special_update` | `used_values` | `IntSet` | gecode/set/ldsb/brancher.hpp:84 | membership tests and value iteration. Derived: the `intersection` values that build the new `ValueSymmetryImp` (gecode/set/ldsb/brancher.hpp:89-107) |
| G9 | `relax` | `r` | `Rnd` | gecode/set/relax.cpp:61 | by-value hop into `Search::relax` (gecode/search/relax.hh:47), which only draws numbers; that copy ends with the call |
| G13 | `annotation_to_set_val_branch` | `rnd` | `Rnd` | gecode/flatzinc/flatzinc.cpp:677 | unused (`(void)rnd`) |
| G13 | `print_int_var` | `name` | `const std::string` | gecode/flatzinc/flatzinc.cpp:1796 | written to the output stream `os` |
| G13 | `print_bool_var` | `name` | `const std::string` | gecode/flatzinc/flatzinc.cpp:1817 | written to the output stream `os` |
| G13 | `print_float_var` | `name` | `const std::string` | gecode/flatzinc/flatzinc.cpp:1825 | written to the output stream `os` |
| G13 | `get_array_element` | `id` | `string` | gecode/flatzinc/parser.tab.cpp:133 | derived: generated from `get_array_element` at gecode/flatzinc/parser.yxx:98. `id` is the symbol-table key; when `annotation` is set, `oss << id << "[" << offset << "]"` formats a new indexed name (gecode/flatzinc/parser.tab.cpp:146-147), which the returned variable node holds, and the error path writes `id` to `pp->err` (gecode/flatzinc/parser.tab.cpp:205). `id` itself is not copied |
| G14 | `Comparator::compare` | `x_n` | `std::string` | gecode/gist/gist.hpp:64 | derived: formats `x_n` into an `std::ostringstream` and returns `ret.str()`, a new std::string (gecode/gist/gist.hpp:68-69) |
| G14 | `Comparator::compare` | `x_n` | `std::string` | gecode/gist/gist.hpp:74 | derived: formats `x_n` into an `std::ostringstream` and returns `ret.str()`, a new std::string (gecode/gist/gist.hpp:77-78) |
| G14 | `Comparator::compare` | `x_n` | `std::string` | gecode/gist/gist.hpp:84 | derived: formats `x_n` into an `std::ostringstream` and returns `ret.str()`, a new std::string (gecode/gist/gist.hpp:90-91) |
| G14 | `Comparator::compare` | `x_n` | `std::string` | gecode/gist/gist.hpp:98 | derived: formats `x_n` into an `std::ostringstream` and returns `ret.str()`, a new std::string (gecode/gist/gist.hpp:101-102) |
| G14 | `Comparator::compare` | `x_n` | `std::string` | gecode/gist/gist.hpp:109 | derived: formats `x_n[i]` into `xni`, passes `xni.str()` to the element `compare`, and returns `ret.str()`, a new std::string (gecode/gist/gist.hpp:116-117, :124, :127) |
| G15 | `tuple_set_to_dfa` | `ts` | `Gecode::TupleSet` | test/int/extensional.cpp:1084 | derived: tuple elements read to build a REG from ints and the returned DFA (test/int/extensional.cpp:1086-1094) |
| G15 | `VarSome::VarSome` | `s` | `std::string` | test/int/gcc.cpp:249 | derived: `Test` receives the new name `"GCC::Var::Some::" + s + ...` |
| G15 | `LDSB::LDSB<T>` | `label` | `std::string` | test/ldsb.cpp:216 | derived: `Test::Base` receives the new name `"LDSB::" + label` |
| G15 | `LDSBSet::LDSBSet<T>` | `label` | `std::string` | test/ldsb.cpp:245 | derived: `Test::Base` receives the new name `"LDSB::" + label` |
| G15 | `LDSBLatin::LDSBLatin` | `label` | `std::string` | test/ldsb.cpp:1252 | derived: `Test::Base` receives the new name `"LDSB::" + label` |
| G15 | `EmptyFixture::EmptyFixture<VarArray>` | `d` | `Gecode::IntSet` | test/ldsb.cpp:1722 | forwarded to the variable-array constructor, which creates variables and keeps nothing |
| G15 | `Queens::val` | `ivb` | `IntValBranch` | test/nogoods.cpp:114 | `select()` read only |
| G15 | `Hamming::val` | `svb` | `SetValBranch` | test/nogoods.cpp:210 | `select()` read only |
| G15 | `TestExecutionControl::write_output` | `output` | `std::string` | test/test.cpp:523 | written to `std::cout` |
| G16 | `dom` | `n` | `FloatVal` | gecode/float/dom.cpp:38 | range check and domain update `xv.eq` |
| G16 | `dom` | `n` | `FloatVal` | gecode/float/dom.cpp:47 | range check and domain update `xv.eq` |
| G16 | `estimate` | `c` | `FloatVal` | gecode/float/linear/post.cpp:44 | copied into the local `est`; only FloatNum bounds leave the call |
| G16 | `overflow` | `c` | `FloatVal` | gecode/float/linear/post.cpp:63 | copied into the local `est`; returns a bool |
| G16 | `FloatView::lq` | `n` | `FloatVal` | gecode/float/view/float.hpp:110 | domain update |
| G16 | `FloatView::gq` | `n` | `FloatVal` | gecode/float/view/float.hpp:123 | domain update |
| G16 | `MinusView::lq` | `n` | `FloatVal` | gecode/float/view/minus.hpp:107 | domain update |
| G16 | `MinusView::gq` | `n` | `FloatVal` | gecode/float/view/minus.hpp:120 | domain update |
| G16 | `OffsetView::lq` | `n` | `FloatVal` | gecode/float/view/offset.hpp:115 | domain update |
| G16 | `OffsetView::gq` | `n` | `FloatVal` | gecode/float/view/offset.hpp:128 | domain update |
| G16 | `rtest_eq` | `n` | `FloatVal` | gecode/float/view/rel-test.hpp:47 | compared only |
| G16 | `rtest_lq` | `n` | `FloatVal` | gecode/float/view/rel-test.hpp:65 | compared only |
| G16 | `rtest_lt` | `n` | `FloatVal` | gecode/float/view/rel-test.hpp:85 | compared only |
| G16 | `ScaleView::lq` | `n` | `FloatVal` | gecode/float/view/scale.hpp:125 | copied into a local, then a domain update |
| G16 | `ScaleView::gq` | `n` | `FloatVal` | gecode/float/view/scale.hpp:144 | copied into a local, then a domain update |
| G16 | `LinFloatRel::LinFloatRel` | `r` | `FloatVal` | gecode/minimodel/float-rel.hpp:49 | derived: `e(l - r)` calls `operator-(const LinFloatExpr&, const FloatVal&)` (gecode/minimodel/float-expr.cpp:470-471), which stores the negation `-c` in a new `NT_ADD` node |
| G16 | `Test::str` | `f` | `Gecode::FloatVal` | test/float.hpp:201 | derived: bounds written to a stream, returned as a new std::string (test/float.hpp:202-204) |
| G16 | `Test::cmp` | `x` | `Gecode::FloatVal` | test/float.hpp:216 | compared only |
| G16 | `Test::cmp` | `y` | `Gecode::FloatVal` | test/float.hpp:216 | compared only |
| G16 | `Test::eq` | `x` | `Gecode::FloatVal` | test/float.hpp:261 | forwarded to `cmp`, which compares |
| G16 | `Test::eq` | `y` | `Gecode::FloatVal` | test/float.hpp:261 | forwarded to `cmp`, which compares |

Outside the baseline rows, these changed parameters are not kept either. They are not counted above.

| HEAD parameter | Parameter | Use |
| --- | --- | --- |
| gecode/int/count/rel.hpp:44 | `y` | unused (`(void)y`) |
| gecode/int/count/rel.hpp:57 | `y` | `assigned()` read only |
| gecode/int/distinct/bnd.hpp:462 | `send` | forwarded to `cbs_distinct`, which only calls it |
| gecode/int/distinct/bnd.hpp:468 | `in` | forwarded to `cbs_size`, which only calls it |
| gecode/int/distinct/dom.hpp:80 | `send` | forwarded to `cbs_distinct`, which only calls it |
| gecode/int/distinct/dom.hpp:86 | `in` | forwarded to `cbs_size`, which only calls it |
| gecode/int/distinct/val.hpp:160 | `send` | forwarded to `cbs_distinct`, which only calls it |
| gecode/int/distinct/val.hpp:166 | `in` | forwarded to `cbs_size`, which only calls it |
| gecode/kernel/branch/action.hpp:340 | `bm` | `Action` constructor; passed to `Storage`, which only calls it |
| gecode/kernel/branch/chb.hpp:337 | `bm` | `CHB` constructor; passed to `Storage`, which only calls it |

Corrections in this round. "Kept" now excludes values computed from the
parameter (see the definition above). Eleven rows whose only retention is such
a derived value moved to "const& other": from A1, the five G14
`Comparator::compare` `x_n` rows (each returns a newly formatted string),
G13 `get_array_element` `id` (only a formatted indexed name survives), and
G16 `LinFloatRel(const LinFloatExpr&, FloatRelType, const FloatVal& r)` (the
node stores `-r`); from A2, G15 `VarSome` `s` and the `LDSB`, `LDSBSet` and
`LDSBLatin` `label` parameters (each base receives a newly built name). These
stay kept because the live object holds the parameter itself: the G4-G6
`*_MERIT`/`*_VAL`/`*_ASSIGN` factories, `get_var_ref_arg` (`AST::Atom(id)`),
the pentominoes `REG` handles, `fill`'s NT_NONLIN `tf->a = m`, the
`LinFloatRel(const FloatVal& l, ...)` node (`n->c = l`), and the Float
`linear` `c` (passed unchanged to `NqFloat` for FRT_NQ/LT/GR). Four uses that
an earlier review proposed as keeps are derived and stay in "const& other":
the DFA transition indices in `LayeredGraph` edges, the `special_update`
intersection, the DFA that `tuple_set_to_dfa` builds, and the string that
`Test::str(const FloatVal&)` returns. Also, `fill` stores `d += m * c` at
gecode/minimodel/float-expr.cpp:349, not :350, and B1 cited the `const&`
`BrancherNoFilter` constructor at filter.hpp:132; it is at :131.

## B. By-value-plus-move paths, by receiver

Every final destination below has a real move constructor: std::function
(all callbacks and BranchTbl), std::string, std::vector, std::shared_ptr,
std::exception_ptr, or a configuration class. The configuration classes
(VarBranch, ValBranch, TieBreak, Int/Bool/Set/FloatVarBranch, *ValBranch,
*Assign, IntBoolVarBranch, AuxVarBrancher's members, OpenVarList's
Functions) declare no copy constructor, copy assignment or destructor, so
their implicit moves exist. A grep of `gecode/kernel/branch/*.hpp`,
`gecode/{int,set,float}.hh`, `gecode/kernel.hh` and
`gecode/flatzinc/branch.hh` for user-declared copy operations or destructors
of these classes finds none. No `std::move` binds a copying `const&`
constructor: the only receivers with both overloads are `SharedData`/`SDO`,
where `Data&&` is chosen for rvalues, and `BrancherNoFilter`/`BrancherNoPrint`,
which store nothing. Their `SharedHandle` members (Rnd, AFC/Action/CHB) have
no move and are copied, which is why those parameters are the `const&` rows in
A1/A2.

### B1. Branch filter and print callbacks into `SharedData` (G2, G7, G8)

Storage: `BrancherFilter<View>::f` is a `SharedData<BranchFilter<Var>>`
(gecode/kernel/branch/filter.hpp:49), and `BrancherPrint<View, Val>::p` is a
`SharedData<VarValPrint<Var, Val>>` (gecode/kernel/branch/print.hpp:50).

- Sink: `BrancherFilter(BranchFilter<Var> bf)` at filter.hpp:92 does
  `f(std::move(bf))` at :93. That calls `SharedData(Data&&)` at
  gecode/kernel/data/shared-data.hpp:89, which does `new SDO(std::move(d))` at
  :90. `SDO(Data&& d0)` at :76 initializes `d(std::move(d0))` at :77; the
  member is `const Data d` at :43, which is move-constructed. Print goes the
  same way: `BrancherPrint(VarValPrint<Var, Val> vvp)` at print.hpp:95 does
  `p(std::move(vvp))` at :96 and continues into the same `SharedData`/`SDO`
  `&&` constructors.
- View-value branchers. `post_view_value_brancher` (view-val.hpp:299) picks
  Filter and Print and calls `ViewValBrancher<..>::post(..., std::move(bf),
  std::move(vvp))` at :304/:307/:312/:315. `post` (:220) does `new
  ViewValBrancher(..., std::move(bf), std::move(vvp))` at :224. The
  constructor (:209) passes `ViewBrancher<View, Filter, n>(home, x, vs,
  std::move(bf))` and `p(std::move(vvp))` at :213. `ViewBrancher` (view.hpp:139)
  does `f(std::move(bf))` at view.hpp:141, which reaches the sink.
- Int entry points (gecode/int/branch.cpp). `branch` at :38 calls
  post_view_value_brancher at :54. The tie-break `branch` at :61 calls it at
  :90/:104/:118. `assign` at :133 calls it at :140, and the tie-break `assign`
  at :145 at :166/:171/:176. The Bool overloads: `branch` :190 → :198, :203 →
  :225/:229/:233; `assign` :246 → :253, :258 → :280/:284/:288. The
  single-variable forms `branch` :126, :239 and `assign` :183, :294 forward with
  `std::move(vvp)`, for example :297. The inline wrappers in
  gecode/int/branch.hpp:37/:41/:46/:50 forward `std::move(bf),
  std::move(vvp)`.
- Set entry points (gecode/set/branch.cpp): `branch` :38 → :46, :51 →
  :73/:77/:81, :87; `assign` :94 → :101, :106 → :128/:132/:136, :142. Float
  entry points (gecode/float/branch.cpp): `branch` :38 → :46, :51 →
  :74/:79/:85, :92; `assign` :99. Wrappers: gecode/set/branch.hpp:37/:42 and
  gecode/float/branch.hpp:37/:42.
- Value-enumerating branchers (INT_VALUES_*). `post_view_values_brancher`
  (gecode/int/branch/view-values.hpp:175) calls `ViewValuesBrancher::post` at
  :180/:183/:188/:191. `post` (:98) does `new ...(std::move(bf),
  std::move(vvp))` at :100. The constructor passes `ViewBrancher(...,
  std::move(bf))` and `p(std::move(vvp))` at :91.
- LDSB. Int: gecode/int/ldsb.cpp `branch` :289 → `post_ldsb_brancher` :324,
  :330 → :378/:400/:422, Bool :430 → :462, :468 → :516/:520/:524. Then
  gecode/int/ldsb/brancher.hpp `post_ldsb_brancher` :251 → :257/:260/:265/:268
  → `LDSBBrancher::post` → `new` at :107. The constructor at :95 passes to
  `ViewValBrancher(..., std::move(bf), std::move(vvp))`. Set:
  gecode/set/ldsb.cpp `branch` :152 → :172, :177 → :210/:214/:218. Then
  gecode/set/ldsb/brancher.hpp `post_ldsb_set_brancher` :240 →
  :246/:249/:254/:257 → `new` at :234. The constructor at :40 passes to
  `LDSBBrancher(..., std::move(bf), std::move(vvp))`.
- Open lists (OpenListBrancher). `branch(Home, OpenIntVarList, IntVarBranch
  vars, ...)` at gecode/int/branch.cpp:311 calls `branch(home, list,
  TieBreak<IntVarBranch>(std::move(vars)), vals, std::move(bf),
  std::move(vvp))` at :313. Bool: :325 → :327. Then :317 → `post_open_brancher`
  at :320 (gecode/int/branch/open.hpp:320), which calls `post_open_brancher<View,
  n>` at :339/:342/:345/:348. That function (open.hpp:307) calls
  `OpenListBrancher<..>::post(..., std::move(bf), std::move(vvp))` at :311/:314.
  `post` (open.hpp:242) does `new OpenListBrancher(..., std::move(bf),
  std::move(vvp))` at :247-248. The constructor (:56) does
  `filter(std::move(bf))` and `printer(std::move(vvp))` at :58, which reach the
  BrancherFilter/BrancherPrint sinks above.
- No-callback policies: `BrancherNoFilter(const BranchFilter<Var>&)` at
  filter.hpp:131 and `(BranchFilter<Var>&&)` at :138, and `BrancherNoPrint` at
  print.hpp:135/:142, receive the moved-from empty callback and store nothing.

### B2. Variable and value selection configurations (G3-G6)

Storage: `VarBranch<Var>::_tbl` (`BranchTbl`, a std::function, at
gecode/kernel/branch/var.hpp:61) and `_mf` (`MeritFunction`, std::function,
:73), plus `ValBranch<Var>::vf`/`cf` (std::function).

- Kernel sinks. `VarBranch(BranchTbl t)` :119, `(double, BranchTbl)` :122,
  `(const AFC&, BranchTbl)` :125, `(const Action&, BranchTbl)` :131 and
  `(const CHB&, BranchTbl)` :137 all do `_tbl(std::move(t))`. `(MeritFunction
  f, BranchTbl t)` does `_tbl(std::move(t))` and `_mf(std::move(f))` at :150.
  `ValBranch(BranchVal v, BranchCommit c)` does `vf(std::move(v))` and
  `cf(std::move(c))` at gecode/kernel/branch/val.hpp:80.
- Int factories and constructors (gecode/int/branch/var.hpp, val.hpp,
  assign.hpp). For example, `INT_VAR_SIZE_MIN(BranchTbl tbl)` at var.hpp:213
  returns `IntVarBranch(SEL_SIZE_MIN, std::move(tbl))` at :214, and
  `IntVarBranch(Select, BranchTbl t)` (:41) does `VarBranch<IntVar>(std::move(t))`
  at :42. `INT_VAR_MERIT_MIN(IntBranchMerit bm, BranchTbl tbl)` at :113 moves
  both at :114 into the merit constructor, then into `VarBranch(MeritFunction,
  BranchTbl)` at kernel var.hpp:150. `INT_VAL(IntBranchVal v, IntBranchCommit
  c)` at val.hpp:93 returns `IntValBranch(std::move(v), std::move(c))` at :94;
  `IntValBranch(v, c)` (:44) does `ValBranch<IntVar>(std::move(v),
  std::move(c))` at :45. `INT_ASSIGN(v, c)` returns `IntAssign(std::move(v),
  std::move(c))` at assign.hpp:74, and the Bool constructor is at :87.
  IntBranchMerit/Val/Commit are the same std::function types as
  `BranchTraits<IntVar>::Merit/Val/Commit` (gecode/int/branch/traits.hpp:42-48),
  so no conversion temporary is made. Set (gecode/set/branch/*.hpp) and Float
  (gecode/float/branch/*.hpp) follow the same pattern; the B-appendix gives the
  HEAD line and move site of every parameter.
- TieBreak. `TieBreak(VarBranch a0, b0, c0, d0)` at
  gecode/kernel/branch/tiebreak.hpp:65 does `a(std::move(a0)) ...
  d(std::move(d0))` at :66. `tie_break(a, b)` (:70), `(a, b, c)` and `(a, b, c,
  d)` build it with `std::move` at :71/:78/:85 and return the named local
  (NRVO or implicit move). The receiving members are VarBranch subclasses with
  implicit moves.

### B3. Execution callbacks into `SharedData` (G10)

- `wait`: gecode/int/exec.cpp:40/:46 (unary) and :53/:60 (n-ary),
  gecode/set/exec.cpp:40/:47, and gecode/float/exec.cpp:40/:47 call
  `UnaryWait<View>::post(..., std::move(c))` or `NaryWait<View>::post`. In
  gecode/kernel/propagator/wait.hpp, `UnaryWait::post` makes `new UnaryWait(...,
  std::move(c))` at :150 and the constructor does `c(std::move(c0))` at :108.
  `NaryWait::post` forwards to `UnaryWait::post(..., std::move(c))` at :237, or
  makes `new NaryWait(..., std::move(c))` at :239, whose constructor does
  `c(std::move(c0))` at :171. The members are
  `SharedData<std::function<void(Space&)>> c` (wait.hpp:47/:80), so this reaches
  `SharedData(Data&&)` and `SDO(Data&&)`.
- `when`: gecode/int/exec.cpp:66 calls `When::post(home, x, std::move(t),
  std::move(e))`, and the else-less overload passes `std::move(t)` at :71.
  gecode/int/exec/when.hpp:58 makes `new When(..., std::move(t),
  std::move(e))`, and the constructor does `t(std::move(t0))` and
  `e(std::move(e0))` at :37. The members are `SharedData<std::function<...>>`
  (gecode/int/exec.hh:55/:57).
- Function branch: `branch(Home, std::function<void(Space&)> f)` calls
  `FunctionBranch::post(home, std::move(f))` at
  gecode/kernel/branch/function.cpp:144. `post` makes `new FunctionBranch(home,
  std::move(f))` at :128, and the constructor does `f(std::move(f0))` at :88.
  The member is a `SharedData<std::function<...>>` (:51).

### B4. Open variable lists (G11)

The `Domain d` and `Transition t` parameters of the `OpenIntVarList`
constructors (gecode/int/open-var-list.cpp:53/:56/:59) and `OpenBoolVarList`
(:67) are moved. `factory(Domain d)` (:40) moves `d` into an init-capture at
:43. `OpenVarList<Var>(Space&, Factory create, [Transition t,] int max)`
(gecode/int/open-var-list.hpp:284/:290) makes `new Object(home,
std::move(create), [std::move(t),] max)` at :285/:291. `Object` (:85/:91)
initializes `functions(Functions(checked(std::move(create)),
checked(std::move(t))))` at :87/:93. `checked` takes its parameter by value
and returns it, which is an implicit move (:72/:79). `Functions(Factory,
Transition)` does `create(std::move(create0))` and
`transition(std::move(transition0))` at :46. The `Functions` prvalue then
reaches `SharedData<Functions>(Data&&)` and `SDO(Data&&)` (member at :61).

### B5. FlatZinc (G13)

- `AuxVarBrancher`: `post` (gecode/flatzinc/flatzinc.cpp:246-254) makes `new
  AuxVarBrancher(home, std::move(int_varsel), ...)` at :257-265. The
  constructor (:113-121) initializes each member with `std::move` at
  :124/:125/:128/:132. The members are `TieBreak<Int/Bool/FloatVarBranch>`,
  `SetVarBranch` and `*ValBranch` values with implicit moves.
- `BranchInformationO(std::vector<BI> v0) : v(std::move(v0))` at :287.
- `BlackBoxContextHandle::fail(std::exception_ptr e)` (blackbox-propagator.cpp:107)
  calls `->fail(std::move(e))` at :108. `BlackBoxContext::fail` (:144) does
  `exception = std::move(e)` at :147.
- `ParserState::output(std::string x, AST::Node* n)` (parser.hh:258) does
  `output_items.emplace_back(std::move(x), n)` at :259.

### B6. Gist and search (G14)

`CPProfilerSearchTracer(int eid, std::string name0, ...)`
(gecode/search/cpprofiler/tracer.cpp:54, declared in gecode/search.hh:454)
does `name(std::move(name0))` at :56 into a std::string member.

### B7. Tests and examples (G15)

The fixture constructors move std::string, std::vector<std::string>,
std::shared_ptr<const Graph>, std::function (BeforeRun, OutputCheck,
`Case::f`, the min-distance callback), OpenIntVarList Domain/Transition, and
NoGoods' `vb0` into their members or delegated constructors. Examples:
test/flatzinc.cpp:103-122, test/test.cpp:122, test/search.cpp:137-138,
test/instance.cpp:118, test/int/open-var-list.cpp:1023-1024,
test/nogoods.cpp:250-254, test/int/min-distance.cpp:114-116. The B-appendix
lists each one with its HEAD line and move site.

### B-appendix. Every by-value-and-move parameter at HEAD

Each row gives the parameter at HEAD, still by value, and the first `std::move` of it in the body.

| Group | Receiver type | Function | Parameter | HEAD parameter | First std::move | Callee use (baseline) |
| --- | --- | --- | --- | --- | --- | --- |
| G3 | `BranchCommit` | `ValBranch::ValBranch<Var>` | `c` | gecode/kernel/branch/val.hpp:80 | gecode/kernel/branch/val.hpp:80 | keeps: initializes cf; copied once (last use) |
| G3 | `BranchVal` | `ValBranch::ValBranch<Var>` | `v` | gecode/kernel/branch/val.hpp:80 | gecode/kernel/branch/val.hpp:80 | keeps: initializes vf; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `TieBreak::TieBreak<VarBranch>` | `a0` | gecode/kernel/branch/tiebreak.hpp:65 | gecode/kernel/branch/tiebreak.hpp:66 | keeps: initializes a; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `TieBreak::TieBreak<VarBranch>` | `b0` | gecode/kernel/branch/tiebreak.hpp:65 | gecode/kernel/branch/tiebreak.hpp:66 | keeps: initializes b; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `TieBreak::TieBreak<VarBranch>` | `c0` | gecode/kernel/branch/tiebreak.hpp:65 | gecode/kernel/branch/tiebreak.hpp:66 | keeps: initializes c; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `TieBreak::TieBreak<VarBranch>` | `d0` | gecode/kernel/branch/tiebreak.hpp:65 | gecode/kernel/branch/tiebreak.hpp:66 | keeps: initializes d; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `a` | gecode/kernel/branch/tiebreak.hpp:70 | gecode/kernel/branch/tiebreak.hpp:71 | keeps: builds ab, which it returns; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `b` | gecode/kernel/branch/tiebreak.hpp:70 | gecode/kernel/branch/tiebreak.hpp:71 | keeps: builds ab, which it returns; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `a` | gecode/kernel/branch/tiebreak.hpp:77 | gecode/kernel/branch/tiebreak.hpp:78 | keeps: builds abc, which it returns; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `b` | gecode/kernel/branch/tiebreak.hpp:77 | gecode/kernel/branch/tiebreak.hpp:78 | keeps: builds abc, which it returns; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `c` | gecode/kernel/branch/tiebreak.hpp:77 | gecode/kernel/branch/tiebreak.hpp:78 | keeps: builds abc, which it returns; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `a` | gecode/kernel/branch/tiebreak.hpp:84 | gecode/kernel/branch/tiebreak.hpp:85 | keeps: builds abcd, which it returns; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `b` | gecode/kernel/branch/tiebreak.hpp:84 | gecode/kernel/branch/tiebreak.hpp:85 | keeps: builds abcd, which it returns; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `c` | gecode/kernel/branch/tiebreak.hpp:84 | gecode/kernel/branch/tiebreak.hpp:85 | keeps: builds abcd, which it returns; copied once (last use) |
| G3 | `Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch / Gecode::*VarBranch` | `tie_break` | `d` | gecode/kernel/branch/tiebreak.hpp:84 | gecode/kernel/branch/tiebreak.hpp:85 | keeps: builds abcd, which it returns; copied once (last use) |
| G4 | `*BranchCommit` | `BOOL_ASSIGN` | `c` | gecode/int/branch/assign.hpp:110 | gecode/int/branch/assign.hpp:111 | keeps: returns a BoolAssign built from it; copied once (last use) |
| G4 | `*BranchCommit` | `IntAssign::IntAssign` | `c` | gecode/int/branch/assign.hpp:44 | gecode/int/branch/assign.hpp:45 | passes to base or delegated constructor ValBranch<IntVar>; copied once (last use) |
| G4 | `*BranchCommit` | `INT_ASSIGN` | `c` | gecode/int/branch/assign.hpp:73 | gecode/int/branch/assign.hpp:74 | keeps: returns a IntAssign built from it; copied once (last use) |
| G4 | `*BranchCommit` | `BoolAssign::BoolAssign` | `c` | gecode/int/branch/assign.hpp:86 | gecode/int/branch/assign.hpp:87 | passes to base or delegated constructor ValBranch<BoolVar>; copied once (last use) |
| G4 | `*BranchCommit` | `BoolValBranch::BoolValBranch` | `c` | gecode/int/branch/val.hpp:116 | gecode/int/branch/val.hpp:117 | passes to base or delegated constructor ValBranch<BoolVar>; copied once (last use) |
| G4 | `*BranchCommit` | `BOOL_VAL` | `c` | gecode/int/branch/val.hpp:140 | gecode/int/branch/val.hpp:141 | keeps: returns a BoolValBranch built from it; copied once (last use) |
| G4 | `*BranchCommit` | `IntValBranch::IntValBranch` | `c` | gecode/int/branch/val.hpp:44 | gecode/int/branch/val.hpp:45 | passes to base or delegated constructor ValBranch<IntVar>; copied once (last use) |
| G4 | `*BranchCommit` | `INT_VAL` | `c` | gecode/int/branch/val.hpp:93 | gecode/int/branch/val.hpp:94 | keeps: returns a IntValBranch built from it; copied once (last use) |
| G4 | `*BranchMerit` | `INT_VAR_MERIT_MIN` | `bm` | gecode/int/branch/var.hpp:113 | gecode/int/branch/var.hpp:114 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `*BranchMerit` | `INT_VAR_MERIT_MAX` | `bm` | gecode/int/branch/var.hpp:118 | gecode/int/branch/var.hpp:119 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `*BranchMerit` | `BOOL_VAR_MERIT_MIN` | `bm` | gecode/int/branch/var.hpp:382 | gecode/int/branch/var.hpp:383 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `*BranchMerit` | `BOOL_VAR_MERIT_MAX` | `bm` | gecode/int/branch/var.hpp:387 | gecode/int/branch/var.hpp:388 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `*BranchVal` | `BOOL_ASSIGN` | `v` | gecode/int/branch/assign.hpp:110 | gecode/int/branch/assign.hpp:111 | keeps: returns a BoolAssign built from it; copied once (last use) |
| G4 | `*BranchVal` | `IntAssign::IntAssign` | `v` | gecode/int/branch/assign.hpp:44 | gecode/int/branch/assign.hpp:45 | passes to base or delegated constructor ValBranch<IntVar>; copied once (last use) |
| G4 | `*BranchVal` | `INT_ASSIGN` | `v` | gecode/int/branch/assign.hpp:73 | gecode/int/branch/assign.hpp:74 | keeps: returns a IntAssign built from it; copied once (last use) |
| G4 | `*BranchVal` | `BoolAssign::BoolAssign` | `v` | gecode/int/branch/assign.hpp:86 | gecode/int/branch/assign.hpp:87 | passes to base or delegated constructor ValBranch<BoolVar>; copied once (last use) |
| G4 | `*BranchVal` | `BoolValBranch::BoolValBranch` | `v` | gecode/int/branch/val.hpp:116 | gecode/int/branch/val.hpp:117 | passes to base or delegated constructor ValBranch<BoolVar>; copied once (last use) |
| G4 | `*BranchVal` | `BOOL_VAL` | `v` | gecode/int/branch/val.hpp:140 | gecode/int/branch/val.hpp:141 | keeps: returns a BoolValBranch built from it; copied once (last use) |
| G4 | `*BranchVal` | `IntValBranch::IntValBranch` | `v` | gecode/int/branch/val.hpp:44 | gecode/int/branch/val.hpp:45 | passes to base or delegated constructor ValBranch<IntVar>; copied once (last use) |
| G4 | `*BranchVal` | `INT_VAL` | `v` | gecode/int/branch/val.hpp:93 | gecode/int/branch/val.hpp:94 | keeps: returns a IntValBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_MERIT_MIN` | `tbl` | gecode/int/branch/var.hpp:113 | gecode/int/branch/var.hpp:114 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_MERIT_MAX` | `tbl` | gecode/int/branch/var.hpp:118 | gecode/int/branch/var.hpp:119 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_DEGREE_MIN` | `tbl` | gecode/int/branch/var.hpp:123 | gecode/int/branch/var.hpp:124 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_DEGREE_MAX` | `tbl` | gecode/int/branch/var.hpp:128 | gecode/int/branch/var.hpp:129 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_AFC_MIN` | `tbl` | gecode/int/branch/var.hpp:133 | gecode/int/branch/var.hpp:134 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_AFC_MIN` | `tbl` | gecode/int/branch/var.hpp:138 | gecode/int/branch/var.hpp:139 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_AFC_MAX` | `tbl` | gecode/int/branch/var.hpp:143 | gecode/int/branch/var.hpp:144 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_AFC_MAX` | `tbl` | gecode/int/branch/var.hpp:148 | gecode/int/branch/var.hpp:149 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_ACTION_MIN` | `tbl` | gecode/int/branch/var.hpp:153 | gecode/int/branch/var.hpp:154 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_ACTION_MIN` | `tbl` | gecode/int/branch/var.hpp:158 | gecode/int/branch/var.hpp:159 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_ACTION_MAX` | `tbl` | gecode/int/branch/var.hpp:163 | gecode/int/branch/var.hpp:164 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_ACTION_MAX` | `tbl` | gecode/int/branch/var.hpp:168 | gecode/int/branch/var.hpp:169 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_CHB_MIN` | `tbl` | gecode/int/branch/var.hpp:173 | gecode/int/branch/var.hpp:174 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_CHB_MIN` | `tbl` | gecode/int/branch/var.hpp:178 | gecode/int/branch/var.hpp:179 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_CHB_MAX` | `tbl` | gecode/int/branch/var.hpp:183 | gecode/int/branch/var.hpp:184 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_CHB_MAX` | `tbl` | gecode/int/branch/var.hpp:188 | gecode/int/branch/var.hpp:189 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_MIN_MIN` | `tbl` | gecode/int/branch/var.hpp:193 | gecode/int/branch/var.hpp:194 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_MIN_MAX` | `tbl` | gecode/int/branch/var.hpp:198 | gecode/int/branch/var.hpp:199 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_MAX_MIN` | `tbl` | gecode/int/branch/var.hpp:203 | gecode/int/branch/var.hpp:204 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_MAX_MAX` | `tbl` | gecode/int/branch/var.hpp:208 | gecode/int/branch/var.hpp:209 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_SIZE_MIN` | `tbl` | gecode/int/branch/var.hpp:213 | gecode/int/branch/var.hpp:214 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_SIZE_MAX` | `tbl` | gecode/int/branch/var.hpp:218 | gecode/int/branch/var.hpp:219 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_DEGREE_SIZE_MIN` | `tbl` | gecode/int/branch/var.hpp:223 | gecode/int/branch/var.hpp:224 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_DEGREE_SIZE_MAX` | `tbl` | gecode/int/branch/var.hpp:228 | gecode/int/branch/var.hpp:229 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_AFC_SIZE_MIN` | `tbl` | gecode/int/branch/var.hpp:233 | gecode/int/branch/var.hpp:234 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_AFC_SIZE_MIN` | `tbl` | gecode/int/branch/var.hpp:238 | gecode/int/branch/var.hpp:239 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_AFC_SIZE_MAX` | `tbl` | gecode/int/branch/var.hpp:243 | gecode/int/branch/var.hpp:244 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_AFC_SIZE_MAX` | `tbl` | gecode/int/branch/var.hpp:248 | gecode/int/branch/var.hpp:249 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_ACTION_SIZE_MIN` | `tbl` | gecode/int/branch/var.hpp:253 | gecode/int/branch/var.hpp:254 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_ACTION_SIZE_MIN` | `tbl` | gecode/int/branch/var.hpp:258 | gecode/int/branch/var.hpp:259 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_ACTION_SIZE_MAX` | `tbl` | gecode/int/branch/var.hpp:263 | gecode/int/branch/var.hpp:264 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_ACTION_SIZE_MAX` | `tbl` | gecode/int/branch/var.hpp:268 | gecode/int/branch/var.hpp:269 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_CHB_SIZE_MIN` | `tbl` | gecode/int/branch/var.hpp:273 | gecode/int/branch/var.hpp:274 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_CHB_SIZE_MIN` | `tbl` | gecode/int/branch/var.hpp:278 | gecode/int/branch/var.hpp:279 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_CHB_SIZE_MAX` | `tbl` | gecode/int/branch/var.hpp:283 | gecode/int/branch/var.hpp:284 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_CHB_SIZE_MAX` | `tbl` | gecode/int/branch/var.hpp:288 | gecode/int/branch/var.hpp:289 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_REGRET_MIN_MIN` | `tbl` | gecode/int/branch/var.hpp:293 | gecode/int/branch/var.hpp:294 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_REGRET_MIN_MAX` | `tbl` | gecode/int/branch/var.hpp:298 | gecode/int/branch/var.hpp:299 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_REGRET_MAX_MIN` | `tbl` | gecode/int/branch/var.hpp:303 | gecode/int/branch/var.hpp:304 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `INT_VAR_REGRET_MAX_MAX` | `tbl` | gecode/int/branch/var.hpp:308 | gecode/int/branch/var.hpp:309 | keeps: returns a IntVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_MERIT_MIN` | `tbl` | gecode/int/branch/var.hpp:382 | gecode/int/branch/var.hpp:383 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_MERIT_MAX` | `tbl` | gecode/int/branch/var.hpp:387 | gecode/int/branch/var.hpp:388 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_DEGREE_MIN` | `tbl` | gecode/int/branch/var.hpp:392 | gecode/int/branch/var.hpp:393 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_DEGREE_MAX` | `tbl` | gecode/int/branch/var.hpp:397 | gecode/int/branch/var.hpp:398 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_AFC_MIN` | `tbl` | gecode/int/branch/var.hpp:402 | gecode/int/branch/var.hpp:403 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_AFC_MIN` | `tbl` | gecode/int/branch/var.hpp:407 | gecode/int/branch/var.hpp:408 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_AFC_MAX` | `tbl` | gecode/int/branch/var.hpp:412 | gecode/int/branch/var.hpp:413 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_AFC_MAX` | `tbl` | gecode/int/branch/var.hpp:417 | gecode/int/branch/var.hpp:418 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_ACTION_MIN` | `tbl` | gecode/int/branch/var.hpp:422 | gecode/int/branch/var.hpp:423 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_ACTION_MIN` | `tbl` | gecode/int/branch/var.hpp:427 | gecode/int/branch/var.hpp:428 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_ACTION_MAX` | `tbl` | gecode/int/branch/var.hpp:432 | gecode/int/branch/var.hpp:433 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_ACTION_MAX` | `tbl` | gecode/int/branch/var.hpp:437 | gecode/int/branch/var.hpp:438 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_CHB_MIN` | `tbl` | gecode/int/branch/var.hpp:442 | gecode/int/branch/var.hpp:443 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_CHB_MIN` | `tbl` | gecode/int/branch/var.hpp:447 | gecode/int/branch/var.hpp:448 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_CHB_MAX` | `tbl` | gecode/int/branch/var.hpp:452 | gecode/int/branch/var.hpp:453 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G4 | `BranchTbl` | `BOOL_VAR_CHB_MAX` | `tbl` | gecode/int/branch/var.hpp:457 | gecode/int/branch/var.hpp:458 | keeps: returns a BoolVarBranch built from it; copied once (last use) |
| G5 | `*BranchCommit` | `SetAssign::SetAssign` | `c` | gecode/set/branch/assign.hpp:44 | gecode/set/branch/assign.hpp:45 | passes to base or delegated constructor ValBranch<SetVar>; copied once (last use) |
| G5 | `*BranchCommit` | `SET_ASSIGN` | `c` | gecode/set/branch/assign.hpp:93 | gecode/set/branch/assign.hpp:94 | keeps: returns a SetAssign built from it; copied once (last use) |
| G5 | `*BranchCommit` | `SetValBranch::SetValBranch` | `c` | gecode/set/branch/val.hpp:44 | gecode/set/branch/val.hpp:45 | passes to base or delegated constructor ValBranch<SetVar>; copied once (last use) |
| G5 | `*BranchCommit` | `SET_VAL` | `c` | gecode/set/branch/val.hpp:93 | gecode/set/branch/val.hpp:94 | keeps: returns a SetValBranch built from it; copied once (last use) |
| G5 | `*BranchMerit` | `SET_VAR_MERIT_MIN` | `bm` | gecode/set/branch/var.hpp:113 | gecode/set/branch/var.hpp:114 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `*BranchMerit` | `SET_VAR_MERIT_MAX` | `bm` | gecode/set/branch/var.hpp:118 | gecode/set/branch/var.hpp:119 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `*BranchVal` | `SetAssign::SetAssign` | `v` | gecode/set/branch/assign.hpp:44 | gecode/set/branch/assign.hpp:45 | passes to base or delegated constructor ValBranch<SetVar>; copied once (last use) |
| G5 | `*BranchVal` | `SET_ASSIGN` | `v` | gecode/set/branch/assign.hpp:93 | gecode/set/branch/assign.hpp:94 | keeps: returns a SetAssign built from it; copied once (last use) |
| G5 | `*BranchVal` | `SetValBranch::SetValBranch` | `v` | gecode/set/branch/val.hpp:44 | gecode/set/branch/val.hpp:45 | passes to base or delegated constructor ValBranch<SetVar>; copied once (last use) |
| G5 | `*BranchVal` | `SET_VAL` | `v` | gecode/set/branch/val.hpp:93 | gecode/set/branch/val.hpp:94 | keeps: returns a SetValBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_MERIT_MIN` | `tbl` | gecode/set/branch/var.hpp:113 | gecode/set/branch/var.hpp:114 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_MERIT_MAX` | `tbl` | gecode/set/branch/var.hpp:118 | gecode/set/branch/var.hpp:119 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_DEGREE_MIN` | `tbl` | gecode/set/branch/var.hpp:123 | gecode/set/branch/var.hpp:124 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_DEGREE_MAX` | `tbl` | gecode/set/branch/var.hpp:128 | gecode/set/branch/var.hpp:129 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_AFC_MIN` | `tbl` | gecode/set/branch/var.hpp:133 | gecode/set/branch/var.hpp:134 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_AFC_MIN` | `tbl` | gecode/set/branch/var.hpp:138 | gecode/set/branch/var.hpp:139 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_AFC_MAX` | `tbl` | gecode/set/branch/var.hpp:143 | gecode/set/branch/var.hpp:144 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_AFC_MAX` | `tbl` | gecode/set/branch/var.hpp:148 | gecode/set/branch/var.hpp:149 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_ACTION_MIN` | `tbl` | gecode/set/branch/var.hpp:153 | gecode/set/branch/var.hpp:154 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_ACTION_MIN` | `tbl` | gecode/set/branch/var.hpp:158 | gecode/set/branch/var.hpp:159 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_ACTION_MAX` | `tbl` | gecode/set/branch/var.hpp:163 | gecode/set/branch/var.hpp:164 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_ACTION_MAX` | `tbl` | gecode/set/branch/var.hpp:168 | gecode/set/branch/var.hpp:169 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_CHB_MIN` | `tbl` | gecode/set/branch/var.hpp:173 | gecode/set/branch/var.hpp:174 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_CHB_MIN` | `tbl` | gecode/set/branch/var.hpp:178 | gecode/set/branch/var.hpp:179 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_CHB_MAX` | `tbl` | gecode/set/branch/var.hpp:183 | gecode/set/branch/var.hpp:184 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_CHB_MAX` | `tbl` | gecode/set/branch/var.hpp:188 | gecode/set/branch/var.hpp:189 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_MIN_MIN` | `tbl` | gecode/set/branch/var.hpp:193 | gecode/set/branch/var.hpp:194 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_MIN_MAX` | `tbl` | gecode/set/branch/var.hpp:198 | gecode/set/branch/var.hpp:199 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_MAX_MIN` | `tbl` | gecode/set/branch/var.hpp:203 | gecode/set/branch/var.hpp:204 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_MAX_MAX` | `tbl` | gecode/set/branch/var.hpp:208 | gecode/set/branch/var.hpp:209 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_SIZE_MIN` | `tbl` | gecode/set/branch/var.hpp:213 | gecode/set/branch/var.hpp:214 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_SIZE_MAX` | `tbl` | gecode/set/branch/var.hpp:218 | gecode/set/branch/var.hpp:219 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_DEGREE_SIZE_MIN` | `tbl` | gecode/set/branch/var.hpp:223 | gecode/set/branch/var.hpp:224 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_DEGREE_SIZE_MAX` | `tbl` | gecode/set/branch/var.hpp:228 | gecode/set/branch/var.hpp:229 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_AFC_SIZE_MIN` | `tbl` | gecode/set/branch/var.hpp:233 | gecode/set/branch/var.hpp:234 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_AFC_SIZE_MIN` | `tbl` | gecode/set/branch/var.hpp:238 | gecode/set/branch/var.hpp:239 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_AFC_SIZE_MAX` | `tbl` | gecode/set/branch/var.hpp:243 | gecode/set/branch/var.hpp:244 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_AFC_SIZE_MAX` | `tbl` | gecode/set/branch/var.hpp:248 | gecode/set/branch/var.hpp:249 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_ACTION_SIZE_MIN` | `tbl` | gecode/set/branch/var.hpp:253 | gecode/set/branch/var.hpp:254 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_ACTION_SIZE_MIN` | `tbl` | gecode/set/branch/var.hpp:258 | gecode/set/branch/var.hpp:259 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_ACTION_SIZE_MAX` | `tbl` | gecode/set/branch/var.hpp:263 | gecode/set/branch/var.hpp:264 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_ACTION_SIZE_MAX` | `tbl` | gecode/set/branch/var.hpp:268 | gecode/set/branch/var.hpp:269 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_CHB_SIZE_MIN` | `tbl` | gecode/set/branch/var.hpp:273 | gecode/set/branch/var.hpp:274 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_CHB_SIZE_MIN` | `tbl` | gecode/set/branch/var.hpp:278 | gecode/set/branch/var.hpp:279 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_CHB_SIZE_MAX` | `tbl` | gecode/set/branch/var.hpp:283 | gecode/set/branch/var.hpp:284 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G5 | `BranchTbl` | `SET_VAR_CHB_SIZE_MAX` | `tbl` | gecode/set/branch/var.hpp:288 | gecode/set/branch/var.hpp:289 | keeps: returns a SetVarBranch built from it; copied once (last use) |
| G6 | `*BranchCommit` | `FloatAssign::FloatAssign` | `c` | gecode/float/branch/assign.hpp:44 | gecode/float/branch/assign.hpp:45 | passes to base or delegated constructor ValBranch<FloatVar>; copied once (last use) |
| G6 | `*BranchCommit` | `FLOAT_ASSIGN` | `c` | gecode/float/branch/assign.hpp:68 | gecode/float/branch/assign.hpp:69 | keeps: returns a FloatAssign built from it; copied once (last use) |
| G6 | `*BranchCommit` | `FloatValBranch::FloatValBranch` | `c` | gecode/float/branch/val.hpp:44 | gecode/float/branch/val.hpp:45 | passes to base or delegated constructor ValBranch<FloatVar>; copied once (last use) |
| G6 | `*BranchCommit` | `FLOAT_VAL` | `c` | gecode/float/branch/val.hpp:68 | gecode/float/branch/val.hpp:69 | keeps: returns a FloatValBranch built from it; copied once (last use) |
| G6 | `*BranchMerit` | `FLOAT_VAR_MERIT_MIN` | `bm` | gecode/float/branch/var.hpp:108 | gecode/float/branch/var.hpp:109 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `*BranchMerit` | `FLOAT_VAR_MERIT_MAX` | `bm` | gecode/float/branch/var.hpp:113 | gecode/float/branch/var.hpp:114 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `*BranchVal` | `FloatAssign::FloatAssign` | `v` | gecode/float/branch/assign.hpp:44 | gecode/float/branch/assign.hpp:45 | passes to base or delegated constructor ValBranch<FloatVar>; copied once (last use) |
| G6 | `*BranchVal` | `FLOAT_ASSIGN` | `v` | gecode/float/branch/assign.hpp:68 | gecode/float/branch/assign.hpp:69 | keeps: returns a FloatAssign built from it; copied once (last use) |
| G6 | `*BranchVal` | `FloatValBranch::FloatValBranch` | `v` | gecode/float/branch/val.hpp:44 | gecode/float/branch/val.hpp:45 | passes to base or delegated constructor ValBranch<FloatVar>; copied once (last use) |
| G6 | `*BranchVal` | `FLOAT_VAL` | `v` | gecode/float/branch/val.hpp:68 | gecode/float/branch/val.hpp:69 | keeps: returns a FloatValBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_MERIT_MIN` | `tbl` | gecode/float/branch/var.hpp:108 | gecode/float/branch/var.hpp:109 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_MERIT_MAX` | `tbl` | gecode/float/branch/var.hpp:113 | gecode/float/branch/var.hpp:114 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_DEGREE_MIN` | `tbl` | gecode/float/branch/var.hpp:123 | gecode/float/branch/var.hpp:124 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_DEGREE_MAX` | `tbl` | gecode/float/branch/var.hpp:128 | gecode/float/branch/var.hpp:129 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_AFC_MIN` | `tbl` | gecode/float/branch/var.hpp:133 | gecode/float/branch/var.hpp:134 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_AFC_MIN` | `tbl` | gecode/float/branch/var.hpp:138 | gecode/float/branch/var.hpp:139 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_AFC_MAX` | `tbl` | gecode/float/branch/var.hpp:143 | gecode/float/branch/var.hpp:144 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_AFC_MAX` | `tbl` | gecode/float/branch/var.hpp:148 | gecode/float/branch/var.hpp:149 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_ACTION_MIN` | `tbl` | gecode/float/branch/var.hpp:153 | gecode/float/branch/var.hpp:154 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_ACTION_MIN` | `tbl` | gecode/float/branch/var.hpp:158 | gecode/float/branch/var.hpp:159 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_ACTION_MAX` | `tbl` | gecode/float/branch/var.hpp:163 | gecode/float/branch/var.hpp:164 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_ACTION_MAX` | `tbl` | gecode/float/branch/var.hpp:168 | gecode/float/branch/var.hpp:169 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_CHB_MIN` | `tbl` | gecode/float/branch/var.hpp:173 | gecode/float/branch/var.hpp:174 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_CHB_MIN` | `tbl` | gecode/float/branch/var.hpp:178 | gecode/float/branch/var.hpp:179 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_CHB_MAX` | `tbl` | gecode/float/branch/var.hpp:183 | gecode/float/branch/var.hpp:184 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_CHB_MAX` | `tbl` | gecode/float/branch/var.hpp:188 | gecode/float/branch/var.hpp:189 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_MIN_MIN` | `tbl` | gecode/float/branch/var.hpp:193 | gecode/float/branch/var.hpp:194 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_MIN_MAX` | `tbl` | gecode/float/branch/var.hpp:198 | gecode/float/branch/var.hpp:199 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_MAX_MIN` | `tbl` | gecode/float/branch/var.hpp:203 | gecode/float/branch/var.hpp:204 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_MAX_MAX` | `tbl` | gecode/float/branch/var.hpp:208 | gecode/float/branch/var.hpp:209 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_SIZE_MIN` | `tbl` | gecode/float/branch/var.hpp:213 | gecode/float/branch/var.hpp:214 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_SIZE_MAX` | `tbl` | gecode/float/branch/var.hpp:218 | gecode/float/branch/var.hpp:219 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_DEGREE_SIZE_MIN` | `tbl` | gecode/float/branch/var.hpp:223 | gecode/float/branch/var.hpp:224 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_DEGREE_SIZE_MAX` | `tbl` | gecode/float/branch/var.hpp:228 | gecode/float/branch/var.hpp:229 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_AFC_SIZE_MIN` | `tbl` | gecode/float/branch/var.hpp:233 | gecode/float/branch/var.hpp:234 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_AFC_SIZE_MIN` | `tbl` | gecode/float/branch/var.hpp:238 | gecode/float/branch/var.hpp:239 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_AFC_SIZE_MAX` | `tbl` | gecode/float/branch/var.hpp:243 | gecode/float/branch/var.hpp:244 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_AFC_SIZE_MAX` | `tbl` | gecode/float/branch/var.hpp:248 | gecode/float/branch/var.hpp:249 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_ACTION_SIZE_MIN` | `tbl` | gecode/float/branch/var.hpp:253 | gecode/float/branch/var.hpp:254 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_ACTION_SIZE_MIN` | `tbl` | gecode/float/branch/var.hpp:258 | gecode/float/branch/var.hpp:259 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_ACTION_SIZE_MAX` | `tbl` | gecode/float/branch/var.hpp:263 | gecode/float/branch/var.hpp:264 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_ACTION_SIZE_MAX` | `tbl` | gecode/float/branch/var.hpp:268 | gecode/float/branch/var.hpp:269 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_CHB_SIZE_MIN` | `tbl` | gecode/float/branch/var.hpp:273 | gecode/float/branch/var.hpp:274 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_CHB_SIZE_MIN` | `tbl` | gecode/float/branch/var.hpp:278 | gecode/float/branch/var.hpp:279 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_CHB_SIZE_MAX` | `tbl` | gecode/float/branch/var.hpp:283 | gecode/float/branch/var.hpp:284 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G6 | `BranchTbl` | `FLOAT_VAR_CHB_SIZE_MAX` | `tbl` | gecode/float/branch/var.hpp:288 | gecode/float/branch/var.hpp:289 | keeps: returns a FloatVarBranch built from it; copied once (last use) |
| G7 | `*BranchFilter` | `assign` | `bf` | gecode/int/branch.cpp:133 | gecode/int/branch.cpp:141 | passes to post_view_value_brancher<IntView, 1, int, 1>; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:145 | gecode/int/branch.cpp:161 | passes to assign, post_view_value_brancher<IntView, 2, int, 1>, post_view_value_brancher<IntView, 3, int, 1>, post_view_value_brancher<IntView, 4, int, 1> |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:190 | gecode/int/branch.cpp:199 | passes to post_view_value_brancher<BoolView, 1, int, 2>; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:204 | gecode/int/branch.cpp:219 | passes to branch, post_view_value_brancher<BoolView, 2, int, 2>, post_view_value_brancher<BoolView, 3, int, 2>, post_view_value_brancher<BoolView, 4, int, 2> |
| G7 | `*BranchFilter` | `assign` | `bf` | gecode/int/branch.cpp:246 | gecode/int/branch.cpp:254 | passes to post_view_value_brancher<BoolView, 1, int, 1>; copied once (last use) |
| G7 | `*BranchFilter` | `assign` | `bf` | gecode/int/branch.cpp:258 | gecode/int/branch.cpp:274 | passes to assign, post_view_value_brancher<BoolView, 2, int, 1>, post_view_value_brancher<BoolView, 3, int, 1>, post_view_value_brancher<BoolView, 4, int, 1> |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:311 | gecode/int/branch.cpp:313 | passes to branch; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:318 | gecode/int/branch.cpp:320 | passes to Int::Branch::post_open_brancher<Int::IntView>; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:325 | gecode/int/branch.cpp:327 | passes to branch; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:332 | gecode/int/branch.cpp:335 | passes to Int::Branch::post_open_brancher<Int::BoolView>; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:38 | gecode/int/branch.cpp:48 | passes to Branch::post_view_values_brancher<1, true>, Branch::post_view_values_brancher<1, false>, post_view_value_brancher<IntView, 1, int, 2> |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.cpp:61 | gecode/int/branch.cpp:77 | passes to branch, Branch::post_view_values_brancher<2, true>, Branch::post_view_values_brancher<2, false>, post_view_value_brancher<IntView, 2, int, 2>, Branch::post_view_values_brancher<3, true>, Branch::post_view_values_brancher<3, false>, post_view_value_brancher<IntView, 3, int, 2>, Branch::post_view_values_brancher<4, true>, Branch::post_view_values_brancher<4, false>, post_view_value_brancher<IntView, 4, int, 2> |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.hpp:36 | gecode/int/branch.hpp:37 | passes to branch; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/branch.hpp:40 | gecode/int/branch.hpp:41 | passes to branch; copied once (last use) |
| G7 | `*BranchFilter` | `assign` | `bf` | gecode/int/branch.hpp:45 | gecode/int/branch.hpp:46 | passes to assign; copied once (last use) |
| G7 | `*BranchFilter` | `assign` | `bf` | gecode/int/branch.hpp:49 | gecode/int/branch.hpp:50 | passes to assign; copied once (last use) |
| G7 | `*BranchFilter` | `post_view_values_brancher` | `bf` | gecode/int/branch/view-values.hpp:175 | gecode/int/branch/view-values.hpp:180 | passes to ViewValuesBrancher<n, min, BrancherFilter<IntView>, BrancherPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherFilter<IntView>, BrancherNoPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherNoFilter<IntView>, BrancherPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherNoFilter<IntView>, BrancherNoPrint<IntView, int>>::post |
| G7 | `*BranchFilter` | `ViewValuesBrancher::ViewValuesBrancher<n, min, Filter, Print>` | `bf` | gecode/int/branch/view-values.hpp:89 | gecode/int/branch/view-values.hpp:91 | passes to base or delegated constructor ViewBrancher<IntView, Filter, n>; copied once (last use) |
| G7 | `*BranchFilter` | `ViewValuesBrancher::post` | `bf` | gecode/int/branch/view-values.hpp:99 | gecode/int/branch/view-values.hpp:100 | passes to ViewValuesBrancher<n, min, Filter, Print>; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/ldsb.cpp:290 | gecode/int/ldsb.cpp:325 | passes to post_ldsb_brancher<IntView, 1, int, 2>; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/ldsb.cpp:331 | gecode/int/ldsb.cpp:346 | passes to branch, post_ldsb_brancher<IntView, 2, int, 2>, post_ldsb_brancher<IntView, 3, int, 2>, post_ldsb_brancher<IntView, 4, int, 2> |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/ldsb.cpp:431 | gecode/int/ldsb.cpp:463 | passes to post_ldsb_brancher<BoolView, 1, int, 2>; copied once (last use) |
| G7 | `*BranchFilter` | `branch` | `bf` | gecode/int/ldsb.cpp:469 | gecode/int/ldsb.cpp:484 | passes to branch, post_ldsb_brancher<BoolView, 2, int, 2>, post_ldsb_brancher<BoolView, 3, int, 2>, post_ldsb_brancher<BoolView, 4, int, 2> |
| G7 | `*VarBranch` | `branch` | `vars` | gecode/int/branch.cpp:311 | gecode/int/branch.cpp:313 | passes to TieBreak<IntVarBranch>; copied once (last use) |
| G7 | `*VarBranch` | `branch` | `vars` | gecode/int/branch.cpp:325 | gecode/int/branch.cpp:327 | passes to TieBreak<BoolVarBranch>; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:126 | gecode/int/branch.cpp:129 | passes to branch; copied once (last use) |
| G7 | `*VarValPrint` | `assign` | `vvp` | gecode/int/branch.cpp:134 | gecode/int/branch.cpp:141 | passes to post_view_value_brancher<IntView, 1, int, 1>; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:146 | gecode/int/branch.cpp:161 | passes to assign, post_view_value_brancher<IntView, 2, int, 1>, post_view_value_brancher<IntView, 3, int, 1>, post_view_value_brancher<IntView, 4, int, 1> |
| G7 | `*VarValPrint` | `assign` | `vvp` | gecode/int/branch.cpp:183 | gecode/int/branch.cpp:186 | passes to assign; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:191 | gecode/int/branch.cpp:199 | passes to post_view_value_brancher<BoolView, 1, int, 2>; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:204 | gecode/int/branch.cpp:219 | passes to branch, post_view_value_brancher<BoolView, 2, int, 2>, post_view_value_brancher<BoolView, 3, int, 2>, post_view_value_brancher<BoolView, 4, int, 2> |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:239 | gecode/int/branch.cpp:242 | passes to branch; copied once (last use) |
| G7 | `*VarValPrint` | `assign` | `vvp` | gecode/int/branch.cpp:247 | gecode/int/branch.cpp:254 | passes to post_view_value_brancher<BoolView, 1, int, 1>; copied once (last use) |
| G7 | `*VarValPrint` | `assign` | `vvp` | gecode/int/branch.cpp:259 | gecode/int/branch.cpp:274 | passes to assign, post_view_value_brancher<BoolView, 2, int, 1>, post_view_value_brancher<BoolView, 3, int, 1>, post_view_value_brancher<BoolView, 4, int, 1> |
| G7 | `*VarValPrint` | `assign` | `vvp` | gecode/int/branch.cpp:294 | gecode/int/branch.cpp:297 | passes to assign; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:312 | gecode/int/branch.cpp:313 | passes to branch; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:318 | gecode/int/branch.cpp:321 | passes to Int::Branch::post_open_brancher<Int::IntView>; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:326 | gecode/int/branch.cpp:327 | passes to branch; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:332 | gecode/int/branch.cpp:335 | passes to Int::Branch::post_open_brancher<Int::BoolView>; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:39 | gecode/int/branch.cpp:48 | passes to Branch::post_view_values_brancher<1, true>, Branch::post_view_values_brancher<1, false>, post_view_value_brancher<IntView, 1, int, 2> |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.cpp:62 | gecode/int/branch.cpp:77 | passes to branch, Branch::post_view_values_brancher<2, true>, Branch::post_view_values_brancher<2, false>, post_view_value_brancher<IntView, 2, int, 2>, Branch::post_view_values_brancher<3, true>, Branch::post_view_values_brancher<3, false>, post_view_value_brancher<IntView, 3, int, 2>, Branch::post_view_values_brancher<4, true>, Branch::post_view_values_brancher<4, false>, post_view_value_brancher<IntView, 4, int, 2> |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.hpp:36 | gecode/int/branch.hpp:37 | passes to branch; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/branch.hpp:40 | gecode/int/branch.hpp:41 | passes to branch; copied once (last use) |
| G7 | `*VarValPrint` | `assign` | `vvp` | gecode/int/branch.hpp:45 | gecode/int/branch.hpp:46 | passes to assign; copied once (last use) |
| G7 | `*VarValPrint` | `assign` | `vvp` | gecode/int/branch.hpp:49 | gecode/int/branch.hpp:50 | passes to assign; copied once (last use) |
| G7 | `*VarValPrint` | `post_view_values_brancher` | `vvp` | gecode/int/branch/view-values.hpp:176 | gecode/int/branch/view-values.hpp:180 | passes to ViewValuesBrancher<n, min, BrancherFilter<IntView>, BrancherPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherFilter<IntView>, BrancherNoPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherNoFilter<IntView>, BrancherPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherNoFilter<IntView>, BrancherNoPrint<IntView, int>>::post |
| G7 | `*VarValPrint` | `ViewValuesBrancher::ViewValuesBrancher<n, min, Filter, Print>` | `vvp` | gecode/int/branch/view-values.hpp:90 | gecode/int/branch/view-values.hpp:91 | keeps: initializes p; copied once (last use) |
| G7 | `*VarValPrint` | `ViewValuesBrancher::post` | `vvp` | gecode/int/branch/view-values.hpp:99 | gecode/int/branch/view-values.hpp:100 | passes to ViewValuesBrancher<n, min, Filter, Print>; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/ldsb.cpp:290 | gecode/int/ldsb.cpp:325 | passes to post_ldsb_brancher<IntView, 1, int, 2>; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/ldsb.cpp:331 | gecode/int/ldsb.cpp:346 | passes to branch, post_ldsb_brancher<IntView, 2, int, 2>, post_ldsb_brancher<IntView, 3, int, 2>, post_ldsb_brancher<IntView, 4, int, 2> |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/ldsb.cpp:431 | gecode/int/ldsb.cpp:463 | passes to post_ldsb_brancher<BoolView, 1, int, 2>; copied once (last use) |
| G7 | `*VarValPrint` | `branch` | `vvp` | gecode/int/ldsb.cpp:469 | gecode/int/ldsb.cpp:484 | passes to branch, post_ldsb_brancher<BoolView, 2, int, 2>, post_ldsb_brancher<BoolView, 3, int, 2>, post_ldsb_brancher<BoolView, 4, int, 2> |
| G7 | `BranchFilter<Var>` | `OpenListBrancher::post` | `bf` | gecode/int/branch/open.hpp:244 | gecode/int/branch/open.hpp:249 | passes to OpenListBrancher; copied once (last use) |
| G7 | `BranchFilter<Var>` | `OpenListBrancher::OpenListBrancher<View, Filter, n>` | `bf` | gecode/int/branch/open.hpp:57 | gecode/int/branch/open.hpp:58 | keeps: initializes filter; copied once (last use) |
| G7 | `BranchFilter<Var>` | `LDSBBrancher::post` | `bf` | gecode/int/ldsb/brancher.hpp:104 | gecode/int/ldsb/brancher.hpp:107 | passes to LDSBBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G7 | `BranchFilter<Var>` | `LDSBBrancher::LDSBBrancher<View, n, Val, a, Filter, Print>` | `bf` | gecode/int/ldsb/brancher.hpp:93 | gecode/int/ldsb/brancher.hpp:95 | passes to base or delegated constructor ViewValBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G7 | `BranchFilter<Var>` | `BrancherFilter::BrancherFilter<View>` | `bf` | gecode/kernel/branch/filter.hpp:92 | gecode/kernel/branch/filter.hpp:93 | keeps: initializes f |
| G7 | `BranchFilter<Var>` | `ViewValBrancher::ViewValBrancher<View, n, Val, a, Filter, Print>` | `bf` | gecode/kernel/branch/view-val.hpp:212 | gecode/kernel/branch/view-val.hpp:213 | passes to base or delegated constructor ViewBrancher<View, Filter, n>; copied once (last use) |
| G7 | `BranchFilter<Var>` | `ViewValBrancher::post` | `bf` | gecode/kernel/branch/view-val.hpp:221 | gecode/kernel/branch/view-val.hpp:224 | passes to ViewValBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G7 | `BranchFilter<Var>` | `ViewBrancher::ViewBrancher<View, Filter, n>` | `bf` | gecode/kernel/branch/view.hpp:140 | gecode/kernel/branch/view.hpp:141 | keeps: initializes f; copied once (last use) |
| G7 | `BranchFilter<typename Gecode::Int::BoolView::VarType>` | `post_open_brancher` | `bf` | gecode/int/branch/open.hpp:308 | gecode/int/branch/open.hpp:311 | passes to OpenListBrancher<View, BrancherFilter<View>, n>::post, OpenListBrancher<View, BrancherNoFilter<View>, n>::post |
| G7 | `BranchFilter<typename Gecode::Int::BoolView::VarType>` | `post_open_brancher` | `bf` | gecode/int/branch/open.hpp:321 | gecode/int/branch/open.hpp:339 | passes to post_open_brancher<View, 1>, post_open_brancher<View, 2>, post_open_brancher<View, 3>, post_open_brancher<View, 4> |
| G7 | `BranchFilter<typename Gecode::Int::BoolView::VarType>` | `post_ldsb_brancher` | `bf` | gecode/int/ldsb/brancher.hpp:252 | gecode/int/ldsb/brancher.hpp:257 | passes to LDSBBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post |
| G7 | `BranchFilter<typename Gecode::Int::BoolView::VarType>` | `post_view_value_brancher` | `bf` | gecode/kernel/branch/view-val.hpp:300 | gecode/kernel/branch/view-val.hpp:304 | passes to ViewValBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post |
| G7 | `VarValPrint<Var, int>` | `OpenListBrancher::post` | `vvp` | gecode/int/branch/open.hpp:244 | gecode/int/branch/open.hpp:249 | passes to OpenListBrancher; copied once (last use) |
| G7 | `VarValPrint<Var, int>` | `OpenListBrancher::OpenListBrancher<View, Filter, n>` | `vvp` | gecode/int/branch/open.hpp:57 | gecode/int/branch/open.hpp:58 | keeps: initializes printer |
| G7 | `VarValPrint<Var, int>` | `LDSBBrancher::post` | `vvp` | gecode/int/ldsb/brancher.hpp:105 | gecode/int/ldsb/brancher.hpp:107 | passes to LDSBBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G7 | `VarValPrint<Var, int>` | `LDSBBrancher::LDSBBrancher<View, n, Val, a, Filter, Print>` | `vvp` | gecode/int/ldsb/brancher.hpp:94 | gecode/int/ldsb/brancher.hpp:95 | passes to base or delegated constructor ViewValBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G7 | `VarValPrint<Var, int>` | `BrancherPrint::BrancherPrint<View, Val>` | `vvp` | gecode/kernel/branch/print.hpp:95 | gecode/kernel/branch/print.hpp:96 | keeps: initializes p |
| G7 | `VarValPrint<Var, int>` | `ViewValBrancher::ViewValBrancher<View, n, Val, a, Filter, Print>` | `vvp` | gecode/kernel/branch/view-val.hpp:212 | gecode/kernel/branch/view-val.hpp:213 | keeps: initializes p; copied once (last use) |
| G7 | `VarValPrint<Var, int>` | `ViewValBrancher::post` | `vvp` | gecode/kernel/branch/view-val.hpp:222 | gecode/kernel/branch/view-val.hpp:224 | passes to ViewValBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G7 | `VarValPrint<typename Gecode::Int::BoolView::VarType, int>` | `post_open_brancher` | `vvp` | gecode/int/branch/open.hpp:309 | gecode/int/branch/open.hpp:312 | passes to OpenListBrancher<View, BrancherFilter<View>, n>::post, OpenListBrancher<View, BrancherNoFilter<View>, n>::post |
| G7 | `VarValPrint<typename Gecode::Int::BoolView::VarType, int>` | `post_open_brancher` | `vvp` | gecode/int/branch/open.hpp:321 | gecode/int/branch/open.hpp:339 | passes to post_open_brancher<View, 1>, post_open_brancher<View, 2>, post_open_brancher<View, 3>, post_open_brancher<View, 4> |
| G7 | `VarValPrint<typename Gecode::Int::BoolView::VarType, int>` | `post_ldsb_brancher` | `vvp` | gecode/int/ldsb/brancher.hpp:253 | gecode/int/ldsb/brancher.hpp:257 | passes to LDSBBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post |
| G7 | `VarValPrint<typename Gecode::Int::BoolView::VarType, int>` | `post_view_value_brancher` | `vvp` | gecode/kernel/branch/view-val.hpp:300 | gecode/kernel/branch/view-val.hpp:304 | passes to ViewValBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post |
| G8 | `*BranchFilter` | `assign` | `bf` | gecode/float/branch.cpp:112 | gecode/float/branch.cpp:127 | passes to assign, post_view_value_brancher<FloatView, 2, FloatNumBranch, 1>, post_view_value_brancher<FloatView, 3, FloatNumBranch, 1>, post_view_value_brancher<FloatView, 4, FloatNumBranch, 1> |
| G8 | `*BranchFilter` | `branch` | `bf` | gecode/float/branch.cpp:38 | gecode/float/branch.cpp:47 | passes to post_view_value_brancher<FloatView, 1, FloatNumBranch, 2>; copied once (last use) |
| G8 | `*BranchFilter` | `branch` | `bf` | gecode/float/branch.cpp:52 | gecode/float/branch.cpp:67 | passes to branch, post_view_value_brancher<FloatView, 2, FloatNumBranch, 2>, post_view_value_brancher<FloatView, 3, FloatNumBranch, 2>, post_view_value_brancher<FloatView, 4, FloatNumBranch, 2> |
| G8 | `*BranchFilter` | `assign` | `bf` | gecode/float/branch.cpp:99 | gecode/float/branch.cpp:107 | passes to post_view_value_brancher<FloatView, 1, FloatNumBranch, 1>; copied once (last use) |
| G8 | `*BranchFilter` | `branch` | `bf` | gecode/float/branch.hpp:36 | gecode/float/branch.hpp:37 | passes to branch; copied once (last use) |
| G8 | `*BranchFilter` | `assign` | `bf` | gecode/float/branch.hpp:41 | gecode/float/branch.hpp:42 | passes to assign; copied once (last use) |
| G8 | `*BranchFilter` | `assign` | `bf` | gecode/set/branch.cpp:106 | gecode/set/branch.cpp:122 | passes to assign, post_view_value_brancher<SetView, 2, int, 1>, post_view_value_brancher<SetView, 3, int, 1>, post_view_value_brancher<SetView, 4, int, 1> |
| G8 | `*BranchFilter` | `branch` | `bf` | gecode/set/branch.cpp:38 | gecode/set/branch.cpp:47 | passes to post_view_value_brancher<SetView, 1, int, 2>; copied once (last use) |
| G8 | `*BranchFilter` | `branch` | `bf` | gecode/set/branch.cpp:51 | gecode/set/branch.cpp:67 | passes to branch, post_view_value_brancher<SetView, 2, int, 2>, post_view_value_brancher<SetView, 3, int, 2>, post_view_value_brancher<SetView, 4, int, 2> |
| G8 | `*BranchFilter` | `assign` | `bf` | gecode/set/branch.cpp:94 | gecode/set/branch.cpp:102 | passes to post_view_value_brancher<SetView, 1, int, 1>; copied once (last use) |
| G8 | `*BranchFilter` | `branch` | `bf` | gecode/set/branch.hpp:36 | gecode/set/branch.hpp:37 | passes to branch; copied once (last use) |
| G8 | `*BranchFilter` | `assign` | `bf` | gecode/set/branch.hpp:41 | gecode/set/branch.hpp:42 | passes to assign; copied once (last use) |
| G8 | `*BranchFilter` | `branch` | `bf` | gecode/set/ldsb.cpp:153 | gecode/set/ldsb.cpp:173 | passes to post_ldsb_set_brancher<SetView, 1, int, 2>; copied once (last use) |
| G8 | `*BranchFilter` | `branch` | `bf` | gecode/set/ldsb.cpp:178 | gecode/set/ldsb.cpp:193 | passes to branch, post_ldsb_set_brancher<SetView, 2, int, 2>, post_ldsb_set_brancher<SetView, 3, int, 2>, post_ldsb_set_brancher<SetView, 4, int, 2> |
| G8 | `*VarValPrint` | `assign` | `vvp` | gecode/float/branch.cpp:100 | gecode/float/branch.cpp:107 | passes to post_view_value_brancher<FloatView, 1, FloatNumBranch, 1>; copied once (last use) |
| G8 | `*VarValPrint` | `assign` | `vvp` | gecode/float/branch.cpp:112 | gecode/float/branch.cpp:127 | passes to assign, post_view_value_brancher<FloatView, 2, FloatNumBranch, 1>, post_view_value_brancher<FloatView, 3, FloatNumBranch, 1>, post_view_value_brancher<FloatView, 4, FloatNumBranch, 1> |
| G8 | `*VarValPrint` | `assign` | `vvp` | gecode/float/branch.cpp:152 | gecode/float/branch.cpp:155 | passes to assign; copied once (last use) |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/float/branch.cpp:39 | gecode/float/branch.cpp:47 | passes to post_view_value_brancher<FloatView, 1, FloatNumBranch, 2>; copied once (last use) |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/float/branch.cpp:52 | gecode/float/branch.cpp:67 | passes to branch, post_view_value_brancher<FloatView, 2, FloatNumBranch, 2>, post_view_value_brancher<FloatView, 3, FloatNumBranch, 2>, post_view_value_brancher<FloatView, 4, FloatNumBranch, 2> |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/float/branch.cpp:92 | gecode/float/branch.cpp:95 | passes to branch; copied once (last use) |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/float/branch.hpp:36 | gecode/float/branch.hpp:37 | passes to branch; copied once (last use) |
| G8 | `*VarValPrint` | `assign` | `vvp` | gecode/float/branch.hpp:41 | gecode/float/branch.hpp:42 | passes to assign; copied once (last use) |
| G8 | `*VarValPrint` | `assign` | `vvp` | gecode/set/branch.cpp:107 | gecode/set/branch.cpp:122 | passes to assign, post_view_value_brancher<SetView, 2, int, 1>, post_view_value_brancher<SetView, 3, int, 1>, post_view_value_brancher<SetView, 4, int, 1> |
| G8 | `*VarValPrint` | `assign` | `vvp` | gecode/set/branch.cpp:142 | gecode/set/branch.cpp:145 | passes to assign; copied once (last use) |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/set/branch.cpp:39 | gecode/set/branch.cpp:47 | passes to post_view_value_brancher<SetView, 1, int, 2>; copied once (last use) |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/set/branch.cpp:52 | gecode/set/branch.cpp:67 | passes to branch, post_view_value_brancher<SetView, 2, int, 2>, post_view_value_brancher<SetView, 3, int, 2>, post_view_value_brancher<SetView, 4, int, 2> |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/set/branch.cpp:87 | gecode/set/branch.cpp:90 | passes to branch; copied once (last use) |
| G8 | `*VarValPrint` | `assign` | `vvp` | gecode/set/branch.cpp:95 | gecode/set/branch.cpp:102 | passes to post_view_value_brancher<SetView, 1, int, 1>; copied once (last use) |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/set/branch.hpp:36 | gecode/set/branch.hpp:37 | passes to branch; copied once (last use) |
| G8 | `*VarValPrint` | `assign` | `vvp` | gecode/set/branch.hpp:41 | gecode/set/branch.hpp:42 | passes to assign; copied once (last use) |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/set/ldsb.cpp:153 | gecode/set/ldsb.cpp:173 | passes to post_ldsb_set_brancher<SetView, 1, int, 2>; copied once (last use) |
| G8 | `*VarValPrint` | `branch` | `vvp` | gecode/set/ldsb.cpp:178 | gecode/set/ldsb.cpp:193 | passes to branch, post_ldsb_set_brancher<SetView, 2, int, 2>, post_ldsb_set_brancher<SetView, 3, int, 2>, post_ldsb_set_brancher<SetView, 4, int, 2> |
| G8 | `BranchFilter<Var>` | `LDSBSetBrancher::post` | `bf` | gecode/set/ldsb/brancher.hpp:232 | gecode/set/ldsb/brancher.hpp:234 | passes to LDSBSetBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G8 | `BranchFilter<Var>` | `LDSBSetBrancher::LDSBSetBrancher<View, n, Val, a, Filter, Print>` | `bf` | gecode/set/ldsb/brancher.hpp:38 | gecode/set/ldsb/brancher.hpp:40 | passes to base or delegated constructor LDSBBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G8 | `BranchFilter<typename Gecode::Set::SetView::VarType>` | `post_ldsb_set_brancher` | `bf` | gecode/set/ldsb/brancher.hpp:241 | gecode/set/ldsb/brancher.hpp:246 | passes to LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post |
| G8 | `VarValPrint<Var, int>` | `LDSBSetBrancher::post` | `vvp` | gecode/set/ldsb/brancher.hpp:233 | gecode/set/ldsb/brancher.hpp:235 | passes to LDSBSetBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G8 | `VarValPrint<Var, int>` | `LDSBSetBrancher::LDSBSetBrancher<View, n, Val, a, Filter, Print>` | `vvp` | gecode/set/ldsb/brancher.hpp:39 | gecode/set/ldsb/brancher.hpp:40 | passes to base or delegated constructor LDSBBrancher<View, n, Val, a, Filter, Print>; copied once (last use) |
| G8 | `VarValPrint<typename Gecode::Set::SetView::VarType, int>` | `post_ldsb_set_brancher` | `vvp` | gecode/set/ldsb/brancher.hpp:242 | gecode/set/ldsb/brancher.hpp:246 | passes to LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post |
| G10 | `std::function<void (Space &)>` | `wait` | `c` | gecode/float/exec.cpp:38 | gecode/float/exec.cpp:40 | passes to UnaryWait<Float::FloatView>::post |
| G10 | `std::function<void (Space &)>` | `wait` | `c` | gecode/float/exec.cpp:44 | gecode/float/exec.cpp:47 | passes to NaryWait<Float::FloatView>::post |
| G10 | `std::function<void (Space &)>` | `wait` | `c` | gecode/int/exec.cpp:38 | gecode/int/exec.cpp:40 | passes to UnaryWait<Int::IntView>::post |
| G10 | `std::function<void (Space &)>` | `wait` | `c` | gecode/int/exec.cpp:44 | gecode/int/exec.cpp:46 | passes to UnaryWait<Int::BoolView>::post |
| G10 | `std::function<void (Space &)>` | `wait` | `c` | gecode/int/exec.cpp:50 | gecode/int/exec.cpp:53 | passes to NaryWait<Int::IntView>::post |
| G10 | `std::function<void (Space &)>` | `wait` | `c` | gecode/int/exec.cpp:57 | gecode/int/exec.cpp:60 | passes to NaryWait<Int::BoolView>::post |
| G10 | `std::function<void (Space &)>` | `when` | `t` | gecode/int/exec.cpp:64 | gecode/int/exec.cpp:66 | passes to Int::Exec::When::post |
| G10 | `std::function<void (Space &)>` | `when` | `e` | gecode/int/exec.cpp:64 | gecode/int/exec.cpp:66 | passes to Int::Exec::When::post |
| G10 | `std::function<void (Space &)>` | `when` | `t` | gecode/int/exec.cpp:70 | gecode/int/exec.cpp:71 | passes to when; copied once (last use) |
| G10 | `std::function<void (Space &)>` | `When::When` | `t0` | gecode/int/exec/when.hpp:36 | gecode/int/exec/when.hpp:37 | keeps: initializes t |
| G10 | `std::function<void (Space &)>` | `When::When` | `e0` | gecode/int/exec/when.hpp:36 | gecode/int/exec/when.hpp:37 | keeps: initializes e |
| G10 | `std::function<void (Space &)>` | `When::post` | `t` | gecode/int/exec/when.hpp:46 | gecode/int/exec/when.hpp:58 | passes to When |
| G10 | `std::function<void (Space &)>` | `When::post` | `e` | gecode/int/exec/when.hpp:46 | gecode/int/exec/when.hpp:58 | passes to When |
| G10 | `std::function<void (Space &)>` | `FunctionBranch::post` | `f` | gecode/kernel/branch/function.cpp:125 | gecode/kernel/branch/function.cpp:128 | passes to FunctionBranch |
| G10 | `std::function<void (Space &)>` | `branch` | `f` | gecode/kernel/branch/function.cpp:143 | gecode/kernel/branch/function.cpp:144 | passes to Kernel::FunctionBranch::post; copied once (last use) |
| G10 | `std::function<void (Space &)>` | `FunctionBranch::FunctionBranch` | `f0` | gecode/kernel/branch/function.cpp:87 | gecode/kernel/branch/function.cpp:88 | keeps: initializes f |
| G10 | `std::function<void (Space &)>` | `UnaryWait::UnaryWait<View>` | `c0` | gecode/kernel/propagator/wait.hpp:107 | gecode/kernel/propagator/wait.hpp:108 | keeps: initializes c |
| G10 | `std::function<void (Space &)>` | `UnaryWait::post` | `c` | gecode/kernel/propagator/wait.hpp:143 | gecode/kernel/propagator/wait.hpp:150 | passes to UnaryWait<View> |
| G10 | `std::function<void (Space &)>` | `NaryWait::NaryWait<View>` | `c0` | gecode/kernel/propagator/wait.hpp:170 | gecode/kernel/propagator/wait.hpp:171 | keeps: initializes c |
| G10 | `std::function<void (Space &)>` | `NaryWait::post` | `c` | gecode/kernel/propagator/wait.hpp:225 | gecode/kernel/propagator/wait.hpp:237 | passes to UnaryWait<View>::post, NaryWait<View> |
| G10 | `std::function<void (Space &)>` | `wait` | `c` | gecode/set/exec.cpp:38 | gecode/set/exec.cpp:40 | passes to UnaryWait<Set::SetView>::post |
| G10 | `std::function<void (Space &)>` | `wait` | `c` | gecode/set/exec.cpp:44 | gecode/set/exec.cpp:47 | passes to NaryWait<Set::SetView>::post |
| G11 | `Domain` | `OpenIntVarList::factory` | `d` | gecode/int/open-var-list.cpp:40 | gecode/int/open-var-list.cpp:43 | keeps: lambda capture |
| G11 | `Domain` | `OpenIntVarList::OpenIntVarList` | `d` | gecode/int/open-var-list.cpp:55 | gecode/int/open-var-list.cpp:56 | passes to factory; copied once (last use) |
| G11 | `Domain` | `OpenIntVarList::OpenIntVarList` | `d` | gecode/int/open-var-list.cpp:58 | gecode/int/open-var-list.cpp:59 | passes to factory; copied once (last use) |
| G11 | `Transition` | `OpenIntVarList::OpenIntVarList` | `t` | gecode/int/open-var-list.cpp:52 | gecode/int/open-var-list.cpp:53 | passes to base or delegated constructor OpenIntVarList; copied once (last use) |
| G11 | `Transition` | `OpenIntVarList::OpenIntVarList` | `t` | gecode/int/open-var-list.cpp:58 | gecode/int/open-var-list.cpp:59 | passes to base or delegated constructor OpenVarList<IntVar>; copied once (last use) |
| G11 | `Transition` | `OpenBoolVarList::OpenBoolVarList` | `t` | gecode/int/open-var-list.cpp:66 | gecode/int/open-var-list.cpp:67 | passes to base or delegated constructor OpenVarList<BoolVar>; copied once (last use) |
| G13 | `*ValBranch` | `AuxVarBrancher::AuxVarBrancher` | `int_valsel0` | gecode/flatzinc/flatzinc.cpp:113 | gecode/flatzinc/flatzinc.cpp:124 | keeps: initializes int_valsel; copied once (last use) |
| G13 | `*ValBranch` | `AuxVarBrancher::AuxVarBrancher` | `bool_valsel0` | gecode/flatzinc/flatzinc.cpp:114 | gecode/flatzinc/flatzinc.cpp:125 | keeps: initializes bool_valsel; copied once (last use) |
| G13 | `*ValBranch` | `AuxVarBrancher::AuxVarBrancher` | `set_valsel0` | gecode/flatzinc/flatzinc.cpp:117 | gecode/flatzinc/flatzinc.cpp:128 | keeps: initializes set_valsel; copied once (last use) |
| G13 | `*ValBranch` | `AuxVarBrancher::AuxVarBrancher` | `float_valsel0` | gecode/flatzinc/flatzinc.cpp:121 | gecode/flatzinc/flatzinc.cpp:132 | keeps: initializes float_valsel; copied once (last use) |
| G13 | `*ValBranch` | `AuxVarBrancher::post` | `int_valsel` | gecode/flatzinc/flatzinc.cpp:246 | gecode/flatzinc/flatzinc.cpp:257 | passes to AuxVarBrancher; copied once (last use) |
| G13 | `*ValBranch` | `AuxVarBrancher::post` | `bool_valsel` | gecode/flatzinc/flatzinc.cpp:247 | gecode/flatzinc/flatzinc.cpp:258 | passes to AuxVarBrancher; copied once (last use) |
| G13 | `*ValBranch` | `AuxVarBrancher::post` | `set_valsel` | gecode/flatzinc/flatzinc.cpp:250 | gecode/flatzinc/flatzinc.cpp:261 | passes to AuxVarBrancher; copied once (last use) |
| G13 | `*ValBranch` | `AuxVarBrancher::post` | `float_valsel` | gecode/flatzinc/flatzinc.cpp:254 | gecode/flatzinc/flatzinc.cpp:265 | passes to AuxVarBrancher; copied once (last use) |
| G13 | `*VarBranch` | `AuxVarBrancher::AuxVarBrancher` | `set_varsel0` | gecode/flatzinc/flatzinc.cpp:117 | gecode/flatzinc/flatzinc.cpp:128 | keeps: initializes set_varsel; copied once (last use) |
| G13 | `*VarBranch` | `AuxVarBrancher::post` | `set_varsel` | gecode/flatzinc/flatzinc.cpp:250 | gecode/flatzinc/flatzinc.cpp:261 | passes to AuxVarBrancher; copied once (last use) |
| G13 | `TieBreak<*VarBranch>` | `AuxVarBrancher::AuxVarBrancher` | `int_varsel0` | gecode/flatzinc/flatzinc.cpp:113 | gecode/flatzinc/flatzinc.cpp:124 | keeps: initializes int_varsel; copied once (last use) |
| G13 | `TieBreak<*VarBranch>` | `AuxVarBrancher::AuxVarBrancher` | `bool_varsel0` | gecode/flatzinc/flatzinc.cpp:114 | gecode/flatzinc/flatzinc.cpp:125 | keeps: initializes bool_varsel; copied once (last use) |
| G13 | `TieBreak<*VarBranch>` | `AuxVarBrancher::AuxVarBrancher` | `float_varsel0` | gecode/flatzinc/flatzinc.cpp:121 | gecode/flatzinc/flatzinc.cpp:132 | keeps: initializes float_varsel; copied once (last use) |
| G13 | `TieBreak<*VarBranch>` | `AuxVarBrancher::post` | `int_varsel` | gecode/flatzinc/flatzinc.cpp:246 | gecode/flatzinc/flatzinc.cpp:257 | passes to AuxVarBrancher; copied once (last use) |
| G13 | `TieBreak<*VarBranch>` | `AuxVarBrancher::post` | `bool_varsel` | gecode/flatzinc/flatzinc.cpp:246 | gecode/flatzinc/flatzinc.cpp:257 | passes to AuxVarBrancher; copied once (last use) |
| G13 | `TieBreak<*VarBranch>` | `AuxVarBrancher::post` | `float_varsel` | gecode/flatzinc/flatzinc.cpp:254 | gecode/flatzinc/flatzinc.cpp:265 | passes to AuxVarBrancher; copied once (last use) |
| G13 | `std::exception_ptr` | `BlackBoxContextHandle::fail` | `e` | gecode/flatzinc/blackbox-propagator.cpp:107 | gecode/flatzinc/blackbox-propagator.cpp:108 | passes to fail; copied once (last use) |
| G13 | `std::exception_ptr` | `BlackBoxContext::fail` | `e` | gecode/flatzinc/blackbox-propagator.cpp:144 | gecode/flatzinc/blackbox-propagator.cpp:147 | keeps: assigns exception; copied once (last use) |
| G13 | `std::string` | `ParserState::output` | `x` | gecode/flatzinc/parser.hh:258 | gecode/flatzinc/parser.hh:259 | keeps: stores via output_items.push_back |
| G13 | `std::vector<BI>` | `BranchInformationO::BranchInformationO` | `v0` | gecode/flatzinc/flatzinc.cpp:287 | gecode/flatzinc/flatzinc.cpp:287 | keeps: initializes v; copied once (last use) |
| G14 | `std::string` | `CPProfilerSearchTracer::CPProfilerSearchTracer` | `name0` | gecode/search/cpprofiler/tracer.cpp:54 | gecode/search/cpprofiler/tracer.cpp:56 | keeps: initializes name; copied once (last use) |
| G15 | `BeforeRun` | `FlatZincTest::FlatZincTest` | `before` | test/flatzinc.cpp:104 | test/flatzinc.cpp:106 | passes to base or delegated constructor FlatZincTest; copied once (last use) |
| G15 | `BeforeRun` | `FlatZincTest::FlatZincTest` | `before` | test/flatzinc.cpp:110 | test/flatzinc.cpp:113 | keeps: initializes _before; copied once (last use) |
| G15 | `Gecode::*ValBranch / Gecode::*ValBranch` | `NoGoods::NoGoods<Model, ValBranch>` | `vb0` | test/nogoods.cpp:250 | test/nogoods.cpp:254 | keeps: initializes vb; passes to Model::val |
| G15 | `Gecode::OpenIntVarList::Domain` | `CallbackSpace::CallbackSpace` | `domain` | test/int/open-var-list.cpp:1023 | test/int/open-var-list.cpp:1024 | keeps: initializes list; copied once (last use) |
| G15 | `Gecode::OpenIntVarList::Transition` | `CallbackSpace::CallbackSpace` | `transition` | test/int/open-var-list.cpp:1023 | test/int/open-var-list.cpp:1024 | keeps: initializes list; copied once (last use) |
| G15 | `OutputCheck` | `FlatZincTest::FlatZincTest` | `check` | test/flatzinc.cpp:103 | test/flatzinc.cpp:106 | passes to base or delegated constructor FlatZincTest; copied once (last use) |
| G15 | `OutputCheck` | `FlatZincTest::FlatZincTest` | `check` | test/flatzinc.cpp:110 | test/flatzinc.cpp:112 | keeps: initializes _check; copied once (last use) |
| G15 | `std::function<bool ()>` | `Case::Case` | `f0` | test/instance.cpp:118 | test/instance.cpp:118 | keeps: initializes f; copied once (last use) |
| G15 | `std::function<int (int, int)>` | `Model::Model` | `distance` | test/int/min-distance.cpp:114 | test/int/min-distance.cpp:116 | passes to Gecode::IntDistance; copied once (last use) |
| G15 | `std::shared_ptr<const Graph>` | `TreeSpace::TreeSpace` | `g` | test/search.cpp:137 | test/search.cpp:138 | keeps: initializes graph; copied once (last use) |
| G15 | `std::string` | `FlatZincErrorTest::FlatZincErrorTest` | `expected_message` | test/flatzinc.cpp:116 | test/flatzinc.cpp:117 | passes to base or delegated constructor FlatZincErrorTest; copied once (last use) |
| G15 | `std::string` | `FlatZincErrorTest::FlatZincErrorTest` | `expected_message` | test/flatzinc.cpp:120 | test/flatzinc.cpp:122 | keeps: initializes _expected_message; copied once (last use) |
| G15 | `std::string` | `Base::Base` | `s` | test/test.cpp:122 | test/test.cpp:122 | passes to base or delegated constructor Base; copied once (last use) |
| G15 | `std::vector<std::string>` | `FlatZincTest::FlatZincTest` | `cmdline_opt` | test/flatzinc.cpp:103 | test/flatzinc.cpp:105 | passes to base or delegated constructor FlatZincTest; copied once (last use) |
| G15 | `std::vector<std::string>` | `FlatZincTest::FlatZincTest` | `cmdline_opt` | test/flatzinc.cpp:109 | test/flatzinc.cpp:112 | keeps: initializes _cmdline_opt; copied once (last use) |
| G15 | `std::vector<std::string>` | `FlatZincErrorTest::FlatZincErrorTest` | `cmdline_opt` | test/flatzinc.cpp:116 | test/flatzinc.cpp:117 | passes to base or delegated constructor FlatZincErrorTest; copied once (last use) |
| G15 | `std::vector<std::string>` | `FlatZincErrorTest::FlatZincErrorTest` | `cmdline_opt` | test/flatzinc.cpp:120 | test/flatzinc.cpp:121 | passes to base or delegated constructor FlatZincTest; copied once (last use) |
