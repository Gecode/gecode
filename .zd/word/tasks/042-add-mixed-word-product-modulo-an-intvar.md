+++
schema_version = 1
id = "word-042"
key = "add-mixed-product-mod"
area = "word"
status = "open"
blocked_by = ["word-041"]
+++
# Add mixed Word product modulo an IntVar

## Outcome

Models can constrain a same-width Word result to the overflow-safe mathematical product of two Word operands modulo a positive IntVar modulus.

## Boundaries

- Add exactly the direct variable API `product_mod(Home, WordVar, WordVar, IntVar, WordVar)`; no constant overload matrix or MiniModel node.
- The modulus is positive and bounded by the existing IntVar limits; this is not a general Word-to-Int conversion and does not use SMT-LIB zero-divisor semantics.
- Follow ordinary mixed-domain Gecode actor and test/word patterns; do not add a bespoke executable or testing framework.

## Done when

- [ ] The public API validates equal Word widths, constrains the modulus to positive values, and computes assigned products modulo the IntVar without host overflow.
- [ ] The mixed actor provides sound useful propagation in all directions, handles aliases, clones, recomputation, failure, and subsumption through standard lifecycle patterns.
- [ ] Normal registered test/word coverage proportionately exercises assigned semantics including overflowing 64-bit products, representative partial propagation, modulus and width contracts, aliases, lifecycle, and search.

## Validation

- Build the focused Word library and monolithic gecode-test target.
- Run the registered ProductMod, Word Arithmetic regression, representative Int arithmetic smoke, and Word TestFramework filters.
- Run git diff --check and focused Release profiling sufficient to confirm the actor does not hide a width-proportional decomposition.
