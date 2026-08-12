+++
schema_version = 1
id = "gcd-005"
key = "product-bounds"
area = "gcd"
status = "done"
blocked_by = []
+++
# Replace n-ary product support enumeration with bounds propagation

## Outcome

The positive n-ary exact-product relation performs sound, useful bounds propagation for general integer domains without enumerating Cartesian supports.

## Boundaries

- Keep the public product API and exact mathematical semantics unchanged.
- Do not change product_mod or unrelated arithmetic propagators.
- Follow the brief's focused testing level and existing arithmetic-test patterns.

## Done when

- [x] The ordinary Product actor derives sound bounds for the result and factors using overflow-safe interval arithmetic rather than Cartesian support enumeration.
- [x] Positive reified product cases rewrite to or otherwise use the same bounds propagator, while negative and undecided reification modes remain sound.
- [x] Subscriptions, propagation conditions, costs, fixpoint status, cloning, aliasing, zero crossing, signs, and representable-limit behavior follow Gecode actor conventions.
- [x] Focused regression tests demonstrate useful bounds pruning on domains whose Cartesian size exceeds the former support-enumeration threshold.
- [x] The affected integer library and focused product tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run the focused Int::Arithmetic::Product tests with multiple iterations.
- Run the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.

## Result

Replaced exact n-ary product Cartesian support enumeration with independently verified overflow-safe forward and backward bounds propagation.

Validation:

- Built gecodeint_shared and gecode-test successfully.
- All 19 exact Int::Arithmetic::Product cases passed for five iterations.
- Broader Int::Arithmetic tests and targeted signed, zero-after-overflow, and nonrepresentable-product checks passed.
- git diff --check and zd check gcd passed.
