+++
schema_version = 1
id = "boost-009"
key = "interval-switch"
area = "boost"
status = "open"
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

- [ ] gecode/float.hh, gecode/float/*.hpp, and the float sources include no Boost header, and FloatVal::FloatValImpType is Float::Interval.
- [ ] The differential test, the rounding probe from background/rounding-probe.md against a fresh install, and the float examples' solutions and search statistics match HEAD before the switch.
- [ ] RoundingScope checks the same properties with native types, and FloatVal is asserted trivially copyable.

## Validation

- Release and Debug builds with and without MPFR, ctest, and bin/gecode-test -test Float:: in Release; the check and standard tags in Debug.
- The rounding probe matrix against a fresh install, and the four float examples compared with HEAD.
- The syntax-only MSVC check of the _control87 code from the spike (msvc/check.sh).
