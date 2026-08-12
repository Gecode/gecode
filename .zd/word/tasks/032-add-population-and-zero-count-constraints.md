+++
schema_version = 1
id = "word-032"
key = "word-bit-counts"
area = "word"
status = "open"
blocked_by = []
+++
# Add population and zero-count constraints

## Outcome

WordVar values can be related directly to bounded IntVar population count, leading-zero count, and trailing-zero count results.

## Boundaries

- The result range is 0 through the Word width, so this task does not introduce general Word-to-Int conversion.
- Define count_leading_zeros and count_trailing_zeros of zero as the Word width, giving total CP semantics analogous to the defined LLVM form.
- Use mixed Word/Int propagators with honest bounds/prefix propagation; do not channel all bits or claim unavailable cardinality correlations in the cube domain.
- Keep tests proportionate and use existing IntVar and test/word machinery.
- Add normal Gecode-style tests for each count propagator through the shared `test/word` infrastructure established by word-002 and register them in the ordinary `gecode-test` inventories.

## Done when

- [ ] Direct APIs relate one WordVar to an IntVar result for popcount, count_leading_zeros, and count_trailing_zeros with result-domain validation and widths 1 through 64.
- [ ] Popcount propagates known/possible-one bounds and fixes all remaining bits at the extrema; leading/trailing-zero actors propagate assigned values and useful prefix information in both directions.
- [ ] MiniModel exposes Int-expression forms only if they can be implemented through the normal LinIntExpr/IntVar lowering pattern without new expression infrastructure.
- [ ] Focused tests cover assigned small widths, representative partial bounds/prefix cases, zero input, width 1 and 64, failure, cloning, recomputation, and subsumption.

## Validation

- Build Word, Int, optional MiniModel changes, and the existing gecode-test target.
- Run focused Word bit-count and Word::TestFramework tests plus a representative Int count regression if shared machinery changes.
- Run git diff --check and inspect that no unrestricted Word-to-Int API was introduced.
