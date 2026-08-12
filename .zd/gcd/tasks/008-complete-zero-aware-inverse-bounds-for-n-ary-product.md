+++
schema_version = 1
id = "gcd-008"
key = "product-zero-aware-inverse-bounds"
area = "gcd"
status = "done"
blocked_by = []
+++
# Complete zero-aware inverse bounds for n-ary product

## Outcome

The exact n-ary Product actor performs sound backward bounds propagation for positive, nonnegative, negative, nonpositive, mixed, and zero cofactor domains.

## Boundaries

- Keep the exact-product public APIs and mathematical semantics unchanged.
- Do not add proof logging, proof encodings, or proof-only state.
- Do not change product_mod in this task.
- Follow the brief's focused testing level and existing arithmetic-test patterns.

## Done when

- [x] Backward factor propagation implements the full zero-aware quotient case split: no filter when cofactor and result both contain zero, failure when a zero cofactor cannot produce the result, magnitude bounds when a cofactor spans zero and the result excludes zero, and directed floor/ceiling quotient bounds for one-sided cofactors.
- [x] Positive, nonnegative, negative, nonpositive, mixed, and fixed-zero domain combinations produce sound bounds without division by zero or signed overflow.
- [x] Cofactor interval computation is structured to avoid unnecessary repeated whole-array recomputation while preserving safe saturation and alias behavior.
- [x] Ordinary propagation and positive reified rewrites reach an honest fixpoint with correct subscriptions, costs, cloning, and subsumption.
- [x] Focused tests exercise every sign/zero case, large domains, aliases, and overflow boundaries through the existing Product harness.
- [x] The affected integer library and focused Product tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run focused Int::Arithmetic::Product sign-class and inverse-bound tests with multiple iterations.
- Run the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.

## Result

Added complete zero-aware inverse bounds for n-ary Product using prefix/suffix cofactor intervals and safe quotient hulls for all sign classes.

Validation:

- Built gecodeint_shared and gecode-test; focused Product tests passed for five iterations and broader Int::Arithmetic passed.
- Independent spec and standards verification, exhaustive small-interval diagnostics, git diff --check, and zd check passed.
