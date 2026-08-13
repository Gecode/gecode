+++
schema_version = 1
id = "word-054"
key = "fold-hash-round-constants"
area = "word"
status = "open"
blocked_by = ["word-053"]
+++
# Fold hash round constants into n-ary additions

## Outcome

The MD5 and SHA-1 examples express each modular step as one existing n-ary addition including its assigned round constant, reducing model actors while keeping authentic semantics and clear Gecode modeling style.

## Boundaries

- Change only the two hash examples and normal documentation comments if needed; do not add an API, actor, rewrite policy, test harness, or production arithmetic change.
- Treat changed propagation/search as an intentional measured model tradeoff and require exact concrete hash semantics.
- Do not fold unrelated additions or tune driver/search defaults.

## Done when

- [ ] Each MD5/SHA-1 step posts one existing n-ary add containing the assigned round constant and removes the redundant intermediate binary constant-add variable/actor.
- [ ] Quick defaults retain stable correct output, and retained configurations preserve exact solutions/checksums while recording actor, propagation, node, failure, runtime, and memory changes.
- [ ] Focused example builds/runs, ordinary Word arithmetic regression checks, zdev check, and diff check pass without new test or benchmark machinery.

## Validation

- Build and run only the two examples at quick defaults and bounded retained scales.
- Run focused existing Word Arithmetic and TestFramework filters.
- Run zd check word and git diff --check.
