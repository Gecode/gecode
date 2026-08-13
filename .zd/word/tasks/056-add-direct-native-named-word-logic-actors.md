+++
schema_version = 1
id = "word-056"
key = "add-direct-native-named-word-logic"
area = "word"
status = "done"
blocked_by = ["word-055"]
+++
# Add direct native named Word logic actors

## Outcome

Public named binary AND, OR, and XOR operations use ordinary native bit-parallel propagators instead of generic tuple enumeration where their exact cube support is directly representable, while preserving Table as the generic and alias fallback.

## Boundaries

- Keep the existing public API, truth-table implementation, n-ary globals, MiniModel lowering, events, and standard actor lifecycle; do not expose a truth-table API or add advisors.
- Handle aliases algebraically only where exact and simple; otherwise use the existing Table path rather than weakening semantics or adding special state.
- Retain AND, OR, and XOR independently only when each has exact semantic/search parity and a useful repeatable gain on relevant realistic and general logic workloads.

## Done when

- [x] Named AND, OR, and XOR posting selects direct conventional actors for supported shapes and preserves generic Table for unsupported alias or nonuniform truth-table cases.
- [x] Normal registered Word Logic tests proportionately cover assigned and partial support, aliases/fallback, failure, clone, recomputation, and subsumption, with Conditional and MiniModel regressions green.
- [x] Exact-baseline Release benchmarks record actors, propagations, nodes, failures, solutions/checksums, runtime, and best-effort memory for both CRCs, the symbolic ALU, and a general logic workload; only measured useful operations are retained.
- [x] No benchmark, profiling hook, special scheduler, advisor machinery, or unrelated Table change is tracked.

## Validation

- Build the Word library and monolithic test target in Release and run focused Word Logic, Conditional, MiniModel Logic, and TestFramework filters.
- Run bounded exact-baseline CRC, ALU, and general logic comparisons with retained-scale confirmation only for useful candidates.
- Run zd check word and git diff --check.

## Result

Implemented and independently verified direct native named OR and XOR actors; retained AND and alias cases on Table based on independent benchmarks.

Validation:

- Fresh Release Logic, Conditional, MiniModel Logic, and TestFramework suites passed; exact c00010933c general, CRC, and ALU comparisons preserved all semantic/search counters while OR/XOR improved runtime; zd and diff checks passed.
