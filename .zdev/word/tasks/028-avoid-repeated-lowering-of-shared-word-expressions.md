+++
schema_version = 1
id = "word-028"
key = "preserve-wordexpr-sharing"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = []
+++
# Avoid repeated lowering of shared Word expressions

## Outcome

Posting a shared WordExpr graph does not expand its repeated subgraphs exponentially.

## Context

minimodel/word-expr.cpp:338 posts both children recursively. Starting e=~WordExpr(x) then repeating e=e|e creates 31,511,8191 live actors at depths 4,8,12, from 6,10,14 expression nodes. Explicitly materializing WordVars avoids this today; SHA/ALU expression reuse can encounter broader fan-out.

## Boundaries

- Reuse nodes within one posting operation or apply sufficient local simplification; never cache variables globally across Homes, separate posts or domain policies.
- Preserve width, semantics, explicit domain-policy handling and exception ownership.

## Done when

- [x] The repeated-sharing family has growth proportional to its expression graph rather than its expanded tree.
- [x] A non-idempotent shared arithmetic/logical fan-out case also reuses work without changing solution semantics.
- [x] Separate posting calls into different spaces or policies remain independent.

## Validation

- Add a focused semantic/sharing regression using the existing MiniModel tests, including distinct-space posting.
- Run Word MiniModel, TestFramework and all Word tests per the brief; measure construction/actor counts on the named family.

## Result

Preserve WordExpr DAG sharing with a per-post node-and-policy lowering cache that leaves ordinary Gecode actors as the only persistent solver state.

Validation:

- Independent verification passed exact 5/9/13 actor growth at depths 4/8/12, exhaustive non-idempotent fan-out semantics, separate posts/spaces/policies, clone/recomputation, MiniModel, TestFramework, fault injection, warning-clean Release and diff checks; known Word::Branch issue remains independent.
