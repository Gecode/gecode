# Word branch performance review — September 2026

## Changes

Fixed mixed-domain equality reaching subsumption before mask synchronization
and signed division losing a valid quotient at the minimum-value boundary.
Progression filtering now avoids an exponential endpoint walk by conservatively
skipping affected endpoint narrowing; assigned-value checks remain exact.
N-ary logic deduplication uses sorting and a linear pass instead of quadratic
scanning, preserving XOR multiplicities.

A scoped posting cache preserves sharing across mixed Boolean/Word expression
callbacks without changing the public Boolean miscellaneous-expression vtable.
For the reviewed nested conditional DAG at depths 4, 8, and 12, actor counts
fell from 61/1,021/16,381 to 13/25/37. The adversarial progression family at
widths 12/16/20/24 fell from 63/255/1,023/4,095 closure passes to one; widths
32 and 40 also completed in one pass. These are targeted regression measures,
not claims about general solver speed.

The SMT2 reader preserves shared bindings, validates literals and declaration
scope, supports more native word operations and nullary definitions, rejects
excessive input/depth, and reports parsing, construction, and search separately.
The benchmark runner terminates timed-out process groups and validates batch
results. Campaign analysis no longer presents enumeration-versus-decision
timings as solver speedups or overwrites the historical checked-in report.

## Final verification

The combined `feature/word-vectors` worktree, including concurrent extensional
changes, built successfully in Release mode on macOS arm64. Checks passed:

- Every `Word::` test with one iteration and seed 1, including extensional tests.
- All 2,640 `Int::MiniModel::BoolExpr` tests with one iteration and seed 1.
- Reader CTest: 360 checks, six resource-limit checks, three invalid-policy checks.
- Benchmark-runner CTest: four checks, including child-process timeout cleanup.
- All 72 generated campaign decisions under all three policies: 216 matches,
  no mismatches, unsupported cases, or two-second timeouts.
- Independent review of the 72-case preflight, six-family smoke run, and campaign
  analysis changes; `git diff --check`.

## Remaining performance questions

The generated campaign establishes compatibility with these encodings, not
broad QF_BV coverage. Next measure a public corpus with identical decision
inputs and time limits across solvers, retaining end-to-end and phase timings.
Keep search, construction, and propagation effects separate. Wider words,
arrays, incremental solving, and parameterized functions remain outside this
reader's scope; the application and corpus roadmap remains the follow-up plan.
