+++
schema_version = 1
id = "word-055"
key = "audit-native-word-propagation-costs"
area = "word"
status = "done"
blocked_by = []
+++
# Audit native Word propagation and scheduling costs

## Outcome

A second exact-baseline profiling round over all five realistic Word examples identifies and ranks removable implementation or algorithmic work using established Gecode actor, event, staging, batching, and cloning patterns, and defines only evidence-backed follow-up tasks.

## Boundaries

- Investigation only: do not retain a production, test, example, API, search-policy, or benchmark-framework change in this task.
- Use temporary detached instrumentation and bounded samples; remove probes after measurement and keep raw artifacts outside the repository.
- Compare proposed mechanisms with representative established Int, Bool, Set, or Float implementations and reject Word-specific scheduler, cache, advisor, or event machinery without measured lifecycle payoff.
- Do not reopen completed NaryAdd closure work or infer a global clone-distance policy from example sensitivity.

## Done when

- [x] All five checked-in examples have reproducible current-commit Release baselines at bounded and, where decision-relevant, retained scales with exact semantic/search counters, timing, best-effort memory, and bounded profiles.
- [x] Temporary instrumentation separates actor calls, useful narrowing/subsumption, event or delta classes, repeated scans/publications, scheduling cost, and clone/recomputation effects for the ranked CRC, binary-Add, notification, and search hypotheses.
- [x] Each confirmed or rejected hypothesis is tied to an established Gecode implementation pattern and records its evidence, tradeoffs, and why advisors, staging, batching, local scratch state, cost changes, or caller-side elimination are or are not appropriate.
- [x] The brief records a small dependency-ordered follow-up split only for concrete candidates with a sound contract and a realistic benchmark gate; no instrumentation or benchmark artifact is tracked.

## Validation

- Build and run the five examples at quick defaults and bounded profiling configurations, using hard time bounds for retained-scale runs.
- Run focused existing Word Arithmetic, Logic, Structure, Conditional, Channel, and TestFramework filters as applicable to any temporary candidate comparison.
- Run zd check word and git diff --check, and confirm no temporary probe or raw result is tracked.

## Result

Completed and independently verified the native Word propagation and scheduling audit.

Validation:

- Exact Release baselines for all five realistic examples, bounded instrumentation and samples, candidate comparisons, focused Word suites, zd checks, and diff check passed; only two evidence-backed follow-ups were retained.
