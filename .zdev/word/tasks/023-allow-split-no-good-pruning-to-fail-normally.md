+++
schema_version = 1
id = "word-023"
key = "repair-split-ngl-assertion"
area = "word"
status = "open"
complexity = "standard"
afk = true
priority = "high"
blocked_by = []
+++
# Allow split no-good pruning to fail normally

## Outcome

A true bounded split no-good fails the space instead of aborting assertion-enabled builds.

## Context

RankLqNGL::prune at word/branch.cpp:300 asserts n < rank_maximum. search/nogoods.cpp:113 legitimately prunes an already-true leaf without checking its status first. Post the no-good forbidding (x<=7 AND y<=7) over unsigned four-bit words, then assign both to 3: debug aborts; Release correctly fails.

## Boundaries

- Keep ordinary no-good and branch-choice contracts; avoid redesigning search.

## Done when

- [ ] The concrete no-good returns SS_FAILED with and without assertions.
- [ ] Focused signed/unsigned split no-good regression covers a true leaf and normal pruning.

## Validation

- Run assertion-enabled Word branching/replay tests and TestFramework; check the same regression in Release.
