+++
schema_version = 1
id = "word-035"
key = "native-bool-ite"
area = "word"
status = "open"
blocked_by = []
+++
# Implement a direct Boolean-controlled Word ITE actor

## Outcome

Boolean-controlled word ITE posts one conventional mixed actor rather than channeling the control through a one-bit WordVar and sign extension when focused measurements justify it.

## Boundaries

- Preserve the existing whole-word Bool control semantics, overloads, aliases, constants, and MiniModel API.
- Do not change the separate per-bit Word-mask ITE or generalize Logic::Table into unrelated mixed-arity infrastructure unless the ordinary actor pattern requires a small reusable seam.
- State the actor's propagation contract and use ordinary BoolView/WordView lifecycle patterns.
- Benchmark with temporary Release artifacts only.
- Add normal Gecode-style tests for the direct actor through the shared `test/word` infrastructure established by word-002; temporary benchmarks do not replace registered `gecode-test` coverage.

## Done when

- [ ] One direct Bool/Word actor propagates among control, both branches, and result, including equal branches and aliases, without model-level control WordVars.
- [ ] All existing direct and MiniModel Bool ITE postings route through the native actor while Word-mask ITE remains unchanged.
- [ ] Focused existing-framework tests cover assigned and unknown controls, representative partial cubes, constants, aliases/failure, width 1 and 64, cloning, recomputation, and subsumption.
- [ ] A temporary Release comparison against the channel/sign-extend decomposition reports actor, propagation, runtime, RSS, and solution parity and supports retaining the actor.

## Validation

- Run focused Word::Conditional, Word MiniModel, Word::Logic regression, and Word::TestFramework tests in Release mode.
- Inspect temporary forward and search benchmark medians and parity.
- Run git diff --check and confirm the Word-mask ITE implementation is unchanged.
