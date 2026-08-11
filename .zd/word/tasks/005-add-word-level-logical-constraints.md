+++
schema_version = 1
id = "word-005"
key = "logic"
area = "word"
status = "open"
blocked_by = ["word-002"]
+++
# Add word-level logical constraints

## Outcome

Direct posting supports complement, conjunction, disjunction, exclusive-or, nand, nor, and xnor/equivalence, including useful n-ary forms.

## Boundaries

- Derived operators may rewrite to an equally strong primitive set.
- Do not expand word arguments into per-bit Boolean variables.

## Done when

- [ ] Logical propagators implement validated lo/hi rules for variables, constants, aliases, and useful n-ary identities.
- [ ] Failure, fixpoint, subsumption, cloning, recomputation, and width errors follow ordinary Gecode actor contracts.
- [ ] Tests state and verify the claimed propagation property for every public logical operation.

## Validation

- Run assigned-value oracle and exhaustive small-width partial-domain tests through the shared WordVar framework.
- Run focused alias, n-ary identity, failure, subsumption, clone, and recomputation tests.
- Check that direct word propagation uses bounded native-word work where practical.
