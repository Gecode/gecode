# Common-pattern comparisons

This compares the `common-pattern.cpp` model with the positional encoding and
prefix filtering in Kemmar et al.,
[PREFIX-PROJECTION Global Constraint for Sequential Pattern Mining](https://loudni.users.greyc.fr/Papers/cp15final.pdf),
CP 2015, DOI [10.1007/978-3-319-23219-5_17](https://doi.org/10.1007/978-3-319-23219-5_17).

The comparison asks how open-list DFA propagation behaves relative to a padded
array using DFAs and a padded array using the paper's Algorithm 2. It measures
complete enumeration of **nonempty patterns present in every input sequence**:
100% support, with each sequence counted once regardless of how many embeddings
it contains. Matching repeated symbols consumes distinct input positions.

## Models

| Variant | Representation | Subsequence filtering |
| --- | --- | --- |
| `open-dfa` | `OpenIntVarList`, following the example | One open regular constraint per input |
| `padded-dfa` | Fixed `IntVarArray`, zero termination | One ordinary regular constraint per input; zeroes only at the end |
| `paper-projection` | Fixed `IntVarArray`, zero termination | Reimplementation of Algorithm 2, specialized to 100% support |

The projection model maintains one earliest-match cursor per input and intersects
the items available in their remaining suffixes. It removes unavailable items
from every remaining pattern position. Empty suffixes still support the current
prefix and allow termination. The database is immutable and shared; the current
projection is copied with each space. Parent spaces and recomputation supply the
projection history instead of an explicit stack inside the propagator. There is
no general minimum-support or gap constraint added to Gecode.

All variants initialize item domains from the actual database alphabet, following
the paper's Section 4.1. The example's A..Z domains give the same open DFA model
after initial propagation. All variants use ascending, left-to-right item
decisions and single-threaded DFS.
The open brancher interleaves closure and growth; the fixed branchers select zero
before another item. Propagation can nevertheless produce different trees. The
ordinary and open regular implementations also differ, so an open/padded timing
ratio does not isolate variable allocation cost alone. Nodes and propagation
calls measure different work across these implementations.

## Relationship to the paper

The paper's Section 5 enumerates all patterns, uses the longest input as its
positional bound, varies support thresholds, and reports CPU seconds on an Intel
Xeon X5670 with 24 GB RAM and a one-hour limit. This runner defaults to that
length-bound convention. `--bound shortest` gives all variants the tighter bound
used by the common-pattern example; it preserves solutions at 100% support.

Built-in cases include Table 1's exact four-sequence database, the example's
database, repeated items, disjoint alphabets, increasing input counts, increasing
input lengths, and increasing common-pattern lengths. Scaling databases are
seeded, planted-pattern workloads. They are **not** the paper's `data-200K`
generator or its other evaluation datasets. Restrictions include the example's
motif and Section 5(d)'s exact RE10, RE14 and RE2 expressions.

This is an algorithm and representation comparison, **not a reproduction of
Tables 3–4 or the published timing curves**. The paper's project link,
<https://sites.google.com/site/prefixprojection4cp/>, redirected to Google
sign-in when checked on 2026-10-13. The authors' implementation and dataset
bundle could not be retrieved. `paper-projection` must not be labelled as their
binary. BAB longest-pattern search is excluded because the paper enumerates
all patterns. Runtime ratios against its historical seconds would be misleading.

Reproducing the complete published experiment would require the original data
and implementation, arbitrary support thresholds in the compared model, and
integer-item input beyond this example's A..Z alphabet. Alphabet truncation,
sampling sequences, or using 100% support would change that experiment. Custom
A..Z databases can be compared here without changing the example's modelling API.

## Build

Run these commands from the repository root. The benchmark is compiled separately
against the normal Gecode libraries; no CMake targets or test infrastructure are
added. The commands work with the shared-library build on macOS and Linux.

```sh
cmake -S . -B build/common-pattern-benchmark -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
  -DGECODE_ENABLE_SET_VARS=OFF -DGECODE_ENABLE_FLOAT_VARS=OFF \
  -DGECODE_ENABLE_FLATZINC=OFF -DGECODE_ENABLE_CPPROFILER=OFF \
  -DGECODE_ENABLE_MPFR=OFF
cmake --build build/common-pattern-benchmark --target common-pattern -j 6
c++ -std=c++17 -O3 -DNDEBUG \
  -I build/common-pattern-benchmark -I . misc/common-pattern/compare.cpp \
  -L build/common-pattern-benchmark \
  -Wl,-rpath,"$PWD/build/common-pattern-benchmark" \
  -lgecodeminimodel -lgecodeint -lgecodesearch -lgecodekernel -lgecodesupport \
  -o build/common-pattern-benchmark/bin/common-pattern-compare
```

Use the same compiler and optimized build for all variants. Keep the executable
in `BUILD/bin/` and its Gecode libraries in `BUILD/`; the runner fingerprints
that layout's libraries to reject resume after a changed build. Rebuild the
companion after editing its source. `--build-label` records compiler and flags;
it does not check that the supplied label describes the build accurately.

## Collect and analyze

```sh
uv run --script misc/common-pattern/benchmark.py run \
  --name release-max --binary build/common-pattern-benchmark/bin/common-pattern-compare \
  --build-label 'AppleClang 21; Release; -O3 -DNDEBUG' --dry-run
uv run --script misc/common-pattern/benchmark.py run \
  --name release-max --binary build/common-pattern-benchmark/bin/common-pattern-compare \
  --build-label 'AppleClang 21; Release; -O3 -DNDEBUG'
uv run --script misc/common-pattern/benchmark.py analyze --name release-max
```

The default scaling suite has 24 cases, three variants and five repetitions:
72 correctness invocations followed by 360 measured invocations, serially, with
a 30-second timeout per invocation. Use `--suite smoke --repetitions 2` for a
small check. `--limit N` bounds the number of new measured samples collected in
one call; correctness runs still precede measurement. Repeat the same command
to resume. Use a new name when changing the bound, seed, inputs, timeout, build,
or repetition plan. Only one process should write a named run at a time.

Before timing, every variant prints its patterns for validation. The runner
rejects duplicate, infeasible or language-invalid patterns and requires identical
solution sets across all variants. It independently enumerates subsequences of
the shortest input when its length is at most 16, checking completeness as well.
Every built-in case meets that oracle bound. Larger custom inputs get feasibility
and cross-implementation checks; completeness is not independently established.

Measured invocations count solutions without printing them. The target timer
includes model and DFA construction, DFS construction, complete enumeration and
solution disposal; it excludes input parsing, process startup and final engine
destruction. End-to-end wall seconds include these costs and input I/O. Validation
invocations warm filesystem/library caches but are excluded from timing. Variant
and repetition order is shuffled with the saved seed; there is no concurrent
measurement or CPU pinning. Very small timings are useful for exercising the
runner, not for deciding a performance ranking.

The named directory under `build/common-pattern-results/` contains configuration,
generated inputs, validation evidence, individual sample JSON, and stdout/stderr.
Completed records are replaced atomically and remain unchanged on resume. Missing
or malformed records are unfinished and rerun; partial output for that unfinished
invocation is replaced. Errors, invalid output and timeouts are terminal records
retained on resume, with a nonzero collection exit status. A new run name retries
them without selecting away failures. Analysis does not execute the target and
reports completed, failed and unfinished samples, with median and range in
seconds. Timed-out work is excluded from successful timing summaries, never
assigned the timeout value as a runtime. Keep host conditions comparable when
resuming across sessions.

For an existing database, provide one nonempty A..Z sequence per line:

```sh
uv run --script misc/common-pattern/benchmark.py run \
  --name custom --binary build/common-pattern-benchmark/bin/common-pattern-compare \
  --build-label 'your compiler and optimized flags' \
  --database /absolute/path/sequences.txt --language re10
```

Raw timings and generated data stay in the ignored build tree. A comparison
report should name the database, support, restriction, length bound, build,
machine and timing boundary, and retain completion counts beside any timing ratio.

## Bounded first comparison

On 2026-10-13, the 24-case scaling suite completed with both length bounds:
720/720 measured samples succeeded, with 144/144 independent enumeration checks
and no disagreements. Each configuration used five repetitions, seed 20261013,
AppleClang 21, a Release Gecode build, and `-O3 -DNDEBUG` for the companion on
macOS 26.6.2 / arm64. These were serial local runs, without CPU pinning.

Selected medians below are **model construction plus exhaustive search in
milliseconds**, with the paper's maximum length bound. Raw per-sample ranges,
node counts and propagation counts are in the named run `release-max-100`.

| Case | Open DFA | Padded DFA | Paper projection |
| --- | ---: | ---: | ---: |
| 128 inputs, no language restriction | 8.227 | 7.766 | 0.450 |
| 128 inserted input items, no restriction | 3.915 | 4.106 | 0.281 |
| Common pattern length 12, no restriction | 13.292 | 11.578 | 3.228 |
| RE14 workload | 2.922 | 1.730 | 0.719 |

The projection implementation was cheaper on these workloads. Open DFA did not
show a consistent timing advantage over padded DFA. Its useful modelling
property remains the absence of padded decision positions; a runtime advantage
is not established. These small planted workloads do not establish performance
on the paper's large mining databases or at lower support thresholds.

The matching shortest-bound run is `release-shortest-serial`. Resume, partial
collection, malformed-record recovery, changed-configuration rejection, timeout,
execution error, invalid output, and analysis of failures were exercised
separately. An additional 30 seeded small databases, each with and without the
motif, matched the independent oracle across all three variants and the actual
`common-pattern` executable. No repository tests or CMake test targets were added.
