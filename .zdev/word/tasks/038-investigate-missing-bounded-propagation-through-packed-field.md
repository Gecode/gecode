+++
schema_version = 1
id = "word-038"
key = "investigate-bounded-structure"
area = "word"
status = "open"
complexity = "advanced"
afk = true
priority = "low"
blocked_by = ["word-031", "word-025"]
+++
# Investigate missing bounded propagation through packed-field operations

## Outcome

A small packed-field model identifies which structural interval propagation is worth implementing next.

## Context

Extract/concat/shifts/extensions connect numeric addresses and fields but several paths retain only cube propagation. Use existing structure dispatch plus a DMA/packet or fixed-point ALU case to demonstrate lost endpoint information. Keep research grounded in the CPKB Alt-Ergo 2025 paper and the branch's immutable signed/unsigned representation.

## Boundaries

- Investigate at most two highest-value structural cases; prototypes remain temporary.
- Preserve signed/modular/overshift semantics; do not introduce interval unions or change WordVar representation.
- No production propagator is required by this investigation; a measured rejection is valid.

## Done when

- [ ] A concrete model exposes the gap and a sound candidate or existing decomposition is compared.
- [ ] Small-width oracle validates the proposed rule; compact and wrapping controls show its cost.
- [ ] Result recommends a narrow implementation follow-up or records why it is not worthwhile.

## Validation

- Use capped Release trials, independent semantic checks and source-backed synthesis; retain a concise result, not raw dumps.
