+++
schema_version = 1
id = "open-search-026"
key = "incremental-cabs-descriptor-accounting"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = []
+++
# Remove CABS full-beam descriptor recounts

## Outcome

CABS maintains descriptor counts and storage incrementally instead of scanning every candidate after each admission.

## Context

gecode/search/cabs.cpp:145,155,165 call record_descriptors():170–185, which scans the entire beam. Even an extractor that always declines with nullopt causes quadratic descriptor inspections while filling width W. Frozen Pareto-OFF results do not measure this cost. Matching candidates also uses a linear scan, but changing that index is a separate question.

## Boundaries

- Preserve live-next-layer-only pruning, multiple-victim replacement, beam truncation and metadata forgetting.
- Keep counters local to candidate ownership. Do not introduce a hash index, persistent CABS registry or generalized beam container as part of this task.

## Done when

- [x] Admission/replacement/removal updates current descriptor count and bytes without a full-beam statistics scan.
- [x] Layer/pass/reset and descriptor-limit forgetting update accounting correctly, and public peak statistics keep their documented meaning.
- [x] Declining extraction does not add work proportional to retained beam size solely for statistics.

## Validation

- Run existing beam-discarded representative, incomparable label, replacement and forgetting fixtures. Add only a focused accounting regression if current assertions cannot check the new totals; use a bounded declining-extractor work comparison, not a benchmark campaign.

## Result

CABS maintains live candidate descriptor count/bytes incrementally, preserving forgetting, replacement, layer/reset and public peak semantics.

Validation:

- Independent whole-task PASS at W99f8fb3bca8bf9ff; both exact comparisons equal.
- Before/after native tests and tightened baseline-library fixture, Release/ASAN builds and main-thread tests, check target, inherited tests and examples passed; git diff --check clean.
- Bounded declining-extractor comparison preserved extracts/nodes/restarts/zero peaks and removed source-derived 43946/700074/11188906 full-beam statistics inspections; timings illustrative only.
