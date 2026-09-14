+++
schema_version = 1
id = "word-018"
key = "repair-selected-equality"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Preserve mixed Word equality through Element and ITE

## Outcome

Equality rewrites stay active until actual synchronized operands are equal.

## Context

In rel/eq.hpp:82 and :100, a shared cube is published to both roles, then assignment of x0 alone causes retirement. Width-three element([a],0,b) with bounded a=[3,4] and compact b cube [0,3] assigns a=3 but later accepts b=0. True Boolean ITE with compact selected a=[0,3] and bounded result r=[3,4] similarly accepts a=0,r=3. Entry seams are element/view.hpp:96 and conditional/ite.hpp:92.

## Boundaries

- Repair the shared equality contract including posting/propagation and affected rewrites; preserve mixed signed/unsigned/compact support.
- Do not restrict previously accepted domain-kind combinations to avoid the bug.

## Done when

- [x] Element and ITE reject unequal selected/result values for initial and delayed control/index/domain tells.
- [x] Both operand orders and aliases preserve supported values and only subsume when equality is entailed.
- [x] Existing equality, conditional and element tests include focused regressions.

## Validation

- Run Word relations, Element, conditional and TestFramework tests.
- Exercise clone/recomputation on the repaired rewrite path.

## Result

Keep mixed Word equality active through Element and ITE synchronization

Validation:

- Word::Rel passed with 5 iterations
- Word::Element passed with 5 iterations
- Word::Conditional passed with 5 iterations
- Word::TestFramework passed with 5 iterations
