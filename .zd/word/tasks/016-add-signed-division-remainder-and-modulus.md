+++
schema_version = 1
id = "word-016"
key = "signed-division"
area = "word"
status = "done"
blocked_by = ["word-015", "word-013"]
+++
# Add signed division remainder and modulus

## Outcome

Direct posting supports SMT-LIB signed division, signed remainder, and signed modulus while WordVar domains remain unsigned lo/hi masks.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Interpret sign only in operation semantics and do not add a second WordSemantics policy.

## Done when

- [x] Signed division, remainder, and modulus implement their distinct SMT-LIB results for every sign combination.
- [x] Zero divisors and minimum-signed divided by minus one follow total fixed-width semantics.
- [x] Constants, aliases, partial domains, failure, cloning, and recomputation are covered.

## Validation

- Run exhaustive small-width assigned oracle tests for all sign and zero-divisor combinations.
- Run minimum/minus-one, remainder-versus-modulus, partial-domain, alias, and lifecycle tests.

## Result

Added signed_div, signed_rem, and signed_mod by lowering sign handling around the verified unsigned division/remainder core.

Validation:

- Focused Signed arithmetic, lifecycle, TestFramework, exhaustive assigned, tuple-level partial soundness, constants, min/-1, zero divisor, rem-vs-mod, aliases, failure, clone, Boolean extensional parity, replay, and subsumption tests pass; independent verifier PASS.
