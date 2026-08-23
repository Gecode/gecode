+++
schema_version = 1
id = "word-004"
key = "add-realistic-mixed-bounded-model"
area = "word"
status = "open"
complexity = "advanced"
blocked_by = []
+++
# Add the mixed DMA descriptor model and comparison

## Outcome

A checked-in DMA descriptor-window Script exercises bounded address arithmetic, Word flags, ordering, Element, and Boolean-controlled ITE in compact, bounded, Word-plus-Int-channel, and Int/Bool formulations.

## Context

Implement the exact shared model in .zdev/word/brief.md for descriptor counts 3, 6, and 9. Lengths repeat 0x20, 0x30, 0x20, 0x40, 0x30, 0x20. The aligned window has exactly 0x20 spare capacity beyond the fixed total length. Ordered non-overlap distributes those two spare alignment units over the layout. Flags are sorted read-only 0x1, read-write 0x3, and read-execute 0x5 with floor(count/3) writes and executes. Element selects base, end, and flags; selected write controls the limit ITE.

## Boundaries

- Add one ordinary Script example and extend benchmarks/word/benchmark.py, public-manifest.json, and README to run it; do not create a second benchmark framework.
- Implement exactly the four formulations in the brief. The channel formulation uses one compact WordVar and one channel-equal IntVar per numeric field, not BoolVars per Word bit.
- Use the prescribed search order and compare the exact solution count and wrapping 64-bit checksum from the brief.
- Treat timings as measurements, not assertions.

## Done when

- [ ] All four formulations produce the same solution count and checksum for descriptor counts 3, 6, and 9.
- [ ] The model uses actual Word mask/channel constraints for alignment and flags, bounds-sensitive arithmetic and ordering, Element selection, and Boolean ITE control.
- [ ] The benchmark reports nodes, failures, propagations, runtime, current object-layout sizes, and the prescribed retained-clone RSS slope; if the slope remains unmeasurable at the specified cap, it is reported explicitly rather than replaced by entity formulas.
- [ ] The example explains the modeling tradeoffs and does not claim that one public instance predicts all Word workloads.

## Validation

- Build and exhaustively run every formulation at descriptor counts 3, 6, and 9 and compare solution counts and checksums.
- Run interleaved Release trials with linked binary identities and the retained-clone memory procedure from the brief.
- Run the affected example and benchmark checks.
