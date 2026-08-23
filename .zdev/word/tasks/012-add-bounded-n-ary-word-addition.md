+++
schema_version = 1
id = "word-012"
key = "add-bounded-nary-add"
area = "word"
status = "open"
complexity = "advanced"
blocked_by = []
+++
# Add bounded n-ary Word addition

## Outcome

Homogeneous bounded n-ary sums perform staged interval sum and inverse reasoning without forcing models into binary trees of intermediate variables.

## Context

Use the exact scatter/gather model in .zdev/word/brief.md. For 4, 6, and 8 segments, each width-12 unsigned length lies in [64,256], has low four bits zero, and is ordered nondecreasingly; one native n-ary Add fixes total to 160*count. The largest possible sum is non-wrapping. Compare solution count and the specified wrapping weighted-length checksum.

## Boundaries

- Start with same-kind bounded operands in a proved useful non-wrapping regime; preserve cube fallback elsewhere.
- Preserve constant folding, modular semantics, aliases, exact assigned behavior, cloning, and recomputation.
- Run cheap bound closure on bound-only events and the existing O(width times arity) cube projection only when bit work is required.
- Do not introduce permanent intermediate variables.

## Done when

- [ ] A bounded n-ary actor performs forward sum bounds and sound inverse operand bounds for the supported regime.
- [ ] Bound-only and bit stages advertise honest distinct costs and reschedule correctly when synchronization exposes bits.
- [ ] A task-owned small-width oracle covers constants, aliases, signed and unsigned cases, fallback, and width-64 arithmetic safety.
- [ ] The exact 4-, 6-, and 8-segment models preserve counts and checksums and show useful search or runtime value without changing compact NaryAdd behavior.

## Validation

- Run focused Word n-ary arithmetic tests and its task-owned small-width oracle.
- Run 20 interleaved Release comparisons on all three scatter/gather scales and compact controls.
- Run the broader Word arithmetic suite and TestFramework.
