+++
schema_version = 1
id = "word-030"
key = "lower-bounded-bool-ite"
area = "word"
status = "done"
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

- [x] Explicit unsigned and signed ITE lowering produce compatible bounded results and preserve interval deductions.
- [x] Default lowering remains cube and direct/WordExpr semantics agree.
- [x] Nested Boolean controls, mixed inputs and alias selections have focused coverage.

## Validation

- Run Word MiniModel and conditional tests, TestFramework and broader Word coverage required by the brief.

## Result

Allow explicit signed and unsigned WordExpr Boolean ITE lowering to reach the native bounded actor while preserving cube-default and unsupported-policy behavior.

Validation:

- Independent verification passed bounded result/interval, direct-vs-expression, nested control, mixed source, alias and cube-default regression; warning-clean Release, MiniModel, Conditional, TestFramework, broad Word and diff checks.
