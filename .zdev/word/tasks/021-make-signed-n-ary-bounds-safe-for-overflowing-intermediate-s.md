+++
schema_version = 1
id = "word-021"
key = "repair-nary-signed-sums"
area = "word"
status = "open"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Make signed n-ary bounds safe for overflowing intermediate sums

## Outcome

Bounded n-ary addition never reports failure merely because an auxiliary sum exceeds the Word width.

## Context

arithmetic/bounded.hpp:187 and :210 compute suffix/leave-one-out sums in narrow signed arithmetic after arithmetic.cpp:142 checks only prefixes. Four-bit [-8,-7]+[6,7]+[6,7] fails although every total is in [4,7] and -8+6+6=4 is a witness.

## Boundaries

- Use adequate temporary arithmetic or skip unsupported deductions conservatively.
- Preserve modular public semantics, constant folding, aliases and compact addition.

## Done when

- [ ] The witness and every supported small-width total survive the named example and operand permutations.
- [ ] Prefix, suffix and leave-one-out overflow cannot be confused with inconsistency.
- [ ] Focused tests cover cancellation, constants, result aliases and relevant width-64 behavior.

## Validation

- Run n-ary Add and bounded arithmetic tests plus TestFramework.
- Check existing scatter/gather solution parity.
