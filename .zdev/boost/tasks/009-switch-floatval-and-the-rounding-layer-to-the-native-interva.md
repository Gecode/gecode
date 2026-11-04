+++
schema_version = 1
id = "boost-009"
key = "interval-switch"
area = "boost"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = ["boost-008"]
+++
# Switch FloatVal and the rounding layer to the native interval

## Outcome

FloatVal stores Float::Interval, and Float::RoundingBase and Float::Rounding are Gecode classes with switched-mode rounding control for every supported target, including MSVC _control87 written from Microsoft's documentation, so no Gecode header outside gecode/third-party includes Boost.

## Context

Follow the spike's Prototype sections (Rounding per target, Scope rule, From Boost to native, Comparisons) and its API and ABI impacts in background/native-spike.md; the prototype's float.hh, rounding.hpp, val.hpp, num.hpp, and arithmetic.cpp changes are in build/zdev-impl/boost-007/native-code.patch. Keep FloatVal's public operations and the float constraint API unchanged. val.hpp implements the possibly/certainly comparisons directly instead of catching comparison_error. Adapt Float::Arithmetic::RoundingScope in test/float/arithmetic.cpp to the native types without weakening it, as the spike's Oracle adaptation section describes. The differential test from interval-core now compares FloatVal operations with the Boost interval. Add static_assert(std::is_trivially_copyable_v<FloatVal>) beside the trait checks in test/array.cpp. The rounding implementation's documentation keeps stating which barriers and flags directed rounding relies on (boost-006).

## Boundaries

- The Boost copy stays in the tree for the differential test until interval-removal.
- FloatVal is still passed by const& (value-params-008).
- No external dependency.
- Initial task-owned path allocation: ["gecode/float.hh","gecode/float/rounding.hpp","gecode/float/val.hpp","gecode/float/num.hpp","gecode/float/arithmetic.cpp","test/float/arithmetic.cpp","test/array.cpp"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [x] gecode/float.hh, gecode/float/*.hpp, and the float sources include no Boost header, and FloatVal::FloatValImpType is Float::Interval.
- [x] The differential test, the rounding probe from background/rounding-probe.md against a fresh install, and the float examples' solutions and search statistics match HEAD before the switch.
- [x] RoundingScope checks the same properties with native types, and FloatVal is asserted trivially copyable.

## Validation

- Release and Debug builds with and without MPFR, ctest, and bin/gecode-test -test Float:: in Release; the check and standard tags in Debug.
- The rounding probe matrix against a fresh install, and the four float examples compared with HEAD.
- The syntax-only MSVC check of the _control87 code from the spike (msvc/check.sh).

## Result

FloatVal stores Float::Interval. Float::RoundingBase and Float::Rounding are Gecode classes: mode control with <cfenv>, or _control87 written from Microsoft's documentation under GECODE_FLOAT_MODE_CONTROL87, a non-copyable Rounding scope that saves, initializes and restores the mode, boost-006 barriers kept and documented. val.hpp forwards to the native functions and implements the certainly comparisons without catching exceptions; num.hpp gives pi as hexfloats. No float header includes Boost. RoundingScope keeps all its checks with native types; FloatVal is asserted trivially copyable and Rounding non-copyable; MatchesBoost compares FloatVal with the Boost interval, ignoring only the unspecified sign bit of NaN bounds.

Validation:

- Release/Debug x MPFR on/off builds without warnings; Float:: Release 1567/1567 and 1429/1429, Debug check+standard 533/533 and 512/512; ctest 7/7; rounding probe identical to HEAD in all 9 arm64 configurations; examples identical; diffops at -O2 identical to HEAD (verifier reran it at raw-bit level, 404800 rows); MSVC syntax check unchanged; lint, format and doc clean. erebor x86-64 GCC 15 and Clang 21: examples, probe and diffops match HEAD, Float:: check+standard 533/533. Fresh verifier (codex gpt-6.1-sol high) found the oracle erased all NaN bits; fixed to clear only the sign, rerun locally and on erebor (152/152 each), focused recheck passed.
