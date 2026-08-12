+++
schema_version = 1
id = "gcd-006"
key = "variable-product-mod"
area = "gcd"
status = "done"
blocked_by = []
+++
# Implement n-ary product_mod with an IntVar modulus

## Outcome

Users can constrain an integer result to the canonical Euclidean residue of an n-ary product under a positive integer-variable modulus.

## Boundaries

- Add the ordinary IntVar-modulus overload without changing the existing fixed-int overload's behavior.
- Keep this slice within integer arithmetic APIs and propagators; do not add MiniModel or FlatZinc support.
- Leave reification to the dependent variable-product-mod-reified task.
- Follow the brief's focused testing level and existing arithmetic-test patterns.

## Done when

- [x] A documented public ordinary product_mod overload accepts IntVarArgs factors, an IntVar modulus, an IntVar result, and IntPropLevel.
- [x] The ordinary constraint enforces modulus > 0 and the canonical result range 0 <= result < modulus.
- [x] A dedicated or appropriately shared actor propagates soundly for assigned and unassigned moduli, negative and zero factors, empty and singleton products, sparse domains, repeated variables, aliases, modulus one, and representable limits.
- [x] Actor posting, subscriptions, propagation, cloning, rescheduling, disposal, rewriting, and subsumption follow existing Gecode conventions without unsafe arithmetic.
- [x] Focused existing-harness tests cover ordinary variable-modulus solutions and propagation, including nonpositive modulus removal and aliases.
- [x] The affected integer library and focused ProductMod tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run the focused ordinary variable-modulus ProductMod tests.
- Run the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.

## Result

Implemented and independently verified ordinary n-ary product_mod with a positive IntVar modulus and canonical Euclidean result.

Validation:

- Built gecode-test and the affected integer library successfully.
- All 25 ProductModVar cases and all 19 fixed ProductMod regressions passed.
- Broader Int::Arithmetic tests, fresh install, and installed-consumer smoke passed.
- git diff --check and zd check gcd passed.
