+++
schema_version = 1
id = "word-016"
key = "investigate-bounded-word-distinct"
area = "word"
status = "done"
complexity = "advanced"
blocked_by = []
+++
# Investigate native bounds-consistent Word distinct

## Outcome

A register-address allocation experiment determines the useful and natural contract for one native bounds-consistent Word distinct constraint without claiming general IntVar compatibility.

## Context

Use the exact model in .zdev/word/brief.md. Two banks each contain 2, 3, or 4 logical registers assigned to four aligned eight-bit addresses. Low four bits encode alignment; bank bit six and ranked intervals select [0x00,0x3f] or [0x40,0x7f]. Logical registers are labeled and have no ordering relation, so uniqueness is not otherwise entailed. Compare native Word distinct, pairwise Word disequality, and compact Word plus channel-equal IntVars with Int distinct using the specified count and weighted-address checksum.

## Boundaries

- Evaluate bounds consistency first for homogeneous unsigned bounded Words; signed semantics are documented but not required by this evidence model.
- Treat value consistency as a separate small-domain possibility and do not promise domain consistency over cube holes.
- Do not introduce a blanket Word-as-Int adapter.
- Retain a production actor only if the propagation contract and measured value are clear; otherwise record the rejection and remove the prototype.

## Done when

- [x] The investigation states public semantics, propagation level, width behavior, alias policy, signed-policy decision, and event conditions for native distinct.
- [x] All three formulations produce equal solution counts and weighted-address checksums at 2, 3, and 4 registers per bank.
- [x] Twenty interleaved Release trials report nodes, failures, propagations, actor topology, runtime, and retained-clone RSS slope using the same 2,000, 8,000, and 32,000 populations and measurement cap as the DMA model.
- [x] The result is either a justified production bounds-consistent distinct with focused tests or a clean rejection with no prototype code left.

## Validation

- Run the exact register-allocation comparison from the brief with linked Release binaries, 20 interleaved trials, and the shared retained-clone memory procedure.
- If code is retained, run focused small-width semantic, alias, clone, and replay tests plus TestFramework.

## Result

Investigated native bounds-consistent Word distinct and rejected the production actor after exact semantic, runtime, topology, profiling, and clone-memory measurements; full evidence is linked from the area brief.

Validation:

- Independent verifier confirmed all nine native/pairwise/channel cases match expected counts/checksums and both 20-trial campaigns are stable.
- Startup-free batches measured native at 2.058x pairwise for k=3 and 1.461x for k=4; sampling attributed 1805/3715 samples to Hall propagation while clone slopes stayed within 0.05 percent.
- Clean rejection verified: no API, actor, tests, runner, or prototype artifacts remain; documentation, raw evidence, diff checks, and immutable context comparison passed.
