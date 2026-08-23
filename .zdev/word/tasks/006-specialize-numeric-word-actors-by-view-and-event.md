+++
schema_version = 1
id = "word-006"
key = "specialize-numeric-word-actors"
area = "word"
status = "done"
complexity = "advanced"
blocked_by = ["word-004", "word-005"]
+++
# Specialize numeric Word actors by view and event

## Outcome

The Word-Int numeric channel and numeric variable-left-shift reasoning use static view properties and the narrowest sound Word propagation conditions.

## Context

Channel::Numeric is runtime-tagged, subscribes to PC_WORD_DOM, and can synchronize a bounded candidate twice. Use the exact channel control in .zdev/word/brief.md: 2,048 unsigned pairs at [1000,2000] and 2,048 signed pairs at [-1000,1000], reach fixpoint, fix encoded bit zero to zero as an endpoint-neutral bit event, then narrow the Int ranges to [1100,1900] and [-900,900]. Retain 100 clones per trial and follow with the DMA comparison.

## Boundaries

- Keep the current public channel API and one public WordVar type.
- Use PC_WORD_BITS for compact channels and PC_WORD_BND for bounded channels; rely on ME_WORD_DOM to wake bounded subscribers when bit synchronization changes endpoints.
- Use bounds events for variable-shift value and result roles and domain events for the amount role.
- Keep clone, reschedule, cancel, cost, and dispose paths symmetric and native to Gecode.

## Done when

- [x] Cube, unsigned, and signed channel paths select static interpretation and sound propagation conditions.
- [x] The bounded channel no longer performs duplicate full synchronization when a range tell suffices.
- [x] Variable left shift does not wake numeric-only value or result roles on endpoint-neutral bit changes.
- [x] The exact 4,096-pair control and DMA model show event, actor-execution, and synchronization reductions without changed solutions or compact regression.

## Validation

- Run focused Word channel and structure tests, including endpoint-neutral bit events, signed values, Int limits, aliases, cloning, and replay.
- Run 20 interleaved Release trials of the exact channel control with 100 retained clones and the DMA comparison.
- Run TestFramework.

## Result

Specialized numeric Word-Int channels and bounded variable-shift subscriptions by static view and event properties.

Validation:

- Independent warning-clean Release, focused Channel/Structure, TestFramework, aliases, clone/replay, width, signed, and Int-limit checks passed.
- The exact 4,096-pair control eliminated all 4,096 endpoint-neutral bounded channel executions with identical actor counts and domain hashes and no compact regression.
- Twenty-trial DMA campaigns preserved solutions and search while reducing channel propagations at sizes 3, 6, and 9; compact propagation counts were unchanged.
