+++
schema_version = 1
id = "open-search-043"
key = "stack-puzzle-neighbors"
area = "open-search"
status = "done"
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

- [x] Neighbor enumeration and scramble candidate filtering use IntArgs inline storage with no vector allocations, preserving every neighbor and the existing order.
- [x] Parent and child examples build; seeded initial boards and solution paths match the prior version in bounded checks.
- [x] Change is committed on feature/open-cp and incorporated into feature/open-search; the parent user edit is unchanged.

## Validation

- Inspect all 16 neighbor sets and their order against the board geometry using a temporary check.
- Build parent/child examples. Compare selected seeded outputs with pre-change binaries, ignoring timing. The user explicitly waived local sanitizer builds/runs for these model changes.
- Independent source review and git diff --check; no new permanent tests.

## Result

Changed the shared puzzle neighbor helper and scramble candidates to IntArgs, using its inline storage for at most four values. Neighbor order and range-loop use are unchanged. Parent source commit 1736f19a4d is incorporated into the search branch. Independent review passed.

Validation:

- Parent and child example builds passed. Seeds 1, 3 and 7 at scramble 6 and move cap 8 preserve the exact initial boards and all incumbent paths; independently repeated by the verifier.
- All 16 neighbor lists and previous-position filtering were checked. IntArgs construction, append and copy stay within its inline capacity. Source review, clang analyzer, ancestry, whitespace and unchanged parent user edit passed. No permanent tests added.
- User explicitly waived local sanitizer builds and runs for these model changes; no sanitizer run was required or used. Reviewed snapshot W42ef9bd16b8a47f1 compared equal.
