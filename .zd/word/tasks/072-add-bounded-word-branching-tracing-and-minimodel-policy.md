+++
schema_version = 1
id = "word-072"
key = "bounded-modeling"
area = "word"
status = "done"
blocked_by = ["word-071"]
+++
# Add bounded Word branching tracing and MiniModel policy

## Outcome

Bounded Word variables are natural to branch on, inspect, trace, and select through model options without changing constraint semantics.

## Context

Implement stage two of the word-070 architecture after the kernel domain is stable.

## Boundaries

- Do not migrate arithmetic or division actors here.
- Keep existing bit branchers and cube MiniModel lowering backward compatible.

## Done when

- [x] Rank-min/max/median assignment and interval-split branchers avoid values excluded by the cube-range intersection and support archive, no-good, clone, and replay.
- [x] Trace and printing expose domain kind, cube, canonical endpoints, and bound/combined deltas.
- [x] A model option and explicit MiniModel lowering policy make cube/unsigned/signed experiments differ only in construction or selected policy; incompatible expression nodes fall back to cube.
- [x] ABI, Doxygen, examples, and focused lifecycle tests cover the new modeling contract.

## Validation

- Build Release examples and tests.
- Run focused Branch, Trace, MiniModel lifecycle, TestFramework, archive/replay, and example-option checks.
- Run zd check word and git diff --check.

## Result

Added ranked bounded Word split and assignment branching, 64-bit choice archival and no-goods, bounded printing/tracing, recursive MiniModel domain policy, and a symbolic-ALU domain option.

Validation:

- Release Branch, Trace, MiniModel Lifecycle and TestFramework pass at 10 iterations; full Word suite passes; cube/unsigned/signed ALU solution sets match; changelog tidy, zd and diff checks pass; independent verifier PASS.
