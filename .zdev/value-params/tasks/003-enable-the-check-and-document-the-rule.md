+++
schema_version = 1
id = "value-params-003"
key = "enable-check"
area = "value-params"
status = "done"
afk = true
blocked_by = ["value-params-002"]
+++
# Enable the check and document the rule

## Outcome

performance-unnecessary-value-param, with its AllowedTypes, is enabled in .clang-tidy. docs/code-style.md and changelog.in describe the rule.

## Context

Add the check next to the other performance checks in .clang-tidy and put AllowedTypes under CheckOptions with a short comment. docs/code-style.md currently says that `performance-unnecessary-value-param` is reserved for manual performance audits. Replace that paragraph with the enabled policy: list the allowed types or the regex and why copying them is shallow, and state that owned and reference-counted types are not exempt.

## Done when

- [x] .clang-tidy enables the check with the AllowedTypes from the inventory, or a revised list explained in the result.
- [x] docs/code-style.md no longer calls the check manual-only, and documents the allowed types and the reason for them.
- [x] changelog.in has a 7.0.0 entry about the changed public signatures.

## Validation

- Last task in the area: run `uv run misc/check.py all --build-dir build/full --jobs N`. Fix failures that this area caused. Report failures that already exist at the area base.

## Result

Enabled performance-unnecessary-value-param with the inventory AllowedTypes, documented the rule in docs/code-style.md, and added the 7.0.0 changelog entry; SharedData(Data&&) now requires a movable Data so copy-only data keeps the copying constructor

Validation:

- uv run misc/tidy.py --check passed
- uv run misc/check.py all --build-dir build/full --jobs 12 exit 0: format, freshness, build, lint with the check enabled, ctest 8/8, both package fixtures, doc
- Array::SharedData::CopyOnly fails to compile without the constraint and passes with it; Array:: 78/78
- Independent Codex verifier: pass on round 3 after AFC wording, move qualifier, SharedData constraint, and deleted-move exception corrections
