+++
schema_version = 1
id = "open-search-001"
key = "native-astar"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Deliver sequential A* with incumbent publication

## Outcome

Ordinary Space users can run bound-ordered A*, obtain improving integer solutions, and distinguish interruption from complete exhaustion.

## Context

Follow the Initial search contract in ../brief.md. Read ../background/integration.md for the combined base and ../background/didp-and-tsptw.md for bound/proof semantics. Start with gecode/search.hh, search/seq/bab.hpp, search/build.hpp, and test/search.cpp. The current best-solution engine accepts external incumbents through compare/constrain; cuts can invalidate choices. There is no reusable native A* engine in this base.

## Boundaries

- Use sequential cloned-frontier tree search for scalar integer minimization/maximization; add no Space virtual methods, graph registry, recomputation scheme, or parallel implementation.
- Keep bound-first A* expansion and incumbent publication semantics from the brief; do not prune by an arbitrary ranking score.
- Native engine/guidance APIs build and run without the Model library, portfolio configuration, or open variables/lists.

## Done when

- [x] A native engine and builder accept explicit read-only total-bound guidance, optional tie ranking, and ordinary Search::Options; a standalone closed Space example returns caller-owned improving solutions.
- [x] Minimization/maximization use direct signed-64-bit comparisons, and existing compare/constrain accept only strictly improving compatible incumbents.
- [x] Root cloning/transfer, failed/solved roots, n-ary expansion, destruction, stop/resume, and external incumbent adoption preserve ownership and search coverage. The existing Space::prepare/Worker::prepare protocol preserves interrupted brancher preparation and its statistics. New cuts cannot reuse an incompatible choice or skip pending alternatives.
- [x] Natural exhaustion after continuing next() certifies the final optimum or infeasibility. Resource interruption reports stopped; statistics count search depth independently of objective cost.
- [x] Reuse the existing sequential control adapter for capacity-one pause/resume; unsupported parallel and tracer settings have explicit diagnostics. Document cloned-frontier storage and the absence of a hard memory guarantee.
- [x] Focused existing-suite tests exercise a misleading first generated goal, signed min/max objectives, n-ary continuation after publication/stop, and external incumbent adoption while work remains. Reuse CompletionNative for interrupted preparation/resumption; tiny optima are independently checkable.

## Validation

- Build the search module and gecode-test in build/open-search; run the new focused Search selection and existing best-solution comparison/stop tests relevant to the touched code. Verify a native-only consumer links search without the Model library or portfolio setup through the established consumer pattern.
- Run the existing check target. Run a focused sanitizer ownership check if supported; record any unavailable sanitizer evidence without adding a harness.

## Result

Implemented native sequential A* with owned frontier clones, total-bound guidance, optional tie ranking, strictly improving publication, resumable preparation/n-ary expansion, external incumbents, capacity-one control, standalone example and documentation. Independently verified against unchanged snapshot W6319d6b22f9c2448.

Validation:

- Release search/test/example/FlatZinc build, five focused A*/completion/comparison/incumbent/pause tests, relevant existing BAB/RBS/control tests, and the check target passed.
- Standalone example and installed find_package(search int) consumer published 4, -2, -7 and proved -7; consumer linked no Model library.
- Native-only AddressSanitizer build and five focused tests passed without reports; inherited open examples and portfolio public contract passed.
