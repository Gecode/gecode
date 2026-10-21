# TSPTW inputs and checking

`examples/tsptw.cpp` solves the travelling salesperson problem with time windows.
The closed constraint programming (CP) model constructs all route variables
before search. The transition model constructs visit variables as the route
grows. Both support depth-first branch and bound (BAB), A*, and complete anytime
beam search (CABS). The bundled data and independent Python checker exercise
their cost and feasibility rules.

## Input and tour formats

The native reader in `examples/tsptw-data.hh` and `misc/tsptw-check.py` accept
the canonical matrix format from the
[López-Ibáñez/Blum collection](https://lopez-ibanez.eu/tsptw-instances): a node
count including depot 0, an n-by-n matrix, then n ready/due pairs. Comments
begin with `#`. Whitespace separates values; exactly n*n + 2*n numeric values
must follow the node count.

An arc is a directed trip between two nodes. Its matrix value is both travel
time and objective cost. The objective includes the return arc. Waiting affects
feasibility but adds no cost. Service time is already included in outgoing arcs where the source data supplies it. Use the
supplied matrix without adding service again. Travel costs may depend on
direction, and indirect travel may be cheaper than a direct arc.

Departure is fixed at zero. Each visit waits until its ready time and must
begin by its due time. Return to depot 0 must also meet its window. The reader
rejects a nonzero depot ready time. A tour lists each customer ID 1 through n-1
once, omitting both depot visits. The checker reconstructs the earliest schedule
and compares an optional claimed cost exactly in original units. Waiting is
allowed, so if this earliest schedule misses a deadline, no later schedule for
the same route can meet it.

The reader accepts ordinary nonnegative decimals, without signs or exponents.
It scales all values by a common power of ten, ignoring trailing zeroes.
The squared node count, scaled values, scale, n*maximum-arc and
maximum-due + n*maximum-arc must fit `Gecode::Int::Limits::max`. The Python
reader checks these limits separately using exact rational arithmetic.

## Bundled fixtures

The five generated tiny instances have optima or infeasibility established by
enumerating every customer order:

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
The bundled published tours are feasible best-known tours; checking them does
not prove optimality.
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

The default `--formulation closed` uses a circuit constraint with domain
propagation to visit every node once and sum travel costs. Choosing an arc
activates its timing constraint.
Search first chooses arc costs, prioritizing a variable with a large gap between
its two largest remaining costs, then chooses successors to complete the route.
`--formulation transition` materializes visit variables as the route grows and
uses explicit waiting and depot return checks.
`--bound prefix` uses travel cost so far as a lower bound on the final cost.
`--bound min-arcs` also bounds the travel still needed, using the larger of
independent cheapest-incoming and cheapest-outgoing arc totals. These totals
need not form a tour, so they can underestimate the remaining cost.

`--residual-propagation off|cheap|unary` controls checks on the remaining visits.
`cheap` checks whether each customer can still be reached and followed by a
return before the deadlines, using shortest travel times. `unary` constructs a
temporary schedule in which visits cannot overlap and each visit occupies
its cheapest outgoing travel time. A failed schedule rules out the
prefix; propagation can also rule out customers as the next visit. The defaults
are `prefix` and `off`. Shortest paths may use already visited customers. This
keeps the checks safe when indirect travel is cheaper than a direct arc, though
it can make them weaker.

CABS enables dominance by default for `transition`. Among live
next-layer candidates with the same location and unvisited customers, an earlier
arrival with no greater travel cost so far can follow every remaining route
available to a later, no cheaper arrival. CABS can therefore discard the latter
candidate.
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
one. Any resource stop reports `stopped`; a reported tour remains feasible, but the
run has not proved optimality or infeasibility. Summaries also include native
search statistics and model settings.
