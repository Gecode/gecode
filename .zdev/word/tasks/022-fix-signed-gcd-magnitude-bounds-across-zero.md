+++
schema_version = 1
id = "word-022"
key = "repair-signed-gcd-magnitude"
area = "word"
status = "open"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Fix signed GCD magnitude bounds across zero

## Outcome

Signed GCD preserves magnitudes supported by interior values in domains crossing zero.

## Context

number_local_min_abs in arithmetic/number.hpp:199 returns the smaller endpoint magnitude if zero is absent. For odd four-bit signed x in [-7,7], signed_gcd(x,x,r) fixes unsigned r=7, losing 1,3,5. The minimum absolute value can occur inside the ranked interval.

## Boundaries

- Use a sound sign-split bound or nearest admitted values without enumerating the full Word domain.
- Preserve signed GCD's unsigned result magnitude and zero/divisibility contracts.

## Done when

- [ ] The odd-domain alias example retains exactly its supported assigned results {1,3,5,7} under complete small search.
- [ ] All users of the minimum-magnitude helper receive sound bounds.
- [ ] Focused tests cover zero-present/absent crossings, single-sign ranges and aliases.

## Validation

- Run Word number/GCD/divisibility tests and TestFramework.
