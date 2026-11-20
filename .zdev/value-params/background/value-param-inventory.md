# performance-unnecessary-value-param inventory

Measured on `feature/breaking-7` at `f91fbadafc90bf93241389ccf2fa6a7d3736bc81`
(2026-11-01) with Homebrew LLVM 22.1.8 (`clang-tidy`, `run-clang-tidy`, and
the libclang Python bindings from `/opt/homebrew/opt/llvm@22`). `.clang-tidy`
and the sources were not changed. This file is self-contained. The
[appendix](#appendix-scripts) holds the complete scripts, and
[Reproduce](#reproduce) gives the commands that rebuild every count and table
below.

## Coverage

- Compilation database: `build/full/compile_commands.json`, a Debug build
  configured as in [Reproduce](#reproduce) (experimental, CBS, fault injection,
  audit, and Gist enabled). It has 491 entries for 490 distinct files, because
  `test/test-main.cpp` is compiled twice. The entries are:
  - 486 owned sources that are linted as whole translation units. These
    include the 15 owned Gist translation units (14 in `gecode/gist/` and
    `gecode/gist/standalone-example/mygist.cpp`) and the CBS, AStar, and CABS
    sources.
  - `gecode/flatzinc/parser.tab.cpp` and `gecode/flatzinc/lexer.yy.cpp`. Only
    their grammar action ranges are linted.
  - Two generated moc translation units,
    `build/full/gecodegist_shared_autogen/mocs_compilation.cpp` and
    `build/full/gecode/flatzinc/exampleplugin/myplugin_autogen/mocs_compilation.cpp`.
    They are not owned, so the source filter skips them, as it does in
    `misc/check.py lint`.
- Source selection is the same as `uv run misc/check.py lint`: owned
  `.c/.cc/.cpp/.cxx` files (git-tracked, minus `gecode/third-party/` and the
  Bison/Flex outputs) that are in the database. Three owned sources are not in
  the database because they are installed-package consumers:
  `test/package/allocator/{boundary,main}.cpp` and
  `test/package/public-test-component/consumer-smoke.cpp`. `check.py all`
  lints them separately. They hold no by-value parameters of owned types.
- The header filter, exclude filter, source filter, and `-UNDEBUG` are copied
  from `misc/check.py` `lint`, and so are its three extra runs:
  - the C++ action ranges of `parser.tab.cpp` that its `#line` directives map
    to `parser.yxx`;
  - the C++ action ranges of `lexer.yy.cpp` that map to `lexer.lxx`;
  - `gecode/flatzinc/parser.hh`, linted with the parser's compile command.

  The lexer-action run reports 0 diagnostics in all three configurations, so
  adding it changed no count.
- The check is selected with `-checks=-*,performance-unnecessary-value-param`.
  `--config` carries the same `Checks` value plus `CheckOptions`, because
  `-checks` cannot set options. `--config` replaces `.clang-tidy`, so no other
  check runs.
- Parameter identity, definitions, and redeclarations come from libclang. The
  translation units that reported the diagnostics are parsed with their
  compile commands plus `-UNDEBUG`. 61 translation units cover all 834
  diagnostics. Uses inside each definition are classified from the source
  text (see [Per-parameter inventory](#per-parameter-inventory)).

## Reproduce

All commands run from the repository root. They need CMake, Ninja, Qt 6
(Gist), Homebrew `llvm@22`, `python3` (3.10 or newer), and `git`.

**Step 1.** Configure and build the compilation database. The build also
generates the Gist moc headers that the Gist sources include.

```sh
cmake -S . -B build/full -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGECODE_ENABLE_EXPERIMENTAL=ON \
  -DGECODE_ENABLE_CBS=ON -DGECODE_ENABLE_FAULT_INJECTION=ON \
  -DGECODE_ENABLE_AUDIT=ON -DGECODE_ENABLE_GIST=ON -DGECODE_ENABLE_QT=ON
cmake --build build/full
```

**Step 2.** Recreate the scripts from the [appendix](#appendix-scripts). Each
`#### <file name>` heading there is followed by one code block that is the
complete file. This command writes them to
`build/zdev-audit/value-params/`:

```sh
python3 - .zdev/value-params/background/value-param-inventory.md build/zdev-audit/value-params <<'EOF'
import pathlib, re, sys
text = pathlib.Path(sys.argv[1]).read_text()
out = pathlib.Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
blocks = re.findall(r"^#### `([\w.-]+)`\n\n```[a-z]*\n(.*?)^```$", text, re.M | re.S)
for name, body in blocks:
    (out / name).write_text(body)
    print(out / name)
EOF
```

It prints the three file names `run.py`, `analyze.py`, and
`reproduce.sh`. The first argument is this file and the second the target
directory. Any directory inside the repository works, since the scripts find
the repository root with `git` and write their output next to themselves.

**Step 3.** Run the three clang-tidy passes and the analysis:

```sh
sh build/zdev-audit/value-params/reproduce.sh build/full 10
```

The arguments are the build directory and the number of jobs. Each pass
takes about three minutes on 10 jobs, and the analysis about two minutes.
The script deletes and recreates `no-allowed/`, `allowed/`, and
`allowed-floatval/` next to itself. `run.py` writes the exact command lines
that ran to `<pass>/commands.json` and the `--config` value to
`<pass>/config.yaml`. The last lines of the output are:

```
no-allowed raw 172584 unique 2016
allowed raw 117975 unique 834
allowed-floatval raw 113879 unique 776
unique parameters 708 fix-its 704 with other declarations 610 checks 0
```

`analyze.py` writes the tables of this document: `counts.md` (the
[Counts](#counts) table and totals), `groups.md` (the file lists in
[Repair groups](#repair-groups)), `hooks.md`
([Verification hooks](#verification-hooks)), and `rows.md`
([Per-parameter inventory](#per-parameter-inventory)). It also writes
`rows.tsv`, which has every computed column, and `checks.txt`. `checks.txt`
lists any fix-it declaration that libclang did not find, any virtual function
without a coupling entry, and any row without a group. It is empty for this
run.

The passes (`reproduce.sh`):

| Pass | AllowedTypes | Purpose |
|---|---|---|
| `no-allowed` | none | Baseline: what the check reports with no exceptions |
| `allowed` | the proposed value below | The inventory. All rows come from this pass |
| `allowed-floatval` | the proposed value plus `^Gecode::FloatVal$` | Measures the [FloatVal exception](#policy-exception-for-the-user-floatval) |

The `--config` of the `allowed` pass:

```json
{"Checks": "-*,performance-unnecessary-value-param", "CheckOptions": {"performance-unnecessary-value-param.AllowedTypes": "^Gecode::(Home|Group|PropagatorGroup|BrancherGroup|Pos|Reify)$;^Gecode::(Int|Bool|Set|Float)Var$;^Gecode::(Int|Bool|Set|Float)VarArray$;^Gecode::(VarArray|ViewArray)(<.*>)?$;^Gecode::(OpenIntVarList|OpenBoolVarList|OpenVarList|OpenViewList)(<.*>)?$;^Gecode::(VarImpView|DerivedView|ConstView)(<.*>)?$;^Gecode::(Int|Set|Float)::[A-Za-z]*View(<.*>)?$;^Gecode::.*::VarType$"}}
```

`no-allowed` uses `"CheckOptions": {}`, and `allowed-floatval` appends
`;^Gecode::FloatVal$` to the value.

## Proposed AllowedTypes

The value for `.clang-tidy`. It follows the brief's definition of a shallow
handle: a pointer-sized copy with no reference count or allocation. In YAML,
quote the value with single quotes, since it contains `$`, `|`, and `;`:

```
^Gecode::(Home|Group|PropagatorGroup|BrancherGroup|Pos|Reify)$;^Gecode::(Int|Bool|Set|Float)Var$;^Gecode::(Int|Bool|Set|Float)VarArray$;^Gecode::(VarArray|ViewArray)(<.*>)?$;^Gecode::(OpenIntVarList|OpenBoolVarList|OpenVarList|OpenViewList)(<.*>)?$;^Gecode::(VarImpView|DerivedView|ConstView)(<.*>)?$;^Gecode::(Int|Set|Float)::[A-Za-z]*View(<.*>)?$;^Gecode::.*::VarType$
```

How clang-tidy 22 matches: an entry that contains `::` is matched against the
qualified name of the type's declaration, and other entries against the bare
name. `^Gecode::Home$`, `^::Gecode::Home$`, and `^Home$` all suppress `Home`
(checked on `examples/fifteen-puzzle.cpp`). Every entry is anchored and
qualified with `Gecode::`. A dependent `typename View::VarType` is matched by
the typedef's own qualified name, not by the type it names. So
`^Gecode::Int::(Int|Bool)View::VarType$` does not suppress
`gecode/int/open-var-list.hpp:396`, but `^Gecode::.*::VarType$` does (checked
on `gecode/int/distinct.cpp`).

The check only reports types that are not trivially copyable. Most views are
trivially copyable, so they are not reported at all. What remains are handles
with a user-declared copy constructor. Each entry below was checked against
that copy constructor. The last column counts unique diagnostics of the
`no-allowed` pass that the entry suppresses.

| Entry | Types | Why the copy is shallow | Suppressed unique diagnostics |
|---|---|---|---:|
| `^Gecode::(Home\|…)$` | `Home` | `Home(const Home&)` copies `Space&`, `Propagator*`, and two group ids (`gecode/kernel/core.hpp:3485`). No reference count or allocation | 450 |
| | `Group`, `PropagatorGroup`, `BrancherGroup` | Copy one `unsigned int gid` (`gecode/kernel/core.hpp:5276`, `5298`, `5335`) | 2 + 21 + 15 |
| | `Pos` | One `const int pos` (`gecode/kernel/branch/view.hpp:43`) | 1 |
| | `Reify` | A `BoolVar` and a `ReifyMode` (`gecode/int/reify.hpp:44`) | 57 |
| `^Gecode::(Int\|Bool\|Set\|Float)Var$` | variables | `IntVar(const IntVar& y) : VarImpVar(y.varimp())` copies one `VarImp*` (`gecode/int/var/int.hpp:48`). Same for `bool.hpp:49`, `set.hpp:48`, and `float.hpp:45` | 309 + 85 + 91 + 80 |
| `^Gecode::(…)VarArray$` and `^Gecode::(VarArray\|ViewArray)(<.*>)?$` | `IntVarArray`, `BoolVarArray`, `SetVarArray`, `FloatVarArray`, `VarArray<>`, `ViewArray<>` | Copy `n` and the space-allocated pointer `x` (`gecode/kernel/data/array.hpp:902`, `1123`). Not to be confused with the `*Args` arrays, which copy heap storage and stay reportable | 2 + 1 + 1 + 6 |
| `^Gecode::(Open…List)(<.*>)?$` | `OpenIntVarList`, `OpenBoolVarList`, `OpenVarList<>`, `OpenViewList<>` | `LocalHandle(const LocalHandle&) : o(lh.o)` copies one pointer into space-local data (`gecode/kernel/core.hpp:4002`) | 24 + 4 + 33 |
| `^Gecode::(VarImpView\|DerivedView\|ConstView)(<.*>)?$` and `^Gecode::(Int\|Set\|Float)::[A-Za-z]*View(<.*>)?$` | views | Copy a `VarImpType*` or another view, plus at most a constant or cached range pointers (`gecode/kernel/view.hpp:139`, `237`; `CachedView` holds two `RangeList*` and a size). Not reported today because they are trivially copyable. They encode the brief's rule in case a view gains a user-declared copy constructor. The implementer may drop these two entries | 0 |
| `^Gecode::.*::VarType$` | `typename View::VarType` | Every `VarType` typedef names a variable type (`gecode/kernel/view.hpp:49`, `133`, `233`) | 2 (counted under `IntVar` and `BoolVar` above) |

Total suppressed: 1182 unique diagnostics (2016 − 834). Copying none of
these types touches an atomic reference count.

Types deliberately **not** allowed (all have reference counts or heap
storage, or are not shallow handles): `FloatVal` (see below), `Rnd`,
`AFC`/`Action`/`CHB` and their `Int`/`Bool`/`Set`/`Float` forms, `IntSet`,
`DFA`, `TupleSet`, `SharedArray<>`/`IntSharedArray`, `TraceFilter`, `REG`, the
`*Args` arrays, the branch configuration classes (`*VarBranch`, `*ValBranch`,
`*Assign`, `TieBreak<>`), every `std::function` alias, `std::string`,
`std::vector`, `std::shared_ptr`, and `std::exception_ptr`.

### Policy exception for the user: FloatVal

`FloatVal` is two doubles, copied by `FloatVal(const FloatVal&) : x(v.x)`
(`gecode/float/val.hpp:55`). It holds no reference count or allocation, but
it is 16 bytes, not pointer-sized, so it is not a shallow handle under the
brief. It is a value type used in arithmetic, so passing it by value is
arguably natural. **Decision for the user:** keep the proposal above and pass
`FloatVal` by `const&` (group G16), or append `;^Gecode::FloatVal$` to
AllowedTypes and keep it by value.

| | Proposed (no FloatVal) | With the FloatVal exception | Difference |
|---|---:|---:|---:|
| Raw diagnostics | 117975 | 113879 | 4096 |
| Unique diagnostics | 834 | 776 | 58 |
| Unique parameters | 708 | 655 | 53 (all of G16) |

The 53 G16 parameters are in `gecode/float` (39), `gecode/minimodel` (6),
and `test` (8). All of them become `const FloatVal&`. `FloatVal` declares a
copy constructor and no move constructor, so no row moves. Where the callee
keeps the value (for example `Lin`'s `c0`, `NqFloat`'s `_c`, and the
`ScaleView` constructor), the member copy from the `const&` is the only copy.
If the user takes the exception, drop G16, and the other groups are
unchanged.

## Counts

"Raw" counts every emitted diagnostic. A header diagnostic repeats once per
translation unit that includes the header. "Unique" deduplicates on file,
line, column, and message. Modules are taken from the reported file path.

| Module | Raw, no AllowedTypes | Unique, no AllowedTypes | Raw, AllowedTypes | Unique, AllowedTypes | Raw, with FloatVal exception | Unique, with FloatVal exception |
|---|---:|---:|---:|---:|---:|---:|
| examples | 43 | 38 | 6 | 6 | 6 | 6 |
| gecode (top-level headers) | 94 | 11 | 71 | 5 | 71 | 5 |
| gecode/flatzinc | 92 | 71 | 74 | 56 | 74 | 56 |
| gecode/float | 28432 | 278 | 24812 | 142 | 21268 | 98 |
| gecode/gist | 2549 | 14 | 980 | 5 | 980 | 5 |
| gecode/int | 66830 | 893 | 54313 | 300 | 54313 | 300 |
| gecode/kernel | 24938 | 190 | 12540 | 150 | 12540 | 150 |
| gecode/minimodel | 20609 | 112 | 741 | 8 | 247 | 2 |
| gecode/search | 4 | 2 | 1 | 1 | 1 | 1 |
| gecode/set | 28811 | 289 | 24341 | 115 | 24341 | 115 |
| test | 182 | 118 | 96 | 46 | 38 | 38 |
| **Total** | **172584** | **2016** | **117975** | **834** | **113879** | **776** |

Unique parameters with AllowedTypes, by module: examples 6, gecode/flatzinc 56, gecode/float 137, gecode/gist 6, gecode/int 282, gecode/kernel 52, gecode/minimodel 8, gecode/search 1, gecode/set 115, test 45; total 708.

Unique diagnostics by kind: const-copy 3, const-ref 403, move 428. Parameters by kind: const-copy 3, const-ref 353, const-ref+move 1, move 351.

Translation units parsed with libclang: 61.

Parameter modules are taken from the definition, so a parameter reported in a
top-level header (such as `gecode/int.hh`) counts under the module that
defines it. A diagnostic is keyed by the parameter that libclang finds at the
reported location (or at the use that a move diagnostic points to). The
diagnostics of one parameter then become one row, keyed by the parameter's
location in its definition. Several diagnostics share a row for two reasons.
A template instantiated with different arguments reports the same parameter
once per spelling of its type. A member of a class template can also be
reported both at its in-class declaration and at its out-of-line definition.
tools/ has no diagnostics.

Of the 834 unique diagnostics, 403 say the parameter is "only used as a const
reference", 428 say it is "passed by value and only copied once; consider
moving", and 3 are `const`-qualified `std::string` parameters in
`gecode/flatzinc/flatzinc.cpp`. By parameter: 353 const-reference, 351 move,
3 const-copy, and 1 that gets both kinds from different instantiations.

## What the diagnostics mean here

Read these before applying a row's treatment.

1. **Where it reports.** A const-reference diagnostic is reported at the
   parameter. A move diagnostic is reported at the copy site, which is the
   last use. libclang resolves the copy site to the parameter it names. Members
   of class templates are often reported at the in-class declaration. Each row
   gives the parameter in the definition, and every other declaration in the
   "Other declarations" column.
2. **Declarations come from libclang, not from fix-its.** A move diagnostic
   exports only `std::move(...)` replacements and says nothing about other
   declarations, so round 1 missed redeclarations for those rows. Now
   `analyze.py` collects every declaration of the parameter's function that
   the parsing translation unit can see (same canonical declaration),
   including in-class declarations, forward declarations in headers, and the
   declarations of function templates. 610 of the 708 parameters have at least
   one other declaration. As a consistency check, every declaration that a
   const-reference fix-it rewrites is also in the libclang set.
   Declarations visible only in other translation units are not searched. A
   stale one would fail to link.
3. **`SharedHandle` types, the `*Args` arrays, and `FloatVal` have no real
   move.** `SharedHandle` declares a copy constructor and copy assignment but
   no move (`gecode/kernel/shared-object.hpp:45`), and so do `IntAFC`,
   `IntAction`, and the other handles. `ArgArray` declares
   `operator=(const ArgArray&) = default`. `FloatVal` declares its copy
   constructor. A derived class such as `IntSet` or `IntArgs` still gets an
   implicit move constructor, but it calls the base copy constructor. So the
   check sometimes says "consider moving" for them, for example
   `gecode/int/count/int-eq.hpp:37` (`IntSet`) and
   `gecode/minimodel/matrix.hpp:128` (`IntArgs`), but `std::move` would only
   copy. Task 002 does not count that as a move. Every such row is treated as
   `const&`. Where the callee keeps the value, its member copy (or copy
   assignment) from the `const&` is the only copy, and the row says so.
4. **Branch configuration classes do move.** `VarBranch`, `ValBranch`,
   `TieBreak<>`, `IntVarBranch`, `IntValBranch`, `IntAssign`, and their
   set/float forms declare no copy operations. Their implicit move
   constructors move the `std::function` members and copy the
   reference-counted handles. So a configuration object that a callee stores,
   or builds into the object it returns, stays by value and moves.
5. **Source compatibility.** Changing a by-value parameter to `const&`, or
   adding `std::move` inside a body, forces no change on any caller. The only
   coupled edits are: every other declaration in the row; the virtual set in
   G1; the explicit specializations `is_int_set` and `isval`
   (`gecode/int/count/rel.hpp`), whose primary templates must change with
   them; and the generated parser (G13). clang-tidy offers no fix-it for 4
   parameters: the two virtual base definitions and the two explicit
   specializations. No other reported function is virtual (`analyze.py`
   checks this with libclang).
6. **Chains.** Many parameters only pass a value to the next layer. Within a
   chain the treatment follows the final destination, not the diagnostic kind
   of one layer. A chain that only reads becomes `const&` throughout, even
   where one layer says "consider moving". A chain that ends in storage stays
   by value and moves on every path to the store. The diagnostic kind is in
   `rows.tsv`.

## Delicate cases

### DFA through OpenRegular

`extensional(Home, OpenIntVarList, DFA dfa, IntPropLevel)`
(`gecode/int/extensional-regular.cpp:56`, declared `gecode/int.hh:2984`) and
the `OpenBoolVarList` form (`:63`, `gecode/int.hh:2995`) pass `dfa` to
`OpenRegular<View, pc>::post(Home, OpenViewList<View>, DFA dfa)`
(`gecode/int/extensional/open.hpp:296`). That function reads it in a loop
(`DFA::Symbols symbols(dfa)`) and then calls
`new OpenRegular(home, list, dfa)`. The constructor
`OpenRegular(Home, OpenViewList<View> list0, DFA dfa0)` (`open.hpp:108`)
copies `dfa0` into the member `dfa` and then reads `dfa0` again in
`distance(accepting_distances(dfa0))`. An early `std::move(dfa0)` would make
that call read a moved-from handle. Since `DFA` is a `SharedHandle`, a move
would be a copy anyway. Treatment: `const DFA&` in all three layers and in
both `gecode/int.hh` declarations. The member copy is the only copy, one
reference-count increment. The array overloads
(`extensional-regular.cpp:38`, `:47`) only pass `dfa` to
`Extensional::post_lgp`, which already takes `const DFA&`. `OpenRegular`'s
`list0` is a shallow `OpenViewList` and is allowed.

### Stored callbacks and SharedData

`SharedData<Data>` (`gecode/kernel/data/shared-data.hpp`) has only
`SharedData(const Data& d) : SharedHandle(new SDO(d))` and
`SDO(const Data& d0) : d(d0)`, with member `const Data d`. Every store
therefore copies, whatever its callers pass. None of the receiving
constructors moves today:

| Receiver | Initialization | Parameter today | Copies from receiver parameter to SDO today |
|---|---|---|---|
| `OpenVarList<Var>::Object` (`gecode/int/open-var-list.hpp:87`, `93`) | `functions(Functions(checked(std::move(create)), checked(std::move(t))))` | by value, moved into a temporary `Functions` | 1 (`SDO` copies the temporary) |
| `Int::Exec::When` (`gecode/int/exec/when.hpp:37`, declared `gecode/int/exec.hh:61`) | `t(t0), e(e0)` | by value | 1 each |
| `Kernel::FunctionBranch` (`gecode/kernel/branch/function.cpp:88`, declared `:55`) | `f(f0)` | by value | 1 |
| `UnaryWait`, `NaryWait` (`gecode/kernel/propagator/wait.hpp:108`, `171`) | `c(c0)` | by value | 1 |
| `BrancherFilter<View>` (`gecode/kernel/branch/filter.hpp:91`) | `f(bf)`, then `if (!bf)` | by value | 1 |
| `BrancherPrint<View, Val>` (`gecode/kernel/branch/print.hpp:94`) | `p(vvp)`, then `if (!vvp)` | by value | 1 |
| `OpenListBrancher` (`gecode/int/branch/open.hpp:58`) | `filter(bf)` (a `BrancherFilter` by value), `printer(vvp)` | by value | 1 each, plus the `BrancherFilter` layer |
| `ValSelFunction` (`gecode/kernel/branch/val-sel.hpp:112`) | `v(vb.val())` | accessor returns by value | 1 (`SDO` copies the accessor's temporary) |
| `ValCommitFunction` (`gecode/kernel/branch/val-commit.hpp:116`) | `c(vb.commit())` | accessor returns by value | 1 |
| `MeritFunction` (`gecode/kernel/branch/merit.hpp:201`) | `f(vb.merit())` | accessor returns by value | 1 |
| `ViewSelChooseTbl` (`gecode/kernel/branch/view-sel.hpp:689`) | `tbl(vb.tbl())` | accessor returns by value | 1 |

The callbacks pass through several by-value layers before they reach these
receivers. For example, a print callback given to `branch(...)` is copied in
`branch` → `post_view_value_brancher` → `ViewValBrancher::post` →
`ViewValBrancher` → `BrancherPrint` → `SDO`.

Task 002 requires a by-value parameter to reach storage through an actual
move, including through SharedData/SDO. So **G2** adds an rvalue path:
`SharedData(Data&& d) : SharedHandle(new SDO(std::move(d)))` and
`SDO(Data&& d0) : d(std::move(d0))`, next to the existing `const Data&`
constructors. A `const Data` member can be move-constructed. With G2, the
four accessor receivers move from their temporaries with no other change.
`OpenVarList::Object` moves its temporary `Functions`. The by-value
receivers become `c(std::move(c0))` and so on. Rows that end in these
receivers say "into SharedData storage (needs G2)". Rows that pass the
callback on toward them say "on each path into SharedData (needs G2)". G2
has no diagnostic of its own, but G7, G8, G10, and G11 depend on it.

Two receivers read the parameter after initializing from it.
`BrancherFilter` checks `if (!bf)` after `f(bf)`, and `BrancherPrint` checks
`if (!vvp)` after `p(vvp)`. After moving, test the stored value
(`if (!f())`, `if (!p())`) or check before the initializer. `FunctionBranch`,
`UnaryWait`, `NaryWait`, and `When` already test the stored value, or test
in `post` before constructing. `BrancherNoFilter` and `BrancherNoPrint`
(`filter.hpp:129`, `print.hpp:133`) only assert that the callback is empty,
so they take `const&`.

Each `post` that calls a callback right away when its view is already
assigned, and otherwise constructs a propagator (`UnaryWait::post`,
`NaryWait::post`, `When::post`), uses it on exclusive paths. Moving on the
constructing path is safe. The same holds for the `if (bf) … else …`
dispatch in `post_view_value_brancher`, `post_view_values_brancher`,
`post_open_brancher`, `post_ldsb_brancher`, and `post_ldsb_set_brancher`. Each
branch passes the callback exactly once.

### Other stored values

The "Callee use" column reports how the definition keeps a value. It covers
member initializers, assignments to members (`_afc = a`), `push_back` and
`emplace` into containers (also through a temporary such as
`push_back(std::pair(x, n))`), lambda captures, returns of the parameter or
of an object constructed from it (`return IntVarBranch(…, tbl)`), and locals
built from it and then returned. Base-class and delegating constructor
initializers forward the value to the next row instead of keeping it.

- **Branch configuration factories** (G4–G6). `INT_VAR_*`, `BOOL_VAR_*`,
  `SET_VAR_*`, and `FLOAT_VAR_*` take `BranchTbl tbl` (and merit functions) by
  value and return a configuration object built from them, so they move:
  `return IntVarBranch(…, std::move(tbl))`. The constructors already move into
  `VarBranch::_tbl` and `_mf` (`gecode/kernel/branch/var.hpp:119`–`150`).
  `INT_VAL`, `INT_ASSIGN`, and their bool/set/float forms likewise move their
  value and commit functions into `IntValBranch(v, c)` and so on. Those pass
  them to `ValBranch(BranchVal v, BranchCommit c)`
  (`gecode/kernel/branch/val.hpp:80`), which copies `vf(v), cf(c)` today. G3
  makes that `vf(std::move(v)), cf(std::move(c))`. The `Rnd`, AFC, Action, and
  CHB arguments become `const&` everywhere, including in the `*_RND`/`*_AFC_*`
  factories that build them into the returned object and in the
  `VarBranch`/`ValBranch` constructors that copy them into members. These
  handles have no move, so the member copy is the only copy.
- **VarBranch setters.** `VarBranch<Var>::afc(AFC a)`, `action(Action a)`,
  and `chb(CHB chb)` (`gecode/kernel/branch/var.hpp:178`, `190`, `202`;
  declared `:101`, `:105`, `:109`) copy-assign the parameter to `_afc`,
  `_act`, and `_chb`. These are reference-counted handles with no move, so they
  become `const&` and the copy assignment is the only copy.
- **Action/CHB merit functions.** `IntAction(…, IntBranchMerit bm)` →
  `Action::init(…, bm)` → `new Storage(…, bm)`
  (`gecode/kernel/branch/action.hpp:272`, `354`; `chb.hpp:257`, `344`). Storage
  only calls `bm` to seed values and does not keep it. The check says
  "consider moving" for the outer layers because each one copies once into
  the next by-value parameter. The chain only reads, so all of it becomes
  `const&` (20 parameters). The declarations change too: `gecode/int.hh`,
  `gecode/set.hh`, and `gecode/float.hh` for the public classes, and
  `action.hpp:58`/`:108` and `chb.hpp` for `Storage` and `init`.
- **TieBreak and tie_break.** `TieBreak(VarBranch a0, …)`
  (`gecode/kernel/branch/tiebreak.hpp:65`, declared `:42`) stores its
  arguments in `a`, `b`, `c`, and `d`. The three `tie_break(VarBranch a, …)`
  templates (`:70`, `:77`, `:84`; declared `:54`, `:57`, `:60`) build a local
  `TieBreak` from all their parameters and return it. All 13 parameters stay
  by value and move: `TieBreak<VarBranch> ab(std::move(a), std::move(b))`.
  The configuration classes have real moves, so this saves the copies of
  their `std::function` members. The public
  `branch(Home, …, IntVarBranch vars, …)` builds `TieBreak<IntVarBranch>(vars)`
  (`gecode/int/branch.cpp:309`, `322`), so `vars` moves there. Posting
  functions that only read a `TieBreak` or a `*ValBranch` take `const&`,
  because the `ViewSel`/`ValSel` constructors copy what they need through
  accessors.
- **FlatZinc.** `AuxVarBrancher` (`gecode/flatzinc/flatzinc.cpp:113`)
  stores all eight selectors, and its `post` (`:246`) forwards them, so both
  stay by value and move. `BranchInformationO(…, v0)` (`:286`) and
  `BlackBoxContext::fail(std::exception_ptr e)`
  (`gecode/flatzinc/blackbox-propagator.cpp:143`, assigning `exception`) move.
  The `IntBoolVarBranch` constructors and the `int_bool_var_*` factories
  (`gecode/flatzinc/branch.hpp`) keep AFC, Action, and CHB handles, which have
  no move, so they take `const&`.
- **Open variable lists** (G11). `OpenIntVarList::factory(Domain d)`
  (`gecode/int/open-var-list.cpp:40`) checks `!d` and then copies `d` into the
  returned lambda. Capture it with `[d = std::move(d)]`. The constructors at
  `:52`–`:65` pass `d` and `t` once to `factory` and the `OpenVarList`
  constructor, which already move into `Functions`.
- **FlatZinc parser.** `ParserState::output(std::string x, AST::Node* n)`
  (`gecode/flatzinc/parser.hh:258`) copies `x` into `output_items` through a
  `std::pair` temporary. Keep it by value and `emplace_back(std::move(x), n)`.
  The three `parser.tab.cpp` parameters are generated from
  `gecode/flatzinc/parser.yxx:98`, `:176`, and `:200`. Edit the grammar, then
  build the `regenerate` target (Bison 3.8 and Flex 2.6 on `PATH`).
  All three only read `id`, so they become `const std::string&`.
  `get_var_ref_arg` copies it into a local only for annotations and reads it
  again afterwards. `add_domain_constraint` passes it to
  `new ConExpr(const std::string&, …)`, which makes the only copy.
- **Tests.** `NoGoods(TestTags, ValBranch vb0, …)` (`test/nogoods.cpp:250`)
  stores `vb0` in `vb`. Its `Base` initializer reads `Model::val(vb0)` first
  (bases are initialized before members), so keep that read and write
  `vb(std::move(vb0))`. `test/flatzinc.cpp`, `test/instance.cpp`,
  `test/search.cpp`, `test/test.cpp`, and `test/int/open-var-list.cpp` store
  strings, vectors, `shared_ptr`s, and callbacks and move them. The labels
  that `test/ldsb.cpp` and `test/int/gcc.cpp` forward to a base constructor
  become `const&`, because that base makes its own copy.
- **Count's generic `VY`.** `EqInt`, `GqInt`, `LqInt`, `EqView`, `GqView`, and
  `LqView` (`gecode/int/count/*.hpp`, declared in `gecode/int/count.hh`) take a
  template parameter `VY y` that is either a view or `IntSet`. The `IntSet`
  instantiation is reported. `const VY&` is fine for both, since the
  constructors copy `y` into a member through `IntBase`/`ViewBase`. The
  explicit specializations `is_int_set(IntSet)` and `isval(IntSet)`
  (`rel.hpp:50`, `:62`) must change with their primary templates
  (`rel.hpp:44`, `:57`; `count.hh:54`, `:57`).
- **CBS virtual hooks** (G1). `Propagator::solution_distribution(Space&,
  SendMarginal) const` and `domain_size_sum(InDecision, unsigned int&,
  unsigned int&) const` are declared virtual at `gecode/kernel/core.hpp:1259`
  and `:1269` and defined empty at `:3781` and `:3784`. They are overridden
  by `Distinct::Val`, `Bnd`, and `Dom` (`gecode/int/distinct.hh:75`, `77`,
  `165`, `167`, `291`, `293`; definitions `gecode/int/distinct/val.hpp:160`,
  `166`, `bnd.hpp:462`, `468`, `dom.hpp:80`, `86`). The overrides are not
  reported because the check skips overriding methods. Change the base and
  all six overrides together. The callers (`gecode/int/branch/cbs.hpp:115`,
  `158`, `179`) pass a lambda or a local `std::function`, and both bind to
  `const&`. The helpers `cbs_distinct` and `cbs_size`
  (`gecode/int/distinct/cbs.hpp:135`, `211`; declared
  `gecode/int/distinct.hh:99`, `102`) only call the callback.

## Validation

Two runs back the counts and tables in this file:

- `sh build/zdev-audit/value-params/reproduce.sh build/full 10` ran from
  scratch after the scripts were final. All twelve clang-tidy runs exited 0,
  and it printed the four lines shown in [Reproduce](#reproduce).
- Steps 2 and 3 of [Reproduce](#reproduce) were then replayed from this
  document alone. The extraction command wrote the appendix blocks to a fresh
  `build/zdev-audit/value-params-replay/`, and
  `sh build/zdev-audit/value-params-replay/reproduce.sh build/full 10`
  ran there. It printed the same four lines. Its `counts.md`, `groups.md`,
  `hooks.md`, `rows.md`, and `rows.tsv` are byte-identical to the first run's,
  and its `checks.txt` is empty.

`git status` showed no change to tracked files.

After a review found that `analyze.py` missed storage reached through
`std::forward` or `std::move` in an initializer (`DomArgs` in
`gecode/minimodel/dom.cpp`), the classification and that row's note were
corrected and `analyze.py` was rerun on the same clang-tidy output. Only the
`DomArgs` row in `rows.md`, `rows.tsv`, and `hooks.md` changed, the four
summary lines stayed the same, and `checks.txt` stayed empty. The prescribed
`DomArgs` change passed a syntax-only compile with the database's command
line.

## Repair groups

Each group keeps a signature together with its definitions, its other
declarations, its override set, and any forced caller changes, and is sized
as one reviewable commit. The file lists are generated (`groups.md`): every
file holding a definition, and every other file holding a declaration that
must change with it. The parameters of each group are listed in the
per-parameter inventory below.

| Group | Parameters | Definition files | Other declaration files |
|---|---:|---|---|
| G1 CBS virtual hooks | 4 | `gecode/int/distinct/cbs.hpp`, `gecode/kernel/core.hpp` | `gecode/int/distinct.hh` |
| G3 kernel branch configurations | 27 | `gecode/kernel/branch/action.hpp`, `gecode/kernel/branch/chb.hpp`, `gecode/kernel/branch/tiebreak.hpp`, `gecode/kernel/branch/val.hpp`, `gecode/kernel/branch/var.hpp` | — |
| G4 int branch configurations | 120 | `gecode/int/branch/action.cpp`, `gecode/int/branch/assign.hpp`, `gecode/int/branch/chb.cpp`, `gecode/int/branch/val.hpp`, `gecode/int/branch/var.hpp` | `gecode/int.hh` |
| G5 set branch configurations | 73 | `gecode/set/branch/action.cpp`, `gecode/set/branch/assign.hpp`, `gecode/set/branch/chb.cpp`, `gecode/set/branch/val.hpp`, `gecode/set/branch/var.hpp` | `gecode/set.hh` |
| G6 float branch configurations | 71 | `gecode/float/branch/action.cpp`, `gecode/float/branch/assign.hpp`, `gecode/float/branch/chb.cpp`, `gecode/float/branch/val.hpp`, `gecode/float/branch/var.hpp` | `gecode/float.hh` |
| G7 kernel and int brancher posting | 111 | `gecode/int/branch.cpp`, `gecode/int/branch.hpp`, `gecode/int/branch/open.hpp`, `gecode/int/branch/view-values.hpp`, `gecode/int/ldsb.cpp`, `gecode/int/ldsb/brancher.hpp`, `gecode/kernel/branch/filter.hpp`, `gecode/kernel/branch/print.hpp`, `gecode/kernel/branch/view-val.hpp`, `gecode/kernel/branch/view.hpp` | `gecode/int.hh`, `gecode/int/branch.hh`, `gecode/int/ldsb.hh` |
| G8 set and float brancher posting | 58 | `gecode/float/branch.cpp`, `gecode/float/branch.hpp`, `gecode/set/branch.cpp`, `gecode/set/branch.hpp`, `gecode/set/ldsb.cpp`, `gecode/set/ldsb/brancher.hpp` | `gecode/float.hh`, `gecode/set.hh`, `gecode/set/ldsb.hh` |
| G9 posting with shared data | 48 | `gecode/float/relax.cpp`, `gecode/int/count/int-eq.hpp`, `gecode/int/count/int-gq.hpp`, `gecode/int/count/int-lq.hpp`, `gecode/int/count/rel.hpp`, `gecode/int/count/view-eq.hpp`, `gecode/int/count/view-gq.hpp`, `gecode/int/count/view-lq.hpp`, `gecode/int/element.cpp`, `gecode/int/extensional-regular.cpp`, `gecode/int/extensional/open.hpp`, `gecode/int/ldsb/sym-obj.cpp`, `gecode/int/relax.cpp`, `gecode/int/sequence/int.hpp`, `gecode/int/sequence/view.hpp`, `gecode/minimodel/aliases.hpp`, `gecode/minimodel/matrix.hpp`, `gecode/set/int.cpp`, `gecode/set/ldsb/brancher.hpp`, `gecode/set/relax.cpp` | `gecode/float.hh`, `gecode/int.hh`, `gecode/int/count.hh`, `gecode/int/ldsb.hh`, `gecode/int/sequence.hh`, `gecode/minimodel.hh`, `gecode/set.hh` |
| G10 execution callbacks | 22 | `gecode/float/exec.cpp`, `gecode/int/exec.cpp`, `gecode/int/exec/when.hpp`, `gecode/kernel/branch/function.cpp`, `gecode/kernel/propagator/wait.hpp`, `gecode/set/exec.cpp` | `gecode/float.hh`, `gecode/int.hh`, `gecode/int/exec.hh`, `gecode/kernel.hh`, `gecode/set.hh` |
| G11 open variable lists | 6 | `gecode/int/open-var-list.cpp` | `gecode/int.hh` |
| G12 tracing | 9 | `gecode/float/trace.cpp`, `gecode/int/trace.cpp`, `gecode/kernel/trace/general.cpp`, `gecode/kernel/trace/recorder.hpp`, `gecode/set/trace.cpp` | `gecode/float.hh`, `gecode/int.hh`, `gecode/kernel.hh`, `gecode/kernel/core.hpp`, `gecode/set.hh` |
| G13 FlatZinc | 56 | `gecode/flatzinc/blackbox-propagator.cpp`, `gecode/flatzinc/branch.cpp`, `gecode/flatzinc/branch.hpp`, `gecode/flatzinc/flatzinc.cpp`, `gecode/flatzinc/parser.hh`, `gecode/flatzinc/parser.tab.cpp` | `gecode/flatzinc/blackbox.hh`, `gecode/flatzinc/branch.hh` |
| G14 Gist and search | 7 | `gecode/gist/gist.hpp`, `gecode/search/cpprofiler/tracer.cpp` | `gecode/gist.hh`, `gecode/search.hh` |
| G15 tests and examples | 43 | `examples/job-shop.cpp`, `examples/pentominoes.cpp`, `test/branch/int.cpp`, `test/flatzinc.cpp`, `test/instance.cpp`, `test/int/extensional.cpp`, `test/int/gcc.cpp`, `test/int/min-distance.cpp`, `test/int/open-var-list.cpp`, `test/ldsb.cpp`, `test/nogoods.cpp`, `test/search.cpp`, `test/test.cpp` | `test/flatzinc.hh`, `test/test.hh` |
| G16 FloatVal (only without the FloatVal exception) | 53 | `gecode/float/dom.cpp`, `gecode/float/linear.cpp`, `gecode/float/linear/nary.hpp`, `gecode/float/linear/post.cpp`, `gecode/float/rel.cpp`, `gecode/float/rel/eq.hpp`, `gecode/float/rel/lq-le.hpp`, `gecode/float/rel/nq.hpp`, `gecode/float/view/float.hpp`, `gecode/float/view/minus.hpp`, `gecode/float/view/offset.hpp`, `gecode/float/view/rel-test.hpp`, `gecode/float/view/scale.hpp`, `gecode/minimodel/dom.cpp`, `gecode/minimodel/float-expr.cpp`, `gecode/minimodel/float-rel.hpp`, `test/float.cpp`, `test/float.hpp`, `test/float/basic.cpp`, `test/float/dom.cpp` | `gecode/float.hh`, `gecode/float/linear.hh`, `gecode/float/rel.hh`, `gecode/float/view.hpp`, `gecode/minimodel.hh`, `test/float.hh` |

| Group | Summary | Depends on |
|---|---|---|
| G1 CBS virtual hooks | `const&` for `SendMarginal`/`InDecision` in the base virtuals, all six overrides (`gecode/int/distinct.hh`, `gecode/int/distinct/{bnd,val,dom}.hpp`, not reported), and the two helpers | — |
| G2 SharedData rvalue sink | No parameters. Add `SharedData(Data&&)` and `SDO(Data&&)` in `gecode/kernel/data/shared-data.hpp` so stored callbacks can arrive by move | — |
| G3 kernel branch configurations | `VarBranch`/`ValBranch` handle parameters and the `afc`/`action`/`chb` setters to `const&`. `ValBranch` value/commit functions, `TieBreak`, and `tie_break` by value plus move. `Action`/`CHB` `init` and `Storage` merit to `const&` | — |
| G4 int branch configurations | `INT_*`/`BOOL_*` factories and constructors: move `tbl`, merit, value, and commit functions into the returned object; `Rnd`/AFC/Action/CHB to `const&`. `IntAction`/`BoolAction`/`IntCHB`/`BoolCHB` merit to `const&` | G3 (for the moves to reach storage) |
| G5 set branch configurations | Same as G4 for `SET_*` | G3 |
| G6 float branch configurations | Same as G4 for `FLOAT_*` | G3 |
| G7 kernel and int brancher posting | Filter and print callbacks by value plus move on every path down to `BrancherFilter`/`BrancherPrint`/`OpenListBrancher`, with the `!bf`/`!vvp` checks reordered. `*ValBranch`/`*Assign` and reading `TieBreak` to `const&`. `IntVarBranch vars` moved into `TieBreak`. If 111 is too many for one review, split it into the kernel and int layers (G7a: the four kernel headers plus `view-values.hpp`, `open.hpp`, `ldsb/brancher.hpp`) and the public entry points (G7b: `branch.cpp`, `branch.hpp`, `ldsb.cpp`, and the declarations in `int.hh`, `int/branch.hh`, `int/ldsb.hh`). Each half compiles on its own | G2 |
| G8 set and float brancher posting | Same as G7 for set and float | G2, G7 (shared kernel layers) |
| G9 posting with shared data | `DFA`, `IntSharedArray`, `IntSet`, `Rnd`, `IntArgs`, and `Matrix`'s `A` to `const&`, including the DFA chain and Count's `VY` with its explicit specializations | — |
| G10 execution callbacks | `wait`, `when`, and function-branch callbacks by value plus move into `UnaryWait`/`NaryWait`/`When`/`FunctionBranch` | G2 |
| G11 open variable lists | `Domain`/`Transition` moves, and the init-capture in `factory` | G2 (for the final `Functions` move) |
| G12 tracing | `TraceFilter` (a `SharedHandle`) to `const&` down to the recorder members | — |
| G13 FlatZinc | `AuxVarBrancher`, `BranchInformationO`, and `exception_ptr` moves. `Rnd`, AFC, Action, CHB, and `SharedArray<bool>` to `const&`. `const std::string` to `const std::string&`. `output` by value plus `emplace_back`. Grammar `id` to `const&` | G4–G8 (uses their signatures, but compiles either way) |
| G14 Gist and search | Comparator name strings to `const&`. The CPProfiler tracer name moves | — |
| G15 tests and examples | Fixture constructors move strings, vectors, `shared_ptr`s, callbacks, and `NoGoods`'s `vb0`. Read-only `IntSet`/`TupleSet`/`DFA`/`*ValBranch`/`REG` parameters and forwarded labels to `const&` | Last, after the library groups, because some test signatures mirror library ones |
| G16 FloatVal | `FloatVal` to `const FloatVal&` in float propagators, views, posting functions, minimodel, and float tests. Only if the user rejects the FloatVal exception | — |

Ordering: do G2 before G7, G8, G10, and G11, and G3 before G4–G6. G7 before
G8 avoids touching the shared kernel headers twice. All other groups are
independent. Every change is source compatible, so each group compiles and
passes on its own, and the order only affects review clarity and copy
counts. The check can only be enabled in `.clang-tidy` (task 003) after
every group is done.

## Verification hooks

For each group, the middle column lists every parameter that becomes `const&`
although its callee keeps a copy (it stores, assigns, captures, or returns it,
or builds it into a returned object). Task 002's verifier checks these by name.
The keeping callee then copies from the `const&`. That copy is the only one,
since these types have no move. The last column counts the by-value-plus-move
parameters by type. Their moves reach the implicit move constructors of the
configuration classes, `std::function`, `std::string`, `std::vector`,
`std::shared_ptr`, `std::exception_ptr`, and, for callbacks, the G2
`SDO(Data&&)`. `rows.tsv` has all these columns in machine-readable form.

| Group | const& parameters whose callee keeps a copy | By value + std::move |
|---|---|---|
| G1 CBS virtual hooks | — | — |
| G3 kernel branch configurations | `ValBranch::ValBranch<Var>` `r0` (gecode/kernel/branch/val.hpp:74:42: initializes r)<br>`VarBranch::VarBranch<Var>` `a` (gecode/kernel/branch/var.hpp:125:42: initializes _afc)<br>`VarBranch::VarBranch<Var>` `a` (gecode/kernel/branch/var.hpp:131:45: initializes _act)<br>`VarBranch::VarBranch<Var>` `c` (gecode/kernel/branch/var.hpp:137:42: initializes _chb)<br>`VarBranch::VarBranch<Var>` `r` (gecode/kernel/branch/var.hpp:143:42: initializes _rnd)<br>`VarBranch::afc` `a` (gecode/kernel/branch/var.hpp:178:29: assigns _afc)<br>`VarBranch::action` `a` (gecode/kernel/branch/var.hpp:190:35: assigns _act)<br>`VarBranch::chb` `chb` (gecode/kernel/branch/var.hpp:202:29: assigns _chb) | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch ×13, BranchCommit ×1, BranchVal ×1 |
| G4 int branch configurations | `INT_ASSIGN_RND` `r` (gecode/int/branch/assign.hpp:68:24: returns a IntAssign built from it)<br>`BOOL_ASSIGN_RND` `r` (gecode/int/branch/assign.hpp:105:25: returns a BoolAssign built from it)<br>`INT_VAL_RND` `r` (gecode/int/branch/val.hpp:68:21: returns a IntValBranch built from it)<br>`BOOL_VAL_RND` `r` (gecode/int/branch/val.hpp:135:22: returns a BoolValBranch built from it)<br>`INT_VAR_RND` `r` (gecode/int/branch/var.hpp:108:21: returns a IntVarBranch built from it)<br>`INT_VAR_AFC_MIN` `a` (gecode/int/branch/var.hpp:138:28: returns a IntVarBranch built from it)<br>`INT_VAR_AFC_MAX` `a` (gecode/int/branch/var.hpp:148:28: returns a IntVarBranch built from it)<br>`INT_VAR_ACTION_MIN` `a` (gecode/int/branch/var.hpp:158:34: returns a IntVarBranch built from it)<br>`INT_VAR_ACTION_MAX` `a` (gecode/int/branch/var.hpp:168:34: returns a IntVarBranch built from it)<br>`INT_VAR_CHB_MIN` `c` (gecode/int/branch/var.hpp:173:28: returns a IntVarBranch built from it)<br>`INT_VAR_CHB_MAX` `c` (gecode/int/branch/var.hpp:183:28: returns a IntVarBranch built from it)<br>`INT_VAR_AFC_SIZE_MIN` `a` (gecode/int/branch/var.hpp:238:33: returns a IntVarBranch built from it)<br>`INT_VAR_AFC_SIZE_MAX` `a` (gecode/int/branch/var.hpp:248:33: returns a IntVarBranch built from it)<br>`INT_VAR_ACTION_SIZE_MIN` `a` (gecode/int/branch/var.hpp:258:39: returns a IntVarBranch built from it)<br>`INT_VAR_ACTION_SIZE_MAX` `a` (gecode/int/branch/var.hpp:268:39: returns a IntVarBranch built from it)<br>`INT_VAR_CHB_SIZE_MIN` `c` (gecode/int/branch/var.hpp:273:33: returns a IntVarBranch built from it)<br>`INT_VAR_CHB_SIZE_MAX` `c` (gecode/int/branch/var.hpp:283:33: returns a IntVarBranch built from it)<br>`BOOL_VAR_RND` `r` (gecode/int/branch/var.hpp:377:22: returns a BoolVarBranch built from it)<br>`BOOL_VAR_AFC_MIN` `a` (gecode/int/branch/var.hpp:407:30: returns a BoolVarBranch built from it)<br>`BOOL_VAR_AFC_MAX` `a` (gecode/int/branch/var.hpp:417:30: returns a BoolVarBranch built from it)<br>`BOOL_VAR_ACTION_MIN` `a` (gecode/int/branch/var.hpp:427:36: returns a BoolVarBranch built from it)<br>`BOOL_VAR_ACTION_MAX` `a` (gecode/int/branch/var.hpp:437:36: returns a BoolVarBranch built from it)<br>`BOOL_VAR_CHB_MIN` `c` (gecode/int/branch/var.hpp:442:30: returns a BoolVarBranch built from it)<br>`BOOL_VAR_CHB_MAX` `c` (gecode/int/branch/var.hpp:452:30: returns a BoolVarBranch built from it) | BoolBranchCommit ×4, BoolBranchMerit ×2, BoolBranchVal ×4, BranchTbl ×56, IntBranchCommit ×4, IntBranchMerit ×2, IntBranchVal ×4 |
| G5 set branch configurations | `SET_ASSIGN_RND_INC` `r` (gecode/set/branch/assign.hpp:83:28: returns a SetAssign built from it)<br>`SET_ASSIGN_RND_EXC` `r` (gecode/set/branch/assign.hpp:88:28: returns a SetAssign built from it)<br>`SET_VAL_RND_INC` `r` (gecode/set/branch/val.hpp:83:25: returns a SetValBranch built from it)<br>`SET_VAL_RND_EXC` `r` (gecode/set/branch/val.hpp:88:25: returns a SetValBranch built from it)<br>`SET_VAR_RND` `r` (gecode/set/branch/var.hpp:108:21: returns a SetVarBranch built from it)<br>`SET_VAR_AFC_MIN` `a` (gecode/set/branch/var.hpp:138:28: returns a SetVarBranch built from it)<br>`SET_VAR_AFC_MAX` `a` (gecode/set/branch/var.hpp:148:28: returns a SetVarBranch built from it)<br>`SET_VAR_ACTION_MIN` `a` (gecode/set/branch/var.hpp:158:34: returns a SetVarBranch built from it)<br>`SET_VAR_ACTION_MAX` `a` (gecode/set/branch/var.hpp:168:34: returns a SetVarBranch built from it)<br>`SET_VAR_CHB_MIN` `c` (gecode/set/branch/var.hpp:178:28: returns a SetVarBranch built from it)<br>`SET_VAR_CHB_MAX` `c` (gecode/set/branch/var.hpp:188:28: returns a SetVarBranch built from it)<br>`SET_VAR_AFC_SIZE_MIN` `a` (gecode/set/branch/var.hpp:238:33: returns a SetVarBranch built from it)<br>`SET_VAR_AFC_SIZE_MAX` `a` (gecode/set/branch/var.hpp:248:33: returns a SetVarBranch built from it)<br>`SET_VAR_ACTION_SIZE_MIN` `a` (gecode/set/branch/var.hpp:258:39: returns a SetVarBranch built from it)<br>`SET_VAR_ACTION_SIZE_MAX` `a` (gecode/set/branch/var.hpp:268:39: returns a SetVarBranch built from it)<br>`SET_VAR_CHB_SIZE_MIN` `c` (gecode/set/branch/var.hpp:278:33: returns a SetVarBranch built from it)<br>`SET_VAR_CHB_SIZE_MAX` `c` (gecode/set/branch/var.hpp:288:33: returns a SetVarBranch built from it) | BranchTbl ×36, SetBranchCommit ×4, SetBranchMerit ×2, SetBranchVal ×4 |
| G6 float branch configurations | `FLOAT_ASSIGN_RND` `r` (gecode/float/branch/assign.hpp:63:26: returns a FloatAssign built from it)<br>`FLOAT_VAL_SPLIT_RND` `r` (gecode/float/branch/val.hpp:63:29: returns a FloatValBranch built from it)<br>`FLOAT_VAR_RND` `r` (gecode/float/branch/var.hpp:118:23: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_AFC_MIN` `a` (gecode/float/branch/var.hpp:138:32: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_AFC_MAX` `a` (gecode/float/branch/var.hpp:148:32: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_ACTION_MIN` `a` (gecode/float/branch/var.hpp:158:38: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_ACTION_MAX` `a` (gecode/float/branch/var.hpp:168:38: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_CHB_MIN` `c` (gecode/float/branch/var.hpp:178:32: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_CHB_MAX` `c` (gecode/float/branch/var.hpp:188:32: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_AFC_SIZE_MIN` `a` (gecode/float/branch/var.hpp:238:37: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_AFC_SIZE_MAX` `a` (gecode/float/branch/var.hpp:248:37: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_ACTION_SIZE_MIN` `a` (gecode/float/branch/var.hpp:258:43: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_ACTION_SIZE_MAX` `a` (gecode/float/branch/var.hpp:268:43: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_CHB_SIZE_MIN` `c` (gecode/float/branch/var.hpp:278:37: returns a FloatVarBranch built from it)<br>`FLOAT_VAR_CHB_SIZE_MAX` `c` (gecode/float/branch/var.hpp:288:37: returns a FloatVarBranch built from it) | BranchTbl ×36, FloatBranchCommit ×4, FloatBranchMerit ×2, FloatBranchVal ×4 |
| G7 kernel and int brancher posting | — | BoolBranchFilter ×10, BoolVarBranch ×1, BoolVarValPrint ×12, BranchFilter<Var> ×8, BranchFilter<typename Gecode::Int::BoolView::VarType> / BranchFilter<typename Gecode::I... ×4, IntBranchFilter ×13, IntVarBranch ×1, IntVarValPrint ×15, VarValPrint<Var, int> ×4, VarValPrint<Var, int> / VarValPrint<Var, Gecode::FloatNumBranch> ×3, VarValPrint<typename Gecode::Int::BoolView::VarType, int> / VarValPrint<typename Gecode... ×4 |
| G8 set and float brancher posting | — | BranchFilter<Var> ×2, BranchFilter<typename Gecode::Set::SetView::VarType> ×1, FloatBranchFilter ×6, FloatVarValPrint ×8, SetBranchFilter ×8, SetVarValPrint ×10, VarValPrint<Var, int> ×2, VarValPrint<typename Gecode::Set::SetView::VarType, int> ×1 |
| G9 posting with shared data | `OpenRegular::OpenRegular<View, pc>` `dfa0` (gecode/int/extensional/open.hpp:108:62: initializes dfa)<br>`ValueSequenceSymmetryObject::ValueSequenceSymmetryObject` `vs` (gecode/int/ldsb/sym-obj.cpp:97:70: initializes values)<br>`Matrix::Matrix<type-parameter-0-0>` `a` (gecode/minimodel/matrix.hpp:128:25: initializes _a) | — |
| G10 execution callbacks | — | std::function<void (Space &)> ×22 |
| G11 open variable lists | — | Domain ×3, Transition ×3 |
| G12 tracing | `ViewTraceRecorder::ViewTraceRecorder<View>` `tf0` (gecode/kernel/trace/recorder.hpp:260:91: initializes tf)<br>`TraceRecorder::TraceRecorder` `tf0` (gecode/kernel/trace/recorder.hpp:390:57: initializes tf) | — |
| G13 FlatZinc | `BlackBoxBounds::BlackBoxBounds` `sub_int0` (gecode/flatzinc/blackbox-propagator.cpp:286:46: initializes sub_int)<br>`BlackBoxBounds::BlackBoxBounds` `sub_float0` (gecode/flatzinc/blackbox-propagator.cpp:288:46: initializes sub_float)<br>`IntBoolVarBranch::IntBoolVarBranch` `i` (gecode/flatzinc/branch.hpp:40:58: initializes int_afc_data)<br>`IntBoolVarBranch::IntBoolVarBranch` `b` (gecode/flatzinc/branch.hpp:40:69: initializes bool_afc_data)<br>`IntBoolVarBranch::IntBoolVarBranch` `i` (gecode/flatzinc/branch.hpp:44:61: initializes int_action_data)<br>`IntBoolVarBranch::IntBoolVarBranch` `b` (gecode/flatzinc/branch.hpp:44:75: initializes bool_action_data)<br>`IntBoolVarBranch::IntBoolVarBranch` `i` (gecode/flatzinc/branch.hpp:48:58: initializes int_chb_data)<br>`IntBoolVarBranch::IntBoolVarBranch` `b` (gecode/flatzinc/branch.hpp:48:69: initializes bool_chb_data)<br>`int_bool_var_afc_max` `ia` (gecode/flatzinc/branch.hpp:115:33: returns a IntBoolVarBranch built from it)<br>`int_bool_var_afc_max` `ba` (gecode/flatzinc/branch.hpp:115:45: returns a IntBoolVarBranch built from it)<br>`int_bool_var_action_max` `ia` (gecode/flatzinc/branch.hpp:123:39: returns a IntBoolVarBranch built from it)<br>`int_bool_var_action_max` `ba` (gecode/flatzinc/branch.hpp:123:54: returns a IntBoolVarBranch built from it)<br>`int_bool_var_chb_max` `ic` (gecode/flatzinc/branch.hpp:131:33: returns a IntBoolVarBranch built from it)<br>`int_bool_var_chb_max` `bc` (gecode/flatzinc/branch.hpp:131:45: returns a IntBoolVarBranch built from it)<br>`int_bool_var_afc_size_max` `ia` (gecode/flatzinc/branch.hpp:139:38: returns a IntBoolVarBranch built from it)<br>`int_bool_var_afc_size_max` `ba` (gecode/flatzinc/branch.hpp:139:50: returns a IntBoolVarBranch built from it)<br>`int_bool_var_action_size_max` `ia` (gecode/flatzinc/branch.hpp:147:44: returns a IntBoolVarBranch built from it)<br>`int_bool_var_action_size_max` `ba` (gecode/flatzinc/branch.hpp:147:59: returns a IntBoolVarBranch built from it)<br>`int_bool_var_chb_size_max` `ic` (gecode/flatzinc/branch.hpp:155:38: returns a IntBoolVarBranch built from it)<br>`int_bool_var_chb_size_max` `bc` (gecode/flatzinc/branch.hpp:155:50: returns a IntBoolVarBranch built from it) | BoolValBranch ×2, FloatValBranch ×2, IntValBranch ×2, SetValBranch ×2, SetVarBranch ×2, TieBreak<BoolVarBranch> ×2, TieBreak<FloatVarBranch> ×2, TieBreak<IntVarBranch> ×2, std::exception_ptr ×2, std::string ×1, std::vector<BI> ×1 |
| G14 Gist and search | — | std::string ×1 |
| G15 tests and examples | `TupleSetTest::TupleSetTest` `ts0` (test/int/extensional.cpp:1194:92: initializes ts) | BeforeRun ×2, IntValBranch / SetValBranch ×1, OpenIntVarList::Domain ×1, OpenIntVarList::Transition ×1, OutputCheck ×2, std::function<bool ()> ×1, std::function<int (int, int)> ×1, std::shared_ptr<const Graph> ×1, std::string ×3, std::vector<std::string> ×4 |
| G16 FloatVal (only without the FloatVal exception) | `Lin::Lin<P, N, pc>` `c0` (gecode/float/linear/nary.hpp:43:80: initializes c)<br>`ReEqFloat::ReEqFloat<View, CtrlView, rm>` `c0` (gecode/float/rel/eq.hpp:272:74: initializes c)<br>`ReLqFloat::ReLqFloat<View, CtrlView, rm>` `c0` (gecode/float/rel/lq-le.hpp:226:74: initializes c)<br>`ReLeFloat::ReLeFloat<View, CtrlView, rm>` `c0` (gecode/float/rel/lq-le.hpp:313:74: initializes c)<br>`NqFloat::NqFloat<View>` `_c` (gecode/float/rel/nq.hpp:86:56: initializes c)<br>`ScaleView::ScaleView` `d` (gecode/float/view/scale.hpp:44:35: initializes a)<br>`DomArgs::DomArgs<Args...>` `args` (gecode/minimodel/dom.cpp:105:43: initializes _args)<br>`LinFloatExpr::LinFloatExpr` `a` (gecode/minimodel/float-expr.cpp:257:60: assigns n->a)<br>`LinFloatExpr::LinFloatExpr` `a` (gecode/minimodel/float-expr.cpp:311:41: assigns n->a)<br>`LinFloatExpr::Node::fill` `m` (gecode/minimodel/float-expr.cpp:345:76: assigns tf->a, assigns a)<br>`LinFloatRel::LinFloatRel` `r` (gecode/minimodel/float-rel.hpp:49:81: initializes e)<br>`LinFloatRel::LinFloatRel` `l` (gecode/minimodel/float-rel.hpp:53:39: initializes e)<br>`Val::Val` `c0` (test/float/dom.cpp:57:41: initializes c) | — |

## Per-parameter inventory

Columns:

- **Definition**: the parameter in the function definition, as
  file:line:column.
- **Other declarations**: every other declaration of the function that must
  change with it, as the file and line of the parameter. Found with libclang
  (see [What the diagnostics mean here](#what-the-diagnostics-mean-here)).
  "none" means the definition is the only declaration.
- **Function**: the enclosing function, qualified by its classes.
- **Parameter**: the parameter name. Unnamed parameters show the check's
  `#n`.
- **Type**: as reported, with aliases kept (for example `BranchTbl`). Where
  several template instantiations report the same parameter, their types are
  joined with `/`.
- **Callee use**: what the definition does with the parameter, from its
  source text. "keeps: …" means the callee keeps a copy: it initializes or
  assigns a member, stores via a container call, captures it in a lambda,
  returns it, or returns an object or local built from it. "Copies into
  local x" means a local copy that is not returned. "Passes to f" means an
  argument to `f`, and "passes to base or delegated constructor" an
  initializer that forwards it; follow the chain to the next row. "Copied
  once" is the check's move diagnostic. "Reads only" means none of these.
- **Treatment**: `const&`, or by value plus `std::move`, with notes for the
  delicate cases.
- **Coupling**: virtual overrides, explicit specializations, or "declarations
  to the left" when the row has other declarations. "none" means there is no
  other declaration and no caller has to change.

### G1 CBS virtual hooks (4 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/int/distinct/cbs.hpp:135:99 | gecode/int/distinct.hh:99 | cbs_distinct | send | Propagator::SendMarginal | reads only | const& (base and all overrides together) | declarations to the left |
| gecode/int/distinct/cbs.hpp:211:63 | gecode/int/distinct.hh:102 | cbs_size | in | Propagator::InDecision | reads only | const& (base and all overrides together) | declarations to the left |
| gecode/kernel/core.hpp:3781:59 | gecode/kernel/core.hpp:1259 | Propagator::solution_distribution | #2 | SendMarginal | unnamed (unused) | const& (base and all overrides together) | virtual Propagator::solution_distribution: base gecode/kernel/core.hpp:1259 and overrides gecode/int/distinct.hh:75,165,291 (defs gecode/int/distinct/bnd.hpp:462, val.hpp:160, dom.hpp:80) |
| gecode/kernel/core.hpp:3784:43 | gecode/kernel/core.hpp:1269 | Propagator::domain_size_sum | #1 | InDecision | unnamed (unused) | const& (base and all overrides together) | virtual Propagator::domain_size_sum: base gecode/kernel/core.hpp:1269 and overrides gecode/int/distinct.hh:77,167,293 (defs gecode/int/distinct/bnd.hpp:468, val.hpp:166, dom.hpp:86) |

### G3 kernel branch configurations (27 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/kernel/branch/action.hpp:272:83 | gecode/kernel/branch/action.hpp:58 | Action::Storage::Storage | bm | typename BranchTraits<typename Gecode::Int::BoolView::VarType>::Merit / typename Branch... | reads only | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/kernel/branch/action.hpp:354:71 | gecode/kernel/branch/action.hpp:108 | Action::init | bm | typename BranchTraits<typename Gecode::Int::BoolView::VarType>::Merit / typename Branch... | passes to new Storage; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/kernel/branch/chb.hpp:257:111 | gecode/kernel/branch/chb.hpp:72 | CHB::Storage::Storage | bm | typename BranchTraits<typename Gecode::Int::BoolView::VarType>::Merit / typename Branch... | reads only | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/kernel/branch/chb.hpp:344:99 | gecode/kernel/branch/chb.hpp:118 | CHB::init | bm | typename BranchTraits<typename Gecode::Int::BoolView::VarType>::Merit / typename Branch... | passes to new Storage; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:65:45 | gecode/kernel/branch/tiebreak.hpp:42 | TieBreak::TieBreak<VarBranch> | a0 | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: initializes a; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:65:59 | gecode/kernel/branch/tiebreak.hpp:42 | TieBreak::TieBreak<VarBranch> | b0 | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: initializes b; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:65:73 | gecode/kernel/branch/tiebreak.hpp:42 | TieBreak::TieBreak<VarBranch> | c0 | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: initializes c; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:65:87 | gecode/kernel/branch/tiebreak.hpp:43 | TieBreak::TieBreak<VarBranch> | d0 | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: initializes d; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:70:25 | gecode/kernel/branch/tiebreak.hpp:54 | tie_break | a | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds ab, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:70:38 | gecode/kernel/branch/tiebreak.hpp:54 | tie_break | b | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds ab, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:77:25 | gecode/kernel/branch/tiebreak.hpp:57 | tie_break | a | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds abc, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:77:38 | gecode/kernel/branch/tiebreak.hpp:57 | tie_break | b | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds abc, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:77:51 | gecode/kernel/branch/tiebreak.hpp:57 | tie_break | c | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds abc, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:84:25 | gecode/kernel/branch/tiebreak.hpp:60 | tie_break | a | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds abcd, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:84:38 | gecode/kernel/branch/tiebreak.hpp:60 | tie_break | b | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds abcd, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:84:51 | gecode/kernel/branch/tiebreak.hpp:60 | tie_break | c | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds abcd, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/tiebreak.hpp:84:64 | gecode/kernel/branch/tiebreak.hpp:60 | tie_break | d | BoolVarBranch / FloatVarBranch / IntVarBranch / SetVarBranch | keeps: builds abcd, which it returns; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/val.hpp:74:42 | gecode/kernel/branch/val.hpp:59 | ValBranch::ValBranch<Var> | r0 | Rnd | keeps: initializes r | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/branch/val.hpp:80:48 | gecode/kernel/branch/val.hpp:61 | ValBranch::ValBranch<Var> | v | BranchVal | keeps: initializes vf; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/val.hpp:80:64 | gecode/kernel/branch/val.hpp:61 | ValBranch::ValBranch<Var> | c | BranchCommit | keeps: initializes cf; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/kernel/branch/var.hpp:125:42 | gecode/kernel/branch/var.hpp:85 | VarBranch::VarBranch<Var> | a | AFC | keeps: initializes _afc | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/branch/var.hpp:131:45 | gecode/kernel/branch/var.hpp:87 | VarBranch::VarBranch<Var> | a | Action | keeps: initializes _act | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/branch/var.hpp:137:42 | gecode/kernel/branch/var.hpp:89 | VarBranch::VarBranch<Var> | c | CHB | keeps: initializes _chb | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/branch/var.hpp:143:42 | gecode/kernel/branch/var.hpp:81 | VarBranch::VarBranch<Var> | r | Rnd | keeps: initializes _rnd | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/branch/var.hpp:178:29 | gecode/kernel/branch/var.hpp:101 | VarBranch::afc | a | AFC | keeps: assigns _afc | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/branch/var.hpp:190:35 | gecode/kernel/branch/var.hpp:105 | VarBranch::action | a | Action | keeps: assigns _act | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/branch/var.hpp:202:29 | gecode/kernel/branch/var.hpp:109 | VarBranch::chb | chb | CHB | keeps: assigns _chb | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |

### G4 int branch configurations (120 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/int/branch/action.cpp:37:99 | gecode/int.hh:5029 | IntAction::IntAction | bm | IntBranchMerit | passes to Action::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/int/branch/action.cpp:43:94 | gecode/int.hh:5044 | IntAction::init | bm | IntBranchMerit | passes to Action::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/int/branch/action.cpp:48:103 | gecode/int.hh:5077 | BoolAction::BoolAction | bm | BoolBranchMerit | passes to Action::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/int/branch/action.cpp:54:97 | gecode/int.hh:5092 | BoolAction::init | bm | BoolBranchMerit | passes to Action::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/int/branch/assign.hpp:40:30 | gecode/int.hh:5640 | IntAssign::IntAssign | r | Rnd | passes to base or delegated constructor ValBranch<IntVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/assign.hpp:44:39 | gecode/int.hh:5642 | IntAssign::IntAssign | v | IntBranchVal | passes to base or delegated constructor ValBranch<IntVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch/assign.hpp:44:58 | gecode/int.hh:5642 | IntAssign::IntAssign | c | IntBranchCommit | passes to base or delegated constructor ValBranch<IntVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch/assign.hpp:68:24 | gecode/int.hh:5689 | INT_ASSIGN_RND | r | Rnd | keeps: returns a IntAssign built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/assign.hpp:73:29 | gecode/int.hh:5696 | INT_ASSIGN | v | IntBranchVal | keeps: returns a IntAssign built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/assign.hpp:73:48 | gecode/int.hh:5696 | INT_ASSIGN | c | IntBranchCommit | keeps: returns a IntAssign built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/assign.hpp:82:32 | gecode/int.hh:5670 | BoolAssign::BoolAssign | r | Rnd | passes to base or delegated constructor ValBranch<BoolVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/assign.hpp:86:42 | gecode/int.hh:5672 | BoolAssign::BoolAssign | v | BoolBranchVal | passes to base or delegated constructor ValBranch<BoolVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch/assign.hpp:86:62 | gecode/int.hh:5672 | BoolAssign::BoolAssign | c | BoolBranchCommit | passes to base or delegated constructor ValBranch<BoolVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch/assign.hpp:105:25 | gecode/int.hh:5703 | BOOL_ASSIGN_RND | r | Rnd | keeps: returns a BoolAssign built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/assign.hpp:110:31 | gecode/int.hh:5710 | BOOL_ASSIGN | v | BoolBranchVal | keeps: returns a BoolAssign built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/assign.hpp:110:51 | gecode/int.hh:5710 | BOOL_ASSIGN | c | BoolBranchCommit | keeps: returns a BoolAssign built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/chb.cpp:37:67 | gecode/int.hh:5129 | IntCHB::IntCHB | bm | IntBranchMerit | passes to CHB::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/int/branch/chb.cpp:43:65 | gecode/int.hh:5141 | IntCHB::init | bm | IntBranchMerit | passes to CHB::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/int/branch/chb.cpp:48:71 | gecode/int.hh:5172 | BoolCHB::BoolCHB | bm | BoolBranchMerit | passes to CHB::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/int/branch/chb.cpp:54:68 | gecode/int.hh:5184 | BoolCHB::init | bm | BoolBranchMerit | passes to CHB::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/int/branch/val.hpp:40:36 | gecode/int.hh:5525 | IntValBranch::IntValBranch | r | Rnd | passes to base or delegated constructor ValBranch<IntVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/val.hpp:44:45 | gecode/int.hh:5527 | IntValBranch::IntValBranch | v | IntBranchVal | passes to base or delegated constructor ValBranch<IntVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch/val.hpp:44:64 | gecode/int.hh:5527 | IntValBranch::IntValBranch | c | IntBranchCommit | passes to base or delegated constructor ValBranch<IntVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch/val.hpp:68:21 | gecode/int.hh:5574 | INT_VAL_RND | r | Rnd | keeps: returns a IntValBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/val.hpp:93:26 | gecode/int.hh:5589 | INT_VAL | v | IntBranchVal | keeps: returns a IntValBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/val.hpp:93:45 | gecode/int.hh:5589 | INT_VAL | c | IntBranchCommit | keeps: returns a IntValBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/val.hpp:112:38 | gecode/int.hh:5555 | BoolValBranch::BoolValBranch | r | Rnd | passes to base or delegated constructor ValBranch<BoolVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/val.hpp:116:48 | gecode/int.hh:5557 | BoolValBranch::BoolValBranch | v | BoolBranchVal | passes to base or delegated constructor ValBranch<BoolVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch/val.hpp:116:68 | gecode/int.hh:5557 | BoolValBranch::BoolValBranch | c | BoolBranchCommit | passes to base or delegated constructor ValBranch<BoolVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch/val.hpp:135:22 | gecode/int.hh:5600 | BOOL_VAL_RND | r | Rnd | keeps: returns a BoolValBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/val.hpp:140:28 | gecode/int.hh:5607 | BOOL_VAL | v | BoolBranchVal | keeps: returns a BoolValBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/val.hpp:140:48 | gecode/int.hh:5607 | BOOL_VAL | c | BoolBranchCommit | keeps: returns a BoolValBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:46:36 | gecode/int.hh:5276 | IntVarBranch::IntVarBranch | r | Rnd | passes to base or delegated constructor VarBranch<IntVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/var.hpp:54:50 | gecode/int.hh:5282 | IntVarBranch::IntVarBranch | a | IntAFC | passes to base or delegated constructor VarBranch<IntVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/var.hpp:58:53 | gecode/int.hh:5284 | IntVarBranch::IntVarBranch | a | IntAction | passes to base or delegated constructor VarBranch<IntVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/var.hpp:62:50 | gecode/int.hh:5286 | IntVarBranch::IntVarBranch | c | IntCHB | passes to base or delegated constructor VarBranch<IntVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/var.hpp:108:21 | gecode/int.hh:5353 | INT_VAR_RND | r | Rnd | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:113:38 | gecode/int.hh:5355 | INT_VAR_MERIT_MIN | bm | IntBranchMerit | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:113:52 | gecode/int.hh:5355 | INT_VAR_MERIT_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:118:38 | gecode/int.hh:5357 | INT_VAR_MERIT_MAX | bm | IntBranchMerit | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:118:52 | gecode/int.hh:5357 | INT_VAR_MERIT_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:123:34 | gecode/int.hh:5359 | INT_VAR_DEGREE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:128:34 | gecode/int.hh:5361 | INT_VAR_DEGREE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:133:41 | gecode/int.hh:5363 | INT_VAR_AFC_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:138:28 | gecode/int.hh:5365 | INT_VAR_AFC_MIN | a | IntAFC | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:138:41 | gecode/int.hh:5365 | INT_VAR_AFC_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:143:41 | gecode/int.hh:5367 | INT_VAR_AFC_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:148:28 | gecode/int.hh:5369 | INT_VAR_AFC_MAX | a | IntAFC | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:148:41 | gecode/int.hh:5369 | INT_VAR_AFC_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:153:44 | gecode/int.hh:5371 | INT_VAR_ACTION_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:158:34 | gecode/int.hh:5373 | INT_VAR_ACTION_MIN | a | IntAction | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:158:47 | gecode/int.hh:5373 | INT_VAR_ACTION_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:163:44 | gecode/int.hh:5375 | INT_VAR_ACTION_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:168:34 | gecode/int.hh:5377 | INT_VAR_ACTION_MAX | a | IntAction | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:168:47 | gecode/int.hh:5377 | INT_VAR_ACTION_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:173:28 | gecode/int.hh:5379 | INT_VAR_CHB_MIN | c | IntCHB | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:173:41 | gecode/int.hh:5379 | INT_VAR_CHB_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:178:31 | gecode/int.hh:5381 | INT_VAR_CHB_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:183:28 | gecode/int.hh:5383 | INT_VAR_CHB_MAX | c | IntCHB | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:183:41 | gecode/int.hh:5383 | INT_VAR_CHB_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:188:31 | gecode/int.hh:5385 | INT_VAR_CHB_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:193:31 | gecode/int.hh:5387 | INT_VAR_MIN_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:198:31 | gecode/int.hh:5389 | INT_VAR_MIN_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:203:31 | gecode/int.hh:5391 | INT_VAR_MAX_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:208:31 | gecode/int.hh:5393 | INT_VAR_MAX_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:213:32 | gecode/int.hh:5395 | INT_VAR_SIZE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:218:32 | gecode/int.hh:5397 | INT_VAR_SIZE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:223:39 | gecode/int.hh:5399 | INT_VAR_DEGREE_SIZE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:228:39 | gecode/int.hh:5401 | INT_VAR_DEGREE_SIZE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:233:46 | gecode/int.hh:5403 | INT_VAR_AFC_SIZE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:238:33 | gecode/int.hh:5405 | INT_VAR_AFC_SIZE_MIN | a | IntAFC | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:238:46 | gecode/int.hh:5405 | INT_VAR_AFC_SIZE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:243:46 | gecode/int.hh:5407 | INT_VAR_AFC_SIZE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:248:33 | gecode/int.hh:5409 | INT_VAR_AFC_SIZE_MAX | a | IntAFC | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:248:46 | gecode/int.hh:5409 | INT_VAR_AFC_SIZE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:253:49 | gecode/int.hh:5411 | INT_VAR_ACTION_SIZE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:258:39 | gecode/int.hh:5413 | INT_VAR_ACTION_SIZE_MIN | a | IntAction | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:258:52 | gecode/int.hh:5413 | INT_VAR_ACTION_SIZE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:263:49 | gecode/int.hh:5415 | INT_VAR_ACTION_SIZE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:268:39 | gecode/int.hh:5417 | INT_VAR_ACTION_SIZE_MAX | a | IntAction | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:268:52 | gecode/int.hh:5417 | INT_VAR_ACTION_SIZE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:273:33 | gecode/int.hh:5419 | INT_VAR_CHB_SIZE_MIN | c | IntCHB | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:273:46 | gecode/int.hh:5419 | INT_VAR_CHB_SIZE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:278:36 | gecode/int.hh:5421 | INT_VAR_CHB_SIZE_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:283:33 | gecode/int.hh:5423 | INT_VAR_CHB_SIZE_MAX | c | IntCHB | keeps: returns a IntVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:283:46 | gecode/int.hh:5423 | INT_VAR_CHB_SIZE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:288:36 | gecode/int.hh:5425 | INT_VAR_CHB_SIZE_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:293:38 | gecode/int.hh:5431 | INT_VAR_REGRET_MIN_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:298:38 | gecode/int.hh:5437 | INT_VAR_REGRET_MIN_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:303:38 | gecode/int.hh:5443 | INT_VAR_REGRET_MAX_MIN | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:308:38 | gecode/int.hh:5449 | INT_VAR_REGRET_MAX_MAX | tbl | BranchTbl | keeps: returns a IntVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:321:38 | gecode/int.hh:5326 | BoolVarBranch::BoolVarBranch | r | Rnd | passes to base or delegated constructor VarBranch<BoolVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/var.hpp:329:53 | gecode/int.hh:5332 | BoolVarBranch::BoolVarBranch | a | BoolAFC | passes to base or delegated constructor VarBranch<BoolVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/var.hpp:333:56 | gecode/int.hh:5334 | BoolVarBranch::BoolVarBranch | a | BoolAction | passes to base or delegated constructor VarBranch<BoolVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/var.hpp:337:53 | gecode/int.hh:5336 | BoolVarBranch::BoolVarBranch | c | BoolCHB | passes to base or delegated constructor VarBranch<BoolVar> | const& (no move constructor) | declarations to the left |
| gecode/int/branch/var.hpp:377:22 | gecode/int.hh:5454 | BOOL_VAR_RND | r | Rnd | keeps: returns a BoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:382:40 | gecode/int.hh:5456 | BOOL_VAR_MERIT_MIN | bm | BoolBranchMerit | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:382:54 | gecode/int.hh:5456 | BOOL_VAR_MERIT_MIN | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:387:40 | gecode/int.hh:5458 | BOOL_VAR_MERIT_MAX | bm | BoolBranchMerit | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:387:54 | gecode/int.hh:5458 | BOOL_VAR_MERIT_MAX | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:392:35 | gecode/int.hh:5460 | BOOL_VAR_DEGREE_MIN | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:397:35 | gecode/int.hh:5462 | BOOL_VAR_DEGREE_MAX | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:402:42 | gecode/int.hh:5464 | BOOL_VAR_AFC_MIN | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:407:30 | gecode/int.hh:5466 | BOOL_VAR_AFC_MIN | a | BoolAFC | keeps: returns a BoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:407:43 | gecode/int.hh:5466 | BOOL_VAR_AFC_MIN | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:412:42 | gecode/int.hh:5468 | BOOL_VAR_AFC_MAX | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:417:30 | gecode/int.hh:5470 | BOOL_VAR_AFC_MAX | a | BoolAFC | keeps: returns a BoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:417:43 | gecode/int.hh:5470 | BOOL_VAR_AFC_MAX | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:422:45 | gecode/int.hh:5472 | BOOL_VAR_ACTION_MIN | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:427:36 | gecode/int.hh:5474 | BOOL_VAR_ACTION_MIN | a | BoolAction | keeps: returns a BoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:427:49 | gecode/int.hh:5474 | BOOL_VAR_ACTION_MIN | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:432:45 | gecode/int.hh:5476 | BOOL_VAR_ACTION_MAX | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:437:36 | gecode/int.hh:5478 | BOOL_VAR_ACTION_MAX | a | BoolAction | keeps: returns a BoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:437:49 | gecode/int.hh:5478 | BOOL_VAR_ACTION_MAX | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:442:30 | gecode/int.hh:5480 | BOOL_VAR_CHB_MIN | c | BoolCHB | keeps: returns a BoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:442:43 | gecode/int.hh:5480 | BOOL_VAR_CHB_MIN | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:447:32 | gecode/int.hh:5482 | BOOL_VAR_CHB_MIN | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:452:30 | gecode/int.hh:5484 | BOOL_VAR_CHB_MAX | c | BoolCHB | keeps: returns a BoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/branch/var.hpp:452:43 | gecode/int.hh:5484 | BOOL_VAR_CHB_MAX | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/int/branch/var.hpp:457:32 | gecode/int.hh:5486 | BOOL_VAR_CHB_MAX | tbl | BranchTbl | keeps: returns a BoolVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |

### G5 set branch configurations (73 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/set/branch/action.cpp:37:99 | gecode/set.hh:1109 | SetAction::SetAction | bm | SetBranchMerit | passes to Action::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/set/branch/action.cpp:43:94 | gecode/set.hh:1124 | SetAction::init | bm | SetBranchMerit | passes to Action::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/set/branch/assign.hpp:40:41 | gecode/set.hh:1444 | SetAssign::SetAssign | r | Rnd | passes to base or delegated constructor ValBranch<SetVar> | const& (no move constructor) | declarations to the left |
| gecode/set/branch/assign.hpp:44:39 | gecode/set.hh:1446 | SetAssign::SetAssign | v | SetBranchVal | passes to base or delegated constructor ValBranch<SetVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/set/branch/assign.hpp:44:58 | gecode/set.hh:1446 | SetAssign::SetAssign | c | SetBranchCommit | passes to base or delegated constructor ValBranch<SetVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/set/branch/assign.hpp:83:28 | gecode/set.hh:1469 | SET_ASSIGN_RND_INC | r | Rnd | keeps: returns a SetAssign built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/assign.hpp:88:28 | gecode/set.hh:1471 | SET_ASSIGN_RND_EXC | r | Rnd | keeps: returns a SetAssign built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/assign.hpp:93:29 | gecode/set.hh:1478 | SET_ASSIGN | v | SetBranchVal | keeps: returns a SetAssign built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/assign.hpp:93:48 | gecode/set.hh:1478 | SET_ASSIGN | c | SetBranchCommit | keeps: returns a SetAssign built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/chb.cpp:37:67 | gecode/set.hh:1161 | SetCHB::SetCHB | bm | SetBranchMerit | passes to CHB::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/set/branch/chb.cpp:43:65 | gecode/set.hh:1173 | SetCHB::init | bm | SetBranchMerit | passes to CHB::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/set/branch/val.hpp:40:47 | gecode/set.hh:1372 | SetValBranch::SetValBranch | r | Rnd | passes to base or delegated constructor ValBranch<SetVar> | const& (no move constructor) | declarations to the left |
| gecode/set/branch/val.hpp:44:45 | gecode/set.hh:1374 | SetValBranch::SetValBranch | v | SetBranchVal | passes to base or delegated constructor ValBranch<SetVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/set/branch/val.hpp:44:64 | gecode/set.hh:1374 | SetValBranch::SetValBranch | c | SetBranchCommit | passes to base or delegated constructor ValBranch<SetVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/set/branch/val.hpp:83:25 | gecode/set.hh:1397 | SET_VAL_RND_INC | r | Rnd | keeps: returns a SetValBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/val.hpp:88:25 | gecode/set.hh:1399 | SET_VAL_RND_EXC | r | Rnd | keeps: returns a SetValBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/val.hpp:93:26 | gecode/set.hh:1407 | SET_VAL | v | SetBranchVal | keeps: returns a SetValBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/val.hpp:93:45 | gecode/set.hh:1407 | SET_VAL | c | SetBranchCommit | keeps: returns a SetValBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:46:36 | gecode/set.hh:1236 | SetVarBranch::SetVarBranch | r | Rnd | passes to base or delegated constructor VarBranch<SetVar> | const& (no move constructor) | declarations to the left |
| gecode/set/branch/var.hpp:54:50 | gecode/set.hh:1242 | SetVarBranch::SetVarBranch | a | SetAFC | passes to base or delegated constructor VarBranch<SetVar> | const& (no move constructor) | declarations to the left |
| gecode/set/branch/var.hpp:58:53 | gecode/set.hh:1244 | SetVarBranch::SetVarBranch | a | SetAction | passes to base or delegated constructor VarBranch<SetVar> | const& (no move constructor) | declarations to the left |
| gecode/set/branch/var.hpp:62:50 | gecode/set.hh:1246 | SetVarBranch::SetVarBranch | c | SetCHB | passes to base or delegated constructor VarBranch<SetVar> | const& (no move constructor) | declarations to the left |
| gecode/set/branch/var.hpp:108:21 | gecode/set.hh:1263 | SET_VAR_RND | r | Rnd | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:113:38 | gecode/set.hh:1265 | SET_VAR_MERIT_MIN | bm | SetBranchMerit | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:113:52 | gecode/set.hh:1265 | SET_VAR_MERIT_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:118:38 | gecode/set.hh:1267 | SET_VAR_MERIT_MAX | bm | SetBranchMerit | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:118:52 | gecode/set.hh:1267 | SET_VAR_MERIT_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:123:34 | gecode/set.hh:1269 | SET_VAR_DEGREE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:128:34 | gecode/set.hh:1271 | SET_VAR_DEGREE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:133:41 | gecode/set.hh:1273 | SET_VAR_AFC_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:138:28 | gecode/set.hh:1275 | SET_VAR_AFC_MIN | a | SetAFC | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:138:41 | gecode/set.hh:1275 | SET_VAR_AFC_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:143:41 | gecode/set.hh:1277 | SET_VAR_AFC_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:148:28 | gecode/set.hh:1279 | SET_VAR_AFC_MAX | a | SetAFC | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:148:41 | gecode/set.hh:1279 | SET_VAR_AFC_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:153:44 | gecode/set.hh:1281 | SET_VAR_ACTION_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:158:34 | gecode/set.hh:1283 | SET_VAR_ACTION_MIN | a | SetAction | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:158:47 | gecode/set.hh:1283 | SET_VAR_ACTION_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:163:44 | gecode/set.hh:1285 | SET_VAR_ACTION_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:168:34 | gecode/set.hh:1287 | SET_VAR_ACTION_MAX | a | SetAction | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:168:47 | gecode/set.hh:1287 | SET_VAR_ACTION_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:173:31 | gecode/set.hh:1289 | SET_VAR_CHB_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:178:28 | gecode/set.hh:1291 | SET_VAR_CHB_MIN | c | SetCHB | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:178:41 | gecode/set.hh:1291 | SET_VAR_CHB_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:183:31 | gecode/set.hh:1293 | SET_VAR_CHB_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:188:28 | gecode/set.hh:1295 | SET_VAR_CHB_MAX | c | SetCHB | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:188:41 | gecode/set.hh:1295 | SET_VAR_CHB_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:193:31 | gecode/set.hh:1297 | SET_VAR_MIN_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:198:31 | gecode/set.hh:1299 | SET_VAR_MIN_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:203:31 | gecode/set.hh:1301 | SET_VAR_MAX_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:208:31 | gecode/set.hh:1303 | SET_VAR_MAX_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:213:32 | gecode/set.hh:1305 | SET_VAR_SIZE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:218:32 | gecode/set.hh:1307 | SET_VAR_SIZE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:223:39 | gecode/set.hh:1309 | SET_VAR_DEGREE_SIZE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:228:39 | gecode/set.hh:1311 | SET_VAR_DEGREE_SIZE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:233:46 | gecode/set.hh:1313 | SET_VAR_AFC_SIZE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:238:33 | gecode/set.hh:1315 | SET_VAR_AFC_SIZE_MIN | a | SetAFC | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:238:46 | gecode/set.hh:1315 | SET_VAR_AFC_SIZE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:243:46 | gecode/set.hh:1317 | SET_VAR_AFC_SIZE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:248:33 | gecode/set.hh:1319 | SET_VAR_AFC_SIZE_MAX | a | SetAFC | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:248:46 | gecode/set.hh:1319 | SET_VAR_AFC_SIZE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:253:49 | gecode/set.hh:1321 | SET_VAR_ACTION_SIZE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:258:39 | gecode/set.hh:1323 | SET_VAR_ACTION_SIZE_MIN | a | SetAction | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:258:52 | gecode/set.hh:1323 | SET_VAR_ACTION_SIZE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:263:49 | gecode/set.hh:1325 | SET_VAR_ACTION_SIZE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:268:39 | gecode/set.hh:1327 | SET_VAR_ACTION_SIZE_MAX | a | SetAction | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:268:52 | gecode/set.hh:1327 | SET_VAR_ACTION_SIZE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:273:36 | gecode/set.hh:1329 | SET_VAR_CHB_SIZE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:278:33 | gecode/set.hh:1331 | SET_VAR_CHB_SIZE_MIN | c | SetCHB | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:278:46 | gecode/set.hh:1331 | SET_VAR_CHB_SIZE_MIN | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:283:36 | gecode/set.hh:1333 | SET_VAR_CHB_SIZE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/set/branch/var.hpp:288:33 | gecode/set.hh:1335 | SET_VAR_CHB_SIZE_MAX | c | SetCHB | keeps: returns a SetVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/branch/var.hpp:288:46 | gecode/set.hh:1335 | SET_VAR_CHB_SIZE_MAX | tbl | BranchTbl | keeps: returns a SetVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |

### G6 float branch configurations (71 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/float/branch/action.cpp:37:107 | gecode/float.hh:1516 | FloatAction::FloatAction | bm | FloatBranchMerit | passes to Action::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/float/branch/action.cpp:43:100 | gecode/float.hh:1531 | FloatAction::init | bm | FloatBranchMerit | passes to Action::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/float/branch/assign.hpp:40:34 | gecode/float.hh:1830 | FloatAssign::FloatAssign | r | Rnd | passes to base or delegated constructor ValBranch<FloatVar> | const& (no move constructor) | declarations to the left |
| gecode/float/branch/assign.hpp:44:45 | gecode/float.hh:1832 | FloatAssign::FloatAssign | v | FloatBranchVal | passes to base or delegated constructor ValBranch<FloatVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/float/branch/assign.hpp:44:66 | gecode/float.hh:1832 | FloatAssign::FloatAssign | c | FloatBranchCommit | passes to base or delegated constructor ValBranch<FloatVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/float/branch/assign.hpp:63:26 | gecode/float.hh:1847 | FLOAT_ASSIGN_RND | r | Rnd | keeps: returns a FloatAssign built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/assign.hpp:68:33 | gecode/float.hh:1853 | FLOAT_ASSIGN | v | FloatBranchVal | keeps: returns a FloatAssign built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/assign.hpp:68:54 | gecode/float.hh:1853 | FLOAT_ASSIGN | c | FloatBranchCommit | keeps: returns a FloatAssign built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/chb.cpp:37:75 | gecode/float.hh:1568 | FloatCHB::FloatCHB | bm | FloatBranchMerit | passes to CHB::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/float/branch/chb.cpp:43:71 | gecode/float.hh:1580 | FloatCHB::init | bm | FloatBranchMerit | passes to CHB::init; copied once (last use) | const& (merit only read by Action/CHB storage to seed values) | declarations to the left |
| gecode/float/branch/val.hpp:40:40 | gecode/float.hh:1774 | FloatValBranch::FloatValBranch | r | Rnd | passes to base or delegated constructor ValBranch<FloatVar> | const& (no move constructor) | declarations to the left |
| gecode/float/branch/val.hpp:44:51 | gecode/float.hh:1776 | FloatValBranch::FloatValBranch | v | FloatBranchVal | passes to base or delegated constructor ValBranch<FloatVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/float/branch/val.hpp:44:72 | gecode/float.hh:1776 | FloatValBranch::FloatValBranch | c | FloatBranchCommit | passes to base or delegated constructor ValBranch<FloatVar>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/float/branch/val.hpp:63:29 | gecode/float.hh:1791 | FLOAT_VAL_SPLIT_RND | r | Rnd | keeps: returns a FloatValBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/val.hpp:68:30 | gecode/float.hh:1798 | FLOAT_VAL | v | FloatBranchVal | keeps: returns a FloatValBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/val.hpp:68:51 | gecode/float.hh:1798 | FLOAT_VAL | c | FloatBranchCommit | keeps: returns a FloatValBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:46:40 | gecode/float.hh:1643 | FloatVarBranch::FloatVarBranch | r | Rnd | passes to base or delegated constructor VarBranch<FloatVar> | const& (no move constructor) | declarations to the left |
| gecode/float/branch/var.hpp:54:56 | gecode/float.hh:1649 | FloatVarBranch::FloatVarBranch | a | FloatAFC | passes to base or delegated constructor VarBranch<FloatVar> | const& (no move constructor) | declarations to the left |
| gecode/float/branch/var.hpp:58:59 | gecode/float.hh:1651 | FloatVarBranch::FloatVarBranch | a | FloatAction | passes to base or delegated constructor VarBranch<FloatVar> | const& (no move constructor) | declarations to the left |
| gecode/float/branch/var.hpp:62:56 | gecode/float.hh:1653 | FloatVarBranch::FloatVarBranch | c | FloatCHB | passes to base or delegated constructor VarBranch<FloatVar> | const& (no move constructor) | declarations to the left |
| gecode/float/branch/var.hpp:108:42 | gecode/float.hh:1672 | FLOAT_VAR_MERIT_MIN | bm | FloatBranchMerit | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:108:56 | gecode/float.hh:1672 | FLOAT_VAR_MERIT_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:113:42 | gecode/float.hh:1674 | FLOAT_VAR_MERIT_MAX | bm | FloatBranchMerit | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:113:56 | gecode/float.hh:1674 | FLOAT_VAR_MERIT_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:118:23 | gecode/float.hh:1670 | FLOAT_VAR_RND | r | Rnd | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:123:36 | gecode/float.hh:1676 | FLOAT_VAR_DEGREE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:128:36 | gecode/float.hh:1678 | FLOAT_VAR_DEGREE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:133:43 | gecode/float.hh:1680 | FLOAT_VAR_AFC_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:138:32 | gecode/float.hh:1682 | FLOAT_VAR_AFC_MIN | a | FloatAFC | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:138:45 | gecode/float.hh:1682 | FLOAT_VAR_AFC_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:143:43 | gecode/float.hh:1684 | FLOAT_VAR_AFC_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:148:32 | gecode/float.hh:1686 | FLOAT_VAR_AFC_MAX | a | FloatAFC | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:148:45 | gecode/float.hh:1686 | FLOAT_VAR_AFC_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:153:46 | gecode/float.hh:1688 | FLOAT_VAR_ACTION_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:158:38 | gecode/float.hh:1690 | FLOAT_VAR_ACTION_MIN | a | FloatAction | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:158:51 | gecode/float.hh:1690 | FLOAT_VAR_ACTION_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:163:46 | gecode/float.hh:1692 | FLOAT_VAR_ACTION_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:168:38 | gecode/float.hh:1694 | FLOAT_VAR_ACTION_MAX | a | FloatAction | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:168:51 | gecode/float.hh:1694 | FLOAT_VAR_ACTION_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:173:33 | gecode/float.hh:1696 | FLOAT_VAR_CHB_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:178:32 | gecode/float.hh:1698 | FLOAT_VAR_CHB_MIN | c | FloatCHB | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:178:45 | gecode/float.hh:1698 | FLOAT_VAR_CHB_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:183:33 | gecode/float.hh:1700 | FLOAT_VAR_CHB_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:188:32 | gecode/float.hh:1702 | FLOAT_VAR_CHB_MAX | c | FloatCHB | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:188:45 | gecode/float.hh:1702 | FLOAT_VAR_CHB_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:193:33 | gecode/float.hh:1704 | FLOAT_VAR_MIN_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:198:33 | gecode/float.hh:1706 | FLOAT_VAR_MIN_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:203:33 | gecode/float.hh:1708 | FLOAT_VAR_MAX_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:208:33 | gecode/float.hh:1710 | FLOAT_VAR_MAX_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:213:34 | gecode/float.hh:1712 | FLOAT_VAR_SIZE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:218:34 | gecode/float.hh:1714 | FLOAT_VAR_SIZE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:223:41 | gecode/float.hh:1716 | FLOAT_VAR_DEGREE_SIZE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:228:41 | gecode/float.hh:1718 | FLOAT_VAR_DEGREE_SIZE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:233:48 | gecode/float.hh:1720 | FLOAT_VAR_AFC_SIZE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:238:37 | gecode/float.hh:1722 | FLOAT_VAR_AFC_SIZE_MIN | a | FloatAFC | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:238:50 | gecode/float.hh:1722 | FLOAT_VAR_AFC_SIZE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:243:48 | gecode/float.hh:1724 | FLOAT_VAR_AFC_SIZE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:248:37 | gecode/float.hh:1726 | FLOAT_VAR_AFC_SIZE_MAX | a | FloatAFC | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:248:50 | gecode/float.hh:1726 | FLOAT_VAR_AFC_SIZE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:253:51 | gecode/float.hh:1728 | FLOAT_VAR_ACTION_SIZE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:258:43 | gecode/float.hh:1730 | FLOAT_VAR_ACTION_SIZE_MIN | a | FloatAction | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:258:56 | gecode/float.hh:1730 | FLOAT_VAR_ACTION_SIZE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:263:51 | gecode/float.hh:1732 | FLOAT_VAR_ACTION_SIZE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:268:43 | gecode/float.hh:1734 | FLOAT_VAR_ACTION_SIZE_MAX | a | FloatAction | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:268:56 | gecode/float.hh:1734 | FLOAT_VAR_ACTION_SIZE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:273:38 | gecode/float.hh:1736 | FLOAT_VAR_CHB_SIZE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:278:37 | gecode/float.hh:1738 | FLOAT_VAR_CHB_SIZE_MIN | c | FloatCHB | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:278:50 | gecode/float.hh:1738 | FLOAT_VAR_CHB_SIZE_MIN | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:283:38 | gecode/float.hh:1740 | FLOAT_VAR_CHB_SIZE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/float/branch/var.hpp:288:37 | gecode/float.hh:1742 | FLOAT_VAR_CHB_SIZE_MAX | c | FloatCHB | keeps: returns a FloatVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/branch/var.hpp:288:50 | gecode/float.hh:1742 | FLOAT_VAR_CHB_SIZE_MAX | tbl | BranchTbl | keeps: returns a FloatVarBranch built from it; copied once (last use) | by value + std::move into storage | declarations to the left |

### G7 kernel and int brancher posting (111 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/int/branch.cpp:38:76 | gecode/int.hh:5724 | branch | vals | IntValBranch | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:38:98 | gecode/int.hh:5725 | branch | bf | IntBranchFilter | passes to Branch::post_view_values_brancher<1, true>, Branch::post_view_values_brancher<1, false>, post_view_value_brancher<IntView, 1, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:39:27 | gecode/int.hh:5725 | branch | vvp | IntVarValPrint | passes to Branch::post_view_values_brancher<1, true>, Branch::post_view_values_brancher<1, false>, post_view_value_brancher<IntView, 1, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:61:86 | gecode/int.hh:5731 | branch | vals | IntValBranch | passes to branch, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:61:108 | gecode/int.hh:5732 | branch | bf | IntBranchFilter | passes to branch, Branch::post_view_values_brancher<2, true>, Branch::post_view_values_brancher<2, false>, post_view_value_brancher<IntView, 2, int, 2>, Branch::post_view_values_brancher<3, true>, Branch::post_view_values_brancher<3, false>, post_view_value_brancher<IntView, 3, int, 2>, Branch::post_view_values_brancher<4, true>, Branch::post_view_values_brancher<4, false>, post_view_value_brancher<IntView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:62:27 | gecode/int.hh:5732 | branch | vvp | IntVarValPrint | passes to branch, Branch::post_view_values_brancher<2, true>, Branch::post_view_values_brancher<2, false>, post_view_value_brancher<IntView, 2, int, 2>, Branch::post_view_values_brancher<3, true>, Branch::post_view_values_brancher<3, false>, post_view_value_brancher<IntView, 3, int, 2>, Branch::post_view_values_brancher<4, true>, Branch::post_view_values_brancher<4, false>, post_view_value_brancher<IntView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:126:46 | gecode/int.hh:5738 | branch | vals | IntValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:126:67 | gecode/int.hh:5738 | branch | vvp | IntVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:133:57 | gecode/int.hh:5819 | assign | vars | IntVarBranch | passes to ViewSelNone<IntView> | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:133:73 | gecode/int.hh:5819 | assign | vals | IntAssign | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:133:95 | gecode/int.hh:5820 | assign | bf | IntBranchFilter | passes to post_view_value_brancher<IntView, 1, int, 1>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:133:114 | gecode/int.hh:5820 | assign | vvp | IntVarValPrint | passes to post_view_value_brancher<IntView, 1, int, 1>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:143:83 | none | branch | vals | IntAssign | passes to assign, Branch::value_selection_commit | const& (read only; copies come from accessors) | none |
| gecode/int/branch.cpp:143:105 | none | branch | bf | IntBranchFilter | passes to assign, post_view_value_brancher<IntView, 2, int, 1>, post_view_value_brancher<IntView, 3, int, 1>, post_view_value_brancher<IntView, 4, int, 1> | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch.cpp:144:27 | none | branch | vvp | IntVarValPrint | passes to assign, post_view_value_brancher<IntView, 2, int, 1>, post_view_value_brancher<IntView, 3, int, 1>, post_view_value_brancher<IntView, 4, int, 1> | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch.cpp:181:43 | gecode/int.hh:5833 | assign | ia | IntAssign | passes to assign; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:181:62 | gecode/int.hh:5833 | assign | vvp | IntVarValPrint | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:188:79 | gecode/int.hh:5798 | branch | vals | BoolValBranch | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:188:102 | gecode/int.hh:5799 | branch | bf | BoolBranchFilter | passes to post_view_value_brancher<BoolView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:189:28 | gecode/int.hh:5799 | branch | vvp | BoolVarValPrint | passes to post_view_value_brancher<BoolView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:201:89 | gecode/int.hh:5805 | branch | vals | BoolValBranch | passes to branch, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:201:112 | gecode/int.hh:5806 | branch | bf | BoolBranchFilter | passes to branch, post_view_value_brancher<BoolView, 2, int, 2>, post_view_value_brancher<BoolView, 3, int, 2>, post_view_value_brancher<BoolView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:202:28 | gecode/int.hh:5806 | branch | vvp | BoolVarValPrint | passes to branch, post_view_value_brancher<BoolView, 2, int, 2>, post_view_value_brancher<BoolView, 3, int, 2>, post_view_value_brancher<BoolView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:237:48 | gecode/int.hh:5812 | branch | vals | BoolValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:237:70 | gecode/int.hh:5812 | branch | vvp | BoolVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:244:59 | gecode/int.hh:5839 | assign | vars | BoolVarBranch | passes to ViewSelNone<BoolView> | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:244:76 | gecode/int.hh:5839 | assign | vals | BoolAssign | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:244:99 | gecode/int.hh:5840 | assign | bf | BoolBranchFilter | passes to post_view_value_brancher<BoolView, 1, int, 1>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:245:28 | gecode/int.hh:5840 | assign | vvp | BoolVarValPrint | passes to post_view_value_brancher<BoolView, 1, int, 1>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:256:86 | none | assign | vals | BoolAssign | passes to assign, Branch::value_selection_commit | const& (read only; copies come from accessors) | none |
| gecode/int/branch.cpp:256:109 | none | assign | bf | BoolBranchFilter | passes to assign, post_view_value_brancher<BoolView, 2, int, 1>, post_view_value_brancher<BoolView, 3, int, 1>, post_view_value_brancher<BoolView, 4, int, 1> | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch.cpp:257:28 | none | assign | vvp | BoolVarValPrint | passes to assign, post_view_value_brancher<BoolView, 2, int, 1>, post_view_value_brancher<BoolView, 3, int, 1>, post_view_value_brancher<BoolView, 4, int, 1> | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch.cpp:292:45 | gecode/int.hh:5853 | assign | ba | BoolAssign | passes to assign; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:292:65 | gecode/int.hh:5853 | assign | vvp | BoolVarValPrint | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:309:57 | gecode/int.hh:5776 | branch | vars | IntVarBranch | passes to TieBreak<IntVarBranch>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch.cpp:309:76 | gecode/int.hh:5776 | branch | vals | IntValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:309:98 | gecode/int.hh:5777 | branch | bf | IntBranchFilter | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:310:27 | gecode/int.hh:5777 | branch | vvp | IntVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:315:67 | gecode/int.hh:5779 | branch | vars | TieBreak<IntVarBranch> | passes to Int::Branch::post_open_brancher<Int::IntView>; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:315:86 | gecode/int.hh:5779 | branch | vals | IntValBranch | passes to Int::Branch::post_open_brancher<Int::IntView>; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:315:108 | gecode/int.hh:5780 | branch | bf | IntBranchFilter | passes to Int::Branch::post_open_brancher<Int::IntView>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:316:27 | gecode/int.hh:5780 | branch | vvp | IntVarValPrint | passes to Int::Branch::post_open_brancher<Int::IntView>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:322:59 | gecode/int.hh:5788 | branch | vars | BoolVarBranch | passes to TieBreak<BoolVarBranch>; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/int/branch.cpp:322:79 | gecode/int.hh:5788 | branch | vals | BoolValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:322:102 | gecode/int.hh:5789 | branch | bf | BoolBranchFilter | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:323:28 | gecode/int.hh:5789 | branch | vvp | BoolVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:328:69 | gecode/int.hh:5791 | branch | vars | TieBreak<BoolVarBranch> | passes to Int::Branch::post_open_brancher<Int::BoolView>; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:328:89 | gecode/int.hh:5791 | branch | vals | BoolValBranch | passes to Int::Branch::post_open_brancher<Int::BoolView>; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.cpp:328:112 | gecode/int.hh:5792 | branch | bf | BoolBranchFilter | passes to Int::Branch::post_open_brancher<Int::BoolView>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.cpp:329:28 | gecode/int.hh:5792 | branch | vvp | BoolVarValPrint | passes to Int::Branch::post_open_brancher<Int::BoolView>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.hpp:36:57 | gecode/int.hh:5864 | branch | vals | IntValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.hpp:36:79 | gecode/int.hh:5864 | branch | bf | IntBranchFilter | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.hpp:36:98 | gecode/int.hh:5865 | branch | vvp | IntVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.hpp:40:59 | gecode/int.hh:5871 | branch | vals | BoolValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.hpp:40:82 | gecode/int.hh:5871 | branch | bf | BoolBranchFilter | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.hpp:40:102 | gecode/int.hh:5872 | branch | vvp | BoolVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.hpp:45:54 | gecode/int.hh:5879 | assign | vals | IntAssign | passes to assign; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.hpp:45:76 | gecode/int.hh:5879 | assign | bf | IntBranchFilter | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.hpp:45:95 | gecode/int.hh:5880 | assign | vvp | IntVarValPrint | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.hpp:49:56 | gecode/int.hh:5886 | assign | vals | BoolAssign | passes to assign; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/branch.hpp:49:79 | gecode/int.hh:5886 | assign | bf | BoolBranchFilter | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch.hpp:49:99 | gecode/int.hh:5887 | assign | vvp | BoolVarValPrint | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch/open.hpp:57:82 | none | OpenListBrancher::OpenListBrancher<View, Filter, n> | bf | BranchFilter<Var> | keeps: initializes filter; copied once (last use) | by value + std::move into SharedData storage (needs G2) | none |
| gecode/int/branch/open.hpp:57:108 | none | OpenListBrancher::OpenListBrancher<View, Filter, n> | vvp | VarValPrint<Var, int> | keeps: initializes printer | by value + std::move into SharedData storage (needs G2) (move at the last use) | none |
| gecode/int/branch/open.hpp:244:32 | none | OpenListBrancher::post | bf | BranchFilter<Var> | passes to OpenListBrancher; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch/open.hpp:244:58 | none | OpenListBrancher::post | vvp | VarValPrint<Var, int> | passes to OpenListBrancher; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch/open.hpp:255:38 | none | check_open_selector | vars | IntVarBranch | reads only | const& (read only; copies come from accessors) | none |
| gecode/int/branch/open.hpp:274:39 | none | check_open_selector | vars | BoolVarBranch | reads only | const& (read only; copies come from accessors) | none |
| gecode/int/branch/open.hpp:290:47 | none | open_valselcommit | vals | IntValBranch | passes to value_selection_commit | const& (read only; copies come from accessors) | none |
| gecode/int/branch/open.hpp:299:48 | none | open_valselcommit | vals | BoolValBranch | passes to value_selection_commit | const& (read only; copies come from accessors) | none |
| gecode/int/branch/open.hpp:308:78 | none | post_open_brancher | bf | BranchFilter<typename Gecode::Int::BoolView::VarType> / BranchFilter<typename Gecode::I... | passes to OpenListBrancher<View, BrancherFilter<View>, n>::post, OpenListBrancher<View, BrancherNoFilter<View>, n>::post | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch/open.hpp:309:65 | none | post_open_brancher | vvp | VarValPrint<typename Gecode::Int::BoolView::VarType, int> / VarValPrint<typename Gecode... | passes to OpenListBrancher<View, BrancherFilter<View>, n>::post, OpenListBrancher<View, BrancherNoFilter<View>, n>::post | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch/open.hpp:318:80 | none | post_open_brancher | vars | TieBreak<Gecode::BoolVarBranch> / TieBreak<Gecode::IntVarBranch> | reads only | const& (read only; copies come from accessors) | none |
| gecode/int/branch/open.hpp:318:96 | none | post_open_brancher | vals | BoolValBranch / IntValBranch | passes to open_valselcommit; copied once (last use) | const& (read only; copies come from accessors) | none |
| gecode/int/branch/open.hpp:319:61 | none | post_open_brancher | bf | BranchFilter<typename Gecode::Int::BoolView::VarType> / BranchFilter<typename Gecode::I... | passes to post_open_brancher<View, 1>, post_open_brancher<View, 2>, post_open_brancher<View, 3>, post_open_brancher<View, 4> | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch/open.hpp:319:106 | none | post_open_brancher | vvp | VarValPrint<typename Gecode::Int::BoolView::VarType, int> / VarValPrint<typename Gecode... | passes to post_open_brancher<View, 1>, post_open_brancher<View, 2>, post_open_brancher<View, 3>, post_open_brancher<View, 4> | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/int/branch/view-values.hpp:89:108 | gecode/int/branch.hh:638 | ViewValuesBrancher::ViewValuesBrancher<n, min, Filter, Print> | bf | IntBranchFilter | passes to base or delegated constructor ViewBrancher<IntView, Filter, n>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch/view-values.hpp:90:82 | gecode/int/branch.hh:639 | ViewValuesBrancher::ViewValuesBrancher<n, min, Filter, Print> | vvp | IntVarValPrint | keeps: initializes p; copied once (last use) | by value + std::move into SharedData storage (needs G2) | declarations to the left |
| gecode/int/branch/view-values.hpp:99:69 | gecode/int/branch.hh:661 | ViewValuesBrancher::post | bf | IntBranchFilter | passes to ViewValuesBrancher<n, min, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch/view-values.hpp:99:88 | gecode/int/branch.hh:662 | ViewValuesBrancher::post | vvp | IntVarValPrint | passes to ViewValuesBrancher<n, min, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch/view-values.hpp:175:106 | gecode/int/branch.hh:669 | post_view_values_brancher | bf | IntBranchFilter | passes to ViewValuesBrancher<n, min, BrancherFilter<IntView>, BrancherPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherFilter<IntView>, BrancherNoPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherNoFilter<IntView>, BrancherPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherNoFilter<IntView>, BrancherNoPrint<IntView, int>>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/branch/view-values.hpp:176:46 | gecode/int/branch.hh:670 | post_view_values_brancher | vvp | IntVarValPrint | passes to ViewValuesBrancher<n, min, BrancherFilter<IntView>, BrancherPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherFilter<IntView>, BrancherNoPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherNoFilter<IntView>, BrancherPrint<IntView, int>>::post, ViewValuesBrancher<n, min, BrancherNoFilter<IntView>, BrancherNoPrint<IntView, int>>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb.cpp:289:76 | gecode/int.hh:6014 | branch | vals | IntValBranch | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/ldsb.cpp:290:28 | gecode/int.hh:6015 | branch | bf | IntBranchFilter | passes to post_ldsb_brancher<IntView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb.cpp:290:47 | gecode/int.hh:6015 | branch | vvp | IntVarValPrint | passes to post_ldsb_brancher<IntView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb.cpp:330:86 | gecode/int.hh:6027 | branch | vals | IntValBranch | passes to branch, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/ldsb.cpp:331:28 | gecode/int.hh:6028 | branch | bf | IntBranchFilter | passes to branch, post_ldsb_brancher<IntView, 2, int, 2>, post_ldsb_brancher<IntView, 3, int, 2>, post_ldsb_brancher<IntView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb.cpp:331:47 | gecode/int.hh:6028 | branch | vvp | IntVarValPrint | passes to branch, post_ldsb_brancher<IntView, 2, int, 2>, post_ldsb_brancher<IntView, 3, int, 2>, post_ldsb_brancher<IntView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb.cpp:430:79 | gecode/int.hh:6040 | branch | vals | BoolValBranch | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/ldsb.cpp:431:29 | gecode/int.hh:6041 | branch | bf | BoolBranchFilter | passes to post_ldsb_brancher<BoolView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb.cpp:431:49 | gecode/int.hh:6041 | branch | vvp | BoolVarValPrint | passes to post_ldsb_brancher<BoolView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb.cpp:468:89 | gecode/int.hh:6053 | branch | vals | BoolValBranch | passes to branch, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/int/ldsb.cpp:469:29 | gecode/int.hh:6054 | branch | bf | BoolBranchFilter | passes to branch, post_ldsb_brancher<BoolView, 2, int, 2>, post_ldsb_brancher<BoolView, 3, int, 2>, post_ldsb_brancher<BoolView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb.cpp:469:49 | gecode/int.hh:6054 | branch | vvp | BoolVarValPrint | passes to branch, post_ldsb_brancher<BoolView, 2, int, 2>, post_ldsb_brancher<BoolView, 3, int, 2>, post_ldsb_brancher<BoolView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb/brancher.hpp:93:93 | gecode/int/ldsb.hh:355 | LDSBBrancher::LDSBBrancher<View, n, Val, a, Filter, Print> | bf | BranchFilter<Var> | passes to base or delegated constructor ViewValBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb/brancher.hpp:94:86 | gecode/int/ldsb.hh:355 | LDSBBrancher::LDSBBrancher<View, n, Val, a, Filter, Print> | vvp | VarValPrint<Var, int> | passes to base or delegated constructor ViewValBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb/brancher.hpp:104:93 | gecode/int/ldsb.hh:370 | LDSBBrancher::post | bf | BranchFilter<Var> | passes to LDSBBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb/brancher.hpp:105:86 | gecode/int/ldsb.hh:370 | LDSBBrancher::post | vvp | VarValPrint<Var, int> | passes to LDSBBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb/brancher.hpp:251:98 | gecode/int/ldsb.hh:376 | post_ldsb_brancher | bf | BranchFilter<typename Gecode::Int::BoolView::VarType> / BranchFilter<typename Gecode::I... | passes to LDSBBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/ldsb/brancher.hpp:252:65 | gecode/int/ldsb.hh:377 | post_ldsb_brancher | vvp | VarValPrint<typename Gecode::Int::BoolView::VarType, int> / VarValPrint<typename Gecode... | passes to LDSBBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, LDSBBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/branch/filter.hpp:90:60 | gecode/kernel/branch/filter.hpp:53 | BrancherFilter::BrancherFilter<View> | bf | BranchFilter<Var> | keeps: initializes f | by value + std::move into SharedData storage (needs G2) (move at the last use); constructor reads bf after f(bf) (`if (!bf)`); after moving, test `!f()` or check before the move | declarations to the left |
| gecode/kernel/branch/filter.hpp:129:64 | gecode/kernel/branch/filter.hpp:75 | BrancherNoFilter::BrancherNoFilter<View> | bf | BranchFilter<Var> | reads only | const& (read only) | declarations to the left |
| gecode/kernel/branch/print.hpp:93:67 | gecode/kernel/branch/print.hpp:54 | BrancherPrint::BrancherPrint<View, Val> | vvp | VarValPrint<Var, int> / VarValPrint<Var, Gecode::FloatNumBranch> | keeps: initializes p | by value + std::move into SharedData storage (needs G2) (move at the last use); constructor reads vvp after p(vvp) (`if (!vvp)`); after moving, test `!p()` or check before the move | declarations to the left |
| gecode/kernel/branch/print.hpp:133:71 | gecode/kernel/branch/print.hpp:77 | BrancherNoPrint::BrancherNoPrint<View, Val> | vvp | VarValPrint<Var, int> / VarValPrint<Var, Gecode::FloatNumBranch> | reads only | const& (read only) | declarations to the left |
| gecode/kernel/branch/view-val.hpp:212:88 | gecode/kernel/branch/view-val.hpp:105 | ViewValBrancher::ViewValBrancher<View, n, Val, a, Filter, Print> | bf | BranchFilter<Var> | passes to base or delegated constructor ViewBrancher<View, Filter, n>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/branch/view-val.hpp:212:114 | gecode/kernel/branch/view-val.hpp:105 | ViewValBrancher::ViewValBrancher<View, n, Val, a, Filter, Print> | vvp | VarValPrint<Var, int> / VarValPrint<Var, Gecode::FloatNumBranch> | keeps: initializes p; copied once (last use) | by value + std::move into SharedData storage (needs G2) | declarations to the left |
| gecode/kernel/branch/view-val.hpp:221:111 | gecode/kernel/branch/view-val.hpp:130 | ViewValBrancher::post | bf | BranchFilter<Var> | passes to ViewValBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/branch/view-val.hpp:222:81 | gecode/kernel/branch/view-val.hpp:130 | ViewValBrancher::post | vvp | VarValPrint<Var, int> / VarValPrint<Var, Gecode::FloatNumBranch> | passes to ViewValBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/branch/view-val.hpp:299:67 | gecode/kernel/branch/view-val.hpp:136 | post_view_value_brancher | bf | BranchFilter<typename Gecode::Int::BoolView::VarType> / BranchFilter<typename Gecode::I... | passes to ViewValBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/branch/view-val.hpp:299:112 | gecode/kernel/branch/view-val.hpp:137 | post_view_value_brancher | vvp | VarValPrint<typename Gecode::Int::BoolView::VarType, int> / VarValPrint<typename Gecode... | passes to ViewValBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, ViewValBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/branch/view.hpp:140:67 | gecode/kernel/branch/view.hpp:98 | ViewBrancher::ViewBrancher<View, Filter, n> | bf | BranchFilter<Var> | keeps: initializes f; copied once (last use) | by value + std::move into SharedData storage (needs G2) | declarations to the left |

### G8 set and float brancher posting (58 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/float/branch.cpp:38:82 | gecode/float.hh:1867 | branch | vals | FloatValBranch | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.cpp:38:106 | gecode/float.hh:1868 | branch | bf | FloatBranchFilter | passes to post_view_value_brancher<FloatView, 1, FloatNumBranch, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:39:29 | gecode/float.hh:1868 | branch | vvp | FloatVarValPrint | passes to post_view_value_brancher<FloatView, 1, FloatNumBranch, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:51:92 | gecode/float.hh:1875 | branch | vals | FloatValBranch | passes to branch, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.cpp:51:116 | gecode/float.hh:1875 | branch | bf | FloatBranchFilter | passes to branch, post_view_value_brancher<FloatView, 2, FloatNumBranch, 2>, post_view_value_brancher<FloatView, 3, FloatNumBranch, 2>, post_view_value_brancher<FloatView, 4, FloatNumBranch, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:52:29 | gecode/float.hh:1876 | branch | vvp | FloatVarValPrint | passes to branch, post_view_value_brancher<FloatView, 2, FloatNumBranch, 2>, post_view_value_brancher<FloatView, 3, FloatNumBranch, 2>, post_view_value_brancher<FloatView, 4, FloatNumBranch, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:89:50 | gecode/float.hh:1882 | branch | vals | FloatValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.cpp:89:73 | gecode/float.hh:1882 | branch | vvp | FloatVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:96:61 | gecode/float.hh:1889 | assign | vars | FloatVarBranch | passes to ViewSelNone<FloatView> | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.cpp:96:79 | gecode/float.hh:1889 | assign | vals | FloatAssign | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.cpp:96:103 | gecode/float.hh:1890 | assign | bf | FloatBranchFilter | passes to post_view_value_brancher<FloatView, 1, FloatNumBranch, 1>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:97:29 | gecode/float.hh:1890 | assign | vvp | FloatVarValPrint | passes to post_view_value_brancher<FloatView, 1, FloatNumBranch, 1>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:108:89 | gecode/float.hh:1896 | assign | vals | FloatAssign | passes to assign, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.cpp:108:113 | gecode/float.hh:1897 | assign | bf | FloatBranchFilter | passes to assign, post_view_value_brancher<FloatView, 2, FloatNumBranch, 1>, post_view_value_brancher<FloatView, 3, FloatNumBranch, 1>, post_view_value_brancher<FloatView, 4, FloatNumBranch, 1> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:109:29 | gecode/float.hh:1897 | assign | vvp | FloatVarValPrint | passes to assign, post_view_value_brancher<FloatView, 2, FloatNumBranch, 1>, post_view_value_brancher<FloatView, 3, FloatNumBranch, 1>, post_view_value_brancher<FloatView, 4, FloatNumBranch, 1> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.cpp:146:47 | gecode/float.hh:1903 | assign | vals | FloatAssign | passes to assign; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.cpp:146:70 | gecode/float.hh:1903 | assign | vvp | FloatVarValPrint | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.hpp:36:61 | gecode/float.hh:1914 | branch | vals | FloatValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.hpp:36:85 | gecode/float.hh:1914 | branch | bf | FloatBranchFilter | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.hpp:36:106 | gecode/float.hh:1915 | branch | vvp | FloatVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.hpp:41:58 | gecode/float.hh:1921 | assign | vals | FloatAssign | passes to assign; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/float/branch.hpp:41:82 | gecode/float.hh:1921 | assign | bf | FloatBranchFilter | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/branch.hpp:41:103 | gecode/float.hh:1922 | assign | vvp | FloatVarValPrint | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:38:76 | gecode/set.hh:1492 | branch | vals | SetValBranch | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.cpp:38:98 | gecode/set.hh:1493 | branch | bf | SetBranchFilter | passes to post_view_value_brancher<SetView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:39:27 | gecode/set.hh:1493 | branch | vvp | SetVarValPrint | passes to post_view_value_brancher<SetView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:50:86 | gecode/set.hh:1499 | branch | vals | SetValBranch | passes to branch, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.cpp:50:108 | gecode/set.hh:1500 | branch | bf | SetBranchFilter | passes to branch, post_view_value_brancher<SetView, 2, int, 2>, post_view_value_brancher<SetView, 3, int, 2>, post_view_value_brancher<SetView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:51:27 | gecode/set.hh:1500 | branch | vvp | SetVarValPrint | passes to branch, post_view_value_brancher<SetView, 2, int, 2>, post_view_value_brancher<SetView, 3, int, 2>, post_view_value_brancher<SetView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:86:46 | gecode/set.hh:1506 | branch | vals | SetValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.cpp:86:67 | gecode/set.hh:1506 | branch | vvp | SetVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:93:57 | gecode/set.hh:1513 | assign | vars | SetVarBranch | passes to ViewSelNone<SetView> | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.cpp:93:73 | gecode/set.hh:1513 | assign | vals | SetAssign | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.cpp:93:95 | gecode/set.hh:1514 | assign | bf | SetBranchFilter | passes to post_view_value_brancher<SetView, 1, int, 1>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:93:114 | gecode/set.hh:1514 | assign | vvp | SetVarValPrint | passes to post_view_value_brancher<SetView, 1, int, 1>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:103:83 | gecode/set.hh:1520 | assign | vals | SetAssign | passes to assign, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.cpp:103:105 | gecode/set.hh:1521 | assign | bf | SetBranchFilter | passes to assign, post_view_value_brancher<SetView, 2, int, 1>, post_view_value_brancher<SetView, 3, int, 1>, post_view_value_brancher<SetView, 4, int, 1> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:104:27 | gecode/set.hh:1521 | assign | vvp | SetVarValPrint | passes to assign, post_view_value_brancher<SetView, 2, int, 1>, post_view_value_brancher<SetView, 3, int, 1>, post_view_value_brancher<SetView, 4, int, 1> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.cpp:139:43 | gecode/set.hh:1527 | assign | vars | SetAssign | passes to assign; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.cpp:139:64 | gecode/set.hh:1527 | assign | vvp | SetVarValPrint | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.hpp:36:57 | gecode/set.hh:1538 | branch | vals | SetValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.hpp:36:79 | gecode/set.hh:1538 | branch | bf | SetBranchFilter | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.hpp:36:98 | gecode/set.hh:1539 | branch | vvp | SetVarValPrint | passes to branch; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.hpp:41:54 | gecode/set.hh:1546 | assign | vals | SetAssign | passes to assign; copied once (last use) | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/branch.hpp:41:76 | gecode/set.hh:1546 | assign | bf | SetBranchFilter | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/branch.hpp:41:95 | gecode/set.hh:1547 | assign | vvp | SetVarValPrint | passes to assign; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb.cpp:152:76 | gecode/set.hh:1570 | branch | vals | SetValBranch | passes to Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/ldsb.cpp:153:28 | gecode/set.hh:1571 | branch | bf | SetBranchFilter | passes to post_ldsb_set_brancher<SetView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb.cpp:153:47 | gecode/set.hh:1571 | branch | vvp | SetVarValPrint | passes to post_ldsb_set_brancher<SetView, 1, int, 2>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb.cpp:177:86 | gecode/set.hh:1578 | branch | vals | SetValBranch | passes to branch, Branch::value_selection_commit | const& (read only; copies come from accessors) | declarations to the left |
| gecode/set/ldsb.cpp:178:28 | gecode/set.hh:1579 | branch | bf | SetBranchFilter | passes to branch, post_ldsb_set_brancher<SetView, 2, int, 2>, post_ldsb_set_brancher<SetView, 3, int, 2>, post_ldsb_set_brancher<SetView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb.cpp:178:47 | gecode/set.hh:1579 | branch | vvp | SetVarValPrint | passes to branch, post_ldsb_set_brancher<SetView, 2, int, 2>, post_ldsb_set_brancher<SetView, 3, int, 2>, post_ldsb_set_brancher<SetView, 4, int, 2> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb/brancher.hpp:38:96 | gecode/set/ldsb.hh:86 | LDSBSetBrancher::LDSBSetBrancher<View, n, Val, a, Filter, Print> | bf | BranchFilter<Var> | passes to base or delegated constructor LDSBBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb/brancher.hpp:39:89 | gecode/set/ldsb.hh:86 | LDSBSetBrancher::LDSBSetBrancher<View, n, Val, a, Filter, Print> | vvp | VarValPrint<Var, int> | passes to base or delegated constructor LDSBBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb/brancher.hpp:232:96 | gecode/set/ldsb.hh:97 | LDSBSetBrancher::post | bf | BranchFilter<Var> | passes to LDSBSetBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb/brancher.hpp:233:89 | gecode/set/ldsb.hh:97 | LDSBSetBrancher::post | vvp | VarValPrint<Var, int> | passes to LDSBSetBrancher<View, n, Val, a, Filter, Print>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/ldsb/brancher.hpp:240:102 | none | post_ldsb_set_brancher | bf | BranchFilter<typename Gecode::Set::SetView::VarType> | passes to LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post | by value + std::move on each path into SharedData (needs G2) | none |
| gecode/set/ldsb/brancher.hpp:241:69 | none | post_ldsb_set_brancher | vvp | VarValPrint<typename Gecode::Set::SetView::VarType, int> | passes to LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post, LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post | by value + std::move on each path into SharedData (needs G2) | none |

### G9 posting with shared data (48 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/float/relax.cpp:56:73 | gecode/float.hh:1951 | relax | r | Rnd | passes to Search::relax<FloatVarArgs, Float::Relax> | const& (no move constructor) | declarations to the left |
| gecode/int/count/int-eq.hpp:37:67 | gecode/int/count.hh:180 | EqInt::EqInt<VX, VY> | y | IntSet | passes to base or delegated constructor IntBase<VX, VY>; copied once (last use) | const& (no move constructor) | declarations to the left |
| gecode/int/count/int-eq.hpp:42:57 | gecode/int/count.hh:188 | EqInt::post | y | IntSet | passes to holds, post_false, post_true, EqInt<VX, VY> | const& (no move constructor) | declarations to the left |
| gecode/int/count/int-gq.hpp:37:67 | gecode/int/count.hh:211 | GqInt::GqInt<VX, VY> | y | IntSet | passes to base or delegated constructor IntBase<VX, VY>; copied once (last use) | const& (no move constructor) | declarations to the left |
| gecode/int/count/int-gq.hpp:42:57 | gecode/int/count.hh:219 | GqInt::post | y | IntSet | passes to holds, post_true, GqInt<VX, VY> | const& (no move constructor) | declarations to the left |
| gecode/int/count/int-lq.hpp:37:67 | gecode/int/count.hh:242 | LqInt::LqInt<VX, VY> | y | IntSet | passes to base or delegated constructor IntBase<VX, VY>; copied once (last use) | const& (no move constructor) | declarations to the left |
| gecode/int/count/int-lq.hpp:42:57 | gecode/int/count.hh:250 | LqInt::post | y | IntSet | passes to holds, post_false, LqInt<VX, VY> | const& (no move constructor) | declarations to the left |
| gecode/int/count/rel.hpp:50:23 | none | is_int_set | y | IntSet | reads only | const& (no move constructor); template parameter VY is also a view; `const VY&` is fine for views | explicit specialization of is_int_set (primary gecode/int/count/rel.hpp:44, declared gecode/int/count.hh:54): change primary and specialization together |
| gecode/int/count/rel.hpp:62:18 | none | isval | y | IntSet | reads only | const& (no move constructor); template parameter VY is also a view; `const VY&` is fine for views | explicit specialization of isval (primary gecode/int/count/rel.hpp:57, declared gecode/int/count.hh:57): change primary and specialization together |
| gecode/int/count/view-eq.hpp:37:74 | gecode/int/count.hh:328 | EqView::EqView<VX, VY, VZ, shr, dom> | y | IntSet | passes to base or delegated constructor ViewBase<VX, VY, VZ>; copied once (last use) | const& (no move constructor) | declarations to the left |
| gecode/int/count/view-eq.hpp:42:72 | gecode/int/count.hh:334 | EqView::post | y | IntSet | passes to isval, EqInt<VX, VY>::post, sharing, EqView<VX, VY, VZ, true, dom>, EqView<VX, VY, VZ, false, dom> | const& (no move constructor) | declarations to the left |
| gecode/int/count/view-gq.hpp:37:74 | gecode/int/count.hh:400 | GqView::GqView<VX, VY, VZ, shr, dom> | y | IntSet | passes to base or delegated constructor ViewBase<VX, VY, VZ>; copied once (last use) | const& (no move constructor) | declarations to the left |
| gecode/int/count/view-gq.hpp:42:72 | gecode/int/count.hh:406 | GqView::post | y | IntSet | passes to isval, GqInt<VX, VY>::post, sharing, GqView<VX, VY, VZ, true, dom>, GqView<VX, VY, VZ, false, dom> | const& (no move constructor) | declarations to the left |
| gecode/int/count/view-lq.hpp:37:69 | gecode/int/count.hh:364 | LqView::LqView<VX, VY, VZ, shr> | y | IntSet | passes to base or delegated constructor ViewBase<VX, VY, VZ>; copied once (last use) | const& (no move constructor) | declarations to the left |
| gecode/int/count/view-lq.hpp:42:67 | gecode/int/count.hh:370 | LqView::post | y | IntSet | passes to LqInt<VX, VY>::post, sharing, LqView<VX, VY, VZ, true>, LqView<VX, VY, VZ, false> | const& (no move constructor) | declarations to the left |
| gecode/int/element.cpp:282:39 | gecode/int.hh:1800 | element | a | IntSharedArray | passes to element | const& (no move constructor) | declarations to the left |
| gecode/int/element.cpp:291:39 | gecode/int.hh:1815 | element | a | IntSharedArray | passes to element | const& (no move constructor) | declarations to the left |
| gecode/int/element.cpp:301:39 | gecode/int.hh:1829 | element | a | IntSharedArray | passes to element | const& (no move constructor) | declarations to the left |
| gecode/int/element.cpp:310:39 | gecode/int.hh:1844 | element | a | IntSharedArray | passes to element | const& (no move constructor) | declarations to the left |
| gecode/int/extensional-regular.cpp:38:53 | gecode/int.hh:2957 | extensional | dfa | DFA | passes to Extensional::post_lgp | const& (no move constructor) | declarations to the left |
| gecode/int/extensional-regular.cpp:47:54 | gecode/int.hh:2971 | extensional | dfa | DFA | passes to Extensional::post_lgp | const& (no move constructor) | declarations to the left |
| gecode/int/extensional-regular.cpp:56:53 | gecode/int.hh:2984 | extensional | dfa | DFA | passes to Int::Extensional::OpenRegular<Int::IntView, Int::PC_INT_DOM>::post | const& (no move constructor); DFA path: extensional -> OpenRegular::post -> OpenRegular constructor | declarations to the left |
| gecode/int/extensional-regular.cpp:63:54 | gecode/int.hh:2995 | extensional | dfa | DFA | passes to Int::Extensional::OpenRegular<Int::BoolView, Int::PC_BOOL_VAL>::post | const& (no move constructor); DFA path: extensional -> OpenRegular::post -> OpenRegular constructor | declarations to the left |
| gecode/int/extensional/open.hpp:108:62 | none | OpenRegular::OpenRegular<View, pc> | dfa0 | DFA | keeps: initializes dfa; passes to accepting_distances | const& (no move constructor; the callee's copy stays and is the only copy); never move: the constructor copies dfa0 into dfa and then reads dfa0 in accepting_distances(dfa0); DFA has no move constructor | none |
| gecode/int/extensional/open.hpp:296:54 | none | OpenRegular::post | dfa | DFA | copies into local symbols; passes to OpenRegular | const& (no move constructor); reads dfa for DFA::Symbols, then forwards to the constructor | none |
| gecode/int/ldsb/sym-obj.cpp:97:70 | gecode/int/ldsb.hh:155 | ValueSequenceSymmetryObject::ValueSequenceSymmetryObject | vs | IntArgs | keeps: initializes values; copied once (last use) | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/int/relax.cpp:64:69 | gecode/int.hh:6129 | relax | r | Rnd | passes to Search::relax<IntVarArgs, Int::Relax> | const& (no move constructor) | declarations to the left |
| gecode/int/relax.cpp:76:71 | gecode/int.hh:6152 | relax | r | Rnd | passes to Search::relax<BoolVarArgs, Int::Relax> | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/int.hpp:105:56 | gecode/int/sequence.hh:122 | Sequence::check | s | IntSet | passes to includes, excludes | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/int.hpp:129:66 | gecode/int/sequence.hh:120 | Sequence::post | s | IntSet | passes to check, Sequence<View, Val>, p->vvsamax.propagate, p->vvsamin.propagate | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:165:86 | gecode/int/sequence/view.hpp:110 | ViewValSupport::alternative_not_possible | s | IntSet | passes to includes | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:172:76 | gecode/int/sequence/view.hpp:108 | ViewValSupport::s_not_possible | s | IntSet | passes to excludes | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:179:79 | gecode/int/sequence/view.hpp:78 | ViewValSupport::init | s | IntSet | passes to alternative_not_possible | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:214:63 | gecode/int/sequence/view.hpp:102 | ViewValSupport::shaved | s | IntSet | passes to excludes, includes | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:223:81 | gecode/int/sequence/view.hpp:94 | ViewValSupport::schedule_conclusion | s | IntSet | passes to includes, excludes | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:254:83 | gecode/int/sequence/view.hpp:106 | ViewValSupport::conclude | s | IntSet | passes to exclude, include | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:289:68 | gecode/int/sequence/view.hpp:104 | ViewValSupport::pushup | s | IntSet | passes to s_not_possible, alternative_not_possible | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:334:76 | gecode/int/sequence/view.hpp:82 | ViewValSupport::advise | s | IntSet | passes to shaved, takes, pushup, schedule_conclusion | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:371:84 | gecode/int/sequence/view.hpp:84 | ViewValSupport::propagate | s | IntSet | passes to conclude, pushup | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:406:99 | gecode/int/sequence.hh:77 | ViewValSupportArray::ViewValSupportArray<View, Val, iss> | s | IntSet | passes to init | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:459:89 | gecode/int/sequence.hh:89 | ViewValSupportArray::propagate | s | IntSet | passes to propagate | const& (no move constructor) | declarations to the left |
| gecode/int/sequence/view.hpp:468:86 | gecode/int/sequence.hh:91 | ViewValSupportArray::advise | s | IntSet | passes to advise | const& (no move constructor) | declarations to the left |
| gecode/minimodel/aliases.hpp:122:51 | gecode/minimodel.hh:1744 | values | y | IntSet | passes to dom | const& (no move constructor) | declarations to the left |
| gecode/minimodel/matrix.hpp:128:25 | gecode/minimodel.hh:1883 | Matrix::Matrix<type-parameter-0-0> | a | BoolVarArgs / IntVarArgs / SetVarArgs / IntArgs / ArgArray<Gecode::IntSet> | keeps: initializes _a; copied once (last use) | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/set/int.cpp:260:39 | gecode/set.hh:838 | weights | elements | IntSharedArray | passes to Set::Int::Weights<Set::SetView>::post | const& (no move constructor) | declarations to the left |
| gecode/set/int.cpp:260:64 | gecode/set.hh:838 | weights | weights | IntSharedArray | passes to Set::Int::Weights<Set::SetView>::post | const& (no move constructor) | declarations to the left |
| gecode/set/ldsb/brancher.hpp:84:67 | none | special_update | used_values | IntSet | passes to v | const& (no move constructor) | none |
| gecode/set/relax.cpp:61:69 | gecode/set.hh:1605 | relax | r | Rnd | passes to Search::relax<SetVarArgs, Set::Relax> | const& (no move constructor) | declarations to the left |

### G10 execution callbacks (22 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/float/exec.cpp:38:66 | gecode/float.hh:1359 | wait | c | std::function<void (Space &)> | passes to UnaryWait<Float::FloatView>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/float/exec.cpp:44:77 | gecode/float.hh:1361 | wait | c | std::function<void (Space &)> | passes to NaryWait<Float::FloatView>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec.cpp:38:64 | gecode/int.hh:4765 | wait | c | std::function<void (Space &)> | passes to UnaryWait<Int::IntView>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec.cpp:44:65 | gecode/int.hh:4767 | wait | c | std::function<void (Space &)> | passes to UnaryWait<Int::BoolView>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec.cpp:50:75 | gecode/int.hh:4769 | wait | c | std::function<void (Space &)> | passes to NaryWait<Int::IntView>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec.cpp:57:76 | gecode/int.hh:4772 | wait | c | std::function<void (Space &)> | passes to NaryWait<Int::BoolView>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec.cpp:64:65 | gecode/int.hh:4775 | when | t | std::function<void (Space &)> | passes to Int::Exec::When::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec.cpp:64:101 | gecode/int.hh:4776 | when | e | std::function<void (Space &)> | passes to Int::Exec::When::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec.cpp:70:65 | gecode/int.hh:4778 | when | t | std::function<void (Space &)> | passes to when; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec/when.hpp:36:72 | gecode/int/exec.hh:61 | When::When | t0 | std::function<void (Space &)> | keeps: initializes t | by value + std::move into SharedData storage (needs G2) (move at the last use) | declarations to the left |
| gecode/int/exec/when.hpp:36:109 | gecode/int/exec.hh:61 | When::When | e0 | std::function<void (Space &)> | keeps: initializes e | by value + std::move into SharedData storage (needs G2) (move at the last use) | declarations to the left |
| gecode/int/exec/when.hpp:46:72 | gecode/int/exec.hh:71 | When::post | t | std::function<void (Space &)> | passes to When | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/exec/when.hpp:46:108 | gecode/int/exec.hh:72 | When::post | e | std::function<void (Space &)> | passes to When | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/branch/function.cpp:87:80 | gecode/kernel/branch/function.cpp:55 | FunctionBranch::FunctionBranch | f0 | std::function<void (Space &)> | keeps: initializes f | by value + std::move into SharedData storage (needs G2) (move at the last use) | declarations to the left |
| gecode/kernel/branch/function.cpp:125:70 | gecode/kernel/branch/function.cpp:73 | FunctionBranch::post | f | std::function<void (Space &)> | passes to FunctionBranch | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/branch/function.cpp:143:56 | gecode/kernel.hh:205 | branch | f | std::function<void (Space &)> | passes to Kernel::FunctionBranch::post; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/propagator/wait.hpp:107:85 | gecode/kernel/propagator/wait.hpp:49 | UnaryWait::UnaryWait<View> | c0 | std::function<void (Space &)> | keeps: initializes c | by value + std::move into SharedData storage (needs G2) (move at the last use) | declarations to the left |
| gecode/kernel/propagator/wait.hpp:143:79 | gecode/kernel/propagator/wait.hpp:63 | UnaryWait::post | c | std::function<void (Space &)> | passes to UnaryWait<View> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/kernel/propagator/wait.hpp:170:95 | gecode/kernel/propagator/wait.hpp:82 | NaryWait::NaryWait<View> | c0 | std::function<void (Space &)> | keeps: initializes c | by value + std::move into SharedData storage (needs G2) (move at the last use) | declarations to the left |
| gecode/kernel/propagator/wait.hpp:225:90 | gecode/kernel/propagator/wait.hpp:96 | NaryWait::post | c | std::function<void (Space &)> | passes to UnaryWait<View>::post, NaryWait<View> | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/exec.cpp:38:64 | gecode/set.hh:966 | wait | c | std::function<void (Space &)> | passes to UnaryWait<Set::SetView>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/set/exec.cpp:44:75 | gecode/set.hh:968 | wait | c | std::function<void (Space &)> | passes to NaryWait<Set::SetView>::post | by value + std::move on each path into SharedData (needs G2) | declarations to the left |

### G11 open variable lists (6 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/int/open-var-list.cpp:40:36 | gecode/int.hh:1106 | OpenIntVarList::factory | d | Domain | keeps: lambda capture | by value + std::move into an init-capture; `if (!d)` runs first; capture with `[d = std::move(d)]` | declarations to the left |
| gecode/int/open-var-list.cpp:52:77 | gecode/int.hh:1096 | OpenIntVarList::OpenIntVarList | t | Transition | passes to base or delegated constructor OpenIntVarList; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/open-var-list.cpp:55:56 | gecode/int.hh:1099 | OpenIntVarList::OpenIntVarList | d | Domain | passes to factory; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/open-var-list.cpp:57:56 | gecode/int.hh:1102 | OpenIntVarList::OpenIntVarList | d | Domain | passes to factory; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/open-var-list.cpp:57:70 | gecode/int.hh:1102 | OpenIntVarList::OpenIntVarList | t | Transition | passes to base or delegated constructor OpenVarList<IntVar>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |
| gecode/int/open-var-list.cpp:65:62 | gecode/int.hh:1125 | OpenBoolVarList::OpenBoolVarList | t | Transition | passes to base or delegated constructor OpenVarList<BoolVar>; copied once (last use) | by value + std::move on each path into SharedData (needs G2) | declarations to the left |

### G12 tracing (9 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/float/trace.cpp:38:57 | gecode/float.hh:2040 | trace | tf | TraceFilter | passes to FloatTraceRecorder::post | const& (no move constructor) | declarations to the left |
| gecode/int/trace.cpp:38:55 | gecode/int.hh:6308 | trace | tf | TraceFilter | passes to IntTraceRecorder::post | const& (no move constructor) | declarations to the left |
| gecode/int/trace.cpp:46:56 | gecode/int.hh:6322 | trace | tf | TraceFilter | passes to BoolTraceRecorder::post | const& (no move constructor) | declarations to the left |
| gecode/kernel/trace/general.cpp:38:34 | gecode/kernel.hh:251<br>gecode/kernel/core.hpp:1849 | trace | tf | TraceFilter | passes to TraceRecorder::post | const& (no move constructor) | declarations to the left |
| gecode/kernel/trace/recorder.hpp:260:91 | gecode/kernel/trace/recorder.hpp:123 | ViewTraceRecorder::ViewTraceRecorder<View> | tf0 | TraceFilter | keeps: initializes tf | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/trace/recorder.hpp:281:78 | gecode/kernel/trace/recorder.hpp:137 | ViewTraceRecorder::post | tf | TraceFilter | passes to ViewTraceRecorder | const& (no move constructor) | declarations to the left |
| gecode/kernel/trace/recorder.hpp:390:57 | gecode/kernel/trace/recorder.hpp:170 | TraceRecorder::TraceRecorder | tf0 | TraceFilter | keeps: initializes tf | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/kernel/trace/recorder.hpp:397:48 | gecode/kernel/trace/recorder.hpp:182 | TraceRecorder::post | tf | TraceFilter | passes to TraceRecorder | const& (no move constructor) | declarations to the left |
| gecode/set/trace.cpp:38:55 | gecode/set.hh:1722 | trace | tf | TraceFilter | passes to SetTraceRecorder::post | const& (no move constructor) | declarations to the left |

### G13 FlatZinc (56 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/flatzinc/blackbox-propagator.cpp:106:52 | gecode/flatzinc/blackbox.hh:69 | BlackBoxContextHandle::fail | e | std::exception_ptr | passes to fail; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| gecode/flatzinc/blackbox-propagator.cpp:143:46 | gecode/flatzinc/blackbox-propagator.cpp:87 | BlackBoxContext::fail | e | std::exception_ptr | keeps: assigns exception; copied once (last use) | by value + std::move into storage | declarations to the left |
| gecode/flatzinc/blackbox-propagator.cpp:286:46 | none | BlackBoxBounds::BlackBoxBounds | sub_int0 | SharedArray<bool> | keeps: initializes sub_int | const& (no move constructor; the callee's copy stays and is the only copy) | none |
| gecode/flatzinc/blackbox-propagator.cpp:288:46 | none | BlackBoxBounds::BlackBoxBounds | sub_float0 | SharedArray<bool> | keeps: initializes sub_float | const& (no move constructor; the callee's copy stays and is the only copy) | none |
| gecode/flatzinc/branch.cpp:112:102 | gecode/flatzinc/branch.hh:336 | branch | vals | IntValBranch | passes to Int::Branch::value_selection_commit, i2b | const& (read only; copies come from accessors) | declarations to the left |
| gecode/flatzinc/branch.hpp:40:58 | gecode/flatzinc/branch.hh:75 | IntBoolVarBranch::IntBoolVarBranch | i | IntAFC | keeps: initializes int_afc_data | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:40:69 | gecode/flatzinc/branch.hh:75 | IntBoolVarBranch::IntBoolVarBranch | b | BoolAFC | keeps: initializes bool_afc_data | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:44:61 | gecode/flatzinc/branch.hh:77 | IntBoolVarBranch::IntBoolVarBranch | i | IntAction | keeps: initializes int_action_data | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:44:75 | gecode/flatzinc/branch.hh:77 | IntBoolVarBranch::IntBoolVarBranch | b | BoolAction | keeps: initializes bool_action_data | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:48:58 | gecode/flatzinc/branch.hh:79 | IntBoolVarBranch::IntBoolVarBranch | i | IntCHB | keeps: initializes int_chb_data | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:48:69 | gecode/flatzinc/branch.hh:79 | IntBoolVarBranch::IntBoolVarBranch | b | BoolCHB | keeps: initializes bool_chb_data | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:115:33 | gecode/flatzinc/branch.hh:103 | int_bool_var_afc_max | ia | IntAFC | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:115:45 | gecode/flatzinc/branch.hh:103 | int_bool_var_afc_max | ba | BoolAFC | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:123:39 | gecode/flatzinc/branch.hh:107 | int_bool_var_action_max | ia | IntAction | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:123:54 | gecode/flatzinc/branch.hh:107 | int_bool_var_action_max | ba | BoolAction | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:131:33 | gecode/flatzinc/branch.hh:111 | int_bool_var_chb_max | ic | IntCHB | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:131:45 | gecode/flatzinc/branch.hh:111 | int_bool_var_chb_max | bc | BoolCHB | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:139:38 | gecode/flatzinc/branch.hh:115 | int_bool_var_afc_size_max | ia | IntAFC | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:139:50 | gecode/flatzinc/branch.hh:115 | int_bool_var_afc_size_max | ba | BoolAFC | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:147:44 | gecode/flatzinc/branch.hh:119 | int_bool_var_action_size_max | ia | IntAction | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:147:59 | gecode/flatzinc/branch.hh:119 | int_bool_var_action_size_max | ba | BoolAction | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:155:38 | gecode/flatzinc/branch.hh:123 | int_bool_var_chb_size_max | ic | IntCHB | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/branch.hpp:155:50 | gecode/flatzinc/branch.hh:123 | int_bool_var_chb_size_max | bc | BoolCHB | keeps: returns a IntBoolVarBranch built from it | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/flatzinc/flatzinc.cpp:113:58 | none | AuxVarBrancher::AuxVarBrancher | int_varsel0 | TieBreak<IntVarBranch> | keeps: initializes int_varsel; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:113:84 | none | AuxVarBrancher::AuxVarBrancher | int_valsel0 | IntValBranch | keeps: initializes int_valsel; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:114:48 | none | AuxVarBrancher::AuxVarBrancher | bool_varsel0 | TieBreak<BoolVarBranch> | keeps: initializes bool_varsel; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:114:76 | none | AuxVarBrancher::AuxVarBrancher | bool_valsel0 | BoolValBranch | keeps: initializes bool_valsel; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:117:37 | none | AuxVarBrancher::AuxVarBrancher | set_varsel0 | SetVarBranch | keeps: initializes set_varsel; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:117:63 | none | AuxVarBrancher::AuxVarBrancher | set_valsel0 | SetValBranch | keeps: initializes set_valsel; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:121:49 | none | AuxVarBrancher::AuxVarBrancher | float_varsel0 | TieBreak<FloatVarBranch> | keeps: initializes float_varsel; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:121:79 | none | AuxVarBrancher::AuxVarBrancher | float_valsel0 | FloatValBranch | keeps: initializes float_valsel; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:246:48 | none | AuxVarBrancher::post | int_varsel | TieBreak<IntVarBranch> | passes to AuxVarBrancher; copied once (last use) | by value + std::move at the copy site | none |
| gecode/flatzinc/flatzinc.cpp:246:73 | none | AuxVarBrancher::post | int_valsel | IntValBranch | passes to AuxVarBrancher; copied once (last use) | by value + std::move at the copy site | none |
| gecode/flatzinc/flatzinc.cpp:246:109 | none | AuxVarBrancher::post | bool_varsel | TieBreak<BoolVarBranch> | passes to AuxVarBrancher; copied once (last use) | by value + std::move at the copy site | none |
| gecode/flatzinc/flatzinc.cpp:247:28 | none | AuxVarBrancher::post | bool_valsel | BoolValBranch | passes to AuxVarBrancher; copied once (last use) | by value + std::move at the copy site | none |
| gecode/flatzinc/flatzinc.cpp:250:27 | none | AuxVarBrancher::post | set_varsel | SetVarBranch | passes to AuxVarBrancher; copied once (last use) | by value + std::move at the copy site | none |
| gecode/flatzinc/flatzinc.cpp:250:52 | none | AuxVarBrancher::post | set_valsel | SetValBranch | passes to AuxVarBrancher; copied once (last use) | by value + std::move at the copy site | none |
| gecode/flatzinc/flatzinc.cpp:254:39 | none | AuxVarBrancher::post | float_varsel | TieBreak<FloatVarBranch> | passes to AuxVarBrancher; copied once (last use) | by value + std::move at the copy site | none |
| gecode/flatzinc/flatzinc.cpp:254:68 | none | AuxVarBrancher::post | float_valsel | FloatValBranch | passes to AuxVarBrancher; copied once (last use) | by value + std::move at the copy site | none |
| gecode/flatzinc/flatzinc.cpp:286:44 | none | BranchInformationO::BranchInformationO | v0 | std::vector<BI> | keeps: initializes v; copied once (last use) | by value + std::move into storage | none |
| gecode/flatzinc/flatzinc.cpp:404:54 | none | annotation_to_int_var_branch | rnd | Rnd | passes to INT_VAR_RND | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:453:88 | none | annotation_to_int_val_branch | rnd | Rnd | passes to INT_VAL_RND | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:514:50 | none | annotation_to_int_assign | rnd | Rnd | passes to INT_ASSIGN_RND | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:533:55 | none | annotation_to_bool_var_branch | rnd | Rnd | passes to BOOL_VAR_RND | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:558:89 | none | annotation_to_bool_val_branch | rnd | Rnd | passes to BOOL_VAL_RND | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:619:51 | none | annotation_to_bool_assign | rnd | Rnd | passes to BOOL_ASSIGN_RND | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:637:54 | none | annotation_to_set_var_branch | rnd | Rnd | passes to SET_VAR_RND | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:676:86 | none | annotation_to_set_val_branch | rnd | Rnd | reads only | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:711:56 | none | annotation_to_float_var_branch | rnd | Rnd | passes to FLOAT_VAR_RND | const& (no move constructor) | none |
| gecode/flatzinc/flatzinc.cpp:1795:55 | none | print_int_var | name | const std::string | passes to else | const& (const-qualified by value, so it cannot be moved) | none |
| gecode/flatzinc/flatzinc.cpp:1816:56 | none | print_bool_var | name | const std::string | reads only | const& (const-qualified by value, so it cannot be moved) | none |
| gecode/flatzinc/flatzinc.cpp:1824:57 | none | print_float_var | name | const std::string | reads only | const& (const-qualified by value, so it cannot be moved) | none |
| gecode/flatzinc/parser.hh:258:28 | none | ParserState::output | x | std::string | keeps: stores via output_items.push_back | by value + std::move into storage (move at the last use); `output_items.push_back(std::pair(x, n))`; take by value and `emplace_back(std::move(x), n)` | none |
| gecode/flatzinc/parser.tab.cpp:133:54 | none | get_array_element | id | string | passes to pp->symbols.get | const& (read only); generated: edit gecode/flatzinc/parser.yxx:98 and build `regenerate` | none |
| gecode/flatzinc/parser.tab.cpp:211:52 | none | get_var_ref_arg | id | string | copies into local n; passes to pp->symbols.get, new AST::Atom | const& (read only); generated: edit gecode/flatzinc/parser.yxx:176 and build `regenerate`; `n = id` copies into a local only for annotations, and id is read afterwards, so no move | none |
| gecode/flatzinc/parser.tab.cpp:235:57 | none | add_domain_constraint | id | std::string | passes to new ConExpr | const& (read only); generated: edit gecode/flatzinc/parser.yxx:200 and build `regenerate` | none |

### G14 Gist and search (7 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/gist/gist.hpp:36:49 | gecode/gist.hh:209 | VarComparator::VarComparator<S> | name | std::string | passes to base or delegated constructor TextOutput | const& (forwarded to TextOutput, which makes the only copy) | declarations to the left |
| gecode/gist/gist.hpp:64:37 | gecode/gist.hh:136 | Comparator::compare | x_n | std::string | reads only | const& (read only) | declarations to the left |
| gecode/gist/gist.hpp:74:37 | gecode/gist.hh:138 | Comparator::compare | x_n | std::string | reads only | const& (read only) | declarations to the left |
| gecode/gist/gist.hpp:84:37 | gecode/gist.hh:141 | Comparator::compare | x_n | std::string | reads only | const& (read only) | declarations to the left |
| gecode/gist/gist.hpp:98:37 | gecode/gist.hh:145 | Comparator::compare | x_n | std::string | reads only | const& (read only) | declarations to the left |
| gecode/gist/gist.hpp:109:37 | gecode/gist.hh:134 | Comparator::compare | x_n | std::string | reads only | const& (read only) | declarations to the left |
| gecode/search/cpprofiler/tracer.cpp:54:73 | gecode/search.hh:454 | CPProfilerSearchTracer::CPProfilerSearchTracer | name0 | std::string | keeps: initializes name; copied once (last use) | by value + std::move into storage | declarations to the left |

### G15 tests and examples (43 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| examples/job-shop.cpp:459:32 | none | JobShopProbe::branch | r | Rnd | passes to INT_VAR_RND | const& (no move constructor) | none |
| examples/pentominoes.cpp:300:61 | none | Pentominoes::tile_reg | mark | REG | reads only | const& (no move constructor) | none |
| examples/pentominoes.cpp:300:82 | none | Pentominoes::tile_reg | eol | REG | reads only | const& (no move constructor) | none |
| examples/pentominoes.cpp:320:31 | none | Pentominoes::get_constraint | mark | REG | passes to tile_reg | const& (no move constructor) | none |
| examples/pentominoes.cpp:320:41 | none | Pentominoes::get_constraint | other | REG | passes to tile_reg | const& (no move constructor) | none |
| examples/pentominoes.cpp:320:52 | none | Pentominoes::get_constraint | eol | REG | passes to tile_reg | const& (no move constructor) | none |
| test/branch/int.cpp:49:42 | none | FilteredRndTiesSpace::FilteredRndTiesSpace | r | Rnd | passes to Gecode::INT_VAR_RND | const& (no move constructor) | none |
| test/flatzinc.cpp:103:77 | test/flatzinc.hh:69 | FlatZincTest::FlatZincTest | cmdline_opt | std::vector<std::string> | passes to base or delegated constructor FlatZincTest; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| test/flatzinc.cpp:103:102 | test/flatzinc.hh:70 | FlatZincTest::FlatZincTest | check | OutputCheck | passes to base or delegated constructor FlatZincTest; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| test/flatzinc.cpp:104:42 | test/flatzinc.hh:70 | FlatZincTest::FlatZincTest | before | BeforeRun | passes to base or delegated constructor FlatZincTest; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| test/flatzinc.cpp:108:106 | test/flatzinc.hh:73 | FlatZincTest::FlatZincTest | cmdline_opt | std::vector<std::string> | keeps: initializes _cmdline_opt; copied once (last use) | by value + std::move into storage | declarations to the left |
| test/flatzinc.cpp:109:44 | test/flatzinc.hh:74 | FlatZincTest::FlatZincTest | check | OutputCheck | keeps: initializes _check; copied once (last use) | by value + std::move into storage | declarations to the left |
| test/flatzinc.cpp:109:61 | test/flatzinc.hh:74 | FlatZincTest::FlatZincTest | before | BeforeRun | keeps: initializes _before; copied once (last use) | by value + std::move into storage | declarations to the left |
| test/flatzinc.cpp:114:67 | test/flatzinc.hh:86 | FlatZincErrorTest::FlatZincErrorTest | cmdline_opt | std::vector<std::string> | passes to base or delegated constructor FlatZincErrorTest; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| test/flatzinc.cpp:114:92 | test/flatzinc.hh:86 | FlatZincErrorTest::FlatZincErrorTest | expected_message | std::string | passes to base or delegated constructor FlatZincErrorTest; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| test/flatzinc.cpp:118:67 | test/flatzinc.hh:89 | FlatZincErrorTest::FlatZincErrorTest | cmdline_opt | std::vector<std::string> | passes to base or delegated constructor FlatZincTest; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| test/flatzinc.cpp:118:92 | test/flatzinc.hh:89 | FlatZincErrorTest::FlatZincErrorTest | expected_message | std::string | keeps: initializes _expected_message; copied once (last use) | by value + std::move into storage | declarations to the left |
| test/instance.cpp:118:62 | none | Case::Case | f0 | std::function<bool ()> | keeps: initializes f; copied once (last use) | by value + std::move into storage | none |
| test/int/extensional.cpp:836:42 | none | OpenRegularSpace::OpenRegularSpace | d | DFA | passes to Gecode::extensional | const& (no move constructor) | none |
| test/int/extensional.cpp:1084:43 | none | tuple_set_to_dfa | ts | TupleSet | reads only | const& (no move constructor) | none |
| test/int/extensional.cpp:1194:71 | none | TupleSetTest::TupleSetTest | d0 | IntSet | passes to base or delegated constructor Test | const& (no move constructor) | none |
| test/int/extensional.cpp:1194:92 | none | TupleSetTest::TupleSetTest | ts0 | TupleSet | keeps: initializes ts | const& (no move constructor; the callee's copy stays and is the only copy) | none |
| test/int/extensional.cpp:1236:77 | none | RandomTupleSetTest::RandomTupleSetTest | d0 | IntSet | passes to base or delegated constructor TupleSetTest; copied once (last use) | const& (no move constructor) | none |
| test/int/extensional.cpp:1236:98 | none | RandomTupleSetTest::RandomTupleSetTest | ts0 | TupleSet | passes to base or delegated constructor TupleSetTest | const& (no move constructor) | none |
| test/int/gcc.cpp:249:33 | none | VarSome::VarSome | s | std::string | passes to base or delegated constructor Test | const& (forwarded to Test, which makes the only copy) | none |
| test/int/min-distance.cpp:114:44 | none | Model::Model | distance | std::function<int (int, int)> | passes to Gecode::IntDistance; copied once (last use) | by value + std::move at the copy site | none |
| test/int/open-var-list.cpp:167:44 | none | SelectorSpace::SelectorSpace | vals | IntValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | none |
| test/int/open-var-list.cpp:274:50 | none | SelectorChoiceSpace::SelectorChoiceSpace | vals | IntValBranch | passes to branch; copied once (last use) | const& (read only; copies come from accessors) | none |
| test/int/open-var-list.cpp:1023:54 | none | CallbackSpace::CallbackSpace | domain | OpenIntVarList::Domain | keeps: initializes list; copied once (last use) | by value + std::move into storage | none |
| test/int/open-var-list.cpp:1023:97 | none | CallbackSpace::CallbackSpace | transition | OpenIntVarList::Transition | keeps: initializes list; copied once (last use) | by value + std::move into storage | none |
| test/ldsb.cpp:216:26 | none | LDSB::LDSB<T> | label | std::string | passes to base or delegated constructor Test::Base | const& (forwarded to Test::Base, which makes the only copy) | none |
| test/ldsb.cpp:245:29 | none | LDSBSet::LDSBSet<T> | label | std::string | passes to base or delegated constructor Test::Base | const& (forwarded to Test::Base, which makes the only copy) | none |
| test/ldsb.cpp:1252:31 | none | LDSBLatin::LDSBLatin | label | std::string | passes to base or delegated constructor Test::Base | const& (forwarded to Test::Base, which makes the only copy) | none |
| test/ldsb.cpp:1722:39 | none | EmptyFixture::EmptyFixture<VarArray> | d | IntSet | reads only | const& (no move constructor) | none |
| test/ldsb.cpp:1784:78 | none | EmptyInt::post_branching | vals | IntValBranch | passes to branch | const& (read only; copies come from accessors) | none |
| test/nogoods.cpp:61:33 | none | Queens::Queens | ivb | IntValBranch | passes to branch | const& (read only; copies come from accessors) | none |
| test/nogoods.cpp:114:30 | none | Queens::val | ivb | IntValBranch | reads only | const& (read only; copies come from accessors) | none |
| test/nogoods.cpp:150:34 | none | Hamming::Hamming | svb | SetValBranch | passes to branch | const& (read only; copies come from accessors) | none |
| test/nogoods.cpp:210:30 | none | Hamming::val | svb | SetValBranch | reads only | const& (read only; copies come from accessors) | none |
| test/nogoods.cpp:250:46 | none | NoGoods::NoGoods<Model, ValBranch> | vb0 | IntValBranch / SetValBranch | keeps: initializes vb; passes to Model::val | by value + std::move into storage (move at the last use); the Base initializer reads Model::val(vb0) before vb is initialized (bases first); keep that read and write `vb(std::move(vb0))` | none |
| test/search.cpp:137:52 | none | TreeSpace::TreeSpace | g | std::shared_ptr<const Graph> | keeps: initializes graph; copied once (last use) | by value + std::move into storage | none |
| test/test.cpp:122:28 | test/test.hh:200 | Base::Base | s | std::string | passes to base or delegated constructor Base; copied once (last use) | by value + std::move at the copy site | declarations to the left |
| test/test.cpp:523:34 | none | TestExecutionControl::write_output | output | std::string | reads only | const& (read only) | none |

### G16 FloatVal (only without the FloatVal exception) (53 parameters)

| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |
|---|---|---|---|---|---|---|---|
| gecode/float/dom.cpp:38:41 | gecode/float.hh:1081 | dom | n | FloatVal | passes to Limits::check, xv.eq | const& (no move constructor) | declarations to the left |
| gecode/float/dom.cpp:47:52 | gecode/float.hh:1083 | dom | n | FloatVal | passes to Limits::check, xv.eq | const& (no move constructor) | declarations to the left |
| gecode/float/dom.cpp:82:41 | gecode/float.hh:1089 | dom | n | FloatVal | passes to Limits::check, Rel::ReEqFloat<FloatView, Int::BoolView, RM_EQV>::post, Rel::ReEqFloat<FloatView, Int::BoolView, RM_IMP>::post, Rel::ReEqFloat<FloatView, Int::BoolView, RM_PMI>::post | const& (no move constructor) | declarations to the left |
| gecode/float/linear.cpp:40:73 | gecode/float.hh:1235 | linear | c | FloatVal | passes to Linear::post | const& (no move constructor) | declarations to the left |
| gecode/float/linear.cpp:54:73 | gecode/float.hh:1257 | linear | c | FloatVal | passes to Linear::post | const& (no move constructor) | declarations to the left |
| gecode/float/linear.cpp:68:96 | gecode/float.hh:1282 | linear | c | FloatVal | passes to Linear::post | const& (no move constructor) | declarations to the left |
| gecode/float/linear.cpp:84:96 | gecode/float.hh:1312 | linear | c | FloatVal | passes to Linear::post | const& (no move constructor) | declarations to the left |
| gecode/float/linear/nary.hpp:43:80 | gecode/float/linear.hh:68 | Lin::Lin<P, N, pc> | c0 | FloatVal | keeps: initializes c | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/linear/nary.hpp:122:72 | gecode/float/linear.hh:115 | Eq::Eq<P, N> | c | FloatVal | passes to base or delegated constructor Lin<P, N, PC_FLOAT_BND> | const& (no move constructor) | declarations to the left |
| gecode/float/linear/nary.hpp:127:74 | gecode/float/linear.hh:121 | Eq::post | c | FloatVal | passes to Eq<P, N> | const& (no move constructor) | declarations to the left |
| gecode/float/linear/nary.hpp:254:72 | gecode/float/linear.hh:144 | Lq::Lq<P, N> | c | FloatVal | passes to base or delegated constructor Lin<P, N, PC_FLOAT_BND> | const& (no move constructor) | declarations to the left |
| gecode/float/linear/nary.hpp:259:74 | gecode/float/linear.hh:150 | Lq::post | c | FloatVal | passes to Lq<P, N> | const& (no move constructor) | declarations to the left |
| gecode/float/linear/post.cpp:44:39 | gecode/float/linear.hh:185 | estimate | c | FloatVal | copies into local est | const& (no move constructor) | declarations to the left |
| gecode/float/linear/post.cpp:63:39 | none | overflow | c | FloatVal | copies into local est | const& (no move constructor) | none |
| gecode/float/linear/post.cpp:103:93 | none | post_nary | c | FloatVal | passes to Eq<View, View>::post, Lq<View, View>::post | const& (no move constructor) | none |
| gecode/float/linear/post.cpp:275:64 | gecode/float/linear.hh:222 | post | c | FloatVal | passes to rel | const& (no move constructor) | declarations to the left |
| gecode/float/rel.cpp:42:60 | gecode/float.hh:1110 | rel | n | FloatVal | passes to Limits::check, x.eq, Rel::NqFloat<FloatView>::post, x.lq, x.gq | const& (no move constructor) | declarations to the left |
| gecode/float/rel.cpp:178:59 | gecode/float.hh:1114 | rel | n | FloatVal | passes to Limits::check, Rel::ReEqFloat<FloatView, Int::BoolView, RM_EQV>::post, Rel::ReEqFloat<FloatView, Int::BoolView, RM_IMP>::post, Rel::ReEqFloat<FloatView, Int::BoolView, RM_PMI>::post, Rel::ReEqFloat<FloatView, Int::NegBoolView, RM_EQV>::post, Rel::ReEqFloat<FloatView, Int::NegBoolView, RM_PMI>::post, Rel::ReEqFloat<FloatView, Int::NegBoolView, RM_IMP>::post, Rel::ReLqFloat<FloatView, Int::BoolView, RM_EQV>::post, Rel::ReLqFloat<FloatView, Int::BoolView, RM_IMP>::post, Rel::ReLqFloat<FloatView, Int::BoolView, RM_PMI>::post, Rel::ReLeFloat<FloatView, Int::BoolView, RM_EQV>::post, Rel::ReLeFloat<FloatView, Int::BoolView, RM_IMP>::post, Rel::ReLeFloat<FloatView, Int::BoolView, RM_PMI>::post, Rel::ReLeFloat<FloatView, Int::NegBoolView, RM_EQV>::post, Rel::ReLeFloat<FloatView, Int::NegBoolView, RM_PMI>::post, Rel::ReLeFloat<FloatView, Int::NegBoolView, RM_IMP>::post, Rel::ReLqFloat<FloatView, Int::NegBoolView, RM_EQV>::post, Rel::ReLqFloat<FloatView, Int::NegBoolView, RM_PMI>::post, Rel::ReLqFloat<FloatView, Int::NegBoolView, RM_IMP>::post | const& (no move constructor) | declarations to the left |
| gecode/float/rel.cpp:285:70 | gecode/float.hh:1122 | rel | c | FloatVal | passes to Limits::check, xi.eq, Rel::NqFloat<FloatView>::post, xi.lq, xi.gq | const& (no move constructor) | declarations to the left |
| gecode/float/rel/eq.hpp:272:74 | gecode/float/rel.hh:162 | ReEqFloat::ReEqFloat<View, CtrlView, rm> | c0 | FloatVal | keeps: initializes c | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/rel/eq.hpp:277:69 | gecode/float/rel.hh:170 | ReEqFloat::post | c | FloatVal | passes to x.eq, overlap, ReEqFloat | const& (no move constructor) | declarations to the left |
| gecode/float/rel/lq-le.hpp:226:74 | gecode/float/rel.hh:310 | ReLqFloat::ReLqFloat<View, CtrlView, rm> | c0 | FloatVal | keeps: initializes c | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/rel/lq-le.hpp:231:69 | gecode/float/rel.hh:318 | ReLqFloat::post | c | FloatVal | passes to ReLqFloat<View, CtrlView, rm>, rtest_lq | const& (no move constructor) | declarations to the left |
| gecode/float/rel/lq-le.hpp:313:74 | gecode/float/rel.hh:339 | ReLeFloat::ReLeFloat<View, CtrlView, rm> | c0 | FloatVal | keeps: initializes c | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/rel/lq-le.hpp:318:69 | gecode/float/rel.hh:347 | ReLeFloat::post | c | FloatVal | passes to ReLeFloat<View, CtrlView, rm>, rtest_lt | const& (no move constructor) | declarations to the left |
| gecode/float/rel/nq.hpp:86:56 | gecode/float/rel.hh:219 | NqFloat::NqFloat<View> | _c | FloatVal | keeps: initializes c | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/rel/nq.hpp:91:53 | gecode/float/rel.hh:225 | NqFloat::post | c | FloatVal | passes to overlap, NqFloat<View> | const& (no move constructor) | declarations to the left |
| gecode/float/view/float.hpp:110:41 | gecode/float/view.hpp:106 | FloatView::lq | n | FloatVal | passes to x->lq | const& (no move constructor) | declarations to the left |
| gecode/float/view/float.hpp:123:41 | gecode/float/view.hpp:113 | FloatView::gq | n | FloatVal | passes to x->gq | const& (no move constructor) | declarations to the left |
| gecode/float/view/minus.hpp:107:41 | gecode/float/view.hpp:206 | MinusView::lq | n | FloatVal | passes to x.gq | const& (no move constructor) | declarations to the left |
| gecode/float/view/minus.hpp:120:41 | gecode/float/view.hpp:213 | MinusView::gq | n | FloatVal | passes to x.lq | const& (no move constructor) | declarations to the left |
| gecode/float/view/offset.hpp:115:42 | gecode/float/view.hpp:323 | OffsetView::lq | n | FloatVal | passes to x.lq | const& (no move constructor) | declarations to the left |
| gecode/float/view/offset.hpp:128:42 | gecode/float/view.hpp:330 | OffsetView::gq | n | FloatVal | passes to x.gq | const& (no move constructor) | declarations to the left |
| gecode/float/view/rel-test.hpp:47:31 | gecode/float/view.hpp:541 | rtest_eq | n | FloatVal | reads only | const& (no move constructor) | declarations to the left |
| gecode/float/view/rel-test.hpp:65:31 | gecode/float/view.hpp:548 | rtest_lq | n | FloatVal | reads only | const& (no move constructor) | declarations to the left |
| gecode/float/view/rel-test.hpp:85:31 | gecode/float/view.hpp:555 | rtest_lt | n | FloatVal | reads only | const& (no move constructor) | declarations to the left |
| gecode/float/view/scale.hpp:44:35 | gecode/float/view.hpp:405 | ScaleView::ScaleView | d | FloatVal | keeps: initializes a | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/float/view/scale.hpp:125:41 | gecode/float/view.hpp:451 | ScaleView::lq | n | FloatVal | copies into local c | const& (no move constructor) | declarations to the left |
| gecode/float/view/scale.hpp:144:41 | gecode/float/view.hpp:458 | ScaleView::gq | n | FloatVal | copies into local c | const& (no move constructor) | declarations to the left |
| gecode/minimodel/dom.cpp:105:43 | gecode/minimodel/dom.cpp:71 | DomArgs::DomArgs<Args...> | args | FloatVal | keeps: initializes _args | const& (no move constructor; the callee's copy stays and is the only copy); the pack is stored in the tuple `_args`; with `const Args&... args` write `_args(args...)`, since `std::forward<Args>` of a const& is ill-formed. Change the declaration at dom.cpp:71 too, and the `DomArgs<IntVar, Args...>` constructor (dom.cpp:88, :115) to match. No pack type in use (SetVar, SetRelType, int, IntSet, FloatVar, FloatVal, FloatNum) moves more cheaply than it copies, so each element is still copied once | declarations to the left |
| gecode/minimodel/float-expr.cpp:257:60 | gecode/minimodel.hh:735 | LinFloatExpr::LinFloatExpr | a | FloatVal | keeps: assigns n->a | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/minimodel/float-expr.cpp:311:41 | gecode/minimodel.hh:753 | LinFloatExpr::LinFloatExpr | a | FloatVal | keeps: assigns n->a | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/minimodel/float-expr.cpp:345:76 | gecode/minimodel/float-expr.cpp:67 | LinFloatExpr::Node::fill | m | FloatVal | keeps: assigns tf->a, assigns a; passes to Float::Limits::check, l->fill, r->fill | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/minimodel/float-rel.hpp:49:81 | gecode/minimodel.hh:797 | LinFloatRel::LinFloatRel | r | FloatVal | keeps: initializes e | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| gecode/minimodel/float-rel.hpp:53:39 | gecode/minimodel.hh:799 | LinFloatRel::LinFloatRel | l | FloatVal | keeps: initializes e | const& (no move constructor; the callee's copy stays and is the only copy) | declarations to the left |
| test/float.cpp:222:70 | test/float.hh:221 | TestSpace::rel | n | FloatVal | passes to Gecode::rel | const& (no move constructor) | declarations to the left |
| test/float.hpp:201:32 | test/float.hh:331 | Test::str | f | FloatVal | reads only | const& (no move constructor) | declarations to the left |
| test/float.hpp:216:32 | test/float.hh:338 | Test::cmp | x | FloatVal | reads only | const& (no move constructor) | declarations to the left |
| test/float.hpp:216:76 | test/float.hh:338 | Test::cmp | y | FloatVal | reads only | const& (no move constructor) | declarations to the left |
| test/float.hpp:261:31 | test/float.hh:340 | Test::eq | x | FloatVal | passes to cmp | const& (no move constructor) | declarations to the left |
| test/float.hpp:261:51 | test/float.hh:340 | Test::eq | y | FloatVal | passes to cmp | const& (no move constructor) | declarations to the left |
| test/float/basic.cpp:56:36 | none | Basic::Basic | v | FloatVal | passes to base or delegated constructor Test | const& (no move constructor) | none |
| test/float/dom.cpp:57:41 | none | Val::Val | c0 | FloatVal | keeps: initializes c; passes to str | const& (no move constructor; the callee's copy stays and is the only copy) | none |

## Appendix: scripts

The blocks below are the complete scripts, one block per file, named by the
heading above each block. Step 2 of [Reproduce](#reproduce) extracts them
verbatim. `run.py` runs the clang-tidy passes with the `misc/check.py lint`
coverage. `analyze.py` counts, deduplicates with libclang, classifies uses,
assigns groups and treatments, and writes the tables. `reproduce.sh` runs both
with the AllowedTypes values of the three passes.

#### `run.py`

```python
#!/usr/bin/env python3
"""Run performance-unnecessary-value-param with misc/check.py lint coverage.

Usage: run.py BUILD_DIR OUT_DIR JOBS [ALLOWED_TYPES_REGEX]
Writes OUT_DIR/{config.yaml,commands.json,tidy.log,parser-actions.log,
lexer-actions.log,parser-hh.log,raw.txt,fixes/,fixes-*.yaml}.
"""
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(subprocess.check_output(["git", "rev-parse", "--show-toplevel"], cwd=Path(__file__).parent,
                                    text=True).strip())
LLVM = Path("/opt/homebrew/opt/llvm@22/bin")
TIDY = LLVM / "clang-tidy"
RUNNER = LLVM / "run-clang-tidy"
CHECK = "performance-unnecessary-value-param"
GENERATED_PARSER = {"gecode/flatzinc/parser.tab.cpp", "gecode/flatzinc/parser.tab.hpp",
                    "gecode/flatzinc/lexer.yy.cpp"}


def path_pattern(path: Path) -> str:
    return r"[\\/]".join(re.escape(part) for part in path.as_posix().split("/"))


def action_ranges(source: Path, grammar: str) -> list[list[int]]:
    """Physical line ranges of source that #line maps to grammar (as misc/check.py lint)."""
    ranges, start = [], None
    lines = source.read_text().splitlines()
    for number, line in enumerate(lines, 1):
        directive = re.match(r'#line \d+ "([^"]+)"', line)
        if directive:
            if start is not None and number > start:
                ranges.append([start, number - 1])
            start = number + 1 if Path(directive[1]).name == grammar else None
    if start is not None:
        ranges.append([start, len(lines)])
    if not ranges:
        sys.exit(f"no {grammar} ranges in {source}")
    return ranges


def main() -> None:
    build_dir = Path(sys.argv[1]).resolve()
    out = Path(sys.argv[2]).resolve()
    jobs = sys.argv[3]
    allowed = sys.argv[4] if len(sys.argv) > 4 else ""
    out.mkdir(parents=True, exist_ok=True)
    (out / "fixes").mkdir(exist_ok=True)
    options = {f"{CHECK}.AllowedTypes": allowed} if allowed else {}
    config = json.dumps({"Checks": f"-*,{CHECK}", "CheckOptions": options})
    (out / "config.yaml").write_text(config + "\n")
    sep = r"[\\/]"
    excluded = path_pattern(Path("gecode/third-party")) + sep + "|" + "|".join(
        path_pattern(Path(p)) + "$" for p in sorted(GENERATED_PARSER))
    owned = "(gecode|test|examples|tools)" + sep
    source_filter = "^" + path_pattern(ROOT) + sep + "(?!" + excluded + ")" + owned
    header_filter = path_pattern(ROOT) + sep + owned
    generated_headers = path_pattern(build_dir / "gecode/kernel") + sep + r"var-(type|imp)\.hpp$"
    exclude_filter = (sep + path_pattern(Path("gecode/third-party")) + sep + "|" + sep
                      + path_pattern(Path("gecode/flatzinc")) + sep
                      + r"(parser\.tab\.(cpp|hpp)|lexer\.yy\.cpp)$")
    hf = f"({header_filter}|{generated_headers})"
    common = [f"--checks=-*,{CHECK}", f"--config={config}", "--quiet"]
    commands = [("tidy.log", [str(RUNNER), "-clang-tidy-binary", str(TIDY), f"-checks=-*,{CHECK}",
                 f"-config={config}", "-p", str(build_dir), "-j", jobs, "-quiet", "-extra-arg=-UNDEBUG",
                 "-header-filter", hf, "-exclude-header-filter", exclude_filter,
                 "-source-filter", source_filter, "-export-fixes", str(out / "fixes")])]
    for log, generated, grammar in (("parser-actions.log", "parser.tab.cpp", "parser.yxx"),
                                    ("lexer-actions.log", "lexer.yy.cpp", "lexer.lxx")):
        source = ROOT / "gecode/flatzinc" / generated
        line_filter = json.dumps([{"name": str(source), "lines": action_ranges(source, grammar)}])
        commands.append((log, [str(TIDY), "--extra-arg=-UNDEBUG", "-p", str(build_dir), *common,
                         f"--export-fixes={out / ('fixes-' + log.removesuffix('.log') + '.yaml')}",
                         "--line-filter=" + line_filter, str(source)]))
    commands.append(("parser-hh.log", [str(TIDY), "--extra-arg=-UNDEBUG", "-p", str(build_dir), *common,
                     f"--export-fixes={out / 'fixes-parser-hh.yaml'}", f"--header-filter={hf}",
                     f"--exclude-header-filter={exclude_filter}", str(ROOT / "gecode/flatzinc/parser.hh")]))
    with open(out / "commands.json", "w") as f:
        json.dump([c for _, c in commands], f, indent=1)
    diag = re.compile(r"^(/[^:\n]+):(\d+):(\d+): warning: (.*) \[" + re.escape(CHECK) + r"\]$")
    raw = []
    for log, command in commands:
        result = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        (out / log).write_text(result.stdout)
        found = [m.groups() for m in map(diag.match, result.stdout.splitlines()) if m]
        print(f"{out.name}/{log}: exit {result.returncode}, {len(found)} diagnostics")
        if result.returncode != 0:
            sys.exit(f"{log} failed; see {out / log}")
        raw += found
    with open(out / "raw.txt", "w") as f:
        for path, line, col, msg in raw:
            f.write(f"{Path(path).resolve().relative_to(ROOT)}:{line}:{col}: {msg}\n")
    print(f"{out.name}: raw {len(raw)} unique {len(set(raw))}")


if __name__ == "__main__":
    main()
```

#### `analyze.py`

```python
#!/usr/bin/env python3
"""Summarize the value-param runs written by run.py.

Usage: analyze.py DIR BUILD_DIR
Reads DIR/{no-allowed,allowed,allowed-floatval}/{raw.txt,fixes/*.yaml,fixes-*.yaml},
parses the reporting translation units of DIR/allowed with libclang to find
each parameter's function, definition, and redeclarations, and writes
DIR/{counts.md,groups.md,hooks.md,rows.md,rows.tsv,checks.txt}. Prints the counts.
"""
import collections
import glob
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(subprocess.check_output(["git", "rev-parse", "--show-toplevel"], cwd=Path(__file__).parent,
                                    text=True).strip())
LLVM = Path("/opt/homebrew/opt/llvm@22")
sys.path.insert(0, sorted(glob.glob(str(LLVM / "lib/python3*/site-packages")))[-1])
import clang.cindex as ci  # noqa: E402

ci.Config.set_library_file(str(LLVM / "lib/libclang.dylib"))
RESOURCE_DIR = subprocess.check_output([str(LLVM / "bin/clang"), "-print-resource-dir"], text=True).strip()
CONFIGS = ("no-allowed", "allowed", "allowed-floatval")
LINE = re.compile(r"^([^:]+):(\d+):(\d+): (.*)$")
TYPE = re.compile(r"of type '([^']*)'(?: \(aka '([^']*)'\))?")
NAME = re.compile(r"parameter (?:'([^']*)'|(#\d+))")
K = ci.CursorKind
FUNCTIONS = {K.FUNCTION_DECL, K.CXX_METHOD, K.CONSTRUCTOR, K.DESTRUCTOR, K.CONVERSION_FUNCTION,
             K.FUNCTION_TEMPLATE}
SCOPES = {K.NAMESPACE, K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL, K.CLASS_TEMPLATE,
          K.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION, K.LINKAGE_SPEC, K.UNEXPOSED_DECL, K.FRIEND_DECL}


def module(path: str) -> str:
    parts = path.split("/")
    if parts[0] == "gecode":
        return "gecode/" + parts[1] if len(parts) > 2 else "gecode (top-level headers)"
    return parts[0]


def raw_rows(path: Path) -> list[tuple[str, int, int, str]]:
    return [(m[1], int(m[2]), int(m[3]), m[4]) for m in map(LINE.match, path.read_text().splitlines())]


def unquote(value: str) -> str:
    value = value.strip()
    if value.startswith("'"):
        return value[1:-1].replace("''", "'")
    if value.startswith('"'):
        return bytes(value[1:-1], "utf-8").decode("unicode_escape")
    return value


def rel(file: str) -> str:
    path = Path(file).resolve()
    return str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)


_text: dict[str, str] = {}
_line_starts: dict[str, list[int]] = {}


def source(path: str) -> str:
    if path not in _text:
        _text[path] = (ROOT / path).read_text(errors="replace")
    return _text[path]


def line_of(path: str, offset: int) -> tuple[int, int]:
    """Line and column of a byte offset (sources are ASCII)."""
    if path not in _line_starts:
        data = (ROOT / path).read_bytes()
        _line_starts[path] = [0] + [i + 1 for i, c in enumerate(data) if c == 10]
    starts = _line_starts[path]
    lo, hi = 0, len(starts)
    while lo + 1 < hi:
        mid = (lo + hi) // 2
        if starts[mid] <= offset:
            lo = mid
        else:
            hi = mid
    return lo + 1, offset - starts[lo] + 1


def offset_of(path: str, line: int, col: int) -> int:
    if path not in _line_starts:
        line_of(path, 0)
    return _line_starts[path][line - 1] + col - 1


def parse_fixes(directory: Path) -> list[dict]:
    """Parse the clang-tidy export-fixes YAML subset that LLVM 22 writes."""
    diagnostics = []
    for file in sorted((directory / "fixes").glob("*.yaml")) + sorted(directory.glob("fixes-*.yaml")):
        main_file, current, replacement = None, None, None
        for line in file.read_text().splitlines():
            stripped = line.strip()
            if stripped.startswith("MainSourceFile:"):
                main_file = rel(unquote(stripped.split(":", 1)[1]))
            elif stripped.startswith("- DiagnosticName:"):
                current = {"tu": main_file, "replacements": []}
                diagnostics.append(current)
                replacement = None
            elif current is None:
                continue
            elif stripped.startswith("Message:"):
                current["message"] = unquote(stripped.split(":", 1)[1])
            elif stripped.startswith("- FilePath:"):
                replacement = {"file": rel(unquote(stripped.split(":", 1)[1]))}
                current["replacements"].append(replacement)
            elif stripped.startswith("FilePath:") and replacement is None:
                current["file"] = rel(unquote(stripped.split(":", 1)[1]))
            elif stripped.startswith("FileOffset:"):
                current["offset"] = int(stripped.split(":", 1)[1])
            elif stripped.startswith("Offset:") and replacement is not None:
                replacement["offset"] = int(stripped.split(":", 1)[1])
            elif stripped.startswith("ReplacementText:") and replacement is not None:
                replacement["text"] = unquote(stripped.split(":", 1)[1])
            elif stripped.startswith("Notes:") or stripped.startswith("Level:"):
                replacement = None
    return diagnostics


# --- libclang: the parameter, its function, and every redeclaration -------

class Units:
    def __init__(self, build_dir: Path):
        self.db = json.loads((build_dir / "compile_commands.json").read_text())
        self.index = ci.Index.create()

    def arguments(self, tu: str) -> list[str]:
        file = ROOT / tu
        for entry in self.db:
            if (Path(entry["directory"]) / entry["file"]).resolve() == file:
                args = entry["arguments"] if "arguments" in entry else entry["command"].split()
                break
        else:
            sys.exit(f"{tu} is not in the compilation database")
        result, skip = [], False
        for a in args[1:]:
            if skip:
                skip = False
            elif a in ("-o", "-MF", "-MT", "-MQ"):
                skip = True
            elif a not in ("-c", entry["file"], str(file)):
                result.append(a)
        return result + ["-UNDEBUG", "-resource-dir", RESOURCE_DIR]

    def parse(self, tu: str):
        # parser.hh diagnostics come from its own run; parser.tab.cpp includes it.
        main = "gecode/flatzinc/parser.tab.cpp" if tu == "gecode/flatzinc/parser.hh" else tu
        unit = self.index.parse(str(ROOT / main), args=self.arguments(main))
        errors = [d for d in unit.diagnostics if d.severity >= ci.Diagnostic.Error]
        if errors:
            sys.exit(f"libclang cannot parse {main}: {errors[0]}")
        return unit


def where(cursor) -> tuple[str, int, int]:
    loc = cursor.location
    return rel(loc.file.name), loc.line, loc.column


def redeclarations(unit) -> dict:
    """Map each function's canonical location to all its declarations in unit."""
    found = collections.defaultdict(list)

    def walk(cursor):
        for child in cursor.get_children():
            file = child.location.file
            if file is None or not file.name.startswith(str(ROOT) + "/"):
                continue
            if child.kind in FUNCTIONS:
                key = where(child.canonical)
                found[key].append(child)
                found.setdefault(("at",) + where(child), [key])
            elif child.kind in SCOPES:
                walk(child)
    walk(unit.cursor)
    return found


def parameters(function) -> list:
    return [c for c in function.get_children() if c.kind == K.PARM_DECL]


def qualified(function) -> str:
    names = [function.spelling]
    parent = function.semantic_parent
    while parent is not None and parent.kind not in (K.TRANSLATION_UNIT, K.NAMESPACE):
        names.append(parent.spelling)
        parent = parent.semantic_parent
    return "::".join(reversed(names))


def locate(unit, path: str, line: int, col: int, msg: str):
    """The parameter that a diagnostic at path:line:col reports."""
    file = unit.get_file(str(ROOT / path))
    cursor = ci.Cursor.from_location(unit, ci.SourceLocation.from_position(unit, file, line, col))
    unnamed = NAME.search(msg)[2]
    if cursor.kind in FUNCTIONS and unnamed:
        cursor = parameters(cursor)[int(unnamed[1:]) - 1]
    elif cursor.kind in FUNCTIONS:
        # A parameter pack declared without a name: the last parameter that starts before the location.
        before = [p for p in parameters(cursor) if (p.extent.start.line, p.extent.start.column) <= (line, col)]
        cursor = before[-1] if before else cursor
    elif cursor.kind != K.PARM_DECL:
        cursor = cursor.referenced
    if cursor is None or cursor.kind != K.PARM_DECL:
        sys.exit(f"no parameter at {path}:{line}:{col}")
    return cursor


# --- How the definition uses the parameter (textual) ------------------------

def matching(text: str, i: int, open_c: str, close_c: str) -> int:
    depth = 0
    while i < len(text):
        if text[i] == open_c:
            depth += 1
        elif text[i] == close_c:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return len(text)


CALLEE = re.compile(r"((?:[A-Za-z_]\w*(?:<[^;{}()]*?>)?(?:::|\.|->))*~?[A-Za-z_]\w*(?:<[^;{}()]*?>)?)\s*$")


def callee_at(text: str, i: int, lo: int) -> tuple[str, int]:
    """The call, initializer, or bracket that directly encloses offset i."""
    depth = 0
    while i > lo:
        i -= 1
        c = text[i]
        if c in ")]}":
            depth += 1
        elif c in "([{":
            if depth == 0:
                if c == "[":
                    return "[", i
                m = CALLEE.search(text[max(lo, i - 200):i])
                if c == "{" and not m:
                    return "{", i
                return (m[1], i - len(m[0])) if m else ("(", i)
            depth -= 1
        elif c == ";" and depth == 0:
            return "", i
    return "", i


DECLARATION_PREFIX = re.compile(r"^(?!(?:return|else|new|throw|delete|case|do)\b)[\w:<>,\s*&]*[\w>*&]\s+$")
STORE = re.compile(r"(?:\.|->)(push_back|emplace_back|emplace|insert|push|push_front|emplace_front)$")


def uses(path: str, line: int, col: int, name: str) -> list[str]:
    """Classify each use of a parameter in its function definition."""
    if name.startswith("#") or not name:
        return ["unnamed (unused)"]
    text = source(path)
    pos = offset_of(path, line, col)
    depth, i = 0, pos
    while i > 0:
        i -= 1
        if text[i] == ")":
            depth += 1
        elif text[i] == "(":
            if depth == 0:
                break
            depth -= 1
    close = matching(text, i, "(", ")")
    brace, depth = -1, 0
    for j in range(close + 1, len(text)):
        c = text[j]
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        elif depth == 0 and c == ";":
            return ["no body"]
        elif depth == 0 and c == "{":
            brace = j
            break
    end = matching(text, brace, "{", "}")
    found = []
    for lo, hi, init in ((close + 1, brace, True), (brace, end, False)):
        for m in re.finditer(r"\b" + re.escape(name) + r"\b", text[lo:hi]):
            at = lo + m.start()
            if text[at - 1] in ".>" and text[at - 2:at] in ("->",) or text[at - 1] == "." or text[at - 2:at] == "::":
                continue  # a member or qualified name that only shares the spelling
            after = text[at + len(name):at + len(name) + 2]
            callee, cpos = callee_at(text, at, lo)
            start = max(text.rfind(c, lo, at) for c in ";{}") + 1
            statement = text[start:at].lstrip()
            returned = re.match(r"return\b", statement) is not None
            if after.startswith(("(", ".", "->", "[")):
                found.append("calls/reads")
            elif init and callee not in ("", "(", "[", "{"):
                if re.fullmatch(r"std::(forward|move)(<.*>)?", callee):
                    callee, cpos = callee_at(text, cpos, lo)  # the initializer the cast feeds
                prefix = text[lo:cpos]
                nested = prefix.count("(") - prefix.count(")") + prefix.count("{") - prefix.count("}")
                if nested or "." in callee:
                    found.append(f"arg {callee}")
                elif re.match(r"[A-Z]|.*(::|<)", callee):
                    found.append(f"base {callee}")  # base or delegating constructor
                else:
                    found.append(f"init {callee}")
            elif callee == "[":
                found.append("captured by lambda")
            elif callee in ("", "{"):
                lhs = re.search(r"([A-Za-z_][\w.]*(?:->\w+)*)\s*(?<![=!<>+\-*/%&|^])=\s*$", text[start:at])
                if returned:
                    found.append("returned")
                elif lhs:
                    before = text[start:start + lhs.start()]
                    local = before.strip() and DECLARATION_PREFIX.match(before.lstrip()) or re.search(
                        r"[\w>]\s+" + re.escape(lhs[1]) + r"\s*[;={(]", text[brace:start])
                    found.append(f"local {lhs[1]}" if local else f"assign {lhs[1]}")
                elif callee == "{" and DECLARATION_PREFIX.match(text[start:cpos].lstrip() + " "):
                    word = re.search(r"(\w+)\s*$", text[start:cpos])
                    found.append(f"local {word[1]}" if word else "expr")
                else:
                    found.append("expr")
            elif re.search(r"\bnew\s+$", text[start:cpos]):
                found.append(f"arg new {callee}")  # a pointer is stored, not the value
            elif STORE.search(callee) or STORE.search(callee_at(text, cpos, lo)[0]):
                found.append(f"store {callee if STORE.search(callee) else callee_at(text, cpos, lo)[0]}")
            elif ("::" not in callee and "." not in callee and "->" not in callee
                  and DECLARATION_PREFIX.match(text[start:cpos].lstrip())):
                found.append(f"local {callee}")
            else:
                constructs = re.match(r"[A-Z]\w*[a-z]", re.sub(r"<.*>", "", callee).split("::")[-1])
                found.append(f"returned object {callee}" if returned and constructs else f"arg {callee}")
    body = text[brace:end]
    result = []
    for f in found:
        if f.startswith("local "):
            local = f[6:]
            if re.search(r"\breturn\s+" + re.escape(local) + r"\s*;", body):
                f = f"returned local {local}"
        if f not in result:
            result.append(f)
    return result or ["unused"]


KEEP_PREFIXES = ("init ", "assign ", "store ", "returned", "captured by lambda")


def keeps(tokens: list[str]) -> list[str]:
    return [t for t in tokens if t.startswith(KEEP_PREFIXES)]


def describe(tokens: list[str], kinds: list[str]) -> str:
    parts = []
    stored = [t for t in tokens if t.startswith(("init ", "assign ", "store "))]
    if stored:
        parts.append("keeps: " + ", ".join(t.replace("init ", "initializes ").replace("assign ", "assigns ")
                                           .replace("store ", "stores via ") for t in stored))
    if "returned" in tokens:
        parts.append("keeps: returns it")
    for t in tokens:
        if t.startswith("returned object "):
            parts.append(f"keeps: returns a {t[16:]} built from it")
    for t in tokens:
        if t.startswith("returned local "):
            parts.append(f"keeps: builds {t[15:]}, which it returns")
        elif t.startswith("local "):
            parts.append(f"copies into local {t[6:]}")
    if "captured by lambda" in tokens:
        parts.append("keeps: lambda capture")
    bases = [t[5:] for t in tokens if t.startswith("base ")]
    if bases:
        parts.append("passes to base or delegated constructor " + ", ".join(bases))
    forwards = [t[4:] for t in tokens if t.startswith("arg ") and t[4:] not in ("if", "assert", "while")]
    if forwards:
        parts.append("passes to " + ", ".join(forwards))
    if "move" in kinds:
        parts.append("copied once (last use)")
    if not parts:
        parts.append(tokens[0] if tokens[0] in ("unused", "unnamed (unused)") else "reads only")
    return "; ".join(parts)


# --- Repair groups and treatments --------------------------------------------

GROUPS = [
    ("G1 CBS virtual hooks", r"^gecode/kernel/core\.hpp:37|^gecode/int/distinct/"),
    ("G3 kernel branch configurations", r"^gecode/kernel/branch/(var|val|tiebreak|action|chb)\.hpp"),
    ("G4 int branch configurations", r"^gecode/int/branch/(var|val|assign)\.hpp|^gecode/int/branch/(chb|action)\.cpp"),
    ("G5 set branch configurations", r"^gecode/set/branch/(var|val|assign)\.hpp|^gecode/set/branch/(chb|action)\.cpp"),
    ("G6 float branch configurations", r"^gecode/float/branch/(var|val|assign)\.hpp|^gecode/float/branch/(chb|action)\.cpp"),
    ("G7 kernel and int brancher posting", r"^gecode/kernel/branch/(filter|print|view|view-val)\.hpp|^gecode/int/branch\.(cpp|hpp)"
     r"|^gecode/int/branch/(view-values|open)\.hpp|^gecode/int/ldsb\.cpp|^gecode/int/ldsb/brancher\.hpp"),
    ("G9 posting with shared data", r"^gecode/int/(extensional|element|count|sequence|relax)|^gecode/set/(int|relax)\.cpp"
     r"|^gecode/float/relax\.cpp|^gecode/minimodel/(aliases|matrix)|^gecode/set/ldsb/brancher\.hpp:84:|^gecode/int/ldsb/sym-obj"),
    ("G8 set and float brancher posting", r"^gecode/set/branch\.(cpp|hpp)|^gecode/set/ldsb|^gecode/float/branch\.(cpp|hpp)"),
    ("G10 execution callbacks", r"^gecode/kernel/propagator/wait\.hpp|^gecode/kernel/branch/function\.cpp|^gecode/int/exec"
     r"|^gecode/set/exec\.cpp|^gecode/float/exec\.cpp"),
    ("G11 open variable lists", r"^gecode/int/open-var-list"),
    ("G12 tracing", r"^gecode/kernel/trace/|^gecode/(int|set|float)/trace\.cpp"),
    ("G13 FlatZinc", r"^gecode/flatzinc/"),
    ("G14 Gist and search", r"^gecode/gist|^gecode/search/"),
    ("G16 FloatVal (only without the FloatVal exception)", r"FloatVal"),
    ("G15 tests and examples", r"^test/|^examples/"),
]

NO_MOVE = re.compile(r"\b(Rnd|(Int|Bool|Set|Float)?(AFC|Action|CHB)|IntSet|DFA|TupleSet|SharedArray<[^>]*>|IntSharedArray"
                     r"|TraceFilter|REG|IntArgs|ArgArray<[^>]*>|(Int|Bool|Set)VarArgs|FloatVal)$")
SINK_CALLBACK = re.compile(r"BranchFilter|VarValPrint|^std::function<void \(Space &\)>$|^std::function<bool \(const Space &"
                           r"|^std::function<void \(const Space &, const Brancher &|Transition|Domain")
CONFIG = re.compile(r"(Var|Val)Branch|Assign\b|TieBreak")


def group_of(location: str, types: list[str]) -> str:
    if all(re.search(r"(^|::|const )FloatVal$", t) for t in types):
        return "G16 FloatVal (only without the FloatVal exception)"
    for name, pattern in GROUPS:
        if re.search(pattern, location):
            return name
    return "G? unassigned"


def movable(names: list[str]) -> bool:
    for t in names:
        base = t.removeprefix("const ").removeprefix("Gecode::")
        if NO_MOVE.search(base) or NO_MOVE.search(base.split("::")[-1]):
            return False
    return True


def treatment(group: str, names: list[str], kinds: list[str], tokens: list[str]) -> str:
    kept = keeps(tokens)
    joined = " ".join(names)
    if group.startswith("G1 "):
        return "const& (base and all overrides together)"
    if not movable(names):
        return "const& (no move constructor" + ("; the callee's copy stays and is the only copy)" if kept else ")")
    if "const-copy" in kinds:
        return "const& (const-qualified by value, so it cannot be moved" + ("; the callee's copy stays)" if kept else ")")
    if any(re.match(r"arg (new )?(Action::init|CHB::init|Storage)\b", t) for t in tokens) or (
            "calls/reads" in tokens and re.search(r"double \(const Space &, (Int|Bool|Set|Float)Var", joined)
            and group.startswith("G3 ")):
        return "const& (merit only read by Action/CHB storage to seed values)"
    if "captured by lambda" in tokens:
        return "by value + std::move into an init-capture"
    if kept and SINK_CALLBACK.search(joined) and group.startswith(("G7 ", "G8 ", "G10 ", "G11 ")):
        last = "" if "move" in kinds else " (move at the last use)"
        return "by value + std::move into SharedData storage (needs G2)" + last
    if kept:
        last = "" if "move" in kinds else " (move at the last use)"
        return "by value + std::move into storage" + last
    if SINK_CALLBACK.search(joined) and group.startswith(("G7 ", "G8 ", "G10 ", "G11 ")):
        if all(t in ("calls/reads", "expr", "arg if", "arg assert") for t in tokens):
            return "const& (read only)"
        return "by value + std::move on each path into SharedData (needs G2)"
    if CONFIG.search(joined) and not any("TieBreak<" in t or "AuxVarBrancher" in t for t in tokens):
        return "const& (read only; copies come from accessors)"
    if "move" in kinds:
        return "by value + std::move at the copy site"
    bases = [t[5:] for t in tokens if t.startswith("base ")]
    if bases:
        return f"const& (forwarded to {', '.join(bases)}, which makes the only copy)"
    return "const& (read only)"


COUPLING = {
    "gecode/kernel/core.hpp:3781": "virtual Propagator::solution_distribution: base gecode/kernel/core.hpp:1259 and "
    "overrides gecode/int/distinct.hh:75,165,291 (defs gecode/int/distinct/bnd.hpp:462, val.hpp:160, dom.hpp:80)",
    "gecode/kernel/core.hpp:3784": "virtual Propagator::domain_size_sum: base gecode/kernel/core.hpp:1269 and "
    "overrides gecode/int/distinct.hh:77,167,293 (defs gecode/int/distinct/bnd.hpp:468, val.hpp:166, dom.hpp:86)",
    "gecode/int/count/rel.hpp:50": "explicit specialization of is_int_set (primary gecode/int/count/rel.hpp:44, "
    "declared gecode/int/count.hh:54): change primary and specialization together",
    "gecode/int/count/rel.hpp:62": "explicit specialization of isval (primary gecode/int/count/rel.hpp:57, "
    "declared gecode/int/count.hh:57): change primary and specialization together",
}
NOTES = {
    "gecode/int/extensional/open.hpp:108": "never move: the constructor copies dfa0 into dfa and then reads dfa0 in "
    "accepting_distances(dfa0); DFA has no move constructor",
    "gecode/int/extensional/open.hpp:296": "reads dfa for DFA::Symbols, then forwards to the constructor",
    "gecode/int/extensional-regular.cpp:56": "DFA path: extensional -> OpenRegular::post -> OpenRegular constructor",
    "gecode/int/extensional-regular.cpp:63": "DFA path: extensional -> OpenRegular::post -> OpenRegular constructor",
    "gecode/kernel/branch/filter.hpp:90": "constructor reads bf after f(bf) (`if (!bf)`); after moving, test `!f()` "
    "or check before the move",
    "gecode/kernel/branch/print.hpp:93": "constructor reads vvp after p(vvp) (`if (!vvp)`); after moving, test `!p()` "
    "or check before the move",
    "gecode/int/open-var-list.cpp:40": "`if (!d)` runs first; capture with `[d = std::move(d)]`",
    "gecode/flatzinc/parser.hh:258": "`output_items.push_back(std::pair(x, n))`; take by value and "
    "`emplace_back(std::move(x), n)`",
    "gecode/flatzinc/parser.tab.cpp:133": "generated: edit gecode/flatzinc/parser.yxx:98 and build `regenerate`",
    "gecode/flatzinc/parser.tab.cpp:211": "generated: edit gecode/flatzinc/parser.yxx:176 and build `regenerate`; "
    "`n = id` copies into a local only for annotations, and id is read afterwards, so no move",
    "gecode/flatzinc/parser.tab.cpp:235": "generated: edit gecode/flatzinc/parser.yxx:200 and build `regenerate`",
    "gecode/int/count/rel.hpp:50": "template parameter VY is also a view; `const VY&` is fine for views",
    "gecode/int/count/rel.hpp:62": "template parameter VY is also a view; `const VY&` is fine for views",
    "gecode/minimodel/dom.cpp:105": "the pack is stored in the tuple `_args`; with `const Args&... args` "
    "write `_args(args...)`, since `std::forward<Args>` of a const& is ill-formed. Change the declaration at "
    "dom.cpp:71 too, and the `DomArgs<IntVar, Args...>` constructor (dom.cpp:88, :115) to match. No pack type in "
    "use (SetVar, SetRelType, int, IntSet, FloatVar, FloatVal, FloatNum) moves more cheaply than it copies, so "
    "each element is still copied once",
    "test/nogoods.cpp:250":"the Base initializer reads Model::val(vb0) before vb is initialized (bases first); "
    "keep that read and write `vb(std::move(vb0))`",
}


def short_type(types: list[str]) -> str:
    names = []
    for t in types:
        alias = re.sub(r"^Gecode::", "", t.split(" = ")[0])
        if alias not in names:
            names.append(alias)
    text = " / ".join(names)
    return text if len(text) < 90 else text[:87] + "..."


def write_rows_md(base: Path, table: list[dict]) -> None:
    by_group = collections.defaultdict(list)
    for row in table:
        by_group[row["group"]].append(row)
    out = []
    for group in sorted(by_group, key=lambda g: int(re.match(r"G(\d+)", g)[1])):
        rows = by_group[group]
        out.append(f"### {group} ({len(rows)} parameters)\n")
        out.append("| Definition | Other declarations | Function | Parameter | Type | Callee use | Treatment | Coupling |")
        out.append("|---|---|---|---|---|---|---|---|")
        for r in rows:
            key = ":".join(r["definition"].split(":")[:2])
            coupling = COUPLING.get(key) or ("declarations to the left" if r["declarations"] else "none")
            if not r["fix"] and key not in COUPLING:
                coupling += "; clang-tidy offers no fix-it"
            treat = r["treatment"] + ("; " + NOTES[key] if key in NOTES else "")
            cells = [r["definition"], "<br>".join(r["declarations"]) or "none", r["function"], r["name"],
                     short_type(r["types"]), r["use"], treat, coupling]
            out.append("| " + " | ".join(c.replace("|", "\\|") for c in cells) + " |")
        out.append("")
    (base / "rows.md").write_text("\n".join(out))


def write_groups_md(base: Path, table: list[dict]) -> None:
    by_group = collections.defaultdict(list)
    for row in table:
        by_group[row["group"]].append(row)
    out = ["| Group | Parameters | Definition files | Other declaration files |", "|---|---:|---|---|"]
    for group in sorted(by_group, key=lambda g: int(re.match(r"G(\d+)", g)[1])):
        rows = by_group[group]
        defs = sorted({r["definition"].split(":")[0] for r in rows})
        decls = sorted({d.split(":")[0] for r in rows for d in r["declarations"]} - set(defs))
        out.append(f"| {group} | {len(rows)} | {', '.join(f'`{d}`' for d in defs)} | "
                   f"{', '.join(f'`{d}`' for d in decls) or '—'} |")
    (base / "groups.md").write_text("\n".join(out) + "\n")


def write_hooks_md(base: Path, table: list[dict]) -> None:
    """Verification hooks: const& parameters whose callee keeps a copy, and the moves."""
    out = ["| Group | const& parameters whose callee keeps a copy | By value + std::move |", "|---|---|---|"]
    by_group = collections.defaultdict(list)
    for row in table:
        by_group[row["group"]].append(row)
    for group in sorted(by_group, key=lambda g: int(re.match(r"G(\d+)", g)[1])):
        rows = by_group[group]
        kept = [f"`{r['function']}` `{r['name']}` ({r['definition']}: {'; '.join(u[7:] for u in r['use'].split('; ') if u.startswith('keeps: '))})" for r in rows
                if r["treatment"].startswith("const&") and keeps(r["tokens"])]
        moved = collections.Counter(short_type(r["types"]) for r in rows if r["treatment"].startswith("by value"))
        out.append(f"| {group} | {'<br>'.join(kept) or '—'} | "
                   f"{', '.join(f'{t} ×{n}' for t, n in sorted(moved.items())) or '—'} |")
    (base / "hooks.md").write_text("\n".join(out) + "\n")


def main() -> None:
    base = Path(sys.argv[1]).resolve()
    units = Units(Path(sys.argv[2]).resolve())
    counts = {}
    for config in CONFIGS:
        rows = raw_rows(base / config / "raw.txt")
        unique = set(rows)
        counts[config] = (len(rows), len(unique), collections.Counter(module(r[0]) for r in rows),
                          collections.Counter(module(r[0]) for r in unique))
    modules = sorted(set().union(*(c[3] for c in counts.values())))
    out = ["| Module | Raw, no AllowedTypes | Unique, no AllowedTypes | Raw, AllowedTypes | Unique, AllowedTypes "
           "| Raw, with FloatVal exception | Unique, with FloatVal exception |", "|---|---:|---:|---:|---:|---:|---:|"]
    for m in modules:
        out.append(f"| {m} | " + " | ".join(f"{counts[c][2][m]} | {counts[c][3][m]}" for c in CONFIGS) + " |")
    out.append("| **Total** | " + " | ".join(f"**{counts[c][0]}** | **{counts[c][1]}**" for c in CONFIGS) + " |")

    # Every emitted diagnostic is in some translation unit's export file.
    fixes = parse_fixes(base / "allowed")
    tus = collections.defaultdict(set)
    fixed, fix_decls = set(), collections.defaultdict(set)
    for diag in fixes:
        line, col = line_of(diag["file"], diag["offset"])
        key = (diag["file"], line, col, diag["message"])
        tus[key].add(diag["tu"])
        for r in diag["replacements"]:
            text = r.get("text", "")
            if "const" in text or "&" in text or "std::move" in text:
                fixed.add(key)
            if "const" in text or "&" in text:
                fix_decls[key].add((r["file"], line_of(r["file"], r["offset"])[0]))
    unique = sorted(set(raw_rows(base / "allowed" / "raw.txt")))
    missing = [u for u in unique if u not in tus]
    if missing:
        sys.exit(f"{len(missing)} diagnostics have no exported translation unit, e.g. {missing[0]}")
    # Parse as few translation units as cover every diagnostic.
    todo, plan = set(unique), []
    while todo:
        by_tu = collections.Counter(tu for u in todo for tu in tus[u])
        best = min(by_tu, key=lambda tu: (-by_tu[tu], tu))
        plan.append((best, sorted(u for u in todo if best in tus[u])))
        todo -= set(plan[-1][1])
    params = {}
    for tu, diags in plan:
        unit = units.parse(tu)
        decls = redeclarations(unit)
        for path, line, col, msg in diags:
            parm = locate(unit, path, line, col, msg)
            function = parm.semantic_parent
            index = parameters(function).index(parm)
            # A use inside a function template names the templated function,
            # whose canonical cursor is not the template's; go by location.
            key = where(function.canonical)
            if key not in decls:
                key = decls.get(("at",) + where(function), [key])[0]
            family = decls.get(key, []) or [function]
            definitions = [d for d in family if d.is_definition()] or [function]
            places = []
            for d in family:
                p = parameters(d)
                if len(p) > index:
                    places.append(where(p[index]))
            dpath, dline, dcol = where(parameters(definitions[0])[index])
            row = params.setdefault((dpath, dline, dcol), {
                "name": parameters(definitions[0])[index].spelling or NAME.search(msg)[2] or NAME.search(msg)[1], "types": [], "kinds": [], "sites": [],
                "decls": set(), "fix_decls": set(), "fix": False, "function": qualified(definitions[0]),
                "virtual": definitions[0].kind == K.CXX_METHOD and definitions[0].is_virtual_method()})
            t = TYPE.search(msg)
            type_name = (t[1] + (f" = {t[2]}" if t[2] else "")) if t else "?"
            kind = "const-copy" if "const qualified" in msg else "move" if "copied once" in msg else "const-ref"
            if type_name not in row["types"]:
                row["types"].append(type_name)
            if kind not in row["kinds"]:
                row["kinds"].append(kind)
            if kind == "move" and f"{path}:{line}:{col}" not in row["sites"]:
                row["sites"].append(f"{path}:{line}:{col}")
            row["decls"] |= {(p, l) for p, l, c in places if (p, l, c) != (dpath, dline, dcol)}
            row["fix_decls"] |= fix_decls[(path, line, col, msg)]
            row["fix"] |= (path, line, col, msg) in fixed
        print(f"parsed {tu}: {len(diags)} diagnostics")
    table, checks = [], []
    for (path, line, col), row in sorted(params.items()):
        tokens = uses(path, line, col, row["name"])
        names = [t.split(" = ")[0] for t in row["types"]] + [t.split(" = ")[1] for t in row["types"] if " = " in t]
        group = group_of(f"{path}:{line}:{col}", [t.split(" = ")[0] for t in row["types"]])
        decls = [f"{p}:{l}" for p, l in sorted(row["decls"])]
        # Fix-its rewrite every redeclaration they know about; libclang must agree.
        for p, l in sorted(row["fix_decls"] - row["decls"] - {(path, line)}):
            checks.append(f"fix-it declaration {p}:{l} not found by libclang for {path}:{line}:{col}")
        if row["virtual"] and f"{path}:{line}" not in COUPLING:
            checks.append(f"virtual function without a coupling entry: {path}:{line}:{col}")
        table.append({"module": module(path), "definition": f"{path}:{line}:{col}", "name": row["name"],
                      "types": row["types"], "kinds": row["kinds"], "sites": row["sites"],
                      "function": row["function"], "declarations": decls, "tokens": tokens, "fix": row["fix"],
                      "group": group, "treatment": treatment(group, names, row["kinds"], tokens),
                      "use": describe(tokens, row["kinds"])})
        if group.startswith("G?"):
            checks.append(f"unassigned: {path}:{line}:{col}")
    with open(base / "rows.tsv", "w") as f:
        f.write("module\tdefinition\tparameter\ttypes\tkind\tcopy-sites\tfunction\tdeclarations\tuses\tfix-it"
                "\tgroup\ttreatment\tcallee-use\n")
        for r in table:
            f.write("\t".join([r["module"], r["definition"], r["name"], " | ".join(r["types"]), "+".join(r["kinds"]),
                               " ".join(r["sites"]) or "-", r["function"], ", ".join(r["declarations"]) or "none",
                               "; ".join(r["tokens"]), "yes" if r["fix"] else "no", r["group"], r["treatment"],
                               r["use"]]) + "\n")
    write_rows_md(base, table)
    write_groups_md(base, table)
    write_hooks_md(base, table)
    (base / "checks.txt").write_text("".join(c + "\n" for c in checks))
    by_module = collections.Counter(r["module"] for r in table)
    kinds = collections.Counter("+".join(sorted(r["kinds"])) for r in table)
    diag_kinds = collections.Counter("const-copy" if "const qualified" in u[3] else "move" if "copied once" in u[3]
                                     else "const-ref" for u in unique)
    out.append("")
    out.append("Unique parameters with AllowedTypes, by module: "
               + ", ".join(f"{m} {c}" for m, c in sorted(by_module.items())) + f"; total {len(table)}.")
    out.append("Unique diagnostics by kind: " + ", ".join(f"{k} {c}" for k, c in sorted(diag_kinds.items()))
               + ". Parameters by kind: " + ", ".join(f"{k} {c}" for k, c in sorted(kinds.items())) + ".")
    out.append(f"Translation units parsed with libclang: {len(plan)}.")
    (base / "counts.md").write_text("\n".join(out) + "\n")
    for c in CONFIGS:
        print(f"{c} raw {counts[c][0]} unique {counts[c][1]}")
    print(f"unique parameters {len(table)} fix-its {sum(r['fix'] for r in table)} "
          f"with other declarations {sum(bool(r['declarations']) for r in table)} checks {len(checks)}")


if __name__ == "__main__":
    main()
```

#### `reproduce.sh`

```sh
#!/bin/sh
# Usage: sh reproduce.sh [BUILD_DIR] [JOBS]
# Runs the three clang-tidy passes into this script's directory and analyzes them.
set -e
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(git -C "$D" rev-parse --show-toplevel)
BUILD=$(cd "${1:-$ROOT/build/full}" && pwd)
JOBS=${2:-10}
ALLOWED='^Gecode::(Home|Group|PropagatorGroup|BrancherGroup|Pos|Reify)$;^Gecode::(Int|Bool|Set|Float)Var$;^Gecode::(Int|Bool|Set|Float)VarArray$;^Gecode::(VarArray|ViewArray)(<.*>)?$;^Gecode::(OpenIntVarList|OpenBoolVarList|OpenVarList|OpenViewList)(<.*>)?$;^Gecode::(VarImpView|DerivedView|ConstView)(<.*>)?$;^Gecode::(Int|Set|Float)::[A-Za-z]*View(<.*>)?$;^Gecode::.*::VarType$'
FLOATVAL='^Gecode::FloatVal$'
rm -rf "$D/no-allowed" "$D/allowed" "$D/allowed-floatval"
python3 "$D/run.py" "$BUILD" "$D/no-allowed" "$JOBS"
python3 "$D/run.py" "$BUILD" "$D/allowed" "$JOBS" "$ALLOWED"
python3 "$D/run.py" "$BUILD" "$D/allowed-floatval" "$JOBS" "$ALLOWED;$FLOATVAL"
python3 "$D/analyze.py" "$D" "$BUILD"
```
