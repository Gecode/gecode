# TSPTW example

`examples/tsptw.cpp` solves the travelling salesperson problem with time windows.
The small `waiting.txt` instance has two customers. Its optimal route is
`0 -> 1 -> 2 -> 0`, with travel cost 5. It reaches customer 1 at time 2,
waits until time 5, and returns to the depot at time 8.

## Running

Enable `GECODE_ENABLE_EXPERIMENTAL=ON` when configuring Gecode. Replace
`build/open-search` below with that build directory.

```sh
cmake --build build/open-search --target tsptw -j 8
build/open-search/bin/tsptw examples/data/tsptw/waiting.txt --engine bab
build/open-search/bin/tsptw examples/data/tsptw/waiting.txt --formulation transition --engine astar
build/open-search/bin/tsptw examples/data/tsptw/waiting.txt --formulation transition --engine cabs
```

Both formulations support depth-first branch and bound (`bab`), A*, and complete
anytime beam search (`cabs`). The executable emits JSON lines: improving
`solution` records followed by a `summary`. Each tour lists every customer once,
omitting the depot at both ends. Costs and return times are exact decimals;
records also include the integer cost and scale used by the model.

Exhausting search reports `optimal` with a solution or `infeasible` without one.
A resource limit reports `stopped`; any reported solution remains feasible, but
the run has not proved optimality or infeasibility.

## Input format

`examples/tsptw-data.hh` reads a node count `n` including depot 0, an `n` by `n`
matrix in row order, then `n` ready/due time pairs in node order. Whitespace
separates values and `#` starts a comment. Exactly `n*n + 2*n` values must follow
the node count.

Each matrix entry supplies both travel time and cost for a directed trip.
The objective includes the return trip to depot 0. Waiting adds no cost.
If the input includes service time, it must already be included in outgoing
matrix entries. Travel times may differ by direction, and travelling through
another node may be cheaper than going directly.

Departure is fixed at time zero. Each visit starts no earlier than its ready
time and no later than its due time; early arrivals wait. Return to depot 0
must meet its window too; the depot ready time must be zero. Values are
nonnegative decimals without signs or exponents. The reader scales them exactly
to integers and rejects inputs whose dimensions, scaled values, or tour
arithmetic exceed Gecode's integer limits.

## Model and search choices

The default `--formulation closed` creates all successor and visit-time variables
before search. A circuit constraint visits every node once and sums travel
costs; choosing a trip activates its timing constraint. Search branches on trip
costs first, then successors. `--formulation transition` creates visit variables
as the route grows, with explicit waiting and depot return checks.

The transition formulation accepts `--bound prefix|min-arcs` and
`--residual-propagation off|cheap|unary`, defaulting to `prefix` and `off`.
The prefix bound uses travel cost so far. `min-arcs` also estimates remaining
cost by summing the cheapest incoming trips and the cheapest outgoing trips,
then taking the larger total. These trips need not form a tour, so the estimate
can be too low.

`cheap` checks whether each remaining customer can still be reached and followed
by a return within the deadlines, using shortest travel times. `unary` adds a
temporary schedule in which visits cannot overlap and each visit occupies its
cheapest outgoing travel time. These checks can reject a partial route or rule
out its next customer. Shortest paths may use visited customers, keeping the
checks valid when indirect travel is cheaper than direct travel.

CABS with `transition` discards a candidate when another live candidate for the
same location and unvisited customers arrives no later and at no greater cost.
Use `--dominance off` to disable this pruning.

`--time-ms N` and `--node-limit N` stop search at engine boundaries; time
includes reading and preprocessing. They cannot interrupt propagation. CABS
also accepts `--max-width N` and `--frontier-limit N`, where zero disables the cap.
Runs default to one search thread; use `--threads N` to change the thread count.
