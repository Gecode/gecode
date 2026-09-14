+++
schema_version = 1
id = "word-017"
key = "repair-logic-publication"
area = "word"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Repair bounded publication in binary and n-ary Word logic

## Outcome

Logical constraints remain sound after cube publication triggers numeric-domain synchronization.

## Context

Review of cced251d26 reproduced unsatisfiable four-bit unsigned AND domains a={11,13}, b={7}, c={0,1,2} accepted as 11&7=1, and OR domains a={10,11}, b={3,6,7}, c={6,10} accepted as 10|6=10. These occur at posting and after later tells. Read logic/table.hpp:96, binary.hpp:74 and nary.hpp:216 plus test/word/logic.cpp. Local cube masks omit deductions caused by narrow(); binary actors retire and n-ary actors return a stale ES_FIX.

## Boundaries

- Preserve the compact fast path, existing aliases and APIs; use ordinary actor scheduling.
- A final assigned-tuple check alone is insufficient: partial synchronized domains must reach an honest fixpoint.

## Done when

- [x] Binary table/native and n-ary AND/OR reject the concrete invalid tuples at posting and after delayed tells.
- [x] Publication, ES_FIX and subsumption reflect actual synchronized domains; supported tuples are preserved.
- [x] Focused small-width mixed-domain and alias regressions exercise this mechanism in the existing test suite.

## Validation

- Run Word logic and TestFramework; run broader Word tests if shared synchronization or view code changes.
- Compare a compact logical control in Release if the compact hot path changes.

## Result

Repair bounded Word logic publication to reach synchronized fixpoints before retirement

Validation:

- build/bin/gecode-test -test Word::Logic passed
- build/bin/gecode-test -iter 5 -test Word::TestFramework passed
- git diff --check passed
