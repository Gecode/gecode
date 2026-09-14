+++
schema_version = 1
id = "word-005"
key = "logic"
area = "word"
status = "done"
blocked_by = ["word-002"]
+++
# Add word-level logical constraints

## Outcome

Direct posting supports complement, conjunction, disjunction, exclusive-or, nand, nor, and xnor/equivalence, including useful n-ary forms.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Derived operators may rewrite to an equally strong primitive set.
- Do not expand word arguments into per-bit Boolean variables.

## Done when

- [x] Logical propagators implement validated lo/hi rules for variables, constants, aliases, and useful n-ary identities.
- [x] Failure, fixpoint, subsumption, cloning, recomputation, and width errors follow ordinary Gecode actor contracts.
- [x] Tests state and verify the claimed propagation property for every public logical operation.

## Validation

- Run assigned-value oracle and exhaustive small-width partial-domain tests through the shared WordVar framework.
- Run focused alias, n-ary identity, failure, subsumption, clone, and recomputation tests.
- Check that direct word propagation uses bounded native-word work where practical.

## Result

Added word-level complement and binary/n-ary AND, OR, XOR, NAND, NOR, and XNOR with explicit-width constants and bounded native-word truth-table propagation.

Validation:

- Independent verification passed focused Word::Logic and Word::TestFramework tests, actor and mask-semantics review, alias/constant/n-ary identity checks, build inventory review, and git diff checks.
