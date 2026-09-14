+++
schema_version = 1
id = "word-059"
key = "reified-equality-aliases"
area = "word"
status = "done"
blocked_by = []
+++
# Normalize reified Word equality aliases

## Outcome

Reified equality and disequality immediately resolve identical WordView operands and subsume without waiting for assignment.

## Context

Rel::eq_test currently recognizes only disjoint or fully assigned views, while ordinary Eq and reified ordering already normalize aliases. Update the established reified relation path and extend normal test/word relation lifecycle coverage.

## Boundaries

- Do not change public relation APIs or reification semantics.
- Use the existing relation actors and ordinary registered Word tests.

## Done when

- [x] Identical WordView operands resolve correctly for equality and disequality under RM_EQV, RM_IMP, and RM_PMI.
- [x] Focused tests verify control inference, failure or tautology behavior, actor subsumption, cloning, and recomputation.

## Validation

- Build the normal Release Word library and gecode-test target.
- Run Word::Rel and Word::TestFramework focused tests.
- Run git diff --check.

## Result

Normalized identical WordView operands in reified equality and disequality, with immediate control resolution and subsumption.

Validation:

- Release gecodeword_shared and gecode-test built.
- Word::Rel passed 10 iterations and Lifecycle passed 100 iterations.
- Word::TestFramework passed 10 iterations.
- Independent Spec and Standards verification passed after forcing c_d=1 recomputation.
- git diff --check passed.
