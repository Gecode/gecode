# Integer arithmetic readiness for Gecode 6.5.0

## Assessment

The local implementation and verification work is complete. The remaining
release gate is the supported-platform CI for [draft PR #240](https://github.com/Gecode/gecode/pull/240).
This is not yet a claim that the complete release matrix is green.

Reviewed initially at `06a6851d54`; fixes and follow-up work are integrated
with `origin/main` at `6b7de57b04` in merge commit `00d8d061ae`.
The changelog contains one unreleased 6.5.0 section. Version/SOVERSION changes
remain a release-wide responsibility.

## Completed work

- Fixed premature fixpoint returns in GCD and positive reified divisibility.
  A delayed divisor/GCD assignment to 2 previously left `x={3,5,6}` after
  propagation, although reposting reduced it to `{6}`. Domain changes now
  request another propagation pass. A focused regression covers both cases.
- Removed repeated space allocation for temporary result-alias product arrays.
  The existing omit-index interval helper now handles that case; allocation
  remains only when a rewrite retains the new array.
- Updated Autoconf dependencies for all four headers, public API propagation
  and exception documentation, and the 6.5.0 release note.
- Added activation, disable/enable rescheduling, cloning, and variable-to-fixed
  modulus rewrite checks. Restored generic fixpoint comparisons for
  `gcd(x,x,x)` and `divides(x,x)`.
- Added independent endpoint and modular square/cube identities, including
  products beyond signed 64-bit range, delayed modulus assignment, negative
  factors, and all reification truth/control combinations.
- Recognize zero products and singleton result aliases in reified exact
  products, and zero-factor/modulus-one identities with variable moduli.
  Positivity remains part of the proposition; the tests check that distinction.
- Audited UBSan testing exposed pre-existing overflow in the `DivMod` and
  `Mod` test oracles. Their arithmetic now uses signed 64-bit intermediates.
- All propagator tests live in Gecode's testing framework in
  `test/int/arithmetic.cpp`, registered like other propagator tests.
  There are no family-specific CI steps, CTest entries, or additions to
  `make check`; the normal test suite handles these propagators.
- Added a [modeling example](integer-number-theory.md) covering signs,
  Euclidean residues, divisibility, and implication semantics.

The Gecode skill informed the lifecycle and memory review. No new propagation
framework, persistent cache, or support enumeration was introduced.

## Lifecycle exceptions

`contest=CTL_NONE` remains appropriate: these propagators do not promise
bounds or domain consistency. Most generic reposting comparisons still use
`testfix=false` for a different, documented reason:

| Family | Interior membership that can strengthen reposting without a bounds event |
| --- | --- |
| Reified GCD | Zero or the GCD computed from assigned operands |
| Reified divides | Zero in the dividend when the divisor is zero |
| Exact product | Zero in factors/result, or an assigned product in the result |
| Fixed modular product | The computed residue, including the empty-product identity |
| Variable modular product | Zero/one in the result or a derived divisor in the modulus |

These are intentional weak-monotonicity cases, not a license to stop before
finishing a propagator's own updates. Family-specific test comments explain
them. Enabling the empty-product comparison reproduced the distinction with
seed 1576866552 and fixprob 1. `NumberTheorySparseBounds` and
`NumberTheoryLifecycle` check own-update completion and actor transitions
separately; applicable alias identity comparisons are enabled.

## Local validation

- All **591 integer arithmetic tests** passed in Release.
- The same **591 tests** passed in Debug with runtime auditing and UBSan,
  with `UBSAN_OPTIONS=halt_on_error=1`.
- The earlier focused lifecycle selection passed three iterations with
  seed 650 and fixprob 1; the completed full-suite runs used one iteration,
  seed 650, and four threads.
- A CMake installation and a separate `find_package(Gecode CONFIG)`
  consumer compiled, linked, and executed all nine public overloads.
  Including the installed `gecode/int/arithmetic.hh` also checked the new
  implementation headers. Results were GCD 6, product -216, residue 1.
- Text changelog generation, repository tidy checks, and
  `git diff --check` passed.

The local host was an Apple M1 Max with Apple Clang 21.0.0. No local Linux,
Windows, or 32-bit execution is implied by these results. Platform coverage
comes from the PR's existing CI matrix, including Autoconf and package checks.

## Scaling sample

This small experiment measures model construction, posting, initial
propagation, and destruction together; it is not an isolated propagation
microbenchmark. Factors have domain `0..1`, so the multiplication-chain
baseline has representable intermediates. Twenty warmup constructions precede
500 measured constructions per sample; results are medians of three samples,
in microseconds. Search is measured separately and exhaustively only at
arities 4 and 12, with the same branching order and solution count checks.

| Arity | Factor pattern | Product (us) | Chain (us) |
| --- | --- | ---: | ---: |
| 4 | distinct | 0.446 | 0.371 |
| 4 | repeated | 0.379 | 0.435 |
| 4 | result alias | 0.475 | 0.362 |
| 12 | distinct | 0.934 | 0.888 |
| 12 | repeated | 0.662 | 0.958 |
| 12 | result alias | 1.136 | 0.838 |
| 64 | distinct | 8.496 | 3.901 |
| 64 | repeated | 6.669 | 4.291 |
| 64 | result alias | 13.094 | 3.921 |
| 256 | distinct | 93.891 | 13.239 |
| 256 | repeated | 76.150 | 14.271 |
| 256 | result alias | 166.132 | 13.393 |

At arity 12, distinct factors took 8,189 search nodes for both implementations.
Repeated factors took 1 versus 3 nodes; result aliases took 4,097 versus 4,117.
All paired exhaustive runs returned the same solution counts.

The quadratic scans are visible at larger arities. A future optimization
should group repeated views once per filtering pass and reuse prefix/suffix
intervals before considering persistent caches. This sample does not measure
modular-product scaling, general signed domains, or application-level speed,
and does not establish a statistically robust performance guarantee.
No optimization was made on the basis of these timings alone.

The small [C++ driver](../misc/bench-number-theory.cpp) and
[Python runner](../misc/bench-number-theory.py) preserve the experiment.
Compile the driver against a Release Gecode build using C++17 and optimization,
then run `python3 misc/bench-number-theory.py /path/to/driver`.
The CSV's zero node/solution fields at arities above 12 mean search was skipped.

## Scope deliberately left out

MiniModel expressions, FlatZinc/MiniZinc exposure, and a non-reified
`divides` overload were excluded by the feature brief. They are possible
future API work, not incomplete deliverables for this PR. Likewise, broad
performance optimization is a follow-up, not a prerequisite for the documented
semantics of these additive constraints.
