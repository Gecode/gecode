+++
schema_version = 1
id = "word-010"
key = "investigate-bounded-size-merits"
area = "word"
status = "done"
complexity = "advanced"
blocked_by = ["word-004"]
+++
# Investigate bounded Word size merits

## Outcome

A measured experiment decides whether cheap bounded-domain size merits improve variable selection beyond unknown-bit count.

## Context

Prototype the brief's candidate min(unknown_size, span_bits), clamped to one for unassigned variables. The exact microcase is one brancher over 2,048 alternating width-16 unsigned variables: wide=[0,65535] and narrow=[32767,32768]. Both retain a full 16-bit cube. Record the first selected position and repeated root-choice scan cost; use the DMA model for search-tree evidence.

## Boundaries

- Apply the candidate consistently to size, degree-size, AFC-size, action-size, and CHB-size denominators or merits.
- Do not enumerate admitted values or compute exact cube-and-interval cardinality.
- Compact views retain unknown_size.
- Retain the selector change only on repeatable search or runtime value without a compact regression; otherwise record the rejection and remove the prototype.

## Done when

- [x] The baseline and candidate first choices are recorded for the exact alternating-variable microcase.
- [x] Width-64 span calculation and all merit denominators are overflow- and zero-safe.
- [x] Twenty interleaved Release trials report root-choice scan time for the microcase and solutions, nodes, failures, propagations, and runtime for the DMA model and compact controls.
- [x] The result is either a justified retained merit or a clean rejection with no prototype code left.

## Validation

- Run focused Word branching tests for every affected selector while the prototype is present.
- Run the exact 2,048-variable microcase and DMA comparisons from the brief.
- Run TestFramework if code is retained.

## Result

Rejected the bounded size-merit prototype after it changed the synthetic choice but added substantial scan cost without DMA search or runtime value; no prototype code was retained.

Validation:

- Independent reconstruction covered all five selectors, width-64 and zero safety, and selected the expected narrow variable in the exact 2,048-variable microcase.
- Twenty interleaved trials showed a 1.552x bounded scan penalty and no repeatable DMA gain with identical solutions, checksums, nodes, failures, and propagations; the final source tree was clean.
