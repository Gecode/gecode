+++
schema_version = 1
id = "comparison-002"
key = "float-comparison"
area = "comparison"
status = "open"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["comparison-001"]
+++
# Compare float quality independently of the improvement step

## Outcome

MiniModel and FlatZinc float optimization expose a documented comparison consistent with their existing pruning boundaries, without enforcing the step between compared solutions.

## Context

The [brief](../brief.md) specifies MiniModel upper-bound minimization/lower-bound maximization and FlatZinc lower-bound minimization/upper-bound maximization. The difference follows from scalar versus interval-valued strict thresholds; see [float analysis](../background/comparison-and-search.md). Start in minimodel/optimize.cpp, flatzinc/flatzinc.cpp, float/rel.cpp, and float/rel/nq.hpp.

## Boundaries

- Implement comparison and focused tests/documentation. Preserve constrain(), strictness, interval arithmetic, and step semantics; do not add a reporting-step filter or legacy comparison fallback.

## Done when

- [ ] Both MiniModel float bases and FlatZinc float optimization implement the agreed family-specific ordering and document what makes their operands comparable, including tight non-singleton intervals.
- [ ] Tests demonstrate ranking of sub-step improvements, equal keys, both directions, zero/nonzero step, strict threshold boundaries, and adjacent-endpoint intervals. Equal keys produce equivalent cuts and improved keys produce nested cuts on representative comparable solutions.
- [ ] Comparison leaves both inputs unchanged. If a boundary check disproves cut compatibility, report its concrete counterexample rather than silently changing the approved semantics.

## Validation

- Run focused float comparison/cut tests and relevant existing float and FlatZinc tests. Derive the cut-nesting argument from the current posting code; no timing tests or broad floating-point matrix.
