+++
schema_version = 1
id = "comparison-003"
key = "sequential-incumbents"
area = "comparison"
status = "open"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["comparison-001"]
+++
# Use comparison for sequential BAB and restart incumbent updates

## Outcome

Sequential BAB and RBS select external incumbents by comparison while preserving ordinary pruning and restart behavior.

## Context

Seq::BAB::constrain in search/seq/bab.hpp and RBS::constrain in search/seq/rbs.cpp currently mutate the retained solution and call status(). test/search.cpp has a balanced custom objective and existing restart/portfolio tests. CarSequencing is an example with a custom scalar constrain(). Follow the [brief](../brief.md); the [source map](../background/comparison-and-search.md) distinguishes arbitration from pruning.

## Boundaries

- Change external-incumbent arbitration, the directly affected model overrides, focused tests, and migration documentation. Keep ordinary sequential next(), path pruning, and restart hooks; no best-effort mode or policy framework.

## Done when

- [ ] Incoming incumbents replace retained ones only when better; equal/worse leave them intact. A valid incomparable outcome is explicitly rejected by the initial engine policy, separately from model comparison errors.
- [ ] Actual working spaces still receive constrain(), and recomputation marks and restart state remain correct. A small internal acceptance helper may keep later BAB/PBS policy handling consistent.
- [ ] Relevant custom optimization test models and examples implement their ordering rather than receiving a generic default ranking. Focused tests cover late better/equal/worse updates, the balanced objective, and direct comparison-policy errors.

## Validation

- Run focused external-update tests and existing sequential BAB/RBS search tests, including stop/resume. Smoke-check the migrated optimization example. No new tests of unrelated pruning internals.
