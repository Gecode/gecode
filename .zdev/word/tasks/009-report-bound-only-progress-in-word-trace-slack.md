+++
schema_version = 1
id = "word-009"
key = "report-bounded-trace-slack"
area = "word"
status = "done"
complexity = "standard"
blocked_by = []
+++
# Report bound-only progress in Word trace slack

## Outcome

Word tracing reports monotone progress when a bounded domain narrows only its ranked interval.

## Context

WordTraceView::slack returns only unknown_size(), although bound deltas are recorded correctly. For bounded views, use the brief's exact metric unknown_size + span_bits, where span_bits is the width-safe ceiling of log2(rank_max-rank_min+1). Compact views retain unknown_size.

## Boundaries

- Handle the full width-64 span without rank-span overflow.
- Keep compact trace slack unchanged.
- Do not change trace text formatting beyond the progress values or add timing assertions.

## Done when

- [x] A pure bound-only update decreases bounded trace slack monotonically and reports nonzero progress.
- [x] Assignment has zero slack and full-width bounded construction has the expected finite saturated slack.
- [x] Width-one, width-64, signed, unsigned, bit-only, and combined-domain trace cases remain correct.

## Validation

- Run focused Word trace tests, including the standard tracer on a bound-only delta.
- Run TestFramework.

## Result

Made bounded Word trace slack include width-safe ranked interval progress while leaving compact slack unchanged.

Validation:

- Independent proof and focused tests covered exact ceil-log2 span semantics, monotonicity, assignment, width-one and width-64 signed/unsigned, bit-only, and combined domains.
- Five anchored iterations of Word::Trace and Word::TestFramework passed; relevant Release production compiled warning-clean and diff checks passed.
