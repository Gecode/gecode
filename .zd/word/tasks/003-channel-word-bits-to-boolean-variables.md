+++
schema_version = 1
id = "word-003"
key = "channel"
area = "word"
status = "done"
blocked_by = ["word-002"]
+++
# Channel word bits to Boolean variables

## Outcome

Models can connect a selected bit of a WordVar to a BoolVar, including already fixed bits and Boolean constants, without exposing the entire word as Boolean variables.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not add eager whole-word bit blasting or a general word/Boolean expression layer.

## Done when

- [x] The direct posting API channels one in-range word bit to a Boolean variable or constant and rejects invalid positions.
- [x] The propagator handles aliases, fixed arguments, failure, subsumption, cloning, and recomputation.
- [x] Tests use the shared WordVar testing framework and establish word-versus-Boolean solution parity for small widths.

## Validation

- Run exhaustive small-width channel tests through the WordVar testing framework.
- Run focused clone, recomputation, alias, and out-of-range tests.

## Result

Added Gecode-style single-bit channeling between WordVar and BoolVar or 0/1 constants using a standard mixed binary propagator, with Word-to-Int component dependency and normal documentation/build integration.

Validation:

- Independent verification passed; all focused Word channel tests, Word test-framework smoke, a threaded repeat, generated-source freshness, build dependency inspection, and git diff checks passed.
