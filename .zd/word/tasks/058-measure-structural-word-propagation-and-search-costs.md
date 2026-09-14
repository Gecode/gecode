+++
schema_version = 1
id = "word-058"
key = "measure-word-structural-performance"
area = "word"
status = "done"
blocked_by = []
+++
# Measure structural Word propagation and search costs

## Outcome

A cross-corpus structural profile identifies multiplicative Word performance costs in actor topology, notification fan-out, propagation waves, information yield, cloning, recomputation, and scaling that targeted hot-function benchmarks do not reveal.

## Boundaries

- Investigation only: retain no production, test, example, kernel, search-default, or benchmark-framework change.
- Use the five checked-in examples and only bounded auxiliary model variants or existing Boolean decompositions needed to separate implementation overhead, cube-domain propagation limits, and model topology.
- Keep instrumentation detached and temporary; do not track raw results, probes, private data, or a durable profiling runner.
- Propose a follow-up only for a concrete established Gecode mechanism with a sound contract and a cross-workload measurement gate.

## Done when

- [x] The current exact Release baseline has root actor/advisor topology and estimated space-owned bytes by family, plus actor/advisor copies and estimated copied bytes per node for representative bounded searches.
- [x] Measurements quantify propagation calls per node/input decision/newly fixed Word bit, notification fan-out, useful narrowing/failure/subsumption/no-visible-work yield, and propagation-wave depth after branch commits by important actor family.
- [x] At least three-point scaling curves for each workload family distinguish width, unknown-bit, or round-depth growth using normalized slopes, and c_d/a_d comparisons quantify clone-byte versus replay-propagation tradeoffs without selecting a global policy.
- [x] The analysis compares relevant Word and established Bool/decomposition shapes, ranks confirmed and rejected structural hypotheses, and records only evidence-backed follow-up tasks in dependency order.
- [x] No temporary instrumentation or result artifact is tracked, and focused quick example runs, zd check word, and git diff --check pass.

## Validation

- Build the current Word examples and any detached instrumented Release variant, then run bounded configurations under hard time limits with exact solutions/checksums and search counters.
- Reproduce selected structural ratios independently from preserved temporary commands/raw data and run all five quick defaults.
- Run zd check word and git diff --check, confirming that only zdev planning state changed.

## Result

Completed and independently verified a structural profile of Word propagation, actor topology, copy traffic, propagation waves, scaling, and search restoration.

Validation:

- Exact f1016bd3cd detached instrumentation, five-example scaling, CRC Word/Bool comparisons, c_d/a_d measurements, selected independent reruns, all five defaults, zdev checks, and diff check passed; no production follow-up was justified.
