+++
schema_version = 1
id = "word-008"
key = "ordering"
area = "word"
status = "done"
blocked_by = ["word-004"]
+++
# Add signed and unsigned word ordering

## Outcome

Direct posting supports signed and unsigned less-than, less-or-equal, greater-than, and greater-or-equal relations with constants and reification.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Rewrite dual relations instead of adding redundant actors when the rewrite is equally strong.

## Done when

- [x] Unsigned and two's-complement signed comparisons implement the settled fixed-width semantics and width checks.
- [x] Constants, aliases, all applicable reification modes, failure, subsumption, cloning, and recomputation are covered.
- [x] Each relation records its direct/rewrite/decomposition shape and claimed propagation property.

## Validation

- Run exhaustive assigned and partial-domain oracle tests at small widths.
- Run two's-complement boundary, reification, alias, clone, and recomputation tests.

## Result

Added signed and unsigned WordVar ordering relations for <, <=, >, and >= with explicit-width constants and all reification modes.

Validation:

- Independent verification passed signed-key and Wang-style pruning review; focused Word::Rel and Word::TestFramework tests passed in the implementation build across variables, constants, all reification modes, partial cubes, aliases, signed boundaries, cloning, and git diff checks.
