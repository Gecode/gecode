+++
schema_version = 1
id = "word-030"
key = "lower-bounded-bool-ite"
area = "word"
status = "open"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["word-018"]
+++
# Carry explicit bounded policy through WordExpr Boolean ITE

## Outcome

Explicit signed/unsigned WordExpr lowering can reach the native bounded Boolean ITE actor.

## Context

compatible_domain at minimodel/word-expr.cpp:275 excludes NT_BOOL_ITE. Consequently ite(control,a,b).post(home,WDT_UNSIGNED) creates a cube result despite the direct bounded ITE added after the whitelist. The default cube policy is intentional; dropping an explicit compatible request is the gap.

## Boundaries

- Preserve cube-default syntax and established explicit-policy contracts; do not infer policy from input types.
- Update compatible ITE paths only, leaving unsupported operator policies documented.

## Done when

- [ ] Explicit unsigned and signed ITE lowering produce compatible bounded results and preserve interval deductions.
- [ ] Default lowering remains cube and direct/WordExpr semantics agree.
- [ ] Nested Boolean controls, mixed inputs and alias selections have focused coverage.

## Validation

- Run Word MiniModel and conditional tests, TestFramework and broader Word coverage required by the brief.
