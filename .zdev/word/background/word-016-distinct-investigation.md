# Bounds-consistent Word distinct investigation

## Question

Would one native bounds-consistent `distinct` for homogeneous unsigned bounded
Words offer enough value to justify a public constraint and production actor?

## Prototype contract

The throwaway actor enforced pairwise inequality over admitted fixed-width Word
values. It accepted only unsigned bounded Words of one width. Empty and
singleton arrays were no-ops, and duplicate handles raised `ArgumentSame`.
Compact, signed, mixed-kind, and mixed-width arrays were outside the contract.

Propagation applied the Int `Distinct::Bnd` Hall-interval algorithm directly to
the unsigned views' ranked endpoints. It maintained bounds consistency for the
ranked interval, not domain consistency over cube holes. It subscribed to
`PC_WORD_BND`, which schedules on Word value, bound, and combined domain events;
bit-only events cannot change the ranked interval. A typed endpoint represented
ranks below zero and above `UINT64_MAX`, while capacity differences saturated at
the variable count. This covered width 64 without overflowing `WordValue`. The
prototype did not add a Word-as-Int adapter, signed behavior, or a
value-consistent mode.

A direct semantic check observed duplicate-handle failure, successful width-64
full-span propagation, failure for three width-64 variables sharing two endpoint
values, and Hall pruning of a third `[0,2]` variable to 2 when two variables
occupied `[0,1]`.

## Exact model results

The model used two banks, two through four labeled registers per bank, and four
aligned eight-bit slots in each bank. Low bits 0 through 3 were zero. Bank zero
fixed bit 6 to zero and used `[0x00,0x3f]`; bank one fixed bit 6 to one and used
`[0x40,0x7f]`. No ordering constraint broke symmetry.

All three formulations produced the expected solution count and wrapping
weighted-address checksum.

| registers per bank | solutions | checksum | native nodes/fails/props | pairwise nodes/fails/props | Word+Int nodes/fails/props |
|---:|---:|---:|---:|---:|---:|
| 2 | 144 | 99,072 | 287 / 0 / 394 | 339 / 26 / 342 | 337 / 25 / 597 |
| 3 | 576 | 843,264 | 1,151 / 0 / 1,963 | 1,951 / 400 / 4,062 | 1,663 / 256 / 3,793 |
| 4 | 576 | 1,456,128 | 1,151 / 0 / 3,028 | 4,351 / 1,600 / 13,570 | 2,567 / 708 / 8,190 |

After root propagation, native used one distinct actor, pairwise used 2, 6, and
12 disequality actors, and Word+Int used one Int distinct plus 4, 6, and 8
numeric channel actors. Each formulation used one brancher.

## Runtime and retained-clone memory

The prototype was compiled with `-O3 -DNDEBUG` at repository commit
`214d83f4b7b22dbdde2eff1cd893082899e251e2` and linked to the shared libraries
from a CMake Release build under `/tmp/gecode-word016/build`.

Two independent Release campaigns ran 20 interleaved subprocess trials per size
and formulation. Subprocess startup dominated the roughly eight-millisecond
runs: the combined native/pairwise median ratios were 1.019 at k=2, 1.052 at
k=3, and 1.048 at k=4.

A second runner removed startup noise by timing 100 identical model
construction and DFS iterations inside each process. Twenty interleaved batches
per formulation produced these medians with Gecode's quicksort, as used by the
Int algorithm:

| registers per bank | native ms/solve | pairwise ms/solve | Word+Int ms/solve | native/pairwise |
|---:|---:|---:|---:|---:|
| 3 | 0.753 | 0.366 | 0.444 | 2.058 |
| 4 | 1.357 | 0.929 | 0.911 | 1.461 |

A five-second sample of a long native k=4 batch placed 1,805 of 3,715 main
thread samples directly in the Hall propagator. Its two endpoint sorts accounted
for 539 samples, followed by the Hall scans and endpoint differences. The
profile showed repeated global propagation cost, not a subprocess artifact.

Fresh subprocesses retained 2,000, 8,000, and 32,000 stabilized root clones at
four registers per bank. Every formulation exceeded the 16 MiB RSS-delta
threshold without escalation. Native measured 2,456.5 and 2,454.9 bytes per
clone in the two campaigns; pairwise measured 2,455.4 bytes per clone in both.
The native/pairwise slope ratios were 1.0004 and 0.9998. Word+Int measured about
3,484 bytes per clone. The native actor object occupied 56 bytes, and the model
object occupied 328 bytes on this build.

Raw evidence remains outside the checkout:

- `/tmp/gecode-word016/results.json`
- `/tmp/gecode-word016/results-repeat.json`
- `/tmp/gecode-word016/batch-results-quicksort.json`
- `/tmp/gecode-word016/native-k4.sample.txt`

## Initial benchmark decision

Reject the production actor. Native propagation reduced the k=3 and k=4 search
trees sharply and kept clone RSS flat, but internal repeated-search timing was
106 percent slower than pairwise at k=3 and 46 percent slower at k=4. This
misses the agreed five-percent runtime ceiling by a wide margin. No public API,
production actor, tests, or prototype code remain.

This result applies to the exact unsigned register-allocation model on the
measured host. It does not settle a future larger-workload global, value
consistency for small domains, signed ranked intervals, or domain consistency
over cube holes.

## Subsequent API decision

The production constraint was subsequently restored because the measured
register model is not representative of every workload. The public overload
defaults to `IPL_VAL`, implemented as native pairwise Word disequalities.
Callers can explicitly request `IPL_BND` to use one Hall-interval actor for a
homogeneous array of unsigned-bounded or signed-bounded Words. Compact and
mixed arrays fall back to value consistency, as does `IPL_DOM`; no domain
consistency over cube holes is claimed.

The measurements above remain the reason that bounds propagation is opt-in
rather than the default.
