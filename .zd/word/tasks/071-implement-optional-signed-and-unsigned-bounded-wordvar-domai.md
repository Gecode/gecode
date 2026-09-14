+++
schema_version = 1
id = "word-071"
key = "bounded-kernel"
area = "word"
status = "done"
blocked_by = []
+++
# Implement optional signed and unsigned bounded WordVar domains

## Outcome

One public WordVar supports compact cube, unsigned-bounded, and signed-bounded implementations with immutable construction-time meaning, conventional Word events/views, safe cloning, and optimized atomic tells.

## Context

Task word-070 selected a non-virtual tagged compact WordVarImp plus a larger derived bounded implementation. Correct the combined layout estimate: the expanded event subscription base makes the expected compact and bounded sizes 56 and 72 bytes, not 48 and 64. Task word-068 proved the constant-operation synchronizer; task word-069 proved lazy publication and typed ownership. The x<y, y<x microbenchmark further shows that 99.9998% of its bound tells are bound-only and the full synchronizer costs about 9.8 ns per propagation relative to IntVar. Implement a direct range-tell fast path that accepts canonical endpoints and no-new-prefix cases, falling back to the full synchronizer otherwise.

## Boundaries

- Do not migrate ordinary relation, arithmetic, division, structural, or MiniModel propagators in this task; they continue through cube fallback except for focused internal test actors.
- Keep one public WordVar and one generated Word variable registry; do not add sidecars, virtual dispatch, external allocation, or runtime signed/unsigned conversion.
- The construction-time domain kind is immutable. Cube operations remain valid on every implementation and must synchronize a bounded implementation before notification.
- Use ordinary generated VarImp events, space allocation, forwarding, recovery, and test machinery.

## Done when

- [x] WordVar has backward-compatible cube construction plus public unsigned-bounded and signed-bounded construction, array/argument support, domain-kind introspection, and guarded canonical endpoint queries using WordValue ranks.
- [x] The compact tagged implementation and derived bounded implementation share one Word handle and generated registry; exact measured sizes and every ordinary-cube branch/byte cost are documented, with no vptr or external allocation.
- [x] Cube-intersection-interval invariants, unsigned and signed rank, width 1/64, assignment, failure, initial masks/bounds, and values outside Int limits are implemented and tested.
- [x] Generated Word events and propagation conditions distinguish value, bit-only, bound-only, and combined changes; WordDelta exposes exact bit and old/new bound information and subscriptions schedule selectively.
- [x] Common WordView works on every implementation and internal UnsignedWordView/SignedWordView wrappers can only be constructed after matching kind checks.
- [x] Every tell is atomic and synchronized before notification; the optimized bound-only range tell bypasses the full synchronizer when endpoints are already admitted and no cube prefix changes, otherwise falls back exactly.
- [x] Copy dispatch allocates the correct concrete size and preserves forwarding/recovery; clone isolation, injected mid-copy failure, and genuine c_d=1 recomputation pass for mixed compact/unsigned/signed variables.
- [x] Exhaustive small-width and targeted width-64 tests validate represented membership, canonical endpoints, tell ordering/batching, events/deltas, mixed variables, failure, and assignment against an enumerated oracle.
- [x] The contradictory-order microbenchmark preserves exact failure and propagation counts while the optimized bounded tell materially narrows the per-propagation gap to the ordinary IntVar model; unaffected cube tests and focused Word tests show no material regression.

## Validation

- Regenerate and freshness-check Word VarImp event/type headers using the established generator path.
- Build Release gecodeword_shared and gecode-test with warnings enabled.
- Run focused Word TestFramework, variable lifecycle, relation fallback, branch/trace compatibility, and ordinary arithmetic regression filters.
- Run exhaustive domain/event/copy tests, injected clone-failure validation, and c_d=1 replay.
- Benchmark exact baseline versus candidate and IntVar on x<y and y<x across adjacent widths, reporting propagation counts, time per propagation, fast/slow tell counts, object sizes, and best-effort memory.
- Run zd check word and git diff --check.

## Result

Implemented one public WordVar with compact cube and immutable unsigned/signed bounded domain implementations, typed views, event/delta support, atomic synchronization, clone/fault recovery, and exhaustive kernel tests.

Validation:

- Release full Word suite and focused bounded tests pass; warning-enabled and fault-injection builds pass; generated headers reproduce; cyclic x<y,y<x benchmark and IntVar comparison recorded with 28-entry verified manifest; independent verifier PASS.
