+++
schema_version = 1
id = "gcd-011"
key = "product-mod-fixed-specialisation"
area = "gcd"
status = "open"
blocked_by = []
+++
# Add algebraic and small-modulus propagation for fixed product_mod

## Outcome

Fixed-modulus product_mod propagates important algebraic cases and small reachable residue sets without relying on full Cartesian tuple enumeration.

## Boundaries

- Keep the fixed positive int modulus API and Euclidean residue semantics unchanged.
- Do not add proof logging or modify variable-modulus APIs except shared helpers that preserve behavior.
- Use bounded residue-set propagation only where its memory and runtime are controlled by an explicit small-modulus policy.
- Preserve all reification modes and inactive implication behavior.

## Done when

- [ ] Modulus one, a fixed-zero factor, and a factor fixed to a multiple of the modulus force result zero; empty and assigned unit factors simplify directly.
- [ ] When exact-product bounds fit one quotient band, the relation rewrites or propagates as result=product-k*modulus, including the no-wrap case.
- [ ] For explicitly bounded small fixed moduli, reachable residues are propagated incrementally across factors without enumerating the full Cartesian product and project sound supports to the result and factors where maintained.
- [ ] Large-modulus or unsuitable domains fall back to the existing sound path without unbounded allocation or work.
- [ ] Ordinary and reified focused tests cover algebraic cases, negative factors, wrap/no-wrap bands, sparse domains, aliases, small residue sets, controls, cloning, and large-domain fallback.
- [ ] The affected integer library and focused ProductMod tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run focused fixed-modulus Int::Arithmetic::ProductMod tests with multiple iterations.
- Run variable-modulus ProductMod regressions and the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.
