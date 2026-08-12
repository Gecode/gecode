+++
schema_version = 1
id = "word-049"
key = "research-realistic-profiling-corpus"
area = "word"
status = "done"
blocked_by = ["word-048"]
+++
# Research realistic WordVar profiling instances

## Outcome

A ranked, source-backed set of realistic public WordVar examples is calibrated to tens-of-seconds runtimes and identifies where native versus decomposed models can reveal algorithmic hot spots.

## Boundaries

- This is a bounded research and temporary-prototype task, not authorization to add production propagators, durable benchmark infrastructure, imported SMT-LIB corpora, or large example programs.
- Prefer public deterministic constructions derived from documented algorithms; do not copy private instances or third-party benchmark payloads into the repository.
- Use Boolean or IntVar decompositions only where they are semantically equivalent and conventional in Gecode; record a justified omission otherwise.
- Reject candidates whose target runtime is obtained mainly through pathological branching, repeated identical roots, printing, or startup overhead.

## Done when

- [x] Primary/local sources establish at least three realistic application families relevant to the implemented Word operations, with provenance and an explicit mapping from each algorithm to WordVar constraints.
- [x] Temporary Release prototypes calibrate at least three retained instances to approximately 10--60 seconds, record exact semantic checks and solver/profile metrics, and include an independent decomposition for at least two cases where reasonable.
- [x] The research ranks retained and rejected candidates by realism, reproducibility, operation coverage, search quality, decomposition value, and profiling usefulness, and identifies concrete suspected algorithmic hot spots without implementing fixes.
- [x] The Word area brief records the final recommended profiling corpus, calibration parameters, evidence, limitations, and follow-up questions; no temporary driver or raw result is tracked.

## Validation

- Verify source claims against the local CP knowledge base and referenced primary papers.
- Build temporary drivers against the current Release Word library and run bounded calibration, native/decomposition parity checks, and sampling profiles.
- Confirm all retained runs are reproducible at the recorded scale, repository status contains only the intended zdev research conclusion, and git diff --check plus zd check word pass.

## Result

Researched and independently verified a realistic WordVar profiling corpus with calibrated CRC and symbolic ALU instances, exact decompositions, profiles, and explicit rejected candidates.

Validation:

- Three deterministic one-root instances completed in 42-59 seconds; two independent Boolean CRC decompositions matched exact solutions, checksums, nodes, and failures.
- Bounded samples identified Word narrowing, Logic Table, channels/shifts, and native Add as concrete hot paths; reduced hash and Tagada candidates were honestly rejected or deferred.
- Primary-source claims, provenance hashes, zdev checks, clean production scope, and git diff --check passed.
