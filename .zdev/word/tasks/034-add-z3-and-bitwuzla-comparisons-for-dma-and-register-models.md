+++
schema_version = 1
id = "word-034"
key = "compare-word-mixed-models"
area = "word"
status = "open"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["word-033", "word-027"]
+++
# Add Z3 and Bitwuzla comparisons for DMA and register models

## Outcome

DMA and register lookup/allocation have equivalent Gecode and external SMT formulations.

## Context

Use the shared contract from the benchmark-controls task and ordinary examples/word-dma-descriptor.cpp and word-register-file.cpp; add allocation controls that exercise current IPL_VAL/IPL_BND. Historical DMA shows bounded Word about tied with Int/Bool at 9.5 ms, with no external comparison. Z3 is the working interpretation of the user's SetFree reference, as recorded in the brief.

## Boundaries

- Encode shared parameters, public decisions and objectives identically; retain native Int/Bool and Word+Int controls where meaningful.
- Use native SMT bitvectors and bounded integer selectors as appropriate; do not force full uint64 ranges into Gecode IntVar.
- Use existing runner hooks or a small external script, not a general import framework.

## Done when

- [ ] DMA and register lookup/allocation each have SAT and justified UNSAT small cases with independently checked witnesses/projection parity.
- [ ] Descriptors/window slack, lookup size/index density and allocation Hall structure vary independently.
- [ ] Z3 and Bitwuzla invocations report versions/options and unsupported or missing solver status honestly.
- [ ] Smoke results distinguish solver comparison from controlled Gecode branching experiments.

## Validation

- Run exact small-model equivalence checks and a bounded one-core smoke, at most 10 seconds per case.
- No broad performance claim until the final campaign.
