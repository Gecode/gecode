+++
schema_version = 1
id = "open-search-036"
key = "check-all-witnesses-against-optimum"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = ["open-search-034"]
+++
# Check all validated benchmark tours against optimality claims

## Outcome

Benchmark proof validation uses every independently checked incumbent, even when its run is excluded from performance comparisons.

## Context

The independent review and coordinator checked real feasible tours of costs 5 and 10. comparison_report accepts a claimed optimum of 10 when the cost-5 row is invalid-output, because paired_sample removes that row's trace before contradiction checking; changing only its status to timeout correctly rejects the proof. The adjacent infeasibility guard already considers every checked witness. Work in misc/tsptw-benchmark.py and misc/test-tsptw-benchmark.py.

## Boundaries

- Separate proof-consistency checking from performance-quality filtering. Failed rows still supply no proof and stay excluded from quality comparisons.
- Use existing record/parser tests and helpers; no new test harness, benchmark campaign, report-schema change or rewriting of historical measurements.

## Done when

- [x] A focused regression fails before the fix and passes after it when an independently checked cheaper tour comes from an invalid-output or other failed run.
- [x] Every independently checked incumbent cost is considered before status-based performance filtering; valid costs equal to or above the proved optimum are accepted.
- [x] Failed-run quality filtering and existing infeasibility contradiction checks keep their behavior.

## Validation

- Run the existing benchmark record test suite and exercise the checked 5-versus-10 counterexample.
- Confirm failed-run performance samples remain excluded and report structures and historical measurement files are unchanged.
- Check Python syntax and diff whitespace; no solver runs are needed for this report-analysis fix.

## Result

Check all independently validated incumbents against optimality claims before performance filtering.

Validation:

- The existing Records regression failed against baseline for invalid-output and execution-error; independently checked cost-5 and cost-10 tours pass after the fix. Equal/higher costs accepted and failed-run quality excluded.
- All 13 record tests, Python syntax checks and diff whitespace passed. Independent Astra review passed; no solver runs, report-schema changes or historical measurement edits.
