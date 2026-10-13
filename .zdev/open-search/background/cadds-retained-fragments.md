# Sequential CADDS retained-fragment study

Defer a production CADDS engine. The sequential schedule preserves finite-tree coverage and uses far fewer live Spaces than native CABS on these three TSPTW cases. The bounded comparison also exposes a quality tradeoff: CABS finds the better Gendreau incumbent, and the cursor proves the Dumas optimum while retaining fewer Spaces. Two short repetitions do not establish a general production advantage.

The study uses clean baseline `a40dd5f238f93aeb7d4516cac430dc6cf3b5a23f`, Apple clang 21.0.0, arm64 macOS 26.6.2 and the existing Release build. Durable source is `misc/tsptw-cadds-study.cpp` and `misc/tsptw-cadds-study.py`; `examples/data/tsptw/cadds-comparison.json` retains the frozen configuration and all 30 checked development rows. The report describes implementation evidence; independent verification appends its own charges to the same ledger.

## Schedule and continuation

Tardivo, Michel and van Hoeve select the deepest nonempty layer, freeze its best `min(B, size)` entries, expand the entire fragment, retain the unselected entries and install surviving successors at the next depth. Algorithm 2 separates sound successor pruning from fragment ranking. This study follows that sequential schedule, with optional state pruning disabled. [Complete Anytime Decision Diagram Search with GPU-Accelerated State Expansion, Algorithms 1–2 and Proposition 1](https://doi.org/10.1007/978-3-032-27242-3_34), CPKB paper `2026-tardivo-gpu-accelerated-state-expansion-for-anytime-exac`, original fulltext lines 279–403.

The local `advance(unsigned commit_quota)` returns an owned solution, a suspension or exhaustion. Selection moves stable entries into one frozen fragment, ordered by lower rank and unique insertion order. Expansion uses `gecode/search/seq/choice-expansion.hh`: the source retains its original Choice and alternative cursor; new incumbent cuts apply to children with older source revisions. The final alternative transfers the source to the child while keeping its Choice alive through commitment, propagation and feedback. Classification handles failed, solved and branching states, publishes strict improvements in optimization mode and retains every surviving branching child. Coverage mode applies no incumbent cuts and publishes every tour.

An incumbent return, preparation suspension or ordinary stop leaves the active fragment and staged successors intact. CADDS selects another layer only after every original alternative of every frozen source has finished. Empty pending layers and no active fragment establish exhaustion. Goals are checked at the root and at each committed depth. Rank selects work; only the admissible bound and the compatible Space cut prune it.

The cursor control retains ancestor source/alternative continuations and descends after the next page of `B` surviving children. It handles goals when generated; a goal counts toward that page even though it needs no retained branching entry. Its pages follow original source/alternative order and do not use the supplied rank to reorder work. It is the minimal cursor described in the [014 continuation study](beam-stack-regeneration.md), adapted to native ChoiceExpansion without its border experiment, descriptor keys or instrumentation path copies. Native CABS uses its actual width-one/doubling passes with no maximum-width cap. No production engine, Model dependency or public API was added.

## Frozen comparison

Before the first solver launch, the runner validated these durable published copies against the existing development manifest:

| Development case | SHA-256 |
| --- | --- |
| Dumas/n20w20.001.txt | `83c354a30228db900577252a845fe2671175cb5e621a9a1061a01f9b56aaa748` |
| GendreauDumasExtended/n20w120.001.txt | `633d3be7abef529e7faf8bd6094a40f548d8310f41cb2f3014d21b550ebf5fba` |
| SolomonPotvinBengio/rc_201.1.txt | `e0c1dfb4fb08559c01c3ab3193316269fa8a3a215d529e0cc893ef300659766f` |

The fixed configurations are CADDS B=1/B=4, cursor B=1/B=4 and native CABS. Each case has two repetitions; the second reverses configuration order. Every method uses the same lazy Transition model, MIN_ARCS bound, rank equal to bound, residual propagation OFF, state pruning OFF, ascending native visit branching, no seed and one thread. An ordinary 1,500 ms TimeStop and a 2-second outer cap give 30 rows and 60 nominal outer-cap seconds. No held-out input was inspected or run, and no configuration changed after results arrived.

All 30 rows completed capture successfully: 26 ordinary stops and four Dumas cursor optimality proofs. There were no outer timeouts, RSS stops, execution errors or invalid-output rows in this matrix. The independent Fraction checker accepted all 48 incumbent publications, which strictly improve within each run. Costs in the table are exact original units. Both repetitions agree on each final cost and peak live Space count. Times are the arithmetic mean of the two native publication clocks; wall time is mean launch-through-exit time. RSS gives the two-sample range in MiB. A dash means no incumbent, not infeasibility.

| Case | Method | Final cost / outcome | First / best publication, ms | Wall, s | Peak live Spaces | Sampled RSS, MiB |
| --- | --- | --- | ---: | ---: | ---: | ---: |
| Dumas | cadds-b1 | 378 / stopped | 55.25 / 55.25 | 1.5563 | 153 | 2.73–2.75 |
| Dumas | cadds-b4 | 378 / stopped | 4.39 / 4.39 | 1.5617 | 394 | 3.38–3.48 |
| Dumas | cursor-b1 | 378 / optimal | 549.41 / 596.51 | 1.5622 | 21 | 2.34–2.38 |
| Dumas | cursor-b4 | 378 / optimal | 539.19 / 585.39 | 1.4716 | 44 | 2.44–2.44 |
| Dumas | cabs | 378 / stopped | 15.69 / 15.69 | 1.5748 | 55,660 | 167.97–174.55 |
| Gendreau | cadds-b1 | 272 / stopped | 1349.76 / 1430.57 | 1.5695 | 180 | 2.80–2.91 |
| Gendreau | cadds-b4 | 274 / stopped | 7.53 / 60.18 | 1.5487 | 545 | 3.70–3.73 |
| Gendreau | cursor-b1 | — / stopped | — / — | 1.5394 | 19 | 2.25–2.31 |
| Gendreau | cursor-b4 | — / stopped | — / — | 1.5727 | 50 | 2.44–2.45 |
| Gendreau | cabs | 267 / stopped | 1433.72 / 1433.72 | 1.5450 | 31,537 | 56.56–63.50 |
| Solomon | cadds-b1 | 444.5425 / stopped | 265.44 / 271.31 | 1.5692 | 149 | 2.80–2.83 |
| Solomon | cadds-b4 | 444.5425 / stopped | 748.20 / 748.20 | 1.5706 | 492 | 3.69–3.75 |
| Solomon | cursor-b1 | — / stopped | — / — | 1.5542 | 15 | 2.31–2.38 |
| Solomon | cursor-b4 | — / stopped | — / — | 1.5716 | 40 | 2.39–2.44 |
| Solomon | cabs | 444.5425 / stopped | 261.43 / 261.43 | 1.5771 | 58,335 | 177.27–178.41 |

CADDS B=4 finds the first Dumas and Gendreau incumbents sooner than B=1, but finds the Solomon incumbent later. CABS obtains Gendreau cost 267; CADDS ends at 272 or 274 without a proof. The Dumas cursor runs prove cost 378; all other methods stop with that same feasible cost. CADDS and CABS find Solomon cost 444.5425 without proving it optimal. The cursor finds no incumbent on Gendreau or Solomon in this allowance. These differences constrain the adoption decision more than a single memory ratio would. The compact JSON retains each row's native node, failure, depth, propagation and restart counts, clone count and measured bound calls/time. OFF performs no residual inference; the reported inference counts/time are zero. CABS frontier subdivisions and exact retained bytes remain unavailable.

## Retained states and memory

The paper's `O(d * B * delta)` argument counts retained states. Selecting the deepest nonempty layer leaves its successor layer empty at fragment start. Expanding a whole fragment creates at most `B * delta` children. Search exhausts a deeper layer before a shallower fragment can replenish it. For finite maximum depth `d` and bounded original branching factor `delta`, each pending layer therefore contains at most one fragment's successor batch, apart from the root. Current-fragment parents, staged successors, an in-flight child, incumbent and caller-owned publication add bounded ownership terms. Deferred work is never width-truncated.

Measured CADDS peak pending counts were 151/390 for Dumas B=1/B=4, 178/543 for Gendreau and 148/491 for Solomon. Peak staged counts were 19/62, 20/64 and 19/60, respectively; peak active source counts were 1/4. These separate maxima need not occur together. Pending layers can contain more than `B` entries. The live Space counter includes pending states, active parents, in-flight children, retained incumbent and caller-owned publications, including CABS restart roots. It uses constant-time construction/destruction counters and no live-Space registry.

These counts do not measure arbitrary Space bytes. Native Choice action vectors, clone-local visit lists, depth metadata, vectors and allocator overhead can grow independently. Choice payload and container/Space byte costs are unavailable here. CABS does not expose its separate pending/source/staged counts. The cursor's retained source maxima ranged from 15 to 49; its pending-layer counter is zero because it retains ancestor pages instead. This study does not establish BSS's `O(B*d)` regeneration claim.

The existing capture policy samples process-group RSS every requested 50 ms with a 1 GiB watchdog. Actual probe intervals differ: the largest development sample gap was 177.49 ms. All development rows had measured RSS; some tiny processes exited before a nonzero RSS observation, which remains null. RSS includes native model regions, libraries and allocator retention and is separate from Space counts. All outer-cap overshoots and RSS overshoots were zero. Native TimeStop checks do not interrupt propagation or destruction: observed native summaries reached 1533.51 ms despite the equal 1500 ms setting. Startup, ps probing, waiting and teardown remain in end-to-end wall time.

## Correctness and budget evidence

The ordinary-Space witness covers n-ary alternatives, one failed child, shallow and deeper goals, failed and solved roots, strict improvement, an intentionally misleading rank with a sound bound, preparation suspension, one-commit slices, ordinary stop/resume during an active fragment and exact finite-tree exhaustion. All four CADDS/cursor width combinations cover the expected three goal identities. Final measured optimization sequences are `[9,5,3]` for both CADDS widths and `[9,3]` for both cursor widths. The CADDS B=4 expansion trace `[0,2,1,4,3]` shows that node 2 generates node 4, then selected parent 1 expands before successor 4. A direct cut witness changes fresh root actions from `[9,1,2]` to `[1,2]`: fresh alternative 1 selects node 2, while original alternative 1 still selects node 1.

Twenty-five tiny TSPTW rows cover all five existing inputs: CADDS/cursor widths 1 and 4 emit every independently enumerated feasible tour exactly once with cuts disabled; native CABS optima/infeasibility agree. All 19 tiny publications pass the Fraction checker. Saved streams are rechecked whenever analysis runs. The first tiny row was collected alone, then resume collected only the other 24. A completed rerun left all raw/sample hashes unchanged and launched nothing. Reusing the name with the ASan binary was rejected before launch because its binary hash changed.

The new frontier ownership was also checked with the standalone AddressSanitizer semantic witness, including a final rerun that emits each configuration's actual cost sequence. It passed without diagnostics. The executable was instrumented; the Release Gecode libraries were not, so this is not a full-library sanitizer result. Seven compilation attempts, including three failed enum-name builds, took 12.985523 seconds in total, separately from solver time. Linker deployment-target warnings were present; both final normal and ASan builds succeeded.

The first sandboxed capture could not execute ps. Existing capture killed and reaped the owned child and charged 0.006755 seconds as interrupted. Its empty raw streams remain under `results/witness-v1`; the successful witness uses a separate name. Subsequent captures used sandbox escalation for the required RSS probe. No unresolved process remains.

The fresh shared ledger is `/private/tmp/open-search-030-budget.jsonl`: task `open-search-030`, ceiling 300 seconds, independent verification reserve 120 seconds and cleanup reserve 20 seconds. Implementation has 60 solver launches charged from launch through confirmed exit: 2.730918 seconds correctness and 46.689475 seconds development, **49.420392 seconds total**. This is an implementation-only snapshot. The verifier appends its own charges; the final completion result owns the final shared total. Admission includes remaining nominal frozen work and both reserves, and any pending launch blocks later launches. Builds remain separate.

The historical `/private/tmp/open-search-016-budget.jsonl` (7200-second campaign) and finished `/private/tmp/open-search-028-budget.jsonl` were read and hashed only; they were not appended to or reset. Task 029's immediately preceding independently verified full check, six focused native selections, inherited open-list/regular/Model/portfolio selections and native/inherited examples provide the unchanged-production baseline. Task 030 did not rerun those checks. Its attributed source delta contains only this standalone study and compact data artifact. After measurement, the witness output was corrected to emit the collected per-configuration cost sequences, and its depth comment was corrected. The scheduling, model, measurement and development-output code remained identical. The original measured source is retained at `/private/tmp/open-search-030-study/measured-study.cpp`; original binary/configuration hashes and raw records were preserved. Final witness evidence uses `study-final` and `study-final-asan`. Reusing an old collection name with the final source is correctly rejected as a changed source hash; offline analysis of its retained records remains available. The [completed TSPTW measurements](../../../examples/data/tsptw/cheap-deadline-comparison.json) and [measurement documentation](../../../examples/data/tsptw/README.md) retain the distinctions between incumbent feasibility, proof, native clocks, launch clocks and missing post-kill summaries; their earlier observations are not new task-030 passes.

## Replay

Run from the checkout root with the existing Release libraries. Source and the three inputs are durable; the old 014 temporary prototype is unnecessary. The collector imports the existing `misc/tsptw-benchmark.py` capture, task-budget and recovery functions unchanged. One writer owns each named run. Configuration, binary/source hashes, inputs, exact arguments, caps and schedule are saved before collection. A new name is required when material configuration changes; repeating a matching name skips terminal rows, including terminal failures. Interrupted attempts retain separate raw streams. `analyze` never launches a solver.

```sh
mkdir -p /private/tmp/open-search-030-replay
clang++ -std=c++17 -O2 -I build/open-search -I . misc/tsptw-cadds-study.cpp -L build/open-search -Wl,-rpath,/Users/zayenz/.codex/worktrees/2d06/gecode/build/open-search -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -o /private/tmp/open-search-030-replay/study
python3 misc/tsptw-cadds-study.py run --name replay-development --binary /private/tmp/open-search-030-replay/study --dry-run
python3 misc/tsptw-cadds-study.py run --name replay-witness --suite witness --phase correctness --binary /private/tmp/open-search-030-replay/study
python3 misc/tsptw-cadds-study.py run --name replay-tiny --suite tiny --phase correctness --binary /private/tmp/open-search-030-replay/study
python3 misc/tsptw-cadds-study.py run --name replay-development --suite development --phase development --binary /private/tmp/open-search-030-replay/study
python3 misc/tsptw-cadds-study.py analyze --name replay-development
```

These commands append to the authorized 030 ledger and refuse admission if its remaining allowance is insufficient. If the temporary ledger no longer exists, the runner creates this task's fixed header only with exclusive creation. Captures require a usable ps probe. Use `uv run --script` in place of `python3` if desired; the script has no external Python dependencies.

The exact implementation runs used `/private/tmp/open-search-030-study/study`, names `witness-v2`, `tiny` and `development`, and the same run/suite/phase arguments above. The development capture was split with `--limit 15`, then resumed without a limit. Raw config, samples and streams are retained under `/private/tmp/open-search-030-study/results`. Offline reproduction of the committed comparison is:

```sh
python3 misc/tsptw-cadds-study.py analyze --name development --output examples/data/tsptw/cadds-comparison.json
python3 misc/tsptw-cadds-study.py analyze --name tiny
python3 misc/tsptw-cadds-study.py analyze --name witness-final
```

The exact sanitizer build/run was:

```sh
clang++ -std=c++17 -O1 -g -fsanitize=address -fno-omit-frame-pointer -I build/open-search -I . misc/tsptw-cadds-study.cpp -L build/open-search -Wl,-rpath,/Users/zayenz/.codex/worktrees/2d06/gecode/build/open-search -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -o /private/tmp/open-search-030-study/study-final-asan
python3 misc/tsptw-cadds-study.py run --name witness-final-asan --suite witness --phase correctness --binary /private/tmp/open-search-030-study/study-final-asan
```

Independent verification can compile to its own binary, run the semantic witness and selected tiny/development rows with `--phase verification --only row000,row004` (row IDs come from that suite's dry-run), and recheck every saved publication offline. `recover --name NAME` delegates conservative absence confirmation to the existing recovery routine; it never kills an unidentified process or erases its budget charge.

A separately reviewed production follow-up would need a clear ordinary-Space API and broader checked evidence that the heuristic scheduling benefits justify another engine. This task creates neither that API nor a follow-up task. The bounded result supports retaining this study as evidence and deferring adoption.
