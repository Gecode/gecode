+++
schema_version = 1
id = "boost-011"
key = "interval-record"
area = "boost"
status = "open"
complexity = "standard"
afk = true
blocked_by = ["boost-010"]
+++
# Record the native interval library in the changelog

## Outcome

changelog.in describes the native interval library under 7.0.0: the removed Boost copy and dependency, the API changes of Float::Rounding, RoundingBase, and FloatValImpType, the trivially copyable FloatVal and its changed calling convention, and the changed empty-interval exception message.

## Context

The spike's API and ABI sections in background/native-spike.md list the source and binary changes; the brief's License decision says the interval code is Gecode code under the MIT license. Follow the existing changelog.in entry style and validate it with uv run misc/tidy.py --check.

## Boundaries

- Initial task-owned path allocation: ["changelog.in"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [ ] changelog.in's unreleased section names each user-visible change listed in the outcome.
- [ ] uv run misc/tidy.py --check passes.

## Validation

- uv run misc/tidy.py --check
