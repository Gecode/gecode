+++
schema_version = 1
id = "boost-011"
key = "interval-record"
area = "boost"
status = "done"
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

- [x] changelog.in's unreleased section names each user-visible change listed in the outcome.
- [x] uv run misc/tidy.py --check passes.

## Validation

- uv run misc/tidy.py --check

## Result

changelog.in's unreleased section describes the native Float::Interval and the removed Boost copy and license file (bounds and results unchanged, new empty-interval message, MIT note), the API changes of Float::RoundingBase, Float::Rounding and FloatValImpType, and the trivially copyable FloatVal with its changed calling convention and the two affected exported functions.

Validation:

- uv run misc/tidy.py --check passes. Fresh verifier (codex gpt-6.1-sol high) checked every named API change against the committed code and the ABI claim with clang on arm64 and x86_64; its finding on the const-reference sentence was corrected, the recheck confirmed FloatValArgs::create and Float::Linear::post are the only exported by-value FloatVal functions, and the remaining overstated sentence was removed.
