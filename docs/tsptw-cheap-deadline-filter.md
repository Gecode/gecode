# Cheap residual deadlines for TSPTW

The native transition example accepts `--residual-propagation off|cheap|unary`.
`off` remains the default. `cheap` checks necessary customer and depot deadlines
after each committed visit, without constructing a residual unary Space.
`unary` uses its scheduling relaxation and Gecode fixpoint policy.

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
Bounds, ranking and representation remain separate choices.

The shortest-path matrix is computed once per root and shared by clones.
Each cheap refresh scans the remaining customers. Its work contributes to
`inference_calls` and `inference_ms`; `propagation_ms` remains zero. These
counters include repeated work during BAB replay. Native BAB does not consume
the separate heuristic bound callback.

## Comparison commands

Compare OFF, CHEAP and UNARY with native BAB and CABS on selected development
cases and tiny infeasible controls. The configurations use the same lazy
transition formulation, prefix bound, default ranking, no seed and
one thread. Preview the schedule before collecting results:

```sh
python3 misc/tsptw-benchmark.py preview --split development --selected-root /tmp/tsptw-selected --case-ids Dumas/n20w20.001.txt,variants/Dumas/customer-due-plus-100/n20w100.003.txt,GendreauDumasExtended/n20w120.001.txt,SolomonPotvinBengio/rc_201.1.txt,tiny/late-customer.txt,tiny/late-return.txt --configs transition-bab,transition-bab-lazy-cheap,transition-bab-lazy-unary,transition-cabs,transition-cabs-lazy-cheap,transition-cabs-lazy-unary --repetitions 2 --seconds 2
python3 misc/tsptw-benchmark.py run --name cheap-deadlines --split development --selected-root /tmp/tsptw-selected --case-ids Dumas/n20w20.001.txt,variants/Dumas/customer-due-plus-100/n20w100.003.txt,GendreauDumasExtended/n20w120.001.txt,SolomonPotvinBengio/rc_201.1.txt,tiny/late-customer.txt,tiny/late-return.txt --configs transition-bab,transition-bab-lazy-cheap,transition-bab-lazy-unary,transition-cabs,transition-cabs-lazy-cheap,transition-cabs-lazy-unary --repetitions 2 --seconds 2
python3 misc/tsptw-benchmark.py analyze --name cheap-deadlines --output build/tsptw-results/cheap-deadlines/comparison.json
```

Use one writer for the named run. Repeating collection skips validated terminal
rows, including failures. Changing the cap rejects reuse before launching.
Interrupted streams remain beside a replacement sample. After inspecting a
pending child, `recover` confirms absence and charges conservatively through
that confirmation:

```sh
python3 misc/tsptw-benchmark.py recover --name cheap-deadlines
```

Raw configuration, terminal records and streams go to
`build/tsptw-results/cheap-deadlines`. Input identities and hashes are frozen
there and in the compact report. Analysis reads saved records and launches no
solver. See [the input and runner documentation](../examples/data/tsptw/README.md)
for acquisition, independent checking, resource limits and timing conventions.

An optional `--budget-ledger FILE` shares a preinitialized allowance across
validation and comparison attempts, including interrupted launches and cleanup.
`--budget-phase` selects the charged phase. Task-scoped ledgers exclude
compilation and held-out collection; report build time separately with
`--compilation-seconds`. Admission preserves unused verification and cleanup
reserves and rejects pending child cleanup. Evaluation ledgers use their own
accounting and reserves.

## Correctness checks

The existing `Search::TSPTW::Transition` test independently enumerates numeric
suffixes and checks CHEAP and UNARY against every feasible completion, including
failed committed residuals. It covers waiting, zero durations, pending and
completed return, and replay for lazy and eager paths. A nonmetric fixture has
customer 2 due at 6 and the depot due at 8: direct `0->2` and the return `1->0`
after waiting miss deadlines, but route `1,2` returns at 8 with cost 5. Both
inference modes preserve it. CHEAP also rejects infeasible residuals.
