+++
schema_version = 1
id = "word-020"
key = "repair-signed-div-overflow"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Handle MIN divided by minus one in bounded signed division

## Outcome

Signed quotient interval pruning preserves solutions across the MIN/-1 discontinuity.

## Context

arithmetic/bounded-divmod.hpp:384 evaluates interval corners. Four-bit signed dividend [-8,-1], divisor -1, unrestricted quotient becomes [-8,1], losing q=7 supported by a=-7. The divisor-sign guard does not exclude this discontinuity.

## Boundaries

- Preserve SMT-LIB wrapping MIN/-1 and zero-divisor semantics.
- Split the exceptional row or conservatively decline the unsound bound rule; no interval-union domain redesign.

## Done when

- [x] The concrete domain retains quotient 7 and all other supported quotients.
- [x] Corner hulls are applied only where sound; signed width-one and width-64 boundary behavior is preserved.
- [x] A small-width support oracle covers intervals containing and excluding MIN with divisor -1.

## Validation

- Run signed division/remainder/modulus and bounded arithmetic tests plus TestFramework.

## Result

Preserve signed quotient supports across MIN divided by minus one

Validation:

- Signed division remainder and modulus groups passed with 5 iterations
- Word::Arithmetic::BoundedLifecycle passed with 5 iterations
- Word::TestFramework passed with 5 iterations
- Compilation and git diff checks passed
