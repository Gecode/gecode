# Enum inventory

## Result

This answers scoped-enums-001. It lists every unscoped enum in the installed
headers at HEAD `ece88c1d4a` (work-context snapshot `Wd79c4d8776aa2aa0`),
records how each enum's values are used, and recommends whether to make it an
`enum class` with `using enum` at the enclosing scope, the spelling the brief
settles. Candidate constant renames for task 002 follow the table.

The measure is a compiler probe. For each named enum, a scratch copy of HEAD
changes `enum E {...};` to `enum class E {...}; using enum E;` (for the one
`typedef enum {...} E;`, to the same named form) and recompiles
every translation unit (TU) that includes the header. Each error is an
operation that scoping blocks. `using enum` keeps every enumerator name
visible where it is today, so `rel(home, x, IRT_EQ, y)`, same-enum `==`, `<`,
and `switch` keep compiling, and only conversions and operator uses fail.

The installed headers hold 80 unscoped enum declarations: 77 named enums, 2
anonymous constant holders, and Bison's `yytokentype`. One of the 77 is the
cumulatives sweep-line event type `ev_t`, an anonymous enum named by
`typedef enum {...} ev_t;`. All 77 named enums were probed. The counts:

| outcome | enums |
|---|---|
| probe compiles with no change: scope | 53 |
| probe reports errors, scope with local changes | 15 |
| probe reports errors, keep unscoped | 9 |

**Recommendation: scope 68 of the 77 named enums and keep 9.** The 9 to keep
are `TraceEvent`, whose values form an `int` event set in the public trace
API, and eight implementation enums whose values are bits or bit positions
kept in an `int`: the two `MaxPropStatus` enums (int and float arithmetic),
`Int::Linear::BinMod` and `TerMod`, `Int::GCC::Node::NodeFlag` and
`Edge::EdgeFlag`, and Gist's `VisualNodeFlags` and `SpaceNodeFlags`. The 2
anonymous enums and `yytokentype` are out of scope.

Among the public enums, the user-visible breaks are small. User code that
compiles today keeps compiling, except code that stores one of these values in
an `int`, does arithmetic or bitwise operations on it, or indexes an array
with it. In this tree such code exists only inside Gecode, its driver, and its
test harness. No example and no tool needs a change for any recommended enum.

### ExecStatus and SpaceStatus

Both can be scoped. The probe separates the kernel's own uses from what a
propagator or engine author writes:

- **Kernel-internal uses all keep compiling.** `Space::status()` and the
  advisor and post paths in `core.cpp` and `core.hpp` switch on the status,
  including the duplicate values (`ES_OK == ES_NOFIX`,
  `ES_NOFIX_FORCE == ES_PARTIAL_`), which are legal in an `enum class`.
  `GECODE_ES_CHECK` and `GECODE_ES_FAIL` (`kernel/macros.hpp:92-113`) test
  `es < ES_OK`, a same-type relational comparison that still compiles.
  `ES_SUBSUMED_DISPOSED` returns enumerators. None of these needs a cast. The
  ordering of the values (failed and subsumed below `ES_OK`) remains an
  invariant those macros rely on, so the values must not be renumbered.
- **One break in propagator-style code.** `ValCommitFunction::commit()`
  (`kernel/branch/val-commit.hpp:130`) returns `ES_FAILED : ES_OK` from a
  function whose result type is `ModEvent`. That is a latent type confusion
  that works only because `ES_FAILED == ME_GEN_FAILED == -1` and
  `ES_OK == ME_GEN_NONE == 0`. The fix is to return `ME_GEN_FAILED` and
  `ME_GEN_NONE`. With that one-line change applied, a second probe of all 480
  dependent TUs reports 0 errors, so the single header error did not hide
  other uses.
- **What breaks in user code.** Propagators that return `ES_FIX`, use
  `GECODE_ES_CHECK`, `GECODE_ME_CHECK`, `GECODE_REWRITE`, or
  `home.ES_SUBSUMED(*this)`, or switch on a status are unaffected. Code that
  stores an `ExecStatus` in an `int`, returns it as `ModEvent` or `bool`, or
  compares it with an integer literal breaks. No example, test, or tool does
  so.

`SpaceStatus` compiles unchanged in all 480 dependent TUs: the kernel, every
search engine (sequential, parallel, and the experimental ones), Gist, the
FlatZinc interpreter, and the tests use it only with `switch`, `==`, and `!=`.
A third-party search engine written the same way is unaffected.

### IntPropLevel

`IntPropLevel` is a value-and-option bit set: the low two bits select
`IPL_VAL`, `IPL_BND`, or `IPL_DOM`, and bits 2 and 3 are `IPL_BASIC` and
`IPL_ADVANCED`. Callers combine and mask it, and today every combination must
cast back from `int` (`static_cast<IntPropLevel>(IPL_BND | IPL_BASIC)`). The
probe finds 42 blocked sites: the masking in `vbd()` and `ba()`
(`int/ipl.hpp:37`, `:42`), sequence's default (`int/sequence/open.hpp:267`),
the option parser that builds the level in an `int` (`driver/options.cpp`,
12 sites), FlatZinc's annotation mapping (`flatzinc/flatzinc.cpp:2607-2622`,
7 sites), the 16 bit-field widths in `IntPropLevels` (`minimodel.hh:105-120`),
and 4 test sites.

Proposed, next to the enum in `gecode/int.hh`:

```cpp
enum class IntPropLevel : unsigned int { IPL_DEF = 0, ..., IPL_BITS_ = 4 };
using enum IntPropLevel;

/// Combine propagation levels \a a and \a b
constexpr IntPropLevel operator|(IntPropLevel a, IntPropLevel b) noexcept;
/// Intersect propagation levels \a a and \a b
constexpr IntPropLevel operator&(IntPropLevel a, IntPropLevel b) noexcept;
/// Add propagation level \a b to \a a
constexpr IntPropLevel& operator|=(IntPropLevel& a, IntPropLevel b) noexcept;
```

Each returns `static_cast<IntPropLevel>(std::to_underlying(a) op
std::to_underlying(b))`. Inside the enum body, `IPL_BASIC_ADVANCED =
IPL_BASIC | IPL_ADVANCED` still compiles, because prior enumerators have the
underlying type there. With these operators the remaining conversions become
explicit:

- The 16 bit-field widths `IntPropLevel _linear : IPL_BITS_;` need an integer
  constant: `static_cast<int>(IPL_BITS_)`, or better a separate
  `constexpr int` (see the renames).
- `driver/options.cpp` builds the level in `int b` and `int m`; they become
  `IntPropLevel`, and `if (cur & IPL_BASIC)` becomes
  `if ((cur & IPL_BASIC) != IPL_DEF)`, since an `enum class` does not convert
  to `bool`.
- `flatzinc.cpp:2607` declares `int ipl`; it becomes `IntPropLevel`.
- The tests at `test/int.hpp:211`, `:213` use `!= IPL_DEF` as above.
  `test/flatzinc/native-registry.cpp:53` and `test/int/inter-distance.cpp:393`
  compile once `operator|` exists.
- `vbd()`, `ba()`, and `sequence/open.hpp:267` keep their `static_cast`, which
  becomes a no-op and can be dropped.

With exactly this patch (`intproplevel.patch` and its unsigned variant, see
Method), a probe of all 440 dependent TUs reports 0 errors. No `operator~`,
`^`, or shift is needed.

**The underlying type must be unsigned.** `IntPropLevels` stores levels in
4-bit bit-fields. An `enum class` without a stated type has underlying type
`int`, so a 4-bit field is signed and holds -8 to 7: `IPL_ADVANCED` (8) reads
back as -8, and `IPL_DOM | IPL_FULL` (15) as -1. The code compiles, and
neither AppleClang nor GCC 14 warns at `-Wall -Wextra`. Clang reports it only
under `-Wbitfield-enum-conversion`. Giving the enum `: unsigned int` (or
widening the field to 5 bits) avoids this. The same clang run also warns
(`-Wsigned-enum-bitfield`) that under the Microsoft ABI the current unscoped
enum is already signed. If so, MSVC builds may already lose `IPL_ADVANCED` in
`IntPropLevels` today. That was not verified on MSVC.

### FlatZinc grammar actions and genvarimp output

The only enum the grammar uses is `FlatZinc::SymbolType`
(`flatzinc/parser.hh:83`), in `case ST_*:` labels and `e.t == ST_*` tests in
`parser.yxx`. Its probe compiles `parser.tab.cpp` (Bison's `#line` directives
map its diagnostics to `./gecode/flatzinc/parser.yxx`) and the other
dependent TU with 0 errors. `lexer.lxx` uses no enum. The FlatZinc
interpreter's own errors for `ScriptMode`, `RestartMode`, and `IntPropLevel`
are in `flatzinc.hh` and `flatzinc.cpp`, not in generated code.

The genvarimp output (`kernel/var-type.hpp`, `kernel/var-imp.hpp`) declares
and uses no enum: its modification events and propagation conditions are the
`int` constants `ME_*` and `PC_*`, which the brief keeps. No probe reported an
error in either file.

### Enums declared in class templates

`using enum` cannot name a dependent type, so the brief's spelling fails for
the two enums declared inside class templates:
`Int::Extensional::CompactStatus<Advisor>::StatusType` and
`Search::Par::Engine<Tracer>::Cmd`. Their first probes report 13 and 17
errors, all caused by that declaration. Re-exporting each enumerator as
`static constexpr StatusType SINGLE = StatusType::SINGLE;` instead gives
0 errors for `Cmd` (2 TUs). For `StatusType` it leaves 4 errors where
`CompactStatus` packs the status into the tagged pointer
`Support::uptrint_t s` (`gecode/int/extensional/compact.hpp:141`, `:160`,
`:169`, `:175`); with those and `:162` written as `std::to_underlying(...)`,
the probe reports 0 errors.
Task 003 needs either this spelling for the two enums or a move of each
enum out of its template.

### Findings outside the enum question

- `test/float.cpp:722` reads `} else if ((sol = MT_FALSE) &&
  s->r.var().assigned()) {`. It assigns where it means to compare, so the
  branch never runs and `sol` is overwritten for the `RM_PMI` checks below it.
  Scoping `Test::Float::MaybeType` rejects it, because the assigned enum no
  longer converts to `bool`.
- `test/set/mm-set.cpp:151` and `:278` build test names with `str(srt0)` for
  a `Gecode::SetRelType`. The class derives from `Test::Int::Test`, which has
  no `str(SetRelType)`, so the relation prints as its integer value: the tests
  are listed as `Int::MiniModel::SetExpr::Const::000::0::0`. Scoping
  `SetRelType` turns this into a compile error. The fix,
  `Test::Set::SetTest::str(srt0)`, would rename those tests.
- `kernel/branch/val-commit.hpp:130` returns `ExecStatus` values as a
  `ModEvent`, as described above.

## Inventory

One row per named unscoped enum in the installed headers, 77 in all. Tiers
are defined under Classification. "Blocked" counts the deduplicated compiler
errors of the enum's probe, by operation, with their call sites; when there
are more than a few, the full list is in the call-site appendix under the
enum's name. "Explicit" lists what changes at those sites when the enum is
scoped. A row whose probe reports 0 errors compiles unchanged with
`enum class` and `using enum` (for `ev_t`, once its `typedef` is written as
a named enum): its values are only passed, stored in the enum
type, compared with each other, or switched on. All paths are relative to the
repository root.

### Kernel

| enum | location | tier | values | blocked (count): call sites | operators to add | explicit | recommendation |
|---|---|---|---|---|---|---|---|
| `Gecode::ExecStatus` | `gecode/kernel/core.hpp:482` | P | 7 | implicit conversion (1): `gecode/kernel/branch/val-commit.hpp:130` returns `ES_FAILED : ES_OK` as `ModEvent` | none | none; return `ME_GEN_FAILED : ME_GEN_NONE` instead (latent type confusion) | **scope**: kernel uses compile unchanged, one in-tree fix, re-probe 0 errors |
| `Gecode::PropCost::ActualCost` | `gecode/kernel/core.hpp:501` | P | 16 | 3705 errors in the first probe, nearly all a cascade: the array bound `ActorLink queue[PropCost::AC_MAX + 1]` (`gecode/kernel/core.hpp:1908`) stops compiling, which breaks `Space` itself in every TU. The real sites are 9, all in the kernel: arithmetic `AC_MAX + 1` (5) at `gecode/kernel/core.hpp:1908`, `:4261`, `:4275`, `:4284`, `gecode/kernel/core.cpp:274`; `int <= AC_MAX` (2) at `gecode/kernel/core.cpp:129`, `:880`; array index (2) at `gecode/kernel/core.hpp:4253` (`queue[cost.ac]`) and `gecode/kernel/core.cpp:539` (`queue[AC_RECORD]`), found by the re-probe | none | `std::to_underlying` at the 9 kernel sites, or one `static constexpr int` queue count in `Space`; `PropCost::cost()` and the factory functions compile unchanged | **scope**: the values are cost-queue indices, but only the kernel indexes with them; propagators use `PropCost::linear(PropCost::LO, n)` and friends, which compile unchanged |
| `Gecode::PropCost::Mod` | `gecode/kernel/core.hpp:524` | P | 2 | none | none | none | **scope** |
| `Gecode::ActorProperty` | `gecode/kernel/core.hpp:566` | P | 4 | bitwise `&` (8): `p & AP_*` in `Space::notice()` and `Space::ignore()`, `gecode/kernel/core.hpp:4289-4317` | none | the 8 tests become `p == AP_*`; `notice()` and `ignore()` take one property, and no caller combines them (`git grep 'AP_[A-Z_]* *\|'` is empty) | **scope**: kernel-only change |
| `Gecode::ViewTraceInfo::What` | `gecode/kernel/core.hpp:930` | P | 4 | bitwise `\|` (3) and assignment to `uptrint_t` (1): pointer tagging in `gecode/kernel/core.hpp:3514-3526`; bit position `1 << what` (11): `gecode/kernel/trace/filter.cpp` 6, `gecode/kernel/trace/filter.hpp` 5 | none | `std::to_underlying(PROPAGATOR)` and so on at the 15 kernel sites; users only read `what()` and compare | **scope**: all sites are kernel trace internals |
| `Gecode::PropagateTraceInfo::Status` | `gecode/kernel/core.hpp:995` | P | 4 | none | none | none | **scope** |
| `Gecode::PostTraceInfo::Status` | `gecode/kernel/core.hpp:1063` | P | 3 | none | none | none | **scope** |
| `Gecode::NGL::Status` | `gecode/kernel/core.hpp:1392` | P | 3 | none | none | none | **scope** |
| `Gecode::MetaInfo::Type` | `gecode/kernel/core.hpp:1672` | P | 2 | none | none | none | **scope** |
| `Gecode::MetaInfo::RestartReason` | `gecode/kernel/core.hpp:1679` | P | 5 | none | none | none | **scope** |
| `Gecode::SpaceStatus` | `gecode/kernel/core.hpp:1752` | P | 3 | none | none | none | **scope**: kernel, engines, Gist, FlatZinc, and tests use only `switch`, `==`, `!=` |
| `Gecode::TraceEvent` | `gecode/kernel/trace/recorder.hpp:39` | P | 8 | bitwise `\|` `\|=` `&` (38), `-=` (7), `&` inside the `GECODE_STATUS_TRACE` macro (5): `gecode/kernel/core.cpp:433`, `:438`, `:455`, `:476`, `:481`; 50 in all: `gecode/driver/options.cpp` 29, `gecode/kernel/core.cpp` 8, `gecode/int.hh` 4, `gecode/float.hh` 2, `gecode/set.hh` 2, `gecode/kernel.hh` 2, `gecode/kernel/trace/recorder.hpp` 3 | would need an event-set type | every `int te` parameter and `events()` result in the public trace API | **keep**: the values are bits of an `int` event set that users pass to `trace()`; scoping needs a new set type, which is an API redesign outside this area |
| `Gecode::TFE::NodeType` | `gecode/kernel/trace/filter.hpp:49` | I | 3 | none | none | none | **scope** (protected member of `TFE`) |

### Search

| enum | location | tier | values | blocked (count): call sites | operators to add | explicit | recommendation |
|---|---|---|---|---|---|---|---|
| `Gecode::SearchTracer::EngineType` | `gecode/search.hh:197` | P | 6 | arithmetic `+` (2) in the array bound `t2s[EngineType::AOE + 1]`: `gecode/search.hh:381`, `gecode/search/tracer.cpp:37`; array index (1) hidden by the invalid bound and found by the re-probe: `gecode/search/tracer.cpp:45`, `t2s[engine(e).type()]` | none | `std::to_underlying` at the 3 sites in `StdSearchTracer` | **scope** |
| `Gecode::SearchTracer::NodeType` | `gecode/search.hh:281` | P | 3 | none | none | none | **scope** |
| `Gecode::Search::HeuristicGuidance::Sense` | `gecode/search.hh:1268` | P (experimental) | 2 | none | none | none | **scope** |
| `Gecode::Search::Beam::Offer` | `gecode/search/heuristic.hh:191` | I (experimental) | 2 | none | none | none | **scope** |
| `Gecode::Search::Seq::ExpansionStep::State` | `gecode/search/seq/choice-expansion.hh:47` | I (experimental) | 3 | none | none | none | **scope** |
| `Gecode::Search::Par::Engine<Tracer>::Cmd` | `gecode/search/par/engine.hh:163` | I | 4 | `using enum` names a dependent type (1), which hides every enumerator (16): `gecode/search/par/engine.hpp` 8, `bab.hh` 4, `dfs.hh` 4 (their `using Engine<Tracer>::C_*` declarations) | none | none beyond the spelling; with `static constexpr Cmd C_WORK = Cmd::C_WORK;` and so on the probe reports 0 errors | **scope** with per-enumerator `static constexpr` aliases (see "Enums declared in class templates") |
| `Gecode::CPProfiler::NodeStatus` | `gecode/search/cpprofiler/message.hpp:50` | I | 4 | none; serialized with existing `static_cast<char>` and read back with `static_cast<NodeStatus>` | none | none | **scope** |
| `Gecode::CPProfiler::MessageMarshalling::Field` | `gecode/search/cpprofiler/message.hpp:289` | I | 4 | none; same casts as above | none | none | **scope** |

### Integer, set, and float modules

| enum | location | tier | values | blocked (count): call sites | operators to add | explicit | recommendation |
|---|---|---|---|---|---|---|---|
| `Gecode::ReifyMode` | `gecode/int.hh:1140` | P | 3 | bit position `1 << rm` (27) in the test harness: `test/int.hpp` 15, `test/float.hpp` 9, `test/int/mm-lin.cpp` 3 | none | `1 << std::to_underlying(rm)` at the 27 test sites (or a small helper in the harness) | **scope**: library and examples compile unchanged |
| `Gecode::IntRelType` | `gecode/int.hh:1218` | P | 6 | none | none | none | **scope** |
| `Gecode::BoolOpType` | `gecode/int.hh:1243` | P | 5 | none | none | none | **scope** |
| `Gecode::IntPropLevel` | `gecode/int.hh:1267` | P | 9 | bitwise `\|` `\|=` `&` (16), conversion to and from `int` (10), bit-field width (16); 42 in all, see "IntPropLevel" and the appendix | `operator\|`, `operator&`, `operator\|=` on `IntPropLevel`, returning `IntPropLevel` | 16 bit-field widths, `int` locals in `driver/options.cpp` and `flatzinc/flatzinc.cpp:2607`, `cur & IPL_BASIC` tests become `!= IPL_DEF` | **scope** with `: unsigned int` and the three operators; re-probe 0 errors |
| `Gecode::TaskType` | `gecode/int.hh:1298` | P | 3 | none | none | none | **scope** |
| `Gecode::TupleSet::Data::State` | `gecode/int.hh:2676` | I | 5 | none | none | none | **scope** |
| `Gecode::IntVarBranch::Select` | `gecode/int.hh:5210` | P | 30 | none | none | none | **scope** (class-scope `using enum`) |
| `Gecode::BoolVarBranch::Select` | `gecode/int.hh:5298` | P | 12 | none | none | none | **scope** |
| `Gecode::IntValBranch::Select` | `gecode/int.hh:5498` | P | 11 | none | none | none | **scope** |
| `Gecode::BoolValBranch::Select` | `gecode/int.hh:5535` | P | 4 | none | none | none | **scope** |
| `Gecode::IntAssign::Select` | `gecode/int.hh:5619` | P | 5 | none | none | none | **scope** |
| `Gecode::BoolAssign::Select` | `gecode/int.hh:5650` | P | 4 | none | none | none | **scope** |
| `Gecode::Int::Arithmetic::MaxPropStatus` | `gecode/int/arithmetic/max.hpp:176` | I | 3 | `int status` built with `\|=`, `&=`, `~`, `&` (7): `gecode/int/arithmetic/max.hpp` | would need `\|`, `&`, `~`, `\|=`, `&=` | - | **keep**: a local bit set held in `int` |
| `Gecode::Int::Cumulatives::ev_t` | `gecode/int/cumulatives/val.hpp:118` | I | 3 | none; `typedef enum {...} ev_t;` names the type of `Event::e` (`:123`) and of `Event`'s constructor parameter (`:131`), and the values are only passed to that constructor (`:261-276`) and compared with `==` and `!=` (`:141-143`, `:303-315`); probe `gecode_int_cumulatives_val.hpp-118-ev_t` with `ev_t-named.patch`: 0 errors in 1 of 1 dependent TU | none | none; the declaration becomes `enum class EventType { EVENT_CHCK, EVENT_PROF, EVENT_PRUN }; using enum EventType;`, and `ev_t` becomes `EventType` at `:123` and `:131` (probe `EventType` with `eventtype-named.patch`: 0 errors) | **scope** as `EventType`: Gecode types are PascalCase (`docs/code-style.md`), `_t` is the C `typedef` idiom, and the type is spelled only at `:123` and `:131` of this internal header, so the rename changes no API; `EVENT_CHCK` and `EVENT_PRUN` are already internal rename candidates |
| `Gecode::Int::Extensional::CompactStatus<Advisor>::StatusType` | `gecode/int/extensional.hh:477` | I | 4 | `using enum` names a dependent type (1), which hides the enumerators (12): `gecode/int/extensional/compact.hpp` | none | with the alias spelling, `std::to_underlying` at the 5 places that store the status in the tagged pointer `Support::uptrint_t s` (`gecode/int/extensional/compact.hpp:141`, `:160`, `:162`, `:169`, `:175`); `type()` already uses `static_cast<StatusType>` | **scope** with per-enumerator `static constexpr` aliases |
| `Gecode::Int::GCC::BC` | `gecode/int/gcc/dom-sup.hpp:47` | I | 2 | none | none | none | **scope** |
| `Gecode::Int::GCC::Node::NodeFlag` | `gecode/int/gcc/dom-sup.hpp:64` | I | 4 | flag arithmetic `&`, `\|=`, `~` (7): `gecode/int/gcc/dom-sup.hpp` | would need a flag-set operator family | - | **keep**: bit flags stored in the `unsigned char` member `nf` |
| `Gecode::Int::GCC::Edge::EdgeFlag` | `gecode/int/gcc/dom-sup.hpp:281` | I | 6 | flag arithmetic `&`, `\|=`, `~` (17) and one implicit conversion: `gecode/int/gcc/dom-sup.hpp` | as above | - | **keep**: bit flags stored in the `unsigned char` member `ef` |
| `Gecode::Int::Linear::BinMod` | `gecode/int/linear/int-bin.hpp:165` | I | 5 | `int bm` work list built with `&`, `-=`, `\|=` in `GECODE_INT_PV` (5) | as above | - | **keep**: a local bit set held in `int` |
| `Gecode::Int::Linear::TerMod` | `gecode/int/linear/int-ter.hpp:124` | I | 7 | as `BinMod`, plus `TM_* \| TM_*` arguments (13) | as above | - | **keep**: as above |
| `Gecode::Int::NValues::RangeEventType` | `gecode/int/nvalues.hh:47` | I | 3 | none | none | none | **scope** |
| `Gecode::Int::Sequence::TakesStatus` | `gecode/int/sequence/set-op.hpp:36` | I | 3 | none | none | none | **scope** |
| `Gecode::Int::SortTaskOrder` | `gecode/int/task.hh:280` | I | 4 | none | none | none | **scope** |
| `Gecode::Int::Event::Type` | `gecode/int/task.hh:497` | I | 6 | bitwise `\|` (1): `gecode/int/task/event.hpp:37`, `e0 \| (i0 << 3)` packs type and task index | none | `std::to_underlying(e0) \| (i0 << 3)`; the reader at `event.hpp` already uses `static_cast<Type>` | **scope** |
| `Gecode::Int::RelTest` | `gecode/int/view.hpp:1704` | I | 3 | none | none | none | **scope** |
| `Gecode::Int::BoolTest` | `gecode/int/view.hpp:1769` | I | 3 | none | none | none | **scope** |
| `Gecode::SetRelType` | `gecode/set.hh:630` | P | 10 | conversion to `int` (2): `test/set/mm-set.cpp:151`, `:278` call `str(int)` | none | call `Test::Set::SetTest::str(srt0)` (renames those tests; see Findings) | **scope** |
| `Gecode::SetOpType` | `gecode/set.hh:647` | P | 4 | none | none | none | **scope** |
| `Gecode::SetVarBranch::Select` | `gecode/set.hh:1196` | P | 26 | none | none | none | **scope** |
| `Gecode::SetValBranch::Select` | `gecode/set.hh:1349` | P | 9 | none | none | none | **scope** |
| `Gecode::SetAssign::Select` | `gecode/set.hh:1421` | P | 9 | none | none | none | **scope** |
| `Gecode::FloatRelType` | `gecode/float.hh:1144` | P | 6 | none | none | none | **scope** |
| `Gecode::FloatVarBranch::Select` | `gecode/float.hh:1682` | P | 26 | none | none | none | **scope** |
| `Gecode::FloatValBranch::Select` | `gecode/float.hh:1835` | P | 4 | none | none | none | **scope** |
| `Gecode::FloatAssign::Select` | `gecode/float.hh:1891` | P | 4 | none | none | none | **scope** |
| `Gecode::Float::Arithmetic::MaxPropStatus` | `gecode/float/arithmetic/min-max.hpp:197` | I | 3 | as the integer `MaxPropStatus` (7): `gecode/float/arithmetic/min-max.hpp` | - | - | **keep**: a local bit set held in `int` |
| `Gecode::Float::RelTest` | `gecode/float/view.hpp:521` | I | 3 | none | none | none | **scope** |

### MiniModel, driver, and FlatZinc

| enum | location | tier | values | blocked (count): call sites | operators to add | explicit | recommendation |
|---|---|---|---|---|---|---|---|
| `Gecode::LinIntExpr::NodeType` | `gecode/minimodel.hh:259` | P | 9 | none | none | none | **scope** |
| `Gecode::LinFloatExpr::NodeType` | `gecode/minimodel.hh:708` | P | 7 | none | none | none | **scope** |
| `Gecode::SetExpr::NodeType` | `gecode/minimodel.hh:981` | P | 7 | none | none | none | **scope** |
| `Gecode::BoolExpr::NodeType` | `gecode/minimodel.hh:1129` | P | 10 | none | none | none | **scope** |
| `Gecode::ScriptMode` | `gecode/driver.hh:88` | P | 4 | conversion to and from `int` (11) and `int == ScriptMode` (1), all at the `Driver::StringOption` boundary, which stores `int`: `gecode/driver/options.cpp:667`, `:686-689`, `gecode/driver/options.hpp:485`, `gecode/flatzinc.hh:272`, `:280-282`, `:326`, `:327` | none | `static_cast<int>` in `add()` and `value(sm)`; the getters already `static_cast<ScriptMode>` | **scope**: examples compile unchanged |
| `Gecode::RestartMode` | `gecode/driver.hh:99` | P | 5 | conversion to `int` (14), same boundary: `gecode/driver/options.cpp:655`, `:691-695`, `gecode/driver/options.hpp:409`, `gecode/flatzinc.hh:264`, `:283-287`, `:403` | none | as `ScriptMode` | **scope** |
| `Gecode::FlatZinc::FlatZincSpace::Meth` | `gecode/flatzinc.hh:494` | P | 3 | none | none | none | **scope** |
| `Gecode::FlatZinc::IntBoolVarBranch::Select` | `gecode/flatzinc/branch.hh:46` | I | 6 | none | none | none | **scope** |
| `Gecode::FlatZinc::SymbolType` | `gecode/flatzinc/parser.hh:83` | I | 16 | none; the grammar actions in `parser.yxx` use `case ST_*` and `==` | none | none | **scope** |

### Gist, support, and iterators

| enum | location | tier | values | blocked (count): call sites | operators to add | explicit | recommendation |
|---|---|---|---|---|---|---|---|
| `Gecode::Gist::NodeStatus` | `gecode/gist/spacenode.hh:43` | I | 6 | shift `s << 20` (1): `gecode/gist/spacenode.hpp:66` packs it into `status_bits` | none | `std::to_underlying(s) << 20`; the getter already uses `static_cast<NodeStatus>` | **scope** |
| `Gecode::Gist::SpaceNode::SpaceNodeFlags` | `gecode/gist/spacenode.hh:120` | I | 3 | bit positions passed as `int` (8): `static const int last_bit = HAS_SOLVED_CHILDREN` at `gecode/gist/spacenode.hh:122`, and `set_flag(int)` and `get_flag(int)` calls in `gecode/gist/spacenode.hpp` (7) | - | - | **keep**: bit positions continued by `VisualNodeFlags` from `SpaceNode::last_bit + 1` |
| `Gecode::Gist::VisualNode::VisualNodeFlags` | `gecode/gist/visualnode.hh:128` | I | 6 | bit positions passed as `int` to `set_flag(int)` and `get_flag(int)` (12): `gecode/gist/visualnode.hpp` | - | - | **keep**: as above |
| `Gecode::Support::BitSetStatus` | `gecode/support/bitset-base.hpp:111` | I | 3 | none | none | none | **scope** |
| `Gecode::Support::IntType` | `gecode/support/int-type.hpp:58` | I | 3 | none | none | none | **scope** |
| `Gecode::Iter::Ranges::CompareStatus` | `gecode/iter/ranges-operations.hpp:59` | I | 3 | none | none | none | **scope** |

### Installed test component

| enum | location | tier | values | blocked (count): call sites | operators to add | explicit | recommendation |
|---|---|---|---|---|---|---|---|
| `Test::MatchType` | `test/test.hh:89` | T | 3 | none | none | none | **scope** |
| `Test::Int::ConTestLevel` | `test/int.hh:153` | T | 4 | none | none | none | **scope** |
| `Test::Float::MaybeType` | `test/float.hh:63` | T | 3 | assignment used as `bool` (2): `test/float.cpp:722`, the `(sol = MT_FALSE)` bug | none | fix to `sol == MT_FALSE` | **scope**: it catches a real bug |
| `Test::Float::AssignmentType` | `test/float.hh:73` | T | 3 | none | none | none | **scope** |

## Classification

Every enum below is declared in an installed header, so users can name it.
The tier says whether it is also an API contract:

- **P, public API.** Declared in namespace `Gecode` or a user-facing class or
  namespace (`Gecode::Search`, `Gecode::Driver`, the classes in
  `gecode/flatzinc.hh`, `Gecode::Gist`'s public classes), or documented in a
  Doxygen `Task*` group. Users pass, store, or compare these values.
- **I, installed internal.** Declared in an implementation namespace
  (`Gecode::Int::*`, `Set::*`, `Float::*`, `Iter`, `Support`, `Search::Seq`,
  `Search::Par`, `TFE`, `CPProfiler`, the Gist and FlatZinc implementation
  classes) or in a kernel class that only the kernel fills in. Users can name
  these, but they are not contracts. The brief owns internal enums too, so
  they get rows.
- **T, installed test component.** `test/test.hh`, `test/int.hh`, and
  `test/float.hh`, installed for the public-test-component consumers.

## Out of scope

- **Anonymous enums** have no type to scope. The two left are integer
  constant holders: `Driver::CombinedStop`'s `SR_*` bits
  (`gecode/driver/script.hpp:69`) and `Gist::Node`'s child-count tags
  `UNDET`, `LEAF`, `TWO_CHILDREN`, `MORE_CHILDREN` (`gecode/gist/node.hh:109`).
  Keep them, or turn them into `static constexpr int` members if task 003
  wants them gone; neither changes behaviour. The cumulatives
  `typedef enum {...} ev_t` (`gecode/int/cumulatives/val.hpp:118`) is not one
  of them: the `typedef` names a type that `Cumulatives::Event` uses, so it
  has a row in the inventory.
- **`yytokentype`** in `gecode/flatzinc/parser.tab.hpp:51` (and its
  `yytoken_kind_t` typedef on line 103) is Bison's token enum, which the brief
  excludes as upstream-generated scaffolding. The Bison-internal
  `yysymbol_kind_t`, `YYENOMEM`, and `YYARGS_MAX` in `parser.tab.cpp` are not
  installed.
- **Third-party code.** None remains in the installed set: the vendored
  Boost copy was removed by boost-005, and no other third-party header is
  installed.
- **Already scoped.** Nine installed enums are `enum class` already:
  `CPProfiler::MsgType` (`search/cpprofiler/message.hpp:58`),
  `Support::FailPoint::Phase` (`support/failpoint.hpp:48`),
  `ExtensionalPropKind` and `MinDistancePropKind` (`int.hh:2592`, `:3166`),
  `Instance::ValueType` (`instance.hh:342`), `Float::Interval`'s `Sign` and
  `ZeroPosition` (`float/interval.hpp:531`, `:615`), `Test::TestTag`
  (`test/test.hh:96`), and `Search::Par`'s `SchedulerLogical`
  (`search/par/engine.hh:97`). Seventeen more are in sources that are not
  installed.
- **Not installed.** 82 unscoped enum declarations are in `.cpp` files or
  headers outside the install set: 63 anonymous option enums in `examples/`
  (`enum { MODEL_..., BRANCH_..., SEARCH_... }` passed to
  `Driver::StringOption`), 9 in `test/` sources, and 10 in `gecode/`. Of
  those 10, 4 lines hold Bison-internal enums in
  `gecode/flatzinc/parser.tab.cpp`, and 6 are in library `.cpp` files
  (`int/extensional/dfa.cpp:145`, `minimodel/int-arith.cpp:47`,
  `float-arith.cpp:45`, `set-expr.cpp:715`, `reg.cpp:60`, `reg.cpp:446`).
  The brief's success criterion covers the installed headers; these can
  follow the same rule later but are not inventoried here.
- **`ModEvent`, `PropCond`, `ModEventDelta`** are `int` typedefs, not enums,
  and stay integers by the brief.

## Candidate constant renames

These are candidates for the user to approve in task 002, not decisions. The
mechanical pass (`rename_candidates.py`) flags every enumerator with a word,
after splitting on `_`, that is neither a settled abbreviation (`EQ`, `NQ`,
`VAL`, `BND`, `DOM`, `MIN`, `MAX`, the known prefixes, and so on) nor an
English word of four or more letters. It flagged 130 enumerators. Most are
plain words the dictionary lacks (`INTVAR`, `NOGOOD`), standard terms (`DFS`,
`LUBY`, `EST`, `LCT`), or settled vocabulary; the review below keeps those
unchanged. A few candidates come from reading rather than from the pass:
`SOT_INTER`, the shared `RM_` and `MT_` prefixes, and `IPL_BITS_`.

Every proposal keeps UPPER_CASE and the enum's prefix. The settled
vocabulary (`ME_*`, `PC_*`, `ES_FIX`, `IRT_EQ`, `ME_INT_VAL`,
`INT_VAR_SIZE_MIN()`) is excluded. "Uses" counts whole-word occurrences,
including declarations and comments, with `git grep -wo NAME -- DIR | wc -l`
for `gecode/`, `examples/`, `test/`, and `tools/` in that order.

### Public API

| current | proposed | reason | uses |
|---|---|---|---|
| `RM_NONE`, `RM_CONSTANT`, `RM_LINEAR`, `RM_LUBY`, `RM_GEOMETRIC` (`RestartMode`) | `RESTART_NONE`, `RESTART_CONSTANT`, `RESTART_LINEAR`, `RESTART_LUBY`, `RESTART_GEOMETRIC` | `RM_` is also `ReifyMode`'s prefix in namespace `Gecode`, and `using enum` puts both sets in that namespace | 11, 5, 5, 5, 5 / 0 / 0 / 0 |
| `RM_PMI` (`ReifyMode`) | `RM_RIMP` | `PMI` is `IMP` spelled backwards, for reverse implication; `RIMP` says it | 307 / 0 / 42 / 0 |
| `TT_FIXP`, `TT_FIXS`, `TT_FIXE` (`TaskType`) | `TT_FIXED_PROCESSING`, `TT_FIXED_START`, `TT_FIXED_END` | the one-letter suffixes are opaque | 21 / 0 / 0 / 0, 17 / 0 / 0 / 0, 14 / 0 / 1 / 0 |
| `SRT_DISJ`, `SRT_CMPL` (`SetRelType`) | `SRT_DISJOINT`, `SRT_COMPLEMENT` | truncated words | 14 / 0 / 38 / 0, 31 / 0 / 35 / 0 |
| `SOT_DUNION` (`SetOpType`) | `SOT_DISJOINT_UNION` | truncated word | 22 / 3 / 16 / 0 |
| `SOT_INTER` (`SetOpType`) | `SOT_INTERSECTION` | truncated word; low priority, the meaning is guessable | 19 / 3 / 21 / 0 |
| `SM_STAT` (`ScriptMode`) | `SM_STATISTICS` | truncated word | 7 / 0 / 1 / 0 |
| `RR_NO`, `RR_SOL`, `RR_CMPL`, `RR_LIM` (`MetaInfo::RestartReason`) | `RR_NONE`, `RR_SOLUTION`, `RR_COMPLETE`, `RR_LIMIT` | truncated words; `RR_INIT` stays | 2, 3, 3, 3 / 0 / 0 / 0 |
| `AOE` (`SearchTracer::EngineType`) | `OTHER` | "any other engine" is not recoverable from `AOE`; the other values are engine names | 8 / 0 / 0 / 0 |
| `IPL_BITS_` (`IntPropLevel`) | a separate `constexpr int`, outside the enum | it is a bit-field width, not a level, and its value 4 equals `IPL_BASIC`; scoping also forces a cast at each of its 16 uses | 17 / 0 / 0 / 0 |

Considered and kept: `ES_NOFIX_FORCE` (documented, and reads as a variant of
`ES_NOFIX`); `ES_SUBSUMED_` and `ES_PARTIAL_`, whose trailing underscore marks
them internal (the public spellings are `home.ES_SUBSUMED(*this)` and
friends); `RM_EQV`, `RM_IMP`, `BOT_EQV`, and `BOT_IMP`, standard logic
abbreviations; the `PropCost::AC_*` names; `SS_*`; and the `Select` values
(`INT_VAR_*` and so on), which are spelled through the settled functions.

### Installed test component

| current | proposed | reason | uses |
|---|---|---|---|
| `CPLT_ASSIGNMENT` (`Test::Float::AssignmentType`) | `COMPLETE_ASSIGNMENT` | truncated; its siblings are `RANDOM_ASSIGNMENT` and `EXTEND_ASSIGNMENT` | 0 / 0 / 66 / 0 |
| `MT_ANY`, `MT_NOT`, `MT_FIRST` (`Test::MatchType`) | `MATCH_ANY`, `MATCH_NOT`, `MATCH_FIRST` | `MT_` is also `Test::Float::MaybeType`'s prefix, and `Test::Float` sees both | 0 / 0 / 4 / 0, 0 / 0 / 3 / 0, 0 / 0 / 3 / 0 |

`CTL_BOUNDS_D` and `CTL_BOUNDS_Z` stay: they are the literature's bounds(D)
and bounds(Z) consistency. Separately, the comments on `MT_FALSE` ("Does
hold") and `MT_TRUE` ("Does not hold") in `test/float.hh:64-65` are swapped
and use `//<`, which Doxygen ignores.

### Internal (tier I)

These are implementation names; renaming them changes no API.

| current | proposed | reason | uses |
|---|---|---|---|
| `EF_MRKLB`, `EF_MRKUB`, `EF_LM`, `EF_UM`, `EF_DEL` (`Int::GCC::Edge::EdgeFlag`) | `EF_MARKED_LBC`, `EF_MARKED_UBC`, `EF_MATCHED_LBC`, `EF_MATCHED_UBC`, `EF_DELETED` | opaque; the comments carry the meaning | 4, 4, 5, 5, 4 / 0 / 0 / 0 |
| `RET_FST`, `RET_LST` (`Int::NValues::RangeEventType`) | `RET_START`, `RET_FINISH` | the comments say "a range starts" and "a range ends"; `RET_END` means "no further events" | 3, 3 / 0 / 0 / 0 |
| `IT_SHRT` (`Support::IntType`) | `IT_SHORT` | truncated, beside `IT_CHAR` and `IT_INT` | 24 / 0 / 0 / 0 |
| `BT_COMP` (`Int::BoolTest`) | `BT_COMPLEMENT` | truncated | 9 / 0 / 0 / 0 |
| `EVENT_CHCK`, `EVENT_PRUN` (`Int::Cumulatives::ev_t`, proposed `EventType`) | `EVENT_CHECK`, `EVENT_PRUNE` | truncated | 5, 5 / 0 / 0 / 0 |

Kept: the GCC `BC` values `UBC` and `LBC` and their `NF_M_*` flags (upper and
lower bound consistency, as in the GCC literature); the task event types
`LRT`, `LCT`, `EST`, `ZRO`, `ERT`, `END` and `SortTaskOrder`'s `STO_*`
(scheduling terms); and the MiniModel node types (`NT_RLIN`, `NT_NONLIN`, and
so on), which only the MiniModel implementation spells.

## Method

All scratch work is under `build/zdev-impl/scoped-enums-001` (`$S` below),
run from the checkout root. Nothing in the checkout changed except this file
and the brief's index. Steps that differ from the approved plan say so.

1. **Scratch tree and baseline.** The source is `git archive HEAD`, configured
   as `build/full` (Debug, Ninja, shared libraries, experimental, Gist and
   Qt 6, MPFR, CP-profiler, examples) plus CBS and audit, so that more
   `#ifdef` code compiles, without fault injection:

   ```sh
   mkdir -p $S/src && git archive HEAD | tar -x -C $S/src
   cmake -S $S/src -B $S/build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
     -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGECODE_ENABLE_EXPERIMENTAL=ON \
     -DGECODE_ENABLE_CBS=ON -DGECODE_ENABLE_AUDIT=ON -DGECODE_ENABLE_GIST=ON \
     -DGECODE_ENABLE_QT=ON -DGECODE_ENABLE_FLATZINC=ON -DGECODE_ENABLE_MPFR=ON \
     -DGECODE_ENABLE_EXAMPLES=ON -DGECODE_ENABLE_CPPROFILER=ON \
     -DCMAKE_CXX_FLAGS=-ferror-limit=0
   ninja -C $S/build -k 0 all gecode-test public-runner-smoke \
     public-int-smoke public-set-smoke public-float-smoke
   ninja -C $S/build -t deps > $S/ninja-deps.txt
   ```

   Deviation: the plan's `ninja -k 0` builds only `all`, 301 of the 490 TUs
   in `compile_commands.json`, because the test executables are
   `EXCLUDE_FROM_ALL`. The test targets are named explicitly. The baseline
   had 0 errors and 0 warnings with AppleClang 21 on macOS arm64 (`all` took
   70 s wall time, the test targets 35 s more). `ninja -t deps` records the
   headers of all 490 TUs.

2. **Installed headers.** `cmake --install $S/build --prefix $S/install`
   installs 636 headers (`find . -name '*.hh' -o -name '*.hpp'` under
   `$S/install/include`). Deriving the list from the rules in
   `CMakeLists.txt:1639-1680` gives the same 636: `git ls-files gecode` headers
   minus the `EXCLUDE` patterns (`mznlib`, `flatzinc/registry-internal.hh`,
   `flatzinc/blackbox*.hh`, `instance/eval.hpp`, `instance/impl.hpp`,
   `exampleplugin`, `standalone-example`, `abi*`), giving 627, plus the
   generated `gecode/support/config.hpp` and the eight test-component files
   `test/{test,int,set,float}.{hh,hpp}`. `kernel/var-type.hpp` and
   `kernel/var-imp.hpp` come from the source tree; the
   `GECODE_REGENERATE_VARIMP` rule installs the same files. `install(DIRECTORY
   gecode ...)` copies every matching header whatever the module options are,
   so the Gist, CP-profiler, and FlatZinc headers, including Bison's
   `flatzinc/parser.tab.hpp`, are installed even when those modules are off.
   The first install ran while a probe had a header modified and copied it;
   it was repeated after the probes, and the installed headers then matched
   HEAD byte for byte (635 headers compared with `git show HEAD:<path>`
   and the generated `config.hpp` with the build tree's copy, 0 differences).

3. **Discovery.** A grep over the installed headers finds 81 lines:

   ```sh
   sed 's|^\./||' $S/installed-headers.txt | grep -v '^gecode/support/config.hpp$' |
     xargs rg -n --pcre2 '^\s*(typedef\s+)?enum\b(?!\s+(class|struct)\b)' > $S/grep-enums.txt
   ```

   The generated `config.hpp` has no match. The 81 lines are 80
   declarations, because `yytokentype` matches twice (`parser.tab.hpp:51` and
   its `typedef` on line 103). Of the 80, 77 name a type: 76 are
   `enum E {...}`, and one is `typedef enum {...} ev_t;`
   (`int/cumulatives/val.hpp:118`), whose body has no name but whose
   `typedef` gives it one. The other 3 are the 2 anonymous constant holders
   and `yytokentype`. `enum_bodies.py` and `enumerators.py` extract
   the bodies (80 enums, 511 enumerators, `$S/enums.json`), and `context.py`
   gives the enclosing scopes, with seven names fixed by hand where an export
   macro or a template confused the brace scan (`FlatZincSpace::Meth`,
   `TupleSet::Data::State`, `NGL::Status`, `MetaInfo::Type`,
   `MetaInfo::RestartReason`, `CPProfiler::MessageMarshalling::Field`, and the
   two template enums). It lists `ev_t` as `Gecode::Int::Cumulatives::<anon>`,
   because it reads the unnamed body rather than the `typedef`; the table
   names it `Int::Cumulatives::ev_t`. The AST cross-check runs `clang-query` with
   `enumDecl(unless(isScoped()))` over every TU (`ast_enums.sh`):
   it parses all 490 TUs with 0 errors (2 min 19 s) and finds 98 distinct
   unscoped enum declarations under `gecode/` and `test/`, 80 of them in
   installed headers. Those 80 are exactly the grep set (`reconcile.py`). The
   only grep line without an AST match is `parser.tab.hpp:103`,
   `typedef enum yytokentype yytoken_kind_t;`, which names the enum declared
   on line 51 instead of declaring one. So every installed enum is seen by
   the compiler, and none is found by grep alone. The other 18 are in
   sources outside the install set (the 19 grep lines there, less the
   `yysymbol_kind_t` typedef line).

4. **Probes.** `probe.py` (appendix) scopes one enum in `$S/src`, compiles
   every TU whose recorded dependencies include the changed header, with its
   exact command from `compile_commands.json` and the object redirected to
   `$S/probe-obj`, restores the header from HEAD, and writes the errors,
   deduplicated by file, line, and column, to `$S/probes/<label>.json`.
   `run-all.sh` runs it for the 77 named enums in `$S/probe-list.txt`, one at
   a time. `probe.py` scopes `enum E {`, so the `ev_t` line names
   `ev_t-named.patch`, which first turns `typedef enum {...} ev_t;` into
   `enum ev_t {...};`; the probe then scopes that as for the others. The
   `ev_t` probe ran later than the others, from the same scratch tree.
   `bucket.py` sorts each error by the blocked operation, and `sites.py`
   writes the call-site appendix.

   Deviations:

   - **A compile driver instead of `ninja -k 0`.** Under ninja, the first
     probe stalled: Qt's AUTOMOC target for Gist depends on the int, set,
     float, and search libraries, and every example and test links Gist, so
     after the first library failed ninja stopped with "cannot make progress
     due to previous errors" at 164 of 536 edges. Compiling the dependent TUs
     directly needs no link and no ordering, so a library error cannot hide
     the TUs after it.
   - **One probe per enum for every tier,** instead of a batched probe for
     tiers I and T. With the driver, a probe of `core.hpp` (480 TUs) took
     about 100 to 140 s and a probe of an implementation header a few
     seconds, so batching was unnecessary, and per-enum probes avoid
     attributing errors by type name, which fails for the 13 `Select` and 6
     `NodeType` enums. `-fsyntax-only` was not needed.
   - **A second host for part of the run.** Another job loaded the Mac
     heavily (load average 180 to 400), so 12 probes ran on a Linux host
     (`erebor`, clang 21, the same options without Qt and Gist, baseline 0
     errors), shipped with `git archive HEAD` and the same scripts: the four
     MiniModel `NodeType`s, `SetValBranch::Select`, `SetAssign::Select`,
     `BitSetStatus`, `IntType`, and the four test-component enums. That tree
     has 13 fewer TUs (the Gist ones); no Gist source names any of these
     enums or their enumerators. Two probes ran on both hosts
     (`SetVarBranch::Select` and `SetOpType`): 0 errors on each.
   - **Lower bounds and patch re-probes.** An error in a declaration (an array
     bound, a bit-field width, an initializer, or a return in a header used
     by every TU) can hide later errors that depend on it. For each enum
     whose recommendation depends on the full list, the probe was repeated
     with the reported sites fixed (`PROBE_PATCH`, patches in
     `$S/patches`, reproduced in "Appendix: patches") until it reported 0
     errors or only expression errors. Each patch applies to HEAD on its own;
     a step-2 patch contains its step-1 changes:

     | enum | first probe | patch | re-probe |
     |---|---|---|---|
     | `ExecStatus` | 1 | `execstatus.patch` | 0 in 480 TUs |
     | `IntPropLevel` | 42 | `intproplevel.patch` | 0 in 440 TUs |
     | `IntPropLevel`, `: unsigned int` | - | `intproplevel-unsigned.patch` | 0 in 440 TUs |
     | `PropCost::ActualCost` | 3705 | `actualcost-step1.patch` (array bound) | 2 in 480 TUs |
     | `PropCost::ActualCost` | - | `actualcost-step2.patch` (+2 indices) | 0 in 480 TUs |
     | `SearchTracer::EngineType` | 2 | `enginetype.patch` (array bound) | 1 in 461 TUs (`tracer.cpp:45`, hidden before) |
     | `SearchTracer::EngineType` | - | `enginetype-step2.patch` | 0 in 461 TUs |
     | `CompactStatus<Advisor>::StatusType` | 13 | alias spelling | 4 in 2 TUs |
     | `CompactStatus<Advisor>::StatusType` | - | alias + `statustype-alias.patch` | 0 in 2 TUs |
     | `Search::Par::Engine<Tracer>::Cmd` | 17 | alias spelling | 0 in 2 TUs |
     | `Cumulatives::ev_t` as `EventType` | 0 | `eventtype-named.patch` (the proposed name) | 0 in 1 TU |

     The kept flag enums were not re-probed; their counts are lower bounds,
     which does not change a keep.
   - **Reproducibility.** Running the `IntPropLevel` probe twice gave the same
     42 errors at the same file, line, and column with the same messages.

5. **Code the probe does not compile.** `off_regions.py` runs `unifdef -l`
   with the probe configuration's certain symbols over every tracked source
   and lists the 948 code lines it removes (Windows, no-MPFR, no-threads,
   `NDEBUG`, fault injection, and so on). Seven tracked sources are compiled
   by no TU in the probe build (`uncompiled-files.txt`): the fault-injection
   files `gecode/support/failpoint.{cpp,hpp}` and `test/fault.cpp`, which
   need `GECODE_ENABLE_FAULT_INJECTION`, and the package fixtures under
   `test/package/`, which build as separate CMake projects. `off_sites.py`
   greps both for any enum's type or enumerator next to an operator, cast, or
   `switch`: 21 hits, all same-enum `==` or `!=`, plain arguments, or
   unrelated `DFS` class names. None is a blocked operation.

6. **Existing casts.** `casts.py` counts `static_cast<E>(...)` and
   `static_cast<int-like>(enumerator)` per enum. These stay legal when the
   enum is scoped. Nonzero counts: `IntPropLevel` 6, `ScriptMode` 2,
   `RestartMode` 2, `ViewTraceInfo::What` 1, `CompactStatus::StatusType` 1
   and 1, and the shared names `NodeStatus`, `Type`, `Field`, and `Status`,
   whose counts cannot be split by name.

7. **Renames.** `rename_candidates.py` flags enumerators as described in
   "Candidate constant renames", followed by review by hand and the
   `git grep -wo` counts.

## Reproduction

From the checkout root at `ece88c1d4a`, with `S=build/zdev-impl/scoped-enums-001`
and the scripts below saved under `$S/scripts`:

```sh
# 1. Scratch tree, baseline, dependencies (commands in Method step 1)
# 2. Installed headers and the derived list
cmake --install $S/build --prefix $S/install
(cd $S/install/include && find . -name '*.hh' -o -name '*.hpp' | LC_ALL=C sort) > $S/installed-headers.txt
git ls-files gecode | grep -E '\.(hh|hpp)$' |
  grep -vE '(^|/)mznlib/|^gecode/flatzinc/registry-internal\.hh$|^gecode/flatzinc/blackbox[^/]*\.hh$|^gecode/instance/(eval|impl)\.hpp$|exampleplugin|standalone-example|(^|/)abi' \
  > $S/derived-gecode.txt                                                       # 627
(sed 's|^|./|' $S/derived-gecode.txt; echo ./gecode/support/config.hpp
 for f in test int set float; do echo ./test/$f.hh; echo ./test/$f.hpp; done) | LC_ALL=C sort > $S/derived-headers.txt
diff $S/installed-headers.txt $S/derived-headers.txt                            # empty, 636 each
# 3. Discovery
sed 's|^\./||' $S/installed-headers.txt | grep -v '^gecode/support/config.hpp$' |
  xargs rg -n --pcre2 '^\s*(typedef\s+)?enum\b(?!\s+(class|struct)\b)' > $S/grep-enums.txt   # 81 lines
python3 -I $S/scripts/enum_bodies.py $S/grep-enums.txt > $S/enum-bodies.txt
python3 -I $S/scripts/enumerators.py $S                                         # 80 511
python3 -I $S/scripts/context.py $S > $S/enum-contexts.txt                      # then the 7 fixes above
sh $S/scripts/ast_enums.sh $PWD/$S                                              # 0 parse errors, 98 enums
python3 -I $S/scripts/reconcile.py $S                                           # 80 in both, grep-only: parser.tab.hpp:103
sed 's|^\./||' $S/installed-headers.txt | grep -v config.hpp |
  xargs rg -n --pcre2 '^\s*enum\s+(class|struct)\b' | wc -l                     # 9 installed enum class
git ls-files gecode test examples tools | grep -E '\.(hh|hpp|cpp|yxx|lxx)$' |
  grep -vxFf <(sed 's|^\./||' $S/installed-headers.txt) |
  xargs rg -n --pcre2 '^\s*(typedef\s+)?enum\b(?!\s+(class|struct)\b)' |
  cut -d/ -f1 | sort | uniq -c                                                  # 63 examples, 10 gecode, 9 test
# 4. Probes (one at a time; each restores the tree). First save the patches in
#    "Appendix: patches" as $S/patches/<name>.patch.
python3 -I - $S/enums.json > $S/probe-list.txt <<'EOF'
import json, sys
for e in json.load(open(sys.argv[1])):
    if e["name"] and e["name"] != "yytokentype":
        print(f"{e['file']}:{e['line']} {e['name']}")
EOF
echo 'gecode/int/cumulatives/val.hpp:118 ev_t patches/ev_t-named.patch' >> $S/probe-list.txt  # 77 lines
sh $S/scripts/run-all.sh $S $PWD                                                # 77 probes
python3 -I $S/scripts/probe.py $S $PWD IntPropLevel gecode/int.hh:1267          # the repeat run
# The re-probe table, row by row:
P=$S/patches
PROBE_PATCH=$P/execstatus.patch python3 -I $S/scripts/probe.py $S $PWD ExecStatus-fixed gecode/kernel/core.hpp:482
PROBE_PATCH=$P/intproplevel.patch python3 -I $S/scripts/probe.py $S $PWD IntPropLevel-fixed gecode/int.hh:1267
PROBE_PATCH=$P/intproplevel-unsigned.patch python3 -I $S/scripts/probe.py $S $PWD IntPropLevel-unsigned gecode/int.hh:1267
PROBE_PATCH=$P/actualcost-step1.patch python3 -I $S/scripts/probe.py $S $PWD ActualCost-step1 gecode/kernel/core.hpp:501
PROBE_PATCH=$P/actualcost-step2.patch python3 -I $S/scripts/probe.py $S $PWD ActualCost-step2 gecode/kernel/core.hpp:501
PROBE_PATCH=$P/enginetype.patch python3 -I $S/scripts/probe.py $S $PWD EngineType-fixed gecode/search.hh:197
PROBE_PATCH=$P/enginetype-step2.patch python3 -I $S/scripts/probe.py $S $PWD EngineType-step2 gecode/search.hh:197
python3 -I $S/scripts/probe.py $S $PWD StatusType-alias gecode/int/extensional.hh:477:alias
PROBE_PATCH=$P/statustype-alias.patch python3 -I $S/scripts/probe.py $S $PWD StatusType-alias-fixed gecode/int/extensional.hh:477:alias
python3 -I $S/scripts/probe.py $S $PWD Cmd-alias gecode/search/par/engine.hh:163:alias
PROBE_PATCH=$P/eventtype-named.patch python3 -I $S/scripts/probe.py $S $PWD EventType gecode/int/cumulatives/val.hpp:118
python3 -I $S/scripts/bucket.py $S                                              # buckets.json, one line per enum
python3 -I $S/scripts/sites.py $S                                               # sites.md (the appendix)
# 5.-7. Uncompiled code, casts, renames
python3 -I $S/scripts/off_regions.py > $S/off-region-lines.txt                  # 948 lines
git ls-files gecode test examples tools | grep -E '\.(hh|hpp|cpp|c)$' |
  grep -vxFf <( (python3 -I -c "import json,sys; [print(e['file']) for e in json.load(open(sys.argv[1]))]" \
                   $S/build/compile_commands.json; sed -n 's/^ *//p' $S/ninja-deps.txt) |
                sed -E "s#^$PWD/$S/(src|build)/##" | sort -u) > $S/uncompiled-files.txt   # 7 files
python3 -I $S/scripts/off_sites.py $S > $S/off-sites.txt                        # 21 hits
python3 -I $S/scripts/casts.py $S > $S/casts.txt
python3 -I $S/scripts/rename_candidates.py $S > $S/rename-flags.txt             # 130 flags
for d in gecode examples test tools; do git grep -wo RM_PMI -- $d | wc -l; done  # and so on per name
```

`uncompiled-files.txt` lists the tracked sources whose path appears in no
`compile_commands.json` entry and in no TU's `ninja -t deps` list. Every input
the steps read is created by an earlier step or given in an appendix: the
scripts in "Appendix: scripts" and the patches in "Appendix: patches".

## Appendix: call sites

Generated by `bucket.py` and `sites.py` from the first probe of each enum with
errors (24 of 77), except where noted. A bucket with 15 or fewer sites lists
each site; larger buckets give per-file counts. The probe host is macOS unless
it says erebor.

### Gecode::ScriptMode (gecode/driver.hh:88)

Probe `gecode_driver.hh-88-ScriptMode` on macOS: 12 errors in 189 of 189
dependent TUs.

- comparison with integer == (1): `gecode/flatzinc.hh:326`
- implicit conversion (11): `gecode/driver/options.cpp:667`,
  `gecode/driver/options.cpp:686`, `gecode/driver/options.cpp:687`,
  `gecode/driver/options.cpp:688`, `gecode/driver/options.cpp:689`,
  `gecode/driver/options.hpp:485`, `gecode/flatzinc.hh:272`,
  `gecode/flatzinc.hh:280`, `gecode/flatzinc.hh:281`, `gecode/flatzinc.hh:282`,
  `gecode/flatzinc.hh:327`

### Gecode::RestartMode (gecode/driver.hh:99)

Probe `gecode_driver.hh-99-RestartMode` on macOS: 14 errors in 189 of 189
dependent TUs.

- implicit conversion (14): `gecode/driver/options.cpp:655`,
  `gecode/driver/options.cpp:691`, `gecode/driver/options.cpp:692`,
  `gecode/driver/options.cpp:693`, `gecode/driver/options.cpp:694`,
  `gecode/driver/options.cpp:695`, `gecode/driver/options.hpp:409`,
  `gecode/flatzinc.hh:264`, `gecode/flatzinc.hh:283`, `gecode/flatzinc.hh:284`,
  `gecode/flatzinc.hh:285`, `gecode/flatzinc.hh:286`, `gecode/flatzinc.hh:287`,
  `gecode/flatzinc.hh:403`

### Gecode::Float::Arithmetic::MaxPropStatus (gecode/float/arithmetic/min-max.hpp:197)

Probe `gecode_float_arithmetic_min-max.hpp-197-MaxPropStatus` on macOS: 7 errors
in 1 of 1 dependent TUs.

- bitwise & (3): `gecode/float/arithmetic/min-max.hpp:233`,
  `gecode/float/arithmetic/min-max.hpp:235`,
  `gecode/float/arithmetic/min-max.hpp:237`
- bitwise |= (2): `gecode/float/arithmetic/min-max.hpp:224`,
  `gecode/float/arithmetic/min-max.hpp:227`
- implicit conversion (1): `gecode/float/arithmetic/min-max.hpp:218`
- unary operator (1): `gecode/float/arithmetic/min-max.hpp:229`

### Gecode::Gist::SpaceNode::SpaceNodeFlags (gecode/gist/spacenode.hh:120)

Probe `gecode_gist_spacenode.hh-120-SpaceNodeFlags` on macOS: 8 errors in 12 of
12 dependent TUs.

- implicit conversion (8): `gecode/gist/spacenode.hh:122`,
  `gecode/gist/spacenode.hpp:50`, `gecode/gist/spacenode.hpp:55`,
  `gecode/gist/spacenode.hpp:60`, `gecode/gist/spacenode.hpp:136`,
  `gecode/gist/spacenode.hpp:141`, `gecode/gist/spacenode.hpp:146`,
  `gecode/gist/spacenode.hpp:151`

### Gecode::Gist::NodeStatus (gecode/gist/spacenode.hh:43)

Probe `gecode_gist_spacenode.hh-43-NodeStatus` on macOS: 1 error in 12 of 12
dependent TUs.

- bitwise << (1): `gecode/gist/spacenode.hpp:66`

### Gecode::Gist::VisualNode::VisualNodeFlags (gecode/gist/visualnode.hh:128)

Probe `gecode_gist_visualnode.hh-128-VisualNodeFlags` on macOS: 12 errors in 12
of 12 dependent TUs.

- implicit conversion (12): `gecode/gist/visualnode.hpp:133`,
  `gecode/gist/visualnode.hpp:138`, `gecode/gist/visualnode.hpp:161`,
  `gecode/gist/visualnode.hpp:166`, `gecode/gist/visualnode.hpp:171`,
  `gecode/gist/visualnode.hpp:176`, `gecode/gist/visualnode.hpp:181`,
  `gecode/gist/visualnode.hpp:186`, `gecode/gist/visualnode.hpp:191`,
  `gecode/gist/visualnode.hpp:196`, `gecode/gist/visualnode.hpp:201`,
  `gecode/gist/visualnode.hpp:206`

### Gecode::ReifyMode (gecode/int.hh:1140)

Probe `gecode_int.hh-1140-ReifyMode` on macOS: 27 errors in 64 of 440 dependent
TUs.

- bit position (n << e) (27): `test/float.hpp` 9, `test/int.hpp` 15,
  `test/int/mm-lin.cpp` 3

### Gecode::IntPropLevel (gecode/int.hh:1267)

Probe `gecode_int.hh-1267-IntPropLevel` on macOS: 42 errors in 440 of 440
dependent TUs.

- bitwise & (6): `gecode/driver/options.cpp:383`,
  `gecode/driver/options.cpp:386`, `gecode/driver/options.cpp:388`,
  `gecode/int/ipl.hpp:42`, `test/int.hpp:211`, `test/int.hpp:213`
- bitwise | (4): `gecode/int/ipl.hpp:37`, `gecode/int/sequence/open.hpp:267`,
  `test/flatzinc/native-registry.cpp:53`, `test/int/inter-distance.cpp:393`
- bitwise |= (6): `gecode/driver/options.cpp:339`,
  `gecode/driver/options.cpp:341`, `gecode/driver/options.cpp:343`,
  `gecode/flatzinc/flatzinc.cpp:2617`, `gecode/flatzinc/flatzinc.cpp:2620`,
  `gecode/flatzinc/flatzinc.cpp:2622`
- implicit conversion (10): `gecode/driver/options.cpp:322`,
  `gecode/driver/options.cpp:323`, `gecode/driver/options.cpp:331`,
  `gecode/driver/options.cpp:333`, `gecode/driver/options.cpp:335`,
  `gecode/driver/options.cpp:337`, `gecode/flatzinc/flatzinc.cpp:2607`,
  `gecode/flatzinc/flatzinc.cpp:2610`, `gecode/flatzinc/flatzinc.cpp:2612`,
  `gecode/flatzinc/flatzinc.cpp:2615`
- integral constant expression (bit-field width, array bound, template argument)
  (16): `gecode/minimodel.hh` 16

### Gecode::Int::Arithmetic::MaxPropStatus (gecode/int/arithmetic/max.hpp:176)

Probe `gecode_int_arithmetic_max.hpp-176-MaxPropStatus` on macOS: 7 errors in 2
of 2 dependent TUs.

- bitwise & (3): `gecode/int/arithmetic/max.hpp:212`,
  `gecode/int/arithmetic/max.hpp:214`, `gecode/int/arithmetic/max.hpp:216`
- bitwise |= (2): `gecode/int/arithmetic/max.hpp:203`,
  `gecode/int/arithmetic/max.hpp:206`
- implicit conversion (1): `gecode/int/arithmetic/max.hpp:197`
- unary operator (1): `gecode/int/arithmetic/max.hpp:208`

### Gecode::Int::Extensional::CompactStatus<Advisor>::StatusType (gecode/int/extensional.hh:477)

Probe `gecode_int_extensional.hh-477-StatusType` on macOS: 13 errors in 2 of 2
dependent TUs.

- mixed comparison == (1): `gecode/int/extensional/compact.hpp:158`
- other: no member named 'MULTIPLE' in
  'Gecode::Int::Extensional::CompactStatus<Gecode::Int::Extensional::Com (2):
  `gecode/int/extensional/compact.hpp:962`,
  `gecode/int/extensional/compact.hpp:1524`
- other: no member named 'NONE' in
  'Gecode::Int::Extensional::CompactStatus<Gecode::Int::Extensional::Compact
  (2): `gecode/int/extensional/compact.hpp:949`,
  `gecode/int/extensional/compact.hpp:1521`
- other: use of undeclared identifier 'MULTIPLE' (1):
  `gecode/int/extensional/compact.hpp:162`
- other: use of undeclared identifier 'NONE'; did you mean 'NGL::NONE'? (2):
  `gecode/int/extensional/compact.hpp:158`,
  `gecode/int/extensional/compact.hpp:169`
- other: use of undeclared identifier 'PROPAGATING' (1):
  `gecode/int/extensional/compact.hpp:175`
- other: use of undeclared identifier 'SINGLE' (3):
  `gecode/int/extensional/compact.hpp:152`,
  `gecode/int/extensional/compact.hpp:160`,
  `gecode/int/extensional/compact.hpp:161`
- using enum declaration (1): `gecode/int/extensional.hh:482`

All errors come from `using enum` on a dependent type. With per-enumerator
`static constexpr` aliases (`StatusType-alias`) 4 remain, conversions into
`Support::uptrint_t s` (implicit conversion):
`gecode/int/extensional/compact.hpp:141`, `:160`, `:169`, `:175`. With those and
`:162` converted (`statustype-alias.patch`), `StatusType-alias-fixed` reports 0
errors in 2 TUs.

### Gecode::Int::GCC::Edge::EdgeFlag (gecode/int/gcc/dom-sup.hpp:281)

Probe `gecode_int_gcc_dom-sup.hpp-281-EdgeFlag` on macOS: 18 errors in 1 of 1
dependent TUs.

- bitwise & (5): `gecode/int/gcc/dom-sup.hpp:847`,
  `gecode/int/gcc/dom-sup.hpp:849`, `gecode/int/gcc/dom-sup.hpp:954`,
  `gecode/int/gcc/dom-sup.hpp:956`, `gecode/int/gcc/dom-sup.hpp:971`
- bitwise |= (5): `gecode/int/gcc/dom-sup.hpp:833`,
  `gecode/int/gcc/dom-sup.hpp:835`, `gecode/int/gcc/dom-sup.hpp:943`,
  `gecode/int/gcc/dom-sup.hpp:945`, `gecode/int/gcc/dom-sup.hpp:961`
- implicit conversion (1): `gecode/int/gcc/dom-sup.hpp:828`
- unary operator (7): `gecode/int/gcc/dom-sup.hpp:840`,
  `gecode/int/gcc/dom-sup.hpp:842`, `gecode/int/gcc/dom-sup.hpp:915`,
  `gecode/int/gcc/dom-sup.hpp:917`, `gecode/int/gcc/dom-sup.hpp:925`,
  `gecode/int/gcc/dom-sup.hpp:927`, `gecode/int/gcc/dom-sup.hpp:966`

### Gecode::Int::GCC::Node::NodeFlag (gecode/int/gcc/dom-sup.hpp:64)

Probe `gecode_int_gcc_dom-sup.hpp-64-NodeFlag` on macOS: 7 errors in 1 of 1
dependent TUs.

- bitwise & (3): `gecode/int/gcc/dom-sup.hpp:529`,
  `gecode/int/gcc/dom-sup.hpp:569`, `gecode/int/gcc/dom-sup.hpp:571`
- bitwise |= (2): `gecode/int/gcc/dom-sup.hpp:577`,
  `gecode/int/gcc/dom-sup.hpp:579`
- unary operator (2): `gecode/int/gcc/dom-sup.hpp:593`,
  `gecode/int/gcc/dom-sup.hpp:596`

### Gecode::Int::Linear::BinMod (gecode/int/linear/int-bin.hpp:165)

Probe `gecode_int_linear_int-bin.hpp-165-BinMod` on macOS: 5 errors in 9 of 9
dependent TUs.

- binary operator inside a macro expansion (4):
  `gecode/int/linear/int-bin.hpp:188`, `gecode/int/linear/int-bin.hpp:189`,
  `gecode/int/linear/int-bin.hpp:190`, `gecode/int/linear/int-bin.hpp:191`
- implicit conversion (1): `gecode/int/linear/int-bin.hpp:186`

### Gecode::Int::Linear::TerMod (gecode/int/linear/int-ter.hpp:124)

Probe `gecode_int_linear_int-ter.hpp-124-TerMod` on macOS: 13 errors in 9 of 9
dependent TUs.

- binary operator inside a macro expansion (6):
  `gecode/int/linear/int-ter.hpp:149`, `gecode/int/linear/int-ter.hpp:150`,
  `gecode/int/linear/int-ter.hpp:151`, `gecode/int/linear/int-ter.hpp:152`,
  `gecode/int/linear/int-ter.hpp:153`, `gecode/int/linear/int-ter.hpp:154`
- bitwise | (6): `gecode/int/linear/int-ter.hpp:149`,
  `gecode/int/linear/int-ter.hpp:150`, `gecode/int/linear/int-ter.hpp:151`,
  `gecode/int/linear/int-ter.hpp:152`, `gecode/int/linear/int-ter.hpp:153`,
  `gecode/int/linear/int-ter.hpp:154`
- implicit conversion (1): `gecode/int/linear/int-ter.hpp:147`

### Gecode::Int::Event::Type (gecode/int/task.hh:497)

Probe `gecode_int_task.hh-497-Type` on macOS: 1 error in 2 of 2 dependent TUs.

- bitwise | (1): `gecode/int/task/event.hpp:37`

### Gecode::ExecStatus (gecode/kernel/core.hpp:482)

Probe `gecode_kernel_core.hpp-482-ExecStatus` on macOS: 1 error in 480 of 480
dependent TUs.

- implicit conversion (1): `gecode/kernel/branch/val-commit.hpp:130`

### Gecode::PropCost::ActualCost (gecode/kernel/core.hpp:501)

Probe `gecode_kernel_core.hpp-501-ActualCost` on macOS: 3705 errors in 480 of
480 dependent TUs. All but 7 are a cascade: the bound of `ActorLink
queue[PropCost::AC_MAX + 1]` stops compiling, so `Space` is invalid and every
derived space, tracer access, and cast fails after it. The 7 sites are listed
below; `actualcost-step1.patch` fixes them and the re-probe (`ActualCost-step1`)
finds 2 more, which `actualcost-step2.patch` fixes (`ActualCost-step2`: 0 errors
in 480 TUs).

- arithmetic + (5): `gecode/kernel/core.cpp:274`, `gecode/kernel/core.hpp:1908`,
  `gecode/kernel/core.hpp:4261`, `gecode/kernel/core.hpp:4275`,
  `gecode/kernel/core.hpp:4284`
- comparison with integer <= (2): `gecode/kernel/core.cpp:129`,
  `gecode/kernel/core.cpp:880`
- array index, re-probe (2): `gecode/kernel/core.cpp:539`,
  `gecode/kernel/core.hpp:4253`

### Gecode::ActorProperty (gecode/kernel/core.hpp:566)

Probe `gecode_kernel_core.hpp-566-ActorProperty` on macOS: 8 errors in 480 of
480 dependent TUs.

- bitwise & (8): `gecode/kernel/core.hpp:4289`, `gecode/kernel/core.hpp:4292`,
  `gecode/kernel/core.hpp:4295`, `gecode/kernel/core.hpp:4299`,
  `gecode/kernel/core.hpp:4308`, `gecode/kernel/core.hpp:4310`,
  `gecode/kernel/core.hpp:4313`, `gecode/kernel/core.hpp:4317`

### Gecode::ViewTraceInfo::What (gecode/kernel/core.hpp:930)

Probe `gecode_kernel_core.hpp-930-What` on macOS: 15 errors in 480 of 480
dependent TUs.

- bit position (n << e) (11): `gecode/kernel/trace/filter.cpp:73`,
  `gecode/kernel/trace/filter.cpp:73`, `gecode/kernel/trace/filter.cpp:77`,
  `gecode/kernel/trace/filter.cpp:83`, `gecode/kernel/trace/filter.cpp:132`,
  `gecode/kernel/trace/filter.cpp:139`, `gecode/kernel/trace/filter.hpp:263`,
  `gecode/kernel/trace/filter.hpp:271`, `gecode/kernel/trace/filter.hpp:278`,
  `gecode/kernel/trace/filter.hpp:312`, `gecode/kernel/trace/filter.hpp:327`
- bitwise | (3): `gecode/kernel/core.hpp:3514`, `gecode/kernel/core.hpp:3518`,
  `gecode/kernel/core.hpp:3522`
- implicit conversion (1): `gecode/kernel/core.hpp:3526`

### Gecode::TraceEvent (gecode/kernel/trace/recorder.hpp:39)

Probe `gecode_kernel_trace_recorder.hpp-39-TraceEvent` on macOS: 50 errors in
480 of 480 dependent TUs.

- arithmetic -= (7): `gecode/driver/options.cpp:469`,
  `gecode/driver/options.cpp:475`, `gecode/driver/options.cpp:481`,
  `gecode/driver/options.cpp:487`, `gecode/driver/options.cpp:493`,
  `gecode/driver/options.cpp:499`, `gecode/driver/options.cpp:505`
- binary operator inside a macro expansion (5): `gecode/kernel/core.cpp:433`,
  `gecode/kernel/core.cpp:438`, `gecode/kernel/core.cpp:455`,
  `gecode/kernel/core.cpp:476`, `gecode/kernel/core.cpp:481`
- bitwise & (12): `gecode/driver/options.cpp:467`,
  `gecode/driver/options.cpp:473`, `gecode/driver/options.cpp:479`,
  `gecode/driver/options.cpp:485`, `gecode/driver/options.cpp:491`,
  `gecode/driver/options.cpp:497`, `gecode/driver/options.cpp:503`,
  `gecode/driver/options.cpp:509`, `gecode/kernel/core.cpp:254`,
  `gecode/kernel/core.cpp:615`, `gecode/kernel/core.cpp:645`,
  `gecode/kernel/trace/recorder.hpp:275`
- bitwise | (18): `gecode/driver/options.cpp` 6, `gecode/float.hh` 2,
  `gecode/int.hh` 4, `gecode/kernel.hh` 2, `gecode/kernel/trace/recorder.hpp` 2,
  `gecode/set.hh` 2
- bitwise |= (8): `gecode/driver/options.cpp:411`,
  `gecode/driver/options.cpp:413`, `gecode/driver/options.cpp:415`,
  `gecode/driver/options.cpp:417`, `gecode/driver/options.cpp:419`,
  `gecode/driver/options.cpp:421`, `gecode/driver/options.cpp:423`,
  `gecode/driver/options.cpp:425`

### Gecode::SearchTracer::EngineType (gecode/search.hh:197)

Probe `gecode_search.hh-197-EngineType` on macOS: 2 errors in 461 of 461
dependent TUs.

- arithmetic + (2): `gecode/search.hh:381`, `gecode/search/tracer.cpp:37`
- array index, re-probe `EngineType-fixed` (1): `gecode/search/tracer.cpp:45`;
  `EngineType-step2`: 0 errors in 461 TUs

### Gecode::Search::Par::Engine<Tracer>::Cmd (gecode/search/par/engine.hh:163)

Probe `gecode_search_par_engine.hh-163-Cmd` on macOS: 17 errors in 2 of 2
dependent TUs.

- other: no member named 'C_RESET' in
  'Gecode::Search::Par::Engine<Gecode::Search::EdgeTraceRecorder>' (2):
  `gecode/search/par/bab.hh:61`, `gecode/search/par/dfs.hh:61`
- other: no member named 'C_TERMINATE' in
  'Gecode::Search::Par::Engine<Gecode::Search::EdgeTraceRecorder>' (2):
  `gecode/search/par/bab.hh:62`, `gecode/search/par/dfs.hh:62`
- other: no member named 'C_WAIT' in
  'Gecode::Search::Par::Engine<Gecode::Search::EdgeTraceRecorder>' (2):
  `gecode/search/par/bab.hh:60`, `gecode/search/par/dfs.hh:60`
- other: no member named 'C_WORK' in
  'Gecode::Search::Par::Engine<Gecode::Search::EdgeTraceRecorder>' (2):
  `gecode/search/par/bab.hh:63`, `gecode/search/par/dfs.hh:63`
- other: use of undeclared identifier 'C_RESET' (1):
  `gecode/search/par/engine.hpp:81`
- other: use of undeclared identifier 'C_TERMINATE' (2):
  `gecode/search/par/engine.hpp:81`, `gecode/search/par/engine.hpp:421`
- other: use of undeclared identifier 'C_WAIT' (2):
  `gecode/search/par/engine.hpp:74`, `gecode/search/par/engine.hpp:119`
- other: use of undeclared identifier 'C_WORK' (3):
  `gecode/search/par/engine.hpp:81`, `gecode/search/par/engine.hpp:218`,
  `gecode/search/par/engine.hpp:513`
- using enum declaration (1): `gecode/search/par/engine.hh:168`

All errors come from `using enum` on a dependent type. With per-enumerator
`static constexpr` aliases (`Cmd-alias`) the probe reports 0 errors in 2 TUs.

### Gecode::SetRelType (gecode/set.hh:630)

Probe `gecode_set.hh-630-SetRelType` on macOS: 2 errors in 1 of 328 dependent
TUs.

- implicit conversion (2): `test/set/mm-set.cpp:151`, `test/set/mm-set.cpp:278`

### Test::Float::MaybeType (test/float.hh:63)

Probe `test_float.hh-63-MaybeType` on erebor: 2 errors in 1 of 12 dependent TUs.

- arithmetic && (1): `test/float.cpp:722`
- conversion to bool (1): `test/float.cpp:722`

## Appendix: scripts

The scripts as run, from `$S/scripts`. `probe.py` is the probe; the others
derive the lists and counts above.

### probe.py

```python
#!/usr/bin/env python3
"""Scope unscoped enums in the scratch tree, recompile dependents, and record errors.

Usage: probe.py <scratch-dir> <checkout-or-pristine-tree> <label> <path>:<line>[:alias] ...

For each <path>:<line>, `enum E {` on that line becomes `enum class E {`, and
`using enum E;` is inserted after the `;` that ends the declaration. Every
translation unit whose recorded header dependencies (ninja-deps.txt, from
`ninja -t deps` after the baseline build) include a modified file is compiled
with its exact command from compile_commands.json, writing the object to
probe-obj/ instead of the build tree. The log is saved as probes/<label>.log,
the files are restored from HEAD, and the errors, deduplicated by
file:line:col, are written to probes/<label>.json.
"""

import concurrent.futures
import json
import os
import re
import subprocess
import sys
import time

ERROR_RE = re.compile(r"^(.+?):(\d+):(\d+): (?:fatal )?error: (.*)$")


def strip_comments(text):
    """Blank out comments while keeping offsets."""
    def blank(m):
        return re.sub(r"[^\n]", " ", m.group(0))
    return re.sub(r"/\*.*?\*/|//[^\n]*", blank, text, flags=re.S)


def scope_enum(text, line, alias=False):
    """Return text with the enum declared at 1-based line scoped, and its name.

    With alias, the enumerators are re-exported as `static constexpr` members
    instead of `using enum`, which C++ rejects for an enum declared inside a
    class template (a dependent type)."""
    starts = [0] + [m.end() for m in re.finditer("\n", text)]
    code = strip_comments(text)
    m = re.compile(r"\benum\s+(\w+)").search(code, starts[line - 1])
    if m is None or m.start() >= starts[line]:
        raise SystemExit(f"no named enum on line {line}")
    name = m.group(1)
    close_brace = code.index("}", code.index("{", m.end()))
    semicolon = code.index(";", close_brace)
    if alias:
        body = code[code.index("{", m.end()) + 1 : close_brace]
        items = [re.match(r"\s*(\w+)", part).group(1) for part in body.split(",") if part.strip()]
        export = " ".join(f"static constexpr {name} {i} = {name}::{i};" for i in items)
    else:
        export = f"using enum {name};"
    scoped = text[: m.start()] + f"enum class {name}" + text[m.end() : semicolon + 1] + " " + export + text[semicolon + 1 :]
    return scoped, name


def dependents(scratch, paths):
    """Objects whose recorded dependencies include one of paths."""
    wanted = {os.path.join(scratch, "src", p) for p in paths}
    result, current = set(), None
    for raw in open(os.path.join(scratch, "ninja-deps.txt")):
        if not raw.startswith(" "):
            current = raw.split(":", 1)[0]
        elif raw.strip() in wanted:
            result.add(current)
    return result


def compile_one(entry, scratch):
    obj = os.path.join(scratch, "probe-obj", re.sub(r"[^\w.]", "_", entry["output"]))
    command = entry["command"].replace(f" -o {entry['output']} ", f" -o {obj} ")
    done = subprocess.run(command, shell=True, cwd=entry["directory"], capture_output=True, text=True)
    return entry["output"], done.returncode, done.stdout + done.stderr


def normalize(path, scratch):
    for prefix in (os.path.join(scratch, "src") + "/", os.path.join(scratch, "build") + "/"):
        if path.startswith(prefix):
            return path[len(prefix) :]
    return path[2:] if path.startswith("./") else path


def parse_errors(log, scratch):
    errors = {}
    for raw in log.splitlines():
        m = ERROR_RE.match(raw)
        if m:
            key = (normalize(m.group(1), scratch), int(m.group(2)), int(m.group(3)))
            errors.setdefault(key, m.group(4))
    return [{"file": f, "line": l, "col": c, "message": msg} for (f, l, c), msg in sorted(errors.items())]


def main():
    scratch, checkout, label = os.path.abspath(sys.argv[1]), sys.argv[2], sys.argv[3]
    specs = []
    for s in sys.argv[4:]:
        alias = s.endswith(":alias")
        p, l = s.removesuffix(":alias").rsplit(":", 1)
        specs.append((p, int(l), alias))
    src = os.path.join(scratch, "src")
    # PROBE_PATCH names a unified diff (paths a/ and b/) applied before
    # scoping, to measure what remains after proposed fixes.
    patch = os.environ.get("PROBE_PATCH")
    patched = re.findall(r"^\+\+\+ b/(\S+)", open(patch).read(), flags=re.M) if patch else []
    heads = {}
    for path in [s[0] for s in specs] + patched:
        if os.path.exists(os.path.join(checkout, ".git")):
            head = subprocess.run(["git", "-C", checkout, "show", f"HEAD:{path}"], check=True, capture_output=True,
                                  text=True).stdout
        else:  # a pristine extraction of `git archive HEAD` (used on erebor)
            head = open(os.path.join(checkout, path)).read()
        if open(os.path.join(src, path)).read() != head:
            raise SystemExit(f"{path} differs from HEAD before probing")
        heads[path] = head
    if patch:
        subprocess.run(["patch", "-p1", "-s", "-d", src, "-i", os.path.abspath(patch)], check=True)
    names = []
    # Apply bottom-up within a file so that line numbers stay valid.
    for path, line, alias in sorted(specs, key=lambda s: (s[0], -s[1])):
        text, name = scope_enum(open(os.path.join(src, path)).read(), line, alias)
        names.append(f"{path}:{line}:{name}")
        with open(os.path.join(src, path), "w") as f:
            f.write(text)
    entries = []
    for e in json.load(open(os.path.join(scratch, "build", "compile_commands.json"))):
        e["output"] = re.search(r" -o (\S+) ", e["command"]).group(1)
        entries.append(e)
    targets = dependents(scratch, heads)
    selected = [e for e in entries if e["output"] in targets]
    os.makedirs(os.path.join(scratch, "probe-obj"), exist_ok=True)
    os.makedirs(os.path.join(scratch, "probes"), exist_ok=True)
    start = time.time()
    try:
        with concurrent.futures.ThreadPoolExecutor(os.cpu_count()) as pool:
            results = sorted(pool.map(lambda e: compile_one(e, scratch), selected))
    finally:
        for path, head in heads.items():
            with open(os.path.join(src, path), "w") as f:
                f.write(head)
    elapsed = time.time() - start
    log = "".join(f"=== {out} rc={rc}\n{text}" for out, rc, text in results)
    with open(os.path.join(scratch, "probes", f"{label}.log"), "w") as f:
        f.write(log)
    errors = parse_errors(log, scratch)
    failed = [out for out, rc, _ in results if rc != 0]
    summary = {"label": label, "enums": names, "patch": patch, "seconds": round(elapsed, 1), "compiled": len(selected),
               "failed_tus": len(failed), "errors": errors}
    with open(os.path.join(scratch, "probes", f"{label}.json"), "w") as f:
        json.dump(summary, f, indent=1)
    print(f"{label}: {len(errors)} errors in {len(failed)}/{len(selected)} TUs, {elapsed:.0f}s")


if __name__ == "__main__":
    main()
```

### run-all.sh

```sh
#!/bin/sh
# Probe every named unscoped enum in probe-list.txt, one at a time. An
# optional third column names a patch, relative to $S, applied before scoping.
S=$1; CHECKOUT=$2
while read spec name patch; do
    label=$(echo "$spec" | sed -E 's|/|_|g; s|:|-|')-$name
    [ -f "$S/probes/$label.json" ] && continue
    PROBE_PATCH=${patch:+$S/$patch} python3 -I "$S/scripts/probe.py" "$S" "$CHECKOUT" "$label" "$spec"
done < "$S/probe-list.txt"
```

### bucket.py

```python
#!/usr/bin/env python3
"""Bucket every probe's deduplicated errors by the operation that scoping blocks.

Usage: bucket.py <scratch-dir>   (reads probes/*.json, writes buckets.json and
prints one summary line per probe)
"""
import glob
import json
import os
import re
import sys

S = sys.argv[1]
TYPES = re.compile(r"'([^']+)'")


def operator_at(e):
    """The operator token at the error column of the HEAD source line."""
    try:
        line = open(os.path.join(S, "src", e["file"])).read().split("\n")[e["line"] - 1]
    except (OSError, IndexError):
        return "?"
    m = re.match(r"(<<=|>>=|<<|>>|<=>|<=|>=|==|!=|&&|\|\||[-+*/%&|^]=|[-+*/%&|^<>])", line[e["col"] - 1 :])
    return m.group(1) if m else "?"


def bucket(e, enum_name):
    msg = e["message"]
    if "array subscript is not an integer" in msg:
        return "array index"
    if msg.startswith("invalid operands to binary expression"):
        op = operator_at(e)
        types = TYPES.findall(msg)
        if any("ostream" in t or "basic_ostream" in t for t in types):
            return "stream <<"
        enum_types = [t for t in types if enum_name in t]
        other = [t for t in types if enum_name not in t]
        if op in ("<", ">", "<=", ">=", "==", "!=") and other and not re.search(r"\b(int|unsigned|long|char|size_t|double)\b", other[0]):
            return f"mixed comparison {op}"
        if op in ("<", ">", "<=", ">=", "==", "!="):
            return f"comparison with integer {op}"
        if op in ("<<", ">>") and len(types) == 2 and enum_name in types[1] and enum_name not in types[0]:
            return "bit position (n << e)"
        if op in ("&", "|", "^", "&=", "|=", "^=", "<<", ">>"):
            return f"bitwise {op}"
        if op == "?":
            return "binary operator inside a macro expansion"
        return f"arithmetic {op}"
    if re.search(r"cannot (increment|decrement)", msg):
        return "increment/decrement"
    if msg.startswith("invalid argument type") and "unary expression" in msg:
        return "unary operator"
    if "integral constant expression must have integral or unscoped enumeration type" in msg:
        return "integral constant expression (bit-field width, array bound, template argument)"
    if re.search(r"cannot initialize|assigning to|no viable conversion|cannot convert|not implicitly convertible|"
                 r"no matching (function|constructor|member function|conversion)|non-type template argument", msg):
        return "implicit conversion"
    if "not contextually convertible to 'bool'" in msg:
        return "conversion to bool"
    if "using enum" in msg or "dependent" in msg:
        return "using enum declaration"
    return "other: " + msg[:100]


def main():
    out = {}
    for path in sorted(glob.glob(os.path.join(S, "probes", "gecode_*.json")) + sorted(glob.glob(os.path.join(S, "probes", "test_*.json")))):
        d = json.load(open(path))
        enum_name = d["enums"][0].rsplit(":", 1)[1]
        rows = []
        for e in d["errors"]:
            rows.append(dict(e, bucket=bucket(e, enum_name)))
        out[d["label"]] = {"enums": d["enums"], "compiled": d["compiled"], "failed_tus": d["failed_tus"],
                           "seconds": d["seconds"], "errors": rows}
        counts = {}
        for r in rows:
            counts[r["bucket"]] = counts.get(r["bucket"], 0) + 1
        print(f"{d['label']}: {len(rows)} in {d['failed_tus']}/{d['compiled']} TUs; "
              + "; ".join(f"{k} {v}" for k, v in sorted(counts.items())))
    json.dump(out, open(os.path.join(S, "buckets.json"), "w"), indent=1)


if __name__ == "__main__":
    main()
```

### sites.py

```python
#!/usr/bin/env python3
"""Render the call-site appendix of the enum inventory from buckets.json.

Usage: sites.py <scratch-dir>  (writes sites.md)

Per enum: the deduplicated errors grouped by bucket. A bucket with 15 or
fewer sites lists each as file:line; larger buckets list per-file counts.
"""
import json
import os
import sys
from collections import Counter, defaultdict

S = sys.argv[1]
buckets = json.load(open(os.path.join(S, "buckets.json")))
contexts = {}
for line in open(os.path.join(S, "enum-contexts.txt")):
    loc, name = line.split()
    contexts[loc] = name
hosts = {}
for label in buckets:
    erebor = os.path.join(S, "probes-erebor", label + ".json")
    local = os.path.join(S, "probes", label + ".json")
    hosts[label] = "erebor" if os.path.exists(erebor) and open(erebor).read() == open(local).read() else "macOS"
out = []
for label, d in buckets.items():
    path, line, _ = d["enums"][0].rsplit(":", 2)
    qual = contexts.get(f"{path}:{line}", label)
    if not d["errors"]:
        continue
    out.append(f"### {qual} ({path}:{line})\n")
    out.append(f"Probe `{label}` on {hosts[label]}: {len(d['errors'])} error{'s' * (len(d['errors']) != 1)} in {d['failed_tus']} of "
               f"{d['compiled']} dependent TUs.\n")
    by_bucket = defaultdict(list)
    for e in d["errors"]:
        by_bucket[e["bucket"]].append(e)
    for b, rows in sorted(by_bucket.items()):
        if len(rows) <= 15:
            sites = ", ".join(f"`{r['file']}:{r['line']}`" for r in rows)
        else:
            per_file = Counter(r["file"] for r in rows)
            sites = ", ".join(f"`{f}` {n}" for f, n in sorted(per_file.items()))
        out.append(f"- {b} ({len(rows)}): {sites}")
    out.append("")
open(os.path.join(S, "sites.md"), "w").write("\n".join(out) + "\n")
print(f"{sum(1 for d in buckets.values() if d['errors'])} enums with errors")
```

### enum_bodies.py

```python
import re, sys
# Print each grep-found enum declaration with its enumerators (comments stripped).
def strip(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)
for line in open(sys.argv[1]):
    path, lno, _ = line.split(":", 2)
    lines = open(path).read().split("\n")
    i = int(lno) - 1
    text = ""
    while "}" not in strip(text) and i < len(lines):
        text += lines[i] + "\n"; i += 1
    body = strip(text)
    body = " ".join(body[: body.index("}") + 1].split())
    print(f"{path}:{lno}: {body}")
```

### enumerators.py

```python
#!/usr/bin/env python3
"""Write enums.json: every grep-found enum with its enumerators and values."""
import json, re, sys
S = sys.argv[1]
out = []
for line in open(f"{S}/enum-bodies.txt"):
    loc, body = line.split(": ", 1)
    path, lno = loc.rsplit(":", 1)
    m = re.search(r"\benum\s*(\w*)\s*\{(.*?)\}", body)
    if not m:
        continue
    items = []
    for part in m.group(2).split(","):
        part = part.strip()
        if part:
            name, _, value = part.partition("=")
            items.append([name.strip(), value.strip()])
    out.append({"file": path, "line": int(lno), "name": m.group(1), "enumerators": items})
json.dump(out, open(f"{S}/enums.json", "w"), indent=1)
print(len(out), sum(len(e["enumerators"]) for e in out))
```

### context.py

```python
#!/usr/bin/env python3
"""Print the enclosing namespace/class chain of each enum in enums.json (brace scan)."""
import json, re, sys
S = sys.argv[1]
def strip(t):
    t = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)), t, flags=re.S)
    t = re.sub(r"//[^\n]*", lambda m: " " * len(m.group(0)), t)
    return re.sub(r'"(?:\\.|[^"\\\n])*"', lambda m: " " * len(m.group(0)), t)
for e in json.load(open(f"{S}/enums.json")):
    text = strip(open(e["file"]).read())
    end = sum(len(l) + 1 for l in text.split("\n")[: e["line"] - 1])
    stack = []
    last = 0
    for m in re.finditer(r"[{}]", text[:end]):
        if m.group(0) == "{":
            head = text[last:m.start()]
            k = re.search(r"\b(namespace|class|struct|union)\s+([\w:]*)[^;{}()]*$", head)
            stack.append(f"{k.group(2) or '(anon)'}" if k else None)
        else:
            if stack: stack.pop()
        last = m.end()
    chain = "::".join(s for s in stack if s)
    print(f"{e['file']}:{e['line']} {chain}::{e['name'] or '<anon>'}")
```

### ast_enums.sh

```sh
#!/bin/sh
# AST discovery: every unscoped enum declared under src/gecode or src/test, from
# every TU in the compile database, deduplicated by file:line.
S=$1
SDK=$(xcrun --show-sdk-path)
python3 -I -c "import json,sys; [print(e['file']) for e in json.load(open(sys.argv[1]))]" "$S/build/compile_commands.json" |
  xargs -P "$(sysctl -n hw.ncpu)" -n 8 /opt/homebrew/opt/llvm/bin/clang-query -p "$S/build" \
    --extra-arg=-isysroot --extra-arg="$SDK" -c 'set output diag' \
    -c 'match enumDecl(unless(isScoped()), isExpansionInFileMatching("scoped-enums-001/(src|build)/(gecode|test)/"))' 2>&1 |
  tee "$S/clang-query.log" | grep -E '"root" binds here' | sed -E "s#^$S/(src|build)/##; s#:[0-9]+: note.*##" | sort -u > "$S/ast-enums.txt"
grep -cE '(^|[^a-z])error:' "$S/clang-query.log"
wc -l < "$S/ast-enums.txt"
```

### reconcile.py

```python
#!/usr/bin/env python3
"""Compare the clang-query enum set with the grep set over the installed headers.

Usage: reconcile.py <scratch-dir>
"""
import sys

S = sys.argv[1]
installed = {l.strip()[2:] for l in open(f"{S}/installed-headers.txt")}
ast = set()
for line in open(f"{S}/ast-enums.txt"):
    path, lno = line.strip().rsplit(":", 1)
    if path in installed:
        ast.add(f"{path}:{lno}")
grep = set()
for line in open(f"{S}/grep-enums.txt"):
    path, lno, _ = line.split(":", 2)
    grep.add(f"{path}:{lno}")
print("ast (installed):", len(ast), "grep lines:", len(grep), "both:", len(ast & grep))
print("grep only:", *sorted(grep - ast), sep="\n  ")
print("ast only:", *sorted(ast - grep), sep="\n  ")
```

### off_regions.py

```python
#!/usr/bin/env python3
"""Print file:line:text for lines that the probe configuration preprocesses away.

unifdef -l blanks removed lines, so line numbers stay aligned. Only symbols
whose value in the probe configuration is certain are given; conditions on
other symbols keep both branches.
"""
import subprocess

DEFINED = ["GECODE_HAS_INT_VARS", "GECODE_HAS_SET_VARS", "GECODE_HAS_FLOAT_VARS", "GECODE_HAS_THREADS",
           "GECODE_HAS_CBS", "GECODE_HAS_EXPERIMENTAL", "GECODE_HAS_MPFR", "GECODE_HAS_GIST", "GECODE_HAS_QT",
           "GECODE_HAS_CPPROFILER", "GECODE_AUDIT", "GECODE_GCC_HAS_CLASS_VISIBILITY", "GECODE_USE_OSX_UNFAIR_MUTEX",
           "GECODE_HAS_POSIX_BLACKBOX_EXEC", "HAVE_MMAP", "__GNUC__=4", "__APPLE__"]
UNDEFINED = ["_WIN32", "_MSC_VER", "__MINGW32__", "__CYGWIN__", "NDEBUG", "GECODE_THREADS_WINDOWS",
             "GECODE_HAS_FAULT_INJECTION", "GECODE_STATIC_LIBS", "GECODE_PEAKHEAP", "GECODE_MIMALLOC",
             "GECODE_HAS_MTRACE", "__ICC", "__GLIBC__", "_M_IX86", "_M_X64", "GECODE_MEMORY_CHECK",
             "GECODE_FLOAT_MODE_CONTROL87", "GIST_STATIC_LIBS"]
args = ["unifdef", "-l"] + [f"-D{s}" for s in DEFINED] + [f"-U{s}" for s in UNDEFINED]
files = subprocess.run(["git", "ls-files", "gecode", "test", "examples", "tools"], capture_output=True,
                       text=True).stdout.split()
for f in files:
    if not f.endswith((".hh", ".hpp", ".cpp", ".yxx", ".lxx")):
        continue
    before = open(f, errors="replace").read().split("\n")
    after = subprocess.run(args + [f], capture_output=True, text=True, errors="replace").stdout.split("\n")
    for n, (a, b) in enumerate(zip(before, after), 1):
        if a != b and not a.lstrip().startswith("#"):
            print(f"{f}:{n}:{a}")
```

### off_sites.py

```python
#!/usr/bin/env python3
"""Uses of each enum that the probe configuration does not compile.

Reads off-region-lines.txt (lines removed by unifdef under the probe
configuration) and uncompiled-files.txt (tracked sources no TU includes), and
prints every such line that names the enum type or one of its enumerators
next to an operator or cast that scoping would block.
"""
import json, re, sys
S = sys.argv[1]
OPS = r"(\||&|\+|-|\[|<<|>>|\(int\)|static_cast<|switch\s*\(|=)"
enums = [e for e in json.load(open(f"{S}/enums.json")) if e["name"] and e["name"] != "yytokentype"]
lines = [l.rstrip("\n").split(":", 2) for l in open(f"{S}/off-region-lines.txt")]
for f in open(f"{S}/uncompiled-files.txt").read().split():
    for n, text in enumerate(open(f, errors="replace").read().split("\n"), 1):
        lines.append([f, str(n), text])
for e in enums:
    names = [n for n, _ in e["enumerators"]] + [e["name"]]
    rx = re.compile(r"\b(" + "|".join(map(re.escape, names)) + r")\b")
    for f, n, text in lines:
        code = re.sub(r"//.*", "", text)
        if rx.search(code) and re.search(OPS, code):
            print(f"{e['file']}:{e['line']} {e['name']}: {f}:{n}: {code.strip()[:110]}")
```

### casts.py

```python
#!/usr/bin/env python3
"""Count explicit casts to or from each named enum (they stay legal when scoped).

Counts static_cast<E>(...), and static_cast<int|unsigned int|...>(x) where x
names an enumerator of E, over gecode/, test/, examples/, tools/.
"""
import json, re, subprocess, sys
from collections import defaultdict
S = sys.argv[1]
enums = [e for e in json.load(open(f"{S}/enums.json")) if e["name"] and e["name"] != "yytokentype"]
files = [f for f in subprocess.run(["git", "ls-files", "gecode", "test", "examples", "tools"], capture_output=True,
                                   text=True).stdout.split() if f.endswith((".hh", ".hpp", ".cpp", ".yxx", ".lxx"))
         and not f.endswith(("parser.tab.cpp", "lexer.yy.cpp"))]
texts = {f: open(f, errors="replace").read() for f in files}
names = defaultdict(list)
for e in enums:
    names[e["name"]].append(e)
for e in enums:
    to_enum = re.compile(r"static_cast<\s*(?:[\w:]+::)?" + e["name"] + r"\s*>")
    enumerators = "|".join(re.escape(n) for n, _ in e["enumerators"])
    from_enum = re.compile(r"static_cast<\s*(?:unsigned\s+)?(?:int|long|char|size_t|std::size_t|unsigned)\s*>\(\s*(?:[\w:]+::)?(?:" + enumerators + r")\b")
    to_n = sum(len(to_enum.findall(t)) for t in texts.values())
    from_n = sum(len(from_enum.findall(t)) for t in texts.values())
    shared = " (name shared by %d enums)" % len(names[e["name"]]) if len(names[e["name"]]) > 1 else ""
    print(f"{e['file']}:{e['line']} {e['name']}: static_cast<{e['name']}> {to_n}{shared}; static_cast<int-like>(enumerator) {from_n}")
```

### rename_candidates.py

```python
#!/usr/bin/env python3
"""Flag enumerators whose words after the prefix are not plain words or settled abbreviations.

A word is accepted if it is in the settled list below or is an English word
of four or more letters from /usr/share/dict/words. Everything else is
printed for review, with a use count over gecode/, test/, examples/, tools/.
"""
import json, re, subprocess, sys
from collections import Counter
S = sys.argv[1]
SETTLED = set("EQ NQ LQ LT GQ GR VAL BND DOM DEF MIN MAX MED RND AFC CHB INC EXC SEL IRT FRT SRT BOT IPL ES ME PC "
              "AND OR XOR NOT ADD SUB MUL SUP ALL NONE YES NO TRUE FALSE MAYBE INT BOOL SET FLOAT VAR VARS LO HI "
              "UNION MINUS ID OK FIX NOFIX TT SM RM AP AC TE RR SS NT ST RT BT CS BSS IT MT CTL MPS BM TM X0 X1 X2 "
              "SOT RET STO EF NF C".split())
words = {w.strip().upper() for w in open("/usr/share/dict/words") if len(w.strip()) >= 4}
files = [f for f in subprocess.run(["git", "ls-files", "gecode", "test", "examples", "tools"], capture_output=True,
                                   text=True).stdout.split() if f.endswith((".hh", ".hpp", ".cpp", ".yxx", ".lxx"))
         and not f.endswith(("parser.tab.cpp", "lexer.yy.cpp", "parser.tab.hpp"))]
uses = Counter()
for f in files:
    uses.update(re.findall(r"\b[A-Z][A-Z0-9_]+\b", open(f, errors="replace").read()))
for e in json.load(open(f"{S}/enums.json")):
    if e["name"] == "yytokentype":
        continue
    for name, _ in e["enumerators"]:
        parts = [p for p in name.split("_") if p]
        odd = [p for p in parts if p not in SETTLED and p not in words]
        if odd or name.endswith("_"):
            print(f"{e['file']}:{e['line']} {e['name'] or '<anon>'} {name}: {' '.join(odd) or 'trailing _'} (uses {uses[name] - 1})")
```

## Appendix: patches

The patches as applied with `PROBE_PATCH`, from `$S/patches`. Save each as
`$S/patches/<name>.patch`; each applies to HEAD with `patch -p1`.

### execstatus.patch

```diff
--- a/gecode/kernel/branch/val-commit.hpp	2026-11-04 18:17:10
+++ b/gecode/kernel/branch/val-commit.hpp	2026-11-04 18:17:10
@@ -127,7 +127,7 @@
         typename View::VarType y(x.varimp());
         GECODE_VALID_FUNCTION(c());
         c()(home, a, y, i, n);
-        return home.failed() ? ES_FAILED : ES_OK;
+        return home.failed() ? ME_GEN_FAILED : ME_GEN_NONE;
     }
     template <class View>
     forceinline NGL*
```

### intproplevel.patch

```diff
--- a/gecode/int.hh	2026-11-04 18:17:10
+++ b/gecode/int.hh	2026-11-04 18:17:10
@@ -1277,6 +1277,22 @@
         IPL_FULL = IPL_BASIC_ADVANCED,                 ///< Use full propagation
         IPL_BITS_ = 4                                  ///< Number of bits required (internal)
     };
+
+    /// Combine propagation levels \a a and \a b
+    constexpr IntPropLevel
+    operator|(IntPropLevel a, IntPropLevel b) noexcept {
+        return static_cast<IntPropLevel>(static_cast<int>(a) | static_cast<int>(b));
+    }
+    /// Intersect propagation levels \a a and \a b
+    constexpr IntPropLevel
+    operator&(IntPropLevel a, IntPropLevel b) noexcept {
+        return static_cast<IntPropLevel>(static_cast<int>(a) & static_cast<int>(b));
+    }
+    /// Add propagation level \a b to \a a
+    constexpr IntPropLevel&
+    operator|=(IntPropLevel& a, IntPropLevel b) noexcept {
+        return a = a | b;
+    }
 
     /// Extract value, bounds, or domain propagation from propagation level
     IntPropLevel vbd(IntPropLevel ipl);
--- a/gecode/minimodel.hh	2026-11-04 18:17:10
+++ b/gecode/minimodel.hh	2026-11-04 18:17:10
@@ -102,22 +102,22 @@
     /// Class for specifying integer propagation levels used by minimodel
     class IntPropLevels {
     protected:
-        IntPropLevel _linear2 : IPL_BITS_; ///< For binary linear
-        IntPropLevel _linear : IPL_BITS_;  ///< For n-ary linear
-        IntPropLevel _abs : IPL_BITS_;     ///< For absolute value
-        IntPropLevel _max2 : IPL_BITS_;    ///< For binary maximum
-        IntPropLevel _max : IPL_BITS_;     ///< For n-ary maximum
-        IntPropLevel _min2 : IPL_BITS_;    ///< For binary minimum
-        IntPropLevel _min : IPL_BITS_;     ///< For minimum
-        IntPropLevel _mult : IPL_BITS_;    ///< For multiplication
-        IntPropLevel _div : IPL_BITS_;     ///< For division
-        IntPropLevel _mod : IPL_BITS_;     ///< For modulo
-        IntPropLevel _sqr : IPL_BITS_;     ///< For square
-        IntPropLevel _sqrt : IPL_BITS_;    ///< For square root
-        IntPropLevel _pow : IPL_BITS_;     ///< For power
-        IntPropLevel _nroot : IPL_BITS_;   ///< For root
-        IntPropLevel _element : IPL_BITS_; ///< For element
-        IntPropLevel _ite : IPL_BITS_;     ///< For if-then-else
+        IntPropLevel _linear2 : static_cast<int>(IPL_BITS_); ///< For binary linear
+        IntPropLevel _linear : static_cast<int>(IPL_BITS_);  ///< For n-ary linear
+        IntPropLevel _abs : static_cast<int>(IPL_BITS_);     ///< For absolute value
+        IntPropLevel _max2 : static_cast<int>(IPL_BITS_);    ///< For binary maximum
+        IntPropLevel _max : static_cast<int>(IPL_BITS_);     ///< For n-ary maximum
+        IntPropLevel _min2 : static_cast<int>(IPL_BITS_);    ///< For binary minimum
+        IntPropLevel _min : static_cast<int>(IPL_BITS_);     ///< For minimum
+        IntPropLevel _mult : static_cast<int>(IPL_BITS_);    ///< For multiplication
+        IntPropLevel _div : static_cast<int>(IPL_BITS_);     ///< For division
+        IntPropLevel _mod : static_cast<int>(IPL_BITS_);     ///< For modulo
+        IntPropLevel _sqr : static_cast<int>(IPL_BITS_);     ///< For square
+        IntPropLevel _sqrt : static_cast<int>(IPL_BITS_);    ///< For square root
+        IntPropLevel _pow : static_cast<int>(IPL_BITS_);     ///< For power
+        IntPropLevel _nroot : static_cast<int>(IPL_BITS_);   ///< For root
+        IntPropLevel _element : static_cast<int>(IPL_BITS_); ///< For element
+        IntPropLevel _ite : static_cast<int>(IPL_BITS_);     ///< For if-then-else
     public:
         /// Initialize with default propagation level
         IntPropLevels(IntPropLevel ipl = IPL_DEF);
--- a/gecode/driver/options.cpp	2026-11-04 18:17:10
+++ b/gecode/driver/options.cpp	2026-11-04 18:17:10
@@ -319,8 +319,8 @@
         int
         IplOption::parse(int argc, char* argv[]) {
             if (char* a = argument(argc, argv)) {
-                int b = IPL_DEF;
-                int m = IPL_DEF;
+                IntPropLevel b = IPL_DEF;
+                IntPropLevel m = IPL_DEF;
                 do {
                     // Search for a comma
                     char* c = a;
@@ -383,9 +383,9 @@
             if ((cur & IPL_FULL) == IPL_FULL)
                 cerr << ",full";
             else {
-                if (cur & IPL_BASIC)
+                if ((cur & IPL_BASIC) != IPL_DEF)
                     cerr << ",basic";
-                if (cur & IPL_ADVANCED)
+                if ((cur & IPL_ADVANCED) != IPL_DEF)
                     cerr << ",advanced";
             }
             cerr << '\n' << "\t\t" << exp << '\n' << std::flush;
--- a/gecode/flatzinc/flatzinc.cpp	2026-11-04 18:17:10
+++ b/gecode/flatzinc/flatzinc.cpp	2026-11-04 18:17:10
@@ -2604,7 +2604,7 @@
 #endif
     IntPropLevel
     FlatZincSpace::annotation_to_int_prop_level(AST::Node* ann) {
-        int ipl = IPL_DEF;
+        IntPropLevel ipl = IPL_DEF;
         if (ann) {
             if (ann->has_atom("val") || ann->has_atom("value_propagation"))
                 ipl = IPL_VAL;
--- a/test/int.hpp	2026-11-04 18:17:10
+++ b/test/int.hpp	2026-11-04 18:17:10
@@ -208,9 +208,9 @@
             s << "Def";
             break;
         }
-        if (ipl & IPL_BASIC)
+        if ((ipl & IPL_BASIC) != IPL_DEF)
             s << "+B";
-        if (ipl & IPL_ADVANCED)
+        if ((ipl & IPL_ADVANCED) != IPL_DEF)
             s << "+A";
         return s.str();
     }
```

### intproplevel-unsigned.patch

```diff
diff --color -ru a/gecode/driver/options.cpp b/gecode/driver/options.cpp
--- a/gecode/driver/options.cpp	2026-11-04 18:55:22
+++ b/gecode/driver/options.cpp	2026-11-04 18:55:23
@@ -319,8 +319,8 @@
         int
         IplOption::parse(int argc, char* argv[]) {
             if (char* a = argument(argc, argv)) {
-                int b = IPL_DEF;
-                int m = IPL_DEF;
+                IntPropLevel b = IPL_DEF;
+                IntPropLevel m = IPL_DEF;
                 do {
                     // Search for a comma
                     char* c = a;
@@ -383,9 +383,9 @@
             if ((cur & IPL_FULL) == IPL_FULL)
                 cerr << ",full";
             else {
-                if (cur & IPL_BASIC)
+                if ((cur & IPL_BASIC) != IPL_DEF)
                     cerr << ",basic";
-                if (cur & IPL_ADVANCED)
+                if ((cur & IPL_ADVANCED) != IPL_DEF)
                     cerr << ",advanced";
             }
             cerr << '\n' << "\t\t" << exp << '\n' << std::flush;
diff --color -ru a/gecode/flatzinc/flatzinc.cpp b/gecode/flatzinc/flatzinc.cpp
--- a/gecode/flatzinc/flatzinc.cpp	2026-11-04 18:55:22
+++ b/gecode/flatzinc/flatzinc.cpp	2026-11-04 18:55:23
@@ -2604,7 +2604,7 @@
 #endif
     IntPropLevel
     FlatZincSpace::annotation_to_int_prop_level(AST::Node* ann) {
-        int ipl = IPL_DEF;
+        IntPropLevel ipl = IPL_DEF;
         if (ann) {
             if (ann->has_atom("val") || ann->has_atom("value_propagation"))
                 ipl = IPL_VAL;
diff --color -ru a/gecode/int.hh b/gecode/int.hh
--- a/gecode/int.hh	2026-11-04 18:55:22
+++ b/gecode/int.hh	2026-11-04 18:55:23
@@ -1264,7 +1264,7 @@
      *
      * \ingroup TaskModelInt
      */
-    enum IntPropLevel {
+    enum IntPropLevel : unsigned int {
         /// Simple propagation levels
         IPL_DEF = 0, ///< Default level of propagation
         IPL_VAL = 1, ///< Value propagation
@@ -1277,6 +1277,22 @@
         IPL_FULL = IPL_BASIC_ADVANCED,                 ///< Use full propagation
         IPL_BITS_ = 4                                  ///< Number of bits required (internal)
     };
+
+    /// Combine propagation levels \a a and \a b
+    constexpr IntPropLevel
+    operator|(IntPropLevel a, IntPropLevel b) noexcept {
+        return static_cast<IntPropLevel>(std::to_underlying(a) | std::to_underlying(b));
+    }
+    /// Intersect propagation levels \a a and \a b
+    constexpr IntPropLevel
+    operator&(IntPropLevel a, IntPropLevel b) noexcept {
+        return static_cast<IntPropLevel>(std::to_underlying(a) & std::to_underlying(b));
+    }
+    /// Add propagation level \a b to \a a
+    constexpr IntPropLevel&
+    operator|=(IntPropLevel& a, IntPropLevel b) noexcept {
+        return a = a | b;
+    }
 
     /// Extract value, bounds, or domain propagation from propagation level
     IntPropLevel vbd(IntPropLevel ipl);
diff --color -ru a/gecode/minimodel.hh b/gecode/minimodel.hh
--- a/gecode/minimodel.hh	2026-11-04 18:55:22
+++ b/gecode/minimodel.hh	2026-11-04 18:55:23
@@ -102,22 +102,22 @@
     /// Class for specifying integer propagation levels used by minimodel
     class IntPropLevels {
     protected:
-        IntPropLevel _linear2 : IPL_BITS_; ///< For binary linear
-        IntPropLevel _linear : IPL_BITS_;  ///< For n-ary linear
-        IntPropLevel _abs : IPL_BITS_;     ///< For absolute value
-        IntPropLevel _max2 : IPL_BITS_;    ///< For binary maximum
-        IntPropLevel _max : IPL_BITS_;     ///< For n-ary maximum
-        IntPropLevel _min2 : IPL_BITS_;    ///< For binary minimum
-        IntPropLevel _min : IPL_BITS_;     ///< For minimum
-        IntPropLevel _mult : IPL_BITS_;    ///< For multiplication
-        IntPropLevel _div : IPL_BITS_;     ///< For division
-        IntPropLevel _mod : IPL_BITS_;     ///< For modulo
-        IntPropLevel _sqr : IPL_BITS_;     ///< For square
-        IntPropLevel _sqrt : IPL_BITS_;    ///< For square root
-        IntPropLevel _pow : IPL_BITS_;     ///< For power
-        IntPropLevel _nroot : IPL_BITS_;   ///< For root
-        IntPropLevel _element : IPL_BITS_; ///< For element
-        IntPropLevel _ite : IPL_BITS_;     ///< For if-then-else
+        IntPropLevel _linear2 : static_cast<int>(IPL_BITS_); ///< For binary linear
+        IntPropLevel _linear : static_cast<int>(IPL_BITS_);  ///< For n-ary linear
+        IntPropLevel _abs : static_cast<int>(IPL_BITS_);     ///< For absolute value
+        IntPropLevel _max2 : static_cast<int>(IPL_BITS_);    ///< For binary maximum
+        IntPropLevel _max : static_cast<int>(IPL_BITS_);     ///< For n-ary maximum
+        IntPropLevel _min2 : static_cast<int>(IPL_BITS_);    ///< For binary minimum
+        IntPropLevel _min : static_cast<int>(IPL_BITS_);     ///< For minimum
+        IntPropLevel _mult : static_cast<int>(IPL_BITS_);    ///< For multiplication
+        IntPropLevel _div : static_cast<int>(IPL_BITS_);     ///< For division
+        IntPropLevel _mod : static_cast<int>(IPL_BITS_);     ///< For modulo
+        IntPropLevel _sqr : static_cast<int>(IPL_BITS_);     ///< For square
+        IntPropLevel _sqrt : static_cast<int>(IPL_BITS_);    ///< For square root
+        IntPropLevel _pow : static_cast<int>(IPL_BITS_);     ///< For power
+        IntPropLevel _nroot : static_cast<int>(IPL_BITS_);   ///< For root
+        IntPropLevel _element : static_cast<int>(IPL_BITS_); ///< For element
+        IntPropLevel _ite : static_cast<int>(IPL_BITS_);     ///< For if-then-else
     public:
         /// Initialize with default propagation level
         IntPropLevels(IntPropLevel ipl = IPL_DEF);
diff --color -ru a/test/int.hpp b/test/int.hpp
--- a/test/int.hpp	2026-11-04 18:55:22
+++ b/test/int.hpp	2026-11-04 18:55:23
@@ -208,9 +208,9 @@
             s << "Def";
             break;
         }
-        if (ipl & IPL_BASIC)
+        if ((ipl & IPL_BASIC) != IPL_DEF)
             s << "+B";
-        if (ipl & IPL_ADVANCED)
+        if ((ipl & IPL_ADVANCED) != IPL_DEF)
             s << "+A";
         return s.str();
     }
```

### actualcost-step1.patch

```diff
--- a/gecode/kernel/core.hpp	2026-11-04 18:28:06
+++ b/gecode/kernel/core.hpp	2026-11-04 18:28:06
@@ -1905,7 +1905,7 @@
                  */
                 ActorLink* active;
                 /// Scheduled propagators according to cost
-                ActorLink queue[PropCost::AC_MAX + 1];
+                ActorLink queue[static_cast<int>(PropCost::AC_MAX) + 1];
                 /**
                  * \brief Id of next brancher to be created plus status control
                  *
@@ -4258,7 +4258,7 @@
 
     forceinline void
     Space::fail(void) {
-        pc.p.active = &pc.p.queue[PropCost::AC_MAX + 1] + 1;
+        pc.p.active = &pc.p.queue[static_cast<int>(PropCost::AC_MAX) + 1] + 1;
         /*
          * Now active points beyond the last queue. This is essential as
          * enqueuing a propagator in a failed space keeps the space
@@ -4272,7 +4272,7 @@
 
     forceinline bool
     Space::failed(void) const {
-        return pc.p.active > &pc.p.queue[PropCost::AC_MAX + 1];
+        return pc.p.active > &pc.p.queue[static_cast<int>(PropCost::AC_MAX) + 1];
     }
     forceinline bool
     Home::failed(void) const {
@@ -4281,7 +4281,7 @@
 
     forceinline bool
     Space::stable(void) const {
-        return ((pc.p.active < &pc.p.queue[0]) || (pc.p.active > &pc.p.queue[PropCost::AC_MAX + 1]));
+        return ((pc.p.active < &pc.p.queue[0]) || (pc.p.active > &pc.p.queue[static_cast<int>(PropCost::AC_MAX) + 1]));
     }
 
     forceinline void
--- a/gecode/kernel/core.cpp	2026-11-04 18:28:06
+++ b/gecode/kernel/core.cpp	2026-11-04 18:28:06
@@ -126,7 +126,7 @@
         // Initialize space as stable but not failed
         pc.p.active = &pc.p.queue[0] - 1;
         // Initialize propagator queues
-        for (int i = 0; i <= PropCost::AC_MAX; i++)
+        for (int i = 0; i <= static_cast<int>(PropCost::AC_MAX); i++)
             pc.p.queue[i].init();
         pc.p.bid_sc = (reserved_bid + 1) << sc_bits;
         pc.p.n_sub = 0;
@@ -271,7 +271,7 @@
         // Check whether space is failed
         if (failed())
             return SS_FAILED;
-        assert(pc.p.active <= &pc.p.queue[PropCost::AC_MAX + 1]);
+        assert(pc.p.active <= &pc.p.queue[static_cast<int>(PropCost::AC_MAX) + 1]);
         Propagator* p;
         // Check whether space is stable but not failed
         if (pc.p.active >= &pc.p.queue[0]) {
@@ -877,7 +877,7 @@
 
         // Initialize propagator queue
         c->pc.p.active = &c->pc.p.queue[0] - 1;
-        for (int i = 0; i <= PropCost::AC_MAX; i++)
+        for (int i = 0; i <= static_cast<int>(PropCost::AC_MAX); i++)
             c->pc.p.queue[i].init();
         // Copy propagation only data
         c->pc.p.n_sub = pc.p.n_sub;
```

### actualcost-step2.patch

```diff
diff --color -ru a/gecode/kernel/core.cpp b/gecode/kernel/core.cpp
--- a/gecode/kernel/core.cpp	2026-11-04 18:58:28
+++ b/gecode/kernel/core.cpp	2026-11-04 18:58:28
@@ -126,7 +126,7 @@
         // Initialize space as stable but not failed
         pc.p.active = &pc.p.queue[0] - 1;
         // Initialize propagator queues
-        for (int i = 0; i <= PropCost::AC_MAX; i++)
+        for (int i = 0; i <= static_cast<int>(PropCost::AC_MAX); i++)
             pc.p.queue[i].init();
         pc.p.bid_sc = (reserved_bid + 1) << sc_bits;
         pc.p.n_sub = 0;
@@ -271,7 +271,7 @@
         // Check whether space is failed
         if (failed())
             return SS_FAILED;
-        assert(pc.p.active <= &pc.p.queue[PropCost::AC_MAX + 1]);
+        assert(pc.p.active <= &pc.p.queue[static_cast<int>(PropCost::AC_MAX) + 1]);
         Propagator* p;
         // Check whether space is stable but not failed
         if (pc.p.active >= &pc.p.queue[0]) {
@@ -536,7 +536,7 @@
         // Mark as failed
         fail();
         // Propagate top priority propagators
-        ActorLink* e = &pc.p.queue[PropCost::AC_RECORD];
+        ActorLink* e = &pc.p.queue[static_cast<int>(PropCost::AC_RECORD)];
         for (ActorLink* a = e->next(); a != e; a = a->next()) {
             Propagator* top = Propagator::cast(a);
             // Keep old modification event delta
@@ -877,7 +877,7 @@
 
         // Initialize propagator queue
         c->pc.p.active = &c->pc.p.queue[0] - 1;
-        for (int i = 0; i <= PropCost::AC_MAX; i++)
+        for (int i = 0; i <= static_cast<int>(PropCost::AC_MAX); i++)
             c->pc.p.queue[i].init();
         // Copy propagation only data
         c->pc.p.n_sub = pc.p.n_sub;
diff --color -ru a/gecode/kernel/core.hpp b/gecode/kernel/core.hpp
--- a/gecode/kernel/core.hpp	2026-11-04 18:58:28
+++ b/gecode/kernel/core.hpp	2026-11-04 18:58:28
@@ -1905,7 +1905,7 @@
                  */
                 ActorLink* active;
                 /// Scheduled propagators according to cost
-                ActorLink queue[PropCost::AC_MAX + 1];
+                ActorLink queue[static_cast<int>(PropCost::AC_MAX) + 1];
                 /**
                  * \brief Id of next brancher to be created plus status control
                  *
@@ -4250,7 +4250,7 @@
     forceinline void
     Space::enqueue(Propagator* p) {
         ActorLink::cast(p)->unlink();
-        ActorLink* c = &pc.p.queue[p->cost(*this, p->u.med).ac];
+        ActorLink* c = &pc.p.queue[static_cast<int>(p->cost(*this, p->u.med).ac)];
         c->tail(ActorLink::cast(p));
         if (c > pc.p.active)
             pc.p.active = c;
@@ -4258,7 +4258,7 @@
 
     forceinline void
     Space::fail(void) {
-        pc.p.active = &pc.p.queue[PropCost::AC_MAX + 1] + 1;
+        pc.p.active = &pc.p.queue[static_cast<int>(PropCost::AC_MAX) + 1] + 1;
         /*
          * Now active points beyond the last queue. This is essential as
          * enqueuing a propagator in a failed space keeps the space
@@ -4272,7 +4272,7 @@
 
     forceinline bool
     Space::failed(void) const {
-        return pc.p.active > &pc.p.queue[PropCost::AC_MAX + 1];
+        return pc.p.active > &pc.p.queue[static_cast<int>(PropCost::AC_MAX) + 1];
     }
     forceinline bool
     Home::failed(void) const {
@@ -4281,7 +4281,7 @@
 
     forceinline bool
     Space::stable(void) const {
-        return ((pc.p.active < &pc.p.queue[0]) || (pc.p.active > &pc.p.queue[PropCost::AC_MAX + 1]));
+        return ((pc.p.active < &pc.p.queue[0]) || (pc.p.active > &pc.p.queue[static_cast<int>(PropCost::AC_MAX) + 1]));
     }
 
     forceinline void
```

### enginetype.patch

```diff
diff --color -ru a/gecode/search/tracer.cpp b/gecode/search/tracer.cpp
--- a/gecode/search/tracer.cpp	2026-11-04 18:54:19
+++ b/gecode/search/tracer.cpp	2026-11-04 18:54:19
@@ -34,7 +34,7 @@
 
 namespace Gecode {
 
-    const char* StdSearchTracer::t2s[SearchTracer::EngineType::AOE + 1] = {"DFS", "BAB", "LDS", "RBS", "PBS", "AOE"};
+    const char* StdSearchTracer::t2s[static_cast<int>(SearchTracer::EngineType::AOE) + 1] = {"DFS", "BAB", "LDS", "RBS", "PBS", "AOE"};
 
     StdSearchTracer::StdSearchTracer(std::ostream& os0) : os(os0) {}
 
diff --color -ru a/gecode/search.hh b/gecode/search.hh
--- a/gecode/search.hh	2026-11-04 18:54:19
+++ b/gecode/search.hh	2026-11-04 18:54:19
@@ -378,7 +378,7 @@
         /// Output stream to use
         std::ostream& os;
         /// Map engine type to string
-        static const char* t2s[EngineType::AOE + 1];
+        static const char* t2s[static_cast<int>(EngineType::AOE) + 1];
 
     public:
         /// Initialize  with output stream \a os
```

### enginetype-step2.patch

```diff
diff --color -ru a/gecode/search/tracer.cpp b/gecode/search/tracer.cpp
--- a/gecode/search/tracer.cpp	2026-11-04 18:54:19
+++ b/gecode/search/tracer.cpp	2026-11-04 19:00:26
@@ -34,7 +34,7 @@
 
 namespace Gecode {
 
-    const char* StdSearchTracer::t2s[SearchTracer::EngineType::AOE + 1] = {"DFS", "BAB", "LDS", "RBS", "PBS", "AOE"};
+    const char* StdSearchTracer::t2s[static_cast<int>(SearchTracer::EngineType::AOE) + 1] = {"DFS", "BAB", "LDS", "RBS", "PBS", "AOE"};
 
     StdSearchTracer::StdSearchTracer(std::ostream& os0) : os(os0) {}
 
@@ -42,7 +42,7 @@
     StdSearchTracer::init(void) {
         os << "trace<Search>::init()" << '\n' << std::flush;
         for (unsigned int e = 0U; e < engines(); e++) {
-            os << "\t" << e << ": " << t2s[engine(e).type()];
+            os << "\t" << e << ": " << t2s[std::to_underlying(engine(e).type())];
             if (engine(e).meta()) {
                 os << ", engines: {";
                 for (unsigned int i = engine(e).efst(); i < engine(e).elst(); i++) {
diff --color -ru a/gecode/search.hh b/gecode/search.hh
--- a/gecode/search.hh	2026-11-04 18:54:19
+++ b/gecode/search.hh	2026-11-04 18:54:19
@@ -378,7 +378,7 @@
         /// Output stream to use
         std::ostream& os;
         /// Map engine type to string
-        static const char* t2s[EngineType::AOE + 1];
+        static const char* t2s[static_cast<int>(EngineType::AOE) + 1];
 
     public:
         /// Initialize  with output stream \a os
```

### statustype-alias.patch

```diff
diff --color -ru a/gecode/int/extensional/compact.hpp b/gecode/int/extensional/compact.hpp
--- a/gecode/int/extensional/compact.hpp	2026-11-04 18:58:28
+++ b/gecode/int/extensional/compact.hpp	2026-11-04 18:58:28
@@ -138,7 +138,7 @@
     template <class Advisor>
     forceinline
     CompactStatus<Advisor>::CompactStatus(StatusType t)
-        : s(t) {}
+        : s(std::to_underlying(t)) {}
 
     template <class Advisor>
     forceinline typename CompactStatus<Advisor>::StatusType
@@ -157,22 +157,22 @@
     CompactStatus<Advisor>::touched(Advisor& a) {
         if (type() == NONE) {
             s = reinterpret_cast<Support::uptrint_t>(&a);
-            assert((s & 0b11) == SINGLE);
+            assert((s & 0b11) == std::to_underlying(SINGLE));
         } else if ((type() == SINGLE) && !single(a)) {
-            s = MULTIPLE;
+            s = std::to_underlying(MULTIPLE);
         }
     }
 
     template <class Advisor>
     forceinline void
     CompactStatus<Advisor>::none(void) {
-        s = NONE;
+        s = std::to_underlying(NONE);
     }
 
     template <class Advisor>
     forceinline void
     CompactStatus<Advisor>::propagating(void) {
-        s = PROPAGATING;
+        s = std::to_underlying(PROPAGATING);
     }
 
     /*
```

### ev_t-named.patch

```diff
--- a/gecode/int/cumulatives/val.hpp	2026-11-04 19:25:59
+++ b/gecode/int/cumulatives/val.hpp	2026-11-04 19:25:59
@@ -115,7 +115,7 @@
     }
 
     /// Types of events for the sweep-line
-    typedef enum { EVENT_CHCK, EVENT_PROF, EVENT_PRUN } ev_t;
+    enum ev_t { EVENT_CHCK, EVENT_PROF, EVENT_PRUN };
     /// An event collects the information for one event for the sweep-line
     class Event {
     public:
```

### eventtype-named.patch

```diff
--- a/gecode/int/cumulatives/val.hpp	2026-11-04 19:25:59
+++ b/gecode/int/cumulatives/val.hpp	2026-11-04 19:25:59
@@ -115,12 +115,12 @@
     }
 
     /// Types of events for the sweep-line
-    typedef enum { EVENT_CHCK, EVENT_PROF, EVENT_PRUN } ev_t;
+    enum EventType { EVENT_CHCK, EVENT_PROF, EVENT_PRUN };
     /// An event collects the information for one event for the sweep-line
     class Event {
     public:
         /// The type of the event
-        ev_t e;
+        EventType e;
         /// The task this event refers to
         int task;
         /// The date of this event
@@ -128,7 +128,7 @@
         /// The quantity changed by this event (if any)
         int inc;
         /// Simple constructor
-        Event(ev_t _e, int _task, int _date, int _inc = 0) : e(_e), task(_task), date(_date), inc(_inc) {}
+        Event(EventType _e, int _task, int _date, int _inc = 0) : e(_e), task(_task), date(_date), inc(_inc) {}
 
         // Default constructor for region-allocated memory
         // NOLINTNEXTLINE(modernize-use-equals-default)
```
