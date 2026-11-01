# Rethink Boost interval handling

## Objective

Gecode 7 has a deliberate, documented way of providing interval arithmetic
and directed rounding for float variables. The 2012 vendored Boost snapshot is
replaced. The area first settles the approach through an investigation and a
user decision, then implements it.

Success:

- A written decision records the chosen approach, the alternatives that were
  measured, and the reasons. It goes in a Doxygen comment beside the interval
  implementation, or in `docs/` if it spans several files.
- Float variables are sound under the C++23 compilers in CI (GCC, Clang,
  AppleClang, MSVC) on x86-64 and AArch64. Directed rounding cannot be
  defeated by constant folding or by reordering across rounding-mode changes,
  and the build states which compiler flags or pragmas this relies on.
- Gecode has no external dependency for interval arithmetic. Installed
  headers either ship a refreshed copy produced entirely by a script, or a
  Gecode support library that replaces the copy.
- The float test suite passes, and float example models give the same results.

## Current state

These facts were read from the tree at the area base.

- `gecode/third-party/boost` holds 105 files: Boost.Interval and the
  Boost.Config closure it needs. The copy was made in 2012 with
  `extract.py`. Its namespace and macros were renamed to `gecode_boost` and
  `GECODE_BOOST_`, and `patch.txt` records one MSVC x64 rounding fix. The copy
  includes a stray `msvc_rounding_control.hpp.orig` and platform configuration
  for systems Gecode no longer supports (AmigaOS, BeOS, Alpha, IA-64, SPARC).
- The public header `gecode/float.hh` includes the copy. The Boost types are
  part of the API and ABI: `Float::RoundingBase` derives from
  `rounded_arith_opp` (`rounded_arith_std` under MSVC/SSE),
  `Float::Rounding` is `save_state<RoundingBase>`, and `FloatVal` stores
  `interval<FloatNum, policies<Rounding, checking_strict<FloatNum>>>`.
- Gecode uses a small part of Boost.Interval. In `gecode/float.hh` and
  `gecode/float/*.hpp`, uses outside the four arithmetic operators are:
  `user::is_neg`/`is_zero`/`is_pos`, `comparison_error` (caught in
  `float/val.hpp` to implement the possibly/certainly comparisons),
  `checking_strict`, `singleton`, `width`, `in`, `zero_in`, `fmod`, `sin`,
  and the `pi` constants. Transcendental functions use MPFR in
  `Float::Rounding` when `GECODE_HAS_MPFR` is set.
- Rounding control comes from `hw_rounding.hpp`. x86-64 non-MSVC and AArch64
  use the C99 `fesetround` control with a `volatile` store as the rounding
  barrier. MSVC uses `_control87`. Nothing in the CMake build sets
  `-frounding-math`, `-ffp-model=strict`, `/fp:strict`, or
  `#pragma STDC FENV_ACCESS`.

## Candidate approaches

The investigation compares these approaches. Depending on an installed
Boost is ruled out (see Terms and settled decisions).

1. **Refresh the vendored copy.** Regenerate it from current Boost with a
   maintained script. The script computes the smallest header closure, drops
   unsupported platform and compiler configuration, renames namespaces and
   macros, and applies the patches kept as files. Rerunning the script
   reproduces the copy exactly, and a check confirms this, like the
   generated-source freshness check. This carries the least risk, but Gecode
   keeps shipping third-party headers.
2. **Improve the existing copy in place.** Keep the 2012 Boost version, but
   make its pruning and renaming reproducible as in approach 1, and remove the
   stray files and dead platforms. Compare it with approach 1 only if current
   Boost has a larger closure or behaves worse.
3. **Gecode support library.** Implement only the operations above as a
   Gecode type with explicit directed rounding: either a rounding mode that is
   switched and restored, or outward rounding with `std::nextafter`. This
   removes the dependency and the foreign types from the public API. It is the
   most work and touches soundness-critical numeric code.

## Boundaries and non-goals

- Owned: `gecode/third-party/boost`, `gecode/float.hh`, `gecode/float/`,
  float-related CMake options and the source inventory, the float tests, and
  `changelog.in`.
- Float propagation semantics do not change. Any loss of tightness, for
  example from `nextafter`-based outward rounding, must be measured and
  accepted by the user before it ships.
- MPFR support stays optional, with the same `GECODE_ENABLE_MPFR` option.
- Do not redesign `FloatVal`'s public operations or the float constraint API.
  Only the implementation types behind them may change.
- Version and ABI numbers stay at 6.5.0 / 52.

## Terms and settled decisions

- "Directed rounding" means that every computed lower bound is rounded toward
  −∞ and every upper bound toward +∞. Float propagators rely on this to never
  remove a solution.
- Gecode takes on no external dependency. Boost is not found or required at
  build or consumer time. The user chose this to keep Gecode free of
  dependencies, and Boost in particular.
- This is a Gecode 7 breaking change. `Float::Rounding`, `RoundingBase`, and
  `FloatVal`'s implementation type may change spelling and base classes.

## Open questions

The investigation answers all but the last. The user decides the last one.

- Does today's build honor directed rounding under optimization, given that
  no floating-point environment flags are set? A small probe that compares
  folded and runtime results under `FE_UPWARD`, built at `-O2`/`-O3` with
  Clang and GCC, answers this. A failure is a soundness bug and is reported
  separately from the redesign.
- What does current Boost.Interval add over the 2012 copy, and how large is
  its dependency closure?
- How large is approach 3? Estimate it from the operations listed in
  Current state and the support code each needs.
- For approach 3, does `nextafter` outward rounding change the results or the
  runtime of the float tests and float examples compared with switched
  rounding modes?
- Which approach does Gecode 7 adopt: a refreshed copy, an improved copy, or
  a Gecode support library?

## Testing

Broader regression coverage. Interval arithmetic underlies every float
propagator, and a rounding error silently removes solutions instead of
failing loudly.

- The float suite (`-test Float::`) checks propagation against enumerated
  solutions and is the main oracle. Run all of it, including sweep-tagged
  tests, for the final implementation.
- Add focused `Test::Base` tests only for interval-core behaviour that the
  constraint tests do not reach: outward rounding of `+ - * /` and `sqrt` on
  values that are not exactly representable, empty and singleton intervals,
  and the comparison exceptions that `float/val.hpp` relies on.
- The rounding probe from the open questions stays as a test if it can run
  inside `bin/gecode-test`.
- Compare the output and search statistics of the float examples
  (`archimedean-spiral`, `cartesian-heart`, `descartes-folium`,
  `golden-spiral`) before and after the change.

## Validation

- Release and Debug builds with float variables both with and without MPFR,
  `ctest`, and the full float suite.
- `uv run misc/check.py all --build-dir <debug-full-dir> --jobs N` before the
  last commit.
- MSVC cannot be built locally. List the MSVC-specific rounding code in the PR
  description for CI to cover.
