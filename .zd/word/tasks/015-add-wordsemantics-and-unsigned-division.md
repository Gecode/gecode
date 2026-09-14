+++
schema_version = 1
id = "word-015"
key = "unsigned-division"
area = "word"
status = "done"
blocked_by = ["word-014", "word-008", "word-011"]
+++
# Add WordSemantics and unsigned division

## Outcome

The direct posting boundary carries an extensible WordSemantics policy that defaults to SMT-LIB, and unsigned division and remainder implement total SMT-LIB semantics.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Implement only the SMT-LIB policy in this task.
- Leave signed division, remainder, and modulus to the following task.

## Done when

- [x] WordSemantics is explicit only on operations whose concrete results can vary by policy and does not alter WordVar domains.
- [x] Unsigned division and remainder implement SMT-LIB behavior for ordinary and zero divisors.
- [x] Constants, aliases, failure, cloning, recomputation, and policy plumbing are covered.

## Validation

- Run exhaustive small-width assigned oracle tests, including every zero-divisor case.
- Run partial-domain soundness, policy-plumbing, alias, lifecycle, and differential tests.

## Result

Added WordSemantics with SMT-LIB default and unsigned div/mod through a restoring word-level shift/compare/conditional-subtract decomposition.

Validation:

- Focused Div, Mod, DivisionLifecycle, and TestFramework tests pass for exhaustive assigned semantics including zero divisors, tuple-level partial soundness, both constant orientations, policy plumbing, aliases, failure, cloning, Boolean extensional parity, replay recomputation, and subsumption; independent verifier PASS.
