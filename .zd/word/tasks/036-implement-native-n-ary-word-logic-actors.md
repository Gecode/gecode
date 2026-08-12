+++
schema_version = 1
id = "word-036"
key = "native-nary-logic"
area = "word"
status = "open"
blocked_by = []
+++
# Implement native n-ary Word logic actors

## Outcome

Existing n-ary AND, OR, and XOR postings can use compact direct global actors instead of folds through temporary WordVars when focused measurements justify them.

## Boundaries

- This is an optimization of the existing WordVarArgs API, not a new modeling operation.
- Implement primitive AND, OR, and XOR globals; NAND, NOR, and XNOR may remain complement rewrites when equally strong.
- Handle empty identities, singleton arrays, duplicates, aliases, and assigned operands through standard Gecode normalization and lifecycle patterns.
- Do not expose the internal truth-table representation or add a generic n-ary table API.
- Add normal Gecode-style tests for each retained n-ary primitive actor through the shared `test/word` infrastructure established by word-002; temporary benchmarks do not replace registered `gecode-test` coverage.

## Done when

- [ ] Direct n-ary AND, OR, and XOR actors propagate all operands and result soundly without model-level fold temporaries and record their consistency contract.
- [ ] The existing six n-ary WordOpType postings preserve their public semantics and route through primitive actors plus conventional complement rewrites.
- [ ] Focused tests cover identities, duplicates/aliases, assigned and representative partial propagation, failure, cloning, recomputation, and subsumption without an arity matrix campaign.
- [ ] A temporary Release comparison across a few arities and widths records actors, propagation calls, runtime, RSS, and solution parity and justifies each retained global.

## Validation

- Run focused Word::Logic, Word::Conditional regression, MiniModel logic, and Word::TestFramework tests in Release mode.
- Inspect temporary fold-versus-global benchmark medians and parity for representative arities.
- Run git diff --check and confirm binary Logic::Table semantics are unchanged.
