+++
schema_version = 1
id = "gcd-003"
key = "nary-product"
area = "gcd"
status = "done"
blocked_by = []
+++
# Implement ordinary and reified n-ary product constraints

## Outcome

Users can constrain an integer result to the exact product of an IntVarArgs array, ordinarily or through any Gecode reification mode.

## Boundaries

- Keep this slice within the integer arithmetic API and propagator implementation; do not add MiniModel or FlatZinc support.
- Implement exact product only; fixed-modulus product belongs to its own task.
- Follow the brief's focused testing level and existing arithmetic and reification test patterns.

## Done when

- [x] Documented public ordinary and Reify product posting APIs accept IntVarArgs factors, an IntVar result, and IntPropLevel and are available from gecode/int.hh.
- [x] Dedicated n-ary actors implement sound posting, propagation, cloning, disposal, subscription, rescheduling, rewriting or subsumption as appropriate, and RM_EQV, RM_IMP, and RM_PMI control.
- [x] The relation uses exact mathematical multiplication, defines the empty product as one, handles singleton, repeated, zero, signed, sparse, assigned, and aliased variables, and never silently overflows internal arithmetic.
- [x] Focused arithmetic tests cover ordinary and reified semantics, all reification modes, fixed and unfixed controls, empty and singleton arrays, zeros, signs, sparse domains, repetition, result aliasing, cloning, and representable-limit cases without new test infrastructure.
- [x] The affected integer library and focused tests build successfully.

## Validation

- Build the configured integer library and test executable.
- Run the focused ordinary and reified Int::Arithmetic::Product tests.
- Run the broader integer test target when available in the existing local build.

## Result

Implemented and independently verified ordinary and reified exact n-ary product constraints with empty-product semantics, alias-aware support filtering, checked arithmetic, and sound large-domain fallback.

Validation:

- Built gecodeint_shared and gecode-test successfully.
- All 18 focused Int::Arithmetic::Product cases passed.
- Broader Int::Arithmetic suite passed.
- Fresh install and installed-consumer smoke tests passed, including large-domain assignment and cloning.
- git diff --check and zd check passed.
