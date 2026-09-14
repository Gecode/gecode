+++
schema_version = 1
id = "word-020"
key = "examples-docs"
area = "word"
status = "done"
blocked_by = ["word-007", "word-018", "word-019"]
+++
# Document and demonstrate word-vector modeling

## Outcome

Users have Doxygen and API documentation plus several redistributable direct-posting and MiniModel examples that are clearer than equivalent manual Boolean-array models.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Include only small, reviewable examples with redistribution-compatible provenance.
- Do not include private benchmark payloads.

## Done when

- [x] Doxygen documents WordVar, posting functions, operation and relation types, WordSemantics, branching, tracing, and MiniModel expressions.
- [x] Several direct-posting and MiniModel examples build through the normal example machinery and check expected solutions.
- [x] The documentation lists each public operation, its direct/rewrite/decomposition shape, and its tested propagation property.
- [x] Public and internal multiword names follow snake_case and types follow CamelCase.

## Validation

- Build and run all word examples with expected-solution checks.
- Build Doxygen and run documentation link checks.
- Audit the public API and propagation-property inventory against the area brief.

## Result

Documented the Word API and implementation properties and added three small direct and MiniModel examples through the normal example machinery.

Validation:

- All three focused examples build and produce their unique expected solutions; documentation statistics generation and diff checks pass; independent verifier PASS.
