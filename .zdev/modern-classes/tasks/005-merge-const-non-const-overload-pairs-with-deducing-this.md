+++
schema_version = 1
id = "modern-classes-005"
key = "deducing-this"
area = "modern-classes"
status = "open"
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

- [ ] Each convertible inventory row is converted or the result names why it was kept.
- [ ] docs/code-style.md states when to use deducing this.

## Validation

- check-standard in build/full and ctest in build/local.
- uv run misc/check.py lint --build-dir build/full and uv run misc/check.py all --build-dir build/full --jobs N before the commit.
