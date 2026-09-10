+++
schema_version = 1
id = "comparison-005"
key = "portfolio-comparison"
area = "comparison"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["comparison-004"]
+++
# Integrate comparison through portfolios and complete the migration

## Outcome

PBS, including nested RBS/BAB assets, consistently applies the initial comparison policy and safely delivers failures, with the complete change documented and regression-checked.

## Context

Par::PBS::CollectBest::add/constrain mutate the retained solution; reporter controls subsequent broadcasts. PBS::report and Slave::run coordinate n_busy, tostop, and the completion handshake. Nested asset next() calls can now raise comparison failures. Use search/par/pbs.hpp and hh, search/seq/pbs.hpp, and the existing SEBPBS tests in test/search.cpp; follow the [brief](../brief.md).

## Boundaries

- Integrate portfolio arbitration, necessary nested failure transport, focused tests, and user-facing migration notes. Preserve satisfaction collection, asset scheduling, and completion handshakes. Do not implement best-effort replacement, Pareto archives, or diversity search.

## Done when

- [x] CollectBest accepts only better candidates initially, preserves reporter on rejection, and never mutates a solution merely to rank it. External updates and pending results use the same policy as BAB/RBS.
- [x] Comparison and policy failures from the collector or nested asset calls stop/wake and complete safely, reaching the outer caller; deletion after failure does not hang or race worker completion.
- [x] Focused direct and mixed PBS/RBS/BAB tests establish expected objective improvement/optimum, sub-step float ranking, equivalent-candidate handling, and safe incomparable/error reporting. Existing satisfaction paths remain functional.
- [x] API/search documentation and the Gecode 7 changelog explain migration, model-defined comparability, float ranking, and current rejection policy. They explicitly preserve the possible future best-effort rule 'incomparable incoming solution becomes current best' without claiming completeness or requiring a changed comparison interface.

## Validation

- Run focused portfolio and nested-failure tests, then the existing Search:: suite and relevant FlatZinc checks. Audit remaining constrain/status calls to confirm they are pruning rather than arbitration. Run the existing no-thread build/check path for touched conditional code; no new configuration matrix.

## Result

PBS now applies non-mutating comparison consistently, safely returns nested comparison failures, and documents the Gecode 7 migration and future incomparability policy option.

Validation:

- Threaded and no-thread gecode-test builds passed.
- The complete Search::PBS:: matrix and focused mixed PBS/RBS/BAB, nested-failure, float, external-incumbent, and FlatZinc checks passed.
- Independent verification confirmed ownership, reporter retention, failure quiescence, satisfaction behavior, and remaining constrain/status pruning uses.
- git diff --check and immutable snapshot comparison passed.
