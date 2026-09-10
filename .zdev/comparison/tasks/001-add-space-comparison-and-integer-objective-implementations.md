+++
schema_version = 1
id = "comparison-001"
key = "integer-comparison"
area = "comparison"
status = "open"
complexity = "standard"
afk = true
priority = "high"
blocked_by = []
+++
# Add Space comparison and integer objective implementations

## Outcome

Models can compare spaces without mutation, with integer convenience classes and FlatZinc providing the standard implementations.

## Context

The approved [brief](../brief.md) defines four ordering outcomes and model-defined comparability. Space currently only has constrain(); MiniModel supplies scalar and lexicographic integer objectives, while Driver::ScriptBase and FlatZinc already have Gist display compare overloads. Start in kernel/core.hpp and core.cpp, minimodel.hh and optimize.cpp, driver.hh, and flatzinc.hh/flatzinc.cpp; use test/search.cpp for focused coverage.

## Boundaries

- Add the API, integer implementations, their documentation, and overload compatibility. Leave search arbitration and float implementations to dependent tasks; do not add generic assignment checks or a capability API.

## Done when

- [ ] Space::compare(const Space&) const returns the four SpaceComparison outcomes; its default reports unsupported use through a Gecode exception. No unavailable result is added.
- [ ] Scalar min/max and lexicographic min/max, plus FlatZinc integer optimization, compare in the agreed direction without mutation. Lexicographic comparison accepts a decisive assigned prefix without requiring later components; insufficient information for the requested comparison raises the appropriate error.
- [ ] Focused tests cover direction, equivalence, a decisive prefix with an unassigned suffix, insufficient scalar data, and incompatible objective families/dimensions. The ordering and Gist display overloads remain callable, including through a custom script.

## Validation

- Build gecode-test and run the new comparison tests plus relevant existing integer/FlatZinc tests. Compile a representative custom script using both compare overloads; no new test harness.
