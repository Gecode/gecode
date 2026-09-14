+++
schema_version = 1
id = "word-067"
key = "hybrid-word-integer-domain"
area = "word"
status = "done"
blocked_by = []
+++
# Investigate hybrid Word cubes with integer bounds and integer-compatible views

## Outcome

Determine whether augmenting Word cubes with unsigned-integer bound storage, signed and unsigned interpretations, and compatible integer views provides enough propagation and modeling value to justify its kernel, event, cloning, and actor costs.

## Context

The current lo and hi masks are the unsigned minimum and maximum of the represented cube, so caching those values alone adds no information. Additional precision requires a hybrid domain: a bit cube intersected with a numeric range. Store bound endpoints as unsigned WordValue values, while supporting both unsigned interpretation and signed two's-complement interpretation. Investigate whether interpretation belongs to immutable variable state or, preferably, to signed and unsigned views. Such views might allow established integer propagators—such as value- and bounds-consistent distinct and mathematical arithmetic constraints—to operate directly on representable Word values.

## Boundaries

- Do not change the production WordVar representation, public API, events, branching, or propagators in this investigation.
- Bound endpoints use unsigned WordValue storage; do not introduce host signed overflow or silently truncate values.
- Support both unsigned and signed two's-complement interpretations in the prototype.
- A signed interval crossing zero is not one ordinary unsigned interval; specify and test its representation rather than treating encoded endpoints as unsigned ordered bounds.
- Determine whether signedness belongs to the variable or to a view; do not assume the answer without comparing lifecycle, reuse, and semantic consequences.
- Standard Int constraints may be reused only for values representable within Gecode's supported integer limits.
- Keep mathematical integer arithmetic through an integer view distinct from modular Word arithmetic.
- Do not claim general compatibility with Int propagators until their required view protocol, propagation conditions, deltas, range iteration, and failure semantics have been audited.
- Keep all prototypes, instrumentation, and benchmark artifacts outside the repository.
- Only propose implementation tasks after an evidence-backed go decision.

## Done when

- [x] The candidate domain is specified as a cube intersected with numeric bounds whose endpoints are stored as WordValue values, including canonicalization to the first and last cube members admitted by the interpreted range.
- [x] Unsigned and signed two's-complement interpretations are both specified, including zero crossing, the sign boundary, width 1, width 64, and values outside Gecode Int limits.
- [x] Exhaustive small-width checking validates non-empty intersection, monotone narrowing, canonicalization, assignment, failure, signed and unsigned extrema, and clone/replay equivalence.
- [x] The investigation compares immutable variable signedness with signed and unsigned views and records an evidence-backed architectural choice.
- [x] The required Gecode integer-view protocol is audited against representative standard propagators, including value-consistent distinct, bounds-consistent distinct, order relations, linear or addition constraints, and at least one non-linear arithmetic constraint.
- [x] A temporary prototype demonstrates which representative Int constraints can operate directly through a Word integer view and which require channeling, adapters, new overloads, or cannot preserve their contract.
- [x] The investigation precisely documents the semantic distinction between mathematical integer arithmetic through the view and modular Word arithmetic.
- [x] The additional bytes per Word variable and clone, update and canonicalization costs, branching effects, and required modification events, deltas, and propagation conditions are measured.
- [x] Representative workloads include fixed-product multiplication and factor recovery, unsigned division or comparison, signed ordering, shifts as a mixed case, and hashes or CRC as overhead controls.
- [x] Exact solution parity and clone/recomputation equivalence are established for every prototype workload.
- [x] The result reports useful propagation, nodes, failures, wall time, memory, event traffic, and comparison with both the current cube and derived-on-demand extrema.
- [x] The Word brief records a go or no-go decision; a go result proposes separately approved kernel, view/API, branching, actor-migration, and validation tasks rather than implementing them here.

## Validation

- Use Release builds, hard time bounds, adjacent scaling points, and preserved temporary provenance.
- Run exhaustive small-width prototype checks and representative integer-view compatibility models.
- Compare value- and bounds-consistent distinct plus representative arithmetic constraints against ordinary IntVar models with exact solution parity.
- Confirm no production, API, test, example, or build file changed.
- Run zd check word and git diff --check.

## Result

Investigated hybrid cube-and-integer Word domains and integer-compatible views; rejected universal WordVar bounds due 67% state growth, event/canonicalization costs, Int-limit/protocol barriers, and little benefit outside range-heavy models.

Validation:

- Independent Spec and Standards verification passed after exact Int/channel and mixed-model parity, genuine c_d=1 replay, corrected successful-tell event accounting, and full provenance validation.
- Domain oracle, view compatibility, workload controls, hashes, zd check word, and git diff --check passed; no production/API/test/example/build files changed.
