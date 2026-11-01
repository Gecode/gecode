+++
schema_version = 1
id = "scoped-enums-004"
key = "rename-constants"
area = "scoped-enums"
status = "open"
afk = true
blocked_by = ["scoped-enums-003"]
+++
# Apply the approved constant renames

## Outcome

Each approved constant rename is applied everywhere, with no compatibility aliases, and listed in the changelog.

## Context

Follow the earlier IRT_LE to IRT_LT rename (commit b835077bb2) as the model for scope, documentation, and changelog. If the user approved no renames, complete this task with a result that says so.

## Done when

- [ ] No old spelling of an approved rename remains as a declaration, a compatibility alias, or a live use in maintained code or generator output. Historical descriptions and explicit migration tables may keep the old spelling.
- [ ] docs/code-style.md's table of renamed spellings and changelog.in list each rename.

## Validation

- Last task in the area: run `uv run misc/check.py all --build-dir build/full --jobs N`. Fix failures that this area caused. Report failures that already exist at the area base.
