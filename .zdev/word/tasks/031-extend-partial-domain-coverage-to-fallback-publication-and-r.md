+++
schema_version = 1
id = "word-031"
key = "audit-word-publication-contract"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = ["word-017", "word-018", "word-019", "word-020", "word-021", "word-022"]
+++
# Extend partial-domain coverage to fallback publication and rewrites

## Outcome

An independent bounded review verifies the repaired publication contract across the remaining Word actor families.

## Context

All 134 existing Word tests passed at cced251d26 despite invalid and lost solutions. Completed word-002 explicitly retained only cases selecting bounded arithmetic actors, excluding the cube fallback interactions at fault. Existing test/word infrastructure and this review's concrete domains supply a focused extension seam.

## Boundaries

- Extend existing oracle machinery; no second test framework, permanent fuzz service or unbounded exhaustive matrix.
- Audit remaining structural/count/channel/reified/conditional publication, assignment and rewrite paths; this task can repair directly demonstrated same-contract defects.
- Do not duplicate regressions already supplied by the fix tasks or treat a complete-assignment check as proof that no supports were lost.

## Done when

- [x] A deterministic small-width set checks preservation of supported tuples, false failure and invalid surviving assignments for cube, bounded and mixed fallback paths.
- [x] Initial posting and delayed tells, operand order, aliases and selected rewrites are covered in the affected families.
- [x] Relevant clone/replay checks pass; any unrelated new defect is described as explicit follow-up rather than silently expanding scope.
- [x] A concise result records actual cases/families and remaining limitations.

## Validation

- Run the focused support oracle, all Word tests and TestFramework on a frozen repaired revision.
- Use assertion-enabled verification for relevant lifecycle paths and warning-clean Release compilation; record actual counts.

## Result

Add a compact cross-family publication audit covering fallback paths, delayed tells, aliases, operand order, reification, rewrites, cloning, and invalid complete assignments; no additional production defect was reproduced.

Validation:

- Independent verification passed: 8 cases across 6 families, 47 supported and 143 rejected tuples; Release and assertion-enabled Debug focused audits and TestFramework; warning-clean Release build; 134 non-Branch Word tests. Follow-up: Word::Branch has a pre-existing seed/order-sensitive failure independent of this audit and requires separate investigation.
