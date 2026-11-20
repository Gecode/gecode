# nextafter outward rounding compared with switched rounding modes

## Result

This answers boost-002 and the brief's open question for approach 3: does
outward rounding with `std::nextafter`, applied after round-to-nearest
operations, change the results or the runtime of the float tests and float
examples, compared with switched rounding modes?

Two prototypes replace the arithmetic in `Float::RoundingBase` and stop
touching the rounding mode. The reference is the switched-rounding
implementation at 3f4b5272f9 (boost-006).

- **P1, plain nextafter.** Each `+ - * /` and `sqrt` is computed in the
  caller's mode (normally round to nearest), and the result is moved one ulp
  outward on each side. Exact results are widened too.
- **P2, error terms.** The same nearest result, plus an error term (Fast2Sum,
  or an `fma` residual) whose sign says on which side of the result the exact
  value lies. The result moves one ulp, and only on that side. Only the sign
  of the error term is used; Fast2Sum does not recover the exact error under
  directed caller modes (see Prototypes). Near
  underflow, at overflow, and with infinite operands, P2 falls back to P1.

Both prototypes were sound in every check: no unsound row in the rounding
probe (three compilers at three optimization levels) or the MPFR-checked
harness (all four caller modes), and no lost solution in the float suite.
P1 widens almost every bound, and that changes the float suite and the
examples. P2 gives the reference's bounds
except in edge cases near zero, underflow, and overflow.

| measure | ref | P1 | P2 | command |
|---|---|---|---|---|
| scalar bounds wider than ref, exact results | - | 100% of 19204 | 3.5% | [C4] |
| scalar bounds wider than ref, inexact results | - | 99.8% of 53668 | 1.2% | [C4] |
| FloatVal results wider than ref | - | 65.3% of 102000 | 0% | [C4] |
| example expressions wider than ref | - | 94.7% of 35000 | 0% | [C4] |
| unsound rows, probe and harness | 0 | 0 | 0 | [C3], [C4] |
| Float:: suite, failed of 1561 | 0 | 452 | 7 | [C5] |
| same, harness cuts clamped (P1h, P2h) | - | 116 | 0 | [C7] |
| example solution counts | 189/572/390/98 | same | same | [C8] |
| example nodes, cartesian-heart | 28112 | 30158 | 28112 | [C8] |
| example runtime, archimedean-spiral | 223.6 ms | 1.073x | 0.978x | [C8] |
| example runtime, cartesian-heart | 337.3 ms | 1.566x | 0.615x | [C8] |
| example runtime, descartes-folium | 1406.0 ms | 1.017x | 1.471x | [C8] |
| example runtime, golden-spiral | 160.9 ms | 1.099x | 0.979x | [C8] |
| microbenchmark, FloatVal loop | 199.9 ns | 62.0 ns | 60.5 ns | [C9] |
| microbenchmark, Rounding loop (ends non-finite) | 43.0 ns | 37.3 ns | 34.2 ns | [C9] |
| Float:: suite, one thread, one run | 1264.6 s | not comparable | 1124.3 s | [C10] |

Example runtimes are the minimum of 11 interleaved runs; the microbenchmark
values are the minimum of 7. Ratios are prototype / ref.

The main findings:

- P1 makes almost every interval wider. A single operation gains one ulp on one
  side for an inexact result, or on both sides for an exact one. Over the
  example models' expressions, the width grows by a median of 3 ulps per
  side and a 99th percentile of 289 to 513 ulps [C4]. In the examples'
  solutions, the X and Y intervals are a median of 5.4 to 7.3 times as wide
  as the reference's [C8].
- P1 fails 452 of the 1561 Float:: tests [C5]. 336 of them fail only because
  the test harness builds its pruning cuts with `Float::Rounding` and assumes
  exact results stay exact. With those cuts clamped, 116 fail: 4 tests of the
  tightness contract (class A), and 112 subsumption and fixpoint checks that
  a widened bound breaks (class B). None loses a solution (class C) [C7].
- P1 keeps the examples' solution counts, but changes their search: more
  propagations in all four examples, and different nodes and failures in
  cartesian-heart and descartes-folium. 4 printed cartesian-heart solutions
  and 1 printed solution each in archimedean-spiral and golden-spiral differ
  in the sixth digit or near zero [C8].
- P2's 7 suite failures are harness cuts too. With the cuts clamped, P2h
  passes all 1561 tests [C7]. P2's example statistics equal the reference's
  except 1 or 2 extra propagations in archimedean-spiral, cartesian-heart,
  and golden-spiral, and its printed solutions differ only in
  archimedean-spiral's first solution, at the origin [C8].
- No mode switching is cheaper per `FloatVal` operation: 199.9 ns to 62.0 ns
  per loop iteration with P1 [C9]. In the examples P1 loses this gain to its
  wider search. P2 is faster than ref in three examples and 1.471 times as
  slow in descartes-folium, for a reason this study did not establish
  [C8], [C11].

## Question and scope

Covered: `Float::RoundingBase` `+ - * /` and `sqrt` in both directions, and
everything built on them (`FloatVal`, Boost.Interval functions, all float
propagators, the float test suite, and the four float examples). The machine
is arm64 macOS, with a Release library built by AppleClang 21 with MPFR.

Not covered:

- x86-64 and MSVC. This machine cannot run x86-64 code, and the prototypes
  drop the MSVC branch. On x86-64 without `-mfma`, P2's `std::fma` becomes a
  library call; its cost there is not measured.
- A library built by GCC or LLVM clang. The probe compiles the inline header
  code with GCC 14 and LLVM 22, but every library and test binary here is
  built by AppleClang.
- Builds without MPFR. Without MPFR the transcendental functions use Boost's
  own code, which this study does not exercise.
- Debug builds. In a Debug build the test harness's own `assert` would stop
  at the cuts described under Float suite.
- A bit-increment `next_up`/`next_down` instead of the `std::nextafter` call
  (see Runtime).
- `float` and `long double`.

## Setup

- Reference revision 3f4b5272f9 (boost-006), full hash
  3f4b5272f9af75f0a204581916fc5e2acbee7ba9, on branch feature/breaking-7.
  `git status --short` was empty before and after the study. The study is
  the only new file outside `build/`; `.zdev/` is listed in
  `.git/info/exclude`, so it shows only with `--ignored`.
- Apple M1 Max (10 cores), macOS 27.0.1 (26A434), SDK 27.0.
- Compiler for all builds: `/Library/Developer/CommandLineTools/usr/bin/c++`,
  Apple clang version 21.0.0 (clang-2100.3.34.2). Probe compilers: the same
  AppleClang as `/usr/bin/clang++`, Homebrew clang version 22.1.8, and g++-14
  (Homebrew GCC 14.2.0_1) 14.2.0.
- MPFR 4.2.2 and GMP 6.3.0 from Homebrew, CMake 3.27.6.
- Reference build: `build/local`, Unix Makefiles, Release (`-O3 -DNDEBUG`),
  `-std=c++2b -arch arm64`, `CMAKE_CXX_FLAGS` empty, `GECODE_ENABLE_MPFR=ON`,
  `GECODE_ENABLE_GIST=OFF`, otherwise defaults. `cmake --build build/local`
  relinked some targets at HEAD; it was then installed to
  `build/nextafter-proto/install-ref` [C1].
- Prototype builds: `build/nextafter-proto/build-p1` and `build-p2`, from
  source copies of HEAD, configured with the same options [C2]. The diff of
  the cache entries `CMAKE_BUILD_TYPE`, `CMAKE_CXX_COMPILER`,
  `CMAKE_CXX_FLAGS*`, `CMAKE_GENERATOR`, `GECODE_ENABLE_*`, `GECODE_BUILD_*`,
  and the MPFR and GMP paths against `build/local` shows only the type tag of
  the `CMAKE_CXX_COMPILER` entry (`FILEPATH` in `build/local`, `STRING` and
  `UNINITIALIZED` in the prototypes, set on the command line). The
  examples' `flags.make` are identical: `-O3 -DNDEBUG -std=c++2b -arch arm64
  -isysroot .../MacOSX27.0.sdk` [C13].
- Load. Other processes (development servers and a Docker VM) used about one
  to one and a half cores throughout. No two timed runs overlapped. Nothing
  else was started during the example and microbenchmark timing; during the
  single-threaded suite runs only single-test replays and text processing
  ran. The load averages before and after
  each timed run are in `results/*.load`: 3.81 at the start of the example
  timing, 4.42 at the end of the microbenchmark.

All paths below are relative to `build/nextafter-proto`, which git ignores.

## Prototypes

Both prototypes edit only `gecode/float.hh` and `gecode/float/rounding.hpp`
in their source copies. The patches are in Appendix A. Neither needed a
change to propagators, Boost headers, or tests to build.

Changes common to both:

- `Float::Rounding` is `save_state_nothing<RoundingBase>` instead of
  `save_state<RoundingBase>`. A scope no longer reads or sets the mode. The
  vendored type exists and supplies `unprotected_rounding`.
  `RoundingBase` still derives from `RoundingControl`, so the code that
  instantiates `save_state<Rounding>` (the RoundingScope test) still
  compiles.
- `next_down(x)` and `next_up(x)` are `std::nextafter(x, -inf)` and
  `std::nextafter(x, +inf)`.
- `median()` computes `(l + u) / 2.0`, with the existing overflow-safe
  halving. The reference calls `rounded_arith_opp::median`, which switches to
  nearest and back to upward. Under a scope that never sets upward mode, that
  would leave the caller in upward mode. In round to nearest the two compute
  the same value.
- `int_down` and `int_up` stay `std::floor` and `std::ceil`, which are exact
  in every mode.
- The MPFR functions in `rounding.cpp` are unchanged. They convert the operand
  exactly and round with MPFR's own directed modes, which do not depend on the
  hardware mode.
- The boost-006 `barrier()` stays around every operand and result, so that a
  prototype differs from the reference only in its rounding. A test program
  compiled with `-DGECODE_PROTOTYPE_NO_BARRIER` turns the barrier into a plain
  return; the functions are inline, so this variant (P1-nb) needs no library
  build.

P1, per operation (`op` is `+ - * /`):

- `op_down = next_down(x op y)`, `op_up = next_up(x op y)`.
- `sqrt_up = next_up(sqrt(x))`. `sqrt_down = r > 0 ? next_down(r) : r` with
  `r = sqrt(x)`, so the lower bound never goes below zero, `sqrt(0)` keeps
  0 as its lower bound, and NaN stays NaN.

Properties of P1, from its construction:

- Exact results are widened on both sides. A zero result becomes
  `[-denorm_min, +denorm_min]`.
- P1 is sound in every caller mode. Any IEEE mode returns one of the two
  doubles next to the exact value, and one ulp outward from either one bounds
  it. Overflow is covered: `nextafter(inf, -inf)` is `DBL_MAX`.
- With an infinite operand, a bound on the far side becomes `-DBL_MAX` or
  `DBL_MAX` instead of the infinity the reference returns. This is sound but
  loose.

P2 adds an error term `e` to P1: `down = e < 0 ? next_down(r) : r` and
`up = e > 0 ? next_up(r) : r`.

- `add`: Fast2Sum after ordering by magnitude: if `|x| < |y|` swap, then
  `z = s - x` and `e = y - z`. `sub(x, y)` is `add(x, -y)`. Fast2Sum is
  chosen over TwoSum because TwoSum is exact only in round to nearest; P2 only
  needs the sign of `e`. Fast2Sum is not exact under directed modes either:
  with `FE_UPWARD`, `x = 1` and `y = 2^-200`, it returns
  `e = -2^-52 + 2^-105` where the exact residual is `-2^-52 + 2^-200`. The
  sign is still right there, and the MPFR-checked harness found no case in
  any caller mode where a wrong sign gave an unsound bound.
- `mul`: `e = fma(x, y, -p)`. `div`: `e = fma(-q, y, x)`, negated when
  `y < 0`. `sqrt`: `e = fma(-s, s, x)`.
- P2 falls back to P1's widening when the result is not finite, when a
  nonzero operand is below 2^-969 in magnitude, and when the product or
  quotient is below 2^-969 (including an underflow to zero). An operand of
  zero in `mul` or a dividend of zero in `div` keeps the exact zero.
  `div` also falls back for an infinite divisor. `sqrt_down` never goes below
  zero.

The first P2 build lacked the underflow-to-zero fallback for `mul` and `div`.
The harness found 4 unsound `mul` rows, in each of three caller modes
(nearest, downward, toward zero), for example `0x1p-540 * 0x1p-540`, where P2
returned `[0, 0]` and the exact bounds are `[0, 0x1p-1074]` [C4]. P2 was fixed
and rebuilt before any other measurement; all P2 numbers in this study are
for the fixed build. `make_proto.py` (Appendix C) produces the fixed
version.

### Assembly

The microbenchmark's assembly [C9] shows the per-operation code. AppleClang
-O3 keeps `std::nextafter` a library call: `bench-p1.s` has 31 `bl
_nextafter`, `bench-p1nb.s` 32, and `bench-p2.s` 34, and none of them has a
`fesetround` or `fegetround` call. `bench-ref.s` has 20 `_fesetround` and 7
`_fegetround` calls and no `_nextafter`. P2's `std::fma` is inlined:
`bench-p2.s` has 41 `fmadd`/`fmsub`/`fnmadd`/`fnmsub` instructions, for
example `fmsub d0, d11, d11, d0` for the `sqrt` residual. A P1 addition is:

```asm
fadd    d0, d0, d1
; InlineAsm Start      (barrier)
; InlineAsm End
mov.16b v1, v8          ; +inf
bl      _nextafter
```

## Commands

All commands run from `build/nextafter-proto` unless they name
`build/local`. The scripts are in Appendix C.

- **[C1] Reference.** `cmake --build build/local -j 10` and
  `cmake --install build/local --prefix build/nextafter-proto/install-ref`,
  from the repository root.
- **[C2] Prototypes.** `git archive HEAD | tar -x -C src-<v>` for `src-head`,
  `src-p1`, and `src-p2`; `python3 -I scripts/make_proto.py <p1|p2>
  src-<v>`; `scripts/build.sh <p1|p2>` (configure, build, install to
  `install-<v>`); `cmake --build build-<v> --target gecode-test -j 10`,
  because `gecode-test` is not in the default target. Patches:
  `diff -ru src-head src-<v> > <v>.patch`.
- **[C3] Rounding probe.** `probe-<v>/run.sh` for `<v>` in `ref`, `p1`,
  `p1nb`, `p2`. Each directory holds unchanged copies of
  `build/rounding-probe/rounding-probe.cpp` and `run.sh` (Appendices A and B
  of `rounding-probe.md`) and an `install` link to `install-ref`,
  `install-p1`, or `install-p2`. `probe-p1nb/run.sh` adds
  `-DGECODE_PROTOTYPE_NO_BARRIER` to `COMMON_FLAGS`. Summary:
  `python3 -I scripts/probe_summary.py probe-ref probe-p1 probe-p1nb
  probe-p2`.
- **[C4] Tightness harness.** `tightness/build.sh <v>` for `ref`, `p1`,
  `p1nb`, `p2`; `tightness/tightness-<v> > tightness/<v>.tsv`;
  `python3 -I tightness/compare.py tightness/ref.tsv tightness/<v>.tsv`.
  Sources in Appendix B.
- **[C5] Float suite, pass/fail.** `scripts/suite-t8.sh`, which runs
  `<build>/bin/gecode-test -test Float:: -stop false -threads 8` for ref
  (`build/local`), P1, and P2 one after another. Logs:
  `results/suite-<v>-t8.log`.
- **[C6] Failure classification.** `scripts/classify.sh <build> <log> <out>`
  replays every failure once with its printed
  `-replay <seed> -test <name>` line plus `-log` and keeps the `Test`,
  `Problem`, and `Assignment` lines. Output:
  `results/classify-<v>.tsv`. Test::Base tests print their messages without
  `-log`; they were run with `gecode-test -test <name>`.
- **[C7] Clamped harness cuts.** Source copies `src-p1h` and `src-p2h` are
  P1 and P2 plus the 6-line `test/float.cpp` change in Appendix A.3. Built
  with `scripts/build.sh`'s configuration plus `-DGECODE_ENABLE_EXAMPLES=OFF`,
  target `gecode-test`. Run: `build-<v>h/bin/gecode-test -test Float:: -stop
  false -threads 8`, then [C6] on the P1h log.
- **[C8] Examples.** `scripts/examples.sh`. For each build and example:
  `-mode solution` of the unmodified binary, `-mode solution` of the
  precise-print copy, and `-mode stat` twice; then 11 rounds, each running
  ref, P1, and P2 in turn on all four examples with `-mode stat`, recording
  the `runtime:` milliseconds. Default options throughout. The precise-print
  copies are made by `python3 -I scripts/make_precise.py ../../examples
  examples-precise` and built by `examples-precise/build.sh` with
  `build/local`'s compile and link flags. Comparison:
  `python3 -I scripts/compare_examples.py <ref.precise> <v.precise>`.
- **[C9] Microbenchmark.** `bench/build.sh` builds
  `build/zdev-plan/boost-006/bench.cpp` unchanged for ref, P1, P1-nb, and
  P2, plus its assembly; `scripts/bench.sh` runs `bench-<v> 10000000` in 7
  interleaved rounds. Assembly counts: `grep -c 'b[l]*[[:space:]]*_nextafter'
  bench-<v>.s`, and the same for `_fesetround`, `_fegetround`, and
  `fmadd|fmsub|fnmadd|fnmsub`.
- **[C10] Float suite, single-threaded time.** In `scripts/timed.sh`, after
  [C8] and [C9]: `/usr/bin/time -p <build>/bin/gecode-test -test Float:: -stop
  false -threads 1`, once per build, one after another.
- **[C11] Profile.** `sample <pid> 1` on `<build>/bin/descartes-folium -mode
  stat`, started 0.3 s after the program. Output:
  `results/profile/folium-<v>.txt`.
- **[C12] Test count.** `<build>/bin/gecode-test -list | grep -c
  '^Float::'`: 1561 in all three builds. Tags:
  `build/local/bin/gecode-test -list-with-tags | grep '^Float::'`, counted by
  the last field.
- **[C13] Configuration.** `grep -E` of the cache entries listed in Setup
  in `build/local/CMakeCache.txt` and `build-<v>/CMakeCache.txt`, then
  `diff`; `diff` of the `FLAGS` lines of
  `examples/CMakeFiles/cartesian-heart.dir/flags.make`.

## Soundness

**Rounding probe [C3].** 160 arithmetic rows and 8 live-scope rows per
build. Every configuration ran: AppleClang, LLVM 22, and GCC 14, each at -O0,
-O2, and -O3.

| library | rows | exact | loose | unsound | both sides loose | mode restored | live scope keeps upward |
|---|---|---|---|---|---|---|---|
| ref | 160 | 160 | 0 | 0 | 0 | 168/168 | 8/8 |
| P1 | 160 | 0 | 160 | 0 | 0 | 168/168 | 2/8 |
| P1-nb | 160 | 0 | 160 | 0 | 0 | 168/168 | 2/8 |
| P2 | 160 | 160 | 0 | 0 | 0 | 168/168 | 2/8 |

The numbers are identical in all nine configurations of each library. The
probe's cases all have inexact results, so P1 is loose on exactly one side in
every row: for example `0.1 + 0.2` gives `[0x1.3333333333333p-2,
0x1.3333333333335p-2]` against the exact `[...333p-2, ...334p-2]`. P1 and P2
exit with status 1 and 0. The prototypes never set upward mode, so a live
scope keeps upward mode only in the 2 rows where the caller is already in
upward mode; this is intended.

**Harness [C4].** Every scalar case is checked against MPFR at 53 bits in
round to nearest, and the first 1000 cases of each operation and class also
in downward, upward, and toward-zero caller modes. Each FloatVal case is
checked against an MPFR hull where one is easy to state (all operations
except `sin` and `cos`), in round to nearest.

| library | scalar rows, nearest | scalar rows, each other mode | unsound | FloatVal rows checked | unsound |
|---|---|---|---|---|---|
| ref | 72872 | 17872 | 0 | 85000 | 0 |
| P1 | 72872 | 17872 | 0 | 85000 | 0 |
| P1-nb | 72872 | 17872 | 0 | 85000 | 0 |
| P2 | 72872 | 17872 | 0 | 85000 | 0 |

Every harness row checks out exact in ref. In P1, 109 scalar rows in round
to nearest are exact, all overflow cases where the nearest result is
infinite. The ref FloatVal rows have 6259 loose results (Boost's `pow`,
`nth_root`, and the compound bounds of `sqr` and `sqrt` are not tight); P2
has the same 6259.

## Tightness

### Scalar operations

From `compare.py` on round-to-nearest rows [C4]. Ulps count the doubles
between two bounds.

| class | rows | P1 wider | P1 extra ulps per side | P2 wider | P2 extra ulps per side |
|---|---|---|---|---|---|
| exact result | 19204 | 100% | 1 on each side | 3.5% | 0 or 1 |
| inexact result | 53668 | 99.8% | 1 on one side | 1.2% | 0 or 1 |

Per operation, P1 is wider in 100% of the `random`, `int`, `dyadic`, and
`zero` rows, and in 69.9% to 100% of the `edge` rows. P2 is wider only in
`edge` rows (31% to 68% of them, by operation), where it falls back to P1's
widening. Neither prototype is narrower than ref in any row. The classes are
defined in the harness (Appendix B): 10000 random operands per operation,
2000 small integers, 2000 dyadic values `k * 2^-j` within [-20, 20], 200 zero
results, and every pair from a list of 23 special values (signed zeros,
subnormals, values near underflow and overflow, infinities).

### FloatVal operations

102000 FloatVal results: 12 operations, each over 5000 random intervals in
[-20, 20], 2000 narrow intervals (width 2^-1 to 2^-40), and 500 each of
integer, dyadic, and point intervals [C4].

| operation | P1 wider (random / narrow / point) | P1 extra ulps per side, median / max | P1 width growth, point intervals (median) |
|---|---|---|---|
| `+`, `-` | 97% / 100% / 100% | 1 / 1 | 1.0 |
| `*`, `/` | 75% / 74-75% / 100% | 0-1 / 1 | 1.0 |
| `sqr`, `sqrt` | 64-75% / 73-77% / 100% | 0-1 / 1 | 1.0 |
| `pow(x, 3)` | 94% / 95% / 100% | 1-2 / 3 | 1.0 |
| `nroot(x, 2)` | 100% / 100% / 100% | 1 / 7 | 3.0 |
| `sin`, `cos` | 22% / 98% / 97-98% | 14-22 (narrow) / 98304 | 1.7-2.25 |
| `abs`, `exp` | 0% | 0 / 0 | 0 |

Width growth is P1's width divided by ref's, minus 1. A growth of 1.0 means
a point interval's result is twice as wide. `sin` and `cos` grow most because
Boost reduces the argument modulo 2 pi with widened operations before the
MPFR call. `abs` does no rounding, and `exp` is a single MPFR call on each
bound. For narrow intervals the relative growth of `sin` is a median of
5.1e-9 and a 99th percentile of 8.3e-3. P2's 102000 results are identical to
ref's.

### The examples' expressions

7 expressions from the four examples, each evaluated as one FloatVal
expression over 2000 random, 2000 narrow, and 1000 dyadic boxes from the
example's domain, 35000 in total [C4]. P1 is wider in 94.7%. For narrow
boxes:

| expression | P1 wider | extra ulps lower side, median / p99 | extra ulps upper side, median / p99 | width growth, median / p99 |
|---|---|---|---|---|
| heart: `sqr(x) + 2 * sqr(y - 0.5 * nroot(abs(x), 2))` | 100% | 3 / 5 | 3 / 5 | 5.5e-11 / 1.1e-3 |
| folium x: `3 * p / (1 + pow(p, 3))` | 100% | 3 / 6 | 3 / 6 | 1.7e-9 / 3.7e-3 |
| folium y: `3 * sqr(p) / (1 + pow(p, 3))` | 100% | 3 / 7 | 4 / 7 | 2.3e-9 / 3.1e-3 |
| folium: `pow(x, 3) + pow(y, 3) - 3 * x * y` | 100% | 3 / 321 | 3 / 257 | 1.4e-12 / 2.7e-5 |
| spiral x: `r * cos(t)` | 98.5% | 9 / 519 | 9 / 508 | 4.4e-11 / 4.2e-4 |
| spiral y: `r * sin(t)` | 99.0% | 14 / 1271 | 11 / 2476 | 4.6e-11 / 1.2e-3 |
| golden: `exp(0.30649 * t)` | 67.3% | 0 / 8 | 0 / 8 | 2.5e-12 / 3.1e-3 |

Ulp counts are large where a bound lies near zero, because doubles are
dense there; the maximum over all rows is 4.4e18 ulps, for a lower bound
that moves from 0 to -6.7e-16. The relative growth does not have this
problem. P2's 35000 results are identical to ref's.

### The examples' solutions

The precise-print copies print every variable's bounds as hexfloat [C8].
Solutions are compared by index; the counts agree in every build.

| example (variables) | P1 wider solutions per variable | P1 extra width in ulps, median / max | P1 width growth, median / max | P2 differing lines |
|---|---|---|---|---|
| archimedean-spiral (theta, x, y, r) | 1, 189, 188, 189 of 189 | x 105 / 29589, y 115 / 12319, r 10 / 12 | x 6.3 / 11.5, y 5.0 / 10.5, r 10 / 12 | 2 of 756 |
| cartesian-heart (x, y) | shifted, see below | 0 / 1 | 0 / 1 | 0 of 1144 |
| descartes-folium (p, x, y) | 84 shifted, 390, 390 of 390 | x 36 / 62, y 46 / 65 | x 4.7 / 6.8, y 5.0 / 8.3 | 0 of 1170 |
| golden-spiral (theta, x, y, r) | 1, 98, 98, 94 of 98 | x 67 / 5973, y 87 / 1.0e16, r 11 / 32 | x 5.4 / 10.5, y 4.4 / 7.1, r 2.5 / 6.5 | 0 of 392 |

The branching variables (theta, cartesian-heart's x and y, folium's p) keep
their widths. P1 reaches different boxes for them where the search changed:
in cartesian-heart 569 of 572 x boxes and 568 y boxes are shifted, not wider.
P2's 2 differing lines are archimedean-spiral's first solution at the
origin: x is `[0, 0x1.8p-1073]` instead of `[0, 0x1p-1074]`, and r is
`[0, 0x1p-1073]` instead of `[0, 0x1p-1074]`. "Extra width" counts both sides
together; widths near zero are many ulps, as in golden-spiral's y.

## Float suite

`gecode-test -test Float:: -stop false -threads 8` [C5]. 1561 tests in every
build [C12]: 9 tagged `check`, 518 `standard`, and 1034 `sweep`
(`gecode-test -list-with-tags`). Without `-tag`, the run selects all of
them. The test inputs differ slightly between builds, because
`test/float.cpp` and `test/float.hpp` generate random values and cuts with
`Float::Rounding`.

| build | passed | failed | elapsed (not compared) |
|---|---|---|---|
| ref | 1561 | 0 | 519.7 s |
| P1 | 1109 | 452 | 108.4 s |
| P2 | 1554 | 7 | 508.3 s |
| P1h (P1, cuts clamped) [C7] | 1445 | 116 | 193.6 s |
| P2h (P2, cuts clamped) [C7] | 1561 | 0 | 501.6 s |

The elapsed time is the suite's own `Elapsed time:` line. A failing test
stops at its first failure, so P1's runs are shorter, and 8 threads share
the machine with other load; these times are not runtime evidence.

### Classes

Every failure was replayed once [C6]. Classes:

- **A**: a test of the switched-mode or tightness contract.
- **B**: a widened bound changes a subsumption, fixpoint, or MAYBE outcome.
- **C**: a lost solution, a soundness bug in the prototype.
- **H**: the test harness posts a cut that excludes the assignment it is
  pruning towards, so the space fails although no propagator is wrong.

Class H was not in the plan. `TestSpace::prune(const Assignment&, ...)` in
`test/float.cpp` draws a cut between the assignment and the domain bound
with `rand_f_val_up` and `rand_f_val_down` (lines 109-126), which use
`Float::Rounding`. A lower cut is `sub_up(a.min(), t)` and an upper cut
`add_down(a.max(), t)`, with `t = mul_down(fraction, width)`. In the
reference `t >= 0`, so the cut never excludes the assignment. In P1,
`sub_up(u, 0)` is `next_up(u)` and `add_down(u, 0)` is `next_down(u)`, both
outside the assignment. In P2, a product that underflows makes `t` equal to
`-denorm_min`, with the same result. The harness asserts that the cut keeps
the assignment (lines 362 and 371), but Release builds compile the `assert`
out, so the cut is posted. One replay shows it:
`Float::Arithmetic::Max::Nary::Shared` with P1 (`-replay 1471113749`), with
assignment `{[-4,-4],[0,4.94066e-324],[0,4.94066e-324]}`, logs the cut
`x[1] <= [-4.94066e-324,-4.94066e-324]`, then `Failed on solution`.
`Float::Basic`, which posts no constraint at all, fails the same way under
both prototypes.

P1h and P2h are P1 and P2 with the cut clamped to the assignment's side, as
the harness's own asserts require (Appendix A.3). The clamp leaves every
check of propagation results in place, so a lost solution would still fail.

### P1

452 failures [C5], replayed with [C6]:

| test / problem | count | families | class |
|---|---|---|---|
| Prune reified (<=>, =>, <=) / Failed | 291 | Linear::Float 146, Linear::Var 139, Rel::Float 6 | H |
| Prune / Failed on solution | 43 | Linear 15, Max 6, Min 6, Rel::Float 6, Dom::Num 4, Div 2, Mult 2, PowBaseConsistency 1, Sin 1 | H |
| Test::Base: Float::Basic (2 tests) | 2 | Basic | H |
| Assignment (before posting) / No subsumption | 66 | MiniModel::LinRel | B |
| Assignment and Partial assignment (after posting) / No subsumption | 10 | Max::Bin, Min::Bin, Max::Nary, Min::Nary | B |
| Prune / No fixpoint | 36 | Abs 3, Div 2, Mult 2, NRoot 4, Pow 4, Sqr 2, Sqrt 2, Exp 2, Log 2, ACos 2, ASin 2, ATan 2, Cos 2, Sin 2, Tan 3 | B |
| Test::Base | 4 | DirectedRounding, RoundingScope, MultZeroEndpoint, PowConsistency | A |

The H rows are exactly the 336 tests that pass in P1h. The 116 tests that
fail in P1h all fail in P1 too, and P1h's replays [C6] show only the B and A
rows above: 66 + 10 = 76 `No subsumption`, 36 `No fixpoint`, and the 4
Test::Base tests. No `Failed on solution`, `Failed`, `Solutions exhausted`,
or `Zero on solution` remains, so there is no class C failure.

Examples of each class, with the replay line from the P1 log:

- A: `Float::Arithmetic::DirectedRounding` (`-replay 1007013737`) reports
  `0.1 + 0.2` as `[0x1.3333333333333p-2, 0x1.3333333333335p-2]`, expected
  `[..333p-2, ..334p-2]`. `Float::Arithmetic::RoundingScope` (`-replay
  1007013737`) expects `add_down(1, 2^-53) == 1`.
  `Float::Arithmetic::MultZeroEndpoint` (`-replay 1354576396`) expects `x`
  contracted exactly to `[0.5, 1]`. `Float::Arithmetic::PowConsistency`
  (`-replay 1459051619`) expects `pow(x, n) = 0` to give exactly 0 for
  `n = 2` and `3`.
- B: `Float::Arithmetic::Abs::XY::A` (`-replay 1290022630`), assignment
  `{[-8,-8],[0.25,0.25]}`: `No fixpoint`; the propagator prunes again when
  reposted on the clone of its fixpoint. `Float::MiniModel::LinRel::*`:
  `No subsumption` after assignment, because the widened bounds of a linear
  sum still overlap the right-hand side.

The full list of 452 names, seeds, and replayed problems is in
`results/classify-p1.tsv`; P1h's 116 are in `results/classify-p1h.tsv`.

### P2

7 failures [C5], all class H; P2h passes all 1561 tests [C7]:

| test | replay | problem |
|---|---|---|
| Float::Basic | `-replay 1459051619` | Test::Base, fails like P1's |
| Float::Arithmetic::Max::Bin::XYZ::Normal | `-replay 800396817` | Prune / Failed on solution |
| Float::Arithmetic::Max::Nary | `-replay 1471113749` | Prune / Failed on solution |
| Float::Arithmetic::Max::Nary::Shared | `-replay 1290022630` | Prune / Failed on solution |
| Float::Arithmetic::Min::Bin::XYZ::Normal | `-replay 1354576396` | Prune / Failed on solution |
| Float::Arithmetic::Min::Nary | `-replay 175127460` | Prune / Failed on solution |
| Float::Arithmetic::Min::Nary::Shared | `-replay 1081987068` | Prune / Failed on solution |

Every assignment contains `[0, 4.94066e-324]`. In the
`Min::Bin::XYZ::Normal` replay the harness posts `x[0] >= 9.88131e-324` for
the assignment `{[0,4.94066e-324],...}`.

## Examples

The unmodified example binaries with default options: all solutions, the
default step, BAB [C8].

### Statistics

`-mode stat`, run twice per build; both runs agreed in every build.

| example | build | solutions | propagations | nodes | failures | peak depth |
|---|---|---|---|---|---|---|
| archimedean-spiral | ref | 189 | 118785 | 10252 | 1738 | 1078 |
| | P1 | 189 | 207745 | 10252 | 1738 | 1078 |
| | P2 | 189 | 118787 | 10252 | 1738 | 1078 |
| cartesian-heart | ref | 572 | 3556954 | 28112 | 4185 | 1075 |
| | P1 | 572 | 3867771 | 30158 | 5173 | 1078 |
| | P2 | 572 | 3556955 | 28112 | 4185 | 1075 |
| descartes-folium | ref | 390 | 1813641 | 19070 | 3343 | 61 |
| | P1 | 390 | 3147399 | 19065 | 3341 | 61 |
| | P2 | 390 | 1813641 | 19070 | 3343 | 61 |
| golden-spiral | ref | 98 | 62384 | 5910 | 965 | 1077 |
| | P1 | 98 | 108394 | 5910 | 965 | 1077 |
| | P2 | 98 | 62386 | 5910 | 965 | 1077 |

### Printed solutions

Diff of the `XY` lines of `-mode solution` (six significant digits):

| example | P1 | P2 |
|---|---|---|
| archimedean-spiral | 1 line: `XY 0 0` becomes `XY -4.94066e-324 0` | 1 line: `XY 0 0` becomes `XY 9.88131e-324 0` |
| cartesian-heart | 4 lines, for example `XY -0.976562 0.646299` becomes `XY -0.976563 0.646299` | identical |
| descartes-folium | identical | identical |
| golden-spiral | 1 line: `XY 1 1.60812e-16` becomes `XY 1 -2.83277e-16` | identical |

## Runtime

### Examples

`runtime:` of `-mode stat`, 11 interleaved rounds per build [C8]. The
minimum and the median, in ms:

| example | ref min (median) | P1 min (median) | P2 min (median) | P1 / ref | P2 / ref |
|---|---|---|---|---|---|
| archimedean-spiral | 223.6 (224.8) | 239.9 (240.6) | 218.7 (219.3) | 1.073 | 0.978 |
| cartesian-heart | 337.3 (338.7) | 528.3 (529.8) | 207.4 (208.3) | 1.566 | 0.615 |
| descartes-folium | 1406.0 (1407.3) | 1430.1 (1432.1) | 2067.9 (2070.8) | 1.017 | 1.471 |
| golden-spiral | 160.9 (161.9) | 176.8 (177.4) | 157.4 (158.2) | 1.099 | 0.979 |

A runtime difference mixes the cost per operation with any change in search.
P1 runs more propagations in every example and more nodes in cartesian-heart.
P2 has the reference's search (up to 2 extra propagations), so its ratios
are per-operation cost.

Profiles of descartes-folium, one second each [C11], counted at the top of
the stack (only frames with at least 5 samples are listed by `sample`):

- ref: `fesetround` 420 of 577 samples.
- P1: `nextafter` 274 and its stub 95 of 628.
- P2: a folded function in `libgecodefloat` 364, the interval `operator-`
  and `operator*` 185, `nextafter` and its stub 76, of 684. The callers are
  Boost's `root_aux` (interval Newton iteration for `nth_root`) and `pow`.

Why P2's error-term code is that much slower than a single upward-rounded
operation in these loops was not investigated further.

### Microbenchmark

`bench-<v> 10000000`, minimum of 7 interleaved runs, ns per iteration [C9]:

| variant | FloatVal loop | all samples | Rounding loop | all samples |
|---|---|---|---|---|
| ref | 199.9 | 200.1, 200.2, 200.1, 200.0, 199.9, 199.9, 200.6 | 43.0 | 43.1, 43.2, 43.1, 43.0, 43.1, 43.0, 43.1 |
| P1 | 62.0 | 62.1, 62.2, 62.0, 62.0, 62.0, 62.0, 62.1 | 37.3 | 37.4, 37.5, 37.4, 37.5, 37.3, 37.4, 37.4 |
| P1-nb | 59.5 | 59.6, 59.5, 59.5, 59.5, 59.7, 59.5, 59.6 | 37.7 | 37.9, 37.7, 38.6, 37.8, 37.8, 37.9, 37.8 |
| P2 | 60.5 | 60.5, 60.5, 60.5, 60.6, 60.6, 60.6, 60.6 | 34.2 | 34.2, 34.3, 34.2, 34.3, 34.2, 34.2, 34.3 |

The FloatVal loop opens a scope for every operation, so it measures the mode
switch: P1 takes 31% of ref's time. The Rounding loop opens one scope, but
its recurrence does not stay finite: the final values that [C9] prints are
NaN for ref and infinity for P1, P1-nb, and P2 (`results/bench.txt`). How
many timed iterations run on non-finite values was not measured, so the
column does not show the cost of ordinary arithmetic. The benchmark's operands are not exact, so P2 calls `nextafter` on one side of
most operations.

### Float suite

One single-threaded run per build, one after another, after the example and
microbenchmark timing [C10]. Each build was run once, so these times are
indicative only. The runtime evidence is the examples and the
microbenchmark.

| build | passed | failed | elapsed | `time -p` user | load average, before / after |
|---|---|---|---|---|---|
| ref | 1561 | 0 | 1264.6 s | 1260.5 s | 4.42 / 4.22 |
| P1 | 1113 | 448 | 216.8 s | 216.5 s | 4.22 / 4.18 |
| P2 | 1554 | 7 | 1124.3 s | 1120.6 s | 4.18 / 4.30 |

`elapsed` is the suite's `Elapsed time:` line. P2 took 0.889 times as long
as ref. P1's time is not comparable, because a failing test stops at its
first failure.

The test inputs come from a random generator, so the failing set depends on
the seeds. P2 failed the same 7 tests as with 8 threads. P1 failed 448: 431
of the 8-thread run's 452, and 17 others. All 17 others were replayed with
`build-p1/bin/gecode-test -replay <seed> -test <name> -log`
(`results/classify-p1-t1only.tsv`). All of them are `Prune` /
`Failed on solution` (12) or `Prune reified` / `Failed` (5), in the
Linear, Rel, PowBaseConsistency, and Sin families, and none fails under P1h
[C7]. The `suite <b> exit` lines in `results/timed.status` record the
status of `uptime`, not of the test run; the pass and fail counts come from
the logs.

## Barrier check

P1 and P1-nb differ only in whether `barrier()` hides values from the
optimizer. The harness output of P1 and P1-nb is byte-identical
(`cmp tightness/p1.tsv tightness/p1nb.tsv`), all 263488 rows in all four
caller modes [C4]. The probe matrix gives the same counts for both in all
nine configurations [C3]. The microbenchmark runs 59.5 against 62.0 ns
(FloatVal) and 37.7 against 37.3 ns (Rounding, which ends non-finite)
[C9]. Without mode changes,
folding a constant or sharing one result between the two directions gives
the same nearest value as evaluating at runtime, and P1 is sound for either
neighbour, so the barrier changes no result here. This was checked with
AppleClang 21 for the harness and with all three compilers for the probe; the
library itself is built with the barrier in both cases.

## Mode-dependent code left unchanged

Plain arithmetic inside a `Float::Rounding` scope runs in upward mode in the
reference and in the caller's mode in the prototypes. Neither prototype
changes it:

- `gecode/float/trigonometric/tanatan.hpp:109-110`:
  `x0.min() + pi_half_upper()` and `x0.max() + pi_half_upper()` feed
  `r.div_up`.
- `gecode/float/trigonometric/sincos.hpp:44-45`: the macros `IPI_3PI_2I` and
  `I3PI_2_2PII` compute `3 * pi_half_upper()` and `3 * pi_half_lower()`.
- `test/float.cpp:109-126`, used at lines 327, 332, 359, and 368: the
  harness's random cut values use `Float::Rounding` and depend on exact
  results staying exact (see Float suite).

`Rounding r` scopes exist in `arithmetic/mult.hpp`, `pow-nroot.hpp`,
`sqr-sqrt.hpp`, `linear/nary.hpp` (2), `trigonometric/sincos.hpp` (2),
`tanatan.hpp`, and `float/val.hpp`; a read of each found no other plain
arithmetic whose rounding direction matters.

## Consequences for approach 3

Facts only; choosing the approach is the user's decision in boost-004.

- Plain nextafter (P1) is sound on this machine and needs no compiler flag,
  pragma, or barrier for soundness. It changes float propagation semantics:
  almost every interval is wider, the examples' search and printed solutions
  change, and the Float:: suite needs changes. The brief requires the user
  to accept such a loss of tightness before it ships.
- Making the suite pass with P1 needs: the test harness's cut
  generation must not assume exact results (336 tests); the 4 tightness tests
  must change or be dropped; and 112 subsumption and fixpoint checks fail
  (MiniModel::LinRel, Max and Min, and 15 arithmetic and transcendental
  propagator families). Those last are propagator behaviour, not test code.
- An error-term variant (P2) reproduces the reference's bounds and
  search, apart from edge cases near zero, underflow, overflow, and infinite
  operands. It passes the suite once the harness's cuts are clamped. Its
  underflow fallback produces bounds on the wrong side of zero (for example
  `-denorm_min` as the lower bound of a positive product), which is sound but
  loose, and which the harness's cut generation trips over (7 tests).
- Both prototypes remove the mode switch from every `FloatVal` operation,
  which is most of the cost of a `FloatVal` operation today. The example
  runtimes do not follow the microbenchmark: P1 runs more propagations and
  is slower in every example (1.017x to 1.566x), and P2 is faster in three
  examples (0.615x to 0.979x) and slower in descartes-folium (1.471x), whose
  profile is dominated by Boost's `pow` and interval Newton `root_aux`.
- Neither prototype depends on the caller's mode for soundness, so a library
  built this way would not need to restore the mode, and a caller in any mode
  gets sound bounds.
- `std::nextafter` stays a library call under AppleClang -O3. A bit-pattern
  increment would avoid it; that is not measured here.

## Not verified

- x86-64 (GCC and Clang), MSVC, and the cost of `std::fma` without hardware
  FMA.
- A library built by GCC or LLVM clang, and builds without MPFR.
- Debug builds of the suite: the harness's `assert` at `test/float.cpp:362`
  and `:371` would stop the P1 and P2 runs at the class H cuts.
- Whether P2's sign-of-zero handling in its fallback can be made tight
  without more branches; a sign-aware fallback was not built.
- Why P2's per-operation code is slower than the reference's in Boost's
  interval Newton loop.
- Float tests outside `Float::`, such as the FlatZinc float tests, were not
  run.

## Appendix A: prototype patches

Made from source copies of 3f4b5272f9 (boost-006) with `make_proto.py`.

### A.1 P1

`diff -ru src-head src-p1`:

```diff
diff -ru src-head/gecode/float/rounding.hpp src-p1/gecode/float/rounding.hpp
--- src-head/gecode/float/rounding.hpp
+++ src-p1/gecode/float/rounding.hpp
@@ -46,14 +46,18 @@
             return u;
 
         const FloatNum h = max / 2.0;
+        // Prototype: no mode switch, computed in the caller's mode.
         if ((u > h) || (l < -h))
-            return std::ldexp(RoundingControl::median(l / 2.0, u / 2.0), 1);
-        return RoundingControl::median(l, u);
+            return std::ldexp((l / 2.0 + u / 2.0) / 2.0, 1);
+        return (l + u) / 2.0;
     }
 
     forceinline FloatNum
     RoundingBase::barrier(FloatNum x) {
         // See the RoundingBase class comment for what this relies on.
+#ifdef GECODE_PROTOTYPE_NO_BARRIER
+        return x;
+#endif
 #if (defined(__GNUC__) || defined(__clang__)) && defined(__aarch64__)
         __asm__ __volatile__("" : "+w"(x));
 #elif (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
@@ -65,108 +69,59 @@
         return x;
     }
 
-#if defined(_MSC_VER) && (defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP > 0)))
-
-    // RoundingControl is rounded_arith_std: each operation sets its direction.
     forceinline FloatNum
-    RoundingBase::add_down(FloatNum x, FloatNum y) {
-        RoundingControl::downward();
-        return barrier(barrier(x) + barrier(y));
+    RoundingBase::next_down(FloatNum x) {
+        return std::nextafter(x, -std::numeric_limits<FloatNum>::infinity());
     }
     forceinline FloatNum
-    RoundingBase::add_up(FloatNum x, FloatNum y) {
-        RoundingControl::upward();
-        return barrier(barrier(x) + barrier(y));
+    RoundingBase::next_up(FloatNum x) {
+        return std::nextafter(x, std::numeric_limits<FloatNum>::infinity());
     }
-    forceinline FloatNum
-    RoundingBase::sub_down(FloatNum x, FloatNum y) {
-        RoundingControl::downward();
-        return barrier(barrier(x) - barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::sub_up(FloatNum x, FloatNum y) {
-        RoundingControl::upward();
-        return barrier(barrier(x) - barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::mul_down(FloatNum x, FloatNum y) {
-        RoundingControl::downward();
-        return barrier(barrier(x) * barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::mul_up(FloatNum x, FloatNum y) {
-        RoundingControl::upward();
-        return barrier(barrier(x) * barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::div_down(FloatNum x, FloatNum y) {
-        RoundingControl::downward();
-        return barrier(barrier(x) / barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::div_up(FloatNum x, FloatNum y) {
-        RoundingControl::upward();
-        return barrier(barrier(x) / barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::sqrt_down(FloatNum x) {
-        RoundingControl::downward();
-        return barrier(std::sqrt(barrier(x)));
-    }
-    forceinline FloatNum
-    RoundingBase::sqrt_up(FloatNum x) {
-        RoundingControl::upward();
-        return barrier(std::sqrt(barrier(x)));
-    }
 
-#else
-
-    // RoundingControl is rounded_arith_opp: the mode stays upward.
+    // Prototype P1: round to nearest in the caller's mode, then move the
+    // result one ulp outward. No rounding-mode access.
     forceinline FloatNum
     RoundingBase::add_down(FloatNum x, FloatNum y) {
-        return -barrier(barrier(-x) - barrier(y));
+        return next_down(barrier(barrier(x) + barrier(y)));
     }
     forceinline FloatNum
     RoundingBase::add_up(FloatNum x, FloatNum y) {
-        return barrier(barrier(x) + barrier(y));
+        return next_up(barrier(barrier(x) + barrier(y)));
     }
     forceinline FloatNum
     RoundingBase::sub_down(FloatNum x, FloatNum y) {
-        return -barrier(barrier(y) - barrier(x));
+        return next_down(barrier(barrier(x) - barrier(y)));
     }
     forceinline FloatNum
     RoundingBase::sub_up(FloatNum x, FloatNum y) {
-        return barrier(barrier(x) - barrier(y));
+        return next_up(barrier(barrier(x) - barrier(y)));
     }
     forceinline FloatNum
     RoundingBase::mul_down(FloatNum x, FloatNum y) {
-        return -barrier(barrier(x) * barrier(-y));
+        return next_down(barrier(barrier(x) * barrier(y)));
     }
     forceinline FloatNum
     RoundingBase::mul_up(FloatNum x, FloatNum y) {
-        return barrier(barrier(x) * barrier(y));
+        return next_up(barrier(barrier(x) * barrier(y)));
     }
     forceinline FloatNum
     RoundingBase::div_down(FloatNum x, FloatNum y) {
-        return -barrier(barrier(x) / barrier(-y));
+        return next_down(barrier(barrier(x) / barrier(y)));
     }
     forceinline FloatNum
     RoundingBase::div_up(FloatNum x, FloatNum y) {
-        return barrier(barrier(x) / barrier(y));
+        return next_up(barrier(barrier(x) / barrier(y)));
     }
     forceinline FloatNum
     RoundingBase::sqrt_down(FloatNum x) {
-        RoundingControl::downward();
+        // The lower bound of a square root never goes below zero.
         const FloatNum r = barrier(std::sqrt(barrier(x)));
-        RoundingControl::upward();
-        return r;
+        return (r > 0.0) ? next_down(r) : r;
     }
     forceinline FloatNum
     RoundingBase::sqrt_up(FloatNum x) {
-        return barrier(std::sqrt(barrier(x)));
+        return next_up(barrier(std::sqrt(barrier(x))));
     }
-
-#endif
 
     forceinline FloatNum
     RoundingBase::int_down(FloatNum x) {
diff -ru src-head/gecode/float.hh src-p1/gecode/float.hh
--- src-head/gecode/float.hh
+++ src-p1/gecode/float.hh
@@ -180,6 +180,10 @@
     private:
         /// Return \a x unchanged, hidden from the optimizer (see the class comment)
         static FloatNum barrier(FloatNum x);
+        /// Return the next double below \a x (prototype)
+        static FloatNum next_down(FloatNum x);
+        /// Return the next double above \a x (prototype)
+        static FloatNum next_up(FloatNum x);
 
     public:
         /// \name Arithmetic operations
@@ -302,7 +306,7 @@
      * on destruction. Nested policies restore the enclosing policy's mode.
      * \ingroup TaskModelFloatVars
      */
-    class Rounding : public gecode_boost::numeric::interval_lib::save_state<RoundingBase> {};
+    class Rounding : public gecode_boost::numeric::interval_lib::save_state_nothing<RoundingBase> {};
 
 }} // namespace Gecode::Float
```

### A.2 P2

The comments in the prototype code below say that the error terms are
exact and that Fast2Sum is exact once the operands are ordered. That holds
in round to nearest only. Under directed caller modes Fast2Sum gives only
the sign P2 relies on, as Prototypes explains. The code is kept verbatim.

`diff -ru src-head src-p2`:

```diff
diff -ru src-head/gecode/float/rounding.hpp src-p2/gecode/float/rounding.hpp
--- src-head/gecode/float/rounding.hpp
+++ src-p2/gecode/float/rounding.hpp
@@ -46,14 +46,18 @@
             return u;
 
         const FloatNum h = max / 2.0;
+        // Prototype: no mode switch, computed in the caller's mode.
         if ((u > h) || (l < -h))
-            return std::ldexp(RoundingControl::median(l / 2.0, u / 2.0), 1);
-        return RoundingControl::median(l, u);
+            return std::ldexp((l / 2.0 + u / 2.0) / 2.0, 1);
+        return (l + u) / 2.0;
     }
 
     forceinline FloatNum
     RoundingBase::barrier(FloatNum x) {
         // See the RoundingBase class comment for what this relies on.
+#ifdef GECODE_PROTOTYPE_NO_BARRIER
+        return x;
+#endif
 #if (defined(__GNUC__) || defined(__clang__)) && defined(__aarch64__)
         __asm__ __volatile__("" : "+w"(x));
 #elif (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
@@ -65,108 +69,138 @@
         return x;
     }
 
-#if defined(_MSC_VER) && (defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP > 0)))
-
-    // RoundingControl is rounded_arith_std: each operation sets its direction.
     forceinline FloatNum
-    RoundingBase::add_down(FloatNum x, FloatNum y) {
-        RoundingControl::downward();
-        return barrier(barrier(x) + barrier(y));
+    RoundingBase::next_down(FloatNum x) {
+        return std::nextafter(x, -std::numeric_limits<FloatNum>::infinity());
     }
     forceinline FloatNum
-    RoundingBase::add_up(FloatNum x, FloatNum y) {
-        RoundingControl::upward();
-        return barrier(barrier(x) + barrier(y));
+    RoundingBase::next_up(FloatNum x) {
+        return std::nextafter(x, std::numeric_limits<FloatNum>::infinity());
     }
-    forceinline FloatNum
-    RoundingBase::sub_down(FloatNum x, FloatNum y) {
-        RoundingControl::downward();
-        return barrier(barrier(x) - barrier(y));
+
+    // Prototype P2: round to nearest in the caller's mode, compute the
+    // rounding error exactly, and move one ulp outward only on the side the
+    // error requires. Falls back to P1's widening where the error term may
+    // not be exact.
+    namespace PrototypeP2 {
+      /// Smallest magnitude for which the error terms below are exact
+      constexpr FloatNum tiny = 0x1p-969;
+      forceinline bool
+      small(FloatNum x) {
+        return (x != 0.0) && (std::fabs(x) < tiny);
+      }
+      /// Error of x+y (sign only matters): exact for |x| >= |y| (Fast2Sum)
+      forceinline FloatNum
+      add_err(FloatNum x, FloatNum y, FloatNum s) {
+        if (std::fabs(x) < std::fabs(y))
+          std::swap(x, y);
+        const FloatNum z = s - x;
+        return y - z;
+      }
     }
-    forceinline FloatNum
-    RoundingBase::sub_up(FloatNum x, FloatNum y) {
-        RoundingControl::upward();
-        return barrier(barrier(x) - barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::mul_down(FloatNum x, FloatNum y) {
-        RoundingControl::downward();
-        return barrier(barrier(x) * barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::mul_up(FloatNum x, FloatNum y) {
-        RoundingControl::upward();
-        return barrier(barrier(x) * barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::div_down(FloatNum x, FloatNum y) {
-        RoundingControl::downward();
-        return barrier(barrier(x) / barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::div_up(FloatNum x, FloatNum y) {
-        RoundingControl::upward();
-        return barrier(barrier(x) / barrier(y));
-    }
-    forceinline FloatNum
-    RoundingBase::sqrt_down(FloatNum x) {
-        RoundingControl::downward();
-        return barrier(std::sqrt(barrier(x)));
-    }
-    forceinline FloatNum
-    RoundingBase::sqrt_up(FloatNum x) {
-        RoundingControl::upward();
-        return barrier(std::sqrt(barrier(x)));
-    }
 
-#else
-
-    // RoundingControl is rounded_arith_opp: the mode stays upward.
     forceinline FloatNum
     RoundingBase::add_down(FloatNum x, FloatNum y) {
-        return -barrier(barrier(-x) - barrier(y));
+      x = barrier(x); y = barrier(y);
+      const FloatNum s = barrier(x + y);
+      if (!std::isfinite(s) || PrototypeP2::small(x) || PrototypeP2::small(y)
+          || PrototypeP2::small(s))
+        return next_down(s);
+      const FloatNum e = barrier(PrototypeP2::add_err(x, y, s));
+      return (e < 0.0) ? next_down(s) : s;
     }
     forceinline FloatNum
     RoundingBase::add_up(FloatNum x, FloatNum y) {
-        return barrier(barrier(x) + barrier(y));
+      x = barrier(x); y = barrier(y);
+      const FloatNum s = barrier(x + y);
+      if (!std::isfinite(s) || PrototypeP2::small(x) || PrototypeP2::small(y)
+          || PrototypeP2::small(s))
+        return next_up(s);
+      const FloatNum e = barrier(PrototypeP2::add_err(x, y, s));
+      return (e > 0.0) ? next_up(s) : s;
     }
     forceinline FloatNum
     RoundingBase::sub_down(FloatNum x, FloatNum y) {
-        return -barrier(barrier(y) - barrier(x));
+      return add_down(x, -y);
     }
     forceinline FloatNum
     RoundingBase::sub_up(FloatNum x, FloatNum y) {
-        return barrier(barrier(x) - barrier(y));
+      return add_up(x, -y);
     }
     forceinline FloatNum
     RoundingBase::mul_down(FloatNum x, FloatNum y) {
-        return -barrier(barrier(x) * barrier(-y));
+      x = barrier(x); y = barrier(y);
+      const FloatNum p = barrier(x * y);
+      if ((x == 0.0) || (y == 0.0))
+        return p;
+      if (!std::isfinite(p) || PrototypeP2::small(x) || PrototypeP2::small(y)
+          || (std::fabs(p) < PrototypeP2::tiny))
+        return next_down(p);
+      const FloatNum e = barrier(std::fma(x, y, -p));
+      return (e < 0.0) ? next_down(p) : p;
     }
     forceinline FloatNum
     RoundingBase::mul_up(FloatNum x, FloatNum y) {
-        return barrier(barrier(x) * barrier(y));
+      x = barrier(x); y = barrier(y);
+      const FloatNum p = barrier(x * y);
+      if ((x == 0.0) || (y == 0.0))
+        return p;
+      if (!std::isfinite(p) || PrototypeP2::small(x) || PrototypeP2::small(y)
+          || (std::fabs(p) < PrototypeP2::tiny))
+        return next_up(p);
+      const FloatNum e = barrier(std::fma(x, y, -p));
+      return (e > 0.0) ? next_up(p) : p;
     }
     forceinline FloatNum
     RoundingBase::div_down(FloatNum x, FloatNum y) {
-        return -barrier(barrier(x) / barrier(-y));
+      x = barrier(x); y = barrier(y);
+      const FloatNum q = barrier(x / y);
+      if (x == 0.0)
+        return q;
+      if (!std::isfinite(q) || !std::isfinite(y) || PrototypeP2::small(x)
+          || PrototypeP2::small(y) || (std::fabs(q) < PrototypeP2::tiny))
+        return next_down(q);
+      FloatNum e = barrier(std::fma(-q, y, x));
+      if (y < 0.0)
+        e = -e;
+      return (e < 0.0) ? next_down(q) : q;
     }
     forceinline FloatNum
     RoundingBase::div_up(FloatNum x, FloatNum y) {
-        return barrier(barrier(x) / barrier(y));
+      x = barrier(x); y = barrier(y);
+      const FloatNum q = barrier(x / y);
+      if (x == 0.0)
+        return q;
+      if (!std::isfinite(q) || !std::isfinite(y) || PrototypeP2::small(x)
+          || PrototypeP2::small(y) || (std::fabs(q) < PrototypeP2::tiny))
+        return next_up(q);
+      FloatNum e = barrier(std::fma(-q, y, x));
+      if (y < 0.0)
+        e = -e;
+      return (e > 0.0) ? next_up(q) : q;
     }
     forceinline FloatNum
     RoundingBase::sqrt_down(FloatNum x) {
-        RoundingControl::downward();
-        const FloatNum r = barrier(std::sqrt(barrier(x)));
-        RoundingControl::upward();
+      x = barrier(x);
+      const FloatNum r = barrier(std::sqrt(x));
+      if (!(r > 0.0))
         return r;
+      if (!std::isfinite(r) || PrototypeP2::small(x))
+        return next_down(r);
+      const FloatNum e = barrier(std::fma(-r, r, x));
+      return (e < 0.0) ? next_down(r) : r;
     }
     forceinline FloatNum
     RoundingBase::sqrt_up(FloatNum x) {
-        return barrier(std::sqrt(barrier(x)));
+      x = barrier(x);
+      const FloatNum r = barrier(std::sqrt(x));
+      if (!(r > 0.0))
+        return r;
+      if (!std::isfinite(r) || PrototypeP2::small(x))
+        return next_up(r);
+      const FloatNum e = barrier(std::fma(-r, r, x));
+      return (e > 0.0) ? next_up(r) : r;
     }
-
-#endif
 
     forceinline FloatNum
     RoundingBase::int_down(FloatNum x) {
diff -ru src-head/gecode/float.hh src-p2/gecode/float.hh
--- src-head/gecode/float.hh
+++ src-p2/gecode/float.hh
@@ -180,6 +180,10 @@
     private:
         /// Return \a x unchanged, hidden from the optimizer (see the class comment)
         static FloatNum barrier(FloatNum x);
+        /// Return the next double below \a x (prototype)
+        static FloatNum next_down(FloatNum x);
+        /// Return the next double above \a x (prototype)
+        static FloatNum next_up(FloatNum x);
 
     public:
         /// \name Arithmetic operations
@@ -302,7 +306,7 @@
      * on destruction. Nested policies restore the enclosing policy's mode.
      * \ingroup TaskModelFloatVars
      */
-    class Rounding : public gecode_boost::numeric::interval_lib::save_state<RoundingBase> {};
+    class Rounding : public gecode_boost::numeric::interval_lib::save_state_nothing<RoundingBase> {};
 
 }} // namespace Gecode::Float
```

### A.3 Clamped harness cuts

`diff -u src-p1/test/float.cpp src-p1h/test/float.cpp`; `src-p2h` has the
same change:

```diff
--- src-p1/test/float.cpp
+++ src-p1h/test/float.cpp
@@ -358,6 +358,9 @@
             if (a[i].max() < x[i].max()) {
                 Gecode::FloatNum v = rand_f_val_down(a[i].max(), x[i].max(), rand);
                 if (v == x[i].max())
+                    v = a[i].max();
+                // Diagnostic: keep the cut on the assignment's side.
+                if (v < a[i].max())
                     v = a[i].max();
                 assert((v >= a[i].max()) && (v <= x[i].max()));
                 rel(i, Gecode::FRT_LQ, v);
@@ -368,6 +371,9 @@
                 Gecode::FloatNum v = rand_f_val_up(x[i].min(), a[i].min(), rand);
                 if (v == x[i].min())
                     v = a[i].min();
+                // Diagnostic: keep the cut on the assignment's side.
+                if (v > a[i].min())
+                    v = a[i].min();
                 assert((v <= a[i].min()) && (v >= x[i].min()));
                 rel(i, Gecode::FRT_GQ, v);
             }
```

## Appendix B: tightness harness

### B.1 Harness

`tightness/tightness.cpp`:

```cpp
// Tightness and soundness harness for the boost-002 nextafter study.
//
// Prints one TSV row per case: id, level, op, class, mode, x, y, down, up,
// oracle_down, oracle_up, verdict. Values are hexfloat; "-" marks a missing
// field. The oracle is MPFR at 53 bits (RNDD/RNDU), which does not depend on
// the hardware rounding mode. All random choices use a fixed seed, and no
// operand is computed through Gecode, so every build sees the same cases.
#include <gecode/float.hh>

#include <mpfr.h>

#include <algorithm>
#include <bit>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <random>
#include <string>
#include <vector>

using Gecode::FloatNum;
using Gecode::FloatVal;

namespace {

constexpr double inf = std::numeric_limits<double>::infinity();

enum class Op { add, sub, mul, div, sqrt };
const char* op_name[] = {"add", "sub", "mul", "div", "sqrt"};

struct Case {
  Op op;
  std::string cls;
  double x, y;
};

std::mt19937_64 rng(20261102);

std::uint64_t bits(int n) { return rng() >> (64 - n); }

// Random sign, 53-bit mantissa, exponent in [lo, hi].
double random_double(int lo, int hi) {
  std::uint64_t mant = bits(52);
  int e = lo + static_cast<int>(bits(32) % static_cast<std::uint64_t>(hi - lo + 1));
  std::uint64_t b = (static_cast<std::uint64_t>(e + 1023) << 52) | mant;
  double d = std::bit_cast<double>(b);
  return bits(1) ? -d : d;
}

long random_int(long lo, long hi) {
  return lo + static_cast<long>(bits(40) % static_cast<std::uint64_t>(hi - lo + 1));
}

// Uniform double in [lo, hi), computed exactly enough to be just data.
double uniform(double lo, double hi) {
  double u = static_cast<double>(bits(53)) * 0x1p-53;
  return lo + (hi - lo) * u;
}

double dyadic(double bound) {
  int j = static_cast<int>(random_int(0, 30));
  long k = random_int(static_cast<long>(-bound * std::ldexp(1.0, j)),
                      static_cast<long>(bound * std::ldexp(1.0, j)));
  return std::ldexp(static_cast<double>(k), -j);
}

using MpfrOp = int (*)(mpfr_ptr, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);

double mpfr_bin(MpfrOp f, double x, double y, mpfr_rnd_t r) {
  mpfr_t a, b, c;
  mpfr_inits2(53, a, b, c, static_cast<mpfr_ptr>(nullptr));
  mpfr_set_d(a, x, MPFR_RNDN);
  mpfr_set_d(b, y, MPFR_RNDN);
  f(c, a, b, r);
  double d = mpfr_get_d(c, r);
  mpfr_clears(a, b, c, static_cast<mpfr_ptr>(nullptr));
  return d;
}

using MpfrUn = int (*)(mpfr_ptr, mpfr_srcptr, mpfr_rnd_t);

double mpfr_un(MpfrUn f, double x, mpfr_rnd_t r) {
  mpfr_t a, c;
  mpfr_inits2(53, a, c, static_cast<mpfr_ptr>(nullptr));
  mpfr_set_d(a, x, MPFR_RNDN);
  f(c, a, r);
  double d = mpfr_get_d(c, r);
  mpfr_clears(a, c, static_cast<mpfr_ptr>(nullptr));
  return d;
}

double mpfr_cube(double x, mpfr_rnd_t r) {
  mpfr_t a, c;
  mpfr_inits2(53, a, c, static_cast<mpfr_ptr>(nullptr));
  mpfr_set_d(a, x, MPFR_RNDN);
  mpfr_pow_ui(c, a, 3, r);
  double d = mpfr_get_d(c, r);
  mpfr_clears(a, c, static_cast<mpfr_ptr>(nullptr));
  return d;
}

double oracle(Op op, double x, double y, mpfr_rnd_t r) {
  switch (op) {
    case Op::add: return mpfr_bin(mpfr_add, x, y, r);
    case Op::sub: return mpfr_bin(mpfr_sub, x, y, r);
    case Op::mul: return mpfr_bin(mpfr_mul, x, y, r);
    case Op::div: return mpfr_bin(mpfr_div, x, y, r);
    case Op::sqrt: return mpfr_un(mpfr_sqrt, x, r);
  }
  return 0.0;
}

const char* verdict(double down, double up, double od, double ou) {
  if (std::isnan(down) || std::isnan(up))
    return "nan";
  if ((down > od) || (up < ou))
    return "unsound";
  if ((down < od) || (up > ou))
    return "loose";
  return "exact";
}

void hex(std::string& s, double d) {
  char buf[64];
  std::snprintf(buf, sizeof buf, "%a", d);
  s += buf;
}

void row(const std::string& id, const char* level, const char* op, const std::string& cls,
         const char* mode, double x, double y, bool has_y, double down, double up, bool has_oracle,
         double od, double ou) {
  std::string s = id + "\t" + level + "\t" + op + "\t" + cls + "\t" + mode + "\t";
  hex(s, x);
  s += "\t";
  if (has_y)
    hex(s, y);
  else
    s += "-";
  s += "\t";
  hex(s, down);
  s += "\t";
  hex(s, up);
  s += "\t";
  if (has_oracle) {
    hex(s, od);
    s += "\t";
    hex(s, ou);
    s += "\t";
    s += verdict(down, up, od, ou);
  } else {
    s += "-\t-\t-";
  }
  std::puts(s.c_str());
}

bool oracle_valid(Op op, double x, double y) {
  double d = oracle(op, x, y, MPFR_RNDD);
  if (std::isnan(d))
    return false;
  if ((op == Op::div) && (y == 0.0))
    return false;
  return true;
}

std::vector<Case> scalar_cases() {
  std::vector<Case> cs;
  const Op ops[] = {Op::add, Op::sub, Op::mul, Op::div, Op::sqrt};
  for (Op op : ops) {
    for (int i = 0; i < 10000; i++) {
      double x = random_double(-20, 20), y = random_double(-20, 20);
      if (op == Op::sqrt)
        x = std::fabs(x);
      cs.push_back({op, "random", x, y});
    }
    for (int i = 0; i < 2000; i++) {
      double x = static_cast<double>(random_int(-1000, 1000));
      double y = static_cast<double>(random_int(-1000, 1000));
      if (op == Op::div) {
        if (y == 0.0)
          y = 7.0;
        x = static_cast<double>(random_int(-1000, 1000)) * y;
      }
      if (op == Op::sqrt) {
        double k = static_cast<double>(random_int(0, 1000));
        x = k * k;
      }
      cs.push_back({op, "int", x, y});
    }
    for (int i = 0; i < 2000; i++) {
      double x = dyadic(20.0), y = dyadic(20.0);
      if ((op == Op::div) && (y == 0.0))
        y = 0.5;
      if (op == Op::sqrt)
        x = std::fabs(x);
      cs.push_back({op, "dyadic", x, y});
    }
    for (int i = 0; i < 200; i++) {
      double x = random_double(-20, 20), y = random_double(-20, 20);
      switch (op) {
        case Op::add: cs.push_back({op, "zero", x, -x}); break;
        case Op::sub: cs.push_back({op, "zero", x, x}); break;
        case Op::mul: cs.push_back({op, "zero", (i & 1) ? -0.0 : 0.0, y}); break;
        case Op::div: cs.push_back({op, "zero", (i & 1) ? -0.0 : 0.0, y}); break;
        case Op::sqrt:
          if (i < 2)
            cs.push_back({op, "zero", (i & 1) ? -0.0 : 0.0, 0.0});
          break;
      }
    }
    const double dmax = std::numeric_limits<double>::max();
    const double dmin = std::numeric_limits<double>::min();
    const double dden = std::numeric_limits<double>::denorm_min();
    const double special[] = {0.0,    -0.0,    dden,     -dden,     0x1.8p-1070, -0x1.8p-1070,
                              dmin,   -dmin,   0x1.123456789abcdp-1000,          -0x1.123456789abcdp-1000,
                              0x1p-540, 0x1.5555555555555p-540, dmax, -dmax, 0x1.fffp+1023,
                              0x1p+540, 0x1.5555555555555p+540, 1.0, -1.0, 0.1, 3.0, inf, -inf};
    for (double x : special)
      for (double y : special) {
        if ((op == Op::sqrt) && ((y != 0.0) || std::signbit(y) || (x < 0.0)))
          continue;
        if (oracle_valid(op, x, y))
          cs.push_back({op, "edge", x, y});
      }
  }
  return cs;
}

double run_down(Gecode::Float::Rounding& r, Op op, double x, double y) {
  switch (op) {
    case Op::add: return r.add_down(x, y);
    case Op::sub: return r.sub_down(x, y);
    case Op::mul: return r.mul_down(x, y);
    case Op::div: return r.div_down(x, y);
    case Op::sqrt: return r.sqrt_down(x);
  }
  return 0.0;
}

double run_up(Gecode::Float::Rounding& r, Op op, double x, double y) {
  switch (op) {
    case Op::add: return r.add_up(x, y);
    case Op::sub: return r.sub_up(x, y);
    case Op::mul: return r.mul_up(x, y);
    case Op::div: return r.div_up(x, y);
    case Op::sqrt: return r.sqrt_up(x);
  }
  return 0.0;
}

struct Mode {
  int mode;
  const char* name;
};

void scalar_level() {
  std::vector<Case> cs = scalar_cases();
  std::vector<double> od(cs.size()), ou(cs.size());
  for (std::size_t i = 0; i < cs.size(); i++) {
    od[i] = oracle(cs[i].op, cs[i].x, cs[i].y, MPFR_RNDD);
    ou[i] = oracle(cs[i].op, cs[i].x, cs[i].y, MPFR_RNDU);
  }
  const Mode modes[] = {{FE_TONEAREST, "nearest"},
                        {FE_DOWNWARD, "downward"},
                        {FE_UPWARD, "upward"},
                        {FE_TOWARDZERO, "towardzero"}};
  std::vector<double> down(cs.size()), up(cs.size());
  for (const Mode& m : modes) {
    // Index of each case within its op and class, to select the first 1000.
    std::vector<int> nth(cs.size());
    {
      std::string key;
      int n = 0;
      for (std::size_t i = 0; i < cs.size(); i++) {
        std::string k = std::string(op_name[static_cast<int>(cs[i].op)]) + "/" + cs[i].cls;
        n = (k == key) ? n + 1 : 0;
        key = k;
        nth[i] = n;
      }
    }
    std::fesetround(m.mode);
    {
      Gecode::Float::Rounding r;
      for (std::size_t i = 0; i < cs.size(); i++) {
        if ((m.mode != FE_TONEAREST) && (nth[i] >= 1000))
          continue;
        down[i] = run_down(r, cs[i].op, cs[i].x, cs[i].y);
        up[i] = run_up(r, cs[i].op, cs[i].x, cs[i].y);
      }
    }
    std::fesetround(FE_TONEAREST);
    for (std::size_t i = 0; i < cs.size(); i++) {
      if ((m.mode != FE_TONEAREST) && (nth[i] >= 1000))
        continue;
      const char* on = op_name[static_cast<int>(cs[i].op)];
      std::string id = std::string("s/") + on + "/" + cs[i].cls + "/" + std::to_string(nth[i]) + "/" + m.name;
      std::string cls = cs[i].cls + ((od[i] == ou[i]) ? ":exact" : ":inexact");
      row(id, "scalar", on, cls, m.name, cs[i].x, cs[i].y, cs[i].op != Op::sqrt, down[i], up[i], true,
          od[i], ou[i]);
    }
  }
}

// FloatVal level: intervals, with an MPFR hull where one is easy to state.

struct Interval {
  double lo, hi;
};

Interval sorted(double a, double b) { return (a <= b) ? Interval{a, b} : Interval{b, a}; }

Interval random_interval(const std::string& cls, double lo, double hi) {
  if (cls == "random")
    return sorted(uniform(lo, hi), uniform(lo, hi));
  if (cls == "narrow") {
    double c = uniform(lo, hi);
    double w = std::ldexp(1.0, -static_cast<int>(random_int(1, 40)));
    return sorted(c, std::clamp(c + w, lo, hi));
  }
  if (cls == "int")
    return sorted(static_cast<double>(random_int(static_cast<long>(std::ceil(lo)), static_cast<long>(std::floor(hi)))),
                  static_cast<double>(random_int(static_cast<long>(std::ceil(lo)), static_cast<long>(std::floor(hi)))));
  if (cls == "dyadic") {
    double b = std::max(std::fabs(lo), std::fabs(hi));
    double a = std::clamp(dyadic(b), lo, hi), c = std::clamp(dyadic(b), lo, hi);
    return sorted(a, c);
  }
  double p = uniform(lo, hi);  // point
  return {p, p};
}

struct Hull {
  bool valid;
  double lo, hi;
};

Hull hull_bin(Op op, Interval x, Interval y) {
  MpfrOp f = (op == Op::add) ? mpfr_add : (op == Op::sub) ? mpfr_sub : (op == Op::mul) ? mpfr_mul : mpfr_div;
  if (op == Op::add)
    return {true, mpfr_bin(f, x.lo, y.lo, MPFR_RNDD), mpfr_bin(f, x.hi, y.hi, MPFR_RNDU)};
  if (op == Op::sub)
    return {true, mpfr_bin(f, x.lo, y.hi, MPFR_RNDD), mpfr_bin(f, x.hi, y.lo, MPFR_RNDU)};
  double lo = inf, hi = -inf;
  for (double a : {x.lo, x.hi})
    for (double b : {y.lo, y.hi}) {
      lo = std::min(lo, mpfr_bin(f, a, b, MPFR_RNDD));
      hi = std::max(hi, mpfr_bin(f, a, b, MPFR_RNDU));
    }
  return {true, lo, hi};
}

void fv_row(const std::string& id, const char* op, const std::string& cls, Interval x, Interval y, bool has_y,
            const FloatVal& r, Hull h) {
  std::string s = id + "\tfloatval\t" + op + "\t" + cls + "\tnearest\t";
  hex(s, x.lo);
  s += ",";
  hex(s, x.hi);
  s += "\t";
  if (has_y) {
    hex(s, y.lo);
    s += ",";
    hex(s, y.hi);
  } else {
    s += "-";
  }
  s += "\t";
  hex(s, r.min());
  s += "\t";
  hex(s, r.max());
  s += "\t";
  if (h.valid) {
    hex(s, h.lo);
    s += "\t";
    hex(s, h.hi);
    s += "\t";
    s += verdict(r.min(), r.max(), h.lo, h.hi);
  } else {
    s += "-\t-\t-";
  }
  std::puts(s.c_str());
}

void floatval_level() {
  const char* classes[] = {"random", "narrow", "int", "dyadic", "point"};
  const int counts[] = {5000, 2000, 500, 500, 500};
  const char* ops[] = {"add", "sub", "mul", "div", "sqr", "sqrt", "pow3", "nroot2", "abs", "exp", "sin", "cos"};
  for (const char* op : ops) {
    std::string o = op;
    for (int c = 0; c < 5; c++) {
      for (int i = 0; i < counts[c]; i++) {
        std::string cls = classes[c];
        std::string id = "f/" + o + "/" + cls + "/" + std::to_string(i);
        bool nonneg = (o == "sqrt") || (o == "nroot2");
        Interval x = random_interval(cls, nonneg ? 0.0 : -20.0, 20.0);
        Interval y = random_interval(cls, -20.0, 20.0);
        if (o == "div") {
          int guard = 0;
          while ((y.lo <= 0.0) && (y.hi >= 0.0) && (guard++ < 1000))
            y = random_interval(cls, -20.0, 20.0);
          if ((y.lo <= 0.0) && (y.hi >= 0.0))
            y = {1.0, 3.0};
        }
        FloatVal fx(x.lo, x.hi), fy(y.lo, y.hi);
        if ((o == "add") || (o == "sub") || (o == "mul") || (o == "div")) {
          Op bop = (o == "add") ? Op::add : (o == "sub") ? Op::sub : (o == "mul") ? Op::mul : Op::div;
          FloatVal r = (o == "add") ? fx + fy : (o == "sub") ? fx - fy : (o == "mul") ? fx * fy : fx / fy;
          fv_row(id, op, cls, x, y, true, r, hull_bin(bop, x, y));
        } else if (o == "sqr") {
          Hull h;
          if (x.lo >= 0.0)
            h = {true, mpfr_bin(mpfr_mul, x.lo, x.lo, MPFR_RNDD), mpfr_bin(mpfr_mul, x.hi, x.hi, MPFR_RNDU)};
          else if (x.hi <= 0.0)
            h = {true, mpfr_bin(mpfr_mul, x.hi, x.hi, MPFR_RNDD), mpfr_bin(mpfr_mul, x.lo, x.lo, MPFR_RNDU)};
          else
            h = {true, 0.0,
                 std::max(mpfr_bin(mpfr_mul, x.lo, x.lo, MPFR_RNDU), mpfr_bin(mpfr_mul, x.hi, x.hi, MPFR_RNDU))};
          fv_row(id, op, cls, x, y, false, sqr(fx), h);
        } else if ((o == "sqrt") || (o == "nroot2")) {
          Hull h = {true, mpfr_un(mpfr_sqrt, x.lo, MPFR_RNDD), mpfr_un(mpfr_sqrt, x.hi, MPFR_RNDU)};
          fv_row(id, op, cls, x, y, false, (o == "sqrt") ? sqrt(fx) : nroot(fx, 2), h);
        } else if (o == "pow3") {
          fv_row(id, op, cls, x, y, false, pow(fx, 3),
                 {true, mpfr_cube(x.lo, MPFR_RNDD), mpfr_cube(x.hi, MPFR_RNDU)});
        } else if (o == "abs") {
          Hull h = (x.lo >= 0.0)  ? Hull{true, x.lo, x.hi}
                   : (x.hi <= 0.0) ? Hull{true, -x.hi, -x.lo}
                                   : Hull{true, 0.0, std::max(-x.lo, x.hi)};
          fv_row(id, op, cls, x, y, false, abs(fx), h);
        } else if (o == "exp") {
          fv_row(id, op, cls, x, y, false, exp(fx),
                 {true, mpfr_un(mpfr_exp, x.lo, MPFR_RNDD), mpfr_un(mpfr_exp, x.hi, MPFR_RNDU)});
        } else if (o == "sin") {
          fv_row(id, op, cls, x, y, false, sin(fx), {false, 0, 0});
        } else {
          fv_row(id, op, cls, x, y, false, cos(fx), {false, 0, 0});
        }
      }
    }
  }
}

// The examples' constraint expressions over intervals from their domains.
void compound_level() {
  const char* classes[] = {"random", "narrow", "dyadic"};
  const int counts[] = {2000, 2000, 1000};
  const char* exprs[] = {"heart", "folium_x", "folium_y", "folium_eq", "spiral_x", "spiral_y", "golden_r"};
  const double six_pi = 6 * 3.141592653589793;
  for (const char* e : exprs) {
    std::string o = e;
    for (int c = 0; c < 3; c++) {
      std::string cls = classes[c];
      for (int i = 0; i < counts[c]; i++) {
        std::string id = "c/" + o + "/" + cls + "/" + std::to_string(i);
        Interval x{}, y{};
        FloatVal r;
        if (o == "heart") {
          x = random_interval(cls, -20.0, 20.0);
          y = random_interval(cls, -20.0, 20.0);
          FloatVal fx(x.lo, x.hi), fy(y.lo, y.hi);
          r = sqr(fx) + 2 * sqr(fy - 0.5 * nroot(abs(fx), 2));
        } else if ((o == "folium_x") || (o == "folium_y")) {
          x = random_interval(cls, -0.9, 20.0);
          FloatVal p(x.lo, x.hi);
          r = (o == "folium_x") ? 3 * p / (1 + pow(p, 3)) : 3 * sqr(p) / (1 + pow(p, 3));
        } else if (o == "folium_eq") {
          x = random_interval(cls, -1.0, 2.0);
          y = random_interval(cls, -1.0, 2.0);
          FloatVal fx(x.lo, x.hi), fy(y.lo, y.hi);
          r = pow(fx, 3) + pow(fy, 3) - 3 * fx * fy;
        } else if ((o == "spiral_x") || (o == "spiral_y")) {
          x = random_interval(cls, 0.0, six_pi);
          y = random_interval(cls, 0.0, six_pi);
          FloatVal fr(x.lo, x.hi), ft(y.lo, y.hi);
          r = (o == "spiral_x") ? fr * cos(ft) : fr * sin(ft);
        } else {
          x = random_interval(cls, 0.0, 20.0);
          FloatVal ft(x.lo, x.hi);
          r = exp(0.30649 * ft);
        }
        std::string s = id + "\tcompound\t" + o + "\t" + cls + "\tnearest\t";
        hex(s, x.lo);
        s += ",";
        hex(s, x.hi);
        s += "\t";
        hex(s, y.lo);
        s += ",";
        hex(s, y.hi);
        s += "\t";
        hex(s, r.min());
        s += "\t";
        hex(s, r.max());
        s += "\t-\t-\t-";
        std::puts(s.c_str());
      }
    }
  }
}

}  // namespace

int main() {
  std::fesetround(FE_TONEAREST);
  std::puts("id\tlevel\top\tclass\tmode\tx\ty\tdown\tup\toracle_down\toracle_up\tverdict");
  scalar_level();
  floatval_level();
  compound_level();
  return (std::fegetround() == FE_TONEAREST) ? 0 : 3;
}
```

### B.2 Build

`tightness/build.sh`:

```sh
#!/bin/sh
# Build the tightness harness against one install: build.sh ref|p1|p1nb|p2
set -e
cd "$(dirname "$0")"
V=$1; I=$V; EXTRA=
if [ "$V" = p1nb ]; then I=p1; EXTRA=-DGECODE_PROTOTYPE_NO_BARRIER; fi
P=$(cd .. && pwd)/install-$I
/Library/Developer/CommandLineTools/usr/bin/c++ -O3 -DNDEBUG -std=c++2b -arch arm64 -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX27.0.sdk $EXTRA -I $P/include -I /opt/homebrew/include \
  tightness.cpp -o tightness-$V -L $P/lib -lgecodefloat -lgecodekernel -lgecodesupport \
  -L /opt/homebrew/lib -lmpfr -lgmp -Wl,-rpath,$P/lib
```

### B.3 Comparison

`tightness/compare.py`:

```python
"""Compare tightness harness output of a prototype against the reference.

Usage: python3 -I compare.py ref.tsv proto.tsv

Joins rows by id. Widths are counted in ulps through the ordered int64 image
of the bits (+0 and -0 both map to 0). Prints, per level, op and class (round
to nearest only): rows, the fraction where the prototype is wider, the
median, 99th percentile and max of the extra ulps on the lower and upper
side, the median, 99th percentile and max of the relative width growth
(proto width / ref width - 1, over rows with a nonzero ref width), and the
number of narrower rows. Ulps are misleading where a bound lies near zero,
because the ulp grid is much finer there; the relative growth is not. Then the
verdict counts per level and caller mode, and every narrower row.
"""
import struct
import sys
from collections import defaultdict


def ordered(h):
    b = struct.unpack("<q", struct.pack("<d", float.fromhex(h)))[0]
    return -(b & 0x7FFFFFFFFFFFFFFF) if b < 0 else b


def load(path):
    rows = {}
    with open(path) as f:
        head = f.readline().rstrip("\n").split("\t")
        for line in f:
            r = dict(zip(head, line.rstrip("\n").split("\t")))
            rows[r["id"]] = r
    return rows


def pick(xs, q):
    xs = sorted(xs)
    return xs[min(len(xs) - 1, int(q * len(xs)))]


def short(u):
    return f"{u:.2e}" if abs(u) >= 10**6 else str(u)


def stats(xs):
    return f"{short(pick(xs, 0.5))}/{short(pick(xs, 0.99))}/{short(max(xs))}"


def relstats(xs):
    if not xs:
        return "-"
    return f"{pick(xs, 0.5):.2e}/{pick(xs, 0.99):.2e}/{max(xs):.2e}"


def main():
    ref, pro = load(sys.argv[1]), load(sys.argv[2])
    assert ref.keys() == pro.keys(), "case sets differ"
    groups = defaultdict(list)
    verdicts = defaultdict(lambda: defaultdict(int))
    narrower = []
    for k, r in ref.items():
        p = pro[k]
        assert (r["x"], r["y"]) == (p["x"], p["y"]), k
        verdicts[(p["level"], p["mode"])][p["verdict"]] += 1
        lo = ordered(r["down"]) - ordered(p["down"])
        wr = float.fromhex(r["up"]) - float.fromhex(r["down"])
        wp = float.fromhex(p["up"]) - float.fromhex(p["down"])
        rel = (wp / wr - 1.0) if wr > 0 and wr != float("inf") else None
        up = ordered(p["up"]) - ordered(r["up"])
        if lo < 0 or up < 0:
            narrower.append((k, r["down"], r["up"], p["down"], p["up"], r["verdict"], p["verdict"]))
        if p["mode"] != "nearest":
            continue
        cls = p["class"]
        groups[(p["level"], p["op"], cls)].append((lo, up, rel))
        base = cls.split(":")[0]
        if ":" in cls:
            groups[(p["level"], p["op"], base + ":all")].append((lo, up, rel))
        groups[(p["level"], "ALL", cls.split(":")[1] if ":" in cls else "all")].append((lo, up, rel))
    print("level\top\tclass\trows\twider%\tlo_ulps(med/p99/max)\tup_ulps(med/p99/max)"
          "\trel_growth(med/p99/max)\tnarrower")
    for (lvl, op, cls), v in sorted(groups.items()):
        n = len(v)
        wider = sum(1 for lo, up, _ in v if lo > 0 or up > 0)
        nar = sum(1 for lo, up, _ in v if lo < 0 or up < 0)
        rels = [r for _, _, r in v if r is not None]
        print(f"{lvl}\t{op}\t{cls}\t{n}\t{100.0 * wider / n:.1f}\t"
              f"{stats([lo for lo, _, _ in v])}\t{stats([up for _, up, _ in v])}\t"
              f"{relstats(rels)}\t{nar}")
    print()
    print("verdicts (prototype)")
    for (lvl, mode), c in sorted(verdicts.items()):
        print(f"{lvl}\t{mode}\t" + " ".join(f"{k}={c[k]}" for k in sorted(c)))
    print()
    print(f"narrower rows: {len(narrower)}")
    for row in narrower:
        print("\t".join(row))


main()
```

## Appendix C: scripts

### C.1 Prototype generator

The comments in the prototype generator below say that the error terms are
exact and that Fast2Sum is exact once the operands are ordered. That holds
in round to nearest only. Under directed caller modes Fast2Sum gives only
the sign P2 relies on, as Prototypes explains. The code is kept verbatim.

`scripts/make_proto.py`:

```python
"""Apply the P1 or P2 prototype edits to a source copy.

Usage: python3 -I make_proto.py p1|p2 <source-copy>
Edits only gecode/float.hh and gecode/float/rounding.hpp.
"""
import pathlib
import sys

variant, root = sys.argv[1], pathlib.Path(sys.argv[2])

hh = root / "gecode/float.hh"
text = hh.read_text()
old_scope = ("class Rounding : public gecode_boost::numeric::interval_lib::"
             "save_state<RoundingBase> {};")
new_scope = ("class Rounding : public gecode_boost::numeric::interval_lib::"
             "save_state_nothing<RoundingBase> {};")
assert text.count(old_scope) == 1
text = text.replace(old_scope, new_scope)
old_priv = """        static FloatNum barrier(FloatNum x);
"""
new_priv = """        static FloatNum barrier(FloatNum x);
        /// Return the next double below \\a x (prototype)
        static FloatNum next_down(FloatNum x);
        /// Return the next double above \\a x (prototype)
        static FloatNum next_up(FloatNum x);
"""
assert text.count(old_priv) == 1
text = text.replace(old_priv, new_priv)
hh.write_text(text)

hpp = root / "gecode/float/rounding.hpp"
text = hpp.read_text()

old_median = """        if ((u > h) || (l < -h))
            return std::ldexp(RoundingControl::median(l / 2.0, u / 2.0), 1);
        return RoundingControl::median(l, u);"""
new_median = """        // Prototype: no mode switch, computed in the caller's mode.
        if ((u > h) || (l < -h))
            return std::ldexp((l / 2.0 + u / 2.0) / 2.0, 1);
        return (l + u) / 2.0;"""
assert text.count(old_median) == 1
text = text.replace(old_median, new_median)

old_barrier = """        // See the RoundingBase class comment for what this relies on.
"""
new_barrier = """        // See the RoundingBase class comment for what this relies on.
#ifdef GECODE_PROTOTYPE_NO_BARRIER
        return x;
#endif
"""
assert text.count(old_barrier) == 1
text = text.replace(old_barrier, new_barrier)

start = text.index("#if defined(_MSC_VER) && (defined(_M_X64)")
end = text.index("#endif\n", text.index("RoundingBase::sqrt_up(FloatNum x) {\n        return barrier(std::sqrt(barrier(x)));")) + len("#endif\n")

common = """    forceinline FloatNum
    RoundingBase::next_down(FloatNum x) {
        return std::nextafter(x, -std::numeric_limits<FloatNum>::infinity());
    }
    forceinline FloatNum
    RoundingBase::next_up(FloatNum x) {
        return std::nextafter(x, std::numeric_limits<FloatNum>::infinity());
    }
"""

p1 = common + """
    // Prototype P1: round to nearest in the caller's mode, then move the
    // result one ulp outward. No rounding-mode access.
    forceinline FloatNum
    RoundingBase::add_down(FloatNum x, FloatNum y) {
        return next_down(barrier(barrier(x) + barrier(y)));
    }
    forceinline FloatNum
    RoundingBase::add_up(FloatNum x, FloatNum y) {
        return next_up(barrier(barrier(x) + barrier(y)));
    }
    forceinline FloatNum
    RoundingBase::sub_down(FloatNum x, FloatNum y) {
        return next_down(barrier(barrier(x) - barrier(y)));
    }
    forceinline FloatNum
    RoundingBase::sub_up(FloatNum x, FloatNum y) {
        return next_up(barrier(barrier(x) - barrier(y)));
    }
    forceinline FloatNum
    RoundingBase::mul_down(FloatNum x, FloatNum y) {
        return next_down(barrier(barrier(x) * barrier(y)));
    }
    forceinline FloatNum
    RoundingBase::mul_up(FloatNum x, FloatNum y) {
        return next_up(barrier(barrier(x) * barrier(y)));
    }
    forceinline FloatNum
    RoundingBase::div_down(FloatNum x, FloatNum y) {
        return next_down(barrier(barrier(x) / barrier(y)));
    }
    forceinline FloatNum
    RoundingBase::div_up(FloatNum x, FloatNum y) {
        return next_up(barrier(barrier(x) / barrier(y)));
    }
    forceinline FloatNum
    RoundingBase::sqrt_down(FloatNum x) {
        // The lower bound of a square root never goes below zero.
        const FloatNum r = barrier(std::sqrt(barrier(x)));
        return (r > 0.0) ? next_down(r) : r;
    }
    forceinline FloatNum
    RoundingBase::sqrt_up(FloatNum x) {
        return next_up(barrier(std::sqrt(barrier(x))));
    }
"""

p2 = common + """
    // Prototype P2: round to nearest in the caller's mode, compute the
    // rounding error exactly, and move one ulp outward only on the side the
    // error requires. Falls back to P1's widening where the error term may
    // not be exact.
    namespace PrototypeP2 {
      /// Smallest magnitude for which the error terms below are exact
      constexpr FloatNum tiny = 0x1p-969;
      forceinline bool
      small(FloatNum x) {
        return (x != 0.0) && (std::fabs(x) < tiny);
      }
      /// Error of x+y (sign only matters): exact for |x| >= |y| (Fast2Sum)
      forceinline FloatNum
      add_err(FloatNum x, FloatNum y, FloatNum s) {
        if (std::fabs(x) < std::fabs(y))
          std::swap(x, y);
        const FloatNum z = s - x;
        return y - z;
      }
    }

    forceinline FloatNum
    RoundingBase::add_down(FloatNum x, FloatNum y) {
      x = barrier(x); y = barrier(y);
      const FloatNum s = barrier(x + y);
      if (!std::isfinite(s) || PrototypeP2::small(x) || PrototypeP2::small(y)
          || PrototypeP2::small(s))
        return next_down(s);
      const FloatNum e = barrier(PrototypeP2::add_err(x, y, s));
      return (e < 0.0) ? next_down(s) : s;
    }
    forceinline FloatNum
    RoundingBase::add_up(FloatNum x, FloatNum y) {
      x = barrier(x); y = barrier(y);
      const FloatNum s = barrier(x + y);
      if (!std::isfinite(s) || PrototypeP2::small(x) || PrototypeP2::small(y)
          || PrototypeP2::small(s))
        return next_up(s);
      const FloatNum e = barrier(PrototypeP2::add_err(x, y, s));
      return (e > 0.0) ? next_up(s) : s;
    }
    forceinline FloatNum
    RoundingBase::sub_down(FloatNum x, FloatNum y) {
      return add_down(x, -y);
    }
    forceinline FloatNum
    RoundingBase::sub_up(FloatNum x, FloatNum y) {
      return add_up(x, -y);
    }
    forceinline FloatNum
    RoundingBase::mul_down(FloatNum x, FloatNum y) {
      x = barrier(x); y = barrier(y);
      const FloatNum p = barrier(x * y);
      if ((x == 0.0) || (y == 0.0))
        return p;
      if (!std::isfinite(p) || PrototypeP2::small(x) || PrototypeP2::small(y)
          || (std::fabs(p) < PrototypeP2::tiny))
        return next_down(p);
      const FloatNum e = barrier(std::fma(x, y, -p));
      return (e < 0.0) ? next_down(p) : p;
    }
    forceinline FloatNum
    RoundingBase::mul_up(FloatNum x, FloatNum y) {
      x = barrier(x); y = barrier(y);
      const FloatNum p = barrier(x * y);
      if ((x == 0.0) || (y == 0.0))
        return p;
      if (!std::isfinite(p) || PrototypeP2::small(x) || PrototypeP2::small(y)
          || (std::fabs(p) < PrototypeP2::tiny))
        return next_up(p);
      const FloatNum e = barrier(std::fma(x, y, -p));
      return (e > 0.0) ? next_up(p) : p;
    }
    forceinline FloatNum
    RoundingBase::div_down(FloatNum x, FloatNum y) {
      x = barrier(x); y = barrier(y);
      const FloatNum q = barrier(x / y);
      if (x == 0.0)
        return q;
      if (!std::isfinite(q) || !std::isfinite(y) || PrototypeP2::small(x)
          || PrototypeP2::small(y) || (std::fabs(q) < PrototypeP2::tiny))
        return next_down(q);
      FloatNum e = barrier(std::fma(-q, y, x));
      if (y < 0.0)
        e = -e;
      return (e < 0.0) ? next_down(q) : q;
    }
    forceinline FloatNum
    RoundingBase::div_up(FloatNum x, FloatNum y) {
      x = barrier(x); y = barrier(y);
      const FloatNum q = barrier(x / y);
      if (x == 0.0)
        return q;
      if (!std::isfinite(q) || !std::isfinite(y) || PrototypeP2::small(x)
          || PrototypeP2::small(y) || (std::fabs(q) < PrototypeP2::tiny))
        return next_up(q);
      FloatNum e = barrier(std::fma(-q, y, x));
      if (y < 0.0)
        e = -e;
      return (e > 0.0) ? next_up(q) : q;
    }
    forceinline FloatNum
    RoundingBase::sqrt_down(FloatNum x) {
      x = barrier(x);
      const FloatNum r = barrier(std::sqrt(x));
      if (!(r > 0.0))
        return r;
      if (!std::isfinite(r) || PrototypeP2::small(x))
        return next_down(r);
      const FloatNum e = barrier(std::fma(-r, r, x));
      return (e < 0.0) ? next_down(r) : r;
    }
    forceinline FloatNum
    RoundingBase::sqrt_up(FloatNum x) {
      x = barrier(x);
      const FloatNum r = barrier(std::sqrt(x));
      if (!(r > 0.0))
        return r;
      if (!std::isfinite(r) || PrototypeP2::small(x))
        return next_up(r);
      const FloatNum e = barrier(std::fma(-r, r, x));
      return (e > 0.0) ? next_up(r) : r;
    }
"""

text = text[:start] + (p1 if variant == "p1" else p2) + text[end:]
hpp.write_text(text)
print("ok", variant, root)
```

### C.2 Prototype build

`scripts/build.sh`:

```sh
#!/bin/sh
# Configure, build, and install one prototype: build.sh p1|p2
# Run from build/nextafter-proto.
set -e
V=$1
cmake -S src-$V -B build-$V -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/c++ \
  -DGECODE_ENABLE_MPFR=ON -DGECODE_ENABLE_GIST=OFF
cmake --build build-$V -j 10
cmake --install build-$V --prefix "$PWD/install-$V"
```

### C.3 Probe summary

`scripts/probe_summary.py`:

```python
"""Summarize rounding-probe results: probe_summary.py <probe-dir>...

Per configuration: arithmetic rows, exact, loose, unsound, rows where both
sides are loose, mode restored over all rows, and the live-scope verdicts.
"""
import csv
import pathlib
import sys
from collections import Counter

for d in sys.argv[1:]:
    for f in sorted(pathlib.Path(d, "results").glob("*.tsv")):
        rows = list(csv.DictReader(open(f), delimiter="\t"))
        arith = [r for r in rows if r["op"] != "live_scope"]
        live = [r for r in rows if r["op"] == "live_scope"]
        v = Counter(r["verdict"] for r in arith)
        both = sum(1 for r in arith if r["observed_down"] != r["expected_down"]
                   and r["observed_up"] != r["expected_up"])
        restored = sum(1 for r in rows if r["mode_restored"] == "yes")
        lv = Counter(r["verdict"] for r in live)
        print(f"{d}\t{f.stem}\trows={len(arith)}\texact={v['exact']}\tloose={v['loose']}"
              f"\tunsound={v['unsound']}\tboth_sides_loose={both}\trestored={restored}/{len(rows)}"
              f"\tlive={dict(lv)}")
```

### C.4 Float suite, 8 threads

`scripts/suite-t8.sh`:

```sh
#!/bin/sh
# Pass/fail Float:: suite with 8 threads for ref, p1, p2, one after another.
cd "$(dirname "$0")/.."
for b in ref p1 p2; do
  case $b in ref) B=../local;; *) B=build-$b;; esac
  uptime > results/suite-$b-t8.load
  /usr/bin/time -p $B/bin/gecode-test -test Float:: -stop false -threads 8 > results/suite-$b-t8.log 2>&1
  echo "$b exit $?" >> results/suite-t8.status
done
```

### C.5 Failure classification

`scripts/classify.sh`:

```sh
#!/bin/sh
# Replay every failure of a suite log once with -log and keep the failure
# description: classify.sh <build-dir> <suite-log> <out-tsv>
cd "$(dirname "$0")/.."
B=$1; LOG=$2; OUT=$3
: > $OUT
grep '^Options: -replay' $LOG | while read -r _ _ seed _ name; do
  res=$($B/bin/gecode-test -replay $seed -test $name -log 2>&1 | tail -6 | grep -E '^  (Test|Problem|Assignment):' | sed 's/^ *//' | paste -sd '|' -)
  printf '%s\t%s\t%s\n' "$name" "$seed" "$res" >> $OUT
done
echo done >> $OUT.status
```

### C.6 Precise-print examples

`scripts/make_precise.py`:

```python
"""Copy the four float examples and make print() write exact bounds.

Usage: python3 -I make_precise.py <repo-examples-dir> <out-dir>
Each solution prints one line per variable: "V <i> <min> <max> <med>", with
min and max as hexfloat and med as %.17g.
"""
import pathlib
import re
import sys

src, out = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
body = '''    print(std::ostream& os) const override {
        for (int i = 0; i < f.size(); i++) {
            char buf[128];
            std::snprintf(buf, sizeof buf, "V %d %a %a %.17g\\n", i, f[i].min(), f[i].max(), f[i].med());
            os << buf;
        }
    }'''
for name in ["archimedean-spiral", "cartesian-heart", "descartes-folium", "golden-spiral"]:
    t = (src / f"{name}.cpp").read_text()
    t, n = re.subn(r"    print\(std::ostream& os\) const override \{\n.*?\n    \}", lambda m: body, t,
                   count=1, flags=re.S)
    assert n == 1, name
    t = t.replace("#include <gecode/driver.hh>", "#include <cstdio>\n#include <gecode/driver.hh>", 1)
    (out / f"{name}.cpp").write_text(t)
```

### C.7 Precise-print build

`examples-precise/build.sh`:

```sh
#!/bin/sh
# Build the precise-print examples against each install, with build/local's flags.
set -e
cd "$(dirname "$0")"
SDK=/Library/Developer/CommandLineTools/SDKs/MacOSX27.0.sdk
for v in ref p1 p2; do
  P=$(cd .. && pwd)/install-$v
  mkdir -p bin-$v
  for e in archimedean-spiral cartesian-heart descartes-folium golden-spiral; do
    /Library/Developer/CommandLineTools/usr/bin/c++ -O3 -DNDEBUG -std=c++2b -arch arm64 -isysroot $SDK \
      -I $P/include -isystem /opt/homebrew/include $e.cpp -o bin-$v/$e -L $P/lib \
      -lgecodedriver -lgecodeminimodel -lgecodeset -lgecodefloat -lgecodesearch -lgecodeinstance \
      -lgecodeint -lgecodekernel -lgecodesupport -L /opt/homebrew/lib -lmpfr -lgmp -Wl,-rpath,$P/lib
  done
done
```

### C.8 Precise-print change, cartesian-heart

`diff -u` of the repository's `examples/cartesian-heart.cpp` and
`examples-precise/cartesian-heart.cpp`:

```diff
--- ../../examples/cartesian-heart.cpp
+++ examples-precise/cartesian-heart.cpp
@@ -30,6 +30,7 @@
  *
  */
 
+#include <cstdio>
 #include <gecode/driver.hh>
 
 #include <gecode/minimodel.hh>
@@ -95,7 +96,11 @@
     /// Print solution coordinates
     void
     print(std::ostream& os) const override {
-        os << "XY " << f[0].med() << " " << f[1].med() << '\n';
+        for (int i = 0; i < f.size(); i++) {
+            char buf[128];
+            std::snprintf(buf, sizeof buf, "V %d %a %a %.17g\n", i, f[i].min(), f[i].max(), f[i].med());
+            os << buf;
+        }
     }
 };
```

### C.9 Examples

`scripts/examples.sh`:

```sh
#!/bin/sh
# Example outputs, statistics, and runtimes for ref, p1, p2.
# Writes results/examples/. Run with nothing else heavy on the machine.
cd "$(dirname "$0")/.."
R=results/examples
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
bin() { case $1 in ref) echo ../local/bin;; *) echo build-$1/bin;; esac; }
for b in ref p1 p2; do
  for e in $EX; do
    $(bin $b)/$e -mode solution > $R/$e-$b.solution
    examples-precise/bin-$b/$e -mode solution > $R/$e-$b.precise
    $(bin $b)/$e -mode stat > $R/$e-$b.stat1
    $(bin $b)/$e -mode stat > $R/$e-$b.stat2
  done
done
uptime > $R/timing.load
: > $R/timing.tsv
for round in 1 2 3 4 5 6 7 8 9 10 11; do
  for b in ref p1 p2; do
    for e in $EX; do
      ms=$($(bin $b)/$e -mode stat | sed -n 's/.*runtime:.*(\(.*\) ms).*/\1/p')
      printf '%s\t%s\t%s\t%s\n' $round $b $e $ms >> $R/timing.tsv
    done
  done
done
uptime >> $R/timing.load
```

### C.10 Example comparison

`scripts/compare_examples.py`:

```python
"""Compare precise example bounds: compare_examples.py <ref.precise> <proto.precise>

Solutions are aligned by index. For each variable prints the number of
solutions, how many have a wider or narrower prototype interval, the median,
99th percentile and max of the extra width in ulps (ordered int64 image of
the bits, +0 and -0 both 0), and the median and max relative width growth.
"""
import struct
import sys


def ordered(x):
    b = struct.unpack("<q", struct.pack("<d", x))[0]
    return -(b & 0x7FFFFFFFFFFFFFFF) if b < 0 else b


def load(path):
    sols = []
    for line in open(path):
        if not line.startswith("V "):
            continue
        _, i, lo, hi, _ = line.split()
        if int(i) == 0:
            sols.append([])
        sols[-1].append((float.fromhex(lo), float.fromhex(hi)))
    return sols


def pick(xs, q):
    xs = sorted(xs)
    return xs[min(len(xs) - 1, int(q * len(xs)))]


ref, pro = load(sys.argv[1]), load(sys.argv[2])
print(f"solutions: ref {len(ref)}, proto {len(pro)}")
n = min(len(ref), len(pro))
for v in range(len(ref[0])):
    extra, rel, wider, narrower = [], [], 0, 0
    for s in range(n):
        (rl, rh), (pl, ph) = ref[s][v], pro[s][v]
        wr, wp = ordered(rh) - ordered(rl), ordered(ph) - ordered(pl)
        extra.append(wp - wr)
        wider += (pl < rl) or (ph > rh)
        narrower += (pl > rl) or (ph < rh)
        if rh - rl > 0:
            rel.append((ph - pl) / (rh - rl) - 1.0)
    print(f"var {v}: wider {wider}/{n}, narrower {narrower}/{n}, extra ulps med/p99/max "
          f"{pick(extra, 0.5)}/{pick(extra, 0.99)}/{max(extra)}, rel growth med/max "
          f"{pick(rel, 0.5):.3g}/{max(rel):.3g}")
```

### C.11 Microbenchmark build

`bench/build.sh`:

```sh
#!/bin/sh
# Build the boost-006 microbenchmark against ref, p1, p1 without barrier, and p2.
set -e
cd "$(dirname "$0")"
SDK=/Library/Developer/CommandLineTools/SDKs/MacOSX27.0.sdk
for v in ref p1 p1nb p2; do
  I=$v; EXTRA=
  if [ $v = p1nb ]; then I=p1; EXTRA=-DGECODE_PROTOTYPE_NO_BARRIER; fi
  P=$(cd .. && pwd)/install-$I
  FLAGS="-O3 -DNDEBUG -std=c++2b -arch arm64 -isysroot $SDK $EXTRA -I $P/include -isystem /opt/homebrew/include"
  /Library/Developer/CommandLineTools/usr/bin/c++ $FLAGS bench.cpp -o bench-$v -L $P/lib \
    -lgecodefloat -lgecodekernel -lgecodesupport -L /opt/homebrew/lib -lmpfr -lgmp -Wl,-rpath,$P/lib
  /Library/Developer/CommandLineTools/usr/bin/c++ $FLAGS -S bench.cpp -o bench-$v.s
done
```

### C.12 Microbenchmark runs

`scripts/bench.sh`:

```sh
#!/bin/sh
# Microbenchmark: 7 interleaved rounds of each variant.
cd "$(dirname "$0")/.."
uptime > results/bench.load
: > results/bench.txt
for round in 1 2 3 4 5 6 7; do
  for v in ref p1 p1nb p2; do
    printf '%s\t%s\t%s\n' $round $v "$(bench/bench-$v 10000000)" >> results/bench.txt
  done
done
uptime >> results/bench.load
```

### C.13 Timed runs

`scripts/timed.sh`:

```sh
#!/bin/sh
# All timed runs, one after another: examples, microbenchmark, then one
# single-threaded Float:: suite run per build.
cd "$(dirname "$0")/.."
scripts/examples.sh; echo "examples done" >> results/timed.status
scripts/bench.sh; echo "bench done" >> results/timed.status
for b in ref p1 p2; do
  case $b in ref) B=../local;; *) B=build-$b;; esac
  uptime > results/suite-$b-t1.load
  /usr/bin/time -p $B/bin/gecode-test -test Float:: -stop false -threads 1 > results/suite-$b-t1.log 2>&1
  uptime >> results/suite-$b-t1.load
  echo "suite $b exit $?" >> results/timed.status
done
```
