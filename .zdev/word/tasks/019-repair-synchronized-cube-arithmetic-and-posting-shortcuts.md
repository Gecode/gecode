+++
schema_version = 1
id = "word-019"
key = "repair-arithmetic-publication"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = ["word-018"]
+++
# Repair synchronized cube arithmetic and posting shortcuts

## Outcome

Cube arithmetic fallback paths remain sound when their operands use bounded Word domains.

## Context

At cced251d26, four-bit encoded tuples 14+10=0, 4-5=11, 11*13=11 and signed_div(13,1)=12 are accepted as solved. Correct encodings are 8,15,15,13. Inspect arithmetic/add.hpp:200, neg-sub.hpp:293, mult.hpp:314, signed-divmod.hpp:68 and carry/borrow/neg/n-ary variants. The latest terminal fix misses changes created by publishing cubes through bounded narrow(). Existing word-002 deliberately excludes cube fallbacks, explaining part of the test gap. Exact posting reproducers, each in a fresh Space s followed by s.status(): (1) WordVar a(s,4,4,14,WDT_SIGNED,12,4), b(s,4,WDT_SIGNED,10,10), r(s,4,0,10,WDT_SIGNED,10,2); add(s,a,b,r). (2) WordVar a(s,4,0,7,WDT_UNSIGNED,3,7), b(s,4,WDT_UNSIGNED,5,5), r(s,4,3,15,WDT_UNSIGNED,7,11); sub(s,a,b,r). (3) WordVar a(s,4,10,15,WDT_SIGNED,10,14), b(s,4,9,15,WDT_SIGNED,11,15), r(s,4,WDT_SIGNED,11,11); mult(s,a,b,r). (4) WordVar a(s,4,12,15,WDT_UNSIGNED,13,14), b(s,4,WDT_UNSIGNED,1,1), r(s,4,0,13,WDT_UNSIGNED,5,12); signed_div(s,a,b,r). Constructor bounds above are encoded Word values, including negative signed endpoints.

## Boundaries

- Close over synchronization-induced changes before claiming satisfaction or fixpoint, including posting identities and terminal flags.
- Preserve modular semantics and compact-path performance; do not add a second propagation engine.

## Done when

- [x] Focused partial-domain regressions reject all named invalid tuples and retain all supported tuples.
- [x] Negation, carry/borrow and n-ary fallback publication obey the same contract.
- [x] Posting shortcuts and later propagation are checked with mixed kinds and relevant aliases.

## Validation

- Run affected Word arithmetic groups and TestFramework; run all Word tests if shared helpers change.
- Run a compact Add/ARX control before and after any hot-path change.

## Result

Close synchronized cube arithmetic fallbacks before retirement

Validation:

- Word::Arithmetic focused groups passed with 5 iterations
- Word::TestFramework passed with 5 iterations
- Complete Word suite passed with 1 iteration
- Compact ARX and symbolic ALU controls completed successfully
- git diff --check passed
