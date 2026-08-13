+++
schema_version = 1
id = "word-057"
key = "prototype-delta-local-word-bit-channel"
area = "word"
status = "open"
blocked_by = ["word-056"]
+++
# Prototype delta-local Word bit channeling

## Outcome

Word-to-Bool bit channeling uses an established Gecode advisor lifecycle only if exact changed-bit advice materially reduces irrelevant CRC scheduling enough to improve end-to-end performance and memory after direct logic actors land.

## Boundaries

- Use ordinary Council and ViewAdvisor patterns with symmetric subscription, copying, rescheduling, disposal, and AP_DISPOSE handling; invent no Word-specific event, scheduler, cache protocol, or persistent external allocation.
- Preserve the public channel API and exact channel semantics, and keep the current simple actor if advisor fan-out or clone footprint outweighs avoided propagation.
- Measure after the direct named logic task so the remaining channel share is not overstated.

## Done when

- [ ] A temporary conventional advisor candidate consumes WordDelta changed-bit masks and avoids scheduling work for unrelated bits without changing solutions or propagation semantics.
- [ ] If retained, normal registered Channel tests cover relevant-bit and unrelated-bit changes, aliases where applicable, failure, clone, recomputation, disposal, and subsumption; if rejected, production and tests are restored cleanly.
- [ ] Exact-baseline Release comparisons on both CRCs record advisor/actor calls, copies, disposal, actors, propagations, nodes, failures, solutions/checksums, runtime, and best-effort memory, plus bounded non-CRC regression workloads.
- [ ] The result explicitly accepts or rejects the advisor based on end-to-end payoff and leaves no temporary instrumentation or benchmark artifact tracked.

## Validation

- Build the Word library and monolithic test target in Release and run focused Word Channel, CRC example, recomputation, and TestFramework checks.
- Run bounded exact-baseline comparisons on both CRCs and representative hash/ALU controls, with retained-scale CRC confirmation only if useful.
- Run zd check word and git diff --check.
