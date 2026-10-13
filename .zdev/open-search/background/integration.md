# Combined base and integration points

Read before implementing heuristic search or refreshing the parent branch.
The first combined base is `0a07191db190745af9a4acdbf0457cd9f2d6db65`.

## Starting branches

- `feature/portfolio`: `ed2f9d0e2562dcf47c07176151e11ccd82947d34`.
- `feature/open-model`: `e105fa6efb690a8b04febdd12fa23400de0707fc`.
- Shared `feature/model` base: `6028fb38eedf7e5467cec1e78ebc02f4d8b424c7`.

Both inputs use the current named-pass Model layer. Open-model's visible Model
addition is open-list support and four planning examples; it does not import
the older A* helper. Portfolio also includes non-failing and other search work;
those inherited features are part of the starting tree, not separate work for
this area.

The integration was resolved as a merge, built and checked, then recorded as a
single commit on portfolio because zdev rejects merge history for task work.
The recorded tree is identical to the validated merge tree. The input branches
were left in place. Zdev uses `portfolio` as the area's single parent.

## Resolutions

The ModelSpace clone constructor retains both portfolio restart/FDS fields and
open integer/Boolean list updates. Its header retains the soft-model declarations,
open-list declarations/accessors, and portfolio `slave()` override. The regular
test file retains the non-failing runtime-retirement test and the open/NFA test
classes and registrations. Both changelog additions remain.

CI and `vcpkg.json` retain portfolio's matching vcpkg baseline
`45f9f39362a4c52e2b1fbe57b7e649db7f3d96d4`. Other files merged automatically.
No heuristic search engine has been introduced during setup.

## Sources to inspect when designing the interface

- `gecode/search.hh`, `gecode/search/engine.hpp`, `gecode/search/seq/`: native
  engine/builders, options, stops, statistics, ownership, and expansion.
- `gecode/search/portfolio.hpp`: `SpacePortfolioSearchFactory`, complete recipes,
  objective capabilities, asset lifecycle, and status. Natural exhaustion is a
  coverage claim: a beam that dropped alternatives must not masquerade as an
  exhausted complete asset. Inspect these contracts before adding an engine.
- `gecode/model/runtime.hh` and `.cpp`: retained variables, root/clone creation,
  custom objective contexts, comparison, and improvement restrictions. Decide
  guidance dependencies and remapping here before adding a public callback API.
- `gecode/model/driver.hh`, `gecode/model/portfolio.cpp`: public model journeys
  and driver dispatch. Add only the route needed by the vertical slice.
- `gecode/model/open-list.hh` and `.cpp`, `gecode/int/open-var-list.hpp` and
  `.cpp`, `gecode/int.hh`: open-list factories, transitions, native resolution,
  horizon/value branching, length and prefix lifetime.
- `models/open-miconic.cpp`, `models/open-puzzle.cpp`, `models/open-valves.cpp`,
  `models/open-maintenance.cpp`: inherited planning examples and open Model use.
- `test/model/open-list.cpp`, `test/int/open-var-list.cpp`, `test/search.cpp`,
  `test/model/runtime.cpp`: reuse these test seams rather than adding a harness.

Factories/transitions must remain deterministic and must not capture space-local
state. A materialized list position is not necessarily an assigned transition.
Check this distinction when representing residual state or defining beam layers.
Model handles can be remapped or removed by compilation; declare every guidance
or output dependency through the appropriate retention mechanism.

## Older implementation reference

`feature/open-cp-model:models/astar.hh` contains a sequential template over a
Space with `cost()`, `estimate()`, and `key()`. It queues propagated clones,
saves their choices, skips stale entries, checks stops, and deletes its frontier.
Its concrete unsigned cost and 64-bit key types, additive comparison, and
statistics conventions are prototype choices to reconsider for the new contract.
Do not import the older Model implementation to obtain this helper.

`feature/open-cp-model:models/open-puzzle.cpp` has explicit residual puzzle state,
a custom move brancher, open-plan variants, and a native A* comparison. Consult
it if the current automaton example leaves transition-state design unclear.
The current `models/open-puzzle.cpp` is a smaller open Model example using the
current layer; the older file is not its current interface specification.

## Completed setup validation

On 2026-10-07 the combined code passed a Release build with AppleClang 21,
CMake 3.27.6, and Ninja 1.11.1. The configuration enables the normal Model,
search, integer, set, float, FlatZinc, and thread modules; MPFR and GUI support
were disabled for this local check. This does not establish Windows, MPFR, or
sanitizer validation.

```sh
cmake -S . -B build/open-search -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DGECODE_ENABLE_QT=OFF -DGECODE_ENABLE_GIST=OFF -DGECODE_ENABLE_MPFR=OFF
cmake --build build/open-search --target gecode-test \
  model-open-maintenance model-open-miconic model-open-valves model-open-puzzle -j 8
cmake --build build/open-search --target check -j 8
build/open-search/bin/gecode-test -iter 1 -threads 0 \
  -test Model::OpenLists -test Int::Extensional::Reg \
  -test Model::Portfolio::Restrictions -test Model::Portfolio::Sequential \
  -test Model::Portfolio::ActivationRoots -test Search::Portfolio::PublicContract
build/open-search/bin/gecode-test -iter 1 -threads 0 -test Int::OpenList
build/open-search/bin/model-open-maintenance
build/open-search/bin/model-open-miconic
build/open-search/bin/model-open-valves
build/open-search/bin/model-open-puzzle
```

The native tests are named `Int::OpenList`, despite the source filename
`open-var-list.cpp`. The existing `check` selection includes Model open-list
lifecycle and sparse NFA checks. All four default example runs found a solution.
The integration added no new tests. For later code changes, choose focused checks
from the actual touched contracts and follow the testing policy in `brief.md`.

## Native CABS validation and fixed comparison

On 2026-10-07, the native CABS implementation passed the following commands in
the existing Release build. The focused search selection is separate from the
repository check target. All four inherited open examples also passed their
default runs listed above.

```sh
cmake --build build/open-search --target gecode-test astar \
  model-open-maintenance model-open-miconic model-open-valves model-open-puzzle -j 8
build/open-search/bin/gecode-test -iter 1 -threads 0 \
  -test Search::CABS -test Search::AStar \
  -test Search::Completion::Native -test Search::WorkerControl::Pause
cmake --build build/open-search --target check -j 8
build/open-search/bin/gecode-test -iter 1 -threads 0 \
  -test Int::OpenList -test Model::OpenLists -test Int::Extensional::Reg \
  -test Model::Portfolio::Restrictions -test Model::Portfolio::Sequential \
  -test Model::Portfolio::ActivationRoots -test Search::Portfolio::PublicContract
build/open-search/bin/astar
otool -L build/open-search/bin/astar
```

The same four focused search selections and native example passed in the
existing Debug AddressSanitizer build, with no sanitizer diagnostics. That
configuration disables Model, set, float, and FlatZinc.

```sh
cmake --build build/open-search-asan --target gecode-test astar -j 8
build/open-search-asan/bin/gecode-test -iter 1 -threads 0 \
  -test Search::CABS -test Search::AStar \
  -test Search::Completion::Native -test Search::WorkerControl::Pause
build/open-search-asan/bin/astar
```

The native example uses one bounded ordinary Space formulation and one thread,
with no ordinary time/node stop. Zero width/frontier limits mean unlimited.
The following is one Release run; elapsed times are illustrative and establish
neither speedups nor suitable defaults. The binary links search and integer
libraries plus their kernel/support dependencies, without Model or driver.

| Engine | Maximum width | Frontier quota | Objective | Result | Milliseconds | Nodes | Restarts |
| --- | ---: | ---: | ---: | --- | ---: | ---: | ---: |
| A* | 0 | 0 | -7 | proven | 0.133 | 7 | 0 |
| BAB | 0 | 0 | -7 | proven | 0.009 | 7 | 0 |
| CABS | 1 | 0 | -2 | stopped | 0.005 | 7 | 0 |
| CABS | 2 | 0 | -7 | proven | 0.006 | 14 | 1 |
| CABS | 4 | 0 | -7 | proven | 0.006 | 14 | 1 |
| CABS | 0 | 0 | -7 | proven | 0.005 | 14 | 1 |
| CABS | 0 | 1 | 4 | stopped | 0.001 | 3 | 0 |
| CABS | 0 | 4 | -7 | proven | 0.005 | 14 | 1 |

A* published 4 then -7; BAB and complete CABS runs published 4, -2, then -7.
Width one loses the optimum to the deliberately misleading rank. Widening
retains the -2 incumbent, whose valid cuts reduce the next pass enough for
width two to recover and prove -7. The frontier quota is the retained pending
node count described in `docs/cabs.md`, not a process-memory limit.

## Configurable best-first ordering (task 029)

The existing AStar worker supports fixed bound-first or rank-first ordering.
Both use admissible bounds for pruning. On the standalone additive example,
greedy `h` publishes 10 then 5; fixed rank `g+2*h` publishes 5. Both exhaust
and prove optimum 5. Native linkage requires search, integer, kernel and support,
with no Model or driver dependency.

The following commands passed in the macOS Release configuration. The baseline
AStar selection also passed before the change. Ninja exposes the concrete
`gecodesearch_shared` target; `gecodesearch` is an INTERFACE target.

```sh
cmake -S . -B build/open-search -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DGECODE_ENABLE_QT=OFF -DGECODE_ENABLE_GIST=OFF -DGECODE_ENABLE_MPFR=OFF
cmake --build build/open-search --target gecodesearch_shared gecode-test astar best-first model-open-maintenance model-open-miconic model-open-valves model-open-puzzle -j 8
build/open-search/bin/gecode-test -iter 1 -threads 0 -test Search::AStar -test Search::CABS -test Search::Column -test Search::Completion::Native -test Search::WorkerControl::Pause -test Search::Portfolio::PublicContract
cmake --build build/open-search --target check -j 8
build/open-search/bin/gecode-test -iter 1 -threads 0 -test Int::OpenList -test Model::OpenLists -test Int::Extensional::Reg -test Model::Portfolio::Restrictions -test Model::Portfolio::Sequential -test Model::Portfolio::ActivationRoots -test Search::Portfolio::PublicContract
build/open-search/bin/best-first
build/open-search/bin/astar
build/open-search/bin/model-open-maintenance
build/open-search/bin/model-open-miconic
build/open-search/bin/model-open-valves
build/open-search/bin/model-open-puzzle
otool -L build/open-search/bin/best-first
git diff --check
```

The ordering cases use nonterminal root alternatives, both objective senses,
missing-rank fallback and optional sound descriptor pruning. Existing lifecycle
and portfolio checks also passed. No new frontier ownership was introduced,
so the plan's conditional sanitizer run did not apply. Autotools, Windows and
sanitizer builds were not run for this task. No performance study was launched.

## Sequential CADDS study (task 030)

Only standalone study source and data were added; production libraries and
tests remain as verified in task 029. Its full check, native/open-feature
selections and examples provide the unchanged-production baseline, and were
not rerun for task 030. The [study report](cadds-retained-fragments.md) records
exact build/capture/analyze commands and the shared 300-second solver budget.

Normal and standalone ASan semantic witnesses passed, including original
Choice continuation, full-fragment scheduling and stop/resume. The ASan
executable linked uninstrumented Release Gecode libraries. All 25 tiny rows
matched independently checked coverage or optima, and all 30 frozen development
rows completed capture with checked publications. The source and replay commands
are retained under `misc`; no production CADDS engine was added. The bounded
quality/memory comparison supports deferring adoption.
