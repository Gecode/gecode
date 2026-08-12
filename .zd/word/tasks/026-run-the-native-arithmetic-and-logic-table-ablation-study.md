+++
schema_version = 1
id = "word-026"
key = "arithmetic-ablation"
area = "word"
status = "done"
blocked_by = ["word-025"]
+++
# Run the native arithmetic and Logic Table ablation study

## Outcome

A temporary development ablation compares the decomposed baseline, native arithmetic only, Logic Table optimization only, and their combination, and the production posting choices follow the measured result.

## Boundaries

- Reconstruct comparison builds from the durable task commits in temporary worktrees or build directories; do not add public runtime or compile-time experiment selectors.
- Keep every benchmark script, fixture, raw result, report, and plot in `/tmp` or another untracked development location.
- Do not add or extend a benchmark CLI, example target, manifest, fixture, plotting dependency, report, or other benchmark infrastructure in Gecode source.
- Do not claim SMT-solver competitiveness, learning behavior, or broad QF_BV robustness from the focused corpus.

## Done when

- [x] Temporary scripts run the four implementation variants over forward assigned, partial-domain, inverse-search, logical-chain, addition, and multiplication slices with captured provenance and raw output outside the repository.
- [x] The temporary analysis records semantic/solution parity, medians, peak RSS, actors, propagation calls, nodes, failures, and width/depth scaling without actor-list traversal in the timed path.
- [x] The recorded conclusions separate native-actor effects from Logic Table effects and their interaction sufficiently to decide the production implementation.
- [x] The final production implementation keeps only measured useful actor/optimization choices and the Git diff contains no benchmark infrastructure, temporary switches, raw results, or private corpus payloads.

## Validation

- Run the focused temporary Release ablation matrix and inspect semantic parity and failed-run output.
- Check that no temporary benchmark artifact or private corpus content is tracked.
- Run focused Word arithmetic, Logic, Conditional, TestFramework tests and git diff --check.

## Result

Ran a temporary four-way Release ablation and retained all three measured production optimizations without adding benchmark infrastructure.

Validation:

- All 620 per-process runs succeeded and all 31 cases preserved solutions, checksums, nodes, and failures across four variants.
- Corrected /usr/bin/time -l RSS measurements, static actor counts, propagation calls, runtime medians, provenance, and resume behavior passed independent audit.
- Focused Arithmetic, Logic, Conditional, and TestFramework selections passed; no benchmark artifacts or private payloads are tracked; git diff --check passed.
