+++
schema_version = 1
id = "word-023"
key = "native-add"
area = "word"
status = "done"
blocked_by = []
+++
# Implement a native modular addition propagator

## Outcome

Modular word addition posts one compact native Gecode actor that propagates the carry relation directly and replaces the model-level carry decomposition.

## Boundaries

- Keep the public add, neg, sub, constant, MiniModel, and SMT-LIB semantics unchanged.
- Use ordinary Word views and actor lifecycle patterns; do not hide the old decomposition or model-level intermediate variables inside the actor.
- Achieve the brief's bit-consistency contract with a bounded width-at-most-64 carry algorithm and declare an honest propagation cost.
- Follow the brief's focused testing level and existing test/word arithmetic patterns; add no new test harness.

## Done when

- [x] A dedicated native addition actor performs sound modulo-2^width propagation in every operand direction and subsumes when the relation is decided.
- [x] The ordinary add posting path uses the actor, and negation and subtraction continue to lower through addition without public API changes.
- [x] Focused assigned and small-width partial-domain tests establish concrete semantics and the declared bit-consistency property, with normal clone, recomputation, failure, and subsumption coverage.
- [x] A focused Release comparison against commit 624cae2a20b4 records actor count, propagation calls, runtime, memory, and search behavior for forward and partial/inverse addition cases.

## Validation

- Build the Word library and existing gecode-test target in Release mode.
- Run the focused Word arithmetic addition tests and Word TestFramework smoke test.
- Run the focused addition benchmark against the recorded decomposed baseline and inspect semantic parity.
- Run git diff --check.

## Result

Replaced decomposed modular addition with one bit-consistent native carry-automaton propagator.

Validation:

- Release Word Add, Lifecycle, and TestFramework selections passed.
- Exhaustive width-two support-hull comparison and independent Boolean parity passed.
- Baseline/current forward and inverse benchmarks preserved semantic/search parity and showed large runtime, actor, and propagation reductions.
- Independent Spec and Standards verification passed; git diff --check passed.
