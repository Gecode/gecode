# Suffix-compatible TSPTW repair

The existing transition model and LNS recipe can repair a fixed TSPTW segment while retaining its prefix and suffix. The repaired boundary can have different location, time and cost. Both native BAB and widening CABS found the checked neighborhood optimum on the tiny cases. The one-attempt recipe publishes its inner engine's first candidate; draining that engine can find a better repair. The two selected development neighborhoods produced no improvement. These observations support feasibility, with no production LNBS integration or performance claim.

Kuroiwa and Beck's [LNBS paper](https://doi.org/10.4230/LIPIcs.CP.2023.23), section 3.1 and Algorithm 2, checks the retained suffix from each repaired state through transition preconditions, state constraints and a base case. It does not require the original endpoint state. Algorithm 2 can return an improving rollout before completing its neighborhood. Algorithm 3's accompanying prose requires the original full search space for global proof. I read the original [derived full text](/Users/zayenz/lab/knowledge/cpkb/sources/derived/papers/2023-kuroiwa-large-neighborhood-beam-search-for-domain-indepe/fulltext.md), lines 320–438; CPKB reported a fresh, complete generation. This study uses a fixed-length permutation neighborhood, while the paper permits variable-length repairs.

The temporary prototype keeps the first customer, frees the next three positions, and fixes every later customer and the depot return. It uses unchanged [TSPTW::Transition](../../../examples/tsptw-transition.hh) with OFF residual inference. Native repair roots use EAGER positions. `SpacePortfolioAdapter::neighbourhood_root` creates a fresh root, posts strict improvement against the checked incumbent, and fixes the complement groups. BAB and CABS then search that root; CABS uses PREFIX guidance throughout.

Explicit replay calls `status`, `prepare`, `status`, obtains current actions and a fresh choice, commits the requested action, then checks status. It replays the prefix and segment, clones the stable boundary, and applies the complete suffix including return. It never retains a choice across a changed Space. EAGER and LAZY replay agree with independent exact checking.

The integer fixture has incumbent `1 2 3 4 5 6`, prefix customer `1`, and suffix `5 6 0`. Prefix travel costs 2; arrival at customer 1 is 2, followed by waiting until time 10. All six segment permutations were independently enumerated with `misc/tsptw-check.py` and exact `Fraction` arithmetic. The suffix cost includes its changed connecting arc and depot return. Infeasible rows show arithmetic totals, not feasible objectives.

| Segment | Boundary (location, time, prefix cost) | Segment cost | Suffix cost | Total | Suffix result |
|---|---|---:|---:|---:|---|
| `2 3 4` | `(4, 34, 26)` | 24 | 10 | 36 | feasible |
| `2 4 3` | `(3, 20, 12)` | 10 | 5 | 17 | feasible |
| `3 2 4` | `(4, 13, 5)` | 3 | 10 | 15 | feasible |
| `3 4 2` | `(2, 21, 13)` | 11 | 54 | 67 | late customer 5: 71 > 45 |
| `4 2 3` | `(3, 22, 14)` | 12 | 5 | 19 | feasible |
| `4 3 2` | `(2, 14, 6)` | 4 | 54 | 60 | late customer 5: 64 > 45 |

Cost 15 is the neighborhood optimum; its return time is 23, versus 44 for the incumbent. Costs 17 and 19 also demonstrate a valid changed boundary location. The two cheaper segments ending at customer 2 leave customer 6 executable, so replay reaches a stable boundary before rejecting retained customer 5. A decimal fixture divides every cost and window by ten; it reproduces the same decisions with exact objectives 3.6 → 1.5. Every emitted full tour passed the independent checker. Drained BAB publishes 17 then 15; drained CABS publishes 15 and continues to natural exhaustion. CABS with maximum width 1 also returns 15, but reports stopped and supplies no completion proof. Both engines resume after an ordinary node stop. Local exhaustion proves only the fixed neighborhood.

The actual `SpacePortfolioLnsRecipe` uses one finite attempt, fixed complement groups, repeatable BAB/CABS factories, and the existing `RandomLnsThenComplete` worker policy with target 1. Its complete asset (identity 100) receives a fresh unrestricted original-model root, with no initial incumbent or neighborhood restrictions. Its LNS asset has identity 200. The [recipe implementation](../../../gecode/search/portfolio.cpp), lines 2543–2640, calls inner `next()` once and destroys the transient engine. The factory runs twice: validation and execution.

| Repair engine/probe | Publications `(asset: cost)` | Outcome |
|---|---|---|
| BAB | `100:36`, `200:17`, `100:15` | unrestricted complete asset closes globally |
| CABS | `100:36`, `200:15` | unrestricted complete asset closes globally |
| BAB local failure limit 1 | `100:36`, `100:17`, `100:15` | local repair stops; complete asset still closes |
| Cancellation after first publication | `100:36` | incomplete; no exhaustion |

Independent enumeration of all 720 full tours confirms global optimum 15 on this fixture. That separate check corroborates the complete asset's result; the LNS asset has no coverage authority. The facade requires a complete recipe. Clone-mode local roots, a transferred CABS root, and transferred recipe roots all finished with zero tracked Spaces alive. Each local engine received an external incumbent that was then destroyed. Each caller-owned portfolio publication was destroyed before checking that the portfolio still retained the same incumbent; later publications strictly improved it.

The final capture records the following single-run development observations. Both seeds were independently checked. The closed comparison uses exactly the frozen `closed-size-bab` arguments from `examples/data/tsptw/cp-baseline-tuning.json`: `--formulation closed --engine bab --circuit-propagation dom --cost-branching size`, with the same seed. No LNS default was selected or retuned.

| Input / engine | Best | Search ms | Nodes | Peak tracked Spaces | Process peak RSS bytes |
|---|---:|---:|---:|---:|---:|
| Dumas `n20w20.001`, repair BAB | 378 | 3.205 | 6147 | 18 | 2441216 |
| Dumas, repair CABS | 378 | 10.742 | 22244 | 902 | 5275648 |
| Dumas, closed BAB | 378 | 0.245 | 7 | unavailable | unavailable |
| AFG `rbg010a`, repair BAB | 671 | 0.433 | 986 | 9 | 2310144 |
| AFG, repair CABS | 671 | 1.491 | 3341 | 155 | 2850816 |
| AFG, closed BAB | 671 | 0.224 | 17 | unavailable | unavailable |

Repair searches exhausted their restricted neighborhoods with no publication; closed BAB proved global optimality. Their search scopes and propagation differ. Dumas repair incumbent replay took 0.042–0.043 ms, root creation 0.0044–0.0045 ms, complement fixing 0.0026–0.0053 ms, and total neighborhood setup 0.025–0.028 ms. AFG took 0.029–0.030, 0.0027–0.0034, 0.0025–0.0026, and 0.017–0.019 ms respectively. Closed preprocessing took 3.306 ms and 0.387 ms. Tiny LAZY replay measured root creation 0.0007–0.014 ms, prefix replay 0.0008–0.0045 ms, segment replay 0.0015–0.0034 ms, and suffix clone plus rollout 0.0026–0.016 ms. Tiny local BAB's first setup was 0.587 ms; subsequent setup observations were about 0.013–0.015 ms. These timings include instrumentation and initialization and are not repeated performance estimates.

Completed position paths retain `n` values including return (7, 21 and 11 here); the customer tour vector has `n-1` values. EAGER allocates all 7 positions at the tiny boundary; LAZY allocates 4 there and all 7 after successful completion. Space counts observe only the temporary `Tracked` subclass. macOS `getrusage` RSS includes process/runtime/library storage; it does not isolate paths or prove a general memory advantage. Launch-through-reap captures also include startup and waiting: final normal samples took 0.0066–0.312 s, distinct from native search timings.

Concrete gaps remain. `Transition::actions()` enumerates applicable visits without consulting fixed EAGER position domains; `commit()` rejects conflicting fixed values before changing time/cost. Thus even this three-position neighborhood can enumerate many doomed alternatives. Fresh LAZY roots expose no public future-position materialization seam, so the fixed-group adapter cannot post the complement there. Separately, the recipe discards inner continuation after its first publication, so a BAB repair can publish 17 while its drained reference reaches 15. The checked development cases show no quality gain, so this study recommends no production follow-up. A future request for LAZY repair or retained inner continuation should receive its own bounded review.

All prototype files and raw captures are in `/tmp/open-search-015-study`; authoritative final captures are `results/final`, with saved full tours under `results/final/tours`. `runner-contract-checks.json` confirms an unchanged-name resume performed no launches or capture changes, while changing the cap to 3 was rejected. Partial named runs were resumed during collection. Saved analysis rechecks every tour and all six neighborhood alternatives without launching a solver.

`launch-ledger.jsonl` was created before the first launch and includes all 51 launches: 4 pilot, 23 study and 24 final, including both runs' deliberate timeout and invalid-mode failure. Implementation launch-through-reap totals **1.2248288770206273 s** (pilot 0.41784162499243394, study 0.24608321001869626, final 0.5609040420094971). The maximum launch took 0.37867308300337754 s; none remain pending. Normal caps are 4 s with 0.5 s reaping reserve; deliberate timeout caps are 0.001 s. The runner enforces implementation ≤300 s, verification ≤300 s and combined ≤600 s. At implementation handoff, verification usage was 0 s, leaving its full 300 s reserve. Compilation is separate: three attempts total **4.9647195829602425 s**. The first duplicate-header include failed; two subsequent builds succeeded. `compile-first-attempt.json` and `compile-history.jsonl` retain commands, diagnostics and timings. The existing libraries emitted macOS deployment-target warnings; the current machine ran the resulting binary successfully.

For independent replay, use the existing fixture files and a separate verification capture name. Keep one runner writer. These commands neither edit the repository nor reset the ledger:

```sh
cd /Users/zayenz/.codex/worktrees/2d06/gecode
export UV_CACHE_DIR=/tmp/open-search-015-study/uv-cache
uv run --script /tmp/open-search-015-study/run-study.py build
uv run --script /tmp/open-search-015-study/run-study.py run --name verification-015 --role verification --cap 4 --max-samples 8
uv run --script /tmp/open-search-015-study/run-study.py run --name verification-015 --role verification --cap 4
uv run --script /tmp/open-search-015-study/run-study.py analyze --name verification-015
python3 misc/tsptw-check.py check /tmp/open-search-015-study/fixtures/integer.txt /tmp/open-search-015-study/results/final/tours/integer-replay-lazy-2.tour --cost 15
```

The same `run` invocation resumes unfinished samples and skips terminal captures. Use a new name if commands, caps, inputs, tours or binary contents change. `results/*/config.json` records the exact commands and content digests. `results/final/analysis.json` reports 24 completed expected outcomes with no failures or unfinished samples.
