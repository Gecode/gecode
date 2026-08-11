# Word differential benchmark

This development benchmark compares the native Gecode Word model with an
equivalent Gecode Boolean decomposition of the same XOR-and-rotate problem.
It does not compare against an SMT solver and must not be cited as SMT parity.

The checked-in public manifest contains one small redistributable smoke
instance. A private corpus remains outside the repository and has this layout:

```text
private-root/
  manifest.json
  instances/
    instance-name.json
```

`manifest.json` uses the same shape as `public-manifest.json`, with
`"corpus": "private"`, `"redistributable": false`, and relative paths to its
instances. The required fields are recorded in `private-layout.json`. Missing
roots, manifests, and referenced files are reported as discovery issues; the
script neither recursively discovers nor copies private payloads.

Run the benchmark from the repository root after building the
`word-benchmark` example:

```sh
uv run --script benchmarks/word/benchmark.py run --name smoke \
  --binary build/bin/word-benchmark --repetitions 3
uv run --script benchmarks/word/benchmark.py analyze --name smoke
uv run --script benchmarks/word/benchmark.py report --name smoke
```

Pass `--private-corpus-root /absolute/external/path` to `run` to add private
instances. Results default to `results/<name>/` and contain stable per-run
JSON, stdout, and stderr as flat `<run-id>.json`, `<run-id>.stdout`, and
`<run-id>.stderr` files beneath `runs/`, aggregate JSON beneath `analysis/`,
and a concise engineering report beneath `reports/`. The empty `plots/`
directory is reserved by the artifact layout; this focused benchmark has no
plot command.

Runtime and peak resident memory are measured by the Python process wrapper.
Peak memory is best effort and is `null` when the platform timing tool does
not expose it. `allocations` counts model variable implementations, while
`clone_footprint` counts those implementations plus propagators and branchers
in one stabilized clone. Both are structural counters, not kernel allocation
counts or byte measurements.

Pass `--package-output PATH` one or more times to inspect an existing package
directory, tar archive, or zip archive by name listing. The report distinguishes
no supplied package, unavailable output, scan error, and successful scans. This
check reads package contents but does not create or modify packages.
