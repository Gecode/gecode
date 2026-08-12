+++
schema_version = 1
id = "word-033"
key = "native-unsigned-divrem"
area = "word"
status = "done"
blocked_by = []
+++
# Implement native unsigned division and remainder propagators

## Outcome

The existing unsigned div and mod APIs can use compact native word actors instead of the width-step shift/subtract graph when focused measurements justify the replacement.

## Boundaries

- Preserve WS_SMTLIB zero-divisor results and every existing direct and MiniModel API.
- State a realistic cube-domain propagation contract; exact assigned semantics and sound useful bidirectional pruning are required, full domain consistency is not.
- Do not encapsulate the existing decomposition inside one actor, add public switches, or commit benchmark infrastructure.
- A shared internal quotient/remainder kernel is allowed, but public calls must retain ordinary actor lifecycle and result ownership.
- Add normal Gecode-style tests for the native quotient and remainder paths through the shared `test/word` infrastructure established by word-002; temporary benchmarks do not replace registered `gecode-test` coverage.

## Done when

- [x] Native unsigned quotient and remainder actors are exact on assigned inputs, sound on partial cubes, handle zero divisors and aliases, and use honest cost, cloning, scheduling, failure, and subsumption behavior.
- [x] The existing div and mod postings route to the retained native actors without width-proportional model-level intermediate variables.
- [x] Focused tests preserve existing assigned/zero/constant/alias semantics and add only representative partial/inverse cases needed for the declared actor contract and lifecycle.
- [x] A temporary Release baseline comparison reports actors, propagation calls, runtime, RSS, nodes, failures, and solution parity; the native path is retained only with a useful result and no material propagation regression.

## Validation

- Run focused Word unsigned Div, Mod, arithmetic lifecycle, MiniModel parity, and Word::TestFramework tests in Release mode.
- Inspect temporary assigned and inverse/partial benchmark medians and semantic parity.
- Run git diff --check and confirm signed postings still have coherent behavior before their dedicated task.

## Result

Replaced unsigned division and remainder model graphs with compact native actors, preserving SMT-LIB semantics while documenting and benchmarking the speed-versus-propagation-strength tradeoff.

Validation:

- Fresh Release focused unsigned/signed arithmetic, MiniModel, and Word framework tests; independent lifecycle/source review; exact baseline 3dc82d8311 five-trial benchmark with solution/checksum parity, runtime/RSS/actor/propagation/node/failure metrics; zd check and git diff --check.
