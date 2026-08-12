+++
schema_version = 1
id = "gcd-002"
key = "reified-divides"
area = "gcd"
status = "done"
blocked_by = []
+++
# Implement the reified integer divides constraint

## Outcome

Users can reify the relation divisor divides dividend between two integer variables in every Gecode reification mode, using the mathematical zero semantics recorded in the brief.

## Boundaries

- Expose only the reified divides API requested in the brief; do not add a separate non-reified overload.
- Keep this slice within the integer arithmetic API and propagator implementation; do not add MiniModel or FlatZinc support.
- Follow the brief's focused testing level and existing reified integer-test patterns.

## Done when

- [x] The documented public divides posting API accepts two IntVar operands, Reify, and IntPropLevel and is available from gecode/int.hh.
- [x] A dedicated reified propagator implements sound posting, propagation, cloning, subscription, rescheduling, rewriting or subsumption as appropriate, and Boolean control for RM_EQV, RM_IMP, and RM_PMI.
- [x] The relation uses existential integer-multiplier semantics, including true for 0 divides 0 and false for 0 dividing any nonzero value.
- [x] Focused arithmetic tests cover all reification modes, fixed and unfixed controls, true and false relations, negative and zero values, sparse domains, aliasing, and cloning without introducing new test infrastructure.
- [x] The affected integer library and focused tests build successfully.

## Validation

- Build the configured integer library and test executable.
- Run the focused Int::Arithmetic::Divides tests.
- Run the broader integer test target when available in the existing local build.

## Result

Implemented and independently verified the reified integer divides relation for all reification modes with existential multiplier semantics, signed operands, zero semantics, aliases, and sound large-domain monitoring.

Validation:

- Built gecodeint_shared and gecode-test successfully.
- Focused Arithmetic::Divides tests passed all nine groups for five iterations.
- Broader Int::Arithmetic tests passed.
- Fresh CMake install included arithmetic/divides.hpp.
- git diff --check and zd check passed.
