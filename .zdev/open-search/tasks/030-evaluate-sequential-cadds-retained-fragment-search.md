+++
schema_version = 1
id = "open-search-030"
key = "retained-fragment-study"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-018", "open-search-025"]
+++
# Evaluate sequential CADDS retained-fragment search

## Outcome

A bounded CPU prototype study determines whether deepest-layer retained-fragment search merits a production Gecode engine, based on checked coverage, anytime behavior and memory cost.

## Context

Read the brief's Additional engine work section, background/beam-stack-regeneration.md and the completed TSPTW measurements before prototyping. The existing Column engine retains depth queues but uses a different progressive schedule. The primary CADDS reference is Tardivo, Michel and van Hoeve, Complete Anytime Decision Diagram Search with GPU-Accelerated State Expansion, CPAIOR 2026, DOI 10.1007/978-3-032-27242-3_34 (CPKB 2026-tardivo-gpu-accelerated-state-expansion-for-anytime-exac, fulltext lines 279–403). Study its sequential retained-fragment schedule, not its GPU speedup claims. Task 025 provides Choice continuation; task 018 repairs run outcome and process cleanup evidence.

## Boundaries

- Use a temporary sequential prototype with ordinary Space/Choice semantics and existing guidance. Freeze the best min(B, layer size) nodes at the deepest nonempty layer, expand the entire fragment before descending, retain unselected nodes and surviving successors, and check goals at every committed depth. Preserve fragment and Choice cursors across incumbent publication or suspension.
- Do not truncate pending work. Keep admissible pruning separate from ranking. Disable optional state pruning for this initial comparison so an unbounded registry cannot silently invalidate the memory analysis.
- No GPU, relaxed decision-diagram merging, generic transition DSL, deterministic replay requirement, public production engine, or broad benchmark campaign. A documented negative adoption result completes this task.
- Use a fixed small development selection and a few fragment widths selected before measuring; keep formulation, bound, rank, propagation and branching equal across comparisons. Do not tune on or rerun held-out cases.
- Allow at most 300 seconds cumulative solver wall time, measured from launch through confirmed exit, shared by implementation and independent verification including correctness runs; build time is separate. Use sequential execution and the existing 1 GiB RSS watchdog policy. Keep a task-local budget record and do not reset or consume the historical 7200-second campaign ledger.

## Done when

- [x] Tiny ordinary-Space cases establish n-ary coverage, goals at varying depths, strict incumbent improvement, stop/resume and exact finite-tree exhaustion; the complete-fragment schedule is distinguished from descending after each parent.
- [x] The prototype is compared with CABS and the existing fixed-width source/alternative cursor on the same small development selection. If temporary cursor code is unavailable, reconstruct the minimal documented control; do not relabel it as beam-stack search.
- [x] Results report checked solutions, proof/stop outcomes, time to first and best solution, runtime and peak retained state/memory where measurable. Explain the O(d*B*delta) retained-node argument and its assumptions; do not equate it with byte usage or the BSS O(B*d) claim.
- [x] A concise durable background report, indexed from the brief, records replay commands, source references, the bounded results and an adopt/defer/reject conclusion. Any production follow-up is proposed separately; no production engine is added by this study.

## Validation

- Read and validate the sequential schedule against the primary CADDS source. Use tiny exhaustive or independently checked cases for correctness and the existing TSPTW checker for benchmark incumbents.
- Retain enough prototype source and invocation detail with the report to reproduce the comparisons without relying on /tmp survival. Reserve part of the shared 300-second allowance for independent verification and report incomplete or unsupported measurements honestly.

## Result

Retained a replayable standalone sequential CADDS/cursor/CABS study with checked coverage and bounded development comparisons. Results support deferring a production engine; no production search API or library was changed.

Validation:

- Independently reviewed the complete task-030 candidate at Wf5b649718be1301c and expected HEAD. CADDS follows the primary source's frozen whole-fragment schedule; original Choice continuation, sound pruning, stop/resume, ownership, finite coverage and memory qualifications are supported. Independent compilation succeeded in 2.459331292 seconds, with deployment-target warnings only. The semantic witness, eight selected tiny rows and seven frozen development controls passed. Rechecked all 25 original tiny rows and 30 development rows offline, accepting 19 and 48 publications; regenerated compact artifact matches exactly, and report numerical claims match records. Final shared ledger usage is 61.300001249008346 seconds, including 11.879608832939994 verification seconds: 76 launches, 76 reaps, no pending processes or launch records. Historical 016/028 ledger hashes remain unchanged. Validation wrote only temporary binaries, captures and analyses; candidate Git status and diffs still match the immutable snapshot. Production checks appropriately reuse the unchanged task-029 baseline. Evidence remains limited to the frozen development selection, sampled RSS and standalone ASan executable linked with uninstrumented Release libraries; the report states these limits and supports defer.
- Implementation compilation 12.985523124982137 seconds plus independent compilation 2.459331292 seconds, counted separately from the final shared 61.300001249008346-second solver ledger.
