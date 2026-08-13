+++
schema_version = 1
id = "word-053"
key = "optimize-nary-add-closure"
area = "word"
status = "done"
blocked_by = ["word-052"]
+++
# Prove and optimize NaryAdd local closure

## Outcome

NaryAdd avoids redundant local support passes through a proved Gecode-style fixpoint rule while preserving its native actor, propagation contract, aliases, lifecycle, and general-model behavior.

## Boundaries

- Do not simply remove the local loop from hash-model parity; derive when another pass can change support and implement the smallest sound rule.
- Preserve the public API, one native actor, bounded-carry representation, propagation conditions, cost, cloning, subsumption, and no per-bit model variables or benchmark switches.
- Do not specialize for MD5/SHA-1, add advisors/finer events without evidence, or broaden into binary Add, Table, channels, shifts, or search policy.

## Done when

- [x] The fixpoint analysis states which publications can invalidate retained support and proves the implemented stopping/reiteration condition for general widths, arities, aliases, and modular discarded carry.
- [x] The retained implementation removes measured redundant work with no benchmark-specific path and follows ordinary Gecode actor, Region, publication, scheduling, clone, and subsumption patterns.
- [x] Normal existing test/word arithmetic coverage is extended only as needed to exercise the changed fixpoint seam, including a small partial/alias case and genuine recomputation when current tests do not already prove it.
- [x] Exact-baseline Release comparisons cover MD5, SHA-1, existing n-ary assigned/partial/inverse/search cases, and at least one non-hash general n-ary workload, recording semantic/search parity, actor/propagation/work counters, wall time, and best-effort memory; retain only a useful repeatable result.

## Validation

- Run focused Word Arithmetic NaryAdd/Lifecycle and TestFramework tests plus any normal changed-seam test.
- Run temporary exact-baseline Release comparisons with raw results and bounded profiles; do not track benchmark artifacts.
- Run zd check word and git diff --check, and inspect the exact source diff for Gecode lifecycle and fixpoint correctness.

## Result

Proved and independently verified a distinct-view NaryAdd idempotence rule that removes redundant local closure passes while preserving alias closure.

Validation:

- Focused NaryAdd/Lifecycle/full Arithmetic/TestFramework tests and 4,489,182 exhaustive small cube/constant idempotence checks passed.
- Exact baseline Release MD5, SHA-1, general n-ary, and alias comparisons preserved semantics/search and showed material hash/search gains with recorded tradeoffs.
