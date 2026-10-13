+++
schema_version = 1
id = "open-search-015"
key = "lnbs-repair-feasibility"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "low"
blocked_by = ["open-search-007", "open-search-012", "open-search-013"]
+++
# Investigate suffix-compatible beam repair with existing LNS

## Outcome

A bounded TSPTW repair study determines how LNBS-style path neighborhoods can reuse Gecode search and portfolio recipes without requiring identical boundary resource states.

## Context

Read the LNBS repair study in ../background/next-search-and-benchmarks.md. Under /Users/zayenz/lab/knowledge/cpkb, read wiki/sources/2023-kuroiwa-large-neighborhood-beam-search-for-domain-indepe.md and sources/derived/papers/2023-kuroiwa-large-neighborhood-beam-search-for-domain-indepe/fulltext.md, especially suffix rollout. Start with the open transition state, its checker, and SpacePortfolioLnsRecipe in gecode/search/portfolio.hpp for neighborhood/build/search lifecycle; compare with the frozen existing CP LNS/BAB configuration.

## Boundaries

- Investigation only, with one fixed segment-neighborhood rule; no new universal LNBS engine, bandit selector or scheduler.
- At most ten minutes solver wall budget on tiny/development cases. Repair quality and global proof are separate.

## Done when

- [x] Demonstrate prefix preservation and independent suffix executability after repairing a segment. A changed boundary time/cost is allowed when the complete suffix stays feasible; exact old boundary equality is not imposed accidentally.
- [x] Tiny cases include a repair with changed resources, an infeasible suffix and objective accounting across prefix/repair/suffix. Existing inner BAB/CABS can be compared without promoting a local optimum to global proof.
- [x] Report checked improvements, repair overhead/storage and the concrete existing recipe/brancher gaps.
- [x] Retain an indexed source-backed conclusion and exact reproduction commands, recommending one bounded follow-up only if useful. No production LNBS integration is silently included.

## Validation

- Check every repaired full tour independently and compare tiny segment optima with enumeration.
- Replay the bounded study and assess ownership/incumbent/stop behavior in the existing recipe seams; no production test harness is added.

## Result

Retained indexed LNBS-style fixed-segment repair findings: complete suffix rollout permits changed boundary resources, native BAB/CABS reach checked local optima, and actual one-attempt recipes publish the first inner candidate. Local proof and unrestricted global coverage remain separate. Development neighborhoods found no improvement; no production follow-up recommended.

Validation:

- Independent whole-task PASS at W5ec7ddc9c0849cfd with equal snapshot comparisons. Original LNBS passage and native recipe/Transition contracts checked; fresh compile and all 24 replay outcomes passed, partial resume and material-change rejection checked. Exact six-permutation neighborhoods, all full tours and 720 full permutations independently checked; local/global proof, interruption, ownership and incumbent probes passed. Combined 75 solver attempts took 1.8115819989761803 s under 600 s; compilation 7.448068998957751 s separate. Two development comparisons use frozen DOM/size BAB. RSS/timing are limited single-run process observations; deployment-target warnings retained.
