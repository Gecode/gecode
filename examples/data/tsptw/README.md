# TSPTW inputs and checking

The native reader in `examples/tsptw-data.hh` and the independent checker in
`misc/tsptw-check.py` use the canonical matrix format from the
[López-Ibáñez/Blum collection](https://lopez-ibanez.eu/tsptw-instances): a node
count including depot 0, an n-by-n matrix, then n ready/due pairs. Comments
begin with `#`. Rows are read as whitespace tokens; the reader requires exactly
n*n + 2*n numeric values after the node count.

Every supplied arc is both elapsed time and objective cost. The objective sums
all tour arcs, including the return arc. Waiting affects feasibility and never
adds objective cost. The canonical conversion includes service at the origin
in outgoing arcs where the original data supplies service. Do not add service
again or subtract it when comparing these inputs. Keep the supplied asymmetric
and nonmetric matrix; coordinates and reversed tours are not substitutes.

Departure is fixed at zero. Each customer visit waits until its ready time and
must begin no later than its due time. Return to depot 0 also observes its time
window. The reader rejects a nonzero depot ready time. Tours contain each
customer ID 1 through n-1 once, without either depot visit. The checker adds
the start and return itself, reports each arrival, wait and visit time, and
compares an optional claimed cost exactly in original units.

The native reader scales all matrix and window decimals by their common power
of ten, ignoring trailing zeroes. It accepts ordinary nonnegative decimal
notation, without exponents, signs or rounding. `Instance::scale` converts
native integer results back to original units. Dimensions, scaled values and
scale must fit `Gecode::Int::Limits::max`. The reader also requires the
conservative n*maximum-arc cost bound and maximum-due + n*maximum-arc time bound
to fit that limit. `cost_limit` and `time_limit` expose those checked bounds for
native variable domains and prefix arithmetic. The independent Python reader
uses rational numbers and checks the same 32-bit Gecode limits separately.

The five generated `tiny/` instances are separate from canonical benchmarks:

| Case | Semantic check | Exhaustive optimum |
| --- | --- | --- |
| waiting | waiting, asymmetric and nonmetric arcs | 5 |
| decimal | exact decimal arcs and waiting | 1.5 |
| folded-service | service already included in outgoing arcs | 9 |
| late-customer | every tour misses a customer due time | infeasible |
| late-return | customer visits can succeed but every return is late | infeasible |

The transition example also offers `--residual-propagation cheap` between OFF
and the existing unary relaxation. [The deadline-filter note](../../../docs/tsptw-cheap-deadline-filter.md)
explains its shortest-path checks and how to compare the inference modes.
The default remains OFF.

`published/` contains one unchanged canonical input and one published
travel-cost tour per selected family. The manifest records their exact costs,
source URLs and content hashes. These are feasible best-known tours; checking a
tour does not prove optimality. The Solomon sample has exact cost 444.5425;
the publisher's table displays 444.54. The validation compares display precision
separately from exact cost. Makespan tables are not travel-cost references.

`manifest.json` freezes 12 development and 24 held-out cases before tuning.
Each family contributes two development and five held-out canonical cases, plus
one window variant in each split. Selection varies sizes and named window
classes without using solver results. Source members and final content hashes
are disjoint across splits, including variant bases. The two deterministic
variants preserve every arc and the depot window: `customer-due-plus-100` adds
100 original time units to each customer due time; `customer-width-half`
replaces each customer due time with ready + (due-ready)/2. These variants are
marked separately and are not published canonical instances or published
best-known references. Tightening can make a variant infeasible.

Acquire the larger selection explicitly; it is not part of routine tests:

```sh
python3 misc/tsptw-acquire.py /tmp/tsptw-selected
```

The acquisition downloads the four pinned family archives, verifies archive
and member SHA-256 hashes, generates the marked variants, and checks their
final hashes and integer limits. It reads named members without extracting an
archive tree. To reuse downloaded archives offline, add `--archives DIRECTORY`.
The publisher may update an archive; a changed hash requires an explicit
selection update rather than silently changing the evaluation inputs.

Use an existing configured CMake build for the generated Gecode headers
(replace `build/open-search` below with that build directory):

```sh
c++ -std=c++17 -Ibuild/open-search -I. misc/tsptw-read.cpp -o /tmp/tsptw-read
python3 misc/tsptw-validate.py --reader /tmp/tsptw-read
python3 misc/tsptw-check.py check examples/data/tsptw/published/SolomonPotvinBengio/rc_201.1.txt examples/data/tsptw/published/SolomonPotvinBengio/rc_201.1.txt.tour --cost 444.5425
python3 misc/tsptw-check.py enumerate examples/data/tsptw/tiny/decimal.txt
```

The focused validation runs offline. It compares every native scaled value
with the independent reader, exhaustively enumerates tiny tours, checks all
four published tours, and rejects malformed data, integer overflow, duplicates,
omissions, late customer visits, late returns and incorrect objective costs.
To validate every acquired case as well, add `--selected /tmp/tsptw-selected`.
The checker exits with status 1 for invalid input or tours, and status 0 for
valid tours or completed enumeration. Enumeration is limited to nine customers.

## Native closed CP solver

Build `tsptw` in the same CMake tree. It links the integer and search libraries
and uses an ordinary `Space` with a domain-propagated costed circuit. Reified
successor constraints propagate customer times from a fixed departure at zero
to a separate depot return time. Shortest-path elapsed bounds preserve
asymmetric and nonmetric inputs. Cost-regret branching followed by successor
branching is identical for BAB, A* and CABS. Timing variables are not branched;
the solver reconstructs the earliest schedule for each complete route.

```sh
cmake --build build/open-search --target tsptw -j 8
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --engine bab
build/open-search/bin/tsptw examples/data/tsptw/published/SolomonPotvinBengio/rc_201.1.txt --engine cabs --seed examples/data/tsptw/published/SolomonPotvinBengio/rc_201.1.txt.tour --time-ms 5000
python3 misc/tsptw-validate.py --reader /tmp/tsptw-read --solver build/open-search/bin/tsptw
```

`--engine` accepts `bab` (the default), `astar` or `cabs`. A* and CABS use the
propagated total-cost minimum as their optimistic total bound. `--seed TOUR`
accepts the checker's whitespace customer IDs. The solver checks the seed's
earliest schedule and exact arc cost before fixing a cloned model to that
route and adopting it through the native engine's incumbent interface. It
publishes the seed before any improvements. The offline validator also checks
each seed with the independent Python checker before invoking the solver.

`--time-ms N` and `--node-limit N` use ordinary search stops. All limits are
nonnegative integers; omitting an ordinary limit disables it. Time is measured
from the start of instance reading and preprocessing. Stops are checked at
engine boundaries and do not interrupt propagation. CABS also accepts
`--max-width N` and `--frontier-limit N`; zero disables either cap. The frontier
cap counts pending nodes and does not limit process memory. Runs use one
search thread.

The executable emits JSON lines. Each `solution` record contains a customer
`tour`, `scaled_cost`, `scale`, exact original-unit `cost` and `return_time`
strings, and `elapsed_ms`. Costs strictly improve after the initial seed or
solution. The final `summary` record contains `engine`, `termination`, the last
`formulation`, `incumbent` (the same tour/cost/time fields, or `null`), `elapsed_ms`, and native
`statistics` (`node`, `fail`, `depth`, `propagate`, `restart`, `nogood`, `state_pruned`,
`state_descriptors`, `state_bytes`). Every
published route is checked without using the model's timing variables.

Natural exhaustion reports `optimal` when an incumbent exists, including an
unchanged seed, and `infeasible` when none exists. Any ordinary stop or CABS
resource stop reports `stopped` and supplies no proof. A first solution or
published best-known cost alone does not prove optimality. Invalid inputs or
seeds exit with status 1 and an error on stderr; successful complete and
stopped runs exit with status 0.

With `--solver`, the offline validator checks every native solution, proves the
tiny cases with all three engines and formulations, and exercises checked suboptimal/optimal
seeds, invalid seeds, nonmetric reachability and resource stops. It runs the
four canonical samples with independently checked published seeds and a
five-second limit per engine. It compares objectives when runs exhaust and
reports stopped canonical runs as incomplete. These are correctness checks;
they do not tune the frozen development or held-out selection.

## Native transition solver and eager control

`--formulation closed` is the default. `--formulation transition` uses the
ordinary native `Space` in `examples/tsptw-transition.hh`. Each stable node
holds its own unvisited customers, current location, time after waiting, prefix
travel cost, committed depth and incumbent ceiling. A shared immutable instance
supplies the arcs and windows. Applicable visits are ordered by ascending
customer ID and checked against their actual incoming arc and due time.
There is no direct-distance test for other residual customers, which would be
unsound for nonmetric matrices.

The visit list has at most n positions: n-1 customer visits and one explicit
depot return. Brancher preparation materializes the next position without
changing the concrete state. One n-ary choice commit assigns one action and
updates that state once. Archived choices store both position and customer
IDs; historical BAB replay can materialize an absent position before assigning
it. An unfinished state with no applicable action fails. The final return
checks the depot due time, adds the return arc exactly once and closes the list.
Output omits the return marker, so all formulations use the same tour format.

The default total bound is prefix travel cost g at unfinished nodes and full
tour cost after return. Nonnegative arcs make this an optimistic bound without
additional distance assumptions. Strict incumbent cuts are copied with each residual
state. A seed is checked independently, then constructed through the same
prepare/choice/commit journey before native incumbent adoption.

`--formulation eager` reserves all n path variables upfront. It uses the same
Space, brancher, applicability, ordering, incumbent cuts and search engines.
Only path allocation changes. The two transition representations emit identical
improving tours under the same engine; the closed model uses its own
cost-regret and successor branching.

```sh
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --formulation transition --engine astar
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --formulation eager --engine cabs
```

Transition summaries also contain `allocation`: `space_bytes` is the C++
`sizeof` the Space subclass, `intvar_handle_bytes` is `sizeof(IntVar)`, and
`root_positions`, `prepared_root_positions` and `solution_positions` count
materialized path variables before preparation, after preparing the root and
in the last incumbent. A failed root may have no prepared position; a run
without an incumbent reports zero solution positions. Lazy roots start at zero,
prepare one position, and grow with the committed prefix plus its next pending
decision. Eager roots start with n positions; both complete tours have n.
These are basic sizes and counts, excluding variable implementations, allocator
storage, list capacity, residual buffers and search frontier storage. They do
not measure process memory or imply a compact dynamic-programming state.


## Optional transition Pareto pruning

`--state-pruning off|pareto` defaults to `off`. Pareto pruning is explicitly
supported for native A*/CABS/column with `--formulation transition` or `eager`; closed
CP and BAB runs reject it. Both transition representations use the same
`Transition::pareto_pruning()` declaration:

```sh
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --formulation transition --engine astar --state-pruning pareto
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --formulation eager --engine cabs --state-pruning pareto
```

The exact key is current vertex i followed by the sorted unvisited customers U.
The label is `(t,g)`: time after waiting and prefix travel cost. Equal labels
are equivalent. At an equal key, one label dominates another only if both its
time and cost are no greater; earlier arrival at greater cost is incomparable.
Hashes select comparisons and never substitute for exact key equality.

This key is sufficient for the example's authored transition rules. The same
remaining arcs and customer/depot windows apply after equal `(U,i)`. Waiting
lets an earlier arrival follow every suffix feasible for a later arrival, and
a no-greater prefix cost preserves a no-greater final objective. Nonnegative
arcs preserve strict incumbent cuts. The ceiling is a monotone global cut, so
only globally improving completions matter. Committed depth follows from U;
the assigned path prefix serves output and adds no future restrictions. Lazy
and eager path variables use the same unrestricted position domains.

Optional residual inference rebuilds its schedule from immutable input and
`(U,i,t)` after each committed visit, including BAB replay. Clones copy only
the resulting allowed first actions. It carries no propagated domains from a
previous path. Every feasible suffix induces a feasible schedule, including
its earliest first visit, so the relaxation cannot remove that suffix. Earlier
arrival still preserves every feasible later suffix under the same windows.
The `(U,i)` key and `(t,g)` relation therefore remain sufficient with inference
enabled.

Arbitrary extra constraints on path history or prefix variables invalidate
that argument. Include their residual effects in a stronger exact key, or
return `std::nullopt` from extraction where sufficient state is unavailable.
The declaration does not justify pruning ordinary partial successor CP stores.
Returned tours remain independently owned and use the same checker.

A* and column store descriptor metadata after expansion and admit improved labels
again, including with inconsistent admissible bounds. CABS compares only live
next-layer candidates; beam-discarded states never become representatives.
See [A*](../../../docs/astar.md) and [CABS](../../../docs/cabs.md) for callback
contracts, optional metadata forgetting and candidate lifetimes. Forgetting
removes metadata alone and never removes search work or causes a stop.

Summary records identify `state_pruning`. `state_pruned` counts sound state
removals; `state_descriptors` and `state_bytes` record peak descriptor storage. The byte
estimate includes descriptor objects and owned key/label vector capacities,
excluding container capacity and allocator overhead. It does not include
Space/frontier storage or measure process RSS. These counters stay zero with
pruning off. Sound pruning is independent of beam loss and resource termination;
only natural complete exhaustion reports a proof.

The existing offline validator compares pruning off/on with exhaustive tiny
enumeration, includes duplicate and cost/time Pareto cases, and independently
checks every published solution. Its canonical runs remain capped at five
seconds each and do not form a development or held-out tuning campaign.

## Minimum-arc bounds and residual scheduling

`--bound prefix|min-arcs` and `--residual-propagation off|unary` are independent
switches for transition and eager formulations. Their defaults remain `prefix`
and `off`. Closed CP uses its propagated total objective minimum (`cp-total`
in summaries) and rejects stronger transition options. Bound evaluation adds
no separate beam ranking.

For nonempty U, `min-arcs` sums minimum outgoing arcs from i and every remaining
customer, and minimum incoming arcs to every remaining customer and the depot.
The current location can enter only U; a remaining customer can enter another
remaining customer or the depot. Incoming customer arcs can originate at i or
another remaining customer, and the depot receives an arc from U. Self arcs
and premature return are excluded. Every completion uses one arc per source
and target, so `g + max(incoming,outgoing)` is a valid total lower bound. The
root follows the same convention. With no customer left, the remaining bound
is the actual return arc; after return it is zero. All sums use checked signed
64-bit arithmetic.

`unary` creates a temporary native scheduling Space without branching. Customer
u has a duration equal to its minimum outgoing arc to U minus u or the depot.
Its start lies between `max(ready[u],t+dist[i,u])` and
`min(due[u],due[0]-dist[u,0],due[0]-duration[u])`. Shortest paths may use any
vertex, including unavailable vertices; this relaxes reachability and is sound
for asymmetric, nonmetric input. Preprocessing happens once per root and the
immutable matrix is shared by clones. Prefix/off roots skip this preprocessing.

Ordinary `status()` runs unary propagation to a fixpoint. Failure rejects the
residual state. A first action survives only if its actual earliest visit time
belongs to its propagated start domain. Each real residual tour yields
nonoverlapping task intervals: its outgoing arc is at least the chosen duration,
waiting can only separate intervals, and the last interval ends by depot due
time. This also covers zero arcs and service already folded into the matrix.
Empty residuals use the explicit return check; completed tours need no schedule.
Materializing a visit-list position still has no residual-state effects.

Transition `measurements` report `bound_calls`, `bound_ms`, `inference_calls`,
`inference_ms` and `propagation_ms`. One run-owned aggregate is shared by clones
and guidance callbacks, counting replay and repeated evaluations. Bound timing
covers each guidance bound calculation. Inference timing covers temporary
Space construction, posting, `status()` and start-domain inspection; propagation
timing covers `status()` alone. Shortest-path preprocessing is outside inference
timing and inside elapsed time. These example counters are separate from native
search statistics. Times are wall milliseconds, rounded to three decimals.

The existing `Search::TSPTW` test independently enumerates residual suffixes
and checks each bound, failure and removed first action. It includes waiting,
nonmetric arcs, unequal time/cost labels, tight scheduling failure/filtering,
zero durations, pending return and completed tours. The offline validator checks
all tiny optima with minimum-arc guidance with/without inference, one
prefix/unary independence case, and strengthened A*/CABS Pareto off/on cases.

## Progressive and fixed-quota column search

Both ordinary Space formulations support `--engine column`. Its default quota
starts at one and grows by one after every complete depth cycle. Set
`--quota-growth 0` for fixed-quota CBFS, or `--initial-quota N` for another
positive initial quota. Quotas schedule expansion; deferred work stays queued.
`--frontier-limit N` caps pending Space count, including an unfinished parent;
zero is unlimited. Cap termination reports `stopped` and supplies no proof.
`--max-width` remains specific to CABS. Column summaries record `initial_quota`,
`quota_growth`, and `frontier_limit`.

```sh
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --engine column
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --formulation transition --engine column --quota-growth 0 --state-pruning pareto
```

The formulations, exact tour checker, seed adoption and prefix/off defaults
remain the same. Transition/eager Pareto pruning supports column's fresh-label
registry; closed CP continues without that declaration. The offline validator
checks both quota modes against exhaustive tiny optima, independently checks
each published tour, and covers selected seed/Pareto/stop cases. Its canonical
loops and five-second limits remain unchanged. See [the native engine
contract](../../../docs/column.md) for full-cycle continuation, ownership,
frontier accounting and storage costs.

## Named comparison runs

`misc/tsptw-benchmark.py` collects one solver process at a time. Its eight default
configurations are closed BAB/CABS, transition BAB/A*/CABS/column and DIDPPy
A*/CABS. Development defaults to the six native configurations; `--configs`
selects a comma-separated subset, including reference solvers when needed.
The transition configurations use the existing prefix/off defaults. Four extra
closed CP tuning names require an explicit `--configs` selection and do not
enter development or held-out defaults. The runner adds no option cross-product
or tuning policy.

```sh
uv run --script misc/tsptw-benchmark.py preview --split held-out
uv run --script misc/tsptw-benchmark.py run --name correctness --split tiny
uv run --script misc/tsptw-benchmark.py run --name correctness --split tiny
uv run --script misc/tsptw-benchmark.py analyze --name correctness
```

Results go to `build/tsptw-results/NAME` unless `--results-root` selects another
location. Relative paths resolve from the command's working directory. Run only
one writer for a name. `config.json` records the full schedule, exact input
hashes, native binary and sibling macOS library hashes, script/checker hashes,
DIDPPy pin, machine and limits. A changed identity requires a new name. Keep the
Python/uv environment and any externally installed native libraries fixed;
the runner hashes sibling build libraries on macOS, not every transitive system
dependency. Completed JSON rows and their stdout/stderr remain unchanged on
resume. Missing, malformed and interrupted rows execute again; partial streams
are renamed before retry. Crashes, timeout, RSS termination and invalid output
are completed failures and remain in the run. Use a new name for deliberate
retries. `--max-samples N` bounds a smoke invocation without changing its saved
experiment. Each row becomes complete by atomic JSON replacement after checking.

For acquired development or held-out inputs, pass the directory produced by
`misc/tsptw-acquire.py` with `--selected-root`. Preview reads the frozen
manifest without inspecting held-out outcomes or requiring acquired files.
The development ceiling is six configurations, 12 cases, two repeats and
15 seconds: 2,160 seconds (36 minutes). The full held-out comparison is eight
configurations, 24 cases, one repeat and 30 seconds: 5,760 seconds (96 minutes).
Development tuning has its own 45-minute allowance. The separate 120-minute
evaluation allowance includes its bounded development ablations and compilation,
so the held-out matrix leaves at most 24 minutes for that work. Full tuning plus
held-out takes 132 minutes before compilation across the two separate budgets;
that total is allowed. Preview prints both budgets and their scopes; it does not
launch a campaign or enforce a budget shared across named runs.

Both solver routes receive one thread, the same launch-to-reap wall deadline
and a 1 GiB sampled RSS watchdog. `ps` reports the summed RSS of the owned process
group, including uv and its Python child. Sampling happens every nominal 50 ms
plus probe overhead; records include sample count, maximum gap and total probe
time. Termination between samples can miss the peak; no observed live RSS is
`null`. The watchdog can overshoot and is not a hard memory limit. Probe failure
terminates the sample as a measurement failure. Stops kill and reap the process
group and cannot establish a proof.

End-to-end wall time includes launch, imports, preprocessing, search, target
incumbent checks, output, watchdog overhead and teardown. The outer independent
Fraction checks run after that measurement and check every emitted incumbent.
Native `preprocessing_ms` covers instance reading, root posting/propagation,
preparation and guidance setup. Native `search_ms` starts at engine construction
and ends at exhaustion/stop, including the target's own checks and output.
DIDPPy's preprocessing includes interpreter-side imports, instance reading and
model construction; its search starts before solver construction and includes
that construction, its own incumbent checks and output.
`startup_and_teardown_ms` is the residual of outer wall time minus target
elapsed time, including sampling/detection delay; it is not an isolated startup
measurement. Missing final summaries leave phase and search metrics unavailable.
Build before capture. `--compilation-seconds S` records a separately measured
build duration; omission explicitly leaves it unavailable. Compilation is never
silently attributed to search.

Each row records checked incumbent times and costs, the summary, final objective,
proof or stop reason, expansions and sampled peak RSS. Native descriptor peaks,
bound/inference timings and basic allocation sizes remain available in the
summary. Frontier bytes and actual clone footprint are unavailable. The native
`sizeof`/position counters exclude buffers, allocator overhead and frontier
storage. DIDPPy does not expose equivalent descriptor/clone counters here.
`analyze` reads only saved config and rows, writes `analysis.json`, and reports
expected/completed/unfinished/failed counts and valid-run wall-time medians by
configuration. With `closed-bab` present, it also pairs every other configuration
with that baseline on the same case and repetition. Pairs record status, checked
final objective, first-incumbent time, quality at 100/1,000/5,000/15,000 reported
milliseconds when within the allowance, proof/time, termination, outer wall time
and sampled peak RSS. Counts distinguish wins, ties, losses, missing/failed pairs
and unavailable metrics. Cases mark canonical data and deterministic window
variants separately. Checked incumbents from timeout or RSS termination count
as feasible quality; such termination never counts as proof. Native reported
clocks start at main entry and include preprocessing; outer wall clocks measure
launch through reap. A shorter wall time after a resource stop need not mean
faster successful solving. Analysis never launches solvers; detailed objectives,
proofs and traces remain in the raw rows. Timing medians across different cases
are descriptive and do not rank solvers or estimate variation.

`misc/tsptw-didppy.py` pins DIDPPy 0.11.1 and reports its installed version.
A* uses its sequential CAASDy solver; CABS uses its sequential CABS solver.
The formulation follows the pinned
[official TSPTW example](https://github.com/domain-independent-dp/didp-rs/blob/59a5330537962bcc2f93c3914cf5fd9ee361b23d/didppy/examples/tsptw.ipynb),
but checks each actual arrival and explicit depot return. It omits the example's
direct-arc residual deadline constraint, which is unsound for nonmetric input.
The incoming/outgoing bounds sum global minimum arcs for the remaining mandatory
sources/targets and take their maximum through DIDPPy's dual-bound interface.
They relax restrictions on the remaining vertices and remain lower bounds for asymmetric,
nonmetric matrices. Time is a less-is-better resource; DIDPPy's cost-aware
registry compares labels for the same unvisited set and location. The exact
scaled arcs, waiting, fixed start zero, folded service, travel-cost objective
and return window match the native model and Fraction checker.

## Conventional CP options

The following closed CP configurations use the same costed
circuit, shortest-path timing bounds, reified arc timing, objective, successor
completion branch and independent checker. Circuit propagation accepts
`IPL_DOM` or `IPL_VAL`. Cost-variable branching accepts
`INT_VAR_REGRET_MAX_MAX()` or `INT_VAR_SIZE_MIN()`; both use `INT_VAL_MIN()`.
The successor branch uses size/min.

| Runner name | Circuit propagation | Cost branching | Search |
| --- | --- | --- | --- |
| `closed-bab` | DOM | regret | BAB |
| `closed-val-bab` | VAL | regret | BAB |
| `closed-size-bab` | DOM | size | BAB |
| `closed-rbs-regret` | DOM | regret | RBS of BAB |
| `closed-rbs-size` | DOM | size | RBS of BAB |

Native `--circuit-propagation dom|val` and `--cost-branching regret|size` apply
only to `--formulation closed`; other formulations reject explicit settings.
The defaults remain DOM/regret. The runner names `closed-dom-size-bab` and
`closed-dom-size-cabs` explicitly select DOM/size with BAB and CABS.
`--engine rbs` is also closed-only and uses the existing `RBS<TSPTWClosed,BAB>`
builder, one thread, Luby cutoff scale 1,000 and
no-good depth 128. These restart parameters are fixed. Summary records report
the chosen model and restart settings. The default `Space::master` posts
incumbent cuts and no-goods; the default `Space::slave` marks each BAB search
complete. Only natural exhaustion reports optimality or infeasibility. A
resource stop supplies no proof.

Select model settings explicitly when comparing engines or formulations:

```sh
build/open-search/bin/tsptw INSTANCE --formulation closed --engine bab --circuit-propagation dom --cost-branching size --time-ms 15000
# Restart recipes use fixed Luby 1000 and no-good depth 128:
build/open-search/bin/tsptw INSTANCE --formulation closed --engine rbs --circuit-propagation dom --cost-branching regret --time-ms 15000
build/open-search/bin/tsptw INSTANCE --formulation closed --engine rbs --circuit-propagation dom --cost-branching size --time-ms 15000
```

These commands use one thread and a native time stop. To compare the five
configurations with the runner's outer deadline and sampled RSS watchdog:

```sh
uv run --script misc/tsptw-benchmark.py preview --split development --configs closed-bab,closed-val-bab,closed-size-bab,closed-rbs-regret,closed-rbs-size --selected-root /tmp/tsptw-selected --solver build/open-search/bin/tsptw --repetitions 2 --seconds 15
uv run --script misc/tsptw-benchmark.py run --name cp-options --split development --configs closed-bab,closed-val-bab,closed-size-bab,closed-rbs-regret,closed-rbs-size --selected-root /tmp/tsptw-selected --solver build/open-search/bin/tsptw --repetitions 2 --seconds 15
uv run --script misc/tsptw-benchmark.py analyze --name cp-options --output build/tsptw-results/cp-options/comparison.json
```
