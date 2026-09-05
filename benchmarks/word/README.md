# Word benchmarks

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
compact counters, and stable counters across all repetitions.

The bounded product-modulo comparison uses the fixed width-nine case with
`x,y in [10,20]`, modulus 509, and result-first ranked splitting. The compact
control links the same bounded search variables to compact product-modulo
operands with binary identities. Run the 20 alternating-order trials with:

```sh
uv run --script benchmarks/word/product-mod-benchmark.py --binary build/bin/word-product-mod --repetitions 20 --output /tmp/word-product-mod.json
```

The bounded n-ary Add scatter/gather comparison uses `word-nary-add` at 4,
6, and 8 segments. Run 20 alternating-order trials without storing artifacts
in the checkout:

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
write-controlled limit ITE. All four formulations must have the same solution
count and wrapping checksum. This instance does not predict every Word workload.

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
Runs are interleaved by repetition, instance, and variant. Analysis requires
the exact planned matrix: each expected run once, successful, with no missing,
duplicate, unexpected, or stale extra result.
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
