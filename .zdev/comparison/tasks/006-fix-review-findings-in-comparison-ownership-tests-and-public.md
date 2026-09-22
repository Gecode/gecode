+++
schema_version = 1
id = "comparison-006"
key = "review-corrections"
area = "comparison"
status = "open"
complexity = "standard"
blocked_by = []
+++
# Fix review findings in comparison ownership, tests, and public contract

## Outcome

Comparison failures do not leak delivered solutions; focused fixtures exercise valid models; the public API documents ordering, pruning, and recovery requirements.

## Context

User approved the five findings of the branch review. Par::BAB::next loses its popped solution if another worker records a failure; Seq::PBS::next loses its candidate when broadcasting throws. PortfolioObjective fixtures inherit a master hook that kills their branchers, and ExternalIncumbent expects a missing comparison from SolveImmediate even though it now overrides compare. The approved brief already contains ordering and nested-cut laws missing from public documentation. Review probes are available under /tmp/gecode-comparison-review.PIGlZy if still present.

## Boundaries

- Keep the comparison API, model-defined readiness, float ranking, and initial incomparability rejection policy unchanged.
- Limit changes to the reviewed ownership paths, focused fixtures/regressions, and public contract/recovery documentation. Avoid a general exception framework.

## Done when

- [ ] Parallel BAB releases any popped solution before rethrowing a comparison failure, and sequential PBS releases its candidate when incumbent broadcasting throws.
- [ ] Focused regression tests check space lifetime after comparison failure and engine destruction; portfolio models preserve/reinstall branchers and unsupported-comparison tests use a genuinely unsupported model.
- [ ] Public documentation states ordering laws, exact-search compatibility with constrain, nested/equivalent cut requirements, compatible asset semantics, float bound ranking, and model-owned readiness.
- [ ] Document conservative, accurate post-exception use/reset/destruction guarantees without promising unimplemented recovery.
- [ ] Rebuilt focused comparison, external-incumbent, BAB/PBS failure, float and FlatZinc checks pass; run existing PBS coverage and applicable no-thread focused checks.

## Validation

- Build gecode-test in build/comparison and build/comparison-no-thread.
- Run focused Search::Comparison, Search::ExternalIncumbent, Search::ParallelBABComparison, Search::PortfolioComparison, Search::FloatObjectiveComparison and FlatZinc objective comparison tests with confirmed matches and process deadlines.
- Run the existing Search::PBS:: matrix at one iteration and focused no-thread comparison/portfolio checks. Validate owned-space cleanup for both reported leaks.
- Run git diff --check and zdev check comparison.
