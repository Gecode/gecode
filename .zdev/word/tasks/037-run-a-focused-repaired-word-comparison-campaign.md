+++
schema_version = 1
id = "word-037"
key = "run-word-external-campaign"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["word-034", "word-035", "word-036", "word-028", "word-029"]
+++
# Run a focused repaired-Word comparison campaign

## Outcome

A reproducible bounded campaign establishes where repaired Word models are competitive with Z3 Bitwuzla and native Gecode alternatives.

## Context

Use six families: DMA, register lookup/allocation, inverse arithmetic, CRC/xorshift, reduced Speck and bounded ALU. The review proposed 3 calibrated levels and 4 independent instances per family/level (72 instances). Keep calibration separate. Current historical DMA medians are not current HEAD evidence. Public SMT-COMP results justify Bitwuzla as a specialist baseline but do not predict these mixed models.

## Boundaries

- Run a frozen Release revision and fixed solver builds/options on one core without competing benchmark load.
- Screen with one run and a 30-second per-case cap; spend at most six aggregate CPU-hours across this task, then record deferred work explicitly. Selected follow-ups may use five interleaved trials up to 300 seconds each within that budget.
- Preserve timeouts/unknown/errors/deferred rows; do not claim the full repeat matrix completed if budget limits it.
- No new solver feature during the measured campaign; implementation defects invalidate affected rows and require a documented rerun.
- Enforce a 4 GiB per-run memory budget through a supported host mechanism, record the mechanism and limit, and classify memory-limit terminations separately. If enforcement is unavailable, mark that configuration unsupported rather than silently running uncapped.

## Done when

- [x] Each planned instance/configuration is accounted for as measured, unsupported or deferred; semantic validation precedes accepted timing.
- [x] Report status split by SAT/UNSAT, median/spread, paired family comparisons, per-process RSS and timeout-aware metrics; do not compare Gecode propagations to SMT conflicts as identical work.
- [x] Small timing comparisons avoid process-startup dominance and controlled Gecode branching is distinguished from native-solver search.
- [x] A concise durable result explains useful application ranges, losses and next one or two priorities with exact revision/options and retained external result locations.
- [x] Use one documented baseline Gecode search and one alternative selected only on calibration instances. Freeze both before measured runs and include both in matrix accounting and the same six-hour total CPU budget.

## Validation

- Validate planned matrix completeness and witness checks, then reproduce a small sample independently.
- No new unit tests expected; do not change production code to improve a measured outcome.

## Result

Ran a reproducible bounded six-family Word comparison campaign with validated batching, resource limits, and durable analysis.

Validation:

- Final 288-cell screen: 216 semantically validated measurements, 72 explicit Bitwuzla unsupported cells, zero errors or deferrals.
- Six selected cases completed five interleaved follow-up trials; report separates screen and repeat evidence with paired ratios, RSS, and timeout-aware counts.
- Frozen runner/source and binary/image hashes match the retained external root; one-core and 4 GiB limiter plus OOM classification probes passed.
- Independent verifier passed snapshot Wa47dfb3b45ccb6ad; Word::Arithmetic passed five iterations.
