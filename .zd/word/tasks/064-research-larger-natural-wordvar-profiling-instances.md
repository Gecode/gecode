+++
schema_version = 1
id = "word-064"
key = "research-larger-natural-word-instances"
area = "word"
status = "done"
blocked_by = ["word-063"]
+++
# Research larger natural WordVar profiling instances

## Outcome

A source-backed corpus of larger, natural WordVar models exercises distinct parts of the system at profiling-relevant runtimes and identifies where end-to-end time is spent.

## Context

The existing five-example corpus covers reduced hashes, CRC recurrences, and a constructed symbolic ALU. Search beyond those models for larger and more complicated natural instances that expose different actor topologies, operation mixes, propagation waves, cloning behavior, and cube-domain limitations. Prefer publicly specified algorithms or benchmark families that can be reconstructed without importing restricted payloads. The task is an investigation: prototype and benchmark candidates temporarily, retain a ranked corpus recommendation in the Word brief, and separate implementation inefficiencies from inherent search or representation costs.

## Boundaries

- Investigation only: do not change production propagators, public APIs, normal tests, examples, search defaults, or build infrastructure.
- Keep prototype models, instrumentation, raw results, and third-party inputs outside the repository.
- Do not obtain runtime by repeating independent roots, printing solutions, deliberately poor branching, or merely increasing an existing enumeration count.
- Use ordinary Gecode modeling, propagation, branching, cloning, and profiling patterns.
- Do not claim that a sampled hot function is inefficient without measuring useful work, scaling, and an established alternative.

## Done when

- [x] At least five natural public application or benchmark families are surveyed and mapped to the Word operations, actor topology, adjustable difficulty, provenance, and expected profiling value they exercise.
- [x] At least three retained instances from distinct model families are calibrated to approximately 20–120 seconds on the current Release build, with runtime arising from one meaningful model and search rather than artificial repetition or output.
- [x] The retained corpus collectively exercises arithmetic, logical, structural, relational or reified, mixed Word/Bool or Word/Int, and search/cloning paths; gaps that cannot be covered naturally are recorded explicitly.
- [x] Each retained instance records exact semantics through a solution count or order-independent checksum, actors or advisors, propagations, nodes, failures, wall and user time, best-effort memory, bounded profiles, and a scaling curve over at least three adjacent sizes.
- [x] At least two retained instances have exact parity against an independent Boolean, integer, or decomposed Word formulation at a bounded adjacent scale where such a comparison is faithful and practical.
- [x] The Word brief ranks retained and rejected candidates, distinguishes algorithmic work, scheduling or copying overhead, cube-domain limitations, and model/search effects, and proposes only evidence-backed follow-up tasks.
- [x] No prototype, raw result, profiling probe, benchmark runner, private corpus payload, or third-party instance is tracked.

## Validation

- Build the current Release Word library and the relevant existing example targets.
- Run every retained temporary model under hard time bounds at its adjacent scaling points and retained configuration.
- Verify exact native/decomposition parity for the selected comparison cases.
- Capture bounded profiles and complete solver counters for retained cases.
- Run zd check word and git diff --check, and confirm repository changes are limited to the task result and generated zdev state.

## Result

Surveyed six natural public WordVar families and retained exhaustive Speck key recovery, xorshift state recovery, and semiprime factor recovery profiles with scaling, parity, memory, and bounded profile evidence.

Validation:

- Independent Spec and Standards verification passed after correcting exhaustive Speck evidence, xorshift observation semantics, source attribution, and profile wording.
- Release Word library and five checked-in examples built and ran; zd check word and git diff --check passed.
