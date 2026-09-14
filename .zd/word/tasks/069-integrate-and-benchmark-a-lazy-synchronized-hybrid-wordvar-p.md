+++
schema_version = 1
id = "word-069"
key = "prototype-hybrid-word-varimp"
area = "word"
status = "done"
blocked_by = ["word-068"]
+++
# Integrate and benchmark a lazy-synchronized hybrid WordVar prototype

## Outcome

Integrate the proven synchronization primitive into a temporary production-shaped WordVarImp, selected arithmetic propagators, events, branching, cloning, and integer-compatible views, then determine whether persistent bounds materially improve arithmetic-heavy models.

## Context

Use the synchronization primitive selected by the preceding task. Propagators should work on local cube and interval snapshots, batch their deductions, call synchronization once near the end of propagation, and atomically publish the resulting domain and event. The prototype must follow ordinary Gecode VarImp, view, propagator, cloning, recomputation, and event patterns. The principal hypothesis is that persistent bounds shared across arithmetic constraints can reduce search by orders of magnitude even if every Word variable grows from roughly 48 to 80 bytes.

## Boundaries

- Work in a temporary exact-source tree; do not retain production changes without a separately approved implementation task.
- Use one interval ordering per variable and WordValue endpoint storage.
- Do not weaken existing modular Word semantics or silently substitute mathematical integer arithmetic.
- Reuse standard Int propagator templates only where the prototype view satisfies their complete protocol and value range.
- Do not invent a separate scheduler, allocation scheme, or propagation framework.
- Treat hashes and CRCs as overhead controls, not expected beneficiaries.

## Done when

- [x] The temporary WordVarImp stores cube masks, interval endpoints, and ordering with correct copy, clone, failure, assignment, and event behavior.
- [x] Word views expose cube operations while unsigned and signed integer-compatible views expose the selected interval ordering.
- [x] Selected arithmetic propagators batch local deductions and synchronize each changed variable at most once per propagation.
- [x] Focused tests cover bound-only events, aliases, width 1 and 64, signed boundaries, branching, clone isolation, genuine recomputation, subsumption, and failure.
- [x] At least unsigned comparison, signed comparison, multiplication, division or divmod, and representative mathematical integer order or linear constraints consume persistent bounds.
- [x] Current cube, eager-hybrid, and lazy-synchronized hybrid variants preserve exact solutions, checksums, failures, and declared semantics.
- [x] Arithmetic-heavy measurements include factor or fixed-product recovery plus at least two natural shared-variable arithmetic chains where bounds can travel across multiple actors.
- [x] Control measurements include symbolic ALU, MD5 or Speck, and CRC workloads.
- [x] Measurements report wall time, nodes, failures, propagations, bound and cube events, synchronization calls, actor and variable state bytes, clone traffic, and best-effort RSS across adjacent scales.
- [x] The result distinguishes gains from stronger propagation, reduced search, cheaper execution, and changed model semantics.
- [x] The Word brief records a go or no-go decision and, on go, proposes separate kernel, view/API, actor-migration, branching, and validation tasks.

## Validation

- Use exact current-commit Release baselines, hard time bounds, alternating trials, adjacent scales, and preserved temporary provenance.
- Run focused Word kernel, arithmetic, branching, clone, recomputation, and TestFramework tests in the temporary build.
- Compare applicable integer-view models with ordinary IntVar models for exact semantic parity.
- Run zd check word and git diff --check.
- Confirm the shared tree contains only the durable brief update.

## Result

Integrated and independently verified a temporary lazy-synchronized hybrid WordVar kernel with explicit interval ownership, typed unsigned/signed views, transactional arithmetic actors, and representative arithmetic/control measurements; rejected unconditional migration while retaining an opt-in direction.

Validation:

- Kernel ownership/events, width 1/64, aliases, failure, clone isolation, branching, and c_d=1 replay passed; transactional Div/Mod/DivMod exhaustive truth and publication bounds passed.
- Direct typed order and mathematical sum matched ordinary Int semantics; arithmetic chains preserved exact solutions/checksums and demonstrated both substantial search gains and scale-dependent propagation tradeoffs.
- Final clean eager/lazy builds, focused Word tests, 249-entry manifest, zd check word, and git diff --check passed independent Spec and Standards verification.
