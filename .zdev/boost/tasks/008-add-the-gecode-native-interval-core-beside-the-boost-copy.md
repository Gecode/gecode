+++
schema_version = 1
id = "boost-008"
key = "interval-core"
area = "boost"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Add the Gecode-native interval core beside the Boost copy

## Outcome

gecode/float/interval.hpp defines Float::Interval and its free functions in Gecode's own style, unused by FloatVal yet, with focused Test::Base tests and a temporary differential test that matches the vendored Boost interval bit for bit.

## Context

The brief records the decision (Terms and settled decisions: Approach, License). The spike in background/native-spike.md built a prototype that matches HEAD on every oracle; its patch is build/zdev-impl/boost-007/native-code.patch and its measurement harnesses and diffops grid are under build/zdev-impl/boost-007/. The spike's interval.hpp translated Boost line for line. This task instead writes the core from the mathematics of each operation in Gecode style (snake_case helpers, Gecode naming, Doxygen contracts), so that the code is Gecode code under the MIT license; Boost.Interval and the spike are references for behaviour, not text to translate. Keep every result the same as Boost's, including the checking_strict empty-interval semantics, the possibly/certainly comparison outcomes that gecode/float/val.hpp derives from comparison_error today, sqrt, fmod, pow, nth_root, the pi constants, and the optimization barriers from boost-006; the spike's 'Deliberate differences from Boost' section lists what may differ. The rounding layer stays Boost's until interval-switch, so the core takes its rounding policy the way FloatVal uses it today or through a small template parameter the switch can fill. Add the Doxygen decision comment beside Float::Interval: the chosen approach, the measured alternatives, the reasons, and an acknowledgement of Boost.Interval as the reference whose results the code reproduces, with no Boost copyright or license text. The differential test runs the spike's diffops grid inside bin/gecode-test against gecode/third-party/boost and compares bounds, truth values, and exceptions bit for bit; it is temporary and removed with the copy.

## Boundaries

- FloatVal, float.hh's rounding layer, and the Boost copy stay unchanged.
- No Boost text is copied; a part that can only be written as a close translation keeps Boost's notice, and the result names it and says why.
- No external dependency.
- Initial task-owned path allocation: ["gecode/float/interval.hpp","test/float/interval.cpp","test/float/interval-oracle.cpp","cmake/GecodeSources.cmake"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [x] Float::Interval in gecode/float/interval.hpp provides every operation that FloatVal and the float propagators use, written in Gecode style, with the decision comment and no Boost copyright or license text.
- [x] Test::Base tests cover outward rounding of + - * / and sqrt on values that are not exactly representable, singleton and empty intervals, throwing empty construction, and the comparison truth table.
- [x] The temporary differential test compares Float::Interval with the vendored Boost interval over the diffops grid and finds no difference in bounds, truth values, or exceptions, with and without MPFR.

## Validation

- Release and Debug builds with and without MPFR; bin/gecode-test -test Float:: in Release and the check and standard tags in Debug.
- uv run misc/check.py lint --build-dir build/full and format-check on the changed files.

## Result

Added gecode/float/interval.hpp, a Gecode/MIT Float::Interval written in Gecode style from the mathematics of each operation (sign-class case tables, named helpers, one Float::Rounding per public operation), covering every operation FloatVal uses, with the decision comment acknowledging Boost.Interval as the reference and no Boost license text. float.hh includes it; FloatVal, the rounding layer and the Boost copy are unchanged. Three Test::Base tests (OutwardRounding, SingletonAndEmpty, Comparison) and the temporary Float::Interval::MatchesBoost oracle, which finds no bit difference from the vendored Boost interval over the diffops grid in all four caller modes with and without MPFR.

Validation:

- Release/Debug x MPFR on/off builds without warnings; Float:: Release 1567/1567 and 1429/1429, Debug check+standard 533/533 and 512/512; lint, format-check and doc clean. Fresh verifier (codex gpt-6.1-sol high) reran the interval and arithmetic tests in all builds, ASan/UBSan on the new tests, an oracle mutation (8584 mismatches) and the license comparison; its INT_MIN pow overflow and NaN-comment findings were fixed and passed a focused recheck with UBSan.
