+++
schema_version = 1
id = "boost-006"
key = "directed-rounding-sound-under-optimization"
area = "boost"
status = "open"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Keep directed rounding sound under optimization

## Outcome

Float::Rounding and FloatVal arithmetic give exact directed bounds for + - * / and sqrt with AppleClang, LLVM 22 clang and GCC 14 at -O0, -O2 and -O3, both in Gecode's own translation units and in consumer translation units built with default flags, and the build states which flags or pragmas this relies on.

## Context

boost-001 showed that today's directed rounding is unsound on arm64 macOS. The full results, probe source (Appendix A), matrix script (Appendix B) and assembly excerpts are in .zdev/boost/background/rounding-probe.md. The probe compares Float::Rounding <op>_down/<op>_up and FloatVal + - * / sqrt with MPFR at 53 bits (RNDD/RNDU), cross-checked against an error-term oracle, for compiler-visible literals and runtime operands under all four caller modes. Unsound rows (of 160 per build): AppleClang 21 and LLVM 22 clang 32 at -O0 and 88 at -O2/-O3; GCC 14 0 at -O0, 120 at -O2 and 128 at -O3. No row was loose, and every scope restored the caller's mode. Four mechanisms: (1) at -O2/-O3 constants are folded in round-to-nearest, which breaks every Float::Rounding operation on literals (e.g. add_down(0.1, 0.2) returns 0x1.3333333333334p-2, expected 0x1.3333333333333p-2); (2) mul_down is -force_rounding(x * (-y)), and Clang at every level (GCC at -O2/-O3) emits one fnmul, which rounds x*y upward and then negates, so mul_down(0.1, 0.3) returns the upper bound 0x1.eb851eb851eb9p-6 even with runtime operands and in Debug builds; (3) at -O2/-O3 sqrt_down and sqrt_up share one fsqrt computed in downward mode, so sqrt_up(2.0) returns 0x1.6a09e667f3bccp+0 (Clang through Float::Rounding, GCC through both entry points); (4) GCC at -O2/-O3 rewrites div_down's -(x / -y) and div_up's x / y to one (-x) / y, so both bounds equal the downward quotient. The volatile store in force_rounding pins only the store, not the computation. Supplementary runs in the note: -frounding-math and -ffp-model=strict make AppleClang and LLVM 22 exact at all levels, but GCC 14 with -frounding-math still shares the downward sqrt (32 unsound rows at -O2/-O3), and GCC does not implement #pragma STDC FENV_ACCESS. The code is inline in gecode/float.hh, gecode/float/rounding.hpp, gecode/float/val.hpp and gecode/third-party/boost/numeric/interval/rounded_arith.hpp, so it is compiled both in libgecodefloat (-O3) and in consumer translation units with the consumer's flags; a change to Gecode's CMake flags alone does not cover consumers. The planner chooses the mechanism (pragma, flags, register barriers around operands and results, or separate computations the compiler cannot merge); the brief requires the build to state what it relies on. Regression test design: add class DirectedRounding : public Base next to RoundingScope in test/float/arithmetic.cpp, named Float::Arithmetic::DirectedRounding with TestTags::check(). It holds the probe's error-term oracle as private static member functions (no MPFR): TwoSum for add and sub, fma(x, y, -p) for mul, fma(-q, y, x) * sign(y) for div and fma(-s, s, x) for sqrt, computed in FE_TONEAREST, with the lower bound nextafter(n, -inf) when the error is negative and the upper bound symmetric. It uses the probe's ten cases (add 0.1+0.2 and -0.1+-0.2, sub 0.1-0.7 and -0.1-0.2, mul 0.1*0.3 and -0.1*0.3, div 1/3 and -1/3, sqrt 2 and sqrt 0.1) with literal operands and const volatile copies, through Float::Rounding and FloatVal, under FE_TONEAREST, FE_DOWNWARD, FE_UPWARD and FE_TOWARDZERO, and checks fegetround restoration after each scope. Before the fix it fails in build/local (Release, AppleClang -O3) and in build/full (Debug, AppleClang -g). boost-002 measures switched rounding modes against nextafter; its switched-mode baseline is this unsound code until the fix is committed.

## Boundaries

- Do not redesign FloatVal's public operations or the float constraint API; only the rounding implementation and build flags behind them may change.
- Stop for the user if soundness requires compiler flags imposed on consumer code.
- Do not change the version or ABI numbers.

## Done when

- [ ] Float::Arithmetic::DirectedRounding in test/float/arithmetic.cpp, using the error-term oracle, fails before the fix in the Release build and passes after it in the Release and Debug builds.
- [ ] The probe from .zdev/boost/background/rounding-probe.md, rerun against a fresh install, is exact and restores the caller's mode in all nine compiler and level configurations.
- [ ] The rounding implementation documents which compiler flags, pragmas or barriers it relies on.
- [ ] changelog.in describes the fix.

## Validation

- build/local/bin/gecode-test -test Float::Arithmetic::DirectedRounding and build/full/bin/gecode-test -test Float::Arithmetic::DirectedRounding, before and after the fix
- bin/gecode-test -test Float:: in the Release and Debug builds
- Rerun the probe matrix from the note's Appendix B (AppleClang, LLVM 22 and GCC 14 at -O0, -O2 and -O3)
- uv run misc/tidy.py --check
