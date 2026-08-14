+++
schema_version = 1
id = "word-075"
key = "bounded-division"
area = "word"
status = "done"
blocked_by = ["word-074"]
+++
# Migrate unsigned and signed Word division families to bounds

## Outcome

Unsigned div/mod/divmod and signed div/rem/mod use transactional bounded actors with exact SMT-LIB exceptional rows and cube fallback.

## Context

Implement stage five after core arithmetic and relations establish the typed-view patterns.

## Boundaries

- Preserve zero-divisor, signed min/-1, alias, and modular semantics exactly.
- Publish every distinct variable once per propagation.

## Done when

- [x] Unsigned and signed division families dispatch only on matching homogeneous bounded variables and otherwise retain cube actors.
- [x] Assigned, inverse, power-of-two, zero-divisor, signed-overflow, alias, width 1/64, failure, subsumption, clone, and replay cases pass.
- [x] Division pipelines show measured benefit without material cube or unrelated-workload regression.

## Validation

- Run full DivisionLifecycle, signed division, MiniModel arithmetic, and TestFramework suites.
- Run exhaustive small-width truth parity and bounded division pipeline benchmarks.
- Run zd check word and git diff --check.

## Result

Added transactional bounded actors for unsigned div/mod/divmod and signed div/rem/mod, with exact exceptional rows, self-divisor identities, alias grouping, and cube fallback for ambiguous or mixed domains.

Validation:

- Full Arithmetic, MiniModel ArithmeticLifecycle, TestFramework, exhaustive widths 1-4 truth and width 1/64 exceptional cases, aliases, clone/replay/subsumption, warning build, six exact pipeline benchmarks, independent verification, zd check word, and diff checks pass.
