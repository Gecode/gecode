+++
schema_version = 1
id = "word-003"
key = "clarify-word-endpoint-contracts"
area = "word"
status = "open"
complexity = "standard"
blocked_by = []
+++
# Clarify Word endpoint and exception contracts

## Outcome

Encoded Word endpoints and internal order ranks have unambiguous names, and the public Word variable exception contract matches established Gecode documentation.

## Context

WordView::minimum and maximum expose encoded values, while UnsignedWordView and SignedWordView hide those names with internal ranks. That static-name change is a concrete maintenance hazard in signed bounded propagation. WordVar constructor and accessor exception documentation also trails IntVar. Current supported posting paths already guard typed-view conversions, so this task does not add speculative runtime checks to hot views.

## Boundaries

- Preserve the public WordVar minimum and maximum encoding semantics.
- Use rank_minimum and rank_maximum consistently for internal ordered endpoints; avoid another wrapper abstraction.
- Keep typed bounded-view constructors internal and assert-guarded as they are unless a supported unsafe construction path is demonstrated.
- Do not change the compact or bounded VarImp layout.

## Done when

- [ ] Internal bounded propagation no longer depends on minimum or maximum names whose meaning changes with static view type.
- [ ] Direct-posting, MiniModel, tracing, and bounded propagator callers use explicit rank names where they consume internal order.
- [ ] WordVar, WordVarArgs, and WordVarArray document width, mask, domain-kind, empty-intersection, cube-endpoint, and unassigned-value exceptions.
- [ ] The mechanical migration compiles warning-clean without representation or runtime changes.

## Validation

- Run the broader Word suite and TestFramework.
- Build the Word library in Release mode with the established warning flags.
