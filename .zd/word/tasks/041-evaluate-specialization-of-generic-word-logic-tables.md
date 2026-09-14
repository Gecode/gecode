+++
schema_version = 1
id = "word-041"
key = "evaluate-logic-table-specialization"
area = "word"
status = "done"
blocked_by = ["word-040"]
+++
# Evaluate specialization of generic word logic tables

## Outcome

Generic arity-two through arity-four Logic::Table propagation uses the smallest justified tuple-support computation while preserving exact bit consistency.

## Boundaries

- Keep the public/internal posting interfaces, allowed truth tables, aliases, actor count, and publish-once lifecycle unchanged.
- Do not expose truth tables publicly, introduce generated code, advisors, durable benchmark infrastructure, or unrelated logic rewrites.
- A clean measured rejection with no production change is an acceptable result if specialization is not worthwhile.

## Done when

- [x] A bounded arity-specialized or precomputed support implementation is compared with the current generic tuple loop, and is retained only when the complexity has a useful repeatable payoff.
- [x] Existing binary Logic, Conditional including arity-four mask ITE, MiniModel logic, aliases, cloning, recomputation, subsumption, and TestFramework coverage passes unchanged.
- [x] Temporary Release benchmarks and profiles cover arities 2, 3, and 4, representative widths and event depths, with exact propagation/search parity.

## Validation

- Build the focused Release Word library and gecode-test target when a production change is retained.
- Run registered Word Logic, Conditional, MiniModel logic, and TestFramework filters.
- Run a temporary exact-baseline Release benchmark/profile and git diff --check.

## Result

Specialized the unchanged exact Logic::Table tuple-support loop for arities one through four, preserving publish-once bit consistency.

Validation:

- Focused Release Logic, Conditional, MiniModel logic, and TestFramework tests passed.
- Exact arity 2/3/4 benchmarks preserved all semantic and search counters and improved event-heavy workloads by 14 to 40 percent.
- Independent verification and git diff --check passed.
