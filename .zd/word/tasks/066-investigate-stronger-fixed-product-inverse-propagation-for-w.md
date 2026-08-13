+++
schema_version = 1
id = "word-066"
key = "mult-fixed-product-inverse"
area = "word"
status = "open"
blocked_by = []
+++
# Investigate stronger fixed-product inverse propagation for Word multiplication

## Outcome

Determine whether sound non-wrapping range and factor-support reasoning can materially reduce fixed-product multiplication search while preserving ordinary modular multiplication behavior.

## Context

The retained semiprime profile uses one Mult actor but produces an almost complete binary tree. At bounded adjacent scales, IntVar multiplication resolves the factors almost immediately. Current Word Mult propagates known low prefixes, forced trailing zeros, and modular inverse prefixes. This task targets the provably non-wrapping fixed-product seam using current cube extrema and established range-hull patterns.

## Boundaries

- Do not add primality or factorization-specific logic.
- Apply numeric range reasoning only when bounds prove that multiplication cannot wrap modulo 2^width.
- Use bounded support enumeration only behind a proved small-candidate gate.
- Do not depend on or modify the proposed hybrid Word-domain representation.
- Keep rejected prototypes and benchmark artifacts outside the repository.
- Retain production changes only when they follow ordinary native Gecode actor and lifecycle patterns.

## Done when

- [ ] A smallest sound candidate derives operand ranges or supported cube hulls from a fixed product in provably non-wrapping cases.
- [ ] Exhaustive small-width checking covers soundness across wrap and non-wrap boundaries, assigned semantics, and every operand/result alias pattern.
- [ ] Focused lifecycle coverage includes cloning, genuine recomputation, failure, fixpoint, and subsumption for any retained candidate.
- [ ] The candidate is measured on the semiprime 24, 25, and 26-bit curves and representative wrapping or modular multiplication workloads.
- [ ] Retention requires substantial node reduction on at least two adjacent semiprime scales, repeatable end-to-end benefit, and no material ordinary-Mult regression; otherwise the experiment is rejected and restored cleanly.

## Validation

- Use the exact current commit as baseline and preserve temporary commands, raw output, profiles, and source hashes.
- Run focused Mult, MultLifecycle, Arithmetic Lifecycle, MiniModel ArithmeticLifecycle, and TestFramework tests for retained code.
- Report wall time, actors, propagations, nodes, failures, solutions, checksum, and best-effort memory.
- Run zd check word and git diff --check.
