+++
schema_version = 1
id = "word-008"
key = "carry-minimodel-domain-policy"
area = "word"
status = "open"
complexity = "advanced"
blocked_by = []
+++
# Carry explicit bounded policy through Word MiniModel Boolean nodes

## Outcome

Explicit Word MiniModel Boolean factories store a WordDomainType in each Word-backed node so compatible arithmetic intermediates can be bounded through ordinary Boolean composition.

## Context

WordExpr::post accepts a WordDomainType, but Word-backed BoolExpr::Misc nodes call the cube-default overload. Add the exact API settled in .zdev/word/brief.md: word_rel(left, relation, right, domain_type); policy overloads of bit and the three reductions; and overflow overloads whose explicit WordDomainType precedes the optional WordSemantics. Each WordRelation, WordBit, WordReduction, UnaryWordOverflow, and BinaryWordOverflow stores the policy and passes it to WordExpr::post.

## Boundaries

- Keep existing operator syntax and existing overloads cube-default.
- Do not change generic BoolExpr::Misc::post. Policy survives &&, ||, !, and equivalence because each Word-backed miscellaneous leaf stores its own domain type.
- Incompatible WordExpr nodes retain cube representation through the existing node_domain policy.
- Do not add a generic Boolean-domain context or duplicate the Boolean expression system.

## Done when

- [ ] The declared word_rel, bit, reduction, and overflow overloads are public, documented, and lower through the direct Word API.
- [ ] A policy-aware Word Boolean leaf retains its domain type after conjunction, disjunction, negation, and equivalence with ordinary BoolExpr nodes.
- [ ] Default operator and named-function expressions retain existing cube variables, actors, solutions, and propagation counters.
- [ ] Direct posting, manual WordExpr materialization, and the new explicit factories agree on solutions and compatible bounded intermediates.

## Validation

- Run focused MiniModel Word tests for every new factory, Boolean composition, unsigned and signed policy, incompatible fallback, cloning, and replay.
- Run the broader Word MiniModel suite and TestFramework.
- Build the Word and MiniModel libraries warning-clean in Release mode.
