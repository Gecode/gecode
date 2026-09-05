+++
schema_version = 1
id = "word-033"
key = "strengthen-word-benchmark-controls"
area = "word"
status = "open"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["word-025", "word-031", "word-026"]
+++
# Add semantically checked Word benchmark controls

## Outcome

Existing Word benchmark families support reliable decisions and comparisons of representation, propagation and search.

## Context

Current public corpus is one XOR/rotate fixture plus DMA sizes 3/6/9. XOR success requires one known input and cannot represent UNSAT. product_mod inputs [10,20] with modulus 509 never reduce. DMA rank-split versus compact LSB and channel arithmetic change multiple factors. Aggregate checksum/count checks are not exact solution-set validation.

## Boundaries

- Extend existing runners/models and small external development fixtures; no universal benchmark service or full SMT parser.
- Keep one shared contract for later external adapters: status, public decision variables, instance parameters and concrete witness evaluation.
- Support first model, UNSAT/uniqueness and small projected enumeration as separately labelled goals; never count auxiliaries.

## Done when

- [ ] Small independent concrete evaluators validate witnesses and exact small projected solution sets.
- [ ] DMA controls isolate comparable branching where feasible; root/construction/search timing and startup-free batches are available for informative tiny cases.
- [ ] Arithmetic fixtures include genuine modulus reduction, multiple quotient regions, wrapping/non-wrapping and signed cases including widths 32/33 and 63/64 where applicable.
- [ ] XOR and specialty runners accept valid UNSAT outcomes, validate repetition consistency and remove obsolete hardcoded actor counts.
- [ ] The format is documented sufficiently for the three external-family tasks to use it.

## Validation

- Run small public smoke/control sets plus deliberate bad-witness, UNSAT and timeout diagnostics.
- Record semantic parity before any timed campaign; use existing regression seams only where they check substantive behavior.
