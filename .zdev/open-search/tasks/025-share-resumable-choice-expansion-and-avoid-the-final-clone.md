+++
schema_version = 1
id = "open-search-025"
key = "share-choice-expansion"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "low"
blocked_by = []
+++
# Share resumable Choice expansion and avoid the final clone

## Outcome

The three native engines reuse a small internal owner of original-Choice continuation and can transfer the final parent into its child instead of cloning it.

## Context

The prepare/choice/commit/cut/status/feedback sequence repeats in gecode/search/astar.cpp:203–236, cabs.cpp:269–305 and column.cpp:246–286. Every alternative currently clones the parent; CABS/Column immediately retire it after the final commit. gecode/search/seq/path.hpp:235–245 provides the established last-alternative reuse precedent. Gecode's separation of state restoration, tree construction and exploration is also discussed by Tack and Le Botlan, Compositional Abstractions for Search Factories (CPKB paper 2004-tack-compositional-abstractions-for-search-factories, fulltext lines 104–148).

## Boundaries

- Design the small call sequence first: an internal component owns the selected parent, original Choice and cursor; engines still own frontier selection, quota/layer advancement, incumbent acceptance and proof status.
- Keep incumbent revision/depth explicit and preserve branch feedback and preparation suspension. No recomputation, new public Space virtuals, common engine inheritance or policy-mode framework.

## Done when

- [x] A*/CABS/Column reuse the same original-choice continuation mechanism without changing their documented search order or coverage.
- [x] Final-alternative transfer avoids one expansion clone per fully expanded parent, with no use of a Choice after its lifetime ends.
- [x] Publication, exceptions, external cuts, ordinary stop/resume and frontier-quota accounting retain correct ownership and continuation.

## Validation

- Run existing native lifecycle/preparation, n-ary continuation, incumbent, cap/reset and focused portfolio tests before and after the change. Use one bounded clone-count comparison on unary and n-ary fixtures to establish eliminated clones; use the existing sanitizer selection for changed ownership. Do not infer a runtime speedup from clone counts.

## Result

AStar, CABS and Column share original-Choice continuation and reuse the selected parent for its final alternative. Scheduling, incumbent cuts, stops, cap accounting and proof responsibilities remain separate. Contract-valid bounded evidence confirms one avoided expansion copy per fully expanded parent.

Validation:

- Independently verified the whole open-search-025 task against admitted snapshot W39fd00b0331993bd at HEAD 14d6ad440b852ff4380e03e63fca6f62f011e22e. Reviewed all 12 changed paths, including the new internal header, callers, ownership, tests, documentation and installation rules. ChoiceExpansion preserves original choices and preparation across interruptions/publications, transfers the final parent with its Choice alive through commit/cut/status/feedback, destroys the Choice before its source on exception/reset/destruction, and retires the parent before cap admission. Engine scheduling, explicit revisions/depth, incumbent ownership, registries, descriptor accounting and proof decisions remain intact; no public framework or Model dependency was added. The focused feedback tests meaningfully cover final-alternative feedback and final-commit exception cleanup. Corrected temporary clone evidence satisfies scalar objective, comparison, strict-cut and admissible-bound contracts. Confirmed the six baseline engine sources/headers byte-exact against HEAD, baseline factory/next definitions through nm/link map, and runtime factory images through dladdr. Ran each bounded clone executable once; outputs exactly match stored corrected evidence. Unary expansion clones decrease 5→0 for all engines, with five fully expanded parents. Ternary AStar/Column decrease 39→26 with 13 fully expanded parents; CABS decreases 57→38 with 19. Every other field matches, including the strict sequence 0 through -26, 27 solutions, node/failure/restart/depth counts, construction/restart/publication/adoption copies and stop/proof outcomes. These corrected counts supersede invalid earlier evidence and imply no runtime claim. Required commands passed: cmake --build build/open-search --target gecode-test astar model-open-maintenance model-open-miconic model-open-valves model-open-puzzle -j 8; build/open-search/bin/gecode-test -iter 1 -threads 1 -test Search::AStar -test Search::CABS -test Search::Column -test Search::Completion::Native -test Search::TSPTW::Transition -test Search::BranchFeedback -test Search::WorkerControl::Pause -test Search::Portfolio::PublicContract; cmake --build build/open-search --target check -j 8; build/open-search/bin/gecode-test -iter 1 -threads 1 -test Int::OpenList -test Model::OpenLists -test Int::Extensional::Reg -test Model::Portfolio::Restrictions -test Model::Portfolio::Sequential -test Model::Portfolio::ActivationRoots -test Search::Portfolio::PublicContract; build/open-search/bin/astar; and all four default model-open examples. Also passed cmake --build build/open-search-asan --target gecode-test astar -j 8, the identical focused selection using build/open-search-asan/bin/gecode-test, and build/open-search-asan/bin/astar without sanitizer diagnostics. ASan remains Debug with Model/set/float/FlatZinc disabled; main-thread harness preserves the known parallel-harness stack limitation while concurrent asset cases still run. git diff --check passed and snapshot comparison returned equal:true. Validation wrote only temporary clone-output files and ignored build/check artifacts; no source, task, index or completed task028 ledger changes.
- Verifier evidence: work_context_snapshot: W39fd00b0331993bd. Exact baseline14d6ad440b852ff4380e03e63fca6f62f011e22e source/next linkage confirmed. Corrected unary counts5->0/full5 perengine; ternary AStar/Column39->26/full13, CABS57->38/full19. All strict improvement sequences, node/failure/restart/depth counts and stop/proof outcomes matched. Counts establish copies avoided, not runtime gain.
