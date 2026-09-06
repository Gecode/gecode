+++
schema_version = 1
id = "word-040"
key = "investigate-fused-word-arithmetic"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "low"
blocked_by = ["word-031", "word-025", "word-026"]
+++
# Evaluate fused sums multiply-add and high-half products

## Outcome

One or two measured arithmetic modeling gaps are selected from weighted modular sums multiply-add and high/low products.

## Context

Word already exposes modular add/mult and overflow; high-half multiplication of 64-bit inputs needs a result wider than one WordVar. DSP/address arithmetic and shift/add circuit synthesis are plausible users. Read the CPKB Lagoon/Metodi 2020 multiplication-by-constant circuit paper; factor recovery alone is not circuit-topology synthesis.

## Boundaries

- Choose at most two cases based on concrete model evidence; saturation starts as existing overflow+ITE composition.
- Use temporary prototypes and a high/low pair if necessary; do not add arbitrary-precision variables or a full ISA.
- Do not retain a fused actor solely because it reduces actor count.

## Done when

- [x] A faithful model and existing decomposition expose the chosen gap.
- [x] Exact small-width and width-64 boundary semantics are specified and checked for any candidate.
- [x] Runtime/search/memory evidence leads to narrowly scoped implementation recommendations or a clean rejection.

## Validation

- Run capped Release comparisons and independent arithmetic evaluators; record source-backed conclusions.

## Result

Measured unsigned high/low multiplication against a Boolean schoolbook network; confirmed the modeling gap, deferred a scalable native actor, and rejected the network as the intended decomposition.

Validation:

- Exact candidate, Boolean-control, and Python-bigint projections matched for 87,380 width-1-through-8 operand pairs and 35 inverse/alias cases.
- Independent verification confirmed all recorded width-64 boundary results and high=max impossibility.
- Seven interleaved capped Release trials and retained-clone RSS evidence were independently reviewed at snapshot W35edb3bf083905c7.
