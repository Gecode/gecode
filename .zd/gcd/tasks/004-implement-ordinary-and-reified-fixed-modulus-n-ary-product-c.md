+++
schema_version = 1
id = "gcd-004"
key = "nary-product-mod"
area = "gcd"
status = "open"
blocked_by = []
+++
# Implement ordinary and reified fixed-modulus n-ary product constraints

## Outcome

Users can constrain an integer result to the canonical Euclidean residue of an IntVarArgs product under a fixed positive modulus, ordinarily or through any Gecode reification mode.

## Boundaries

- Accept a fixed positive int modulus only; do not add an IntVar modulus overload.
- Keep this slice within the integer arithmetic API and propagator implementation; do not add MiniModel or FlatZinc support.
- Follow the brief's focused testing level and existing arithmetic and reification test patterns.

## Done when

- [ ] Documented public ordinary and Reify product_mod posting APIs accept IntVarArgs factors, a fixed int modulus, an IntVar result, and IntPropLevel and are available from gecode/int.hh.
- [ ] Posting rejects a nonpositive modulus through the repository's established integer-argument exception convention and constrains every result to the canonical range zero through modulus minus one.
- [ ] Dedicated n-ary actors implement overflow-safe modular arithmetic, sound posting, propagation, cloning, disposal, subscription, rescheduling, rewriting or subsumption as appropriate, and RM_EQV, RM_IMP, and RM_PMI control.
- [ ] The relation uses SMT-LIB-compatible Euclidean residues, defines the empty product as one modulo the modulus, and handles modulus one, singleton, repeated, zero, signed, sparse, assigned, and aliased variables.
- [ ] Focused arithmetic tests cover ordinary and reified semantics, all reification modes, fixed and unfixed controls, modulus one, canonical residues for negative factors, empty and singleton arrays, zeros, sparse domains, repetition, result aliasing, and cloning without new test infrastructure.
- [ ] The affected integer library and focused tests build successfully.

## Validation

- Build the configured integer library and test executable.
- Run the focused ordinary and reified Int::Arithmetic::ProductMod tests.
- Run the broader integer test target when available in the existing local build.
