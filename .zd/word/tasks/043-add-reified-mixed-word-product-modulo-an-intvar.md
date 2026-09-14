+++
schema_version = 1
id = "word-043"
key = "add-reified-mixed-product-mod"
area = "word"
status = "done"
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

- [x] RM_EQV, RM_IMP, and RM_PMI implement the correct product-mod truth relation, including all assigned allowed and forbidden rows.
- [x] A decided control rewrites or subsumes conventionally, and aliases, cloning, recomputation, failure, and actor disposal remain correct.
- [x] Normal registered test/word coverage proportionately exercises all modes, both Boolean values, partial control propagation, aliases, lifecycle, and the direct actor regression.

## Validation

- Build the focused Word library and monolithic gecode-test target.
- Run registered reified ProductMod, direct ProductMod, Word Arithmetic, and Word TestFramework filters.
- Run git diff --check and inspect standard Reify lifecycle/rewrite behavior.

## Result

Implemented and independently verified standard EQV, IMP, and PMI reification for mixed Word/Int product_mod.

Validation:

- Release ProductMod, full Word Arithmetic, and Word TestFramework filters passed.
- All truth rows, decided rewrites, aliases, cloning, and c_d=8/a_d=64 recomputation/control inference passed, including seed 48271.
- zdev checks and git diff --check passed.
