+++
schema_version = 1
id = "word-060"
key = "self-modulus-identities"
area = "word"
status = "done"
blocked_by = []
+++
# Normalize Word self-modulus identities

## Outcome

Unsigned mod, signed remainder/modulus, and combined divmod force the remainder of x by x to zero at posting while preserving quotient semantics.

## Context

The selected SMT-LIB semantics make these remainder identities exact even for x=0, but current generic actors retain weakly constrained results until x is assigned. Add conventional posting rewrites in the existing arithmetic actors.

## Boundaries

- Do not change public arithmetic APIs or the selected zero-divisor semantics.
- Preserve the quotient actor where combined divmod still needs it.
- Use ordinary test/word arithmetic lifecycle tests.

## Done when

- [x] Unsigned mod(x,x), signed_rem(x,x), and signed_mod(x,x) immediately force zero.
- [x] Combined divmod(x,x,q,r) immediately forces r=0 while q remains correctly constrained.
- [x] Tests cover unassigned aliases, result aliases, widths 1 and 64, failure, cloning, recomputation, and subsumption.

## Validation

- Build the normal Release Word library and gecode-test target.
- Run Word::Arithmetic, Word::MiniModel::ArithmeticLifecycle, and Word::TestFramework focused tests.
- Run git diff --check.

## Result

Normalized unsigned and signed self-modulus identities and preserved exact combined self-divmod quotient semantics.

Validation:

- Release gecodeword_shared and gecode-test built.
- Full Word::Arithmetic, DivisionLifecycle, SignedDivisionLifecycle, MiniModel::ArithmeticLifecycle, and Word::TestFramework passed.
- Independent Spec and Standards verification passed, including aliases, widths 1/64, clone, c_d=1 replay, and subsumption.
- git diff --check passed.
