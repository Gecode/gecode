+++
schema_version = 1
id = "value-params-003"
key = "enable-check"
area = "value-params"
status = "open"
afk = true
blocked_by = ["value-params-002"]
+++
# Enable the check and document the rule

## Outcome

performance-unnecessary-value-param, with its AllowedTypes, is enabled in .clang-tidy. docs/code-style.md and changelog.in describe the rule.

## Context

Add the check next to the other performance checks in .clang-tidy and put AllowedTypes under CheckOptions with a short comment. docs/code-style.md currently says that `performance-unnecessary-value-param` is reserved for manual performance audits. Replace that paragraph with the enabled policy: list the allowed types or the regex and why copying them is shallow, and state that owned and reference-counted types are not exempt.

## Done when

- [ ] .clang-tidy enables the check with the AllowedTypes from the inventory, or a revised list explained in the result.
- [ ] docs/code-style.md no longer calls the check manual-only, and documents the allowed types and the reason for them.
- [ ] changelog.in has a 7.0.0 entry about the changed public signatures.

## Validation

- Last task in the area: run `uv run misc/check.py all --build-dir build/full --jobs N`. Fix failures that this area caused. Report failures that already exist at the area base.
