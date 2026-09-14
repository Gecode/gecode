+++
schema_version = 1
id = "word-041"
key = "assess-word-next-applications"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "low"
blocked_by = ["word-037", "word-038", "word-039", "word-040"]
+++
# Assess Word application coverage and the next public corpus

## Outcome

A source-backed roadmap selects the next useful application and states which larger integrations remain outside the Word module.

## Context

Reconcile campaign evidence with constrained device/packet configuration, short-horizon occupancy/timetabling, circuit topology synthesis and bounded verification. Use CPKB Wombit 2019, Alt-Ergo 2025, Lagoon/Metodi 2020 and Demirovic/Musliu bitvector timetabling (doi:10.1007/s10479-016-2220-6); check current primary web sources for public SMT-LIB releases and solver capabilities.

## Boundaries

- Research only: no full SMT importer, FlatZinc support, arbitrary widths, SAT explanations, solver-wide learning or production GF(2) domain.
- Use the bit-network task's GF(2) comparison to assess an optional model-level affine global; do not assume a new kernel domain is needed.
- Public QF_BV sampling must define supported operators/width<=64 and report exclusions; do not truncate or misencode integer conversions.
- No broad platform rewrite or large new application suite.

## Done when

- [x] Rank applications by observed fit and missing capability, with one concrete next model and acceptance criteria.
- [x] State whether a native GF(2) global merits an implementation task based on linear-network evidence.
- [x] Define a small stratified public SAT/UNSAT corpus subset and the minimum adapter needed, or explain why current model comparisons should remain the priority.
- [x] Document learning/backjumping, arrays, multiword values and tooling as distinct strategic decisions with evidence; no implicit implementation commitment.

## Validation

- Check recommendations against campaign outcomes and primary papers; distinguish demonstrated results from hypotheses.
- No new unit tests or production code expected.

## Result

Produced a source-backed Word application roadmap selecting a bounded occupancy experiment, deferring GF(2), and defining a deterministic 24-case QF_BV compatibility corpus and benchmark-only adapter.

Validation:

- Recommendations were independently checked against word-036 through word-040 and the named primary papers and official solver/corpus sources.
- The timetable experiment has exact semantic controls and quantitative runtime, search, actor, and clone-memory acceptance criteria.
- The QF_BV corpus, adapter rejection rules, conversion semantics, and distinct strategic exclusions passed independent review at snapshot Wd201711fe9fad38c.
