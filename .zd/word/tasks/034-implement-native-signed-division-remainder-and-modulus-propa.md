+++
schema_version = 1
id = "word-034"
key = "native-signed-divremmod"
area = "word"
status = "open"
blocked_by = ["word-033"]
+++
# Implement native signed division, remainder, and modulus propagators

## Outcome

The existing signed_div, signed_rem, and signed_mod APIs use compact signed-aware actors or minimal native rewrites over the accepted unsigned core rather than the current sign/absolute-value graph.

## Boundaries

- Preserve WS_SMTLIB zero-divisor, sign, and signed minimum divided by minus one semantics exactly.
- Reuse the native unsigned kernel when that is smaller and equally strong; do not require a distinct class for each surface operation.
- Do not introduce a new signed Word domain/view or expose intermediate magnitudes.
- Use temporary focused Release benchmarks only; no permanent benchmark switches or harness.
- Add normal Gecode-style tests for the signed quotient, remainder, and modulus paths through the shared `test/word` infrastructure established by word-002; benchmarks do not replace registered `gecode-test` coverage.

## Done when

- [ ] Signed quotient, remainder, and modulus postings avoid the width-proportional model graph and provide exact assigned semantics plus documented sound partial propagation.
- [ ] Zero divisors, min/-1, sign-of-remainder versus sign-of-modulus, aliases, constants, failure, cloning, recomputation, and subsumption follow ordinary Gecode behavior.
- [ ] Direct and MiniModel APIs and WordSemantics records remain source-compatible and lower through the accepted native path.
- [ ] A temporary Release comparison against the signed decomposition records actor/propagation/runtime/RSS/search parity and justifies retaining the production change.

## Validation

- Run focused Word signed Div, Rem, Mod, arithmetic lifecycle, MiniModel, and Word::TestFramework tests in Release mode.
- Inspect temporary assigned and partial/inverse benchmark medians including zero and min/-1 cases.
- Run git diff --check and verify no unsigned regression.
