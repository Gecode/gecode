+++
schema_version = 1
id = "word-002"
key = "add-bounded-arithmetic-soundness-oracle"
area = "word"
status = "open"
complexity = "standard"
blocked_by = []
+++
# Add a partial-domain oracle for bounded arithmetic

## Outcome

A deterministic small-width corpus checks sound bounded Add, Subtract, Multiply, Negate, carry, and borrow propagation over partial cube-and-interval domains.

## Context

test/word/arithmetic.cpp has selected bounded scenarios and exhaustive assigned division tables, but it does not systematically check partial-domain inverse arithmetic and alias interactions. Add the missing soundness seam inside the existing bounded arithmetic lifecycle coverage; do not create a new test framework.

## Boundaries

- Use widths 2, 3, and 4 for both signed and unsigned kinds. At each width, normalize and deduplicate exactly these domain rows: full; lower rank half; upper rank half; interior rank interval [1,mask-1]; singleton ranks 0, 1, and mask; low bit fixed to zero and one; most-significant bit fixed to zero and one; and the width-masked alternating 0x5 cube.
- For distinct roles, cross the deterministic rows and retain only cases whose posting predicates select the bounded actor. For aliases, exercise x=y, x=z, y=z, and x=y=z using each single row plus every full/lower/upper other-role row.
- For carry and borrow, cover terminal zero and terminal one over the same full/lower/upper/singleton rows.
- Check supported concrete tuples and failure behavior, not actor counts or exact intermediate steps.

## Done when

- [ ] Every retained row proves that bounded propagation preserves all supported concrete tuples and fails only when none remain.
- [ ] The corpus exercises forward and inverse interval deductions, cube-to-interval feedback, all four alias mappings, terminal carry, and terminal borrow.
- [ ] The test asserts that each intended case selected a bounded actor rather than silently falling back to cube propagation.
- [ ] The coverage is integrated into the existing Word arithmetic test structure without a new reusable harness abstraction.

## Validation

- Run the focused Word arithmetic tests and record the corpus case count.
- Run TestFramework.
