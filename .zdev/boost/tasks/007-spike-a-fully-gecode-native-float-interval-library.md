+++
schema_version = 1
id = "boost-007"
key = "native-spike"
area = "boost"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = ["boost-003"]
+++
# Spike a fully Gecode-native float interval library

## Outcome

A working prototype in a scratch copy of the tree replaces every use of gecode/third-party/boost with Gecode-native code, and a study measures it against HEAD and records its wider impacts, so the user can decide boost-004.

## Context

Read the brief in full, then background/approach-study.md (its part (3) lists the operations, the Boost code behind them by line count, the rounding layer, the comparison semantics, the API and ABI changes, and the soundness-critical parts; its probes and scripts under build/zdev-impl/boost-003/ are ready oracles), background/rounding-probe.md, and background/nextafter-study.md. Build the prototype in a scratch source copy under build/zdev-impl/<task-id>/ and keep it there: an interval type with the checking_strict empty-interval semantics, the possibly/certainly comparisons that gecode/float/val.hpp now gets by catching comparison_error, the four arithmetic operators and the existing optimization barriers from boost-006, sqrt, fmod, pow and nth_root, the pi constants, the MPFR transcendental functions, and switched-mode rounding control for every target Gecode supports, including the MSVC _control87 path written from source. Keep the switched-mode rounding of today; nextafter is a separate later decision. FloatVal's public operations and the float constraint API keep their signatures. Port the algorithms as they are so bounds stay the same; where the port deliberately differs, say why. Some oracles use Boost directly: the Float::Arithmetic::RoundingScope test in test/float/arithmetic.cpp instantiates Boost policies, intervals, fmod, and sin, and the approach study's comparison probe constructs raw Boost intervals and catches Boost exceptions. Give them native equivalents in the scratch copy that check the same things (the public comparison truth table, rounding-mode restoration, and the same operations); removing or weakening a check to get a passing suite is not acceptable, and the study lists each adapted check. Then measure against HEAD with the same oracles as the approach study: the full Float:: suite (including sweep-tagged tests, with and without MPFR), the four float examples' solutions and search statistics, the rounding probe matrix, the nth_root and comparison probes, the DirectedRounding test, runtime of the examples and of a FloatVal microbenchmark, build time of the float module, and the installed header footprint. Record the wider impacts: public API and ABI changes in gecode/float.hh (Float::Rounding, RoundingBase, FloatValImpType, any gecode_boost name in installed headers); whether FloatVal becomes trivially copyable once both causes are removed (its own hand-written copy constructor and copy assignment in gecode/float/val.hpp and the Boost member type), measured with std::is_trivially_copyable_v on the prototype, and what that means for the value-params rule (FloatVal by const&, which stays unless the user revisits it) and the modern-classes area; FlatZinc, Gist, the examples, and known downstream users that use these types; MPFR stays optional; the MSVC and x86-64 code that only CI can test; CMake sources, install rules, and the Boost license file; the work boost-005 would need in tracked code, as a proposed child split; and any risk that remains. Write .zdev/boost/background/native-spike.md, keep a patch of the prototype against HEAD under build/zdev-impl/<task-id>/ and name its path, and index the study in the brief.

## Boundaries

- No tracked source changes; the prototype, builds, and outputs stay under build/.
- No external dependency, and no Boost code in the prototype.

## Done when

- [x] .zdev/boost/background/native-spike.md records the prototype's design and size, every measurement against HEAD with the command that produced it, the wider impacts listed in the context, and the proposed child split for boost-005, and the brief indexes it.
- [x] The prototype builds with and without MPFR in Debug and Release, and any difference from HEAD in bounds, solutions, search statistics, or test results is listed and explained, or the study says the spike failed and why.

## Validation

- Every number in the study names the command or measurement that produced it.
- Rerun the rounding probe and the Float:: suite with MPFR on the prototype from the recorded commands.

## Result

Spike verdict: success. A Gecode-native double-only Float::Interval and rounding layer, ported from Boost's algorithms with no Boost code, built in a scratch copy, matches HEAD bit for bit on every oracle on arm64 macOS and x86-64 Linux (GCC 15, Clang 21); only the empty-interval exception text differs. 843 new lines, 95 fewer installed headers, identical exports, same or faster runtime. FloatVal becomes trivially copyable. Study: background/native-spike.md, with a proposed four-part boost-005 split. Open for the user: real MSVC execution needs an authorized CI run; whether the ported file's BSL-1.0 attribution is acceptable.

Validation:

- Fresh verifier (codex gpt-6.1-sol high) reran the native MPFR Float:: suite (1563/1563 passed), all nine rounding-probe binaries (identical to HEAD, 0 unsound), -O2 diffops (404800/381920 rows match), nth-root, comparison, pi and copy-trait probes, patch hash and installed-file counts; two study claims (export count, grep scope) corrected and passed a focused recheck.
