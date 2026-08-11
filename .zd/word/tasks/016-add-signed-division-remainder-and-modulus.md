+++
schema_version = 1
id = "word-016"
key = "signed-division"
area = "word"
status = "open"
blocked_by = ["word-015", "word-013"]
+++
# Add signed division remainder and modulus

## Outcome

Direct posting supports SMT-LIB signed division, signed remainder, and signed modulus while WordVar domains remain unsigned lo/hi masks.

## Boundaries

- Interpret sign only in operation semantics and do not add a second WordSemantics policy.

## Done when

- [ ] Signed division, remainder, and modulus implement their distinct SMT-LIB results for every sign combination.
- [ ] Zero divisors and minimum-signed divided by minus one follow total fixed-width semantics.
- [ ] Constants, aliases, partial domains, failure, cloning, and recomputation are covered.

## Validation

- Run exhaustive small-width assigned oracle tests for all sign and zero-divisor combinations.
- Run minimum/minus-one, remainder-versus-modulus, partial-domain, alias, and lifecycle tests.
