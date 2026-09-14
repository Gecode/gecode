+++
schema_version = 1
id = "word-070"
key = "design-wordvar-domain-variants"
area = "word"
status = "done"
blocked_by = []
+++
# Design a unified WordVar with optional signed or unsigned bounds

## Outcome

Produce an implementation-ready architecture for one public WordVar type backed by compact cube and extended bounded implementations, with immutable construction-time numeric meaning, natural modeling APIs, bounds-aware propagator selection, and a complete migration and validation roadmap.

## Context

Tasks word-068 and word-069 proved that a cube intersected with one numeric interval can be synchronized in one constant-operation pass, that lazy publication is substantially cheaper than eager synchronization, and that persistent bounds can materially reduce arithmetic search. Universal enlargement of every WordVar was rejected, but an optional extended implementation is promising. The user has settled that bounded variables are constructed as signed or unsigned and retain that meaning for their lifetime. The public modeling type should remain WordVar. Ordinary cube constraints must work on every implementation, while posting and views should select bounds-aware propagation only where the variable representations and operation semantics support it.

## Boundaries

- This is a deep architecture and API investigation; do not change production WordVar, public API, propagators, tests, or build files.
- Treat one public WordVar type as a settled product requirement unless repository mechanics prove it infeasible.
- Treat cube, unsigned-bounded, and signed-bounded meaning as immutable construction-time properties.
- Do not design runtime conversion between signed and unsigned bounded interpretations.
- Do not require every propagator to pay the extended-variable cost.
- Preserve modular Word semantics independently of signed or unsigned numeric interpretation.
- Follow ordinary Gecode VarImp, view, event, copying, forwarding, subscription, brancher, trace, and MiniModel patterns.
- Keep architecture probes and measurements temporary; record only durable conclusions and a separately approvable task split.

## Done when

- [x] The investigation compares at least tagged base plus derived bounded implementations, one always-large tagged implementation, trailing or sidecar bounded storage, and separate public variable types, with a justified recommendation.
- [x] A temporary compile-and-clone probe proves whether a compact WordVarImp and larger bounded implementation can share one WordVar handle while preserving VarImp forwarding, perform_copy dispatch, static update and recovery lists, allocation size, subscriptions, failure recovery, and recomputation.
- [x] The proposed object layouts report exact cube, bounded, handle, and view sizes and identify every runtime branch paid by ordinary cube variables.
- [x] The construction API is specified for cube, unsigned-bounded, and signed-bounded WordVar values, including backward-compatible defaults, initial cube masks, initial numeric bounds, arrays, invalid combinations, and public introspection.
- [x] The domain invariants and update API are specified, including immutable ordering, one-pass synchronization, atomic publication, assignment, failure, aliases, width 1 and 64, and values outside Gecode Int limits.
- [x] The modification-event and propagation-condition lattice is specified for cube-only, bound-only, combined, and assigned changes, including deltas, advisor access, subscriptions, clone recovery, and notification batching.
- [x] The view architecture is specified: a common cube WordView for all implementations plus signed and unsigned bounded views that are available only for matching extended variables.
- [x] Posting dispatch is specified for homogeneous and mixed operands, including when to select a bounds-aware actor, when to retain the cube actor, and how constants and result variables participate.
- [x] The investigation settles behavior when an operation's numeric interpretation differs from a variable's construction-time interpretation, including signed relations on unsigned-bounded variables and unsigned arithmetic on signed-bounded variables.
- [x] Every public Word operation is classified as cube-only, bounds-neutral, bounds-enhanced, interpretation-specific, mixed Word/Int, or requiring further research.
- [x] The classification covers relations and reification, logic, conditional, structural operations, shifts and rotations, binary and n-ary arithmetic, multiplication, signed and unsigned division families, product_mod, divmod, reductions and counts, element, channeling, branching, tracing, arrays, MiniModel, and examples.
- [x] A full bounds-aware propagator strategy is specified for arithmetic, including which existing actors can be templated over views, which require separate algorithms, how aliases are handled locally, and how each distinct variable is synchronized once per propagation.
- [x] The investigation specifies which standard Int bounds propagators can operate directly through bounded Word views, which need Word-specific variants, and why domain-consistent Int actors remain separate.
- [x] Bounds-aware branching and tracing are designed so experiments do not branch on cube values excluded by the interval and diagnostics expose both cube and numeric state.
- [x] The modeling experience supports easy controlled experiments, including a concise constructor mode, model-option switching between cube and bounded variants, consistent arrays, introspection, and examples that differ only in variable construction.
- [x] Backward source compatibility, ABI implications, library exports, generated VarImp files, Doxygen inventory, changelog, serialization or archive concerns, and downstream build effects are audited.
- [x] The validation strategy covers exhaustive small-width domain semantics, events, aliases, actor lifecycle, clone isolation, genuine recomputation, branching, tracing, MiniModel lowering, width-64 arithmetic, mixed representations, and failure recovery.
- [x] The benchmark plan includes arithmetic-heavy shared-variable chains, division and multiplication families, signed arithmetic, standard Int parity, and hash, CRC, logic, and structural overhead controls, with exact semantic parity and clone-traffic measurements.
- [x] The Word brief records the recommended architecture, rejected alternatives, unresolved decisions, public modeling examples, complete operator matrix, performance expectations, and an expand-migrate-contract task split for separate approval.

## Validation

- Use temporary architecture probes based on exact current HEAD and preserve source, commands, raw output, object sizes, and hashes.
- Compile and run bounded clone, recomputation, event, and mixed-view probes sufficient to validate the recommended variable-implementation design.
- Inspect generated VarImp configuration and cloning code rather than assuming polymorphic copying works.
- Review the complete public Word API and ordinary test inventories against the operator classification.
- Run zd check word and git diff --check.
- Confirm no production, API, test, example, or build file changed.

## Result

Selected one public WordVar with a 48-byte tagged compact implementation and 64-byte derived unsigned/signed bounded implementation; specified construction-time meaning, events, views, dispatch, complete operator migration, modeling, ABI, validation, and staged rollout.

Validation:

- Independent PASS: exact-HEAD compile/clone probe verified mixed implementation sizes in one VIC registry, failure recovery, c_d=1 recomputation, 8/48/64-byte layouts, and generated BITS/BND/DOM events; complete API/operator audit and SHA manifest passed; zd check word and git diff --check passed; no production changes.
