+++
schema_version = 1
id = "word-035"
key = "compare-word-numeric-models"
area = "word"
status = "open"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["word-033"]
+++
# Add external comparisons for inverse arithmetic and an ALU path

## Outcome

Arithmetic inversion and a bounded ALU/program fragment have faithful Gecode/Z3/Bitwuzla comparisons.

## Context

Exercise mult/divmod/product_mod, signed/unsigned relations, variable shifts, ITE and overflow. Use examples/word-symbolic-alu.cpp as a constructed control plus one small documented instruction trace. product_mod is the mathematical product reduced by a positive modulus; same-width bvurem(bvmul(x,y),m) is wrong when multiplication wraps.

## Boundaries

- Use widened bitvector products or mathematically equivalent encodings for product_mod, preserving positive-modulus constraints under reification.
- Define machine wrapping and division semantics explicitly; do not silently claim C undefined behavior is SMT total arithmetic.
- Default Z3 is primary; any PolySAT configuration is separate and checked against the installed release.

## Done when

- [ ] Inverse mult/divmod/product_mod cases cover fixed/tight/free operands, real reduction, wrapping and sign boundaries.
- [ ] The short trace has an independent concrete evaluator and an explicitly identified public input/output projection.
- [ ] SAT and justified UNSAT cases agree across supported solvers; width-64 behavior is not silently truncated.
- [ ] Family scale parameters and supported input subset are recorded for campaign reuse.

## Validation

- Run exact small-width support/solution checks and bounded decision smoke tests, at most 10 seconds per case.
