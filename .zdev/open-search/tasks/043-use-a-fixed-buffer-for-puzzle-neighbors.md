+++
schema_version = 1
id = "open-search-043"
key = "stack-puzzle-neighbors"
area = "open-search"
status = "open"
complexity = "routine"
afk = true
blocked_by = []
+++
# Use IntArgs for puzzle neighbors

## Outcome

The shared 15-puzzle neighbor helper returns IntArgs, whose inline storage handles at most four neighbors without heap allocation.

## Context

The user wants the small neighbors helper to avoid obvious allocation waste while remaining exceedingly simple and idiomatic Gecode. Source belongs on feature/open-cp, then comes into feature/open-search. The user selected IntArgs rather than raw buffers. Return IntArgs from neighbors; append using operator<< and filter the previous position into an IntArgs choices in the scramble loop. The existing range loop in post_slide stays unchanged. IntArgs has 16 inline entries, including when copied or returned. Keep up/left/right/down order and stable removal of the previous position so seeded scrambles do not change. Preserve the parent open-route edit.

## Boundaries

- Only the shared neighbor helper, its two callers and now-unused includes. No new helper type, general abstraction, kernel/API changes or permanent tests.
- Coordinator owns source commit, parent integration, stack refresh, verification and publication.

## Done when

- [ ] Neighbor enumeration and scramble candidate filtering use IntArgs inline storage with no vector allocations, preserving every neighbor and the existing order.
- [ ] Parent and child examples build; seeded initial boards and solution paths match the prior version in bounded checks, including a sanitized representative.
- [ ] Change is committed on feature/open-cp and incorporated into feature/open-search; the parent user edit is unchanged.

## Validation

- Inspect all 16 neighbor sets and their order against the board geometry using a temporary check.
- Build parent/child examples and the existing sanitizer configuration. Compare selected seeded outputs with pre-change binaries, ignoring timing, and run one sanitized case.
- Independent source review and git diff --check; no new permanent tests.
