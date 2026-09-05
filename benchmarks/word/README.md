# Word benchmarks

The distinct benchmark measures the shipped opt-in `IPL_BND` actor against
the default pairwise `IPL_VAL` formulation and compact Word variables with
exact unsigned Int channels. It checks the three formulations at two, three,
and four registers per bank, exercises no-pruning arrays at
8, 16, 32, and 64 variables, and adds a bounds-only 4,096-variable scale
case. Timings are batched inside the process and trials are interleaved; the
artifact records stable solver counters and median/minimum/maximum time. It
identifies the source revision explicitly because the word-016 measurements
describe a historical prototype, not this production actor. It makes no
memory claim.

```sh
cmake --build build-release --target word-distinct-benchmark
uv run --script benchmarks/word/distinct-benchmark.py \
  --binary build-release/bin/word-distinct-benchmark \
  --revision "$(git rev-parse HEAD)" --output /tmp/word-distinct.json
```

The arithmetic-progression closure check covers the aligned divisibility, GCD,
and fixed-result product-modulo family at widths 16, 24, 32, 40, and 64. Each
actor runs in its own subprocess with a wall timeout and must reach the exact
singleton with no remaining propagator at every width. The exhaustive internal
Word lifecycle test checks the helper directly without exposing instrumentation
through the production API. The runner also records a deterministic work proxy
from the helper's fixed-prefix checks, inverse refinements, and two endpoint
roundings; it is bounded by the Word width and independent of the domain span:

```sh
uv run --script benchmarks/word/progression-closure-benchmark.py \
  --binary build/bin/word-progression-closure --timeout 5
```

The register-file Element comparison uses four overlapping cube windows whose
numeric ranges leave only two supported indices. Build the current
`word-register-file` example and the same example source against the exact
parent libraries, then run the compact control and bounded candidate in
alternating order:

```sh
python3 benchmarks/word/register-file-benchmark.py \
  --parent-binary /tmp/gecode-word015-parent-release/bin/word-register-file \
  --candidate-binary /tmp/gecode-word015-candidate-release/bin/word-register-file \
  --repetitions 20 --output /tmp/word-register-file.json
```

The runner requires exact solution/checksum parity, unchanged parent/candidate
compact counters, and stable status/counters across all repetitions. All
specialty timing runners accept `--timeout`, retain individual samples when an
output artifact is requested, and report median plus the observed
minimum/maximum spread. Their one-solve subprocess timings are unsuitable for
tiny solves; batch tiny solves in-process before using them for performance
conclusions.

The product-modulo runner checks the mathematical product before modulus
reduction. Its small exact case uses `x,y in [10,30]` and modulus 17, so it
crosses several quotient regions. Singleton controls cover unsigned and signed
domains at widths 32, 33, 63, and 64, both wrapping and non-wrapping products,
and an expected UNSAT result. Signed inputs use their two's-complement Word
encodings in the multiplication. The native bounded actor applies only when
all three Word variables are unsigned; signed controls exercise the documented
cube fallback. Run the interleaved controls with:

```sh
uv run --script benchmarks/word/product-mod-benchmark.py --binary build/bin/word-product-mod --repetitions 20 --output /tmp/word-product-mod.json
```

The bounded n-ary Add scatter/gather comparison uses `word-nary-add` at 4,
6, and 8 segments. Each normal case emits one independently checked public
length-vector witness while retaining the full solution/count controls. A
four-segment total of one is an expected UNSAT control, not a runner error.
Run 20 alternating-order trials without storing artifacts in the checkout:

```sh
uv run --script benchmarks/word/nary-add-benchmark.py --binary build/bin/word-nary-add --repetitions 20 --output /tmp/word-nary-add.json
```

The runner supports two benchmark kinds without duplicating campaign machinery.
The original XOR/rotate differential compares native Word with a Boolean
decomposition. The DMA descriptor benchmark compares compact Word,
unsigned-bounded Word, compact Word with one unsigned Int channel per numeric
Word, and equivalent Int/Bool formulations at 3, 6, and 9 descriptors.

The DMA model uses 12-bit aligned bases, fixed repeated lengths, ordered
non-overlap, sorted read/write/execute flags, Element selection, and a
write-controlled limit ITE. All four formulations must have the same public
projection set. The three-descriptor case is independently enumerated; larger
cases validate a concrete first witness. Native search retains each
formulation's normal value choice. The `public-min` control aligns public
decision order and minimum-first values where the native APIs permit it, and
never branches derived channel variables. Compact Word uses MSB-first bit
branching, while bounded Word uses ranked interval splitting. The control makes
branching closer, not identical. This instance does not predict every Word
workload.

For the three-descriptor case, the runner uses an in-process batch and records
construction, explicit root propagation, and search time separately. Outer
wall time remains useful for end-to-end runs but includes process startup.

Build and run a DMA-only Release campaign:

```sh
cmake -S . -B build-word-004 -DCMAKE_BUILD_TYPE=Release -DGECODE_ENABLE_WORD_VARS=ON -DGECODE_ENABLE_EXAMPLES=ON
cmake --build build-word-004 --target word-dma-descriptor
uv run --script benchmarks/word/benchmark.py run --name word-004-release --kind dma --binary build-word-004/bin/word-dma-descriptor --repetitions 20
uv run --script benchmarks/word/benchmark.py analyze --name word-004-release
uv run --script benchmarks/word/benchmark.py report --name word-004-release
```

The default DMA discovery target is `build/bin/word-dma-descriptor`. Build
configuration is read from the executable's CMake cache when available;
otherwise it is recorded as `unverified`. Callers are responsible for supplying
a Release binary for performance comparisons.

The prior XOR route remains available:

```sh
cmake --build build --target word-benchmark
uv run --script benchmarks/word/benchmark.py run --name xor-smoke --kind xor --word-benchmark-binary build/bin/word-benchmark --repetitions 3
```

Use `--kind all` with both binary options to run the public kinds together.
Runs are interleaved by repetition and instance, with variant order rotated
between repetitions. Analysis requires the exact planned matrix: each expected
run once with a terminal status, with no missing, duplicate, unexpected, or
stale extra result. Status and solver counters must be stable across
repetitions; a uniformly timed-out or failed group remains reportable.
`--limit` is accepted only when it ends on a complete formulation group; cuts
that would make parity analysis unusable are rejected before artifacts are written.

DMA memory evidence reports current handle, implementation, and model layout
sizes plus RSS slopes from fresh subprocesses retaining stabilized root clones
at 2,000, 8,000, and 32,000. Populations grow by four when the RSS range is
below 16 MiB, capped at 256,000; resource-limited or unresolved measurements
are explicitly `unmeasured`. Structural entity formulas are not byte evidence.

Private XOR corpora retain the established external layout: `manifest.json`
with `"corpus": "private"`, `"redistributable": false`, and relative paths to
instances matching `private-layout.json`. Pass `--private-corpus-root` to load
one. Pass `--package-output PATH` one or more times to inspect a package
directory, tar archive, or zip by name listing for private/result payloads.
Missing private roots and package outputs remain reported rather than copied.

Raw run JSON/stdout/stderr, analysis, and reports live under
`results/<name>/`; completed records resume without being overwritten.
Timeout, error, and failed records remain explicit in raw results and reports.
Peak RSS is measured for the individual child with the platform's
`/usr/bin/time` (`-l` on macOS, `-v` on Linux); unsupported platforms report
the measurement as unavailable.

## Semantic result contract

`semantics.py` defines the small contract shared by these runners and intended
for later external-solver adapters. A fixture declares:

- `goal`: `first`, `unique`, `enumerate`, or `unsat`;
- `expected_status`: semantic `sat` or `unsat`;
- `parameters`: the complete public instance parameters; and
- `decision_variables`: the ordered public projection, excluding auxiliaries.

A successful solver row reports the same ordered names, its semantic status,
and `projections`. `first` and `unique` return one concrete witness;
`enumerate` returns the exact small projected set; `unsat` returns no witness.
The Python evaluator checks every witness and independently enumerates only the
small controls. Solver counters and timings remain benchmark metadata.

An external adapter must map its status to `sat` or `unsat`, emit only the
declared public variables in the declared order, use unsigned Word encodings
for bit-vector values, and apply signed interpretation only where the fixture
declares it. It must keep timeout/error distinct from UNSAT and validate
semantic parity before timed comparison. Exact enumeration counts projected
public tuples, never auxiliary assignments.
