+++
schema_version = 1
id = "modern-classes-002"
key = "inventory"
area = "modern-classes"
status = "open"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["modern-classes-001"]
+++
# Inventory defaultable members, comparisons, and overload pairs

## Outcome

A measured inventory lists every candidate for = default, = delete, operator<=> or defaulted operator==, and deducing this, with its treatment, so the following tasks apply only what keeps behaviour.

## Context

Read the brief in full, especially Boundaries (expression-building operators, FloatVal interval comparisons, partial orders). Measure with clang-tidy 22 (/opt/homebrew/opt/llvm@22/bin/clang-tidy) over the lint configuration that misc/check.py uses: modernize-use-equals-default and modernize-use-equals-delete, with their options (for example IgnoreMacros), and also report what modernize-use-default-member-init and cppcoreguidelines-special-member-functions would flag, so the user can see whether they are worth enabling. modernize-use-equals-default does not handle move constructors or move assignments and modernize-use-equals-delete offers only partial fixes, so list the manual candidates too: hand-written move members, private undefined members, and out-of-line `= default` definitions that keep a class non-trivial. For comparisons, list every class with operator==, !=, <, <=, >, or >= and classify it: plain value comparison that can become defaulted operator== or operator<=>; hand-written but equivalent to a defaulted form; or excluded, with the reason (builds an expression, interval or set semantics, heterogeneous comparison, order differs from member order). For deducing this, list const/non-const overload pairs (operator[], begin/end, accessors) and CRTP helpers that a single explicit-object member would replace, and say for each whether it simplifies the code without changing overload resolution for callers, following the checklist in the deducing-this task. Record the commands so the measurement can be rerun. Write .zdev/modern-classes/background/inventory.md and index it in the brief.

## Boundaries

- No source changes in this task; scratch builds and outputs stay under build/.

## Done when

- [ ] .zdev/modern-classes/background/inventory.md records the clang-tidy commands and counts per check and module, one row per comparison class and per overload pair with its treatment and reason, and the proposed check options, and the brief indexes it.
- [ ] Every number in the inventory names the command that produced it.

## Validation

- Rerun one recorded clang-tidy command and confirm its count.
