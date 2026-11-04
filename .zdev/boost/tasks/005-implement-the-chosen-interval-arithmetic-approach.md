+++
schema_version = 1
id = "boost-005"
key = "implement"
area = "boost"
status = "done"
complexity = "advanced"
afk = true
blocked_by = ["boost-004", "boost-008", "boost-009", "boost-010", "boost-011"]
+++
# Implement the chosen interval-arithmetic approach

## Outcome

Float variables use the approach the user chose, and the brief's success criteria hold. This is the area's terminal task: later areas depend on it.

## Context

Read the decision in the brief and the background studies. This task is expected to split into children through the derived-work contract, before editing, into coherent steps for the chosen approach (for example: a reproducible extraction script and refreshed copy, or the support-library core, the rounding layer, the FloatVal migration, and removal of gecode/third-party/boost). It stays open until its children are done, then completes with the full validation below.

## Boundaries

- No external dependency (see the brief's Terms and settled decisions).

## Done when

- [x] Installed headers either contain a vendored copy that a committed script reproduces exactly, with a check like the generated-source freshness check, or no Boost code at all.
- [x] The build states and applies the compiler flags or pragmas that directed rounding relies on, and the rounding probe or its committed test passes with every compiler from the probe.
- [x] The decision is documented in a Doxygen comment beside the interval implementation, or in docs/ if it spans several files, and changelog.in describes the change under 7.0.0.
- [x] The full float suite, including sweep-tagged tests, passes in Debug and Release with and without MPFR.
- [x] The four float examples give the same solutions as before. Changes in search statistics are explained in the result.

## Validation

- Debug and Release builds with float variables, with and without MPFR, ctest, and `bin/gecode-test -test Float::` including sweeps.
- Last task in the area: run `uv run misc/check.py all --build-dir build/full --jobs N`. Fix failures that this area caused. Report failures that already exist at the area base.

## Result

The native interval library (boost-008 to boost-011) meets every boost-005 done condition with no further code change. Installed headers contain no Boost code; directed rounding relies on boost-006's barriers with no compiler flags, as the RoundingBase Doxygen states; the decision comment is in gecode/float/interval.hpp and changelog.in records the change. MSVC rounding code runs only in CI; build/zdev-impl/boost-005/msvc-ci.md lists what a CI run must cover.

Validation:

- Full Float:: suite including sweeps passed in Release and Debug, with and without MPFR (1566/1566 with MPFR, 1428/1428 without), and Debug without MPFR on erebor x86-64 GCC 15 (1428/1428).
- Rounding probe with AppleClang, LLVM 22 and GCC 14 at -O0/-O2/-O3 exact, modes restored, identical to the boost-009 baseline.
- Four float examples give the same solutions and search statistics as before the switch.
- ctest --test-dir build/local 7/7; uv run misc/check.py all --build-dir build/full --jobs 10 passed; uv run misc/tidy.py --check passed.
- Fresh verifier (gpt-6.1-sol high) PASS with no findings.
