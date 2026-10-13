# Fixed-width beam-stack regeneration study

Fixed-width continuation can preserve coverage on the worked deterministic
TSPTW transitions and reduce retained Spaces when CABS must widen substantially.
The simpler original-Choice cursor achieved the same storage reduction with
less repetition in this study, while using a different search order. These
results support a separately reviewed, restricted follow-up; they do not justify
generic Space border replay or a production engine here.

The study used clean HEAD `b2bae0b94038ee0c2f723660d709315b114b7008` on
2026-10-07. All prototypes, inputs and records remain under
`/tmp/open-search-014-study`; production source was unchanged.

## Two continuation contracts

For original decision coverage, keep the prepared source Space unchanged and
retain its original Choice and cursor over alternatives `0..n-1`. Clone the
source, commit that recorded alternative, invalidate retained branchers, and
apply a newer incumbent to the child before propagation. Never mutate the
source, obtain a fresh Choice, and reuse an old index. Keep preparation state,
partial page selection, the next alternative and pending goals across pauses
and solution returns. Advance the cursor only after the child is retained,
processed or soundly pruned. Distinguish failure, branching, solution and
preparation suspension; inspect goals at every committed depth.

The native n-ary witness demonstrates the index problem: a cut removes action
0 from original actions `[0,1,2]`. Fresh alternative 1 now commits action 2;
original alternative 1 commits action 1, whose deeper branch contains a better
goal. The retained cursor covers goals `[0]`, `[1,11]`, `[2]`, encounters a failed
child `[1,10]`, and survives seven preparation/expansion pauses and solution
returns. Adopting a ceiling of 6 after the first cost-9 goal preserves both later
paths. This is a finite continuation witness, not a general engine.

Historical replay must preserve original Choice payloads in their original
commit order. It need not recover identical fixpoints, subsequent branchers or
heuristic values. Schulte and Tack distinguish solution-preserving path replay
from replaying newly chosen branch indices: non-monotonic propagation can change
the latter and lose solutions. Their completeness result allows extra or less
search after replay. [Weakly Monotonic Propagators, recomputation discussion and
Theorem 10](https://doi.org/10.1007/978-3-642-04244-7_56), local fulltext lines
195–252. The native Choice/prepare contracts are in `gecode/kernel/core.hpp`;
current CABS keeps its original parent/Choice cursor in `gecode/search/cabs.cpp`.

Border pagination requires more: the same successor identities and an immutable
strict total order on every regeneration. Chen and Beck retain the first omitted
successor and regenerate successors greater than or equal to that border. Their
implementation orders concrete states by bound and lexicographic state values,
with duplicate-state removal. Their frontier theorem uses the least admissible
bound of all unexpanded states with expanded parents; it does not require bounds
to increase along a path. [Linear-Memory Beam Search Algorithms in DIDP,
border definition, Theorem 3 and BSS appendix](https://doi.org/10.4230/LIPIcs.CP.2026.12),
local fulltext lines 244–279, 382–415 and 868–921.

The concrete experiment uses `examples/tsptw-transition.hh`, residual propagation
OFF, MIN_ARCS guidance, explicit depot return, and no state pruning. Its pre-cut
key is `(rank, bound, location, time, prefix_cost, returned, unvisited, complete
alternative path)`; here rank equals bound. Equal residual values remain separate
paths. New cuts affect committed children after key generation and never redefine
a stored page boundary. Older inherited cuts only remove permanently non-improving
paths. This restriction avoids changed residual inference or choice enumeration
during regeneration. A rank-first border with an unrelated rank cannot alone
certify the least frontier bound.

Original Choices preserve original decisions on arbitrary clone-related Spaces;
they do not freeze regenerated child bounds or ranks. The mutable-rank witness
shows why: after selecting A with rank 1 and retaining B's rank-2 border, changing
A to rank 3 and B to rank 0 makes the old boundary repeat A and skip B. Freezing
all edge keys could avoid that problem but adds storage. The cursor control
instead uses fixed original source/alternative order; it does not reproduce the
border experiment's bound ordering.

## Checked bounded results

Coverage runs disable incumbent pruning and compare every feasible tour identity
with independent rational-arithmetic enumeration in `misc/tsptw-check.py`. Both
representations and widths 1 and 2 agree on waiting (one feasible tour), decimal
(one), folded-service (two), and late-return (none). The only generated case has
six customers, unit off-diagonal arcs, customer windows `[0,100]`, and depot
return deadline 6. Every tour requires seven units. Both methods visit all 720
distinct failed path identities, checked against the permutation-derived
original alternative indices. This also exercises equal bounds and different
histories with equal residual values; the full path tie prevents their collapse.

Separate optimization runs reach checked optimum 9 on folded-service and prove
the generated case infeasible. External runs adopt a checked cost-13 tour during
a partially expanded prototype page, then publish cost 9 and exhaust. Native
CABS receives the same external tour at its safe entry boundary. Every reported
tour and improvement is checked. The native n-ary witness tests the cut-induced
choice shift explicitly. The prototype uses one-commit work slices, preserving
partial candidate scans; its native preparation-suspension witness is separate
from the TSPTW brancher, whose preparation completes immediately.

Generated-case optimization counts are identical for lazy and eager:

| Continuation | Commit attempts | Bound evaluations | Extra page scans / CABS restarts | Peak Spaces | Peak original Choices | Peak border keys | Retained path / observed key capacity, ints |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Border, B=1 | 3906 | 3186 | 719 scans | 7 | 6 | 5 | 21 / 36 |
| Border, B=2 | 3888 | 3168 | 359 scans | 12 | 11 | 5 | 42 / 48 |
| Cursor, B=1 | 1956 | 1236 | 0 | 7 | 6 | 0 | 21 / 0 |
| Cursor, B=2 | 1956 | 1236 | 0 | 12 | 11 | 0 | 42 / 0 |
| Native CABS, widths 1 through 1024 | 7211 | 5479 | 10 restarts | 722 | at most 1* | 0 | unavailable* |

Border commitment counts exceed cursor counts by 1950 and 1932, respectively.
CABS makes 5255 more commitments than the cursor and additionally clones its
restart root. CABS commitments are derived from native `node - (restart + 1)`;
all roots in these cases are initially branching. Its saved Choice ceiling is
source-derived from its single `choice` field, not direct Choice instrumentation.
CABS path/vector capacity is not exposed. Both prototypes keep only the selected
B children and the first omitted key, plus one in-flight child; they never keep
every generated child Space. The table includes metadata from partial scans.

The feasible control limits the recommendation: on folded-service, native CABS
peaks at 4 Spaces, border at 6/9 and cursor at 6/9 for B=1/2. CABS makes 4
commitments and one restart; border makes 6, cursor 4/6. All find cost 9. The
memory advantage depends on the search tree and how soon valid cuts close it.

## Memory and measurement limits

The counting subclass includes the seed/root, stored sources, incumbent,
in-flight children and caller-owned returned solutions in live Space counts.
Its allocated-position totals are sampled, not exact byte measurements.
Original Choice counts do not measure their action-vector payloads. Path/key
capacity counts exclude allocator/container overhead; retained path counts
exclude transient local path copies, while observed key counts include the
in-flight key. Full paths and clone-local visit lists grow with depth, so an
O(Bd) count of Spaces does not imply O(Bd) scalar storage or bytes.

| Generated-case process peak RSS | Lazy, bytes | Eager, bytes |
| --- | ---: | ---: |
| Border B=1 | 2260992 | 2228224 |
| Border B=2 | 2244608 | 2244608 |
| Cursor B=1 | 2260992 | 2228224 |
| Cursor B=2 | 2228224 | 2228224 |
| Native CABS | 4521984 | 4472832 |

A separate probe warms 16 clones, then retains 2048 root clones. Resident memory
increases by 5488640 bytes (lazy) and 5439488 (eager). Each lazy clone has one
materialized position; eager has seven. `sizeof(State)` is 488 in both. These
process deltas include native regions, vectors, the counting registry and
allocator effects; the instance is shared. They are bounded retention samples,
not exact Space sizes. The slight lazy/eager difference is noise at this scale.

There is one observation per configuration, on arm64 macOS with Apple clang 21
and the existing Release build. Startup and subprocess wait polling dominate
many tiny times. No speedup or workload-wide memory claim follows. The final
suite has 73 launches; preliminary runs and the added failed-path check bring
the cumulative implementation ledger to 148 launches, zero failed/timeout
launches, and **3.062307 seconds** actual launch-to-reap solver wall. Four
successful compilation attempts total **8.87 seconds**, counted separately.
Each solver launch has a five-second cap and the runner refuses launches that
could exceed its 300-second cumulative allowance. Independent replay has a
separate allowance of at most 300 seconds; study plus replay must stay below
600. `wall-ledger.json` and saved per-launch samples retain the totals.

## Reproduction and decision

The following commands require the temporary artifacts; they deliberately do
not install a production engine or commit prototype source. Run from the stated
checkout and use a fresh results directory for independent replay. Repeating
the same run resumes completed samples without launching them again. The
checker analyzes saved output without invoking any solver.

```sh
cd /Users/zayenz/.codex/worktrees/2d06/gecode
clang++ -std=c++17 -O2 -I build/open-search -I . /tmp/open-search-014-study/study.cpp -L build/open-search -Wl,-rpath,/Users/zayenz/.codex/worktrees/2d06/gecode/build/open-search -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -o /tmp/open-search-014-study/study
/tmp/open-search-014-study/run-study.sh /tmp/open-search-014-study/replay-results
env UV_CACHE_DIR=/tmp/open-search-014-study/uv-cache uv run --script /tmp/open-search-014-study/check.py /tmp/open-search-014-study/replay-results --repository /Users/zayenz/.codex/worktrees/2d06/gecode > /tmp/open-search-014-study/replay-checked.json
env UV_CACHE_DIR=/tmp/open-search-014-study/uv-cache uv run --script /tmp/open-search-014-study/benchmark.py analyze --name return-limit-7-border-opt-lazy-b1-v4 --results-root /tmp/open-search-014-study/replay-results
python3 misc/tsptw-check.py enumerate examples/data/tsptw/tiny/folded-service.txt
python3 misc/tsptw-check.py check examples/data/tsptw/tiny/folded-service.txt /tmp/open-search-014-study/folded-optimum.tour --cost 9
```

The final raw records are the names ending `-v4` in `results/`; the earlier
records remain only to preserve the cumulative wall ledger. `final-results.json`
contains the final checked rows, `checked-results.json` includes every attempt,
and `compile-1.log` through `compile-4.log` retain compiler timing. Compiler
deployment-target warnings were present; all recorded launches succeeded.

Defer arbitrary-Space heuristic border replay. A separately reviewed bounded
follow-up could implement fixed-width continuation for deterministic transitions
with a declared immutable path-distinguishing key and the existing prepare,
incumbent and ownership contracts. Compare it with the fixed-alternative cursor
before choosing border regeneration: the cursor needs fewer keys and fewer
commitments here, but sacrifices heuristic beam ordering. The study supplies
enough evidence for that restricted proposal; it does not select a public API,
authorize production adoption, or create a follow-up task.
