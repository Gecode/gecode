+++
schema_version = 1
id = "open-search-012"
key = "tsptw-benchmark-runner"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["open-search-006", "open-search-007"]
+++
# Build a small resumable TSPTW comparison runner

## Outcome

A named run captures correct comparable TSPTW results, resumes missing work, and regenerates analysis without rerunning solvers.

## Context

Read the brief's budget/timing rules and ../background/next-search-and-benchmarks.md comparison. Start with misc/ existing uv benchmark CLIs and their subprocess measurement helpers; reuse only suitable low-level pieces. Apply the zayenz-benchmark-script skill when implementing. Do not import the unrelated FlatZinc pipeline wholesale.

## Boundaries

- One measured subprocess at a time; no new benchmarking framework or option cross-product.
- Keep collection and analysis separate, use exact data/solver/config identities and simple records. Large campaigns remain bounded by the brief.

## Done when

- [x] An executable uv run --script CLI provides preview, named run/resume and analyze. Configuration changes invalidate reuse; finished raw records remain unchanged and interrupted rows alone resume.
- [x] The runner captures checked incumbent traces, final objective, proof/incomplete/stop reason, startup/preprocessing/search wall time, expansions, peak RSS and supported frontier/descriptor/clone measurements. Missing measurements are explicit.
- [x] Every returned incumbent is independently checked. Invalid results, crashes, watchdog timeout and RSS termination are retained as failures, never converted into proof.
- [x] A pinned DIDPPy adapter runs travel-cost TSPTW with identical scaling, depot return rules, valid bounds and nonmetric reachability; its tours use the same checker. Native and DIDPPy processes share thread/time/RSS-watchdog settings.
- [x] Tiny smoke collection, resume, one intentionally missing row, configuration mismatch, timeout/failure, and offline reanalysis are demonstrated. Matrix preview calculates the full budget before execution.

## Validation

- Exercise the actual CLI with disposable named correctness runs and inspect records, then resume and analyze without target invocation.
- Compare tiny DIDPPy A*/CABS optima/infeasibility with enumeration and both native formulations.
- Keep new tests only for nontrivial record matching/parser/measurement correctness; use direct smoke checks for CLI/help.

## Result

Implemented a serial resumable TSPTW comparison runner with eight native and DIDPPy configurations, independent incumbent validation, pinned DIDPPy adapter, phase timing, and offline analysis.

Validation:

- Independent whole-task PASS against Wad702d9acbcd63ae; build, five focused tests, executable help, diff check, and budget preview passed. Forty fresh tiny-instance rows across eight configurations matched exhaustive enumeration. Resume, selective recovery, mismatch rejection, guarded offline analysis, six failure modes, RSS cleanup, and actual solver-constructor timing passed. No development or held-out campaign was launched; RSS sampling and unavailable frontier/clone measurements are documented.
