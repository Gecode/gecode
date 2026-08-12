+++
schema_version = 1
id = "gcd-004"
key = "nary-product-mod"
area = "gcd"
status = "done"
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

- [x] Documented public ordinary and Reify product_mod posting APIs accept IntVarArgs factors, a fixed int modulus, an IntVar result, and IntPropLevel and are available from gecode/int.hh.
- [x] Posting rejects a nonpositive modulus through the repository's established integer-argument exception convention and constrains every result to the canonical range zero through modulus minus one.
- [x] Dedicated n-ary actors implement overflow-safe modular arithmetic, sound posting, propagation, cloning, disposal, subscription, rescheduling, rewriting or subsumption as appropriate, and RM_EQV, RM_IMP, and RM_PMI control.
- [x] The relation uses SMT-LIB-compatible Euclidean residues, defines the empty product as one modulo the modulus, and handles modulus one, singleton, repeated, zero, signed, sparse, assigned, and aliased variables.
- [x] Focused arithmetic tests cover ordinary and reified semantics, all reification modes, fixed and unfixed controls, modulus one, canonical residues for negative factors, empty and singleton arrays, zeros, sparse domains, repetition, result aliasing, and cloning without new test infrastructure.
- [x] The affected integer library and focused tests build successfully.

## Validation

- Build the configured integer library and test executable.
- Run the focused ordinary and reified Int::Arithmetic::ProductMod tests.
- Run the broader integer test target when available in the existing local build.

## Result

Implemented and independently verified ordinary and reified fixed-modulus n-ary product constraints with canonical Euclidean residues, positive-modulus validation, overflow-safe modular arithmetic, alias-aware support filtering, and correct inactive implication behavior.

Validation:

- Built the integer library and gecode-test successfully.
- All 19 focused Int::Arithmetic::ProductMod cases passed.
- All 473 broader Int::Arithmetic tests passed.
- Fresh install, exported-symbol/header assertions, and targeted implication/range/modulus-one/max-modulus consumer smokes passed.
- git diff --check and zd check passed.
