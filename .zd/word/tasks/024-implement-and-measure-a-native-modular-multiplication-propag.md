+++
schema_version = 1
id = "word-024"
key = "native-mult"
area = "word"
status = "done"
blocked_by = ["word-023"]
+++
# Implement and measure a native modular multiplication propagator

## Outcome

Modular multiplication uses one compact native Gecode actor with useful bidirectional cube propagation instead of the schoolbook model-level actor network.

## Boundaries

- Keep the public mult overloads, aliases, constants, MiniModel lowering, and modulo semantics unchanged.
- Do not retain a hidden schoolbook decomposition inside the actor or expose an experimental propagation selector in public API.
- Document the actor's propagation property honestly; exact assigned semantics and sound narrowing are required, but the brief does not require exhaustive bit consistency for multiplication.
- Follow the existing arithmetic test and benchmark machinery without an exhaustive partial-cube campaign or a new harness.

## Done when

- [x] A dedicated ternary multiplication actor is exact for assigned operands/results, detects incompatible fixed information, and performs documented useful forward and backward cube narrowing.
- [x] The ordinary multiplication posting path uses the actor with conventional alias, copy, scheduling, cost, and subsumption behavior.
- [x] Focused tests cover assigned small-width semantics, representative partial and inverse cases, constants, aliases, failure, clone/recomputation, and solution parity with the Boolean reference.
- [x] A focused Release width matrix records the crossover, actor/propagation reduction, runtime, peak RSS, and any search-strength change relative to both the previous word decomposition and Boolean schoolbook model.

## Validation

- Build the Word library and existing gecode-test target in Release mode.
- Run focused Word multiplication tests and Word TestFramework smoke test.
- Run assigned and partial/inverse multiplication benchmark slices and verify result/solution parity.
- Run git diff --check.

## Result

Replaced schoolbook modular multiplication with one native low-prefix and modular-inverse propagator.

Validation:

- Release Mult, MultLifecycle, and TestFramework selections passed.
- Focused tests passed assigned widths 1-4, partial prefix/inverse rules, constants, aliases, failure, clone/recomputation, and Boolean parity.
- Corrected three-way benchmark against exact parent cf5af7f150 preserved all solution/node/failure counts and showed lower runtime, actors, propagation calls, and memory.
- Independent Spec and Standards verification passed; git diff --check passed.
