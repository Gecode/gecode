+++
schema_version = 1
id = "gcd-012"
key = "product-mod-variable-specialisation"
area = "gcd"
status = "done"
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

- [x] A factor aliased with the modulus forces canonical result zero in the positive relation, while modulus=result remains correctly impossible.
- [x] The empty product propagates the piecewise identity m=1 implies result=0 and m>=2 implies result=1, including backward pruning from the result.
- [x] For assigned factors and result, divisor reasoning tightens modulus bounds and detects absence of a feasible divisor without scanning the modulus domain or creating a large set of holes; product=result preserves every modulus above result.
- [x] A zero result exposes sound modulus-divides-product bounds where the product is safely known, without Cartesian enumeration or projecting every divisor into the modulus domain.
- [x] Reified false and unfixed paths treat invalid modulus/result combinations correctly and leave inactive implications untouched.
- [x] Focused tests cover empty products, assigned product/difference divisors, factor-modulus aliases, zero result, sparse and large modulus domains, all reification modes, cloning, and arithmetic limits.
- [x] The affected integer library and focused ProductModVar tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run focused Int::Arithmetic::ProductModVar algebraic tests with multiple iterations.
- Run fixed-modulus ProductMod regressions and the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.

## Result

Replaced variable-modulus Cartesian support filtering with algebraic empty/alias/status reasoning and bounds-only divisor extrema, avoiding dense-domain scans and mass hole creation.

Validation:

- Built the integer library and gecode-test; ProductModVar passed three focused iterations and fixed ProductMod regressions passed.
- Broader Int::Arithmetic passed; dense full-range modulus smoke completed in 0.01 seconds with sound bound propagation.
- Independent spec and standards verification passed; git diff --check and zd check passed.
