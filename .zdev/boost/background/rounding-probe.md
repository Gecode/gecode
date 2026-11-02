# Rounding probe: does directed rounding survive optimization?

## Result

No. Today's build computes unsound directed bounds with all three compilers.
At -O2 and -O3, every operation through `Float::Rounding` is unsound with
compiler-visible constants. Multiplication and square root are unsound with
runtime operands too, and with GCC so is division. With AppleClang and LLVM
22 clang, multiplication is unsound even at -O0, so a Debug build is affected
too. Only GCC 14 at -O0 gave exact bounds in every row. Every unsound row has one bound on the wrong side of the exact result,
so the interval excludes the true value. No row was loose (sound but wider).
Every Float::Rounding and FloatVal scope restored the caller's rounding mode,
under all four caller modes, and a live Float::Rounding scope kept upward
mode between operations as intended.

| configuration | unsound rows (of 160) | operations unsound with runtime operands |
|---|---|---|
| AppleClang -O0, LLVM 22 -O0 | 32 | mul |
| AppleClang -O2/-O3, LLVM 22 -O2/-O3 | 88 | mul; sqrt via Float::Rounding |
| GCC 14 -O0 | 0 | none |
| GCC 14 -O2 | 120 | mul, sqrt; div via Float::Rounding |
| GCC 14 -O3 | 128 | mul, div, sqrt |

The rows did not depend on the caller's mode. Four mechanisms cause the
failures (see Mechanisms):

1. At -O2 and -O3, constants are folded in round-to-nearest. Through
   `Float::Rounding` this hits every operation. Clang still computed
   `FloatVal` division and square root of constants correctly.
2. Clang at every level, and GCC at -O2 and -O3, compile `x * (-y)` in
   `mul_down` to one `fnmul`. That instruction rounds `x * y` and then negates
   it, so `mul_down` returns the upward-rounded product.
3. At -O2 and -O3, `sqrt_down` and `sqrt_up` share one `fsqrt`, which runs in
   downward mode, so the upper bound is rounded down.
4. GCC at -O2 and -O3 shares one `fdiv` between `div_down` and `div_up`, since
   `-(x / -y)` equals `x / y` under round-to-nearest. Both bounds are then
   rounded the same way.

## Question and scope

This answers boost-001 and the brief's first open question: does the build
honor directed rounding under optimization, given that it sets no
floating-point environment flags (no `-frounding-math`, `-ffp-model=strict`,
or `#pragma STDC FENV_ACCESS`)?

Covered: `+ - * /` and `sqrt` in both directions, through
`Gecode::Float::Rounding` and through `FloatVal` operators, with
compiler-visible literals and runtime operands, under the four caller modes,
for AppleClang, LLVM 22 clang and GCC 14 at -O0, -O2 and -O3 on arm64 macOS.

Not covered: x86-64 (erebor could run the same probe with GCC 15 and Clang
21), MSVC and its `_control87` path (CI only), `float` and `long double`, the
MPFR transcendental functions, `median`, `int_down` and `int_up`.

## Setup

- Gecode HEAD d552491470b138051924659b8360f9b2cfbd9c64 on branch feature/breaking-7.
- arm64 (Apple silicon), macOS 27.0.1 (26A434), SDK 27.0 at
  `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk`.
- Gecode build under test: `build/local`, Release (`-O3 -DNDEBUG`), AppleClang
  21, `GECODE_ENABLE_MPFR=ON`, `CMAKE_CXX_FLAGS` empty. Rebuilt at HEAD with
  `cmake --build build/local` and installed with
  `cmake --install build/local --prefix build/rounding-probe/install`. The
  installed `gecode/support/config.hpp` defines `GECODE_HAS_MPFR`.
- Compilers:
  - AppleClang: `/usr/bin/clang++`, Apple clang version 21.0.0
    (clang-2100.3.34.2).
  - LLVM 22: `/opt/homebrew/opt/llvm@22/bin/clang++`, Homebrew clang version
    22.1.8.
  - GCC 14: `/opt/homebrew/bin/g++-14`, g++-14 (Homebrew GCC 14.2.0_1) 14.2.0.
- MPFR and GMP from `/opt/homebrew/opt/mpfr` and `/opt/homebrew/opt/gmp`.

The rounding code is inline in the headers. `Float::Rounding` is
`save_state<RoundingBase>`, and `RoundingBase` derives from
`rounded_arith_opp<double>` with the C99 `fesetround` control on arm64. So
each compiler compiles the code under test itself. The probe links the
installed libraries only because GCC emits `Gecode::Choice` vtables from the
headers.

## Method

The probe is `build/rounding-probe/rounding-probe.cpp` (Appendix A), built and
run by `build/rounding-probe/run.sh` (Appendix B).

- Entry points. `rounding` opens one `Float::Rounding` scope and computes
  `<op>_down` and then `<op>_up`. `float_val` evaluates
  `FloatVal(x) <op> FloatVal(y)`, or `sqrt(FloatVal(x))`, and takes `min()` and
  `max()`; each `FloatVal` operator opens its own scope.
- Operand kinds. `constant` calls a function in which the case's literals are
  compile-time constants (`observe_constant<index>`). `opaque` passes values
  that `main` parsed from argv with `strtod` and checked bit for bit against
  the literals.
- Cases. No exact result is representable, and every result is a normal
  double; the probe checks both.

  | op | x | y | x (%a) | y (%a) |
  |---|---|---|---|---|
  | add | 0.1 | 0.2 | 0x1.999999999999ap-4 | 0x1.999999999999ap-3 |
  | add | -0.1 | -0.2 | -0x1.999999999999ap-4 | -0x1.999999999999ap-3 |
  | sub | 0.1 | 0.7 | 0x1.999999999999ap-4 | 0x1.6666666666666p-1 |
  | sub | -0.1 | 0.2 | -0x1.999999999999ap-4 | 0x1.999999999999ap-3 |
  | mul | 0.1 | 0.3 | 0x1.999999999999ap-4 | 0x1.3333333333333p-2 |
  | mul | -0.1 | 0.3 | -0x1.999999999999ap-4 | 0x1.3333333333333p-2 |
  | div | 1.0 | 3.0 | 0x1p+0 | 0x1.8p+1 |
  | div | -1.0 | 3.0 | -0x1p+0 | 0x1.8p+1 |
  | sqrt | 2.0 | - | 0x1p+1 | - |
  | sqrt | 0.1 | - | 0x1.999999999999ap-4 | - |

  The planned case `sub 0.1 - 0.3` has an exactly representable result, so
  the probe rejected it; `0.1 - 0.7` replaces it.
- Caller modes. Each row runs with the caller in `FE_TONEAREST`,
  `FE_DOWNWARD`, `FE_UPWARD` and `FE_TOWARDZERO`.
- Oracle. MPFR at 53 bits: `mpfr_set_d` (exact), then the operation with
  `MPFR_RNDD` and `MPFR_RNDU`. This does not depend on the hardware mode.
- Cross-check. Bounds from exact error terms, computed in `FE_TONEAREST`: the
  nearest result `n` and the sign of `exact - n`, from TwoSum for add and sub,
  `fma(x, y, -p)` for mul, `fma(-q, y, x) * sign(y)` for div and
  `fma(-s, s, x)` for sqrt. The lower bound is `nextafter(n, -inf)` when the
  error is negative and `n` otherwise; the upper bound is symmetric. The probe
  exits with status 2 if this disagrees with MPFR. It agreed for every case in
  all nine builds, so this MPFR-free oracle can serve the regression test.
- Verdicts. `exact` equals the oracle in both directions. `unsound` has
  `down > expected_down` or `up < expected_up`. `loose` is sound but not
  tight.
- Restoration. `mode_restored` requires `fegetround()` before and after the
  scope to equal the caller's mode. A separate `live_scope` row checks that
  `fegetround() == FE_UPWARD` inside a live `Float::Rounding` scope after
  `add_down` and `sqrt_down` (for `float_val`, after a `FloatVal` sum and
  square root). This is `rounded_arith_opp`'s intended behaviour.

Each build writes 160 arithmetic rows (2 entries x 10 cases x 2 kinds x 4
caller modes) and 8 live-scope rows to
`build/rounding-probe/results/<compiler><level>.tsv`. Its exit status is in
`<compiler><level>.exit`, and its assembly in `<compiler><level>.s`. These
files are in `build/`, which git ignores. Exit statuses: 1 for every build
except GCC 14 -O0, which exited 0. No build exited 2.

## Commands

`run.sh` runs these nine builds. `$SDK` is the SDK path above and `$GCC` is
`/opt/homebrew/Cellar/gcc/14.2.0_1`; the commands run from
`build/rounding-probe`.

```sh
COMMON="-std=c++23 -I install/include -I /opt/homebrew/opt/mpfr/include -I /opt/homebrew/opt/gmp/include"
LINK="-L install/lib -lgecodefloat -lgecodekernel -lgecodesupport -L /opt/homebrew/opt/mpfr/lib -lmpfr \
  -L /opt/homebrew/opt/gmp/lib -lgmp -Wl,-rpath,$PWD/install/lib"
for L in -O0 -O2 -O3; do
  /usr/bin/clang++ $COMMON $L rounding-probe.cpp -o bin/probe-appleclang$L $LINK
  /opt/homebrew/opt/llvm@22/bin/clang++ $COMMON $L rounding-probe.cpp -o bin/probe-llvm22$L $LINK
  /opt/homebrew/bin/g++-14 $COMMON $L -isysroot $SDK -nostdinc -nostdinc++ \
    -isystem $GCC/include/c++/14 -isystem $GCC/include/c++/14/aarch64-apple-darwin23 \
    -isystem $GCC/lib/gcc/current/gcc/aarch64-apple-darwin23/14/include \
    -isystem $SDK/usr/include -mmacosx-version-min=27.0 rounding-probe.cpp -o bin/probe-gcc14$L $LINK
done
bin/probe-<compiler><level> <compiler> <level> \
  0.1 0.2 -0.1 -0.2 0.1 0.7 -0.1 0.2 0.1 0.3 -0.1 0.3 1.0 3.0 -1.0 3.0 2.0 0.0 0.1 0.0
```

No other flags are passed; `-DNDEBUG` is left out so that only the level
varies. GCC alone needs the header and deployment-target flags. Homebrew GCC
14.2.0_1 is built for darwin23, and its include-fixed headers fail against
SDK 27. These flags select headers only and do not change floating-point
semantics. Without `-mmacosx-version-min=27.0` the driver passes an invalid
version 18.0 to the linker.

## Expected and observed bounds

| op | x | y | expected down | expected up |
|---|---|---|---|---|
| add | 0.1 | 0.2 | 0x1.3333333333333p-2 | 0x1.3333333333334p-2 |
| add | -0.1 | -0.2 | -0x1.3333333333334p-2 | -0x1.3333333333333p-2 |
| sub | 0.1 | 0.7 | -0x1.3333333333333p-1 | -0x1.3333333333332p-1 |
| sub | -0.1 | 0.2 | -0x1.3333333333334p-2 | -0x1.3333333333333p-2 |
| mul | 0.1 | 0.3 | 0x1.eb851eb851eb8p-6 | 0x1.eb851eb851eb9p-6 |
| mul | -0.1 | 0.3 | -0x1.eb851eb851eb9p-6 | -0x1.eb851eb851eb8p-6 |
| div | 1.0 | 3.0 | 0x1.5555555555555p-2 | 0x1.5555555555556p-2 |
| div | -1.0 | 3.0 | -0x1.5555555555556p-2 | -0x1.5555555555555p-2 |
| sqrt | 2.0 | 0.0 | 0x1.6a09e667f3bccp+0 | 0x1.6a09e667f3bcdp+0 |
| sqrt | 0.1 | 0.0 | 0x1.43d136248490fp-2 | 0x1.43d136248491p-2 |

The tables below show the rows with the caller in `FE_TONEAREST`. The other
three caller modes gave identical bounds in every cell (checked over all
1440 rows), so no caller-mode axis is shown. Each cell is
`observed down, observed up`; non-exact cells are bold.

### AppleClang 21

| entry | case | kind | -O0 | -O2 | -O3 |
|---|---|---|---|---|---|
| rounding | add(0.1, 0.2) | constant | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) |
| rounding | add(0.1, 0.2) | opaque | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 |
| rounding | add(-0.1, -0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| rounding | add(-0.1, -0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| rounding | sub(0.1, 0.7) | constant | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) |
| rounding | sub(0.1, 0.7) | opaque | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 |
| rounding | sub(-0.1, 0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| rounding | sub(-0.1, 0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| rounding | mul(0.1, 0.3) | constant | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb8p-6** (unsound) | **0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb8p-6** (unsound) |
| rounding | mul(0.1, 0.3) | opaque | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) |
| rounding | mul(-0.1, 0.3) | constant | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| rounding | mul(-0.1, 0.3) | opaque | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| rounding | div(1.0, 3.0) | constant | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) |
| rounding | div(1.0, 3.0) | opaque | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 |
| rounding | div(-1.0, 3.0) | constant | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | **-0x1.5555555555555p-2, -0x1.5555555555555p-2** (unsound) | **-0x1.5555555555555p-2, -0x1.5555555555555p-2** (unsound) |
| rounding | div(-1.0, 3.0) | opaque | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 |
| rounding | sqrt(2.0) | constant | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | **0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0** (unsound) | **0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0** (unsound) |
| rounding | sqrt(2.0) | opaque | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | **0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bccp+0** (unsound) | **0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bccp+0** (unsound) |
| rounding | sqrt(0.1) | constant | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) |
| rounding | sqrt(0.1) | opaque | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) |
| float_val | add(0.1, 0.2) | constant | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) |
| float_val | add(0.1, 0.2) | opaque | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 |
| float_val | add(-0.1, -0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| float_val | add(-0.1, -0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| float_val | sub(0.1, 0.7) | constant | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) |
| float_val | sub(0.1, 0.7) | opaque | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 |
| float_val | sub(-0.1, 0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| float_val | sub(-0.1, 0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| float_val | mul(0.1, 0.3) | constant | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) |
| float_val | mul(0.1, 0.3) | opaque | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) |
| float_val | mul(-0.1, 0.3) | constant | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| float_val | mul(-0.1, 0.3) | opaque | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| float_val | div(1.0, 3.0) | constant | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 |
| float_val | div(1.0, 3.0) | opaque | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 |
| float_val | div(-1.0, 3.0) | constant | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 |
| float_val | div(-1.0, 3.0) | opaque | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 |
| float_val | sqrt(2.0) | constant | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 |
| float_val | sqrt(2.0) | opaque | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 |
| float_val | sqrt(0.1) | constant | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 |
| float_val | sqrt(0.1) | opaque | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 |

### LLVM 22 clang

| entry | case | kind | -O0 | -O2 | -O3 |
|---|---|---|---|---|---|
| rounding | add(0.1, 0.2) | constant | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) |
| rounding | add(0.1, 0.2) | opaque | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 |
| rounding | add(-0.1, -0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| rounding | add(-0.1, -0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| rounding | sub(0.1, 0.7) | constant | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) |
| rounding | sub(0.1, 0.7) | opaque | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 |
| rounding | sub(-0.1, 0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| rounding | sub(-0.1, 0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| rounding | mul(0.1, 0.3) | constant | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb8p-6** (unsound) | **0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb8p-6** (unsound) |
| rounding | mul(0.1, 0.3) | opaque | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) |
| rounding | mul(-0.1, 0.3) | constant | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| rounding | mul(-0.1, 0.3) | opaque | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| rounding | div(1.0, 3.0) | constant | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) |
| rounding | div(1.0, 3.0) | opaque | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 |
| rounding | div(-1.0, 3.0) | constant | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | **-0x1.5555555555555p-2, -0x1.5555555555555p-2** (unsound) | **-0x1.5555555555555p-2, -0x1.5555555555555p-2** (unsound) |
| rounding | div(-1.0, 3.0) | opaque | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 |
| rounding | sqrt(2.0) | constant | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | **0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0** (unsound) | **0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0** (unsound) |
| rounding | sqrt(2.0) | opaque | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | **0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bccp+0** (unsound) | **0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bccp+0** (unsound) |
| rounding | sqrt(0.1) | constant | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) |
| rounding | sqrt(0.1) | opaque | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) |
| float_val | add(0.1, 0.2) | constant | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) |
| float_val | add(0.1, 0.2) | opaque | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 |
| float_val | add(-0.1, -0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| float_val | add(-0.1, -0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| float_val | sub(0.1, 0.7) | constant | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) |
| float_val | sub(0.1, 0.7) | opaque | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 |
| float_val | sub(-0.1, 0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| float_val | sub(-0.1, 0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| float_val | mul(0.1, 0.3) | constant | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) |
| float_val | mul(0.1, 0.3) | opaque | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) |
| float_val | mul(-0.1, 0.3) | constant | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| float_val | mul(-0.1, 0.3) | opaque | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| float_val | div(1.0, 3.0) | constant | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 |
| float_val | div(1.0, 3.0) | opaque | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 |
| float_val | div(-1.0, 3.0) | constant | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 |
| float_val | div(-1.0, 3.0) | opaque | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 |
| float_val | sqrt(2.0) | constant | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 |
| float_val | sqrt(2.0) | opaque | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 |
| float_val | sqrt(0.1) | constant | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 |
| float_val | sqrt(0.1) | opaque | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 |

### GCC 14

| entry | case | kind | -O0 | -O2 | -O3 |
|---|---|---|---|---|---|
| rounding | add(0.1, 0.2) | constant | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) |
| rounding | add(0.1, 0.2) | opaque | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 |
| rounding | add(-0.1, -0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| rounding | add(-0.1, -0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| rounding | sub(0.1, 0.7) | constant | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) |
| rounding | sub(0.1, 0.7) | opaque | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 |
| rounding | sub(-0.1, 0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| rounding | sub(-0.1, 0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| rounding | mul(0.1, 0.3) | constant | 0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb9p-6 | **0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb8p-6** (unsound) | **0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb8p-6** (unsound) |
| rounding | mul(0.1, 0.3) | opaque | 0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb9p-6 | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) |
| rounding | mul(-0.1, 0.3) | constant | -0x1.eb851eb851eb9p-6, -0x1.eb851eb851eb8p-6 | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| rounding | mul(-0.1, 0.3) | opaque | -0x1.eb851eb851eb9p-6, -0x1.eb851eb851eb8p-6 | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| rounding | div(1.0, 3.0) | constant | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) |
| rounding | div(1.0, 3.0) | opaque | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) |
| rounding | div(-1.0, 3.0) | constant | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | **-0x1.5555555555555p-2, -0x1.5555555555555p-2** (unsound) | **-0x1.5555555555555p-2, -0x1.5555555555555p-2** (unsound) |
| rounding | div(-1.0, 3.0) | opaque | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | **-0x1.5555555555556p-2, -0x1.5555555555556p-2** (unsound) | **-0x1.5555555555556p-2, -0x1.5555555555556p-2** (unsound) |
| rounding | sqrt(2.0) | constant | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | **0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0** (unsound) | **0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0** (unsound) |
| rounding | sqrt(2.0) | opaque | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | **0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bccp+0** (unsound) | **0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bccp+0** (unsound) |
| rounding | sqrt(0.1) | constant | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) |
| rounding | sqrt(0.1) | opaque | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) |
| float_val | add(0.1, 0.2) | constant | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) | **0x1.3333333333334p-2, 0x1.3333333333334p-2** (unsound) |
| float_val | add(0.1, 0.2) | opaque | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 | 0x1.3333333333333p-2, 0x1.3333333333334p-2 |
| float_val | add(-0.1, -0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| float_val | add(-0.1, -0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| float_val | sub(0.1, 0.7) | constant | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) | **-0x1.3333333333333p-1, -0x1.3333333333333p-1** (unsound) |
| float_val | sub(0.1, 0.7) | opaque | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 | -0x1.3333333333333p-1, -0x1.3333333333332p-1 |
| float_val | sub(-0.1, 0.2) | constant | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) | **-0x1.3333333333334p-2, -0x1.3333333333334p-2** (unsound) |
| float_val | sub(-0.1, 0.2) | opaque | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 | -0x1.3333333333334p-2, -0x1.3333333333333p-2 |
| float_val | mul(0.1, 0.3) | constant | 0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb9p-6 | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb8p-6** (unsound) |
| float_val | mul(0.1, 0.3) | opaque | 0x1.eb851eb851eb8p-6, 0x1.eb851eb851eb9p-6 | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) | **0x1.eb851eb851eb9p-6, 0x1.eb851eb851eb9p-6** (unsound) |
| float_val | mul(-0.1, 0.3) | constant | -0x1.eb851eb851eb9p-6, -0x1.eb851eb851eb8p-6 | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| float_val | mul(-0.1, 0.3) | opaque | -0x1.eb851eb851eb9p-6, -0x1.eb851eb851eb8p-6 | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) | **-0x1.eb851eb851eb8p-6, -0x1.eb851eb851eb8p-6** (unsound) |
| float_val | div(1.0, 3.0) | constant | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) |
| float_val | div(1.0, 3.0) | opaque | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | 0x1.5555555555555p-2, 0x1.5555555555556p-2 | **0x1.5555555555555p-2, 0x1.5555555555555p-2** (unsound) |
| float_val | div(-1.0, 3.0) | constant | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | **-0x1.5555555555555p-2, -0x1.5555555555555p-2** (unsound) | **-0x1.5555555555555p-2, -0x1.5555555555555p-2** (unsound) |
| float_val | div(-1.0, 3.0) | opaque | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | -0x1.5555555555556p-2, -0x1.5555555555555p-2 | **-0x1.5555555555556p-2, -0x1.5555555555556p-2** (unsound) |
| float_val | sqrt(2.0) | constant | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | **0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0** (unsound) | **0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0** (unsound) |
| float_val | sqrt(2.0) | opaque | 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0 | **0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bccp+0** (unsound) | **0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bccp+0** (unsound) |
| float_val | sqrt(0.1) | constant | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) |
| float_val | sqrt(0.1) | opaque | 0x1.43d136248490fp-2, 0x1.43d136248491p-2 | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) | **0x1.43d136248490fp-2, 0x1.43d136248490fp-2** (unsound) |

### Restoration and live scope

| configuration | rows | exact | loose | unsound | mode restored | upward kept in live scope |
|---|---|---|---|---|---|---|
| appleclang -O0 | 160 | 128 | 0 | 32 | 168/168 | 8/8 |
| appleclang -O2 | 160 | 72 | 0 | 88 | 168/168 | 8/8 |
| appleclang -O3 | 160 | 72 | 0 | 88 | 168/168 | 8/8 |
| llvm22 -O0 | 160 | 128 | 0 | 32 | 168/168 | 8/8 |
| llvm22 -O2 | 160 | 72 | 0 | 88 | 168/168 | 8/8 |
| llvm22 -O3 | 160 | 72 | 0 | 88 | 168/168 | 8/8 |
| gcc14 -O0 | 160 | 160 | 0 | 0 | 168/168 | 8/8 |
| gcc14 -O2 | 160 | 40 | 0 | 120 | 168/168 | 8/8 |
| gcc14 -O3 | 160 | 32 | 0 | 128 | 168/168 | 8/8 |

Every row restored the caller's mode, and every live scope stayed upward.

## Mechanisms

The excerpts come from `build/rounding-probe/mechanisms.cpp` (Appendix C),
compiled with `-S` by `mechanisms.sh` into
`build/rounding-probe/results/mechanisms/`. On arm64, `fesetround` with
`0x400000` selects upward and `0x800000` downward mode.

**Constant folding (all compilers, -O2 and -O3).** `add_down(0.1, 0.2)` loads
the precomputed nearest value `-0.30000000000000004` (`0xbfd3333333333334`)
and negates it. No `fadd` runs in upward mode (AppleClang -O2; LLVM 22 and GCC
-O2 are the same):

```asm
bl    _fesetround             ; upward
mov   x8, #3689348814741910323 ; 0x3333333333333333
movk  x8, #13108
movk  x8, #49107, lsl #48     ; 0xbfd3333333333334
str   x8, [sp, #8]            ; volatile store of a constant
ldr   d8, [sp, #8]
bl    _fesetround             ; restore
fneg  d0, d8
```

The other constant rows fail the same way. Nothing is folded at -O0.

**Fused negate-multiply (Clang at all levels, GCC at -O2 and -O3).**
`mul_down` is `-force_rounding(x * (-y))` in upward mode. Clang selects
`fnmul`, which computes `-(round(x * y))` rather than `round(-(x * y))`.
Under upward rounding it yields `-up(x * y)`, and the outer negation returns
the upward product as the lower bound. AppleClang -O0, out-of-line
`rounded_arith_opp<double>::mul_down`:

```asm
ldr   d1, [x8]                ; x
ldr   d0, [x8]                ; y
fnmul d0, d0, d1              ; -(y * x), product rounded upward
bl    ...force_rounding...
fneg  d0, d0
```

GCC -O0 keeps `fneg` and `fmul` separate, which is why it is exact there. At
-O2, Clang (`fnmul d0, d8, d9`) and GCC (`fnmul d0, d15, d14`) both emit one
`fnmul` inside the upward window.
This failure needs no constant and no code motion, and occurs in Debug builds
made with Clang.

**One square root for both bounds (Clang and GCC at -O2 and -O3).**
`sqrt_down` switches to downward, computes, and switches back to upward;
`sqrt_up` then computes `sqrt(x)` again. The compilers treat `sqrt` as
independent of the rounding mode and reuse the first result, so the upper
bound is the downward root. AppleClang -O2, `Float::Rounding` `sqrt_down` and
`sqrt_up`:

```asm
bl    _fesetround             ; 0x400000, upward (scope)
mov   w0, #8388608
bl    _fesetround             ; 0x800000, downward (sqrt_down)
fsqrt d9, d8                  ; the only fsqrt
str   d9, [sp, #8]            ; volatile store: lower bound
ldr   d8, [sp, #8]
mov   w0, #4194304
bl    _fesetround             ; upward
str   d9, [sp, #8]            ; volatile store of the same d9: upper bound
ldr   d9, [sp, #8]
```

GCC -O2 does the same on its non-negative path: one `fsqrt` runs in downward
mode, and its result is reused for the upper bound. (For negative input it
calls `_sqrt` twice.) Clang got
`sqrt(FloatVal)` right, apparently because the interval code reaches the two
calls along separate paths. GCC got it wrong as well.

**One division for both bounds (GCC at -O2 and -O3).** `div_down` is
`-force_rounding(x / (-y))`, and `div_up` is `force_rounding(x / y)`. GCC
rewrites both as negations of `(-x) / y` and emits one `fdiv` in upward mode.
So both bounds are the downward quotient. GCC -O2:

```asm
bl    _fesetround             ; upward
fneg  d15, d15                ; -x
fdiv  d15, d15, d14           ; (-x) / y, the only fdiv
str   d15, [x29, 56]          ; volatile store (div_down)
fneg  d15, d15                ; x / y for div_up, rounded downward
...
fneg  d14, d14                ; lower bound
```

Clang emits two `fdiv` here and is exact with runtime operands.

**The volatile store does not help.** `force_rounding` stores the result to a
volatile and reads it back. That pins a store, not the computation of the
stored value: the compiler may still fold the value, select `fnmul`, or reuse
an earlier result.

### Supplementary: floating-point environment flags

This is outside the boost-001 matrix, but it is relevant to the fix.
`build/rounding-probe/extra-flags.sh` reran the same probe with added flags
(results in `results/extra/`):

| compiler and flag | -O0 | -O2 | -O3 |
|---|---|---|---|
| AppleClang `-frounding-math` | exact | exact | exact |
| AppleClang `-ffp-model=strict` | exact | exact | exact |
| LLVM 22 `-ffp-model=strict` | exact | exact | exact |
| GCC 14 `-frounding-math` | exact | 32 unsound (sqrt only) | 32 unsound (sqrt only) |

With `-frounding-math`, GCC 14 still reuses one downward `sqrt` for both
bounds, for both entry points and both operand kinds. GCC also does not
implement `#pragma STDC FENV_ACCESS`. So a flag alone does not make GCC sound;
the square root needs a barrier, or a separate computation that GCC cannot
merge. Every flagged build restored the caller's mode.

## Consequences

- Float propagation can lose solutions in shipped builds. `libgecodefloat` is
  built at -O3 by default, so its propagators' bound computations are subject
  to mechanisms 2 to 4, and to folding wherever a bound involves a constant.
- Applications compile these inline headers with their own flags. Changing
  Gecode's CMake flags would cover Gecode's own translation units, but not
  application code that uses `FloatVal` arithmetic or `Float::Rounding`
  directly. A fix has to hold for the headers under the consumer's default
  flags, or the build has to state what consumers must pass.
- With Clang, Debug builds are also unsound for multiplication, so testing in
  Debug does not protect against this.
- boost-002 measures switched rounding modes against `nextafter`-based
  outward rounding. Its switched-mode baseline is the code measured here,
  which is unsound under optimization until a fix lands.

## Test::Base decision

No `Test::Base` test was added. A directed-rounding test fits in
`bin/gecode-test`, but it would fail. `build/local` (Release, AppleClang -O3)
gives the unsound rows above. `build/full` (Debug, AppleClang, `-g`, that is
-O0) gives the unsound multiplication rows. The task allows a test only if
every configuration is sound, and it forbids committing a failing test. The
regression test therefore moves to the proposed fix task.

## Proposed follow-up

A fix task, "Keep directed rounding sound under optimization", carries the
permanent regression test `Float::Arithmetic::DirectedRounding`. It goes next
to `Float::Arithmetic::RoundingScope` in `test/float/arithmetic.cpp`, with
`TestTags::check()`. The test uses the error-term oracle from this probe as
private static member functions (no MPFR). It runs these cases with literal
operands and with `const volatile` copies, through `Float::Rounding` and
`FloatVal`, under all four caller modes, and checks `fegetround`
restoration. It fails before the fix in both `build/local` and `build/full`
with AppleClang.

## Differences from the planning findings

- The plan expected `+ - * /` with runtime operands to be exact. Its
  planning probe tested only addition and square root with runtime operands.
  With runtime operands, multiplication is unsound with all three compilers,
  and with Clang even at -O0. Division is unsound with GCC at -O2 and -O3.
- The plan suspected GCC 14 -O0 to be unsound for literal `add_down`, from a
  build it described as faulty. The clean rebuild gives exact bounds in all
  160 GCC -O0 rows, so that result does not reproduce.
- The plan expected a `Test::Base` test to pass in Debug and fail in Release.
  `build/full` uses AppleClang, so it would fail in Debug as well. The
  decision not to add the test stands.
- The plan described AppleClang -O2's shared `fsqrt` as following both
  `fesetround` calls. The excerpt above shows it between the switch to
  downward and the switch back to upward, so it runs in downward mode and the
  upper bound is wrong. The planner's observed values (both bounds equal to
  the lower one) agree with this.
- The planned case `sub 0.1 - 0.3` is exactly representable and was replaced
  by `0.1 - 0.7`.

## Appendix A: rounding-probe.cpp

```cpp
// Probe for boost-001: does directed rounding in Float::Rounding and FloatVal
// survive optimization? Each bound is compared with MPFR at 53 bits (RNDD and
// RNDU), which is cross-checked against bounds derived from exact error terms.
//
// Usage: rounding-probe <compiler> <level> <x0> <y0> <x1> <y1> ...
// The operands must be the decimal texts of the case table, in order. They are
// parsed at run time so that the compiler cannot see them.
//
// Exit status: 0 if every row is exact and restores the caller's mode, 1 if
// any row does not, 2 for a probe or oracle error.

#include <gecode/float.hh>

#include <mpfr.h>

#include <array>
#include <bit>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>

using Gecode::FloatVal;
namespace Float = Gecode::Float;

namespace {

enum class Op { add, sub, mul, div, sqrt };
enum class Kind { constant, opaque };
enum class Entry { rounding, float_val };
enum class Verdict { exact, loose, unsound };

struct Bounds {
  double down;
  double up;
};

struct Observation {
  Bounds bounds;
  int mode_before;
  int mode_after;
};

struct Case {
  Op op;
  double x;
  double y;
  const char* x_text;
  const char* y_text;
};

// No exact result of these cases is representable; main checks this.
constexpr Case cases[] = {
  {Op::add, 0.1, 0.2, "0.1", "0.2"},     {Op::add, -0.1, -0.2, "-0.1", "-0.2"},
  {Op::sub, 0.1, 0.7, "0.1", "0.7"},     {Op::sub, -0.1, 0.2, "-0.1", "0.2"},
  {Op::mul, 0.1, 0.3, "0.1", "0.3"},     {Op::mul, -0.1, 0.3, "-0.1", "0.3"},
  {Op::div, 1.0, 3.0, "1.0", "3.0"},     {Op::div, -1.0, 3.0, "-1.0", "3.0"},
  {Op::sqrt, 2.0, 0.0, "2.0", "0.0"},    {Op::sqrt, 0.1, 0.0, "0.1", "0.0"},
};
constexpr std::size_t case_count = std::size(cases);

constexpr int caller_modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};

// Float::Rounding computes both bounds in one scope, down first.
template <Op op>
[[gnu::always_inline]] inline Bounds rounding_bounds(double x, double y) {
  Float::Rounding r;
  if constexpr (op == Op::add) {
    return {r.add_down(x, y), r.add_up(x, y)};
  } else if constexpr (op == Op::sub) {
    return {r.sub_down(x, y), r.sub_up(x, y)};
  } else if constexpr (op == Op::mul) {
    return {r.mul_down(x, y), r.mul_up(x, y)};
  } else if constexpr (op == Op::div) {
    return {r.div_down(x, y), r.div_up(x, y)};
  } else {
    return {r.sqrt_down(x), r.sqrt_up(x)};
  }
}

// FloatVal opens its own rounding scope inside each operator.
template <Op op>
[[gnu::always_inline]] inline Bounds float_val_bounds(double x, double y) {
  FloatVal v;
  if constexpr (op == Op::add) {
    v = FloatVal(x) + FloatVal(y);
  } else if constexpr (op == Op::sub) {
    v = FloatVal(x) - FloatVal(y);
  } else if constexpr (op == Op::mul) {
    v = FloatVal(x) * FloatVal(y);
  } else if constexpr (op == Op::div) {
    v = FloatVal(x) / FloatVal(y);
  } else {
    v = sqrt(FloatVal(x));
  }
  return {v.min(), v.max()};
}

template <Op op>
[[gnu::always_inline]] inline Observation observe(Entry entry, double x, double y) {
  Observation o;
  o.mode_before = std::fegetround();
  o.bounds = entry == Entry::rounding ? rounding_bounds<op>(x, y) : float_val_bounds<op>(x, y);
  o.mode_after = std::fegetround();
  return o;
}

// The case's literals are compile-time constants inside this function.
template <std::size_t index>
[[gnu::noinline]] Observation observe_constant(Entry entry) {
  constexpr Case c = cases[index];
  return observe<c.op>(entry, c.x, c.y);
}

using ConstantObserver = Observation (*)(Entry);

template <std::size_t... index>
constexpr std::array<ConstantObserver, sizeof...(index)> make_constant_observers(std::index_sequence<index...>) {
  return {&observe_constant<index>...};
}

constexpr auto constant_observers = make_constant_observers(std::make_index_sequence<case_count>());

// The operands come from argv and are unknown to the compiler.
[[gnu::noinline]] Observation observe_opaque(Entry entry, Op op, double x, double y) {
  switch (op) {
  case Op::add:
    return observe<Op::add>(entry, x, y);
  case Op::sub:
    return observe<Op::sub>(entry, x, y);
  case Op::mul:
    return observe<Op::mul>(entry, x, y);
  case Op::div:
    return observe<Op::div>(entry, x, y);
  case Op::sqrt:
    return observe<Op::sqrt>(entry, x, y);
  }
  std::abort();
}

// Inside a live Float::Rounding scope, rounded_arith_opp keeps upward mode
// between calls on purpose, also after sqrt_down switched to downward.
[[gnu::noinline]] bool upward_inside_live_scope(Entry entry, double x, double y) {
  Float::Rounding r;
  volatile double sink;
  if (entry == Entry::rounding) {
    sink = r.add_down(x, y);
    sink = r.sqrt_down(x);
  } else {
    sink = (FloatVal(x) + FloatVal(y)).min();
    sink = sqrt(FloatVal(x)).min();
  }
  (void)sink;
  return std::fegetround() == FE_UPWARD;
}

int apply_mpfr(Op op, mpfr_t result, const mpfr_t x, const mpfr_t y, mpfr_rnd_t direction) {
  switch (op) {
  case Op::add:
    return mpfr_add(result, x, y, direction);
  case Op::sub:
    return mpfr_sub(result, x, y, direction);
  case Op::mul:
    return mpfr_mul(result, x, y, direction);
  case Op::div:
    return mpfr_div(result, x, y, direction);
  case Op::sqrt:
    return mpfr_sqrt(result, x, direction);
  }
  std::abort();
}

// Correctly rounded bounds at 53 bits; independent of the hardware mode.
// Sets representable when either direction is exact.
Bounds mpfr_bounds(const Case& c, bool& representable) {
  mpfr_t x, y, down, up;
  mpfr_inits2(53, x, y, down, up, static_cast<mpfr_ptr>(nullptr));
  mpfr_set_d(x, c.x, MPFR_RNDN);
  mpfr_set_d(y, c.y, MPFR_RNDN);
  int down_ternary = apply_mpfr(c.op, down, x, y, MPFR_RNDD);
  int up_ternary = apply_mpfr(c.op, up, x, y, MPFR_RNDU);
  representable = down_ternary == 0 || up_ternary == 0;
  Bounds b{mpfr_get_d(down, MPFR_RNDN), mpfr_get_d(up, MPFR_RNDN)};
  mpfr_clears(x, y, down, up, static_cast<mpfr_ptr>(nullptr));
  return b;
}

struct NearestResult {
  double value;
  // Positive when the exact result lies above value, negative when below.
  double error_sign;
};

NearestResult two_sum(double a, double b) {
  double s = a + b;
  double b_virtual = s - a;
  double error = (a - (s - b_virtual)) + (b - b_virtual);
  return {s, error};
}

// Must run in FE_TONEAREST: the error terms are exact only for nearest results.
NearestResult nearest_with_error(const Case& c) {
  switch (c.op) {
  case Op::add:
    return two_sum(c.x, c.y);
  case Op::sub:
    return two_sum(c.x, -c.y);
  case Op::mul: {
    double p = c.x * c.y;
    return {p, std::fma(c.x, c.y, -p)};
  }
  case Op::div: {
    double q = c.x / c.y;
    double remainder = std::fma(-q, c.y, c.x);
    return {q, remainder * (c.y < 0 ? -1.0 : 1.0)};
  }
  case Op::sqrt: {
    double s = std::sqrt(c.x);
    return {s, std::fma(-s, s, c.x)};
  }
  }
  std::abort();
}

Bounds error_term_bounds(const Case& c) {
  constexpr double infinity = std::numeric_limits<double>::infinity();
  NearestResult n = nearest_with_error(c);
  double down = n.error_sign < 0 ? std::nextafter(n.value, -infinity) : n.value;
  double up = n.error_sign > 0 ? std::nextafter(n.value, infinity) : n.value;
  return {down, up};
}

Verdict classify(const Bounds& observed, const Bounds& expected) {
  if (!(observed.down <= expected.down) || !(observed.up >= expected.up))
    return Verdict::unsound;
  if (observed.down < expected.down || observed.up > expected.up)
    return Verdict::loose;
  return Verdict::exact;
}

const char* op_name(Op op) {
  switch (op) {
  case Op::add:
    return "add";
  case Op::sub:
    return "sub";
  case Op::mul:
    return "mul";
  case Op::div:
    return "div";
  case Op::sqrt:
    return "sqrt";
  }
  std::abort();
}

const char* mode_name(int mode) {
  switch (mode) {
  case FE_TONEAREST:
    return "nearest";
  case FE_DOWNWARD:
    return "downward";
  case FE_UPWARD:
    return "upward";
  case FE_TOWARDZERO:
    return "towardzero";
  default:
    return "unknown";
  }
}

const char* entry_name(Entry entry) { return entry == Entry::rounding ? "rounding" : "float_val"; }

const char* kind_name(Kind kind) { return kind == Kind::constant ? "constant" : "opaque"; }

const char* verdict_name(Verdict verdict) {
  switch (verdict) {
  case Verdict::exact:
    return "exact";
  case Verdict::loose:
    return "loose";
  case Verdict::unsound:
    return "unsound";
  }
  std::abort();
}

bool same_bits(double a, double b) { return std::bit_cast<std::uint64_t>(a) == std::bit_cast<std::uint64_t>(b); }

// Fills operands from argv and returns false unless they match the case table.
bool parse_operands(int argc, char* argv[], std::array<Case, case_count>& operands) {
  if (argc != 3 + 2 * static_cast<int>(case_count))
    return false;
  for (std::size_t i = 0; i < case_count; i++) {
    operands[i] = cases[i];
    operands[i].x = std::strtod(argv[3 + 2 * i], nullptr);
    operands[i].y = std::strtod(argv[4 + 2 * i], nullptr);
    if (!same_bits(operands[i].x, cases[i].x) || !same_bits(operands[i].y, cases[i].y))
      return false;
  }
  return true;
}

// Returns false if a case is unsuitable or the two oracles disagree.
bool compute_oracle(std::array<Bounds, case_count>& expected) {
  for (std::size_t i = 0; i < case_count; i++) {
    bool representable;
    Bounds by_mpfr = mpfr_bounds(cases[i], representable);
    Bounds by_error_term = error_term_bounds(cases[i]);
    if (representable || !std::isnormal(by_mpfr.down) || !std::isnormal(by_mpfr.up)) {
      std::fprintf(stderr, "case %zu is representable or not normal\n", i);
      return false;
    }
    if (!same_bits(by_mpfr.down, by_error_term.down) || !same_bits(by_mpfr.up, by_error_term.up)) {
      std::fprintf(stderr, "case %zu: MPFR [%a, %a] and error terms [%a, %a] disagree\n", i, by_mpfr.down,
                   by_mpfr.up, by_error_term.down, by_error_term.up);
      return false;
    }
    expected[i] = by_mpfr;
  }
  return true;
}

void print_header() {
  std::printf("compiler\tlevel\tentry\top\tx\ty\tkind\tcaller_mode\texpected_down\tobserved_down\texpected_up\t"
              "observed_up\tverdict\tmode_restored\n");
}

// Returns true if the row is exact and the caller's mode was restored.
bool run_row(const char* compiler, const char* level, Entry entry, std::size_t i, Kind kind, int caller_mode,
             const Case& operand, const Bounds& expected) {
  std::fesetround(caller_mode);
  Observation o = kind == Kind::constant ? constant_observers[i](entry)
                                         : observe_opaque(entry, operand.op, operand.x, operand.y);
  std::fesetround(FE_TONEAREST);
  Verdict verdict = classify(o.bounds, expected);
  bool restored = o.mode_before == caller_mode && o.mode_after == caller_mode;
  std::printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%a\t%a\t%a\t%a\t%s\t%s\n", compiler, level, entry_name(entry),
              op_name(operand.op), operand.x_text, operand.y_text, kind_name(kind), mode_name(caller_mode),
              expected.down, o.bounds.down, expected.up, o.bounds.up, verdict_name(verdict),
              restored ? "yes" : "no");
  return verdict == Verdict::exact && restored;
}

// Informational: rows with op "live_scope" record whether upward mode is kept.
void run_live_scope_check(const char* compiler, const char* level, Entry entry, int caller_mode,
                          const Case& operand) {
  std::fesetround(caller_mode);
  bool upward = upward_inside_live_scope(entry, operand.x, operand.y);
  bool restored = std::fegetround() == caller_mode;
  std::fesetround(FE_TONEAREST);
  std::printf("%s\t%s\t%s\tlive_scope\t%s\t%s\topaque\t%s\t-\t-\t-\t-\t%s\t%s\n", compiler, level, entry_name(entry),
              operand.x_text, operand.y_text, mode_name(caller_mode), upward ? "upward_kept" : "upward_lost",
              restored ? "yes" : "no");
}

} // namespace

int main(int argc, char* argv[]) {
  std::array<Case, case_count> operands;
  if (!parse_operands(argc, argv, operands)) {
    std::fprintf(stderr, "usage: %s <compiler> <level> followed by the case table's operand texts\n", argv[0]);
    return 2;
  }
  std::array<Bounds, case_count> expected;
  if (std::fegetround() != FE_TONEAREST || !compute_oracle(expected))
    return 2;
  const char* compiler = argv[1];
  const char* level = argv[2];
  print_header();
  bool all_exact = true;
  for (int caller_mode : caller_modes)
    for (Entry entry : {Entry::rounding, Entry::float_val})
      for (std::size_t i = 0; i < case_count; i++)
        for (Kind kind : {Kind::constant, Kind::opaque})
          all_exact &= run_row(compiler, level, entry, i, kind, caller_mode, operands[i], expected[i]);
  for (int caller_mode : caller_modes)
    for (Entry entry : {Entry::rounding, Entry::float_val})
      run_live_scope_check(compiler, level, entry, caller_mode, operands[0]);
  return all_exact ? 0 : 1;
}
```

## Appendix B: run.sh

```sh
#!/bin/sh
# Build and run the boost-001 rounding probe for each compiler and level.
# Expects the Gecode install under ./install (cmake --install build/local).
# Writes results/<compiler>-<level>.{tsv,exit,s} and results/commands.txt.
set -eu
cd "$(dirname "$0")"

SDK=$(xcrun --show-sdk-path)
GCC=/opt/homebrew/Cellar/gcc/14.2.0_1
MPFR=/opt/homebrew/opt/mpfr
GMP=/opt/homebrew/opt/gmp
OPERANDS="0.1 0.2 -0.1 -0.2 0.1 0.7 -0.1 0.2 0.1 0.3 -0.1 0.3 1.0 3.0 -1.0 3.0 2.0 0.0 0.1 0.0"

# Homebrew GCC 14.2.0_1 targets darwin23; its fixed headers break against the
# current SDK. These flags select headers and the deployment target only.
GCC_FLAGS="-isysroot $SDK -nostdinc -nostdinc++ -isystem $GCC/include/c++/14 \
-isystem $GCC/include/c++/14/aarch64-apple-darwin23 \
-isystem $GCC/lib/gcc/current/gcc/aarch64-apple-darwin23/14/include \
-isystem $SDK/usr/include -mmacosx-version-min=27.0"

COMMON_FLAGS="-std=c++23 -I install/include -I $MPFR/include -I $GMP/include"
LINK_FLAGS="-L install/lib -lgecodefloat -lgecodekernel -lgecodesupport -L $MPFR/lib -lmpfr -L $GMP/lib -lgmp \
-Wl,-rpath,$(pwd)/install/lib"

rm -rf results bin
mkdir -p results bin
: > results/commands.txt

for compiler in appleclang llvm22 gcc14; do
  case $compiler in
    appleclang) CXX=/usr/bin/clang++; EXTRA= ;;
    llvm22) CXX=/opt/homebrew/opt/llvm@22/bin/clang++; EXTRA= ;;
    gcc14) CXX=/opt/homebrew/bin/g++-14; EXTRA=$GCC_FLAGS ;;
  esac
  echo "$compiler: $($CXX --version | head -n 1)" >> results/commands.txt
  for level in -O0 -O2 -O3; do
    name=$compiler$level
    compile="$CXX $COMMON_FLAGS $level $EXTRA rounding-probe.cpp -o bin/probe-$name $LINK_FLAGS"
    echo "  $compile" >> results/commands.txt
    $compile
    $CXX $COMMON_FLAGS $level $EXTRA -S rounding-probe.cpp -o results/$name.s
    set +e
    bin/probe-$name $compiler $level $OPERANDS > results/$name.tsv
    echo $? > results/$name.exit
    set -e
    echo "$name: exit $(cat results/$name.exit), $(grep -c unsound results/$name.tsv || true) unsound rows"
  done
done
echo "run: bin/probe-<compiler><level> <compiler> <level> $OPERANDS" >> results/commands.txt
```

## Appendix C: mechanisms.cpp

```cpp
// Small functions whose assembly shows how each failing row loses its
// rounding direction. Compiled with -S only; see mechanisms.sh.

#include <gecode/float.hh>

namespace Float = Gecode::Float;

struct Bounds {
  double down;
  double up;
};

extern "C" {

double add_down_constant() {
  Float::Rounding r;
  return r.add_down(0.1, 0.2);
}

double mul_down_opaque(double x, double y) {
  Float::Rounding r;
  return r.mul_down(x, y);
}

Bounds div_opaque(double x, double y) {
  Float::Rounding r;
  return {r.div_down(x, y), r.div_up(x, y)};
}

Bounds sqrt_opaque(double x) {
  Float::Rounding r;
  return {r.sqrt_down(x), r.sqrt_up(x)};
}

}
```

`mechanisms.sh` compiles this file with each compiler and level, using the
same include and GCC flags as `run.sh` plus `-fno-asynchronous-unwind-tables
-S`, and extracts each function into `results/mechanisms/`.
`summarize.py` turns the TSV files into the tables above.
