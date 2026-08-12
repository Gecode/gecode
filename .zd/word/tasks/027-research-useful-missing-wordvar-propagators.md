+++
schema_version = 1
id = "word-027"
key = "missing-propagators-research"
area = "word"
status = "done"
blocked_by = []
+++
# Research useful missing WordVar propagators

## Outcome

A source-backed gap analysis identifies and ranks additional WordVar constraints that would materially improve modeling or propagation beyond the completed API.

## Boundaries

- This is a read-only investigation; do not implement propagators, change public APIs, add tests, or add benchmark infrastructure.
- Distinguish a missing public modeling operation from a missing dedicated actor for an already expressible operation.
- Prefer operations supported by bit-vector literature, SMT-LIB practice, real modeling use, or established Gecode global-constraint patterns; do not produce an unranked operator wishlist.
- Account for width-at-most-64 cube domains, SMT-LIB semantics, clone footprint, and whether useful propagation can remain word-level.

## Done when

- [x] The current direct-posting and MiniModel operation inventory is checked against the Wang/Wombit papers, relevant bit-vector theory operations, and analogous Gecode constraint families.
- [x] Each candidate records modeling value, whether it is already expressible, expected propagation advantage, plausible implementation shape, and principal risk or limitation.
- [x] Candidates are ranked into recommended next work, worthwhile only after evidence, and deliberately omitted categories.
- [x] The investigation concludes with a small proposed task split for only the highest-value additions, without creating those implementation tasks.

## Validation

- Cross-check every claimed gap against gecode/word.hh, MiniModel WordExpr declarations/definitions, and the current actor inventory.
- Cite the local CP knowledge-base source pages or primary full text near literature-derived claims.
- Run zd status and git diff --check; confirm no production source changes.

## Result

Completed a source-backed gap analysis of the finished WordVar and WordExpr surfaces against Wang/Wombit, current SMT-LIB bit-vector operations, LLVM bit-manipulation practice, and established Gecode constraint families. Highest-value missing modeling constraints are Word-array element and Word-to-Bool reductions; overflow/carry predicates and popcount are strong gated candidates. Dedicated subtraction, Boolean ITE, division/remainder, and n-ary logic are performance candidates for profiling rather than missing semantics. Full Word-Int conversion, generic permutations, variable rotations, and strong all-different remain deferred because of Gecode Int limits, weak cube-domain propagation, or insufficient model evidence.

Validation:

- Cross-checked gecode/word.hh, gecode/minimodel.hh, current actor sources, Wang 2016 and Wombit 2019 local CPKB full text, official SMT-LIB 2.7 FixedSizeBitVectors additions, LLVM primary intrinsic documentation, and analogous Gecode element/count/distinct patterns. Independent read-only research review completed. No production source was changed; git diff --check passed; zd status was fresh, anchor-valid, and branch-matching.
