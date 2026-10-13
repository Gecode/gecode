# Common-pattern mining and comparisons

`examples/common-pattern.cpp` mines frequent ordered subsequences, with optional
constraints on their length, items and language. Its decision variable is a list
of items of unknown length. DFS enumerates all qualifying patterns; BAB finds a
maximum-length qualifying pattern.

## The mining model

For a database of records `D`, a record supports pattern `P` when there are strictly
increasing positions in that record matching every item of `P`. Matching may skip
items. Each supporting record contributes one unit, regardless of how many
embeddings it contains; duplicate rows are distinct records. The frequency
constraint is `support(P) >= ceil(percent * |D| / 100)`.

This separates three familiar CP decisions:

- **Variables:** the pattern items and their length, bounded by the longest input.
- **Constraints:** sufficient support, minimum length, required items, and a pattern language.
- **Search:** left-to-right item choices, deciding at each prefix whether to close or extend.

The constructor expresses those constraints directly:

```cpp
pattern = OpenIntVarList(*this,database.domain(),database.max_length());
rel(*this,pattern.length(),IRT_GQ,opt.min_length());
PatternMining::frequent(*this,pattern,database,opt.support());
if (opt.language() != PatternMining::NONE)
  extensional(*this,pattern,PatternMining::language(opt.language(),database));
REG alphabet(IntArgs(database.alphabet));
for (int item : opt.required())
  extensional(*this,pattern,*alphabet + REG(item) + *alphabet);
branch(*this,pattern,INT_VAR_NONE(),INT_VAL_MIN());
```

`frequent` is private example support, not a new Gecode library constraint.
Length, membership and regular constraints apply to the **pattern**. They do not
constrain which input positions match it. This follows the distinction between
pattern, cover, embedding and output constraints in Negrevergne and Guns,
[Constraint-based sequence mining using constraint programming](https://arxiv.org/abs/1501.01178),
CPAIOR 2015. All frequent patterns, a longest pattern, closed patterns (no proper
supersequence with the same support), maximal frequent patterns (no proper
frequent supersequence), and top-k by support are different output queries. This
example implements the first two.

## Why use an open list?

The pattern's actual length is unknown. An `OpenIntVarList` constructs its
positions as needed, avoiding an end marker, unused suffix variables, and padding
transitions in additional pattern languages. Constraints follow growth; when no
frequent extension remains, the list closes. `open-route` is the smaller
introduction to construction, cloning and search.

The private frequency propagator uses prefix projection: after assigning an item,
it retains the input suffixes that can extend the current pattern. One earliest
matching cursor per supporting record suffices for ordinary subsequences. It
filters locally infrequent items from constructed positions and bounds further
growth by the remaining supporting suffix lengths. Immutable, rank-compressed
data are shared between spaces; each space owns its cursors. Sparse integer IDs
are preserved in output.

This adapts Kemmar et al.,
[PREFIX-PROJECTION Global Constraint for Sequential Pattern Mining](https://loudni.users.greyc.fr/Papers/cp15final.pdf),
CP 2015, DOI [10.1007/978-3-319-23219-5_17](https://doi.org/10.1007/978-3-319-23219-5_17),
which uses a fixed positional array and termination value. The open representation
also addresses a cost identified in Aoga, Guns and Schaus,
[An Efficient Algorithm for Mining Frequent Sequence with Constraint Programming](https://arxiv.org/abs/1604.01166),
ECML PKDD 2016, §4.2: filtering and restoring future array positions that the search
may never use. Their implementation filters only the next position and optimizes
suffix scans and state restoration. The example does not reproduce those
optimizations or establish a speedup; the comparisons below distinguish variable
representation from filtering strength.

The input scope is sequences of **individual items**, not sequences of itemsets.
Gap, span, timestamps and event attributes are outside the model. Earliest matching
alone would not suffice with embedding restrictions: in `AXBAB`, an adjacent `AB`
exists at the end although matching the first `A` cannot find it adjacently.

## Use the example

With no arguments, the example enumerates `AC` and `ABC` from `ABCBC,BABC,ABDC`,
requiring all records and the language `A.*C`. The paper's Table 1, with minimum
support two out of four records and no language restriction, is:

```sh
build/common-pattern-benchmark/bin/common-pattern \
  -sequences ABCBC,BABC,AB,BCD -support 50 -language none -search dfs
```

It enumerates nine patterns, including `AC` with support 2/4. Use `-search bab`
for a longest pattern, `-min-length 3` for a length restriction, and `-required A,C`
to require both items. Support is a percentage in `(0,100]` with up to six decimal
places; the threshold is the exact ceiling of that fraction of the record count.

`-language` accepts `none`, `motif`, `re10`, `re14`, or `re2`. The last three are
Section 5(d)'s expressions, retained as comparison presets. Explicit `-language`
overrides `-motif`; the default language uses `-motif` (enabled by default). Letters mean item IDs `A=1` through
`Z=26`. Named expressions on integer data require that same mapping; the parser
does not infer or remap dataset labels. The motif's interior and RE2's middle
position accept any item in the input alphabet.

| Language | Accepted pattern |
| --- | --- |
| `none` | Any nonempty sequence over the input alphabet |
| `motif` | `A.*C` |
| `re10` | `A*B(B\|C)D*EF*(G\|H)I*` |
| `re14` | `A*(Q\|BS*(B\|C))D*E(I\|S)*(F\|H)G*R` |
| `re2` | `(S\|T).(R\|K)` |

Here juxtaposition means concatenation, `|` means alternative, `*` means zero or
more repetitions, and `.` means one item from the actual input alphabet.

For larger databases use `-file` and select a format:

- `-format letters`: one nonempty A..Z string per line.
- `-format integers`: one sequence of positive integer IDs per line, separated by spaces.
- `-format spmf`: singleton itemsets separated by `-1`, with `-2` ending each sequence.
  For example, `1 -1 2 -1 3 -1 -2` represents ABC. SPMF metadata/comment lines
  beginning with `@`, `#`, or `%` are skipped. Multi-item itemsets are rejected;
  silently flattening them would change the mining problem.

Empty lines are skipped. Every retained row has its own support identity. Required
items use comma-separated letters in letter mode and integer IDs otherwise:

```sh
build/common-pattern-benchmark/bin/common-pattern \
  -file /absolute/path/database.txt -format spmf \
  -support 1 -min-length 3 -required 1001,2002 -language none -search dfs
```

## Build

Run from the repository root. The companion is compiled against the normal Gecode
libraries; it adds no CMake targets. Regression tests use the existing Gecode test
harness in `test/int/open-var-list.cpp`.

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

Use the same compiler and compatible build settings, including macOS deployment
targets, for the libraries and companion. Keep the executable in `BUILD/bin/`
and libraries in `BUILD/`; that layout is fingerprinted for resume. Rebuild the
companion after changing its source or private header. The build label records
compiler and flags but does not verify them automatically.

## Compare equivalent mining queries

| Variant | Pattern representation | Filtering |
| --- | --- | --- |
| `open-projection` | `OpenIntVarList` | Local item support and remaining-suffix length bound, including initial filtering |
| `padded-projection` | Fixed array, zero termination | Same extra pruning, applied to all preallocated positions |
| `paper-projection` | Fixed array, zero termination | Algorithms 1–2 reimplementation, without added initial scheduling or suffix-length pruning |
| `open-dfa`, `padded-dfa` | Open list or padded array | Per-input subsequence DFAs; require all-input support |

The first two variants help assess the representation using the same extra pruning.
Their regular constraints still propagate differently because the open constraint
sees constructed positions and the ordinary constraint sees the entire array.
The paper variant distinguishes the effect of additional pruning. Nodes and
propagation calls represent different amounts of work across implementations.
All variants use single-threaded DFS, ascending left-to-right item decisions,
and exhaustive enumeration; BAB is excluded from mining comparisons.

The default smoke matrix covers six workloads at 100%, 50%, and 25% support,
with three projection variants and five repetitions: 54 correctness invocations
followed by 270 measured invocations, serially, each with a 30-second timeout.
These small inputs exercise loss of support, repeated items and disjoint records.

```sh
uv run --script misc/common-pattern/benchmark.py run \
  --name frequency --binary build/common-pattern-benchmark/bin/common-pattern-compare \
  --build-label 'your compiler; Release; -O3 -DNDEBUG' --dry-run
uv run --script misc/common-pattern/benchmark.py run \
  --name frequency --binary build/common-pattern-benchmark/bin/common-pattern-compare \
  --build-label 'your compiler; Release; -O3 -DNDEBUG'
uv run --script misc/common-pattern/benchmark.py analyze --name frequency
```

`--suite scaling --support 100` adds database-count, input-length, pattern-length,
and the paper's regex workloads. Lower support on long inputs can produce very
large outputs; start with a bounded query. For a real database, use the same
formats as the example:

```sh
uv run --script misc/common-pattern/benchmark.py run \
  --name database --binary build/common-pattern-benchmark/bin/common-pattern-compare \
  --build-label 'your compiler; Release; -O3 -DNDEBUG' \
  --database /absolute/path/database.txt --format spmf \
  --support 5 2 1 --min-length 3 --required 1001 2002 --language none
```

`--bound max` follows the paper's longest-record bound. `--bound shortest` and
DFA variants are valid only when the computed support count requires every record.
Select DFAs explicitly with `--support 100 --variants open-dfa padded-dfa
open-projection padded-projection paper-projection`. The default companion
invocation from the previous comparison remains valid.

Before timing, the runner checks duplicate output, item/length/language constraints,
and actual record support. All variants must enumerate exactly the same set.
An independent oracle enumerates subsequences of the `n-k+1` shortest records:
every pattern supported by `k` out of `n` records occurs in at least one of those
records. The oracle runs when each chosen record has at most 16 items and their
combined subset budget is at most 200,000. Otherwise feasibility and agreement are
checked, with independent completeness explicitly unavailable. The smoke matrix
meets the oracle budget at every threshold.

Measured runs count solutions without printing. The target timer includes rank
compression, model construction, complete DFS and solution disposal; it excludes
file parsing, startup and final engine destruction. End-to-end wall time includes
those costs. Validation runs warm caches and are excluded from timing. Ordering
is shuffled with the saved seed. Small timings should not determine a performance
ranking. The example driver's runtime excludes database import; it is a separate
metric from the companion's model-plus-search timer.

Named results live under `build/common-pattern-results/`, with configuration,
normalized integer input, validation evidence, raw samples and stdout/stderr.
One writer per run; collection resumes unchanged terminal records. `--limit N`
bounds new timed invocations, while correctness runs still precede measurement.
Changed inputs, queries, build, helper, runner, timeout or repetition plan require
another name. Missing/malformed records are rerun and their partial logs replaced.
Errors, invalid output and timeouts remain terminal evidence and cause a nonzero
collection exit. Analysis invokes no targets, retains older format-1 results, and
reports failures/unfinished work beside successful medians and ranges in seconds.

The authors' project link <https://sites.google.com/site/prefixprojection4cp/>
required Google sign-in when checked on 2026-10-13. These baselines are
reimplementations, not their executable. Real dataset versions and label mappings
still need checking before comparison with published pattern counts. Historical
CPU seconds on the paper's Xeon X5670 are not a runtime baseline for this machine.

## Literature and comparison scope

Backward citation tracing from CP2015 identifies projected pattern growth,
vertical occurrence lists, CP embedding models, and regular-language restrictions.
Projection is one way to calculate support, not the definition of the problem.
The following primary sources give useful alternative views:

| Work | Relationship and relevance |
| --- | --- |
| Pei et al., [PrefixSpan](https://hanj.cs.illinois.edu/pdf/span01.pdf), ICDE 2001 | Cited by CP2015; prefix-projected pattern growth predates its CP global. The original also handles sequences of itemsets. |
| Zaki, [SPADE](https://www.cs.rpi.edu/~zaki/PaperDir/MLJ01.pdf), Machine Learning 2001 | Cited by CP2015; temporal joins of vertical occurrence lists provide a different support computation. |
| Negrevergne and Guns, [CP sequence mining](https://arxiv.org/abs/1501.01178), CPAIOR 2015 | Cited and compared by CP2015; distinguishes a global existence-of-embedding constraint from an explicit embedding formulation. Our DFA baseline is neither formulation. |
| Aoga, Guns and Schaus, [efficient CP sequence mining](https://arxiv.org/abs/1604.01166), ECML PKDD 2016 | Cites CP2015; improves projection, future-position filtering and restoration. Motivates measuring these costs separately. |
| Kemmar et al., [prefix projection and top-k](https://loudni.users.greyc.fr/Papers/Cons17final.pdf), Constraints 2017 | Extends the CP2015 line with a separate top-k query. Maximum length is not top-k by support. |
| Hosseininasab, van Hoeve and Ciré, [SPM with decision diagrams](https://arxiv.org/abs/1811.06086), AAAI 2019 | Cites later CP projection work; represents the database with an MDD and handles constraints on event attributes. It is not an open-versus-padded representation comparison. |
| Wang et al., [Seq2Pat](https://ojs.aaai.org/index.php/AAAI/article/view/21542), AAAI 2022 | Builds a declarative interface over the decision-diagram approach, with attribute constraints including gap, span, average and median. These constrain embeddings, unlike the example's pattern language. |

The CPKB search confirmed CP2015's outgoing references and incoming citations from
ClosedPattern (CP 2016) and CoverSize (CP 2017). Those two globals concern unordered
itemsets; their closure and cover algorithms do not transfer directly to sequential
mining. Locally available references also led to the later sequential papers
above, which were read on the web. The local corpus did not contain an integrated
sequential successor; that is a corpus limit, not a claim that none exists.

The resulting example keeps the query independent of support computation and
uses ordinary CP constraints for pattern restrictions. The comparison keeps the
paper's data and language presets available alongside integer/SPMF input, arbitrary
support thresholds, and independent enumeration checks. A comparison with another
miner must match item-versus-itemset semantics, support rounding, pattern and
embedding restrictions, and the output query before comparing counts or time.
There is no MDD, vertical-join or embedding-decomposition baseline here.

## Historical DFA comparison at commit 4f929ea7c3

The following measurements predate the projection-based example and broader
support queries; their raw records remain readable and are not reinterpreted as
measurements of the current implementation.

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
