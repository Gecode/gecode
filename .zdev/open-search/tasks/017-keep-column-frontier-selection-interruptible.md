+++
schema_version = 1
id = "open-search-017"
key = "interrupt-column-selection"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = []
+++
# Keep column frontier selection interruptible

## Outcome

Column honors ordinary stops and worker parking while discarding obsolete frontier entries, and resumes with its search position intact.

## Context

Review at f4523c04be found that gecode/search/column.cpp:187–218 drains bound-pruned entries inside select() without consulting Worker::stop; next() checks only before entering it. A newly adopted incumbent can invalidate a large frontier. A*/CABS already return through a stop check between discarded entries. Read the existing Column lifecycle fixtures in test/search.cpp and the worker-control adapter before changing selection.

## Boundaries

- Preserve cycle depth, quota accounting, original Choice coverage and natural-exhaustion semantics; this is an interruption fix, not a different column algorithm.

## Done when

- [x] Selection yields to an ordinary failure/time stop or worker-control request during a stale-frontier drain.
- [x] Resuming preserves the remaining queue and reaches the same optimum or exhaustion as uninterrupted search.

## Validation

- Add one deterministic existing-fixture regression that queues several nodes, makes them obsolete with an incumbent, stops after a small number of discarded nodes and resumes. Run the focused Column and existing parked-portfolio checks; no new harness.

## Result

Column polls the existing stop hook during frontier selection, preserving queued work and search position across interruption.

Validation:

- Build and focused Search::Column and Search::Portfolio::PublicContract checks passed; the new stale-frontier regression failed on the original code and passes with the fix. Independent verification passed and the reviewed context remained unchanged.
