+++
schema_version = 1
id = "word-014"
key = "add-bounded-bool-ite"
area = "word"
status = "open"
complexity = "advanced"
blocked_by = ["word-004"]
+++
# Preserve bounds through Boolean-controlled Word ITE

## Outcome

Boolean-controlled ITE over homogeneous bounded Words propagates cube and interval hulls and can decide the control from branch disjointness.

## Context

Bool-controlled ITE always instantiates the bit-only WordView actor. With an undecided control it publishes only the cube hull, dropping numeric information at program-like control and data joins. Focused measurements found a useful bounded hull gap.

## Boundaries

- Use typed homogeneous unsigned or signed views and preserve the current actor for compact, mixed-kind, and unsupported alias cases.
- The interval union may be non-convex; publish only its sound convex hull.
- Preserve selected-branch equality rewrites, Boolean lifecycle, cloning, rescheduling, cancellation, and disposal.
- Do not change Word-mask ITE in this task.

## Done when

- [ ] An undecided bounded ITE publishes both the cube hull and sound ranked interval hull.
- [ ] Cube-plus-range disjointness can eliminate a branch and decide the Boolean control.
- [ ] Small-width oracles cover signed and unsigned intervals, aliases, duplicate branches, selected controls, cloning, and replay.
- [ ] The realistic mixed model demonstrates useful retained bounds or search value without compact regression.

## Validation

- Run focused Word Conditional tests and the small-width oracle.
- Run interleaved Release comparisons on the realistic mixed model and compact controls.
- Run TestFramework.
