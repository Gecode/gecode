+++
schema_version = 1
id = "word-037"
key = "native-nary-add"
area = "word"
status = "done"
blocked_by = []
+++
# Add native n-ary modular addition

## Outcome

Models can post a WordVarArgs modular sum through one native n-ary carry actor, avoiding chains of temporary binary sums.

## Boundaries

- Add one Gecode-style n-ary add posting with zero as the empty identity and equality as the singleton case; constants compose through existing explicit-width WordVars rather than an overload matrix.
- The actor must maintain bounded word-level carry state and an honest width-and-arity cost; do not hide a binary Add decomposition inside it.
- State a realistic propagation contract and avoid claiming full domain consistency across arbitrary arity cubes.
- Do not add weighted linear arithmetic, coefficients, dot products, or a general modular-linear subsystem.
- Add normal Gecode-style tests for the n-ary actor through the shared `test/word` infrastructure established by word-002; temporary benchmarks do not replace registered `gecode-test` coverage.

## Done when

- [x] The public WordVarArgs add API implements modular sum for empty, singleton, and multi-operand arrays with uniform width checking and ordinary alias normalization.
- [x] One native n-ary actor is exact on assigned operands, performs sound useful bidirectional cube narrowing, and follows conventional posting, copying, scheduling, cost, failure, and subsumption patterns.
- [x] Focused shared-framework tests cover assigned small-width sums, representative partial carry propagation, identities, duplicates/aliases, width 64 wrap, failure, cloning, recomputation, and subsumption.
- [x] A temporary Release comparison against a binary-Add fold reports actor count, propagation calls, runtime, RSS, nodes/failures, and solution parity for representative arities; retain the actor only with a useful result.

## Validation

- Run focused Word n-ary Add, existing binary arithmetic regression, optional MiniModel parity if expression support is added, and Word::TestFramework tests in Release mode.
- Inspect temporary fold-versus-native benchmark medians and semantic/search parity.
- Run git diff --check and confirm no general linear-arithmetic API was introduced.

## Result

Added one native WordVarArgs modular-add actor with bounded carry propagation, preserving modular identities and aliases without introducing a general linear subsystem.

Validation:

- Fresh Release full Word arithmetic and framework tests; independent carry/lifecycle/source verification including exhaustive width-1 partial cubes; exact baseline 45a3b672f3 arity-4/8 benchmark with semantic/search parity and documented assigned/partial gains plus tiny-search slowdown; zd check and git diff --check.
