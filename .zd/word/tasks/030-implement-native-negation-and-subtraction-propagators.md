+++
schema_version = 1
id = "word-030"
key = "native-neg-sub"
area = "word"
status = "done"
blocked_by = []
+++
# Implement native negation and subtraction propagators

## Outcome

The existing modular neg and sub APIs post compact native actors with no model-level complement, constant-one, or intermediate addition variables when focused measurements justify them.

## Boundaries

- Preserve every existing public and MiniModel semantic contract; this is an actor replacement, not a new operation.
- Use the same bounded carry/borrow automaton style as native Add where suitable and state whether each actor achieves bit consistency.
- Do not hide the old decomposition inside an actor or add public implementation switches.
- Benchmark only with temporary Release drivers and raw results outside the repository.
- Add normal Gecode-style tests for each replacement actor through the shared `test/word` infrastructure established by word-002; benchmarks do not replace registered `gecode-test` coverage.

## Done when

- [x] Native Neg and Sub actors perform sound bidirectional cube narrowing for widths 1 through 64, handle aliases directly, and follow ordinary posting, cloning, cost, scheduling, failure, and subsumption patterns.
- [x] Existing neg and sub posting functions route through the native actors without model-level intermediate WordVars.
- [x] Focused existing-framework tests establish assigned modular semantics, the declared small-width partial propagation contract, representative aliases/failure, width 64, cloning, recomputation, and subsumption.
- [x] A temporary baseline-versus-native Release comparison records actor count, propagation calls, runtime, RSS, and solution/search parity for assigned and partial/inverse cases; production keeps each actor only if the result is useful without a material propagation regression.

## Validation

- Run focused Word arithmetic Neg, Sub, lifecycle, and Word::TestFramework tests in Release mode.
- Inspect temporary benchmark medians and parity without committing scripts or raw results.
- Run git diff --check and confirm the existing add/mult APIs and actors are unchanged.

## Result

Replaced modular negation and subtraction decompositions with native bit-consistent Word actors using bounded two-state carry and borrow automata. Existing APIs now post one actor without model-level intermediates; aliases, widths 1 through 64, cloning, recomputation, and subsumption follow standard Gecode patterns. Added proportionate registered arithmetic tests.

Validation:

- Independent verification PASS. Fresh Release focused Neg, Sub, Arithmetic Lifecycle, and Word::TestFramework tests passed. Exhaustive width-two support-hull checks support the bit-consistency claim. Temporary seven-process Release comparison against exact baseline cfa2079f9f showed 1.50-1.79x Neg and 1.74-1.91x Sub speedups, 2-3x actor/propagation reductions, identical nodes/failures/solutions, and flat RSS. No benchmark artifacts were tracked; git diff --check passed.
