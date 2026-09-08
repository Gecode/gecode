# Integer arithmetic PR readiness for Gecode 6.5.0

Reviewed on 2026-09-08: `feature/gcd` at `06a6851d54`, plus the local
review fixes described below. The feature delta is measured from merge base
`e1ec3da365` with local `main` (`6b7de57b04`, also the locally cached
`origin/main`). Remote refs and CI results were not refreshed.

## Recommendation

Implementation follow-up: the initial fixes are committed as `788879564e`.
Targeted lifecycle tests now cover delayed Boolean activation, disabled actor
rescheduling, cloning before and after activation, and the variable-to-fixed
modulus rewrite. The GCD-all-aliased and divides-self tests now retain the
generic fixpoint comparisons. Other exceptions have family-specific comments
explaining the interior membership observations missed by bounds events.
For example, enabling the empty-product comparison reproduces that legitimate
exception (seed 1576866552, fixprob 1). All 95 focused Release tests pass with
three iterations, seed 650, and fixprob 1. The workflow now executes its Debug
audit arithmetic binary; local audit/UBSan validation is in progress.

Close, but I would not yet give an unconditional release-readiness sign-off.
The implementation is a credible addition to 6.5.0: it has a small public API,
explicit mathematical semantics, dedicated propagators, and substantial
solution coverage. This review found no incorrect accepted solutions or lost
solutions, but did reproduce two incorrect fixpoint claims, now fixed.

Before merging, close the lifecycle-coverage gap below and obtain validation
on the intended integration revision. Add a few independent arithmetic
boundary checks before release. Performance measurements and stronger
propagation are improvements, not reasons by themselves to reject the feature.
No general rewrite or new test framework is warranted.

## Fixed during this review

- **GCD and positive reified divisibility claimed a premature fixpoint.** With
  `x={1,3,5,6}`, an initially variable divisor/GCD in `1..2`, and then assignment
  to 2, both left `x={3,5,6}` after `status()`. For GCD the other operand was 10.
  Reposting reduced the domain to `{6}`. Bounds updates had jumped over holes
  to another non-multiple, while `propagate()` returned `ES_FIX`. Both now
  return `ES_NOFIX` when their own filtering changed a domain. The new
  `NumberTheorySparseBounds` regression checks both delayed-assignment cases.
- **Result-aliased products allocated scratch arrays in the space on every
  propagation call.** When zero remained possible, the temporary `ViewArray`
  was used only to compute an omitted-factor interval and was not retained.
  The code now uses the existing omit-index helper, allocating a persistent
  array only for an actual rewrite. Existing alias tests cover this path.
- **Autoconf incremental dependencies omitted the four new headers.** Added
  them to both affected rules in `Makefile.dep`; checked their inclusion with
  `misc/makedepend.py` from the configured build directory. Header installation
  was already covered by `Makefile.in` and CMake's directory installation.
- **Public documentation did not explain ignored propagation levels or fixed
  modulus exceptions.** The overloads now state the actual propagation
  contract, including conservative reified reasoning, ignored `ipl`, and
  `Int::OutOfLimits` for invalid fixed moduli even with inactive implications.
  The difference from the signed remainder returned by `mod()` is explicit.
- **Release notes omitted the feature.** Added a 6.5.0 entry using the same
  unreleased section heading already present on `main`.

## Remaining work before sign-off

### 1. Restore targeted lifecycle checks — before merge

The new generic tests in [test/int/arithmetic.cpp](../test/int/arithmetic.cpp)
all set `testfix=false` and `contest=CTL_NONE`. Disabling consistency checks is
appropriate when bounds/domain consistency is not promised. Disabling every
fixpoint comparison is a separate decision: it helped hide the two defects
found here.

Several propagators deliberately use `AP_WEAKLY`, so blindly enabling every
reposting comparison would also be wrong. Separate legitimate strengthening
after an interior domain change from failure to finish the propagator's own
bounds updates. Re-enable applicable existing checks and document the specific
reason for each remaining exception. Add only focused regressions for delayed
Boolean activation, sparse endpoints, and clone/reschedule behavior through
rewrites where the existing tests do not establish the invariant.

Acceptance: own-update fixpoints are tested, weakly monotonic exceptions have
an explicit rationale, and the selected tests run with runtime auditing.
The existing workflow's Debug audit smoke configures and builds an audit
binary but does not execute that binary; extend that validation as part of
this task. See [.github/workflows/build.yml](../.github/workflows/build.yml).

### 2. Add independent limit-value checks — before release

Coverage is good for small signed domains, aliases, empty arrays, and all
reification modes. There are also some large-product tests. However, the
product and modular-product test oracles closely follow the implementation's
checked multiplication and modular accumulation. That leaves common mistakes
less likely to be detected independently.

Use a small set of mathematically known answers, not another general oracle:

- GCD and divisibility at both `Int::Limits` endpoints, including zero and
  negative operands.
- With `M=Int::Limits::max`, `(M-1)^2 mod M = 1`, including a negated factor
  and an assigned variable-modulus version.
- A product too large for signed 64-bit arithmetic whose modular residue is
  known, followed by assigning the modulus to exercise the fixed-modulus
  rewrite. Check the result, not only that propagation has not failed.
- The corresponding false/equivalent and inactive implication cases, so
  arithmetic overflow is not confused with a false modular proposition.

Acceptance: expected values are justified independently, and the cases pass
under an undefined-behavior sanitizer on a supported compiler.

### 3. Validate the integration revision — before merge/release

The checks below are local macOS CMake checks, not the release build matrix.
Run the existing supported-platform CI on the revision to be merged, including
Autoconf and installed-header/library consumption. This review did not run
Linux, Windows, 32-bit, sanitizers, an installed consumer, or runtime auditing.
These are unverified areas, not observed platform defects.

Local `main` has five commits absent from the branch. A read-only merge-tree
check of the committed tips found no conflict markers; it does not validate
the uncommitted fixes or a future remote tip. Integrate the review changes
with current `main`, preserving a single 6.5.0 changelog section, then use the
resulting CI evidence. Canonical version metadata still says 6.4.0 on this
branch; updating release version/SOVERSION is a release-wide decision, not a
reason to change the feature API in this review.

## Useful improvements that need not block the PR

- **Measure n-ary scaling before optimizing it.**
  [product.hpp](../gecode/int/arithmetic/product.hpp) scans for repeated views
  in `product_interval`, then groups them again during inverse propagation.
  Both modular propagators recompute omitted-factor intervals in loops. This
  gives quadratic work per filtering pass; multiple passes can add more.
  Compare a few arities with distinct factors, repeated factors, and result
  aliases, recording propagation time and search nodes. For exact products,
  use a multiplication decomposition only where intermediate values fit.
  Prefix/suffix reuse or grouping once may help, but add no persistent cache
  without a measured need.
- **Recognize cheap reified identities earlier.** `product_status` cannot
  establish `product([0,x])=0` until all factors are assigned, and a variable
  modulus misses some analogous zero-factor entailments. These are safe but
  weak cases. Algebraic zero/unit/identity checks could determine the Boolean
  or subsume earlier. Preserve the full positivity proposition for a variable
  modulus and the inactive implication semantics.
- **Document a short modeling example.** Show Euclidean residues for negative
  factors and how to assert divisibility using a true reification variable.
  A non-reified `divides` overload would improve symmetry but is not required
  for correctness. MiniModel expressions and FlatZinc/MiniZinc exposure were
  explicitly excluded by the feature brief and are not missing PR deliverables.

## Verification performed

- Release CMake rebuild of `gecode-test`, with no compiler warnings reported.
- All 588 `Int::Arithmetic::` tests passed after the fixes, one iteration,
  seed 650, four threads. The original branch also passed this selection.
- Debug CMake rebuild and all 94 selected GCD/divides/product/sparse-bounds
  tests passed, three iterations, seed 650, four threads. Runtime audit and
  sanitizers were disabled in this existing configuration.
- The delayed sparse-domain probe reproduced the defect against the old
  Debug library and reached `{6}` immediately against the fixed Release library.
- Text changelog generation succeeded; `git diff --check` passed.

Commands for repeating the test selections from the feature worktree:

```sh
.build-gcd-release/bin/gecode-test -test '^Int::Arithmetic::' -iter 1 -seed 650 -threads 4
.build-gcd/bin/gecode-test -test '^Int::Arithmetic::Gcd' -test '^Int::Arithmetic::Divides' -test '^Int::Arithmetic::Product' -test '^Int::Arithmetic::NumberTheorySparseBounds' -iter 3 -seed 650 -threads 4
```

The Gecode propagator guidance informed the lifecycle and allocation review;
the findings above are based on this branch's code and reproduced behavior.
