+++
schema_version = 1
id = "word-021"
key = "benchmark-corpus"
area = "word"
status = "done"
blocked_by = ["word-007", "word-018", "word-019"]
+++
# Add differential word benchmarking with a private corpus boundary

## Outcome

A repeatable benchmark harness compares native word propagation with an equivalent Gecode Boolean decomposition, accepts an explicit external private corpus root, and remains useful when private data is absent.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not place private corpus payloads in Git, .zd, source archives, binary packages, or public CI.
- Do not claim parity with specialist SMT solvers.

## Done when

- [x] Checked-in manifests describe benchmark metadata and expected external layout without copying private instances.
- [x] The harness runs redistributable smoke fixtures and reports missing or partial private corpora cleanly.
- [x] Reports capture runtime, peak memory, allocations, propagation calls, clone footprint, and solver variant.
- [x] Ignore rules and repository/package scans make accidental private-payload redistribution difficult.

## Validation

- Run the public fixture comparison with and without an external corpus root.
- Validate manifests and representative private-root discovery when local data is available.
- Scan tracked files and package outputs for private corpus payloads.

## Result

Added a resumable native Word versus Gecode Boolean benchmark with public fixtures, external private-corpus discovery, honest structural metrics, and repository/package payload safeguards.

Validation:

- Public, missing-root, partial-private, resume, corrupt/nonterminal rerun, package scan, analyze, report, and diff checks pass; independent verifier PASS.
