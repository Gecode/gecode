+++
schema_version = 1
id = "open-search-016"
key = "tsptw-heldout-evaluation"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-009", "open-search-010", "open-search-012", "open-search-013", "open-search-014", "open-search-015"]
+++
# Evaluate frozen formulations and engines on held-out TSPTW

## Outcome

A bounded held-out study compares strong closed CP, open transitions and DIDPPy, separates engine/representation/pruning effects, and recommends defaults and any further search work from measured evidence.

## Context

Read the brief's exact evaluation matrix/budget and the retained runner, frozen development choices and BSS/LNBS conclusions. The six native configurations are closed BAB/CABS and transition BAB/A*/CABS/column; DIDPPy A*/CABS adds two references. The eager transition control and pruning/bound/propagation ablations use a small development subset inside the total budget.

## Boundaries

- Freeze configurations and held-out selection before measurement; do not retune on held-out results or claim an overall Gecode/DIDP ranking.
- At most 120 minutes total solver wall budget, 24 held-out cases, 30 seconds each, one thread and the same documented 1 GiB RSS-watchdog policy. A larger campaign needs a separate budget.

## Done when

- [x] The runner records the complete frozen matrix, checker results and honest failures/time/resource stops. Missing references or unsupported measurements remain visible rather than being estimated.
- [x] Report first solution and quality over time, final cost/gap where a valid reference exists, proof/infeasibility coverage, elapsed time, expansions, RSS and retained frontier/descriptor/Space costs, including setup and inference overhead.
- [x] Compare engines on identical models first; compare closed/open formulations separately. Bounded development ablations isolate eager/open path allocation, dominance, cheap bounds and residual propagation one change at a time.
- [x] Reproduce summaries from retained raw records and distinguish canonical families from generated variants. A slow or unsuccessful option is an acceptable finding.
- [x] Publish useful public documentation and benchmark commands, chosen explicit defaults and a concrete next-work recommendation grounded in the BSS/LNBS studies and measured bottlenecks. Do not silently implement deferred algorithms or close the area.

## Validation

- Preview budget and frozen identities, execute the bounded named campaign, independently check all publications and replay analysis without solver invocation.
- Run the applicable correctness/search/open-list/Model/portfolio checks and repository check target for any final default/config changes.
- No tests for report formatting or broad configuration matrix; retain exact commands, counts, limitations and results.

## Result

Implemented the frozen benchmark presets, development selector, exact argument validation, cumulative budget accounting and offline comparison support. Completed all 192 held-out observations and published the reproducible summary and README results.

Validation:

- Independent whole-task PASS against W8867a7f4bdb6afaa; final comparison equal. Verified all 192 frozen rows, 101 identities, 569 terminal and three archived publications, 81 summaries, 34 optimal and ten infeasible proofs. Public summary reproduces byte-for-byte; completed resume and changed-cap rejection launch no solvers or alter captures. Native search/Model/open-list/portfolio selections, seven benchmark tests, repository check and full TSPTW validator passed; historical tuning remains analyzable. Required correctness replay passed in its original no-RSS wrapper after the extra 1 GiB wrapper stopped an initial attempt; that failure remains charged and benchmark limits are unchanged. Verification used 141.712165 seconds of 300; total charge 4031.446727 of 7200 comprises 3845.615011 measured seconds and 185.831716 conservative cleanup seconds, no pending process. Two interrupted termination attempts are preserved and disclosed. Raw captures and exact commands remain under build/tsptw-results/heldout-comparison-016 and /tmp/open-search-016-study; findings empty.
