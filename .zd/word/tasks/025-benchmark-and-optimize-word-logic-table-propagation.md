+++
schema_version = 1
id = "word-025"
key = "logic-table"
area = "word"
status = "done"
blocked_by = ["word-024"]
+++
# Benchmark and optimize Word Logic Table propagation

## Outcome

The generic Word Logic Table actor performs less repeated narrowing and scheduling work while preserving its public behavior and bit-consistency contract.

## Boundaries

- Treat this as an independent generic actor optimization; do not change addition or multiplication algorithms in this task.
- Preserve the existing truth-table semantics, alias projection, cloning, subscriptions, failure, fixpoint, and subsumption behavior.
- Specialize arities or common truth tables only when focused measurements justify the added code; do not create public operation-specific APIs or a parallel actor framework.
- Reuse existing logic, conditional, and Word framework tests and add only a focused regression for a concrete optimized seam when needed.

## Done when

- [x] A focused microbenchmark separates arity, width, partial-domain density, and repeated-event behavior and identifies the concrete repeated work being removed.
- [x] The optimized actor preserves soundness and bit consistency for every currently supported arity and truth table.
- [x] Release measurements show the change's effect on propagation calls or per-call runtime for logical chains and the arithmetic workloads that still use Logic Table.
- [x] Sampling confirms whether Logic Table propagation and WordView narrowing remain the dominant native hot paths, with any residual limitation recorded.

## Validation

- Build the Word library and existing gecode-test target in Release mode.
- Run focused Word Logic, Conditional, Arithmetic, and TestFramework selections only.
- Run the focused table microbenchmark and representative deep logical benchmark.
- Run a bounded sampling profile and git diff --check.

## Result

Optimized Logic Table by computing closure on local masks and publishing each changed view once.

Validation:

- Focused Release Logic, Conditional, Arithmetic, and TestFramework selections passed.
- A 108-cell microbenchmark preserved event and propagation counts and improved all cells with 1.061x median speedup.
- Deep logical search preserved all search metrics and improved runtime by 1.061x; sampling showed a 65 percent reduction in WordView narrow top-stack samples.
- Independent Spec and Standards verification passed; git diff --check passed.
