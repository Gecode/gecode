# Native spike: a Gecode-native interval library

## Result

This answers boost-007. In a scratch copy of HEAD, it replaces the vendored
Boost.Interval copy in `gecode/third-party/boost` with Gecode code: a
double-only `Float::Interval` and a rounding layer without any Boost code.
It then measures the prototype against HEAD with the oracles of the approach
study and one new probe.

**Verdict: success.** On arm64 macOS and on x86-64 Linux, the prototype
gives the same bounds, truth values, exceptions, and caller rounding modes
as HEAD on every oracle that was run. The only observable difference is the
text of the empty-interval exception. The float suite, the FlatZinc tests,
and the examples give the same results. The prototype has no external
dependency, installs 95 fewer header files, and its interval code is about
a third smaller than the approach study's lower estimate. Real MSVC runs
are the one gap: they need a CI run that the user has to authorize (see Not
verified).

| measure | HEAD | native | command |
|---|---|---|---|
| Boost or third-party files in the source tree | 105 | 0 | [C19] |
| installed header files / lines | 731 / 177270 | 636 / 167426 | [C19] |
| installed files that mention `gecode_boost`; `LICENSE_1_0.txt` installed | 40; yes | 0; no | [C19] |
| compiler warnings: arm64 builds / x86-64 GCC / x86-64 Clang | 0 / 17 / 0 | 0 / 17 (the same) / 0 | [C3], [C17] |
| `FloatVal` operation lines (`diffops`) differing from HEAD, of 404800 (MPFR) and 381920 (no MPFR), at -O0 and -O2 | - | 0 | [C5] |
| the same on x86-64, GCC 15 and Clang 21 at -O2, of 404800 | - | 0 | [C17] |
| `diffops` lines that leave the caller's mode changed | 0 | 0 | [C5], [C17] |
| rounding probe configurations with 0 unsound rows and HEAD's output, of 9 on arm64 and 6 on each x86-64 install | 9 and 6 | 9 and 6 | [C6], [C17] |
| `nth_root` rows: unsound / mode changed / bounds differ from HEAD, of 240 | 0 / 0 / - | 0 / 0 / 0 | [C5] |
| comparison probe as HEAD, apart from the exception message | - | yes | [C5] |
| pi bounds as HEAD | - | yes | [C5] |
| `FloatVal` trivially copyable | no | yes | [C5] |
| Float:: suite, failed of 1563 (MPFR) and 1425 (no MPFR), Release | 0 and 0 | 0 and 0 | [C9] |
| Float:: check and standard tags, failed of 529 and 508, Debug | 0 and 0 | 0 and 0 | [C10] |
| Float:: suite on x86-64: GCC failed of 1563; Clang check and standard failed of 529 | 0; 0 | 0; 0 | [C17] |
| FlatZinc:: failed of 152; ctest failed of 7 | 0; 0 | 0; 0 | [C12] |
| builds whose example outputs and statistics equal HEAD's, of 7 native builds | - | 7 | [C13], [C17] |
| hexfloat example output equal to HEAD's, of 4 (MPFR) and 2 (no MPFR) | - | 4 and 2 | [C13] |
| example runtime, x86-64 median ms, GCC 15: archimedean / cartesian / descartes / golden | 264.8 / 212.2 / 505.6 / 190.7 | 265.3 / 210.3 / 504.8 / 192.5 | [C18] |
| the same, Clang 21 | 266.1 / 232.1 / 633.3 / 191.4 | 265.2 / 225.1 / 560.8 / 191.4 | [C18] |
| microbenchmark, x86-64 median ns per `FloatVal` iteration, GCC / Clang | 60.6 / 69.2 | 60.5 / 62.9 | [C18] |
| Float module rebuild, x86-64 GCC 15 -j 4, median real s | 34.21 | 33.79 | [C18] |
| preprocessed lines of a TU including `gecode/float.hh`, AppleClang | 140788 | 138191 | [C16] |
| exported symbols, 14 physical libraries; differing | 2185; - | 2185; 0 | [C20] |
| interval code, source lines, without / with MPFR | Boost: 974 to 1316 / +139 to 215 (estimate) | 627 / +117 | [C22] |
| rounding layer, source lines | Boost: 290 | 99 added to Gecode's | [C22] |
| patch without third-party: files / added / removed lines | - | 7 / 1290 / 156 | [C23] |

The prototype also passes the CMake consumer smoke tests with and without
MPFR [C21], and the MSVC per-operation arithmetic style passes the rounding
tests, the check and standard Float:: tags, and the rounding probe in a
simulation copy [C6], [C7], [C11]. The `_control87` code passes a
syntax-only compile for x64 and x86 MSVC targets [C14].

Runtime is the same or better. With GCC, every example is within 1 % of
HEAD. With Clang, descartes-folium is 11 % faster on x86-64 [C18], and the
`FloatVal` microbenchmark is 9 % faster. The arm64 machine was loaded by
another process during the timing runs, so its runtime numbers are listed
only in Measurements, as indicative [C15].

## Question and scope

boost-004 decides how Gecode 7 handles interval arithmetic. The approach
study (boost-003) recommended approach 3, a Gecode support library, from a
line count of Boost's code alone. The user has low trust in Boost and asked
for a spike of a fully native library before deciding. This spike answers:

- Does a double-only interval type and rounding layer with no Boost code
  keep today's bounds, search, and runtime?
- How large is it, compared with the approach study's estimate?
- What does it change in the API, the ABI, the build, users, and the other
  Gecode 7 areas?
- How should boost-005 split the work?

Covered:

- A complete prototype in `build/zdev-impl/boost-007/src/native`, built in
  Release and Debug, with and without MPFR.
- Fresh HEAD reference builds at 781a883e6b, built the same way.
- Every oracle of the approach study, plus a new probe that compares every
  `FloatVal` operation bit for bit with HEAD.
- x86-64 Linux with GCC 15 and Clang 21.
- MSVC's per-operation arithmetic style, run on arm64 in a simulation copy,
  and a syntax-only compile of the `_control87` code.
- Runtime, Float module build time, installed headers, exported symbols,
  and a downstream code search.

Not covered:

- Real MSVC, MinGW, or Windows runs. No CI branch was pushed.
- The license decision for the ported code (see Wider impacts).
- Any tracked source change. The prototype exists only as a patch under
  `build/`.

## Setup

- Revision 781a883e6bba80524e1d5d2a0f097eb60c5187e0 on branch
  feature/breaking-7, clean tree [C1]. `.zdev/` is listed in
  `.git/info/exclude`, so this study shows only with `--ignored`.
- arm64: Apple M1 Max, 10 cores, macOS 27.0.1 [C1].
  - Builds use `/Library/Developer/CommandLineTools/usr/bin/c++`, Apple
    clang 21.0.0 (clang-2100.3.34.2), as build/local and boost-003 did.
  - Probes use `/usr/bin/clang++`, the same compiler. The rounding probe adds
    Homebrew LLVM 22.1.8 and GCC 14.2.0, as in rounding-probe.md.
  - CMake 3.27.6, MPFR 4.2.2, clang-format 22.1.8.
- x86-64: erebor, Ubuntu with GCC 15.2.0 and Clang 21.1.8, 4 cores [C1].
- Scratch layout, all under `build/zdev-impl/boost-007` (W below), which git
  ignores:
  - `src/head`: `git archive HEAD`, never edited;
  - `src/native`: the same archive with the prototype's edits;
  - `src/native-msvcsim`: `src/native` with one predicate forced on (see
    Rounding per target);
  - `bld/<tree>-{rel,rel-nompfr,dbg,dbg-nompfr}`, `inst/<variant>`,
    `logs/`, and `results/`.
- Build options (`scripts/build.sh`): build/local's compiler,
  `GECODE_ENABLE_GIST=OFF`, `GECODE_ENABLE_QT=OFF`, `CMAKE_BUILD_TYPE`, and
  `GECODE_ENABLE_MPFR`. Release builds use Unix Makefiles and Debug builds
  use Ninja. The CMake caches of each head and native pair are equal apart
  from their paths [C3].
- On erebor, `~/boost-007` holds the same two trees, shipped with tar, and
  Ninja Release builds with MPFR for each compiler [C17].
- Python ran with `python3 -I`.

All paths below are relative to W unless they start with `gecode/`, `test/`,
or another source directory, which are relative to `src/native` or
`src/head`.

## Prototype

### Files

| file | change |
|---|---|
| `gecode/float/interval.hpp` | new, 1042 lines: `Float::Interval` and its operations |
| `gecode/float.hh` | the Boost include goes; `RoundingBase` has no base class and gets the mode-control members; `Rounding` becomes a Gecode scope class; `FloatValImpType` is `Float::Interval` |
| `gecode/float/rounding.hpp` | mode control through `<cfenv>` or `_control87`, the `Rounding` scope, and the median's nearest-mode step |
| `gecode/float/num.hpp` | the pi bounds become hexfloat literals |
| `gecode/float/val.hpp` | native calls, and the comparisons as plain conditions |
| `test/float/arithmetic.cpp` | RoundingScope uses `Float::Interval` instead of a Boost interval |
| `CMakeLists.txt` | the `LICENSE_1_0.txt` install pattern and the third-party doc filter go |
| `gecode/third-party/` | deleted, 105 files and 11308 lines |

`cmake/GecodeSources.cmake` needs no change: it lists no headers, and the
install rule picks headers up by glob. `rounding.cpp`, which holds the MPFR
functions, is unchanged.

### Interval type and invariants

`Float::Interval` holds two `FloatNum` bounds with member initializers for
the default [0,0], which is Boost's default. It declares no copy, move, or
destructor, so it is trivially copyable, and so is `FloatVal` (FloatVal's own
members were defaulted by modern-classes-003).

The type keeps Boost's `checking_strict` behaviour exactly:

- `Interval(v)` does not check its argument and accepts NaN.
- `Interval(l,u)` and `assign(l,u)` throw `std::runtime_error` unless
  `l <= u`, which also rejects NaN bounds.
- There is no empty state. Every place where Boost returns an empty
  interval throws the same exception through one `[[noreturn]]` helper:
  - `pow(x,0)` with x=[0,0];
  - `sqrt` or `log` of an interval with no positive part;
  - an even `nth_root` of a negative interval;
  - division by [0,0] or by 0.0, and the inverse of [0,0];
  - a disjoint `intersect`;
  - out-of-domain `asin`, `acos`, `acosh`, and `atanh`.
- Internal results are built with `Interval::unchecked(l,u)`, exactly where
  Boost uses `I(l,u,true)`. NaN bounds from 0·∞ or ∞−∞ therefore pass
  through as today.
- Boost's `test_input`, `is_nan`, and `is_empty` branches are dropped. Under
  `checking_strict` they are constant false
  (`checking_no_nan::is_nan` and `checking_no_empty::is_empty` return
  false), so this removes dead code and changes no behaviour. This corrects
  the approach study's remark that the NaN checks still matter.

### Rounding per target

Two predicates in `gecode/float.hh` select the code, each defined once:

```cpp
#if defined(_MSC_VER) && !defined(__x86_64__) && (defined(_M_X64) || defined(_M_IX86))
#define GECODE_FLOAT_MODE_CONTROL87
#include <float.h>
#endif
#if defined(_MSC_VER) && (defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP > 0)))
#define GECODE_FLOAT_DIRECTED_PER_OPERATION
#endif
```

- **Mode control.** `GECODE_FLOAT_MODE_CONTROL87` selects `_control87`.
  Everything else uses `std::fegetround` and `std::fesetround`: x86-64 GCC
  and Clang including MinGW, AArch64 on macOS and Linux, and MSVC ARM64.
  - Boost's `hw_rounding.hpp` takes the C99 control for x86-64 whenever C99
    is available, before it looks at `_M_X64`. clang-cl defines
    `__x86_64__`, so Boost uses the C99 control there, and so does the
    native code. cl.exe does not define `__x86_64__` and gets `_control87`.
  - Boost reaches `msvc_rounding_control` for x86 MSVC, and the native code
    also uses `_control87` there.
  - [C14] confirms the selection with clang's predefined macros: the
    x64 MSVC target selects only the per-operation style, i686 MSVC selects
    both, and ARM64 MSVC and x86-64 MinGW select neither.
- **Arithmetic style.** `GECODE_FLOAT_DIRECTED_PER_OPERATION` keeps today's
  predicate. Under it, every operation sets its own direction, as Boost's
  `rounded_arith_std`. Elsewhere the mode stays upward between operations,
  as `rounded_arith_opp`. The boost-006 arithmetic in `RoundingBase` already
  implements both styles and is unchanged.
- `RoundingBase` has no base class. Its new protected static members are
  `Mode`, `current_mode`, `restore_mode`, `upward`, `downward`,
  `to_nearest`, and `init`. `init` sets upward for the opp style and does
  nothing for the per-operation style, as Boost's.
- `Rounding` saves the mode, calls `init()`, and restores the mode in its
  destructor, as `save_state` did. Copying is deleted.
- The `_control87` code (18 lines in `rounding.hpp`):
  - the saved mode is `_control87(0,0)`;
  - restore is `_control87(saved, mask)`;
  - upward is `_control87(base | _RC_UP, mask)`, and downward and to_nearest
    follow the same pattern;
  - on x64 the mask is `_MCW_EM | _MCW_RC` and the base is `_MCW_EM`;
  - on x86 the mask adds `_MCW_PC | _MCW_IC` and the base adds `_PC_64 |
    _IC_AFFINE`. These are what Boost's x87 words 0x137f, 0x177f, and 0x1b7f
    decode to through `hard2msvc`: all six exceptions masked, 64-bit
    precision, affine infinity, and the rounding bits.
- The median ports `rounded_arith_opp::median` and `rounded_arith_std::median`:
  to_nearest, then `(x+y)/2` through `barrier()`, then upward again for the
  opp style. This replaces Boost's `volatile` store. The boost-006
  overflow-safe wrapper around it stays.
- `barrier()`, the `sqrt_down` mode switch, and `int_down`/`int_up` are
  unchanged.

### Scope rule

Every public operation that rounds opens one `Rounding` at entry, as Boost
declares `typename Policies::rounding rnd` on the protected type. The
helpers in namespace `IntervalDetail` take that scope as a `RoundingBase&`
and open none, as Boost runs them on the unprotected type: the nested
arithmetic in `fmod`, `cos`, `sin`, and `tan`; `nth_root`'s Newton steps
with their `median`, `pow`, `intersect`, and `equal`; and `pow`'s recursion
through the inverse. changelog.in already documents this behaviour for HEAD.

### From Boost to native

Source lines per group, measured with boost-003's `sloc.py` [C22]. The
Boost column is the approach study's measurement [C22 in approach-study.md].

| Boost file | native functions | native lines |
|---|---|---|
| `interval.hpp` (class, construction, update operators) | `Interval`, `unchecked`, `assign`, `lower`, `upper`, `+= -= *= /=`, `throw_empty_interval` | 104 |
| `arith.hpp`, `detail/division.hpp` | `+ - * /` for all operand kinds, `add`, `sub`, `mul` (the 9 sign cases), `div`, `div_non_zero`, `div_positive`, `div_negative`, `div_zero`, `sqrt` | 255 |
| `arith2.hpp` | `fmod` (3), `square`, `pow`, `pow_down`, `pow_up`, `multiplicative_inverse`, `nth_root`, `root_newton`, `root_down`, `root_up` | 167 |
| `utility.hpp` | `width`, `singleton`, `in`, `zero_in`, `subset`, `proper_subset`, `overlap`, `equal`, `intersect`, `hull` (3), `abs`, `max` (3), `min` (3) | 89 |
| `constants.hpp` | `pi_half`, `pi`, `pi_twice` | 12 |
| **interval code, no MPFR** | | **627** |
| `transc.hpp` (MPFR) | `exp`, `log`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh` | 117 |
| rounding control (`hw_rounding.hpp`, `rounding.hpp`, the control files) | the mode members, `Rounding`, `nearest_median`, the predicates | 99 added to `float.hh` and `rounding.hpp` |

The estimate was 974 to 1316 lines of interval code, plus 139 to 215 with
MPFR, plus 290 lines of rounding control. The port needs 627, plus 117,
plus 99. The lower estimate already left out template plumbing and the
empty-operand checks. The rest of the difference comes from one number type,
one policy, and the dead `checking_strict` branches. `val.hpp` shrinks by 46
lines, because the comparisons no longer catch exceptions [C22].

`std::min`/`std::max` keep Boost's argument order, which decides NaN and ±0
ties. One-line functions are `forceinline`, the rest `inline`.

### Comparisons

All `FloatVal` comparisons are "certainly" comparisons: Boost returns true
only on the certain branch, and val.hpp maps the `comparison_error` it
otherwise throws to false. The native code states the conditions directly
and throws nothing:

| comparison | `FloatVal x`, `FloatVal y` | `FloatVal x`, number `n` |
|---|---|---|
| `<` | `x.max < y.min` | `x.max < n` |
| `<=` | `x.max <= y.min` | `x.max <= n` |
| `>` | `x.min > y.max` | `x.min > n` |
| `>=` | `x.min >= y.max` | `x.min >= n` |
| `==` | equal bounds (patch 03's set equality) | today's code without the constant-false `is_empty` test |
| `!=` | `x.max < y.min \|\| x.min > y.max` | `x.max < n \|\| x.min > n` |

The forms with the number on the left are unchanged. The comparison probe
and `diffops` reproduce HEAD's truth table for all of them [C5].

### Deliberate differences from Boost

1. **One scope per public operation.** Boost's division helpers and nested
   `pow` each open their own `save_state`, and Boost skips the mode switch
   for results that need no arithmetic: division by an interval that
   contains zero in its interior, a [0,0] dividend, `pow(x,0)`, and
   `nth_root` with k=1. The port opens one scope at entry. The bounds are the
   same, because the mode inside is upward either way. The x86-64 assembly of
   the microbenchmark has 2 or 3 fewer `fesetround` calls and 2 fewer
   `fegetround` calls [C18].
2. **`Interval::unchecked` is a public static factory**, not a private tag
   constructor as the plan proposed. A private constructor would need a list
   of friend functions.
3. **The exception message** is "Gecode::Float::Interval: empty interval"
   instead of "gecode_boost::interval: empty interval created". The type is
   still `std::runtime_error`.
4. **`Rounding` cannot be copied.** Boost's `save_state` could be copied
   implicitly, and the copy restored the mode a second time.
5. **The median uses `barrier()`** instead of Boost's `volatile` store.
6. **`_control87` stores the control word directly** instead of Boost's
   round trip through the x87 layout (`msvc2hard`, then `hard2msvc`). For
   every bit in the mask, the round trip is the identity: the six exception
   bits map one to one, the four rounding values map one to one, the three
   valid precision values map one to one, and `_IC_AFFINE` maps to itself.
   The only exception is the reserved precision value `_MCW_PC`, which Boost
   would turn into `_PC_24`, and which no caller sets.
7. **MSVC ARM64 uses `<cfenv>`.** Boost's copy reaches an `#error` there
   unless `/Zc:__cplusplus` is set. Untested.
8. **x86-32 GCC uses `<cfenv>`** instead of Boost's x87 inline assembly
   (`x86gcc_rounding_control`). The class comment already states that x87
   arithmetic is not supported.
9. **The dead `checking_strict` branches are dropped** (see Interval type
   and invariants).
10. `median(Interval)` exists only inside the Newton step, as
    `r.median(l,u)`. Gecode never called Boost's free `median`.

### Size and patch

- `native.patch`: `diff -ruN src/head src/native`, 112 files, 1290 lines
  added and 11464 removed. SHA-256 `c1fd81db7e2b60f57d1b2912eabaa27e60b2bd9195bd75fab0862dc4fd6bd2c9`.
- `native-code.patch`: the same with `-x third-party`, 7 files, 1290 added
  and 156 removed. SHA-256 `745f6ccdf2027f370e8474231250ac60cade97c0fbdf83cbe9f2379730e95598`.
- Both are in W [C23]. `grep -rnE 'gecode_boost|GECODE_BOOST|third-party/boost'
  --exclude-dir=.zdev src/native` finds nothing in the implementation files,
  and `src/native/gecode/third-party` does not exist [C26]. The archived
  `.zdev` records in the copy still mention these names.
- clang-format 22 leaves every changed file unchanged [C26].

## Oracle adaptation

Three checks used Boost directly. Each got a native version in the scratch
copy; one probe is new.

| check | at HEAD | native version | why it is equivalent |
|---|---|---|---|
| `Float::Arithmetic::RoundingScope` | `boost_operations<Policy>` builds a Boost interval with `checking_strict` and checks `fmod(5,3) == [2,2]` and, with MPFR, that `sin(0.5)` brackets 0.479425538604203, each with the caller's mode unchanged; it runs for `Policy = Rounding` and for `save_state<Rounding>` | `interval_operations()` makes the same calls on `Float::Interval`; it runs once in the outer `Rounding` and once inside an extra nested `Rounding`, the equivalent of `save_state<Rounding>` | the same operations, values, and mode checks at the same nesting depths; the half-ulp `add_down`/`add_up` check, the nested median scope, all four caller modes, the name, and the tags are unchanged |
| comparison probe | boost-003's `cmp.cpp`, with a column of raw Boost results | `probe/cmp-native.cpp`: the same rows, `FloatVal` and number-left columns, and `FloatVal(2,1)` line, without the raw column | HEAD's output with the raw column masked by `sed` equals the native output apart from the exception message [C5] |
| `nth_root` probe | boost-003's `nroot.cpp` | unchanged; it uses only `FloatVal` | - |
| `diffops` (new) | - | every `FloatVal` operation and query on an edge grid, in 4 caller modes | it uses only `FloatVal`'s public API, so the same source builds against both trees |

`diffops` covers what no earlier oracle reaches, including
`proper_subset`, `fmod`, and the hyperbolic functions:

- 22 numbers: ±∞, ±largest normal, ±1e5, ±3, ±1, ±0.1, ±smallest normal,
  ±smallest denormal, ±0, 1+ulp, 2, 7, and NaN;
- 52 intervals: the 21 non-NaN singletons, 30 ranges (straddling or touching
  0, infinite and extreme bounds, [0,−0], denormal ranges, 1-ulp and tight
  intervals), and the NaN singleton;
- unary operations, `pow` with n from −3 to 5, `nroot` with k from 1 to 5,
  `size`, `med`, `tight`, `singleton`, `zero_in`, and the 14 MPFR functions;
- all binary operations, update operators, the 18 comparisons, `max`,
  `min`, the set operations, `hull`, `in`, and `fmod` for every interval
  pair and every interval-number pair in both orders;
- construction, `assign`, and both `hull`s from every number pair; the
  default value and the pi constants;
- each line prints the `%a` bounds, the truth value, or the exception type
  and message, and whether the caller's mode was kept.

The rounding probe, `bench.cpp`, DirectedRounding, MedianRoundingMode, and
the examples are unchanged.

## Measurements

### Builds [C3], [C17]

| build | HEAD s / warnings | native s / warnings |
|---|---|---|
| rel | 150 / 0 | 162 / 0 |
| rel-nompfr | 153 / 0 | 251 / 0 |
| dbg | 94 / 0 | 79 / 0 |
| dbg-nompfr | 89 / 0 | 84 / 0 |
| msvcsim rel | - | 146 / 0 |
| x86-64 GCC 15 | 744 / 17 | 739 / 17 |
| x86-64 Clang 21 | 673 / 0 | 668 / 0 |

The arm64 build times ran while other work loaded the machine and are not
comparable; the Float module timing below is. The 17 GCC warnings are the
same in both trees, and none is in float code: `-Wreturn-type` in four int
arithmetic headers, `-Wattributes` in `bool-expr.cpp`,
`-Wfree-nonheap-object` in `allocator.hpp`, and `-Wunused-result` in a
FlatZinc test.

### Header probes [C5]

- `diffops`: 404800 lines with MPFR and 381920 without. After masking the
  exception message, native equals HEAD on all lines at -O0 and at -O2, with
  and without MPFR. 14548 lines throw, all `std::runtime_error`, in both
  trees. No line leaves the caller's mode changed. -O0 and -O2 outputs are
  identical in each tree.
- `nth_root`: 240 rows at -O0 and -O2, 0 unsound and 0 with the mode
  changed in both trees; the outputs and the `-DPRINT_ALL` rows are
  identical.
- Comparison probe: identical apart from the `FloatVal(2,1)` message.
- pi: identical. The hexfloat literals equal Boost's `pi_d_l` and
  `pi_d_u`, their halves, and their doubles.
- Traits: HEAD's `FloatVal` is not trivially copyable, nor trivially copy
  constructible or assignable. The native `FloatVal` and `Float::Interval`
  are all three. Both are trivially destructible and standard layout, with
  size 16 and alignment 8.

### Rounding probe [C6]

All 9 arm64 configurations (AppleClang, LLVM 22, and GCC 14 at -O0, -O2, and
-O3) exit 0 with 0 unsound rows for both trees, and the 9 native TSV files
are byte-identical to HEAD's.

In the msvcsim copy, all 9 configurations also have 0 unsound rows. Each
file differs from HEAD's in 7 of 169 rows, all of them the probe's
informational `live_scope` rows, which report `upward_lost` instead of
`upward_kept`. These rows record whether the mode stays upward between
operations inside a live scope. Under the per-operation style it is not
meant to, as with Boost's `rounded_arith_std`. All bounds and all
mode-restored columns equal HEAD's.

### Suites [C7] to [C12]

| selection | build | HEAD | native |
|---|---|---|---|
| DirectedRounding, RoundingScope, MedianRoundingMode | rel, rel-nompfr, dbg, dbg-nompfr, msvcsim; x86-64 GCC and Clang | 3 of 3 passed on each of 6 builds | 3 of 3 passed on each of 7 builds |
| Float::, -threads 8 | rel | 1563 passed, 557 s | 1563 passed, 533 s |
| Float::, -threads 8 | rel-nompfr | 1425 passed, 491 s | 1425 passed, 504 s |
| Float:: check and standard | dbg | 529 passed, 273 s | 529 passed, 256 s |
| Float:: check and standard | dbg-nompfr | 508 passed, 332 s | 508 passed, 279 s |
| Float:: check and standard | msvcsim rel | - | 529 passed, 65 s |
| FlatZinc:: | rel | 152 passed | 152 passed |
| ctest | rel | 7 of 7 passed | 7 of 7 passed |
| Float::, -threads 4 | x86-64 GCC | 1563 passed, 455 s | 1563 passed, 457 s |
| Float:: check and standard, -threads 4 | x86-64 Clang | 529 passed, 47 s | 529 passed, 46 s |

Test counts per tag are equal in both trees [C8]: 1563 Float:: tests with
MPFR (9 check, 1034 sweep) and 1425 without (9 check, 917 sweep). The arm64
suite times are wall times while other work shared the machine, and say
nothing about runtime. The
native ctest run took 441 s instead of 34 s, because its
`build-gecode-test` step rebuilt the tree after the formatting step had
touched two headers; its `test` step took 20.4 s against HEAD's 18.5 s.

### Examples [C13], [C17]

On every build, each example's `-mode solution` output and `-mode stat`
statistics without the runtime line equal HEAD's: rel, rel-nompfr, dbg,
dbg-nompfr, and msvcsim on arm64, and GCC and Clang on x86-64. The
hexfloat-printing copies give byte-identical output against the head,
native, and msvcsim installs. The statistics also equal boost-003's HEAD
statistics:

| example | solutions | propagations | nodes | failures |
|---|---|---|---|---|
| archimedean-spiral | 189 | 118785 | 10252 | 1738 |
| cartesian-heart | 572 | 3556954 | 28112 | 4185 |
| descartes-folium | 390 | 1813641 | 19070 | 3343 |
| golden-spiral | 98 | 62384 | 5910 | 965 |

### Runtime [C15], [C18]

x86-64 (erebor, load average below 1.1 before and after each run), 11
interleaved rounds, ms:

| example | GCC HEAD min / median | GCC native | Clang HEAD | Clang native |
|---|---|---|---|---|
| archimedean-spiral | 261.7 / 264.8 | 261.9 / 265.3 | 262.9 / 266.1 | 262.4 / 265.2 |
| cartesian-heart | 211.1 / 212.2 | 209.0 / 210.3 | 231.2 / 232.1 | 221.6 / 225.1 |
| descartes-folium | 504.5 / 505.6 | 503.5 / 504.8 | 631.6 / 633.3 | 560.0 / 560.8 |
| golden-spiral | 189.5 / 190.7 | 190.0 / 192.5 | 190.3 / 191.4 | 190.4 / 191.4 |

Microbenchmark (`bench.cpp` at -O3, 10^7 iterations, 7 interleaved rounds),
ns per iteration, min / median:

| variant | `FloatVal` loop | `Rounding` loop | `fesetround` / `fegetround` calls in the assembly |
|---|---|---|---|
| GCC HEAD | 60.3 / 60.6 | 11.7 / 11.8 | 33 / 13 |
| GCC native | 60.2 / 60.5 | 11.7 / 11.7 | 30 / 11 |
| Clang HEAD | 69.0 / 69.2 | 11.7 / 11.7 | 17 / 7 |
| Clang native | 62.6 / 62.9 | 11.6 / 11.6 | 15 / 5 |

The `Rounding` loop uses only `RoundingBase` arithmetic, which the prototype
does not change, and its times are equal. The `FloatVal` loop and the
examples that use `FloatVal` arithmetic most are faster with Clang and equal
with GCC. The likely reasons are the fewer mode calls and passing a
trivially copyable `FloatVal`; the spike did not separate the two.

arm64 (indicative only; another session's CPU-load benchmark ran ten busy
loops, with load averages from 28 to 123 on 10 cores):

| example | HEAD min / median | native min / median |
|---|---|---|
| archimedean-spiral | 223.4 / 224.2 | 224.1 / 224.3 |
| cartesian-heart | 336.0 / 338.4 | 326.2 / 327.0 |
| descartes-folium | 1403.5 / 1407.1 | 1238.9 / 1242.2 |
| golden-spiral | 159.9 / 161.4 | 160.0 / 161.2 |

The microbenchmark spread too widely under that load to compare (the
`FloatVal` loop ranged from 222.6 to 314.4 ns for HEAD and 247.1 to 311.7
ns for native). Its arm64 assembly has 20 `fesetround` and 7 `fegetround`
calls for HEAD, and 18 and 5 for native. The example results point the same
way as x86-64 Clang: descartes-folium 12 % faster, cartesian-heart 3 %, the
spirals equal.

### Float module build time [C16], [C18]

x86-64, GCC 15, Ninja Release with MPFR, -j 4 on 4 cores. Each round
deletes the object files of `gecodefloat_shared` and rebuilds it; rounds
alternate between the trees [C18]:

| round | HEAD real / user s | native real / user s |
|---|---|---|
| 1 | 34.21 / 93.15 | 33.85 / 91.78 |
| 2 | 34.21 / 93.20 | 33.68 / 91.80 |
| 3 | 34.14 / 92.87 | 33.79 / 91.97 |

A one-line file that includes `gecode/float.hh` preprocesses to 179823
lines for HEAD and 177552 for native with GCC 15, and to 140788 and 138191
with AppleClang [C16]. Ten `-fsyntax-only` runs of it take 16.11 s for HEAD
and 16.12 s for native with GCC 15. The Float module builds about 1 %
faster; including the header costs the same.

The arm64 rebuild rounds are not reported. The first run of
`scripts/buildtime.sh` removed the whole object directory, which also holds
the Makefile rules, so its rebuilds failed; the script was fixed, but the
machine stayed loaded. Only its preprocessed line counts are used.

### Footprint and ABI [C19], [C20]

- Installed headers: 731 files and 177270 lines for HEAD, 636 and 167426 for
  native. HEAD installs 96 files with 10978 lines under
  `include/gecode/third-party`, including `LICENSE_1_0.txt`; native installs
  none. 40 installed HEAD headers mention `gecode_boost`, and no native
  header does.
- Exported symbols: 2185 in both trees across the 14 physical libraries,
  1895 distinct demangled names, all identical. No
  exported symbol names a `gecode_boost` or `Float::Interval` type, because
  `FloatVal` holds the implementation type as a member and its member
  functions are inline.
- Two exported functions take `FloatVal` by value:
  `Float::Linear::post(Home, Term*, int, FloatRelType, FloatVal)` and
  `FloatValArgs::create(int, FloatVal, int)`. One out-of-line function
  returns it by value, `LinFloatExpr::Node::fill(Home, Term*)`, and it is
  not exported. Their names do not change, but their calling convention
  does (see Wider impacts).

### Final validation [C27]

The main suites above ran on `src/native` before its last formatting pass,
which changed only whitespace and Doxygen comments in `interval.hpp` and
`rounding.hpp`. The final run rebuilt `bld/native-rel` from the formatted
sources and repeated the key checks:

- The rebuild finished with 0 warnings in 77 s, and the install succeeded.
- The rounding probe ran in all 9 configurations with 0 unsound rows, and
  every TSV equals HEAD's.
- The Float:: suite with MPFR passed 1563 of 1563 tests in 824 s, on the
  loaded machine.
- `src/native-msvcsim` was rebuilt from the same formatted files (0
  warnings, 28 s) before its probe and check-and-standard runs above.

## Differences from HEAD

Every difference that any measurement found:

1. **The empty-interval message.** 14548 `diffops` lines and one comparison
   probe line differ only in the text after the exception type [C5].
   Nothing in Gecode, its tests, or the FlatZinc tests reads the message.
2. **`FloatVal` is trivially copyable** [C5]. The value is the same; the
   calling convention for by-value parameters and returns changes (see Wider
   impacts).
3. **Fewer rounding-mode calls**: 2 or 3 `fesetround` and 2 `fegetround`
   calls fewer in the microbenchmark's assembly [C18]. The caller's mode is
   restored as before on every `diffops` line [C5].
4. **The msvcsim `live_scope` rows** report `upward_lost` [C6]. This is
   the intended per-operation style. The same rows at HEAD under MSVC would
   report the same, but HEAD cannot be built in that style on arm64 to show
   it.
5. **Runtime**: equal with GCC, faster with Clang [C18].
6. **The API changes** listed under Wider impacts.

No bound, truth value, exception type, caller mode, search statistic, or
test result differs.

## Wider impacts

### API

`FloatVal`'s public operations and the float constraint API are unchanged.
The diff of `gecode/float.hh` (167 lines) touches only these declarations
[C23]:

- `RoundingControl`, the Boost typedef, is gone.
- `RoundingBase` no longer derives from `rounded_arith_opp` or
  `rounded_arith_std`. What it inherited publicly from Boost is gone or
  protected:
  - `init`, `upward`, `downward`, and `to_nearest` become protected;
  - `toward_zero`, `get_rounding_mode`, `set_rounding_mode`, `to_int`,
    `conv_down`, `conv_up`, `force_rounding`, and the `rounding_mode`
    typedef are gone.
  - Gecode's own public members (the `*_down`/`*_up` arithmetic, the MPFR
    functions, `median`, `int_down`, `int_up`) are unchanged.
- `Rounding` is a Gecode class instead of `save_state<RoundingBase>`. It
  loses `save_state`'s public `mode` member and `unprotected_rounding`
  typedef, and it cannot be copied.
- `FloatVal::FloatValImpType` is `Float::Interval`, and the protected
  typedefs `R` and `P` are gone.
- `Float::Interval` and its free functions are new public names in
  `Gecode::Float`.

Source that copies a `Rounding`, calls the Boost mode members, or names the
Boost types stops compiling. The downstream search found no such code
outside copies of Gecode itself (see Downstream). The CMake consumer smoke
test, which uses `Float::Rounding`'s public arithmetic, passes unchanged
[C21].

### ABI

The library ABI changes even though the exported names do not:

- `FloatVal`'s layout (two doubles, 16 bytes) is unchanged.
- Being trivially copyable changes how `FloatVal` is passed and returned by
  value. On AArch64 it is a homogeneous aggregate of two doubles and travels
  in two floating-point registers; on SysV x86-64 it travels in two SSE
  registers. At HEAD, the user-provided copy constructor of Boost's interval
  forces both ABIs to pass it through memory by invisible reference. On MSVC
  x64 a 16-byte struct is passed by reference either way.
- So a program built against HEAD's headers that calls
  `Float::Linear::post` or `FloatValArgs::create` cannot run against a
  native library on AArch64 or SysV x86-64. The symbol names match, so the
  dynamic linker would not catch it.
- Gecode 7 is the breaking release, and the brief keeps the ABI number at 52
  until a release is prepared. The release must change the soname, as it
  will for the other Gecode 7 changes.
- The exported MPFR members of `RoundingBase` keep their names.

### value-params

Gecode passes `FloatVal` by `const&`, and that rule stays. A trivially
copyable `FloatVal` of 16 bytes would allow passing it by value in
registers on AArch64 and SysV x86-64, but not on MSVC x64. Whether to revisit
value-params-008 and its allowed-types list is the user's decision; the
prototype changes no signature.

### modern-classes

modern-classes-003 already defaulted `FloatVal`'s members, so its
triviality now depends only on the member type. A
`static_assert(std::is_trivially_copyable_v<FloatVal>)` could join the
trait checks in `test/array.cpp`. The `FloatVal` comparisons stay outside
`operator<=>`: they are "certainly" relations, not a total or partial order
that `<=>` could express.

### FlatZinc, Gist, examples, and tools

A grep of HEAD for `gecode_boost`, `GECODE_BOOST`, `third-party`,
`Float::Rounding`, `RoundingBase`, `FloatValImpType`, `LICENSE_1_0`, and
`interval_lib` outside the float headers finds only the build and
documentation files listed under CMake below, `test/float/arithmetic.cpp`,
`rounding.cpp` (Gecode's own `RoundingBase` members), and the consumer smoke
scripts' use of `Float::Rounding` [C25]. FlatZinc, Gist, the examples, and
the tools use only `FloatVal`. The FlatZinc tests and the examples give the
same results [C12], [C13].

### Downstream

- `gh search code` (read-only) for `gecode_boost`, `GECODE_BOOST`,
  `Float::Rounding`, `Gecode::Float::Rounding`, `RoundingBase gecode`,
  `FloatValImpType`, `gecode/third-party/boost`, and `interval_lib gecode`
  [C24]. The hits are:
  - vendored or forked copies of Gecode itself (for example
    `cp-profiler/gecode-profiling`, `lingmar/CT-Gecode`,
    `fzlinshu/DPSolver`, and several course repositories);
  - packaging that lists or patches the third-party headers: the FreeBSD
    port's `pkg-plist` (and its mirrors), a bioconda patch for Gecode 5.1.0,
    and a Debian patch in `LebedevRI/minizinc-meta`;
  - `Gecode/MPG`'s `release-reference-inventory.json`, which names
    `Gecode::Float::Rounding`;
  - unrelated projects with their own `Float::Rounding`.
  No project outside a copy of Gecode uses `RoundingBase`, `Rounding`'s
  Boost members, `FloatValImpType`, or `gecode_boost`.
- MiniZinc/libminizinc at 4a84a48ac8 (2026-11-02), shallow-cloned into
  `dl/` and only grepped: no match for any of these names; its Gecode solver
  uses `FloatVal` through the public API only [C24].
- Packagers who list installed files (FreeBSD's `pkg-plist`) must drop the
  96 third-party entries, and MPG's reference inventory should be updated
  for `Rounding`.

### MPFR

MPFR stays optional with the same `GECODE_ENABLE_MPFR` option. Without it,
`interval.hpp` compiles without the transcendental functions and `cos`, as
before. The nompfr builds, suites, probes, and examples pass [C5], [C9],
[C13], and the consumer smoke test passes in core mode [C21].

### MSVC and x86-64 code that only CI can test

- `_control87` mode control: 18 lines in `rounding.hpp` and the predicate
  block in `float.hh`. [C14] compiles them for x64 and x86 MSVC targets
  with `-fms-compatibility -fsyntax-only` against a stub `float.h` that
  declares `_control87` and the `_MCW_*`, `_RC_*`, `_EM_*`, `_PC_*`, and
  `_IC_*` constants from Microsoft's documentation. This checks the syntax
  and the constant names only.
- The per-operation arithmetic style: the msvcsim copy runs it on arm64
  with `<cfenv>` mode control. The rounding tests, the check and standard
  Float:: tags, the rounding probe, and the examples pass [C6], [C11],
  [C13]. What msvcsim does not cover is `_control87` itself and MSVC's code
  generation.
- MSVC ARM64 moves from Boost's `#error` to `<cfenv>`, untested.
- x86-64 GCC and Clang: erebor covers them with `<cfenv>` [C17].
- MinGW: the same `<cfenv>` path as x86-64 GCC; not run.

### CMake, documentation, and checks

The prototype removes:

- the `LICENSE_1_0.txt` install pattern (`CMakeLists.txt` line 1644);
- `third-party` from the doc-input filter (line 923).

boost-005 must also update, outside the prototype:

- `misc/check.py` lines 63, 118, and 127 (third-party exclusions);
- `.clang-tidy` line 39 (`ExcludeHeaderFilterRegex`);
- `doxygen/doxygen.conf.in` line 523 (exclude pattern);
- `docs/code-style.md` line 5 and `docs/breaking-7.md` lines 13 and 49,
  which mention third-party code and the Boost headers.

### License

The ported file keeps the Boost.Interval copyright lines and the full
BSL-1.0 text next to Gecode's MIT header. The port follows Boost's case
analyses line for line, so it is arguably a derivative work, and BSL-1.0
requires the notice in source copies. BSL-1.0 is permissive and compatible
with Gecode's MIT license, and it does not require the notice in binaries.
**Question for the user:** keep the notice in `interval.hpp` (and then
where to mention it in Gecode's license documentation), or decide otherwise
after legal review. The spike does not decide this.

### Risks

- The `_control87` code has never run. A CI run on MSVC x64, and x86 if it
  is supported, must pass the rounding probe and the float suite before
  boost-005 merges.
- Coverage: `diffops` is a grid, not a proof. It covers each case of
  every operation's case analysis with edge values, but not every bound
  combination.
- The exception message changes. Code that parses it would break; none was
  found.
- NaN: the port keeps Boost's handling, including NaN bounds that pass
  through unchecked results, and `diffops` checks NaN operands for every
  operation. NaN behaviour remains as unspecified as today.
- Source that copies a `Rounding` stops compiling, which is intended.
- The median relies on `barrier()` instead of a `volatile` store. The
  rounding probe, MedianRoundingMode, and the `nth_root` probe cover it at
  -O0 to -O3 with three compilers on arm64 and two on x86-64.

## Proposed boost-005 split

Measured sizes come from the prototype [C22], [C23].

1. **C1, interval core** (`interval.hpp`, 1042 lines, plus new tests). Add `gecode/float/interval.hpp` next to the Boost copy,
   unused by `FloatVal`. Add `Test::Base` tests: outward rounding of the four
   operators and `sqrt` on non-representable values, singletons, throwing
   empty construction, and the comparison truth table. Add a temporary
   in-binary differential test that runs the `diffops` grid against the
   Boost interval and compares bit for bit. This commit carries the license
   decision.
2. **C2, switch** (about 400 changed lines by diffstat). The rounding layer in
   `float.hh` and `rounding.hpp`, the `Rounding` scope, `FloatValImpType`,
   `val.hpp`, `num.hpp`, the RoundingScope adaptation, and the Doxygen text.
   The differential test now compares `FloatVal` with the Boost interval.
3. **C3, removal** (105 files and 11308 lines removed, plus about 10
   lines in build, check, and documentation files).
   Delete `gecode/third-party`, the CMake install and doc lines, the
   `check.py`, `.clang-tidy`, and Doxygen exclusions, and the documentation
   lines; delete the temporary differential test.
4. **C4, record.** The decision comment beside `Float::Interval`, and the
   7.0.0 changelog entries: the API changes above, the trivially copyable
   `FloatVal` and its calling convention, the message, and the license note.

Then boost-005's own final validation: the full Float:: suite including
sweeps, the rounding probe, the examples, and MSVC in CI.

## Commands

All commands run from W = `build/zdev-impl/boost-007` unless stated. Script
and probe sources are in Appendix A.

- **[C1] Setup.** From the repository root: `pwd`, `git branch
  --show-current`, `git rev-parse HEAD`, `git status --short`, the tool
  versions, `sysctl -n machdep.cpu.brand_string hw.ncpu`, `sw_vers`, and on
  erebor `uname -m`, `g++-15 --version`, `clang++-21 --version`, `nproc`.
  Output in `results/preconditions.txt`.
- **[C2] Scratch trees.** `git archive HEAD | tar -x -C src/head`, the
  same into `src/native`, then the prototype edits; `src/native-msvcsim` is
  a copy of `src/native` with the per-operation predicate replaced by
  `#if 1`.
- **[C3] Builds.** `scripts/build-head.sh > logs/build-head.log` and
  `scripts/build-all.sh > logs/build-all.log`; warnings are `grep -c
  'warning:'` per build log. Then `cmake --build bld/native-msvcsim-rel -j
  10` for its install. Caches: for each pair, `diff` of
  `bld/<tree>-<config>/CMakeCache.txt` without comments, `INTERNAL` entries,
  and tree paths, in `results/builds/cache-*.diff`; the only difference is
  `CMAKE_CXX_COMPILER`'s type in dbg-nompfr.
- **[C4] Installs.** `cmake --install bld/<build> --prefix $PWD/inst/<v>`
  for head-rel, head-rel-nompfr, native-rel, native-rel-nompfr, and
  native-msvcsim-rel.
- **[C5] Header probes.** `scripts/probes.sh head` and `scripts/probes.sh
  native`. Comparisons: `diff <(scripts/mask.sh
  results/probe/diffops-head<cfg><opt>.txt) <(scripts/mask.sh
  results/probe/diffops-native<cfg><opt>.txt)` for the four builds; `grep -c
  'CHANGED$'`; `diff` of the `nroot-*` outputs; `diff <(sed 's/ raw
  [TFX]\{6\} //' results/probe/cmp-head.txt | grep -v 'I(2,1)')
  results/probe/cmp-native.txt`; `diff` of the `pi-*` outputs; `cat` of
  `tc-*`.
- **[C6] Rounding probe.** `sh probe-head/run.sh`, `sh probe-native/run.sh`,
  and `sh probe-msvcsim/run.sh`, each with `install` linked to its
  `inst/<v>`; then `cmp probe-head/results/<f>.tsv
  probe-<v>/results/<f>.tsv` for the 9 files, and for msvcsim the row
  classification in `results/probe/rounding-msvcsim-cmp.txt`. The probe
  copies are unchanged: `rounding-probe.cpp` SHA-256
  0704233d2257ee2245664dedf9c8cd9dc494e846e000efbc93ae0364fce50390 and
  `run.sh` f94e29820cc7cbb9b47e3ba302957e00f344392f86032072b779f41090de42dc.
- **[C7] Rounding tests.** `scripts/suites.sh arith`.
- **[C8] Test counts.** `scripts/suites.sh counts`.
- **[C9] Release suites.** `scripts/suites.sh float`; `grep 'Test summary'
  results/suite/float-<build>.log`.
- **[C10] Debug suites.** `scripts/suites.sh debug`.
- **[C11] msvcsim suite.** `scripts/suites.sh msvcsim`.
- **[C12] FlatZinc and ctest.** `scripts/suites.sh flatzinc`; per-test times
  from `results/suite/ctest-<build>.log`.
- **[C13] Examples.** `scripts/examples.sh head-rel native-rel
  head-rel-nompfr native-rel-nompfr head-dbg native-dbg head-dbg-nompfr
  native-dbg-nompfr native-msvcsim-rel`, then `scripts/compare-examples.sh`
  with bases head-rel (variants native-rel and native-msvcsim-rel),
  head-rel-nompfr, head-dbg, head-dbg-nompfr, head (variants native and
  native-msvcsim, for the hexfloat output), and head-nompfr. The boost-003
  cross-check compares the statistics lines of
  `../boost-003/results/examples/<e>-head.stat` with `<e>-head-rel.stat`.
  The hexfloat copies in `examples-precise/` come from
  `build/nextafter-proto/scripts/make_precise.py` applied to
  `src/head/examples`.
- **[C14] MSVC syntax check.** `msvc/check.sh`, output in
  `results/msvc/check.txt`.
- **[C15] arm64 runtime.** `scripts/timing.sh 11 head-rel native-rel` and
  `scripts/bench.sh 7 head native`, summarized with `python3 -I
  scripts/timing-summary.py`. Uptime is recorded in the outputs.
- **[C16] arm64 preprocessed size.** `scripts/buildtime.sh 3 head native`
  (first version, output in `results/buildtime/buildtime-failed-loaded.txt`);
  only its `preprocessed` lines are used. Appendix A shows the fixed script,
  which deletes object files instead of the object directory.
- **[C17] erebor.** `tar -C src -c head native | ssh erebor-dev 'mkdir -p
  ~/boost-007/src && tar -x -C ~/boost-007/src'`; `erebor/erebor.sh` to
  `~/boost-007` and `erebor/{rounding-probe.cpp,diffops.cpp,run-probe-linux.sh}`
  to `~/boost-007/probe`; `ssh erebor-dev 'cd ~/boost-007 && setsid nohup sh
  erebor.sh > erebor.log 2>&1 < /dev/null &'`; after `EREBOR-DONE` in
  `results/status`, `ssh erebor-dev 'cd ~/boost-007 && tar -c results logs' |
  tar -x -C results/erebor`. Comparisons as in [C5], [C6], [C13] on the
  pulled files, and `python3 -I scripts/hexnorm.py` for x86-64 against arm64
  `diffops`, in `results/erebor/cross-platform.txt`.
- **[C18] erebor timing.** `erebor/timing.sh` copied to `~/boost-007` with
  `bench/bench.cpp`, started like [C17]; results pulled into
  `results/erebor/results/timing`, summarized with `scripts/timing-summary.py`.
- **[C19] Footprint.** For each install, `find inst/<v>/include -type f |
  wc -l`, the same through `cat | wc -l`, the same for
  `include/gecode/third-party`, `find inst/<v> -name LICENSE_1_0.txt`, and
  `grep -rl gecode_boost inst/<v>/include | wc -l`; for the trees, `find
  src/<t>/gecode/third-party -type f`. Output in
  `results/footprint/installed.txt`.
- **[C20] ABI.** `find inst/<v>/lib -name 'libgecode*.dylib' -type f`
  selects each of the 14 physical libraries once, skipping the version
  symlinks; `nm -gU` on each, without blank and file-name lines, counts the
  exported symbols. Their names through `c++filt` and `sort -u` go into
  `results/abi/exports-physical-<v>.txt`, compared with `diff`; the
  by-value parameters with `grep -E '[(,] ?Gecode::FloatVal[,)]'`; the
  by-value returns with a grep of `src/head/gecode` for out-of-line
  definitions returning `FloatVal`, checked against the exports.
- **[C21] Consumer smoke.** `RUNNER_TEMP=$PWD/tmp/smoke-native
  CXX=/Library/Developer/CommandLineTools/usr/bin/c++ bash
  src/native/.github/ci/cmake-consumer-smoke.sh $PWD/inst/native
  $PWD/inst/native/include mpfr`, and the same for `inst/native-nompfr` in
  `core` mode.
- **[C22] Source lines.** `python3 -I ../boost-003/scripts/sloc.py
  src/native/gecode/float/interval.hpp`, and the same on `unifdef
  -UGECODE_HAS_MPFR` of it; groups by function name from that output; the
  `FILE` totals of `sloc.py` on the four edited headers in both trees.
  Output in `results/size/`.
- **[C23] Patches.** `/usr/bin/diff -ruN src/head src/native >
  native.patch`, `/usr/bin/diff -ruN -x third-party src/head src/native >
  native-code.patch`, `diffstat -s`, and `shasum -a 256`; `diff -u` of the
  two `float.hh` files into `results/size/float-hh.diff`.
- **[C24] Downstream.** `gh search code "<q>" --limit 100` for each query,
  paced to stay under the rate limit, in
  `results/downstream/gh-search*.txt`; `git clone --depth 1
  https://github.com/MiniZinc/libminizinc.git dl/libminizinc` and `grep
  -rnE` for the names, in `results/downstream/`.
- **[C25] References.** From `src/head`: `grep -rnE
  'gecode_boost|GECODE_BOOST|third-party|Float::Rounding|RoundingBase|FloatValImpType|LICENSE_1_0|interval_lib'
  --exclude-dir=third-party .`, in `results/footprint/head-references.txt`;
  the same pattern without the type names in `src/native`.
- **[C26] Format and grep checks.** For each changed first-party file,
  `clang-format --style=file <f> | diff <f> -` with clang-format 22 from
  `src/native`; `grep -rnE 'gecode_boost|GECODE_BOOST|third-party/boost'
  --exclude-dir=.zdev src/native`; `test ! -e src/native/gecode/third-party`.
- **[C27] Final validation.** `cmake --build bld/native-rel -j 10`, `cmake
  --install bld/native-rel --prefix $PWD/inst/native`, `sh
  probe-native/run.sh` with the [C6] comparison, and
  `bld/native-rel/bin/gecode-test -test Float:: -stop false -threads 8`;
  from the repository root, `git status --short` and `git status --short
  --ignored .zdev`.

## Deviation from the plan

The plan asks for an empty `git status --short` at the end. That cannot
hold: `.zdev/boost/brief.md` is tracked, so the allowed index line shows as
` M .zdev/boost/brief.md`. The final check accepts that one line instead.
`native-spike.md` is new under the excluded `.zdev/` directory and shows
only with `--ignored`.

## Not verified

- Real MSVC builds and runs, x64 and x86. The `_control87` code is only
  syntax-checked against a stub header [C14], and the per-operation style
  only simulated on arm64 [C6], [C11]. **A CI run on a throwaway branch
  needs the user's authorization**; it should build both trees and run the
  rounding probe, `diffops`, and the float suite.
- MSVC ARM64 and MinGW: not built.
- Linux AArch64: not built; macOS arm64 covers the architecture, glibc's
  `fesetround` does not.
- The arm64 runtime and build times: the machine was loaded by another
  session's benchmark during the timing runs [C15], [C16]. The x86-64 runs
  on an idle machine are the runtime evidence.
- The full Debug sweep and the full Float:: suite with Clang on x86-64:
  only the check and standard tags ran there.
- Whether the Clang speedup comes from fewer mode calls or from the
  trivially copyable `FloatVal`.
- The license question (see Wider impacts).

## Appendix A: scripts and probes

### A.1 `scripts/build.sh`

```sh
#!/bin/sh
# Configure and build one scratch variant:
#   build.sh <src> <name> <Release|Debug> <ON|OFF mpfr> [targets...]
# Release uses Unix Makefiles, Debug uses Ninja; the compiler is build/local's.
# With no targets: the default target, then gecode-test. Run from boost-007.
set -e
src=$1; name=$2; type=$3; mpfr=$4; shift 4
gen="Unix Makefiles"; [ "$type" = Debug ] && gen=Ninja
cmake -S src/$src -B bld/$name -G "$gen" -DCMAKE_BUILD_TYPE=$type \
  -DCMAKE_CXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/c++ \
  -DGECODE_ENABLE_MPFR=$mpfr -DGECODE_ENABLE_GIST=OFF -DGECODE_ENABLE_QT=OFF \
  > logs/$name.configure.log 2>&1
start=$(date +%s)
if [ $# -eq 0 ]; then
  cmake --build bld/$name -j 10 > logs/$name.build.log 2>&1
  cmake --build bld/$name --target gecode-test -j 10 >> logs/$name.build.log 2>&1
else
  cmake --build bld/$name -j 10 --target "$@" > logs/$name.build.log 2>&1
fi
echo "$name built in $(( $(date +%s) - start )) s; warnings: $(grep -c 'warning:' logs/$name.build.log)"
```

### A.2 `scripts/build-head.sh`

```sh
#!/bin/sh
# HEAD reference builds, one after another. Run from boost-007.
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
scripts/build.sh head head-rel Release ON
scripts/build.sh head head-rel-nompfr Release OFF
scripts/build.sh head head-dbg Debug ON gecode-test $EX
# head-dbg-nompfr: rebuilt by build-all.sh (no spiral examples without MPFR)
echo HEAD-BUILDS-DONE
```

### A.3 `scripts/build-all.sh`

```sh
#!/bin/sh
# All scratch builds after build-head.sh, one after another. Run from boost-007.
# Release: default target plus gecode-test. Debug: gecode-test plus the float
# examples each configuration has (MPFR-free builds lack the two spirals).
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
EX_NOMPFR="cartesian-heart descartes-folium"
scripts/build.sh head head-dbg-nompfr Debug OFF gecode-test $EX_NOMPFR
scripts/build.sh native native-rel Release ON
scripts/build.sh native native-rel-nompfr Release OFF
scripts/build.sh native native-dbg Debug ON gecode-test $EX
scripts/build.sh native native-dbg-nompfr Debug OFF gecode-test $EX_NOMPFR
scripts/build.sh native-msvcsim native-msvcsim-rel Release ON gecode-test $EX
echo ALL-DONE
```

### A.4 `scripts/probes.sh`

```sh
#!/bin/sh
# Header-level probes against installs: probes.sh <variant>...
# For each variant v: diffops (inst/v and inst/v-nompfr, -O0 and -O2), nroot
# (-O0, -O2, and -DPRINT_ALL at -O2), the comparison probe (cmp.cpp for head,
# cmp-native.cpp otherwise), tc, and pi. Outputs go to results/probe.
# Run from boost-007.
CXX="/usr/bin/clang++ -std=c++23"
MPFR="-I /opt/homebrew/include -L /opt/homebrew/lib -lmpfr -lgmp"
O=results/probe
mkdir -p $O
link() { echo "-L $1/lib -lgecodefloat -lgecodekernel -lgecodesupport -Wl,-rpath,$PWD/$1/lib"; }
for v in "$@"; do
  for cfg in "" -nompfr; do
    inst=inst/$v$cfg
    for opt in -O0 -O2; do
      $CXX $opt -I $inst/include probe/diffops.cpp $(link $inst) $MPFR -o $O/diffops-$v$cfg$opt || exit 1
      timeout 1200 $O/diffops-$v$cfg$opt > $O/diffops-$v$cfg$opt.txt; echo "diffops-$v$cfg$opt rc=$? lines=$(wc -l < $O/diffops-$v$cfg$opt.txt)"
    done
  done
  inst=inst/$v
  for opt in -O0 -O2; do
    $CXX $opt -I $inst/include probe/nroot.cpp $MPFR -o $O/nroot-$v$opt || exit 1
    $O/nroot-$v$opt > $O/nroot-$v$opt.txt; echo "nroot-$v$opt rc=$? $(tail -1 $O/nroot-$v$opt.txt)"
  done
  $CXX -O2 -DPRINT_ALL -I $inst/include probe/nroot.cpp $MPFR -o $O/nroot-all-$v || exit 1
  $O/nroot-all-$v > /dev/null 2> $O/nroot-all-$v.txt
  src=probe/cmp-native.cpp; [ $v = head ] && src=probe/cmp.cpp
  $CXX -O2 -I $inst/include $src $MPFR -o $O/cmp-$v || exit 1
  $O/cmp-$v > $O/cmp-$v.txt
  for p in tc pi; do
    $CXX -O2 -I $inst/include probe/$p.cpp $(link $inst) $MPFR -o $O/$p-$v || exit 1
    $O/$p-$v > $O/$p-$v.txt
  done
done
echo PROBES-DONE
```

### A.5 `scripts/mask.sh`

```sh
#!/bin/sh
# Print a diffops output with each exception message replaced by MSG: mask.sh <file>
sed -E 's/(throw [^:]*): .* (kept|CHANGED)$/\1: MSG \2/' "$1"
```

### A.6 `scripts/suites.sh`

```sh
#!/bin/sh
# Test suites, one at a time: suites.sh <phase>...
#   float: the full Float:: selection on the four Release builds
#   debug: Float:: check and standard tags on the four Debug builds
#   arith: the three rounding tests on every build
#   msvcsim: Float:: check and standard tags on native-msvcsim-rel
#   flatzinc: ctest and FlatZinc:: on head-rel and native-rel
#   counts: the Float:: test count per Release build, by tag
# Logs go to results/suite/<phase>-<build>.log. Run from boost-007.
O=results/suite
mkdir -p $O
run() {
  phase=$1; b=$2; shift 2
  start=$(date +%s)
  bld/$b/bin/gecode-test "$@" > $O/$phase-$b.log 2>&1
  rc=$?
  echo "$phase $b rc=$rc seconds=$(( $(date +%s) - start ))"
}
for phase in "$@"; do
  case $phase in
    float)
      for b in head-rel native-rel head-rel-nompfr native-rel-nompfr; do
        run float $b -test Float:: -stop false -threads 8
      done ;;
    debug)
      for b in head-dbg native-dbg head-dbg-nompfr native-dbg-nompfr; do
        run debug $b -test Float:: -tag check -tag standard -stop false -threads 8
      done ;;
    arith)
      for b in head-rel native-rel head-rel-nompfr native-rel-nompfr head-dbg native-dbg head-dbg-nompfr \
               native-dbg-nompfr native-msvcsim-rel; do
        run arith $b -test Float::Arithmetic::DirectedRounding -test Float::Arithmetic::RoundingScope \
          -test Float::Arithmetic::MedianRoundingMode -stop false
      done ;;
    msvcsim)
      run msvcsim native-msvcsim-rel -test Float:: -tag check -tag standard -stop false -threads 8 ;;
    flatzinc)
      for b in head-rel native-rel; do
        run flatzinc $b -test FlatZinc:: -stop false -threads 8
        start=$(date +%s)
        ctest --test-dir bld/$b -j 8 > $O/ctest-$b.log 2>&1
        echo "ctest $b rc=$? seconds=$(( $(date +%s) - start ))"
      done ;;
    counts)
      for b in head-rel native-rel head-rel-nompfr native-rel-nompfr; do
        bld/$b/bin/gecode-test -list-with-tags > $O/list-$b.txt 2>&1
        echo "counts $b float=$(grep -c '^Float::' $O/list-$b.txt)" \
          "check=$(grep '^Float::' $O/list-$b.txt | grep -c check)" \
          "sweep=$(grep '^Float::' $O/list-$b.txt | grep -c sweep)"
      done ;;
  esac
done
echo SUITES-DONE
```

### A.7 `scripts/examples.sh`

```sh
#!/bin/sh
# Example solutions and statistics: examples.sh <build>...
# For each build b in bld/: every float example it has, with -mode solution and
# -mode stat. Then the precise-print copies (examples-precise, from
# nextafter-proto's make_precise.py) built against each install in inst/ with
# build/local's flags, run with -mode solution; MPFR-free installs skip the
# two spirals. Run from boost-007.
SDK=$(xcrun --show-sdk-path)
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
R=results/examples
mkdir -p $R examples-precise/bin
for b in "$@"; do
  for e in $EX; do
    [ -x bld/$b/bin/$e ] || continue
    bld/$b/bin/$e -mode solution > $R/$e-$b.solution 2>&1
    bld/$b/bin/$e -mode stat > $R/$e-$b.stat 2>&1
  done
done
for v in $(ls inst); do
  P=$PWD/inst/$v
  libs="-lgecodedriver -lgecodeminimodel -lgecodeset -lgecodefloat -lgecodesearch -lgecodeinstance -lgecodeint"
  for e in $EX; do
    case $v in *-nompfr) case $e in *spiral) continue;; esac;; esac
    /Library/Developer/CommandLineTools/usr/bin/c++ -O3 -DNDEBUG -std=c++23 -isysroot $SDK \
      -I $P/include -isystem /opt/homebrew/include examples-precise/$e.cpp -o examples-precise/bin/$e-$v \
      -L $P/lib $libs -lgecodekernel -lgecodesupport -L /opt/homebrew/lib -lmpfr -lgmp -Wl,-rpath,$P/lib || exit 1
    examples-precise/bin/$e-$v -mode solution > $R/$e-$v.precise 2>&1
  done
done
echo EXAMPLES-DONE
```

### A.8 `scripts/compare-examples.sh`

```sh
#!/bin/bash
# Compare example outputs with HEAD's, without runtime lines:
#   compare-examples.sh <base> <variant>...
# Compares <e>-<variant>.{solution,stat,precise} with <e>-<base>.* in
# results/examples, for every file the base has. Run from boost-007.
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
R=results/examples
base=$1; shift
for v in "$@"; do
  for e in $EX; do
    for k in solution stat precise; do
      a=$R/$e-$base.$k; b=$R/$e-$v.$k
      [ -e $a ] || continue
      if [ ! -e $b ]; then echo "$v $e $k MISSING"; continue; fi
      n=$(diff <(grep -v 'runtime:' $a) <(grep -v 'runtime:' $b) | grep -c '^[<>]')
      echo "$v $e $k differing_lines=$n"
    done
  done
done
for e in $EX; do
  [ -e $R/$e-$base.stat ] || continue
  printf '%s %s: ' $e $base; grep -E 'solutions|propagations|nodes|failures' $R/$e-$base.stat | tr -s ' \t' ' ' | tr '\n' ';'; echo
done
```

### A.9 `scripts/timing.sh`

```sh
#!/bin/sh
# Example runtime: timing.sh <rounds> <build>...
# Runs every float example of each build with -mode stat, interleaving the
# builds within each round, and prints one line per run with the runtime in
# ms from the example's own statistics. Prints uptime before and after.
# Run from boost-007 on a quiet machine.
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
rounds=$1; shift
echo "uptime-before $(uptime)"
i=1
while [ $i -le $rounds ]; do
  for e in $EX; do
    for b in "$@"; do
      [ -x bld/$b/bin/$e ] || continue
      ms=$(bld/$b/bin/$e -mode stat 2>&1 | sed -n 's/.*runtime:[^(]*(\([0-9.]*\) ms).*/\1/p')
      echo "round=$i example=$e build=$b ms=$ms"
    done
  done
  i=$((i + 1))
done
echo "uptime-after $(uptime)"
```

### A.10 `scripts/bench.sh`

```sh
#!/bin/sh
# Microbenchmark: bench.sh <rounds> <variant>...
# Builds bench/bench.cpp at -O3 against each install in inst/, writes its
# assembly, counts fesetround and fegetround calls in it, then runs
# bench-<v> 10000000 in interleaved rounds. Run from boost-007.
SDK=$(xcrun --show-sdk-path)
CXX=/Library/Developer/CommandLineTools/usr/bin/c++
rounds=$1; shift
for v in "$@"; do
  P=$PWD/inst/$v
  FLAGS="-O3 -DNDEBUG -std=c++23 -arch arm64 -isysroot $SDK -I $P/include -isystem /opt/homebrew/include"
  $CXX $FLAGS bench/bench.cpp -o bench/bench-$v -L $P/lib -lgecodefloat -lgecodekernel -lgecodesupport \
    -L /opt/homebrew/lib -lmpfr -lgmp -Wl,-rpath,$P/lib || exit 1
  $CXX $FLAGS -S bench/bench.cpp -o bench/bench-$v.s || exit 1
  echo "asm $v fesetround=$(grep -c 'bl.*_fesetround' bench/bench-$v.s) fegetround=$(grep -c 'bl.*_fegetround' bench/bench-$v.s)"
done
echo "uptime-before $(uptime)"
i=1
while [ $i -le $rounds ]; do
  for v in "$@"; do
    echo "round=$i variant=$v $(bench/bench-$v 10000000)"
  done
  i=$((i + 1))
done
echo "uptime-after $(uptime)"
```

### A.11 `scripts/buildtime.sh`

```sh
#!/bin/sh
# Float module build time: buildtime.sh <rounds> <src>...
# For each source tree s: a fresh Release MPFR configure in bld/time-<s> and a
# build of support and kernel. Then, interleaved per round, removes the
# gecodefloat_shared object directory and times a rebuild of that target.
# Finally the preprocessed size of a one-line TU including gecode/float.hh
# and 10 timed -fsyntax-only runs of it. Run from boost-007.
SDK=$(xcrun --show-sdk-path)
CXX=/Library/Developer/CommandLineTools/usr/bin/c++
rounds=$1; shift
for s in "$@"; do
  rm -rf bld/time-$s
  cmake -S src/$s -B bld/time-$s -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=$CXX \
    -DGECODE_ENABLE_GIST=OFF -DGECODE_ENABLE_QT=OFF -DGECODE_ENABLE_MPFR=ON > logs/time-$s.configure.log 2>&1 || exit 1
  cmake --build bld/time-$s --target gecodefloat_shared -j 10 > logs/time-$s.base.log 2>&1 || exit 1
done
i=1
while [ $i -le $rounds ]; do
  for s in "$@"; do
    find bld/time-$s/CMakeFiles/gecodefloat_shared.dir -name '*.o' -delete
    t=$( { /usr/bin/time -p cmake --build bld/time-$s --target gecodefloat_shared -j 10 > logs/time-$s.float.log; } 2>&1 )
    echo "round=$i src=$s $(echo "$t" | tr '\n' ' ')"
  done
  i=$((i + 1))
done
for s in "$@"; do
  I=bld/time-$s
  echo '#include <gecode/float.hh>' > $I/tu.cpp
  FLAGS="-std=c++23 -O2 -DNDEBUG -isysroot $SDK -I src/$s -I $I -isystem /opt/homebrew/include"
  echo "preprocessed $s lines=$($CXX $FLAGS -E $I/tu.cpp | wc -l | tr -d ' ')"
  start=$(python3 -I -c 'import time; print(time.time())')
  j=0
  while [ $j -lt 10 ]; do $CXX $FLAGS -fsyntax-only $I/tu.cpp || exit 1; j=$((j + 1)); done
  echo "syntax-only $s ten_runs_seconds=$(python3 -I -c "import time; print(round(time.time() - $start, 3))")"
done
```

### A.12 `scripts/timing-summary.py`

```python
"""Summarize timing.sh, bench.sh, and erebor timing output.

Usage: python3 -I timing-summary.py <file>...

For "round=... example=E build=B ms=T" lines, prints the minimum and median
ms per example and build. For "round=... variant=V floatval A ns/iter,
rounding B ns/iter" lines, prints the minimum and median of both per variant.
"""

import collections
import re
import statistics
import sys

EXAMPLE = re.compile(r"round=\d+ example=(\S+) build=(\S+) ms=([\d.]+)")
BENCH = re.compile(r"round=\d+ variant=(\S+) floatval ([\d.]+) ns/iter, rounding ([\d.]+) ns/iter")


def describe(values: list[float]) -> str:
    return f"n={len(values)} min={min(values):.1f} median={statistics.median(values):.1f}"


def main() -> None:
    examples = collections.defaultdict(list)
    floatval = collections.defaultdict(list)
    rounding = collections.defaultdict(list)
    for name in sys.argv[1:]:
        with open(name) as f:
            for line in f:
                if m := EXAMPLE.match(line):
                    examples[(m[1], m[2])].append(float(m[3]))
                elif m := BENCH.match(line):
                    floatval[m[1]].append(float(m[2]))
                    rounding[m[1]].append(float(m[3]))
    for (example, build), values in sorted(examples.items()):
        print(f"example {example} {build} ms {describe(values)}")
    for variant in sorted(floatval):
        print(f"bench {variant} floatval_ns {describe(floatval[variant])} rounding_ns {describe(rounding[variant])}")


main()
```

### A.13 `scripts/hexnorm.py`

```python
"""Compare two diffops outputs up to the printf format of their numbers.

Usage: python3 -I hexnorm.py <a> <b>

glibc and Apple's libc print denormals differently with %a (0x0.0000000000001p-1022
against 0x1p-1074) and may differ in the sign they print for NaN. Each hexadecimal
number is replaced by Python's repr of its value and "-nan" by "nan" before the
lines are compared. Prints the line counts and the number of differing lines.
"""

import re
import sys

NUMBER = re.compile(r"-?0x[0-9a-fA-F.]+p[+-]?\d+")


def normalise(line: str) -> str:
    return NUMBER.sub(lambda m: repr(float.fromhex(m.group(0))), line).replace("-nan", "nan")


def main() -> None:
    with open(sys.argv[1]) as f:
        a = f.read().splitlines()
    with open(sys.argv[2]) as f:
        b = f.read().splitlines()
    differing = sum(1 for x, y in zip(a, b) if normalise(x) != normalise(y)) + abs(len(a) - len(b))
    print(f"lines {len(a)} {len(b)} differing_after_normalising {differing}")


main()
```

### A.14 `scripts/appendix.sh`

```sh
#!/bin/sh
# Print Appendix A of native-spike.md: every script and probe source, each
# under a numbered heading in a fenced block. cmp-native.cpp is shown as a
# diff against boost-003's cmp.cpp. Run from boost-007.
n=0
section() {
  n=$((n + 1))
  printf '### A.%d `%s`\n\n' $n "$1"
  printf '```%s\n' "$2"
}
for f in scripts/build.sh scripts/build-head.sh scripts/build-all.sh scripts/probes.sh scripts/mask.sh \
         scripts/suites.sh scripts/examples.sh scripts/compare-examples.sh scripts/timing.sh scripts/bench.sh \
         scripts/buildtime.sh scripts/timing-summary.py scripts/hexnorm.py scripts/appendix.sh msvc/check.sh \
         msvc/stub/float.h erebor/erebor.sh erebor/run-probe-linux.sh erebor/timing.sh probe/diffops.cpp \
         probe/tc.cpp probe/pi.cpp; do
  case $f in *.py) lang=python;; *.cpp|*.h) lang=cpp;; *) lang=sh;; esac
  section "$f" $lang
  cat "$f"
  printf '```\n\n'
done
section "probe/cmp-native.cpp (diff against probe/cmp.cpp)" diff
/usr/bin/diff -u probe/cmp.cpp probe/cmp-native.cpp
printf '```\n'
```

### A.15 `msvc/check.sh`

```sh
#!/bin/sh
# Syntax-only check of the _control87 branch (boost-007). Builds msvc/tu.cpp
# from the prototype's predicate block in float.hh and the
# GECODE_FLOAT_MODE_CONTROL87 branch of rounding.hpp, copied verbatim, around a
# stub RoundingBase with the same mode-control declarations. Compiles it for
# cl.exe-like x64 and x86 targets (clang with __x86_64__ undefined), then
# reports which predicates each MSVC-ABI target selects. Run from boost-007.
S=src/native/gecode
predicates() {
  sed -n '/^#if defined(_MSC_VER) && !defined(__x86_64__)/,/^#endif/p' $S/float.hh
  sed -n '/^#if defined(_MSC_VER) && (defined(_M_X64)/,/^#endif/p' $S/float.hh
}
control87_branch() {
  awk '/^#ifdef GECODE_FLOAT_MODE_CONTROL87/ { on = 1; depth = 1; next }
       on && /^#if/ { depth++ }
       on && /^#endif/ { depth-- }
       on && depth == 1 && /^#else/ { exit }
       on { print }' $S/float/rounding.hpp
}
{
  echo '#define forceinline inline'
  predicates
  echo '#ifndef GECODE_FLOAT_MODE_CONTROL87'
  echo '#error GECODE_FLOAT_MODE_CONTROL87 not selected'
  echo '#endif'
  echo 'namespace Gecode { namespace Float {'
  echo '    class RoundingBase {'
  echo '    public:'
  echo '        typedef unsigned int Mode;'
  echo '        static Mode current_mode(void);'
  echo '        static void restore_mode(Mode m);'
  echo '        static void upward(void);'
  echo '        static void downward(void);'
  echo '        static void to_nearest(void);'
  echo '    };'
  control87_branch
  echo '}}'
  echo 'unsigned int probe_modes(void) {'
  echo '    using Gecode::Float::RoundingBase;'
  echo '    const RoundingBase::Mode m = RoundingBase::current_mode();'
  echo '    RoundingBase::upward(); RoundingBase::downward(); RoundingBase::to_nearest();'
  echo '    RoundingBase::restore_mode(m);'
  echo '    return m;'
  echo '}'
} > msvc/tu.cpp
MS="-fms-compatibility -fmsc-version=1940"
for t in x86_64-pc-windows-msvc i686-pc-windows-msvc; do
  /usr/bin/clang++ --target=$t $MS -U__x86_64__ -std=c++23 -fsyntax-only -isystem msvc/stub -Wall -Wextra msvc/tu.cpp \
    && echo "syntax $t (__x86_64__ undefined, cl.exe-like): ok"
done
for t in x86_64-pc-windows-msvc i686-pc-windows-msvc aarch64-pc-windows-msvc x86_64-w64-windows-gnu; do
  sel=$( { predicates
           echo '#ifdef GECODE_FLOAT_MODE_CONTROL87'; echo 'selected MODE_CONTROL87'; echo '#endif'
           echo '#ifdef GECODE_FLOAT_DIRECTED_PER_OPERATION'; echo 'selected DIRECTED_PER_OPERATION'; echo '#endif'
         } | /usr/bin/clang++ --target=$t $MS -isystem msvc/stub -E -P -x c++ - 2>&1 | grep -E 'selected|error' | tr '\n' ' ')
  echo "predicates $t (clang default macros): ${sel:-none}"
done
```

### A.16 `msvc/stub/float.h`

```cpp
/* Stub of the MSVC <float.h> control-word interface for a syntax-only check
 * (boost-007). Values from the Microsoft documentation of _control87. */
#pragma once
#define _MCW_EM 0x0008001Fu
#define _EM_INEXACT 0x00000001u
#define _EM_UNDERFLOW 0x00000002u
#define _EM_OVERFLOW 0x00000004u
#define _EM_ZERODIVIDE 0x00000008u
#define _EM_INVALID 0x00000010u
#define _EM_DENORMAL 0x00080000u
#define _MCW_RC 0x00000300u
#define _RC_NEAR 0x00000000u
#define _RC_DOWN 0x00000100u
#define _RC_UP 0x00000200u
#define _RC_CHOP 0x00000300u
#define _MCW_PC 0x00030000u
#define _PC_64 0x00000000u
#define _PC_53 0x00010000u
#define _PC_24 0x00020000u
#define _MCW_IC 0x00040000u
#define _IC_AFFINE 0x00040000u
#define _IC_PROJECTIVE 0x00000000u
extern "C" unsigned int __cdecl _control87(unsigned int new_value, unsigned int mask);
```

### A.17 `erebor/erebor.sh`

```sh
#!/bin/sh
# x86-64 runs on erebor-dev, from ~/boost-007 after src/head and src/native
# are unpacked there. Started detached; writes results/ and logs/, and
# results/status with one line per finished step.
cd ~/boost-007
export LC_ALL=C
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
ARITH="-test Float::Arithmetic::DirectedRounding -test Float::Arithmetic::RoundingScope -test Float::Arithmetic::MedianRoundingMode"
mkdir -p bld inst logs results/examples results/suite results/probe
S=results/status
uname -a > results/machine.txt; nproc >> results/machine.txt
g++-15 --version | head -1 >> results/machine.txt; clang++-21 --version | head -1 >> results/machine.txt
for cc in gcc clang; do
  case $cc in gcc) CXX=g++-15;; clang) CXX=clang++-21;; esac
  for v in head native; do
    b=$v-$cc
    cmake -S src/$v -B bld/$b -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=$CXX \
      -DGECODE_ENABLE_MPFR=ON -DGECODE_ENABLE_GIST=OFF -DGECODE_ENABLE_QT=OFF > logs/$b.configure.log 2>&1
    start=$(date +%s)
    cmake --build bld/$b -j 4 --target gecode-test $EX > logs/$b.build.log 2>&1
    rc=$?
    cmake --build bld/$b -j 4 >> logs/$b.build.log 2>&1
    cmake --install bld/$b --prefix $PWD/inst/$b > logs/$b.install.log 2>&1
    echo "build $b rc=$rc $(( $(date +%s) - start )) s warnings $(grep -c 'warning:' logs/$b.build.log)" >> $S
    bld/$b/bin/gecode-test $ARITH > results/suite/$b-arith.log 2>&1; echo "exit $?" >> results/suite/$b-arith.log
    for e in $EX; do
      bld/$b/bin/$e -mode solution > results/examples/$e-$b.solution 2>&1
      bld/$b/bin/$e -mode stat > results/examples/$e-$b.stat 2>&1
    done
    sh probe/run-probe-linux.sh $PWD/inst/$b $PWD/results/probe/rounding-$b > results/probe/rounding-$b.log 2>&1
    $CXX -std=c++23 -O2 -I inst/$b/include probe/diffops.cpp -o results/probe/diffops-$b \
      -L inst/$b/lib -lgecodefloat -lgecodekernel -lgecodesupport -lmpfr -lgmp -Wl,-rpath,$PWD/inst/$b/lib
    timeout 1800 results/probe/diffops-$b > results/probe/diffops-$b.txt; echo "diffops $b rc=$?" >> $S
  done
done
for b in head-gcc native-gcc; do
  /usr/bin/time -p bld/$b/bin/gecode-test -test Float:: -stop false -threads 4 > results/suite/$b-t4.log 2>&1
  echo "exit $?" >> results/suite/$b-t4.log; echo "suite $b done" >> $S
done
for b in head-clang native-clang; do
  /usr/bin/time -p bld/$b/bin/gecode-test -test Float:: -tag check -tag standard -stop false -threads 4 \
    > results/suite/$b-std.log 2>&1
  echo "exit $?" >> results/suite/$b-std.log; echo "suite $b done" >> $S
done
echo EREBOR-DONE >> $S
```

### A.18 `erebor/run-probe-linux.sh`

```sh
#!/bin/sh
# Linux version of build/rounding-probe/run.sh (same probe source, operands,
# and levels): run-probe-linux.sh <install> <out-dir>, for g++ and clang++ at
# -O0, -O2, and -O3. Writes <out-dir>/<compiler><level>.{tsv,exit}.
set -eu
cd "$(dirname "$0")"
INST=$1; OUT=$2
OPERANDS="0.1 0.2 -0.1 -0.2 0.1 0.7 -0.1 0.2 0.1 0.3 -0.1 0.3 1.0 3.0 -1.0 3.0 2.0 0.0 0.1 0.0"
mkdir -p $OUT/bin
for compiler in gcc15 clang21; do
  case $compiler in gcc15) CXX=g++-15;; clang21) CXX=clang++-21;; esac
  for level in -O0 -O2 -O3; do
    name=$compiler$level
    $CXX -std=c++23 $level -I $INST/include rounding-probe.cpp -o $OUT/bin/probe-$name \
      -L $INST/lib -lgecodefloat -lgecodekernel -lgecodesupport -lmpfr -lgmp -Wl,-rpath,$INST/lib
    set +e
    $OUT/bin/probe-$name $compiler $level $OPERANDS > $OUT/$name.tsv
    echo $? > $OUT/$name.exit
    set -e
    echo "$name: exit $(cat $OUT/$name.exit), $(grep -c unsound $OUT/$name.tsv || true) unsound rows"
  done
done
```

### A.19 `erebor/timing.sh`

```sh
#!/bin/sh
# x86-64 timing on erebor-dev, from ~/boost-007 after erebor.sh. Started
# detached; writes results/timing/ and appends to results/status.
#   - examples: -mode stat in 11 interleaved rounds per compiler, head and
#     native, runtime in ms from the example's statistics;
#   - bench.cpp at -O3 against inst/<v>-<cc>, with fesetround and fegetround
#     call counts in its assembly, then 7 interleaved rounds of 10000000;
#   - Float module build time: a fresh Release MPFR configure per variant
#     with GCC 15, a build of gecodefloat_shared, then 3 interleaved rounds of
#     deleting its object files and timing its rebuild with -j 4; then the
#     preprocessed size of a one-line TU including gecode/float.hh and 10
#     timed -fsyntax-only runs of it.
cd ~/boost-007
export LC_ALL=C
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
R=results/timing
S=results/status
mkdir -p $R
{
  echo "uptime-before $(uptime)"
  for i in 1 2 3 4 5 6 7 8 9 10 11; do
    for cc in gcc clang; do
      for e in $EX; do
        for v in head native; do
          ms=$(bld/$v-$cc/bin/$e -mode stat 2>&1 | sed -n 's/.*runtime:[^(]*(\([0-9.]*\) ms).*/\1/p')
          echo "round=$i example=$e build=$v-$cc ms=$ms"
        done
      done
    done
  done
  echo "uptime-after $(uptime)"
} > $R/examples.txt 2>&1
echo "timing examples done" >> $S
{
  for cc in gcc clang; do
    case $cc in gcc) CXX=g++-15;; clang) CXX=clang++-21;; esac
    for v in head native; do
      P=$PWD/inst/$v-$cc
      $CXX -O3 -DNDEBUG -std=c++23 -I $P/include bench/bench.cpp -o $R/bench-$v-$cc -L $P/lib \
        -lgecodefloat -lgecodekernel -lgecodesupport -lmpfr -lgmp -Wl,-rpath,$P/lib
      $CXX -O3 -DNDEBUG -std=c++23 -I $P/include -S bench/bench.cpp -o $R/bench-$v-$cc.s
      echo "asm $v-$cc fesetround=$(grep -c 'call.*fesetround' $R/bench-$v-$cc.s)" \
        "fegetround=$(grep -c 'call.*fegetround' $R/bench-$v-$cc.s)"
    done
  done
  echo "uptime-before $(uptime)"
  for i in 1 2 3 4 5 6 7; do
    for cc in gcc clang; do
      for v in head native; do
        echo "round=$i variant=$v-$cc $($R/bench-$v-$cc 10000000)"
      done
    done
  done
  echo "uptime-after $(uptime)"
} > $R/bench.txt 2>&1
echo "timing bench done" >> $S
{
  for v in head native; do
    rm -rf bld/time-$v
    cmake -S src/$v -B bld/time-$v -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-15 \
      -DGECODE_ENABLE_MPFR=ON -DGECODE_ENABLE_GIST=OFF -DGECODE_ENABLE_QT=OFF > logs/time-$v.configure.log 2>&1
    cmake --build bld/time-$v --target gecodefloat_shared -j 4 > logs/time-$v.base.log 2>&1
  done
  echo "uptime-before $(uptime)"
  for i in 1 2 3; do
    for v in head native; do
      find bld/time-$v/CMakeFiles/gecodefloat_shared.dir -name '*.o' -delete
      t=$( { /usr/bin/time -p cmake --build bld/time-$v --target gecodefloat_shared -j 4 > logs/time-$v.float.log; } 2>&1 )
      echo "round=$i src=$v $(echo "$t" | tr '\n' ' ')"
    done
  done
  for v in head native; do
    I=bld/time-$v
    echo '#include <gecode/float.hh>' > $I/tu.cpp
    FLAGS="-std=c++23 -O3 -DNDEBUG -I src/$v -I $I"
    echo "preprocessed $v lines=$(g++-15 $FLAGS -E $I/tu.cpp | wc -l)"
    start=$(date +%s.%N)
    for j in 1 2 3 4 5 6 7 8 9 10; do g++-15 $FLAGS -fsyntax-only $I/tu.cpp; done
    echo "syntax-only $v ten_runs_seconds=$(echo "$(date +%s.%N) - $start" | bc)"
  done
  echo "uptime-after $(uptime)"
} > $R/buildtime.txt 2>&1
echo "timing buildtime done" >> $S
echo TIMING-DONE >> $S
```

### A.20 `probe/diffops.cpp`

```cpp
// Differential probe of every FloatVal operation (boost-007).
// Prints one line per caller mode, operation, and operands: the result as
// hexadecimal bounds, a truth value, or "throw <type>: <message>", and
// "kept" or "CHANGED" for the caller's rounding mode afterwards. Built
// against HEAD and against the native prototype, the outputs are compared
// after masking the exception message. Uses only FloatVal's public API.
#include <gecode/float.hh>

#include <cfenv>
#include <cmath>
#include <cstdio>
#include <exception>
#include <limits>
#include <string>
#include <typeinfo>
#include <vector>

using Gecode::FloatNum;
using Gecode::FloatVal;

namespace {

    const FloatNum inf = std::numeric_limits<FloatNum>::infinity();
    const FloatNum max_normal = std::numeric_limits<FloatNum>::max();
    const FloatNum min_normal = std::numeric_limits<FloatNum>::min();
    const FloatNum denorm = std::numeric_limits<FloatNum>::denorm_min();
    const FloatNum nan = std::numeric_limits<FloatNum>::quiet_NaN();

    struct Mode {
        int value;
        const char* name;
    };
    const Mode modes[] = {
        {FE_TONEAREST, "nearest"}, {FE_DOWNWARD, "downward"}, {FE_UPWARD, "upward"}, {FE_TOWARDZERO, "towardzero"}};

    /// Numbers for singletons and number operands: infinities, extreme
    /// normals, denormals, signed zeros, non-representable decimals, a
    /// 1-ulp neighbour, and NaN
    std::vector<FloatNum> numbers(void) {
        return {-inf,       -max_normal, -1e5, -3.0, -1.0, -0.1, -min_normal, -denorm, -0.0, 0.0, denorm,
                min_normal, 0.1,         1.0,  std::nextafter(1.0, 2.0),      2.0,     3.0,  7.0, 1e5, max_normal,
                inf,        nan};
    }

    /// Interval operands: every non-NaN singleton, ranges straddling or
    /// touching zero, infinite and extreme bounds, tight intervals, and the
    /// NaN singleton
    std::vector<FloatVal> intervals(void) {
        std::vector<FloatVal> v;
        for (FloatNum n : numbers())
            if (n == n)
                v.push_back(FloatVal(n, n));
        const FloatNum one_up = std::nextafter(1.0, 2.0);
        const FloatNum ranges[][2] = {
            {-inf, inf},   {-inf, -1.0},        {-inf, 0.0},  {-inf, 0.1},     {0.0, inf},           {1.0, inf},
            {-max_normal, max_normal},          {-1.0, 1.0},  {-0.0, 0.0},     {0.0, -0.0},          {-1.0, 0.0},
            {0.0, 1.0},    {-0.1, 0.1},         {0.1, 0.3},   {1.0, one_up},   {1.0, 2.0},           {2.0, 3.0},
            {-3.0, -2.0},  {-3.0, 7.0},         {-0.1, -0.0}, {0.0, 0.1},      {denorm, min_normal}, {-denorm, denorm},
            {1e5, max_normal},                  {-max_normal, -1e5},           {0.1, 7.0},           {3.0, 1e5},
            {-1.0, -0.0},  {0.5, 1.0},          {-7.0, -0.1}};
        for (const auto& r : ranges)
            v.push_back(FloatVal(r[0], r[1]));
        v.push_back(FloatVal(nan));
        return v;
    }

    std::string text(FloatNum n) {
        char b[64];
        std::snprintf(b, sizeof(b), "%a", n);
        return b;
    }
    std::string text(const FloatVal& x) {
        return "[" + text(x.min()) + "," + text(x.max()) + "]";
    }
    std::string text(bool b) {
        return b ? "true" : "false";
    }

    /// Run \a f in caller mode \a m and print its result and the mode check
    template <class F>
    void probe(const Mode& m, const char* op, const std::string& args, F f) {
        std::fesetround(m.value);
        std::string result;
        try {
            result = text(f());
        } catch (std::exception& e) {
            result = std::string("throw ") + typeid(e).name() + ": " + e.what();
        } catch (...) {
            result = "throw unknown";
        }
        const bool kept = std::fegetround() == m.value;
        std::fesetround(FE_TONEAREST);
        std::printf("%s %s %s -> %s %s\n", m.name, op, args.c_str(), result.c_str(), kept ? "kept" : "CHANGED");
    }

    void unary(const Mode& m, const FloatVal& x) {
        const std::string a = text(x);
        probe(m, "pos", a, [&] { return +x; });
        probe(m, "neg", a, [&] { return -x; });
        probe(m, "abs", a, [&] { return abs(x); });
        probe(m, "sqrt", a, [&] { return sqrt(x); });
        probe(m, "sqr", a, [&] { return sqr(x); });
        for (int n = -3; n <= 5; n++)
            probe(m, "pow", a + " " + std::to_string(n), [&] { return pow(x, n); });
        for (int k = 1; k <= 5; k++)
            probe(m, "nroot", a + " " + std::to_string(k), [&] { return nroot(x, k); });
        probe(m, "size", a, [&] { return x.size(); });
        probe(m, "med", a, [&] { return x.med(); });
        probe(m, "tight", a, [&] { return x.tight(); });
        probe(m, "singleton", a, [&] { return x.singleton(); });
        probe(m, "zero_in", a, [&] { return x.zero_in(); });
#ifdef GECODE_HAS_MPFR
        probe(m, "exp", a, [&] { return exp(x); });
        probe(m, "log", a, [&] { return log(x); });
        probe(m, "sin", a, [&] { return sin(x); });
        probe(m, "cos", a, [&] { return cos(x); });
        probe(m, "tan", a, [&] { return tan(x); });
        probe(m, "asin", a, [&] { return asin(x); });
        probe(m, "acos", a, [&] { return acos(x); });
        probe(m, "atan", a, [&] { return atan(x); });
        probe(m, "sinh", a, [&] { return sinh(x); });
        probe(m, "cosh", a, [&] { return cosh(x); });
        probe(m, "tanh", a, [&] { return tanh(x); });
        probe(m, "asinh", a, [&] { return asinh(x); });
        probe(m, "acosh", a, [&] { return acosh(x); });
        probe(m, "atanh", a, [&] { return atanh(x); });
#endif
    }

    void binary(const Mode& m, const FloatVal& x, const FloatVal& y) {
        const std::string a = text(x) + " " + text(y);
        probe(m, "add", a, [&] { return x + y; });
        probe(m, "sub", a, [&] { return x - y; });
        probe(m, "mul", a, [&] { return x * y; });
        probe(m, "div", a, [&] { return x / y; });
        probe(m, "add_assign", a, [&] { FloatVal z = x; z += y; return z; });
        probe(m, "sub_assign", a, [&] { FloatVal z = x; z -= y; return z; });
        probe(m, "mul_assign", a, [&] { FloatVal z = x; z *= y; return z; });
        probe(m, "div_assign", a, [&] { FloatVal z = x; z /= y; return z; });
        probe(m, "lt", a, [&] { return x < y; });
        probe(m, "le", a, [&] { return x <= y; });
        probe(m, "gt", a, [&] { return x > y; });
        probe(m, "ge", a, [&] { return x >= y; });
        probe(m, "eq", a, [&] { return x == y; });
        probe(m, "ne", a, [&] { return x != y; });
        probe(m, "max", a, [&] { return max(x, y); });
        probe(m, "min", a, [&] { return min(x, y); });
        probe(m, "subset", a, [&] { return Gecode::Float::subset(x, y); });
        probe(m, "proper_subset", a, [&] { return Gecode::Float::proper_subset(x, y); });
        probe(m, "overlap", a, [&] { return Gecode::Float::overlap(x, y); });
        probe(m, "intersect", a, [&] { return Gecode::Float::intersect(x, y); });
        probe(m, "hull", a, [&] { return Gecode::Float::hull(x, y); });
#ifdef GECODE_HAS_MPFR
        probe(m, "fmod", a, [&] { return fmod(x, y); });
#endif
    }

    void mixed(const Mode& m, const FloatVal& x, FloatNum n) {
        const std::string a = text(x) + " " + text(n);
        const std::string b = text(n) + " " + text(x);
        probe(m, "add", a, [&] { return x + n; });
        probe(m, "add", b, [&] { return n + x; });
        probe(m, "sub", a, [&] { return x - n; });
        probe(m, "sub", b, [&] { return n - x; });
        probe(m, "mul", a, [&] { return x * n; });
        probe(m, "mul", b, [&] { return n * x; });
        probe(m, "div", a, [&] { return x / n; });
        probe(m, "div", b, [&] { return n / x; });
        probe(m, "add_assign", a, [&] { FloatVal z = x; z += n; return z; });
        probe(m, "sub_assign", a, [&] { FloatVal z = x; z -= n; return z; });
        probe(m, "mul_assign", a, [&] { FloatVal z = x; z *= n; return z; });
        probe(m, "div_assign", a, [&] { FloatVal z = x; z /= n; return z; });
        probe(m, "lt", a, [&] { return x < n; });
        probe(m, "lt", b, [&] { return n < x; });
        probe(m, "le", a, [&] { return x <= n; });
        probe(m, "le", b, [&] { return n <= x; });
        probe(m, "gt", a, [&] { return x > n; });
        probe(m, "gt", b, [&] { return n > x; });
        probe(m, "ge", a, [&] { return x >= n; });
        probe(m, "ge", b, [&] { return n >= x; });
        probe(m, "eq", a, [&] { return x == n; });
        probe(m, "eq", b, [&] { return n == x; });
        probe(m, "ne", a, [&] { return x != n; });
        probe(m, "ne", b, [&] { return n != x; });
        probe(m, "max", a, [&] { return max(x, n); });
        probe(m, "max", b, [&] { return max(n, x); });
        probe(m, "min", a, [&] { return min(x, n); });
        probe(m, "min", b, [&] { return min(n, x); });
        probe(m, "hull", a, [&] { return Gecode::Float::hull(x, n); });
        probe(m, "hull", b, [&] { return Gecode::Float::hull(n, x); });
        probe(m, "in", a, [&] { return x.in(n); });
#ifdef GECODE_HAS_MPFR
        probe(m, "fmod", a, [&] { return fmod(x, n); });
        probe(m, "fmod", b, [&] { return fmod(n, x); });
#endif
    }

    void numbers_only(const Mode& m, FloatNum l, FloatNum u) {
        const std::string a = text(l) + " " + text(u);
        probe(m, "construct", a, [&] { return FloatVal(l, u); });
        probe(m, "assign", a, [&] { FloatVal z; z.assign(l, u); return z; });
        probe(m, "hull_static", a, [&] { return FloatVal::hull(l, u); });
        probe(m, "hull_numbers", a, [&] { return Gecode::Float::hull(l, u); });
    }

} // namespace

int main() {
    const std::vector<FloatNum> ns = numbers();
    const std::vector<FloatVal> is = intervals();
    for (const Mode& m : modes) {
        probe(m, "default", "", [&] { return FloatVal(); });
        probe(m, "pi_half", "", [&] { return FloatVal::pi_half(); });
        probe(m, "pi", "", [&] { return FloatVal::pi(); });
        probe(m, "pi_twice", "", [&] { return FloatVal::pi_twice(); });
        for (FloatNum n : ns) {
            probe(m, "from_number", text(n), [&] { return FloatVal(n); });
            probe(m, "assign_number", text(n), [&] { FloatVal z(1.0, 2.0); z = n; return z; });
        }
        for (FloatNum l : ns)
            for (FloatNum u : ns)
                numbers_only(m, l, u);
        for (const FloatVal& x : is)
            unary(m, x);
        for (const FloatVal& x : is)
            for (const FloatVal& y : is)
                binary(m, x, y);
        for (const FloatVal& x : is)
            for (FloatNum n : ns)
                mixed(m, x, n);
    }
    return 0;
}
```

### A.21 `probe/tc.cpp`

```cpp
// Copy and layout traits of FloatVal and, on the native prototype, of
// Float::Interval (boost-007).
#include <gecode/float.hh>

#include <cstdio>
#include <type_traits>

namespace {

    template <class T>
    void print_traits(const char* name) {
        std::printf("%s is_trivially_copyable=%d is_trivially_copy_constructible=%d "
                    "is_trivially_copy_assignable=%d is_trivially_destructible=%d is_standard_layout=%d "
                    "sizeof=%zu alignof=%zu\n",
                    name, int(std::is_trivially_copyable_v<T>), int(std::is_trivially_copy_constructible_v<T>),
                    int(std::is_trivially_copy_assignable_v<T>), int(std::is_trivially_destructible_v<T>),
                    int(std::is_standard_layout_v<T>), sizeof(T), alignof(T));
    }

} // namespace

int main() {
    print_traits<Gecode::FloatVal>("FloatVal");
#if __has_include(<gecode/float/interval.hpp>)
    print_traits<Gecode::Float::Interval>("Float::Interval");
#endif
    return 0;
}
```

### A.22 `probe/pi.cpp`

```cpp
// The pi bounds as hexadecimal floating point (boost-007).
#include <gecode/float.hh>

#include <cstdio>

int main() {
    std::printf("pi_half %a %a\n", Gecode::pi_half_lower(), Gecode::pi_half_upper());
    std::printf("pi %a %a\n", Gecode::pi_lower(), Gecode::pi_upper());
    std::printf("pi_twice %a %a\n", Gecode::pi_twice_lower(), Gecode::pi_twice_upper());
    std::printf("FloatVal::pi_half %a %a\n", Gecode::FloatVal::pi_half().min(), Gecode::FloatVal::pi_half().max());
    std::printf("FloatVal::pi %a %a\n", Gecode::FloatVal::pi().min(), Gecode::FloatVal::pi().max());
    std::printf("FloatVal::pi_twice %a %a\n", Gecode::FloatVal::pi_twice().min(), Gecode::FloatVal::pi_twice().max());
    return 0;
}
```

### A.23 `probe/cmp-native.cpp (diff against probe/cmp.cpp)`

```diff
--- probe/cmp.cpp	2026-11-04 07:13:12
+++ probe/cmp-native.cpp	2026-11-04 07:13:12
@@ -1,6 +1,7 @@
-// Comparison semantics of FloatVal and of the interval type behind it.
-// Prints one row per operand pair: the raw interval result (T, F, or X for
-// comparison_error) and the FloatVal result (T or F), for < <= > >= == !=.
+// Comparison semantics of FloatVal (boost-007: cmp.cpp from boost-003 without
+// the raw Boost interval column, which has no native counterpart).
+// Prints one row per operand pair: the FloatVal result (T or F), and for a
+// number operand also with the number on the left, for < <= > >= == !=.
 #include <gecode/float.hh>
 
 #include <cmath>
@@ -8,42 +9,22 @@
 
 using Gecode::FloatNum;
 using Gecode::FloatVal;
-// The same type as the protected FloatVal::FloatValImpType.
-using I = gecode_boost::numeric::interval<
-    FloatNum, gecode_boost::numeric::interval_lib::policies<
-                  Gecode::Float::Rounding, gecode_boost::numeric::interval_lib::checking_strict<FloatNum>>>;
 
 namespace {
 
-    template <class F>
-    char raw(F f) {
-        try {
-            return f() ? 'T' : 'F';
-        } catch (gecode_boost::numeric::interval_lib::comparison_error&) {
-            return 'X';
-        }
-    }
-
     char tf(bool b) {
         return b ? 'T' : 'F';
     }
 
     void row_vv(const char* name, FloatNum xl, FloatNum xu, FloatNum yl, FloatNum yu) {
-        I x(xl, xu), y(yl, yu);
         FloatVal a(xl, xu), b(yl, yu);
-        std::printf("%-28s raw %c%c%c%c%c%c  FloatVal %c%c%c%c%c%c\n", name, raw([&] { return x < y; }),
-                    raw([&] { return x <= y; }), raw([&] { return x > y; }), raw([&] { return x >= y; }),
-                    raw([&] { return x == y; }), raw([&] { return x != y; }), tf(a < b), tf(a <= b), tf(a > b),
-                    tf(a >= b), tf(a == b), tf(a != b));
+        std::printf("%-28s FloatVal %c%c%c%c%c%c\n", name, tf(a < b), tf(a <= b), tf(a > b), tf(a >= b), tf(a == b),
+                    tf(a != b));
     }
 
     void row_vn(const char* name, FloatNum xl, FloatNum xu, FloatNum y) {
-        I x(xl, xu);
         FloatVal a(xl, xu);
-        std::printf("%-28s raw %c%c%c%c%c%c  FloatVal %c%c%c%c%c%c  num-left %c%c%c%c%c%c\n", name,
-                    raw([&] { return x < y; }), raw([&] { return x <= y; }), raw([&] { return x > y; }),
-                    raw([&] { return x >= y; }), raw([&] { return x == y; }), raw([&] { return x != y; }),
-                    tf(a < y), tf(a <= y), tf(a > y), tf(a >= y), tf(a == y), tf(a != y), tf(y < a), tf(y <= a),
+        std::printf("%-28s FloatVal %c%c%c%c%c%c  num-left %c%c%c%c%c%c\n", name, tf(a < y), tf(a <= y), tf(a > y), tf(a >= y), tf(a == y), tf(a != y), tf(y < a), tf(y <= a),
                     tf(y > a), tf(y >= a), tf(y == a), tf(y != a));
     }
 
@@ -75,12 +56,6 @@
     row_vn("x=[1,1+ulp] y=1+ulp tight", 1, one_up, one_up);
     std::printf("empty interval\n");
     try {
-        I e(2, 1);
-        std::printf("I(2,1) constructed: [%g,%g]\n", e.lower(), e.upper());
-    } catch (std::exception& ex) {
-        std::printf("I(2,1) throws %s\n", ex.what());
-    }
-    try {
         FloatVal e(2, 1);
         std::printf("FloatVal(2,1) constructed: [%g,%g]\n", e.min(), e.max());
     } catch (std::exception& ex) {
```
