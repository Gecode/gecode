+++
schema_version = 1
id = "word-050"
key = "calibrate-reduced-md5-sha1"
area = "word"
status = "done"
blocked_by = ["word-049"]
+++
# Calibrate faithful reduced MD5 and SHA-1 preimages

## Outcome

A faithful reduced-step MD5 preimage, and SHA-1 when bounded calibration succeeds, provides a reproducible tens-of-seconds WordVar profiling workload with an exact independent decomposition where practical.

## Boundaries

- This is bounded research with temporary Release drivers only; do not add examples, benchmark infrastructure, propagators, APIs, or tests to the repository.
- Preserve authentic 32-bit hash steps and scale the inverse boundary or unknown message bits; do not manufacture runtime with repeated roots, deliberately poor branching, sparse digest constraints, printing, or startup work.
- Use a hard timeout and stop the SHA-1 ladder after bounded evidence; a documented rejection is preferable to open-ended calibration.

## Done when

- [x] A temporary MD5 model uses authentic 32-bit step functions, constants, schedule, modular addition, and rotation, with a deterministic known target and a clearly documented inverse-difficulty parameter.
- [x] A bounded calibration ladder retains at least one single-root MD5 instance near 10--60 seconds, or records enough measured evidence to reject the approach without silently changing the problem; SHA-1 is attempted only after MD5 through the same bounded method.
- [x] The retained model records solutions/checksum, actors, propagation calls, nodes, failures, wall time, best-effort memory, and a bounded profile, and an independent Boolean decomposition has exact parity when it fits the budget.
- [x] The Word brief records construction, provenance, calibration results, rejected scales, hot spots, limitations, and the recommended profiling use; no temporary artifact is tracked.

## Validation

- Check the modeled step equations against the Zaikin paper and standard MD5/SHA-1 definitions in the local research corpus.
- Build temporary drivers against the current Release libraries, use hard timeouts, and verify retained results with an independent concrete oracle and decomposition where practical.
- Run zd check word, git diff --check, and confirm repository changes are limited to zdev research state.

## Result

Calibrated and independently verified faithful single-root reduced MD5 and SHA-1 inverse workloads within the profiling budget.

Validation:

- MD5-16/148 completed in 14.33s with 1,048,576 solutions and recorded counters/checksum.
- SHA1-16/180 completed in 18.51s with 1,048,576 solutions and recorded counters/checksum.
- Independent Boolean MD5 at 132 unknown bits matched exact solutions/checksum; source equations, ladders, profiles, hashes, zdev checks, and repository scope independently passed.
