+++
schema_version = 1
id = "parallel-search-worker-control-007"
proposal_key = "validate-concurrency-lifecycle"
proposal_hash = "sha256:41054f37d0b63a05c54eefd1cc09e763ecc95fc9a2a7d7d49d10418245f9247a"
area = "parallel-search-worker-control"
kind = "implementation"
interaction = "afk"
priority = "high"
status = "done"
blocked_by = ["parallel-search-worker-control-006"]
+++
# Stress resize races and lifecycle transitions

## Source

Parallel engine stop flags, path stealing, and PBS runnable completion have required recent race and lifetime fixes; resizing adds new cross-thread transitions.

## What to build

Add a deterministic stress harness and sanitizer validation matrix that repeatedly changes limits during exploration, solution delivery, stop, reset, restart, no-good extraction, constrain, and destruction. Fix any defects uncovered within the approved worker-control scope and retain focused regressions.

## Non-goals

- Broad sanitizer cleanup outside search-worker control.
- Performance tuning.

## Acceptance criteria

- [x] ThreadSanitizer reports no races across repeated concurrent resize scenarios.
- [x] ASan and UBSan report no lifetime or undefined-behavior failures.
- [x] Stress runs do not deadlock under reset, termination, PBS stop propagation, or destruction.
- [x] Failures are reproducible through focused regression tests rather than timing-only scripts.
- [x] The validation record lists exact configurations and commands used.

## Validation

- Run the focused test set under TSan, ASan, and UBSan builds.
- Repeat the concurrency stress harness with fixed seeds and high iteration counts.
