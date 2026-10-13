+++
schema_version = 1
id = "open-search-014"
key = "beam-stack-feasibility"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "low"
blocked_by = ["open-search-007", "open-search-012"]
+++
# Investigate fixed-width beam-stack regeneration in Gecode

## Outcome

A bounded study establishes whether BSS-style regeneration can preserve coverage and reduce retained Space memory, and gives a concrete go/no-go follow-up recommendation.

## Context

Read ../background/next-search-and-benchmarks.md BSS recommendation, the CP 2026 original border/total-order passages, and native n-ary/prepare/incumbent invariants. Start with the worked transition model and eager control; compare a concrete-state border with a retained source clone/original Choice cursor. Gecode CP domains and weakly monotonic propagation are not the paper's concrete DP states.

## Boundaries

- Investigation only: keep prototypes outside tracked source and retain a concise reproducible conclusion, not a production engine or generic replay framework.
- At most ten minutes solver wall budget on tiny/development cases; no claim of O(Bd) bytes or universal Space ordering.

## Done when

- [x] Work out a coverage-preserving continuation contract for arbitrary native Spaces or identify the exact model restriction needed. Include equal ranking/bounds, n-ary alternatives, changing incumbents, variable-depth goals and interrupted expansion.
- [x] A tiny prototype or established counterexample checks no lost alternative against enumeration; report regenerations, peak retained Spaces/Choice/border data and measured process/Space cost separately.
- [x] The conclusion compares BSS with current CABS storage/repetition on the same transition model and explains whether total ordering, descriptors or saved choices suffice.
- [x] Retain source-backed findings and reproduction commands in an indexed area background document. Recommend a bounded implementation task only if supported, otherwise specify why to defer; production adoption requires a separately reviewed follow-up.

## Validation

- Read original definitions/proofs and inspect the prototype's exact tiny coverage cases independently.
- Replay the recorded bounded commands and compare checked tours and memory counts; no new production tests are expected.

## Result

Retained an indexed BSS continuation study: immutable border pagination and original-Choice cursors preserve checked coverage on restricted deterministic transitions. Cursor used fewer commitments with different ordering; generic Space heuristic border replay is deferred. No production engine or follow-up task added.

Validation:

- Independent verifier PASS at Wf4c8eae006f1eef7; original sources and native contracts inspected; fresh compilation plus all 73 documented replay launches/checks passed. Tiny tour identities and 720 failed paths match enumeration, optimum 9/infeasibility and external adoption checked, all logical counts reproduced. Combined 221 solver launches took 4.292976246913895 s under 600 s; compilation 11.489148541998584 s separate. RSS varies and does not establish exact Space bytes or universal gains; deployment-target warnings recorded. Exact snapshot comparisons stayed equal.
