# TSPTW inputs and checking

`examples/tsptw.cpp` solves the travelling salesperson problem with time windows.
It has a conventional closed CP model and a lazy transition model, with BAB, A*
and CABS search. The bundled data and independent Python checker exercise their
cost and feasibility rules.

## Input and tour formats

The native reader in `examples/tsptw-data.hh` and `misc/tsptw-check.py` accept
the canonical matrix format from the
[López-Ibáñez/Blum collection](https://lopez-ibanez.eu/tsptw-instances): a node
count including depot 0, an n-by-n matrix, then n ready/due pairs. Comments
begin with `#`. Whitespace separates values; exactly n*n + 2*n numeric values
must follow the node count.

Each matrix arc is both travel time and objective cost. The objective includes
the return arc. Waiting affects feasibility but adds no cost. Service time is
already folded into outgoing arcs where the source data supplies it. Keep the
supplied asymmetric, nonmetric matrix without adding service again.

Departure is fixed at zero. Each visit waits until its ready time and must
begin by its due time. Return to depot 0 must also meet its window. The reader
rejects a nonzero depot ready time. A tour lists each customer ID 1 through n-1
once, omitting both depot visits. The checker reconstructs the earliest schedule
and compares an optional claimed cost exactly in original units.

The reader accepts ordinary nonnegative decimals, without signs or exponents.
It scales all values by a common power of ten, ignoring trailing zeroes.
Dimensions, scaled values, scale, n*maximum-arc and maximum-due + n*maximum-arc
must fit `Gecode::Int::Limits::max`. The Python reader checks these limits
separately using exact rational arithmetic.

## Bundled fixtures

The five generated tiny instances have optima established by enumeration:

| Case | Check | Optimum |
| --- | --- | --- |
| waiting | waiting, asymmetric and nonmetric arcs | 5 |
| decimal | exact decimal arcs and waiting | 1.5 |
| folded-service | service included in outgoing arcs | 9 |
| late-customer | every tour misses a customer due time | infeasible |
| late-return | every return is late | infeasible |

`published/` contains an unchanged canonical input and a published travel-cost
tour from each of the Dumas, GendreauDumasExtended, SolomonPotvinBengio and AFG
families in the López-Ibáñez/Blum collection. `manifest.json` records the source
archive and tour-table URLs, bundled file hashes and exact tour costs.
These are feasible best-known tours; checking them does not prove optimality.
The Solomon tour costs exactly 444.5425, while the published table displays
444.54. Makespan tables are not travel-cost references.

## Running and checking

Enable this example with `GECODE_ENABLE_EXPERIMENTAL=ON` when configuring Gecode.
Replace `build/open-search` below with that build directory.

```sh
cmake --build build/open-search --target tsptw -j 8
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --engine bab
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --formulation transition --engine astar
build/open-search/bin/tsptw examples/data/tsptw/tiny/waiting.txt --formulation transition --engine cabs
uv run --script misc/tsptw-check.py enumerate examples/data/tsptw/tiny/decimal.txt
uv run --script misc/tsptw-check.py check examples/data/tsptw/published/SolomonPotvinBengio/rc_201.1.txt examples/data/tsptw/published/SolomonPotvinBengio/rc_201.1.txt.tour --cost 444.5425
c++ -std=c++17 -Ibuild/open-search -I. misc/tsptw-read.cpp -o /tmp/tsptw-read
uv run --script misc/tsptw-validate.py --reader /tmp/tsptw-read --solver build/open-search/bin/tsptw
```

Validation runs offline. It compares native scaled values against the independent
reader, enumerates tiny optima, checks published tours, and rejects malformed
inputs and invalid tours. With `--solver`, it checks the native formulations
and engines against tiny optima, nonmetric cases, bounds, seeds and resource
stops. Omit `--solver` to check data and readers alone. The checker exits with
status 1 for invalid data or tours. Enumeration is limited to nine customers.

The default `--formulation closed` uses a costed circuit with domain propagation,
reified timing constraints and maximum-regret branching on arc costs, followed
by successor branching. `--formulation transition` materializes visit variables
as the route grows and uses explicit waiting and depot return checks.
`--bound prefix|min-arcs` selects the transition lower bound, and
`--residual-propagation off|cheap|unary` selects no
residual filtering, shortest-path deadline checks, or a temporary unary
scheduling relaxation. Defaults are `prefix` and `off`. Shortest-path bounds
preserve soundness for nonmetric arcs.

CABS enables dominance by default for `transition`. Among live
next-layer candidates with the same location and unvisited customers, an earlier
arrival with no greater prefix cost subsumes a later, no cheaper arrival.
Use `--dominance off` to compare runs without this pruning.

`--seed TOUR` checks the seed before adopting it and emits it before any
improvements. `--time-ms N` and `--node-limit N` use ordinary search stops;
time includes reading and preprocessing. CABS also accepts `--max-width N` and
`--frontier-limit N`, where zero disables the cap. Limits are checked at engine
boundaries and cannot interrupt propagation. Runs default to one search thread;
use `--threads N` to change the thread count.

The executable emits JSON lines: improving `solution` records followed by a
`summary`. Each tour has exact cost and return time, scaled cost and scale.
Natural exhaustion reports `optimal` with an incumbent or `infeasible` without
one. Any resource stop reports `stopped` and supplies no proof. Summaries also include native search statistics and model settings.
