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

- Rewrite dual relations instead of adding redundant actors when the rewrite is equally strong.

## Done when

- [ ] Unsigned and two's-complement signed comparisons implement the settled fixed-width semantics and width checks.
- [ ] Constants, aliases, all applicable reification modes, failure, subsumption, cloning, and recomputation are covered.
- [ ] Each relation records its direct/rewrite/decomposition shape and claimed propagation property.

## Validation

- Run exhaustive assigned and partial-domain oracle tests at small widths.
- Run two's-complement boundary, reification, alias, clone, and recomputation tests.
