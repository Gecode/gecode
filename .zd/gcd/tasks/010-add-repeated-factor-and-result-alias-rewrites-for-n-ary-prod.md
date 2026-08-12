+++
schema_version = 1
id = "gcd-010"
key = "product-alias-power-rewrites"
area = "gcd"
status = "open"
blocked_by = ["gcd-009"]
+++
# Add repeated-factor and result-alias rewrites for n-ary product

## Outcome

Exact product propagation recognises repeated variables as powers and result aliases as algebraic identities instead of independent interval occurrences.

## Boundaries

- Build on the zero, unit, and sign simplification machinery from the blocking task.
- Do not add proof logging or change product_mod.
- Use existing Gecode power, square, view, and actor patterns where they preserve the direct n-ary overflow and zero semantics.
- Follow the brief's focused testing level and existing arithmetic-test patterns.

## Done when

- [ ] Repeated occurrences of the same factor exploit even/odd power structure, including a nonnegative lower bound for even powers over mixed domains and sound inverse power bounds.
- [ ] The implementation does not use an intermediate-product decomposition that rejects a valid final zero merely because an earlier partial product is nonrepresentable.
- [ ] A result variable appearing among the factors is simplified using the identity result=0 or product(other factors)=1, with correct handling of multiple occurrences.
- [ ] Aliased and repeated views remain sound under cloning, propagation, reification, overflow, and zero cases.
- [ ] Focused tests demonstrate stronger pruning for squares/even powers, odd powers, repeated factors, single and multiple result aliases, signs, zeros, and reification modes.
- [ ] The affected integer library and focused Product tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run focused Int::Arithmetic::Product alias and power tests with multiple iterations.
- Run the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.
