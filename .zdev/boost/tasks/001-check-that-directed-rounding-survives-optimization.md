+++
schema_version = 1
id = "boost-001"
key = "rounding-probe"
area = "boost"
status = "open"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Check that directed rounding survives optimization

## Outcome

We know whether the current build computes correct directed bounds at -O0, -O2, and -O3 with AppleClang, Homebrew LLVM 22 clang, and GCC 14, given that it sets no floating-point environment flags. Each result is checked against an independent oracle, and the probe can be rerun.

## Context

The brief's Current state section explains the rounding control: C99 fesetround with a volatile store on x86-64 non-MSVC and AArch64, _control87 on MSVC, and no -frounding-math, -ffp-model=strict, or FENV_ACCESS pragma. The risk is that the compiler constant-folds or moves floating-point operations across rounding-mode changes. Gecode::Float::Rounding (gecode/float.hh, gecode/float/rounding.hpp) and FloatVal arithmetic are the entry points. For +, -, *, / and sqrt in both directions, compare each bound with an independent oracle: MPFR at high precision (installed at /opt/homebrew/opt/mpfr), or exact error terms from fma/TwoSum. A downward result must be the largest double not above the exact value, and an upward result the smallest double not below it. Use operands that are not exactly representable, both as compiler-visible constants and as runtime values the compiler cannot see. Also check that the rounding mode is restored when each Float::Rounding scope is destroyed; a live rounded_arith_opp scope keeps upward mode between calls on purpose. The existing Float::Arithmetic::RoundingScope test in test/float/arithmetic.cpp shows the scope pattern. GCC 14 is at /opt/homebrew/bin/g++-14. LLVM 22 is at /opt/homebrew/opt/llvm@22/bin.

## Boundaries

- Do not change float, rounding, or build-flag code in this task.
- A demonstrated soundness failure completes this investigation. Propose a separate fix task, with the permanent regression test, through the derived-work contract. Do not commit a failing test.

## Done when

- [ ] .zdev/boost/background/rounding-probe.md records the probe source (or its path), the exact compilers and flags, and the expected and observed bounds for each operation, direction, operand kind, and configuration, and whether the rounding mode was restored. The brief indexes it.
- [ ] If every configuration is sound and the probe can run inside bin/gecode-test as a Test::Base test without depending on an optimization level, the test is added under test/float/ and passes. Otherwise the result says why it was not added.

## Validation

- Run the probe with each compiler at -O0, -O2, and -O3.
- When a test is added: `bin/gecode-test -test <its name>` in the Release and Debug builds.
