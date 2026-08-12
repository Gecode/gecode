# Integer number-theoretic and product propagators

## Objective

Expose ordinary and reified integer constraints for GCD, divisibility, n-ary
product, and fixed- or variable-modulus n-ary product, with sound propagation,
public APIs, and focused regression coverage.

## Success

- The public integer API can post `gcd(home, x, y, z, ipl)` for integer
  variables and constrains `z` to the mathematical greatest common divisor of
  `x` and `y`.
- The GCD relation also has a `Reify` overload supporting `RM_EQV`, `RM_IMP`,
  and `RM_PMI` for the proposition `z = gcd(x,y)`.
- The public integer API can post
  `divides(home, divisor, dividend, reify, ipl)` and supports `RM_EQV`,
  `RM_IMP`, and `RM_PMI` with the same meaning as existing reified integer
  constraints.
- The public integer API can post ordinary and reified
  `product(home, factors, result, ..., ipl)` constraints for an n-ary exact
  product.
- The public integer API can post ordinary and reified
  `product_mod(home, factors, modulus, result, ..., ipl)` constraints for a
  fixed positive integer modulus and canonical Euclidean residue.
- The public integer API can also post ordinary and reified `product_mod`
  constraints whose positive modulus is an integer variable.
- All constraints propagate soundly across negative, zero, sparse, assigned,
  empty, repeated, and aliased variable domains as applicable; their actors
  clone, subscribe, reschedule, rewrite, and subsume according to existing
  Gecode conventions.
- Focused integer tests validate solutions and propagation through the existing
  test harness.

## Boundaries

- Implement integer-variable propagators and their public posting APIs.
- Keep the work in the integer arithmetic constraint family and follow its
  existing header/source/build organization.
- Do not add MiniModel expressions, FlatZinc builtins, or unrelated arithmetic
  constraints.
- Do not introduce a new test harness or broad solver-wide test matrix.
- Only the reified `divides` API is in scope; a separate non-reified overload is
  not required.
- Variable-modulus `product_mod` accepts one `IntVar` modulus; MiniModel and
  FlatZinc exposure remain outside scope.

## Terms

- `gcd(x,y)` is the unique nonnegative greatest common divisor. Negative
  operands are interpreted by absolute value, `gcd(0,n)=abs(n)`, and
  `gcd(0,0)=0`.
- `divisor divides dividend` means there exists an integer `k` such that
  `dividend = divisor*k`. Consequently, zero divides zero, while zero does not
  divide a nonzero integer.
- `product(x)` is the exact mathematical product of all variables in `x`, with
  the empty product equal to one. Implementations must not silently overflow
  while computing supports or bounds.
- `product_mod(x,m)` is the unique residue `r` such that
  `m > 0`, `r` is congruent to `product(x)` modulo `m`, and
  `0 <= r < m`. Thus `product_mod([],m) = 1 mod m`, including zero when `m=1`.
- Reification follows Gecode's `Reify` modes: equivalence (`RM_EQV`), forward
  implication (`RM_IMP`), and reverse implication (`RM_PMI`).

## Decisions

- Use the mathematical zero semantics selected by the user rather than treating
  a zero divisor as unconditionally invalid.
- Expose `IntPropLevel` consistently with neighboring integer arithmetic APIs;
  implementations may map unsupported strengths to the strongest sound level
  they actually provide, as existing APIs do.
- Prefer dedicated propagators and standard Gecode view/actor base classes over
  a decomposition into multiplication, modulo, or extensional constraints.
- Use SMT-LIB's Euclidean remainder convention for `product_mod`, specialized
  to a fixed positive modulus, rather than Gecode's existing dividend-signed
  `mod` convention.
- Pair each ordinary n-ary product relation with a reified overload; reified
  GCD means reifying the full proposition `z = gcd(x,y)`.
- Propagate the positive n-ary exact-product relation from variable bounds in
  general, without making useful propagation depend on Cartesian support
  enumeration or a small-domain tuple threshold.
- For a variable modulus, positivity is part of the `product_mod` relation.
  The ordinary constraint enforces it. Reification controls the full
  proposition, so inactive implications do not narrow the modulus or result.

## Open questions

None.

## Testing

Use focused coverage in the existing `test/int/arithmetic.cpp` framework. Add
solution and propagation tests for small dense and sparse domains containing
negative values and zero, assigned inputs, variable aliasing, and cloning. For
`divides`, exercise all three reification modes and fixed/unfixed Boolean
controls, including the selected `0 | 0` behavior. For `gcd`, cover sign
normalization, the `(0,0)` case, and all reification modes. For `product`, cover
empty and singleton arrays, zeros, signs, repeated or aliased factors, result
aliasing, exact-product overflow boundaries, and all reification modes. For
`product_mod`, cover modulus one, negative factors, canonical residues, empty
and singleton arrays, aliasing, and all reification modes. Variable-modulus
coverage also includes nonpositive candidates, sparse modulus domains,
modulus/result or modulus/factor aliasing, and inactive implication behavior.
Reuse existing arithmetic test helpers and commands; do not create new test
infrastructure or an exhaustive large-domain performance matrix.

## Validation

- Build the affected integer library and test target with the repository's
  established build configuration.
- Run the focused integer arithmetic tests.
- Run the broader integer test target if the established local build exposes it
  without requiring a new configuration.
- Run `zd check gcd --format json` after task-state changes.
