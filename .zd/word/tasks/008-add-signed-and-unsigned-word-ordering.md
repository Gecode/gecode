+++
schema_version = 1
id = "word-008"
key = "ordering"
area = "word"
status = "open"
blocked_by = ["word-004"]
+++
# Add signed and unsigned word ordering

## Outcome

Direct posting supports signed and unsigned less-than, less-or-equal, greater-than, and greater-or-equal relations with constants and reification.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Rewrite dual relations instead of adding redundant actors when the rewrite is equally strong.

## Done when

- [ ] Unsigned and two's-complement signed comparisons implement the settled fixed-width semantics and width checks.
- [ ] Constants, aliases, all applicable reification modes, failure, subsumption, cloning, and recomputation are covered.
- [ ] Each relation records its direct/rewrite/decomposition shape and claimed propagation property.

## Validation

- Run exhaustive assigned and partial-domain oracle tests at small widths.
- Run two's-complement boundary, reification, alias, clone, and recomputation tests.
