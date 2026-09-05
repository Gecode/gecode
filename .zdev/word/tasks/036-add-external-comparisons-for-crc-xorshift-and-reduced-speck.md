+++
schema_version = 1
id = "word-036"
key = "compare-word-bit-networks"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["word-033"]
+++
# Add external comparisons for CRC xorshift and reduced Speck

## Outcome

Linear bit networks and reduced ARX recovery have faithful external comparison fixtures.

## Context

The historical natural-case sources under word064 no longer survive; recreate from published recurrence definitions, not stale timing claims. xorshift32 uses x^=x<<13; x^=x>>17; x^=x<<5. Reduced Speck varies unknown key bits and rounds independently. Existing CRC/hash examples supply nearby modeling seams.

## Boundaries

- Keep cryptographic claims limited to reduced known instances; no general hash-breaking claims.
- Use concrete reference evaluation and correct observation/key projections; a flipped output bit is not automatically UNSAT.
- Include a small external GF(2) elimination baseline for linear cases only; no production affine domain or solver-wide learning.

## Done when

- [x] CRC/xorshift and reduced Speck cases have known valid witnesses and independently established SAT/UNSAT companions.
- [x] Unknown bits, observations and rounds are separate scaling controls.
- [x] Gecode, Z3 and Bitwuzla use the same decision goal; uniqueness and enumeration are separately labelled.
- [x] The GF(2) control establishes when local propagation loses linear correlations without altering WordVar.

## Validation

- Run reference known vectors, exact small projected checks and capped decision smoke tests.
- Record each recurrence source and actual solver options without adding private payloads.

## Result

Added exact CRC, xorshift, reduced Speck, and GF(2) comparison controls

Validation:

- Independent verifier PASS for snapshot Wce8c2701a4d945fa
- All 15 native Gecode and 15 Z3 exact projection cases passed under the per-cell cap
- Scale-axis validation, known vector, GF(2) correlation-loss, build, syntax, and diff checks passed
