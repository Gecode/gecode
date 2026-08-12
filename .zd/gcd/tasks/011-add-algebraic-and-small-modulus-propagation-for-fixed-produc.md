+++
schema_version = 1
id = "gcd-011"
key = "product-mod-fixed-specialisation"
area = "gcd"
status = "done"
blocked_by = []
+++
# Add algebraic and bounds propagation for fixed product_mod

## Outcome

Fixed-modulus product_mod propagates important algebraic, interval, and congruence cases without enumerating domain tuples or projected supports.

## Boundaries

- Keep the fixed positive int modulus API and Euclidean residue semantics unchanged.
- Do not add proof logging or modify variable-modulus APIs except shared helpers that preserve behavior.
- Use bounds, quotient-band, divisibility, and congruence reasoning; do not build reachable-residue support tables or enumerate factor/result combinations.
- Preserve all reification modes and inactive implication behavior.

## Done when

- [x] Modulus one, a fixed-zero factor, and a factor fixed to a multiple of the modulus force result zero; empty and assigned unit factors simplify directly.
- [x] When exact-product bounds fit one quotient band, the relation rewrites or propagates as result=product-k*modulus, including the no-wrap case.
- [x] Fixed-modulus reasoning derives sound canonical-result and factor bounds from interval, quotient-band, divisibility, and congruence properties without constructing residue supports.
- [x] Domains for which those deductions are inconclusive retain a sound conservative path without domain-product thresholds, support allocation, or tuple enumeration.
- [x] Ordinary and reified focused tests cover algebraic cases, negative factors, wrap/no-wrap bands, sparse domains, aliases, controls, cloning, and large-domain behavior.
- [x] The affected integer library and focused ProductMod tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run focused fixed-modulus Int::Arithmetic::ProductMod tests with multiple iterations.
- Run variable-modulus ProductMod regressions and the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.

## Result

Replaced fixed-modulus Cartesian support filtering with threshold-free algebraic bounds, quotient-band, inverse-bound, zero/unit, and congruence propagation, with sound reified status handling.

Validation:

- Built gecodeint_shared and gecode-test successfully.
- Fixed ProductMod tests passed for five iterations; ProductModVar regression and broader Int::Arithmetic passed.
- Independent spec and standards verification passed after correcting ProductMod scheduling cost; git diff --check and zd check passed.
