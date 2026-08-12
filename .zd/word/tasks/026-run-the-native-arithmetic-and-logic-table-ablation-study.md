+++
schema_version = 1
id = "word-026"
key = "arithmetic-ablation"
area = "word"
status = "open"
blocked_by = ["word-025"]
+++
# Run the native arithmetic and Logic Table ablation study

## Outcome

A reproducible benchmark report compares the decomposed baseline, native arithmetic only, Logic Table optimization only, and their combination, and the production posting choices follow the measured result.

## Boundaries

- Reconstruct comparison builds from the durable task commits in temporary worktrees or build directories; do not add public runtime or compile-time experiment selectors.
- Extend the existing benchmarks/word single-file harness and artifact contract rather than adding a second benchmark framework.
- Keep benchmark payloads and raw results outside tracked source; commit only reusable fixtures, manifests, harness changes, and concise documentation warranted by the result.
- Do not claim SMT-solver competitiveness, learning behavior, or broad QF_BV robustness from the focused corpus.

## Done when

- [ ] The harness can run and resume the four implementation variants over forward assigned, partial-domain, inverse-search, logical-chain, addition, and multiplication slices with raw stdout/stderr and provenance.
- [ ] Analysis reports semantic/solution parity, medians, peak RSS, actors, propagation calls, nodes, failures, and width/depth scaling without actor-list traversal in the timed path.
- [ ] Markdown plus reusable scaling plots present claim-by-claim conclusions and separate native-actor effects from Logic Table effects and their interaction.
- [ ] The final production implementation keeps only measured useful actor/optimization choices and contains no temporary switches, copied private corpus, or benchmark-only production hooks.

## Validation

- Run a public smoke matrix through run, analyze, plot, and report, including resume behavior.
- Run the focused Release ablation matrix and inspect semantic parity and failed-run artifacts.
- Scan tracked/package payloads for raw results or private corpus content.
- Run focused Word arithmetic, Logic, Conditional, TestFramework tests and git diff --check.
