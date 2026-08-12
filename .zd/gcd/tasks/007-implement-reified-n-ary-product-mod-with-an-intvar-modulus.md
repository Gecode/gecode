+++
schema_version = 1
id = "gcd-007"
key = "variable-product-mod-reified"
area = "gcd"
status = "done"
blocked_by = ["gcd-006"]
+++
# Implement reified n-ary product_mod with an IntVar modulus

## Outcome

Users can reify the full positive-variable-modulus product_mod proposition in every Gecode reification mode.

## Boundaries

- Build on the ordinary variable-modulus relation and preserve both existing fixed-modulus overloads.
- Keep this slice within integer arithmetic APIs and propagators; do not add MiniModel or FlatZinc support.
- Follow the brief's focused testing level and existing reified arithmetic-test patterns.

## Done when

- [x] A documented public product_mod overload accepts IntVarArgs factors, an IntVar modulus, an IntVar result, Reify, and IntPropLevel.
- [x] The reified proposition includes modulus > 0, canonical range, and congruence, with correct RM_EQV, RM_IMP, and RM_PMI behavior.
- [x] Inactive implications do not narrow the modulus, factors, or result; positive control rewrites to or shares the ordinary variable-modulus propagator.
- [x] The actor handles fixed and unfixed Boolean controls, nonpositive modulus candidates, sparse domains, modulus one, empty and singleton products, signs, zeros, aliases, cloning, rewriting, and subsumption soundly.
- [x] Focused existing-harness tests cover all reification modes and fixed/unfixed controls, including inactive implications and false supports from nonpositive moduli or noncanonical results.
- [x] The affected integer library and focused ProductMod tests build and pass.

## Validation

- Build the configured integer library and gecode-test executable.
- Run the focused reified variable-modulus ProductMod tests with all reification modes.
- Run the broader Int::Arithmetic tests when available.
- Run git diff --check and zd check gcd --format json.

## Result

Implemented and independently verified reified n-ary product_mod with an IntVar modulus for RM_EQV, RM_IMP, and RM_PMI.

Validation:

- Built gecode-test and the affected integer library successfully.
- All 26 ProductModVar cases and all 19 fixed ProductMod regressions passed.
- Broader Int::Arithmetic tests, fresh install, exported API checks, and installed-consumer reification smoke passed.
- git diff --check and zd check gcd passed.
