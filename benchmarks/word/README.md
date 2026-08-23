# Word benchmarks

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
