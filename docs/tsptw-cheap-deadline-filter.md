# Cheap residual deadlines for TSPTW

The native transition example accepts `--residual-propagation off|cheap|unary`.
`off` remains the default. `cheap` checks necessary customer and depot deadlines
after each committed visit, without constructing a residual unary Space.
`unary` retains its existing scheduling relaxation and Gecode fixpoint policy.

For current location `i`, time `t`, and each unvisited customer `u`, let
`d[i,u]` be the shortest path length in the supplied matrix. The cheap check uses

```text
L = max(ready[u], t + d[i,u])
reject if L > due[u] or L + d[u,0] > due[0]
```

Any completion must reach `u` no earlier than `L` and return to the depot no
earlier than `L + d[u,0]`. The matrix contains nonnegative elapsed times, including
any folded service. Shortest paths may traverse already visited or unavailable
vertices; allowing those paths weakens the test and preserves its necessity.
This reasoning does not require symmetry or the triangle inequality. With no
customers left, the check uses the actual direct return arc. All additions in
the customer check use the existing checked cost helper.

The customer test follows equation 28 of [Marijnissen et al., *Domain-Independent
Dynamic Programming with Constraint Propagation*](https://doi.org/10.1609/icaps.v36i1.42826).
The depot test adds the corresponding necessary condition for this example's
explicit return window. Waiting affects arrival time and contributes no cost.
Bounds, ranking, Pareto pruning and representation remain separate choices.

The shortest-path matrix is computed once per root and shared by clones.
Each cheap refresh scans the remaining customers. Its work contributes to
`inference_calls` and `inference_ms`; `propagation_ms` remains zero. These
counters include repeated work during BAB replay. Native BAB does not consume
the separate heuristic bound callback.

## Fixed comparison

The [saved comparison](../examples/data/tsptw/cheap-deadline-comparison.json)
records four development cases and two tiny infeasible controls selected before
any solver launch. It compares OFF, CHEAP and UNARY with native BAB and CABS:

| Case | Role |
| --- | --- |
| `Dumas/n20w20.001.txt` | canonical narrow windows |
| `variants/Dumas/customer-due-plus-100/n20w100.003.txt` | separately marked looser customer windows |
| `GendreauDumasExtended/n20w120.001.txt` | canonical wider windows |
| `SolomonPotvinBengio/rc_201.1.txt` | canonical Solomon family |
| `tiny/late-customer.txt` | infeasible customer deadline |
| `tiny/late-return.txt` | infeasible depot return |

Each configuration uses the same lazy transition formulation, prefix bound,
default ranking, Pareto OFF, no seed, one thread and a two-second outer cap.
Two repetitions reverse configuration order on the second pass: 72 rows and
144 nominal solver seconds. Every published incumbent passes the independent
Fraction checker. Natural exhaustion alone supplies a proof. The runner checks
conflicting optimality and infeasibility claims across each case.

The sampled RSS watchdog sums the process group, limits it to 1 GiB and requests
samples every 50 ms. Probe and cleanup time belongs to launch-through-reap wall
time. Sampling can miss peaks and overshoot limits. Reported publication clocks
begin at native main entry; launch-inclusive publication times are unavailable.
Missing post-kill summaries leave inference time, nodes and other phase metrics
unavailable. Propagation time is part of inference time, not an additive phase.
The runner does not measure frontier or clone buffer bytes.

## Observed results and decision

All 72 rows completed. The checker accepted 476 terminal-row publications and
three more publications in retained interrupted streams. There were 23 outer
timeouts, no RSS stops and no terminal execution, measurement or invalid-output
failures. Forty-nine rows have summaries; the other 23 have unavailable phase
times and node counts. Each configuration has 12 observations, including four
tiny infeasibility proofs:

| Configuration | Checked tour | Optimal | Infeasible | Ordinary stop | Outer timeout |
| --- | ---: | ---: | ---: | ---: | ---: |
| BAB OFF | 4 | 2 | 4 | 3 | 3 |
| BAB CHEAP | 8 | 4 | 4 | 1 | 3 |
| BAB UNARY | 8 | 2 | 4 | 4 | 2 |
| CABS OFF | 8 | 0 | 4 | 1 | 7 |
| CABS CHEAP | 8 | 4 | 4 | 0 | 4 |
| CABS UNARY | 8 | 2 | 4 | 2 | 4 |

Both repetitions agree on these useful differences:

- On narrow-window Dumas, CHEAP and UNARY prove cost 378 with either engine.
  BAB visits 1,079 nodes with CHEAP, 1,014 with UNARY and 2,785,283 with OFF.
  CHEAP BAB search takes 0.882–0.886 ms, including 0.055 ms of inference;
  UNARY takes 7.626–7.797 ms, including 6.873–6.924 ms of inference.
  Launch wall times are 93–100 ms for both inference modes, where process startup
  and observation dominate the small native search. OFF CABS finds 378 but
  supplies no proof within the cap.
- On the looser Dumas variant, BAB CHEAP ends at 310 and BAB UNARY at 337;
  OFF finds no tour. Every CABS mode ends at 249. No mode proves this case.
- On Gendreau, BAB CHEAP ends at 302 and BAB OFF/UNARY at 305. Every CABS mode
  ends at 267, with no proof. The available second-repetition BAB summary records
  3,268,871 CHEAP nodes and 125.113 ms of inference, compared with 627,236 UNARY
  nodes and 1,576.976 ms of inference. Fewer nodes do not establish a runtime gain.
- On Solomon, CHEAP proves exact original cost 444.5425 with both engines.
  BAB proof wall times are 0.562–0.580 s and CABS proof wall times 1.121–1.123 s.
  BAB UNARY ends at 447.6347 then 444.5425; CABS OFF and UNARY reach 444.5425
  without proof. BAB OFF finds no tour. CHEAP BAB inference costs 34.910–35.054 ms;
  UNARY's stopped summaries record 1,545.946–1,548.630 ms.

Retain CHEAP as an explicit option. It removes enough deadline dead ends to
improve this small comparison while avoiding full unary inference. These four
development cases and two easy controls do not establish a universal speedup
or settle defaults. The variant and Gendreau cases remain unproved.

The available two-second UNARY summaries spend roughly 77–79% of search time
in inference. This supports a separately scoped incremental-unary study to ask
whether reusable residual state can preserve its stronger pruning at lower
cost. This comparison adds no RSS-stop evidence for a fixed-width continuation
study: its largest sampled CABS peaks are about 283 MiB OFF, 97 MiB CHEAP and
77 MiB UNARY, and frontier bytes are unmeasured. Any continuation study should
retain its separate justification and budget. No such study is implemented here.

The 72 terminal rows use 77.537 s of launch-through-reap wall time. Two capture
cleanup calls failed with a permission error; inspection found both children
absent. The ledger charges 39.635 s and 20.234 s conservatively through absence
confirmation, including their observed 2.008 s and 2.006 s attempts. Their partial
streams remain available. Validation includes two retained short-cap timeouts
before successful retries of the required integration checks.

At implementation handoff, shared usage is **170.957548 s**: correctness
33.551175 s and development 137.406373 s, with no pending launches. Of the
300-second allowance, 129.042452 s remains, including the unused 60-second
verification reserve and 20-second cleanup reserve. Observed capture periods
total 115.103497 s; the remaining 55.854051 s is conservative absence-confirmation
accounting. Builds took 2.111298 s and
10.162398 s separately, totalling 12.273696 s. Independent verification will
append to this ledger without changing this implementation snapshot.

## Reproduction

Use one writer for the named run. The task ledger begins with this distinct
header, created only when the file does not already exist:

```json
{"event":"budget","scope":"task","task_id":"open-search-028","total_seconds":300,"verification_reserve_seconds":60,"cleanup_reserve_seconds":20}
```

The shared allowance counts solver validation, comparison attempts, interrupted
launches and cleanup. Builds are measured separately. Admission preserves
unused verification and cleanup reserves and rejects pending child cleanup.
Task ledgers exclude compilation and held-out collection. Historical
7,200-second evaluation ledgers keep their original accounting and reserves.

The exact preview, collection and analysis commands are:

```sh
python3 misc/tsptw-benchmark.py preview --split development --selected-root /private/tmp/open-search-tsptw-selected-network --case-ids Dumas/n20w20.001.txt,variants/Dumas/customer-due-plus-100/n20w100.003.txt,GendreauDumasExtended/n20w120.001.txt,SolomonPotvinBengio/rc_201.1.txt,tiny/late-customer.txt,tiny/late-return.txt --configs transition-bab,transition-bab-lazy-cheap,transition-bab-lazy-unary,transition-cabs,transition-cabs-lazy-cheap,transition-cabs-lazy-unary --repetitions 2 --seconds 2 --budget-ledger /private/tmp/open-search-028-budget.jsonl --budget-phase development --compilation-seconds 12.273696000047494
python3 misc/tsptw-benchmark.py run --name cheap-deadline-028 --split development --selected-root /private/tmp/open-search-tsptw-selected-network --case-ids Dumas/n20w20.001.txt,variants/Dumas/customer-due-plus-100/n20w100.003.txt,GendreauDumasExtended/n20w120.001.txt,SolomonPotvinBengio/rc_201.1.txt,tiny/late-customer.txt,tiny/late-return.txt --configs transition-bab,transition-bab-lazy-cheap,transition-bab-lazy-unary,transition-cabs,transition-cabs-lazy-cheap,transition-cabs-lazy-unary --repetitions 2 --seconds 2 --budget-ledger /private/tmp/open-search-028-budget.jsonl --budget-phase development --compilation-seconds 12.273696000047494
python3 misc/tsptw-benchmark.py analyze --name cheap-deadline-028 --output examples/data/tsptw/cheap-deadline-comparison.json
```

Repeating collection skips validated terminal rows, including failures. Changing
the cap rejects reuse before launching. Interrupted streams remain beside a
replacement sample. After inspecting a pending child, the existing `recover`
command confirms absence and charges conservatively through that confirmation:

```sh
python3 misc/tsptw-benchmark.py recover --name cheap-deadline-028
```

Raw configuration, terminal records and streams remain in
`build/tsptw-results/cheap-deadline-028`. Input identities and hashes are frozen
there and in the compact report. Analysis reads saved records and launches no
solver. The report's budget snapshot covers implementation; independent
verification appends to the same external ledger, and the task result records
the final shared total.

## Historical correctness checks

The existing `Search::TSPTW::Transition` test independently enumerates numeric
suffixes and checks CHEAP and UNARY against every feasible completion, including
failed committed residuals. It covers waiting, zero durations, pending and
completed return, and replay for lazy and eager paths. A nonmetric fixture has
customer 2 due at 6 and the depot due at 8: direct `0->2` and the return `1->0`
after waiting miss deadlines, but route `1,2` returns at 8 with cost 5. Both
inference modes preserve it. CHEAP also rejects infeasible residuals.

The frozen report records checks from the earlier combined branch. Its tiny
CLI checks with BAB and CABS proved waiting cost 5 and both late fixtures
infeasible with `residual_propagation=cheap` and `propagation_ms=0`. The report's
build paths and timings describe that run, rather than current native branch
validation. The standalone branch retains the same suffix tests and CLI checker.
