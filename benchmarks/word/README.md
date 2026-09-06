# Word benchmarks

## Focused word-037 campaign

`comparison-campaign.py` validates the exact 72-instance preflight matrix before
collection. The checked-in manifest contains six families, three levels, and
four explicit, parameter-distinct instances (two SAT and two UNSAT) per
family/level. Validation derives every expected public projection with the
existing concrete evaluators and derives the exact baseline, candidate, and Z3
commands and SMT input with the existing encoding helpers.
Each case stores its expected solution count and explicit scale metadata. The
preflight checks that the maximum workload scale increases from small through
large in every family, rejects fixed-input bit-network cases, and executes one
representative Z3 semantic check per family. The calibration-candidate list is
explicit and contains only SAT cases.

```sh
python3 benchmarks/word/comparison-campaign.py validate
python3 benchmarks/word/comparison-campaign.py validate \
  --output /private/tmp/word-037-preflight.json
python3 benchmarks/word/comparison-campaign.py smoke \
  --bit-binary build/bin/word-bit-network-comparison \
  --dma-binary build/bin/word-dma-descriptor \
  --lookup-binary build/bin/word-register-file \
  --allocation-binary build/bin/word-distinct-benchmark \
  --inverse-binary build/bin/word-inverse-arithmetic \
  --alu-binary build/bin/word-symbolic-alu --z3 z3
```

The smoke command runs two complete in-process repetitions of one small SAT
instance from each family through native
baseline and candidate search, and through Z3. Compact CRC/xorshift/Speck uses
LSB versus MSB branching; bounded families use LSB versus lower-ranked split.
Bitwuzla is reported as unsupported because this campaign has no validated
adapter for it.

Collection requires an explicitly frozen image and a new result root. It first
calibrates search using only the manifest's calibration cases, then screens all
288 instance/configuration cells. Each Podman run has no network, one CPU, and
a 4 GiB memory-and-swap cap; the runner verifies those settings before start.
It resumes terminal records and refuses a changed manifest, image, revision,
script, options, or CPU budget at an existing root.

```sh
ROOT=/private/tmp/gecode-word-037-campaign-v2
IMAGE=localhost/gecode-word037-runtime:<frozen-tag>
python3 benchmarks/word/comparison-campaign.py calibrate --image "$IMAGE" --root "$ROOT"
python3 benchmarks/word/comparison-campaign.py screen --image "$IMAGE" --root "$ROOT"
python3 benchmarks/word/comparison-campaign.py followup --image "$IMAGE" --root "$ROOT" \
  --case <selected-case> --case <selected-case>
python3 benchmarks/word/comparison-campaign.py analyze --root "$ROOT"
```

Screen runs have a 30-second cap. Follow-ups interleave five repeats with a
300-second cap. All phases share a hard 21,600 CPU-second ledger; a run is
deferred unless its whole timeout can be preauthorized. Tiny cases are batched
to at least 0.25 seconds, with Z3 repeating reset, declarations, assertions,
and `check-sat` inside one process. Raw records, frozen metadata,
`analysis.json`, and `result.md` stay under the external root.

## CRC, xorshift, and reduced-Speck comparisons

`word-bit-network-comparison.py` checks 15 small cases: five each for
CRC-16, xorshift32, and reduced Speck32/64.  Each family has a base
enumeration plus independent unknown-bit, output-observation, and round-count
axes.  Its UNSAT case first has a unique public projection under the concrete
evaluator and then excludes that exact projection; it does not assume that a
changed output bit is impossible.  The public projections are `[message]`,
`[state]`, and `[key0,key1,key2,key3]`, respectively.  Every fixture records
explicit output mask/value observations and the three scale fields
`unknown_bits`, `observation_bits`, and `rounds`.

The CRC recurrence follows the MSB-first catalogue convention with the
repository example's deliberately non-catalogue initial state `0x1d0f` and
polynomial `0x1021`: feedback is the current top bit XOR the next message bit,
then the state shifts left and conditionally XORs the polynomial.  The CRC
parameter terminology comes from the
[CRC catalogue](https://reveng.sourceforge.io/crc-catalogue/16.htm).
xorshift32 is Marsaglia's `x ^= x << 13; x ^= x >> 17; x ^= x << 5`
recurrence ([paper](https://www.jstatsoft.org/article/view/v008i14)).
Speck uses 16-bit words, alpha 7, beta 2, and the published key schedule and
round orientation.  Before any cases run, the evaluator checks the official
Speck32/64 vector: key words `[0100,0908,1110,1918]`, plaintext
`6574:694c`, and ciphertext `a868:42f2`, as documented in the
[NSA implementation guide](https://nsacyber.github.io/simon-speck/implementations/ImplementationGuide1.1.pdf).

The linear families additionally use a small evaluator-side GF(2) Gaussian
elimination.  Results include rank, nullity, and consistency and compare them
with the number of public bits fixed at the Gecode root.  A full-rank system
whose public Word still has unknown root bits is reported as correlation loss;
this is evidence about the fixture, not a production affine-domain feature.
Speck is intentionally excluded from this linear control.

The external encodings are direct QF_BV recurrences.  Enumeration adds an
exact blocking clause over only the public projection after each model.  Z3 is
invoked with `-smt2 -in`; Bitwuzla is invoked with `--lang smt2
--produce-models`.  Executable, version, and options are retained in the
artifact.  A missing Bitwuzla, timeout, or process error remains an explicit
incomplete cell.  The ten-second cap is per solver/case, and these smoke runs
make no timing or cryptanalytic claim.

```sh
cmake --build build --target word-bit-network-comparison
uv run --script benchmarks/word/word-bit-network-comparison.py \
  --binary build/bin/word-bit-network-comparison --timeout 10 \
  --output /tmp/word-bit-network-comparison.json
```

The focused mixed-model comparison checks DMA descriptor windows, register
lookup, and register allocation against direct QF_BV formulations for Z3 and
Bitwuzla. Its checked-in case table varies descriptor count, window slack,
selected cap, lookup size and selector density, and allocation Hall structure
independently. Every small case is enumerated by a separate concrete Python
evaluator; SMT models are enumerated through public-decision blocking clauses.
The same case parameters and exact public projection sets are checked through
all applicable Gecode formulations, including the deliberately UNSAT cases.

The result keeps aligned semantic rows separate from the Gecode native-search
controls. The latter include compact/bounded DMA and lookup examples plus
`IPL_VAL`/`IPL_BND` allocation controls. This avoids presenting differences in
native branching as solver-comparison results. Each solver row records its
executable, version and options. A missing or unsupported solver, timeout, or
process error is an explicit status and is never reported as UNSAT.

```sh
cmake --build build --target word-dma-descriptor word-register-file word-distinct-benchmark
python3 benchmarks/word/mixed-model-comparison.py \
  --dma-binary build/bin/word-dma-descriptor \
  --lookup-binary build/bin/word-register-file \
  --allocation-binary build/bin/word-distinct-benchmark \
  --timeout 10 --output /tmp/word-mixed-smoke.json
```

The cases run sequentially in one solver process apiece and use the solver
defaults (one solving thread for these command-line configurations). A missing
Bitwuzla installation is a valid incomplete matrix, not a successful Bitwuzla
comparison. Available-solver semantic mismatches make the command fail. These
small smoke timings support no broad performance claim.

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

## Inverse arithmetic and short ALU comparisons

`mixed-model-comparison.py` also compares native Word constraints with Z3 and
Bitwuzla for modular multiplication, SMT-total unsigned and signed division,
and mathematical `product_mod`.  Values in the case file are unsigned
width-bit encodings; multiplication wraps with the explicit `(1<<width)-1`
mask.  `product_mod` widens both operands to `2*width` before remainder.  Its
positive-modulus condition is unconditional under reification: disabling the
relation permits an arbitrary result but does not permit a zero modulus.

The `alu` family publishes only `[input,output]`.  Its internal trace computes
`amount=input&3`, `s1=input<<amount`, adds `0x1d` with unsigned carry, records
the sign bit of that sum, then selects either `sum xor 0x15` on carry or an
arithmetic right shift by `amount`; a negative sum increments the selected
value.  All operations use SMT overshift and width wrapping semantics.  For
the documented width-five input 19 the internal values are `amount=3`,
`s1=24`, `s2=21`, `carry=1`, `negative=1`, `s3=0`, and public output `1`.

The exact supported subset is intentionally small: full enumeration at width
3--5, tight or fixed cases at width 8, and fixed single-witness cases at width
64.  Campaign scale is the case's width plus operand interval sizes; do not
extrapolate free enumeration to width 64.  Run the aligned checks with:

```
uv run --script benchmarks/word/mixed-model-comparison.py \
  --inverse-binary build/bin/word-inverse-arithmetic \
  --alu-binary build/bin/word-symbolic-alu --timeout 10
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
