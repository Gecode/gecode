+++
schema_version = 1
id = "modern-classes-005"
key = "deducing-this"
area = "modern-classes"
status = "done"
complexity = "standard"
afk = true
priority = "low"
blocked_by = ["modern-classes-002"]
+++
# Merge const/non-const overload pairs with deducing this

## Outcome

Overload pairs and CRTP helpers that the inventory marks as simpler with an explicit object parameter use deducing this; the rest are unchanged.

## Context

Follow the deducing-this rows in background/inventory.md. Merge a const/non-const pair only when, for every receiver the original accepted, the merged member returns the same type and accepts the same cv and ref categories, including rvalue receivers and calls through derived classes. An explicit-object member cannot be virtual or static and takes no trailing cv or ref qualifier, and its address is a plain function pointer rather than a member pointer; check each row for these. State the rule in docs/code-style.md, and name any change to a public callable in changelog.in under 7.0.0, or leave that row unchanged.

## Boundaries

- Rows the inventory marks as not simpler stay unchanged.

## Done when

- [x] Each convertible inventory row is converted or the result names why it was kept.
- [x] docs/code-style.md states when to use deducing this.

## Validation

- check-standard in build/full and ctest in build/local.
- uv run misc/check.py lint --build-dir build/full and uv run misc/check.py all --build-dir build/full --jobs N before the commit.

## Result

Kept Gecode::Matrix::operator()(int, int), the inventory's only convert row, as a const/non-const pair: a merged explicit-object template reaches the members through the derived receiver type, so a class that inherits Matrix privately and re-exports the operator with using stops compiling, and derived members could hide them. The other 48 pairs and the CRTP rows stay as the inventory says. docs/code-style.md states when to use deducing this. No public callable changed.

Validation:

- Independent verifier: full review confirmed the keep with AppleClang 21 and Clang 22 probes and found one docs inaccuracy (visibility requirement applied to all explicit-object members), fixed by the coordinator; focused recheck PASS. check.py all rc=0 on the docs change, Doxygen 91 warnings as at baseline, tidy and git diff --check passed; no C++ changed.
