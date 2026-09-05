+++
schema_version = 1
id = "word-025"
key = "repair-benchmark-measurement"
area = "word"
status = "open"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = []
+++
# Make Word benchmark timing and memory records reliable

## Outcome

Existing Word runners produce per-run resource records with bounded execution and interpretable timing summaries.

## Context

benchmark.py:449 uses cumulative RUSAGE_CHILDREN.ru_maxrss on macOS, contaminating later runs. A retained XOR run lasted 2.586 s and inflated a mean to 179 ms while median was 7.250 ms. Specialty runners omit timeouts or trust counters from only one repetition. Existing exact-matrix validation is useful and should remain.

## Boundaries

- Extend existing scripts; do not create a general benchmark framework.
- Use per-process RSS or explicitly unavailable values; never subtract cumulative maxima.
- Record actual executable/loaded build and rotate variant order; preserve retained raw results.

## Done when

- [ ] macOS per-run memory uses a genuine individual-child measurement, and unsupported platforms report unavailable.
- [ ] All runners have timeouts and stable-status/counter validation across repetitions.
- [ ] Summaries report median and spread; tiny solves use or clearly require in-process batching; timeout/error rows remain explicit.
- [ ] Existing matrix/resume behavior and no-private-payload rules are preserved.

## Validation

- Use a high-memory child followed by a low-memory child to validate isolation where supported.
- Run a tiny public smoke with interleaved variants, a forced timeout and an intentionally inconsistent result; no permanent tests beyond useful runner regression coverage.
