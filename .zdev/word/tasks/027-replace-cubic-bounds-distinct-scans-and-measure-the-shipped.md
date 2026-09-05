+++
schema_version = 1
id = "word-027"
key = "scale-word-distinct"
area = "word"
status = "open"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = []
+++
# Replace cubic bounds-distinct scans and measure the shipped actor

## Outcome

Opt-in Word bounds-distinct uses a scalable Hall-interval algorithm with current implementation evidence.

## Context

word/distinct/bnd.hpp:52 scans n^2 endpoint pairs and n variables per pair, repeating to fixpoint. Identical wide intervals trigger cubic scans even without pruning. The linked word-016 background measures a different sorted Int-derived prototype, not current production. Default IPL_VAL remains pairwise; IPL_BND applies to homogeneous signed/unsigned bounded words.

## Boundaries

- Preserve IPL_VAL default, opt-in bounds semantics and width-64 endpoint safety; do not claim domain consistency over cube holes.
- Adapt the established efficient Hall approach or an equivalently justified algorithm, without a blanket Word-as-Int adapter.

## Done when

- [ ] No-pruning propagation no longer enumerates all endpoint pairs with full inner scans.
- [ ] Hall failure/pruning and signed rank semantics pass focused oracle checks.
- [ ] Current IPL_VAL/IPL_BND/Int-channel controls are measured at the small register model and n=8,16,32,64 plus a bounded large-array no-pruning case.
- [ ] New evidence clearly distinguishes production from the historical prototype; defaults do not change without evidence.

## Validation

- Run Word distinct tests and TestFramework.
- Use Release interleaved timings with construction separated or batched; report counters, runtime/spread and any claimed memory through actual measurements.
