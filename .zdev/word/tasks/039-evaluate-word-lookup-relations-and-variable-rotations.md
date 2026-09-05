+++
schema_version = 1
id = "word-039"
key = "investigate-word-lookup-rotations"
area = "word"
status = "open"
complexity = "advanced"
afk = true
priority = "low"
blocked_by = ["word-031", "word-025"]
+++
# Evaluate Word lookup relations and variable rotations

## Outcome

Concrete decoding/substitution and rotation models determine whether native constraints improve on current decompositions.

## Context

Word has Int-indexed Element and constant rotations, but no variable-amount rotation API or general small Word table relation. Compare a small instruction decode or byte substitution using Word/Int channels and Element, and a variable rotation using shift/OR decomposition. GCD, counts, overflow and ITE are already implemented and not missing features.

## Boundaries

- Keep this a two-case investigation with explicit independent conclusions; do not add a complete table engine or instruction set.
- Use public tiny lookup data and exact modulo-width rotation semantics; no large private corpus.
- Any production follow-up must be justified by model usefulness and measured decomposition cost.

## Done when

- [ ] Each case has a precise domain/API proposal, current formulation and semantic oracle.
- [ ] Measure propagation/search/construction costs against current decomposition at useful small scales.
- [ ] Record separate implement/defer decisions with concrete evidence and bounded follow-up scope.

## Validation

- Run exact small input/amount checks and capped Release comparisons; no permanent tests for discarded prototypes.
