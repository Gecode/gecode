+++
schema_version = 1
id = "word-032"
key = "refresh-word-operation-docs"
area = "word"
status = "open"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["word-030", "word-027", "word-019"]
+++
# Refresh Word operation coverage and historical performance claims

## Outcome

Public Word documentation matches current dispatch and states the limits of retained performance evidence.

## Context

gecode/word.hh's operation inventory still calls newer bounded n-ary Add/product_mod/ITE/Element paths cube-only and omits newer number/distinct coverage. .zdev/word/background/word-016 describes a sorted prototype while production changed. Historical results span commits and use different search; old multiplication driver actor formulas are obsolete.

## Boundaries

- Document actual supported semantics/propagation without claiming external competitiveness or general bit consistency.
- Preserve historical completed task results; append correction/context rather than rewriting history.
- No new runtime feature or documentation framework.

## Done when

- [ ] Inventory covers current bounded and fallback paths, GCD/divides/distinct and explicit MiniModel policy.
- [ ] Historical prototype/current implementation and historical/current benchmark evidence are clearly separated.
- [ ] Documented caveats include 1-64 widths, single-interval/cube limitations, reified positive modulus and mathematical product_mod before reduction.

## Validation

- Cross-check entries against dispatch source and examples; run relevant documentation/build checks.
- No new tests expected.
