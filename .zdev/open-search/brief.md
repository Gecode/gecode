## Approved branch split (2026-10-13)

This section supersedes the earlier combined-branch topology and validation scope.
The user approved reconstructing `feature/open-search` on `feature/open-cp`,
and rebuilding `feature/open-model` on `feature/portfolio` with the native
search changes integrated there. Preserve native heuristic behavior and the
existing Model/portfolio integration. Keep only generic native dependencies
needed by the search engines; no Model or portfolio implementation belongs in
this branch. Ranking, sound bounds and optional state pruning stay separate.

Backups are `backup/open-search-before-split-20261013` at
`83f9574c3e4d3f097532c81040a09d76135b5510` and
`backup/open-model-before-split-20261013` at
`e105fa6efb690a8b04febdd12fa23400de0707fc`. The native base is
`55dc729c80ed5d60a742177612c870bdf366c532`; the integration base is
`198661dfa15af030936ea66ccedd95c88c82232b`. Parent branches stay untouched.
Retain historical tasks/research as completed evidence from the combined
branch; references to Model behavior describe the later integration branch.
The coordinator repaired this area's parent/base anchor after the explicitly
authorized reconstruction, since the old portfolio boundary is no longer an
ancestor. The backup retains the original metadata and history.

For native extraction, run a fresh Release build and existing full check,
focused native search/open-list/regular checks, ordinary native examples and
tiny TSPTW correctness. Verify Model and portfolio source/dependencies are
absent. Port relevant semantic tests into the existing test system; do not
add a new harness or performance campaign. Generic prerequisite changes need
focused existing comparison/continuation/worker-control coverage. Any generic
hooks retained only for integration must have a concrete compatibility need.
The integration task has its own area and validates Model/portfolio behavior
and the four open Model examples. All newly completed work receives independent
verification before its final commit. Keep PR-only zdev records during work.

# Heuristic search and open transition models

## Objective

Add heuristic search to Gecode, including A* and complete anytime beam search
(CABS), usable with ordinary `Space` models and the `Model` interface. Neither
the Model layer, the portfolio system, nor open variables/lists are required. Open constraint models are an important application. Native engines
are usable directly from ordinary handwritten `Space` subclasses. Demonstrate
an actual state-transition formulation of TSPTW using open lists, then measure
when heuristic search, dominance, and propagation repay their costs.

Success means a user can configure heuristic search, obtain valid solutions and
improving incumbents, and distinguish optimality or infeasibility proofs from
resource termination. An ordinary bounded CP model and an open transition model
must both exercise the public journey. TSPTW solutions must pass an independent
tour/time-window checker. Measurements must report time and memory as well as
expansions, and separate model changes from engine changes.

## Boundaries and settled decisions

- Work lives on `feature/open-search`, combining `feature/portfolio` and
  `feature/open-model`; see [integration notes](background/integration.md).
  The zdev area is isolated, with `portfolio` as its parent. Open-model changes
  are recorded in one linear integration commit because zdev task work rejects
  merge history; the resulting tree matches the validated combination.
- Keep `.zdev` committed during development, then remove it before squash merge
  using the existing pull-request record policy.
- Keep native engines and guidance in the search layer, independent of Model
  declarations, compilation, analysis, and portfolio configuration. Ordinary
  closed Spaces and Spaces using native open lists use the same engine API.
- Start with sequential engines and explicit expert-supplied guidance. Model
  configuration and portfolio composition are optional conveniences around
  those engines. Build working increments before tuning or generalizing APIs.
- Reuse `Space`, `Search` ownership/stops/statistics, compiled Model handles,
  objective comparison, and existing portfolio asset boundaries where they fit.
  Do not introduce a separate modeling language or rebuild the portfolio.
- Keep ranking separate from a bound allowed to prune. For a CP node, an
  objective bound may already be a bound on total cost; adding a prefix cost to
  it again is incorrect. Additive transition models can supply `g + h`.
- State merging is optional and model-justified. Generic CP spaces cannot be
  merged by an arbitrary key without proving equivalent residual problems.
  Tree search remains available when no such declaration exists.
- Beam truncation is heuristic loss of coverage. CABS recovers alternatives by
  widening and restarting while retaining the incumbent. Width, time, or memory
  limits do not constitute an infeasibility or optimality proof.
- General unbounded cyclic planning, parallel beams, learned guidance, LNBS,
  GPU search, arbitrary cost algebras, and automatic heuristic discovery are
  outside the initial delivery. Reconsider only after the core measurements.
- Do not import the older A* prototype or its Model layer wholesale. Consult it
  for concrete expansion and planning examples, and port only what is useful.

## Research and implementation sequence

The first four tasks delivered native A*, native CABS, optional Model guidance,
and ordinary-Space portfolio integration. The next bundle adds a real TSPTW
workload, optional state pruning, and a column-search alternative, then measures
the choices before expanding the engine family further.

| Increment | Outcome | Evidence before moving on |
| --- | --- | --- |
| 1. Settle the search contract | Define ranking, admissible total bounds, solution comparison, optional state equivalence/dominance, ownership, stopping, and proof status. Compare ordinary CP and transition-model needs with the existing APIs. | A short contract and worked calls for both model forms; check finite search, inconsistent admissible estimates, overflow, and solution-return semantics. |
| 2. Sequential A* vertical slice | A usable native `Space` engine with queued stable states, n-ary choice expansion, deterministic ties, and correct stop/cleanup behavior. Initially use cloned frontier states. | Small independently checkable optima, a misleading first goal, no-solution exhaustion, and interruption. Measure frontier memory; defer recomputation machinery until needed. |
| 3. Sequential CABS | Layer beam search and widening restarts on the native Space journey, carrying incumbents and recording truncation independently of valid pruning. | A standalone width-1 run that misses the optimum or every solution, recovery at larger width, variable-depth goals, and resource/width termination without false proof. |
| 4. Optional integration | Model supplies retained guidance, scalar objective-bound defaults, roots and rendering for both engines. Ordinary-Space portfolio recipes compose their native builders. | The same native engines work standalone and in the portfolio. Model analysis offers supported defaults/diagnostics; resource-limited CABS never supplies a coverage proof. |
| 5. Open TSPTW transition model | Model visits, residual state, travel cost, time windows, waiting, and return-to-depot semantics using current open-list/Model features. Provide A*/CABS and DFS/BAB comparisons on the same formulation. | Tiny exhaustive tours and independent path checking; demonstrate lazy growth and clone independence. Confirm one search layer means what the beam algorithm expects. |
| 6. TSPTW state merging and bounds | Add sufficient residual keys, cost-aware resource dominance, and inexpensive admissible bounds where measured useful. Integrate with engines through the settled optional contract. | Cross-check dominance on/off on tiny instances; include earlier arrival with worse prefix cost and incomparable resource/cost labels. Measure retained-state memory. |
| 7. Evaluation, tuning, and integration | Compare engines and formulations on loose, tight, and infeasible TSPTW plus one bounded CP case and one existing planning example. Expose eligible engines through existing portfolio/driver seams when lifecycle and proof contracts fit. | Held-out results at equal resources, ablations, useful documentation, and regression checks for inherited model/open/portfolio behavior. |

An increment may change after an investigation. In particular, do not force a
single generic callback framework before the native and Model calls establish
what is shared. CABS requires no dominance registry for correctness on a finite
search tree; optional state merging should be driven by the TSPTW workload.

## Initial search contract

The first approved bundle settled and implemented the decisions below. Later
numerical/objective families remain area work only when separately planned.

- Deliver scalar integral minimization and maximization. Guidance values use
  signed 64-bit integers; compare directions directly instead of negating values.
  Model float, lexicographic, and custom objectives receive an explicit unsupported
  diagnostic in this slice. Arithmetic in user-supplied estimates must stay in
  range; Gecode helpers must check their own additions.
- Use a read-only evaluation of a propagated stable Space to provide an optimistic
  bound on total objective value. It is a lower bound for minimization and an upper
  bound for maximization, and equals the scalar objective at a solved node. The
  caller must justify the bound. A weaker valid bound is sufficient. The engine
  never adds an implicit prefix cost; transition models compute their own `g+h`.
- A* expands the best total bound first, with an optional separate ranking score
  to break equal-bound ties and a deterministic insertion-order final tie. CABS
  may use an arbitrary ranking score to select the beam, with bound order as its
  default. Ranking is never a reason for sound pruning.
- Both engines use `next()` to return owning, strictly improving solutions as
  they are discovered. A* can publish a generated goal before the remaining
  frontier proves it best. A first returned solution therefore does not imply
  optimality. Continue until natural exhaustion with `stopped()==false` to prove
  optimality of the last incumbent, or infeasibility if none was found. This is
  an incumbent-returning extension of A* expansion order, not a claim of classical
  first-goal return semantics. Global dual-bound output can wait until required.
- Reuse `Space::compare` for incumbent acceptance and `Space::constrain` for
  strict-improvement restrictions. The scalar guidance, comparison, and pruning
  contracts must agree. Adopt an external incumbent only when it strictly
  improves the retained one; reject incomparable incumbents consistently with
  existing exact engines. Keep an owned incumbent independently of caller-owned
  returned solutions.
- Start with tree search and cloned frontier spaces. No state key, dominance
  registry, replay/recomputation scheme, or new Space virtual method is required.
  Statistics depth means committed search edges, never objective cost.
- Follow the existing `Space::prepare` / `Worker::prepare` protocol before
  generating a choice or changing the search path. Interrupted brancher
  preparation remains resumable and its work is counted in existing statistics.
  Reuse the completion fixture when checking this behavior in both engines.
- Queued nodes are stable. Save choices only while they remain compatible with
  their Space. An incumbent cut can invalidate a choice: propagate and obtain a
  fresh choice before expanding the changed Space. For a partially expanded
  parent, retain its original choice/alternative cursor and apply new cuts to
  child clones, or use another demonstrated coverage-preserving approach; a new
  choice may enumerate different alternatives. Preserve unvisited n-ary
  alternatives when `next()` returns midway through an expansion or a stop fires.
- The initial engines are sequential. Support ordinary stop objects, root-clone
  ownership options, engine builders, and external incumbent adoption. Reuse
  the sequential worker-control adapter for capacity-one pause/resume; a paused
  asset is not exhausted. Diagnose unsupported parallel/tracer settings explicitly.
  Do not imply replay behavior from `c_d`/`a_d`; document cloned-frontier storage. No-goods use
  the existing empty-result convention rather than inventing extraction.
- CABS layers count one native `Choice` commit. Roots/terminal nodes may have
  different depths. Every layer's terminal states are considered without dropping
  deeper improving alternatives. A pass that returns early after an improvement
  is incomplete unless the remaining alternatives were preserved for continuation.
- CABS retains an unconstrained stable root for widening passes and its incumbent
  across passes. Begin at width one and double after an incomplete pass, with
  checked growth. A configured maximum width or live-frontier node limit is a
  resource stop, reported through `stopped()`, not natural exhaustion. The node
  limit is a stored-node quota, not a byte or process-memory guarantee. A complete
  finite pass that exhausts improving alternatives can close the search.
- Model engine selection can use the propagated scalar objective lower/upper
  bound as its safe total-bound default. Expert-supplied bounds and optional
  ranking have explicit declarations with dependencies retained through
  compilation, not a side effect of printing or dummy posting. Reuse existing
  authored-handle resolution and retention patterns. No ordinary model pays for
  guidance unless it enables this search route or declares custom guidance. Evaluation exposes declared variables/open
  lists where needed through a read context and must not post constraints. The
  Model route must not require a Space subclass or retaining every lowered variable.

Read `gecode/search.hh`, `gecode/search/seq/bab.hpp`, and the Model runtime,
compile, optimize, and custom-objective declaration seams before implementing
these contracts. The source-backed DIDP context remains in the background index.
Exact public names and placement are implementation choices to document alongside
working calls, subject to these semantics and the existing Gecode conventions.

## Native, Model, and portfolio integration

The native search API takes a `Space`, its normal branching/comparison contract,
and read-only guidance. It owns the frontier and search lifecycle. It must build
and run without linking the Model library or constructing a portfolio. No open
list or special transition representation is needed. A native user can provide
bounds and ranking directly from variables or state in their Space subclass.

Model adds convenience around the same native engines: objective sense and
family validation, a propagated scalar objective-bound default, declared custom
bound/ranking dependencies, handle remapping, root creation, output, and driver
selection. Reuse `CompiledModel::has_integer_objective`, `objective_sense`, and
the objective materialized in `model/runtime.cpp`. Enabling heuristic search
need not require a custom guidance callback for a supported scalar objective.
This safe default is useful even when it is weak; users can replace it with a
problem-specific valid total bound and select a separate beam ranking.

Existing objective-connected groups, functional-dependency generators, and
scheduling metadata may later help suggest branching or ranking configurations.
Investigate these suggestions against worked models before adding an analyzer.
A structural group or guessed rank alone does not justify an admissible bound,
a sufficient transition state, or a dominance rule. The initial Model task uses
objective/dependency information already available; it does not discover domain
heuristics or add a general analysis framework. Every Model-assisted setting
must translate to ordinary native engine/guidance calls.

Portfolio integration uses `Search::Builder` and
`SpacePortfolioSearchFactory`, with owned solutions, external incumbents, stops,
and honest exhaustion. Reuse `WorkerToEngine` in `search/support.hh` for the
existing capacity-one control protocol instead of adding a scheduler. First
exercise ordinary-Space recipes with A*/CABS and BAB, including parked assets and
resource-stopped CABS. Forward external incumbents at a safe engine boundary;
the current complete-recipe adapter inherits a no-op adoption method. Treat
CABS-local width/frontier limits as coverage-free asset termination so another
complete asset can finish; preserve shared/global stop semantics.
The Model adapter can supply the same builders/guidance;
future Model portfolio presets can select them without changing native engines.
Portfolio scheduling and policy remain separate from search implementation.

## Staged work and remaining investigations

The initial bundle is complete, including its bounded closed CP example. The
remaining TSPTW, state-pruning, and tuning work is described below. Completing
either bundle does not silently close the area.

For TSPTW, investigate how existing open-list factories and custom brancher posting
can represent clone-local `(U,i,t,g)` state and assigned visit effects. A list
position is materialized before its visit decision is committed. Prefer a
problem-specific n-ary transition brancher, so a commit means one visit, over
changing the engine to understand list horizons or adding a generic transition
language. Determine return-to-depot, service, and window conventions from the
chosen data and the independent checker before implementing the example.

Optional state equality/resource dominance must include every fact affecting
future feasibility and must compare prefix cost. Design it against the actual
TSPTW state, without changing the tree-search contract. Measure bounds and residual
propagation on loose/tight windows before strengthening them. The Gecode fixpoint
policy and the paper's one-pass Pumpkin policy remain distinct experimental setups.
Parallel search, float/custom/lexicographic support, and cyclic planning remain
outside this initial bundle. The area's later integration work can revisit them
with concrete demand and lifecycle evidence.

## Next bundle: state pruning, column search, and TSPTW

Read [search extensions and benchmark decisions](background/next-search-and-benchmarks.md)
before implementing this bundle. It records the literature comparison and the
reasons for the proposed scope. Approval of the next bundle settles these
choices; it does not authorize every algorithm in the survey.

Add one native progressive column engine with per-depth queues. Default quota
is one and grows by one after each complete depth cycle; fixed quota one is
CBFS. Pending alternatives remain stored, so quota controls expansion effort
and does not justify deletion. Use existing total bounds for pruning and an
optional separate lower-first rank for ordering within each queue. Preserve
partially expanded n-ary choices and the cycle cursor across publications,
ordinary stops, and worker parking. Checked quota saturation preserves a
positive finite quota; a retained-frontier cap is an incomplete resource stop.
Keep sequential finite-tree semantics, ordinary builders, and scalar min/max.
No generic engine-policy framework, Space virtual method, or Model prerequisite
is required. BAB with deliberate branching is the depth-first baseline.

Add a separate optional native state-pruning declaration, driven by the worked
transition model. It extracts an owned descriptor with an exact grouping key
and cost/resource labels, then compares only matching keys. Hash collisions
must not imply equality. Distinguish equivalence, dominance, and incomparable
labels, preserving at least one equivalent representative. The author must
justify every omitted historical fact and include prefix cost in dominance.
Tree search remains the default. Registry descriptors must not retain native
handles or extra Space clones. Registry forgetting can only reduce pruning;
it cannot delete pending search work. Document descriptor/storage costs.

For A* and column search, a better label admits fresh work even after the old
representative was expanded; do not rely on heuristic consistency. For CABS,
initial pruning is among live next-layer candidates only. Remove registry
entries when their candidate is removed and clear between layers and passes.
Never use a beam-discarded representative to suppress a surviving candidate.
Width loss, sound dominance pruning, and resource termination stay distinct.
Reusing an exact representative is the merging supported here; relaxed store
merging and generic transition-dominance rules remain separate future work.

Model supplies optional descriptor dependencies and callbacks through its
existing retention/remapping/read-context conventions and the same native
declaration. Explicit declarations alone retain auxiliaries. Extend its engine
selection and ordinary portfolio builders for column search, preserving the
safe asset-thread adoption already implemented. Structural metadata can inform
explicit branching suggestions; this bundle adds no automatic key, dominance,
or admissibility inference and no general analysis framework.

Use exact scaled canonical matrix costs for TSPTW, minimizing the sum of arcs
including return and excluding waiting. Service already folded into arcs is
not added again. Start at time zero; wait to customer ready times, enforce
customer due times and the depot window on return. Reject nonzero start-depot
ready times in the initial reader. Check scaling, dimensions, nonnegativity,
and Gecode variable/arithmetic limits. Support asymmetric and nonmetric data;
do not impose tour-reversal symmetry or assume direct-distance reachability
is a sound bound on a path through other customers.

Build two ordinary native Space formulations using identical inputs/output and
an independent checker: a costed-circuit closed CP model with time propagation,
and a finite transition model with clone-local (U,i,t,g) and an open visit list.
A problem-specific n-ary brancher commits one assigned visit, with an explicit
final return; list materialization does not update the residual state. Start
with g as the total lower bound except that solved bounds equal full tour cost.
Compare a fixed path-variable control with the same transition brancher to
isolate lazy list allocation. Keep Model optional throughout.

Use cheap minimum incoming/outgoing remaining-cost bounds, checked g+h, and
shortest-path lower bounds for nonmetric reachability. Compare a residual
scheduling relaxation separately, with ordinary Gecode fixpoint propagation.
At equal (U,i), (t1,g1) dominates (t2,g2) only if both components are no greater
and the residual formulation preserves the earlier-arrival preference. Test
incomparable labels and propagation/history that invalidates a proposed key.
Do not add a 1-tree, generic transition DSL, or new propagator before measurements.

Treat BSS regeneration and LNBS suffix-compatible repair as separate bounded
investigations. Each produces a concrete feasibility conclusion and, if useful,
a proposed follow-up task; neither silently adds a production engine. Existing
BAB/LDS/LNS capabilities remain serious comparison points. APPS, a separate
DFBnB/DBDFS port, weighted A*, BULB/BUDDS, learned ranking, relaxed DDs, parallel
and GPU engines are deferred until the retained results justify them.

For the first measurements, separate a tiny correctness set from 12 development
and 24 held-out cases selected before tuning, across canonical families and
separately marked deterministic window variants. Record the selection and
input content hashes in a small data manifest, not a new data-management system.
The named runner executes one process at a time, records all incumbents for
checking, and compares equal thread/time/RSS-watchdog limits. A node quota is
not a byte limit; a sampled RSS watchdog has documented granularity.

Development tuning is bounded to six candidate configurations, two repetitions,
15 seconds per case, with at most 45 minutes of solver wall budget. Held-out
evaluation uses six frozen native configurations (closed BAB/CABS, transition
BAB/A*/CABS/column) and pinned DIDPPy A*/CABS, 30 seconds per case, at most 120
minutes total solver wall budget including bounded development ablations.
Use one thread and the same 1 GiB RSS-watchdog policy; include startup,
preprocessing, and compilation where applicable. Record incomplete runs and
unsupported resource enforcement honestly. Do not launch a larger campaign
without a separately agreed budget or tune on held-out outcomes.

## Additional engine work

The approved extension adds configurable native best-first ordering and a
bounded sequential CADDS investigation. Best-first ordering reuses the native
frontier, descriptor registry and Choice continuation, with sound bounds kept
independent of arbitrary ranking. Preserve AStar's default ordering. Greedy and
fixed weighted ranking are examples for explicit additive costs; dynamic
weighted-search repair and approximation guarantees remain deferred.

The CADDS study freezes a fragment at the deepest nonempty layer and expands
that whole fragment before descending, retaining unselected work. Compare it
with CABS and the documented fixed-width source/alternative cursor using equal
model and heuristic settings. This is a CPU study, with no production-engine
commitment or GPU implementation. It has a separate 300-second cumulative
solver allowance, shared with verification, and uses development cases only.

Primary CPKB reading: Hansen and Zhou, *Anytime Heuristic Search*
(`2007-hansen-anytime-heuristic-search`, fulltext lines 140–176), and Tardivo,
Michel and van Hoeve, *Complete Anytime Decision Diagram Search with
GPU-Accelerated State Expansion* (`2026-tardivo-gpu-accelerated-state-expansion-for-anytime-exac`,
fulltext lines 279–403; DOI `10.1007/978-3-032-27242-3_34`). Read these passages
when implementing ordering or investigating the retained-fragment schedule.
The earlier algorithm deferrals still apply outside these two additions.

## Testing

Use existing checks for the combined base; add no new tests for planning.
For engine and state-pruning implementations, add focused semantic coverage in
existing search/Model test suites. Test plausible correctness failures rather
than queue layout, callback wiring, or every option combination. Tiny exhaustive
TSPTW enumeration and an independent solution checker are valuable because
incorrect bounds, state keys, or dominance can silently remove the optimum.

For the next bundle, test hash collisions, equivalent and incomparable labels,
better-label reopening, beam-discarded representatives, registry forgetting,
and disabled-policy behavior in existing suites. Reuse native lifecycle tests
for column search and Model retention tests for declared state descriptors.
Use focused tiny TSPTW checks rather than a combinatorial option matrix. No
new test harness or tests of documentation-only planning are expected. The
existing ASan search selection can use the main-thread test runner (`-threads 1`):
the automatic parallel harness overflowed its stack during root propagation
in the portfolio fixture, while concurrent portfolio asset tests passed on the
main harness thread. Preserve that validation limitation.

Cover ownership/stops, all Space status cases, goal acceptance, n-ary choices,
strict improvement, beam-loss/proof distinction, and compilation retention where
the implementation actually touches them. Reuse existing lifecycle and
portfolio tests. Use sanitizers for new frontier ownership when supported;
choose additional configurations from touched feature boundaries, not a full
matrix by default.

## Validation

Build with CMake/Ninja and run the existing `check` target, open-list lifecycle,
regular/NFA tests touched by the merge, and targeted portfolio/search tests.
Build and run the four inherited open examples. Record exact successful
commands and limits in [integration notes](background/integration.md).

For performance, first use fixed tiny/development cases to detect algorithmic
mistakes. Then select public TSPTW families and separate training from held-out
instances. Record data semantics, solver versions, seed where relevant, thread
count, elapsed-time and memory limits, and preprocessing/compilation costs.
Report feasible quality over time, time to first solution, final objective,
proof/infeasibility count, stop reason, expansions, propagation time, clone
cost, and peak frontier/registry memory. Add counters only when needed to
answer these comparisons. Do not treat fewer expansions as a runtime gain.

Compare engine changes on the same model, then compare closed versus open
formulations separately. Use ordinary Gecode DFS/BAB and suitable DIDPPy/RPID
references, with separately checked formulations and equal resource limits.
Ablate dominance, bound strength, propagation, beam growth, and representation
one at a time where feasible. Select recommendations among the frozen
configurations from held-out runtime, solution quality, proof, and memory
evidence, without further tuning on those outcomes; there is no universal speedup
requirement across every model.

## Background index

- [Sequential CADDS retained fragments](background/cadds-retained-fragments.md):
  read before proposing a production retained-fragment engine. Includes checked
  coverage, reproducible development comparisons and the decision to defer adoption.

- [DIDP, CABS, and TSPTW](background/didp-and-tsptw.md): read before choosing
  search semantics, state dominance, bounds, or benchmark families. Includes
  CPKB reading paths, primary sources, and inspected implementation links.
- [Combined base and integration](background/integration.md): read before
  implementation or a parent refresh. Identifies the current interfaces, older
  prototype, merge resolutions, and validation commands.
- [Search extensions and benchmark decisions](background/next-search-and-benchmarks.md):
  read before the next bundle's column engine, state pruning, BSS/LNBS studies,
  or TSPTW models and evaluation. Separates selected work from deferred algorithms.
- [Fixed-width beam-stack regeneration study](background/beam-stack-regeneration.md):
  read before proposing deterministic-transition continuation or evaluating BSS
  memory claims. Includes checked coverage, cursor comparison, and replay commands.
- [Suffix-compatible TSPTW repair](background/suffix-compatible-repair.md): read
  before proposing LNBS integration. Records checked boundary changes, suffix
  rollout, existing recipe limitations, and bounded reproduction commands.
