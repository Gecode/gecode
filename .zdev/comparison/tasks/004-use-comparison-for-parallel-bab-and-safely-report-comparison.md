+++
schema_version = 1
id = "comparison-004"
key = "parallel-bab"
area = "comparison"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["comparison-002", "comparison-003"]
+++
# Use comparison for parallel BAB and safely report comparison failures

## Outcome

Parallel BAB arbitrates solutions without mutating them, preserves pending-result correctness, and returns comparison failures to the controlling caller.

## Context

Par::BAB::solution and constrain in search/par/bab.hpp currently probe with constrain/status under m_search. search/par/engine.hpp returns queued results before resuming workers; support/thread/thread.cpp has no exception forwarding. The preceding tasks supply objective implementations and the initial acceptance policy. Follow the ownership and nesting requirements in the [brief](../brief.md).

## Boundaries

- Change BAB arbitration, its required queue/error handling, and focused tests/documentation. Preserve worker locks, ownership, pruning, and recomputation. Limit error transport to comparison and acceptance failures; no general thread-runtime rewrite.

## Done when

- [x] Candidates and external incumbents use incoming-versus-retained comparison. Only accepted incumbents are cloned/broadcast; completed solutions are never constrained or propagated to rank them.
- [x] After an external incumbent update, queued results that violate the new incumbent contract are not returned. Accepted queue ordering and ownership remain valid.
- [x] Missing comparison, model comparison errors, and initial-policy rejection of incomparability wake/block workers safely and reach the controlling call instead of terminating a detached thread, hanging, or reporting normal exhaustion. The engine can be safely destroyed after failure.
- [x] Focused tests check monotone results and the known optimum for a small custom objective, late external bounds with pending results, sub-step float ranking, and safe failure delivery.

## Validation

- Run focused parallel BAB tests and existing BAB stop/resume/recomputation coverage. Use bounded failure tests that detect hangs; do not require deterministic parallel solution counts or schedules.

## Result

Parallel BAB now arbitrates incumbents with non-mutating comparison, invalidates superseded queued results, and safely delivers comparison failures with restart support.

Validation:

- Built the gecode-test target successfully.
- Search::ParallelBABComparison passed 25 bounded iterations, including all failure categories and reset/reuse.
- Focused float and external-incumbent regressions passed repeatedly.
- Existing parallel BAB coverage passed under bounded execution, and git diff --check passed.
