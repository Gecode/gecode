+++
schema_version = 1
id = "gcd-001"
key = "ternary-gcd"
area = "gcd"
status = "done"
blocked_by = []
+++
# Implement ternary and reified integer GCD constraints

## Outcome

Users can post ordinary and reified ternary integer constraints for z = gcd(x, y) with the mathematical sign and zero semantics recorded in the brief.

## Boundaries

- Keep this slice within the integer arithmetic API and propagator implementation; do not add MiniModel or FlatZinc support.
- Do not implement the reified divides constraint in this task.
- Follow the brief's focused testing level and existing arithmetic-test patterns.

## Done when

- [x] Documented public gcd posting APIs for the ordinary relation and its Reify overload are available from gecode/int.hh and dispatch through the integer arithmetic implementation.
- [x] A dedicated ternary propagator implements sound posting, propagation, cloning, subscription, rescheduling, and subsumption for negative, zero, sparse, assigned, and aliased domains.
- [x] The reified GCD relation implements sound Boolean control, rewriting or subsumption as appropriate, and RM_EQV, RM_IMP, and RM_PMI semantics for the full proposition z = gcd(x,y).
- [x] The result is constrained to the unique nonnegative gcd, including gcd(0,0)=0 and gcd(0,n)=abs(n).
- [x] Focused arithmetic tests cover ordinary and reified solution semantics, all reification modes, fixed and unfixed controls, propagation, sign normalization, zero cases, sparse domains, aliasing, and cloning without introducing new test infrastructure.
- [x] The affected integer library and focused tests build successfully.

## Validation

- Build the configured integer library and test executable.
- Run the focused ordinary and reified Int::Arithmetic::Gcd tests.
- Run the broader integer test target when available in the existing local build.

## Result

Implemented and independently verified ordinary and reified ternary integer GCD constraints with mathematical sign and zero semantics, alias-aware support filtering, and sound large-domain fallback.

Validation:

- Built gecodeint_shared and gecode-test successfully.
- Focused Arithmetic::Gcd tests passed all 15 variants.
- Broader Int::Arithmetic tests passed.
- Traditional header-list and staged CMake install checks included arithmetic/gcd.hpp.
- git diff --check passed.
