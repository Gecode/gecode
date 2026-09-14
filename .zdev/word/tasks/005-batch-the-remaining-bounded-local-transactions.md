+++
schema_version = 1
id = "word-005"
key = "batch-bounded-local-transactions"
area = "word"
status = "done"
complexity = "standard"
blocked_by = []
+++
# Batch the remaining bounded local transactions

## Outcome

Bounded div, mod, combined divmod, signed division, and variable left shift batch local deductions and synchronize each distinct role once per fixpoint pass.

## Context

Bounded Add, Subtract, Multiply, Negate, and fixed left shift use deferred local deductions. The remaining actors synchronize after individual range deductions and republish unchanged snapshots. Measure the exact two controls in .zdev/word/brief.md: 512 independent width-12 divmod tuples with a=[1536,2047], b=[17,31], q=[48,120], r=[0,30]; and 512 width-13 variable shifts with x=[250,550], amount=[2,3], result=[2000,4400]. Each trial retains 100 clones.

## Boundaries

- Reuse the existing local-domain transaction pattern; do not introduce a general transaction framework.
- Preserve alias grouping, SMT-LIB exceptional division rows, fixed-point closure, staged costs, cloning, and recomputation.
- Skip locally unchanged publications but keep VarImp tells as the validator for actual changes; do not add a trusted bypass API.

## Done when

- [x] All named actors defer deductions, synchronize each distinct aliased role once per local pass, and iterate when synchronization exposes new information.
- [x] Unchanged local snapshots do not enter narrow_domain.
- [x] The two exact controls show fewer synchronization calls with identical root domains, actors, solutions where searched, and propagation outcomes.
- [x] Twenty interleaved Release trials of the bounded and compact controls show no repeatable compact regression.

## Validation

- Run focused Word arithmetic and structure tests, including aliases, width one, width 64, exceptional division, cloning, and replay.
- Run the exact 512-constraint, 100-clone, 20-trial controls from the brief against the parent commit.
- Run TestFramework.

## Result

Batched bounded div/mod and variable-shift local transactions with alias-aware synchronization and unchanged-publication guards.

Validation:

- Independent focused arithmetic and structure lifecycle, alias, exception, clone, replay, width-one, and width-64 tests passed.
- Exact parent/current controls preserved statuses, actor counts, and domain hashes while reducing bounded synchronization calls from 18,999 to 10,807 for divmod and 9,784 to 6,711 for variable shift.
- Twenty interleaved Release trials showed no repeatable compact regression; TestFramework and diff checks passed.
