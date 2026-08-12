+++
schema_version = 1
id = "word-043"
key = "add-reified-mixed-product-mod"
area = "word"
status = "open"
blocked_by = ["word-042"]
+++
# Add reified mixed Word product modulo an IntVar

## Outcome

The mixed Word/Int modular-product relation can be posted through the standard Reify interface in EQV, IMP, and PMI modes.

## Boundaries

- Add exactly the Reify overload of the direct product_mod API; reuse or rewrite to the normal actor when the control decides.
- Preserve the direct task's positive-modulus, equal-width, and overflow-safe semantics; no additional overloads, policies, or MiniModel syntax.
- Use existing BoolView/Reify and test/word patterns without a new harness or broad matrix.

## Done when

- [ ] RM_EQV, RM_IMP, and RM_PMI implement the correct product-mod truth relation, including all assigned allowed and forbidden rows.
- [ ] A decided control rewrites or subsumes conventionally, and aliases, cloning, recomputation, failure, and actor disposal remain correct.
- [ ] Normal registered test/word coverage proportionately exercises all modes, both Boolean values, partial control propagation, aliases, lifecycle, and the direct actor regression.

## Validation

- Build the focused Word library and monolithic gecode-test target.
- Run registered reified ProductMod, direct ProductMod, Word Arithmetic, and Word TestFramework filters.
- Run git diff --check and inspect standard Reify lifecycle/rewrite behavior.
