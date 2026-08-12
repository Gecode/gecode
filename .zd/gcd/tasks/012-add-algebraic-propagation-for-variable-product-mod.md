+++
schema_version = 1
id = "gcd-012"
key = "product-mod-variable-specialisation"
area = "gcd"
status = "open"
blocked_by = []
+++
# Add algebraic propagation for variable product_mod

## Outcome

Variable-modulus product_mod derives useful modulus and result information from identities and assigned products even when Cartesian support enumeration is capped.

## Boundaries

- Keep positivity as part of the full reified proposition and preserve inactive implication no-narrowing behavior.
- Do not add proof logging or change fixed-modulus semantics.
- Do not compute an overflowing exact product; use safe zero detection, checked arithmetic, or divisor reasoning only when the required difference is representable.
- Follow the brief's focused testing level and existing ProductModVar patterns.

## Done when

- [ ] A factor aliased with the modulus forces canonical result zero in the positive relation, while modulus=result remains correctly impossible.
- [ ] The empty product propagates the piecewise identity m=1 implies result=0 and m>=2 implies result=1, including backward pruning from the result.
- [ ] For assigned factors and result, modulus candidates are filtered by m>result and divisibility of product-result; product=result preserves every modulus above result.
- [ ] A zero result exposes sound modulus-divides-product reasoning where the product is safely known, without requiring Cartesian enumeration.
- [ ] Reified false and unfixed paths treat invalid modulus/result combinations correctly and leave inactive implications untouched.
- [ ] Focused tests cover empty products, assigned product/difference divisors, factor-modulus aliases, zero result, sparse and large modulus domains, all reification modes, cloning, and arithmetic limits.
- [ ] The affected integer library and focused ProductModVar tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run focused Int::Arithmetic::ProductModVar algebraic tests with multiple iterations.
- Run fixed-modulus ProductMod regressions and the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.
