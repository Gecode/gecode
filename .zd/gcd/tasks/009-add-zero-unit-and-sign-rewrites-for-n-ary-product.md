+++
schema_version = 1
id = "gcd-009"
key = "product-zero-unit-sign-rewrites"
area = "gcd"
status = "open"
blocked_by = []
+++
# Add zero, unit, and sign rewrites for n-ary product

## Outcome

Exact product posting and propagation simplify assigned zero, one, and minus-one factors and exploit stable aggregate sign information.

## Boundaries

- Keep repeated-variable powers and result-alias identities for the dependent alias-rewrites task.
- Do not add proof logging or change product_mod.
- Preserve all ordinary and reified API semantics, including inactive implications.
- Follow the brief's focused testing level and existing arithmetic-test patterns.

## Done when

- [ ] A fixed-zero factor assigns the positive relation's result to zero; assigned one factors are removed; assigned minus-one factors are removed with the result sign transformed safely.
- [ ] If the result excludes zero, bounds-visible zero endpoints are removed from nonnegative or nonpositive factors, and a zero result with exactly one possible zero source propagates that source to zero when sound.
- [ ] Aggregate sign parity and zero possibility constrain the result sign and permit safe normalisation or rewriting to a positive-magnitude path when all remaining factors have strict signs.
- [ ] Reified actors apply these deductions only when truth is required and preserve no-narrowing inactive implication behavior.
- [ ] Focused tests cover zero, one, minus one, strict and non-strict sign combinations, sign parity, reification controls, cloning, and representable limits.
- [ ] The affected integer library and focused Product tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run focused Int::Arithmetic::Product simplification and sign tests with multiple iterations.
- Run the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.
