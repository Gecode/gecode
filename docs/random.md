# Random generators for a future Gecode release

This is a provisional Gecode 7 design. It changes APIs, seeded sequences, and
randomized choice archives and is intended only for a breaking-change release.

## State belongs to the consumer

A generator is a small value. A model, selector, or custom brancher stores that
value directly in its own state. Copying it copies all its state independently.
There is no random context in Space, no registration, no shared mutable handle,
and no automatic coordination between consumers.

```cpp
Rnd a(42);
Rnd b = a;              // Independent copy at the same state.
Rnd child = a.split(1); // Does not change a.
```

The default `Rnd` contains exactly the configured engine's state: 16 bytes
for splittable SplitMix or 8 bytes for xorshift64*. It has no pointer, vtable,
reference count, or heap allocation. Default construction initializes seed 1.

Passing a generator to two branching descriptions gives them two independent
copies. To start them differently, pass explicitly split generators. Posting
copies the description's state into its selector. Cloning copies selector state
without drawing or splitting. A model member is copied normally in the model's
copy constructor; it is not aliased to a selector that was initialized from it.

## Branch selection and recomputation

After choosing its position and value, a randomized brancher records the complete
state of its own selectors in its choice. Committing alternative `a` restores
that recorded state and derives each selector's `split(a)` state before the
value commit. The destination's current selector state is not the splitting
input. Archive reconstruction restores choice data without drawing or splitting.

Only the active brancher's selectors participate. A deterministic brancher does
not advance a later random brancher or a model-owned generator. Nor does finishing
one random brancher change a separately posted brancher's state. Earlier branch
constraints can still affect later selection, of course.

The rule applies to binary, multiway, and one-alternative assignment branchers.
It uses the public alternative index even when values are visited in reverse
order. Each random choice stores one parent state per selector, not one state
per alternative. Late alternatives require no replay of preceding alternatives.

Randomized choices use `RndChoice<Base>`, with packed state words in the same
allocation as the choice. Nonrandom choices retain their original layout and
archive format. The base `Space` and `Choice` classes contain no RNG storage or
snapshot hooks. Multiple selectors are recorded independently, in tie-break order
followed by value selection; their types determine the state-word layout.

This guarantees random state for the same recorded path. It does not guarantee
identical scheduling, solution order, adaptive heuristics, or entire search trees
under parallel search or weakly monotonic propagation.

## Engines and extension

`Support::Random<Engine>` is the low-level value wrapper, supplying bounded
integer draws and canonical state text. `RndGenerator<Engine>` is the modeling
wrapper; `Rnd` names its build-configured specialization.

An engine supplies:

- A nonempty `State = std::array<uint64_t,N>` containing all mutable state and
  per-stream parameters.
- Construction and `seed(uint64_t)`, with a documented rule for every seed.
- `next()`, `min()`, and `max()`, with raw output in `[0,UINT64_MAX]` or
  `[1,UINT64_MAX]`.
- `state()` and `state(const State&)` for exact capture and validated restoration.
- `name()`, identifying the algorithm and state format.
- `split(uint32_t) const`, returning a child without changing its parent.

All valid sibling indices must yield distinct states, not merely different first
outputs. State restoration must reproduce subsequent draws and splits and reject
invalid input before mutation. Ordinary copying must make state independent.

The runnable `examples/random-engine.cpp` defines a three-word engine outside
Gecode. It delegates to SplitMix and adds a path-local raw-draw counter. Both
variable and value selectors store this user engine inline:

```cpp
using Random = RndGenerator<MyEngine>;
using VariableSelector = ViewSelRnd<Int::IntView,Random>;
using ValueSelector = Int::Branch::ValSelRnd<Int::IntView,Random>;
```

The example posts these with the existing generic view/value brancher machinery.
Its optional argument is a complete custom-engine state:

```sh
cmake --build build/random --target random-engine
build/random/bin/random-engine
build/random/bin/random-engine counted-splitmix-v1:000000000000002a:9e3779b97f4a7c15:0000000000000000
```

Both runs print the same initial state and enumerate the same 24 permutations.

Custom selectors participate through `random_words()`, `random_save()`, and
`random_commit()`. State size and layout must be stable across clones.
Custom branchers can instead store their generator and its choice snapshot as
ordinary typed members and implement the same capture/restore/split rule directly.

Randomness hidden inside a callback is not automatically recorded. If a callback
draws model-owned state during choice generation, its custom choice/commit
implementation must preserve that state for replay. A callback must not capture
a mutable RNG in a shared function object and assume that cloning copies it.
Callbacks that operate on model state during commit should access the destination
model and make their state transitions explicit.

## Built-in algorithms

| Engine | State | Indexed splitting |
| --- | ---: | --- |
| `Support::SplitMix` | 16 bytes | Constant-time sequential SplitMix child |
| `Support::Xorshift64Star` | 8 bytes | Jump in its native recurrence |

For SplitMix parent `(s,g)`, child `a` is
`(Mix13(s+(2a+1)g), mixGamma(s+(2a+2)g))` modulo 2^64. This is the child
obtained by sequentially splitting `a+1` times, computed directly. Since `g`
is odd and Mix13 is a permutation, the first child word is distinct for every
32-bit alternative index. Seed expansion sets `s` to the seed and `g` to
`9e3779b97f4a7c15`.

Xorshift64* uses shifts 12, 25, and 27 and multiplier 2685821657736338717.
Child `a` starts `(a+1)*2^32` recurrence steps ahead. Since 2^32 is coprime
to the period 2^64-1, sibling states are distinct. A shared immutable 16 KiB jump
table is initialized once; it is not per-generator state. The maximum index
jumps 2^64 steps, equivalent to one step modulo the period. Seeds set the state
directly, except seed zero maps to one; restoring zero state is an error.

Neither construction promises globally non-overlapping streams throughout an
unbounded search tree. Xorshift64* has known low-bit weaknesses; see
[Vigna's discussion](https://prng.di.unimi.it/xorshift.php) and the
[SplitMix paper](https://gee.cs.oswego.edu/dl/papers/oopsla14.pdf).

Bounded generation uses integer rejection sampling. Bounds zero, one, and negative
signed bounds return zero without drawing. Full-range engines reject an incomplete
bottom bucket; nonzero engines subtract one and reject an incomplete top bucket.
This accounts for xorshift's missing zero. There is no distribution cache.

## Seeds, state, and command lines

Full state text contains an algorithm/format identifier and fixed-width hexadecimal
words in a defined order, independent of host byte order. For example:
`splitmix-v1:000000000000002a:9e3779b97f4a7c15`. Restoration bypasses seed
expansion; it never repairs invalid state silently.

The example driver accepts `-seed`; FlatZinc accepts `-r`. Values are checked
unsigned 64-bit decimal or `0x` hexadecimal integers, or `time` and `hw`.
Both accept `-state` for the configured engine and reject conflicting seed/state
arguments, invalid states, and incompatible identifiers. Double-hyphen spellings
are accepted. A 64-bit seed need not enumerate every possible full state.

Time and hardware initialization occur once during parsing and report
`% Random state: -state ...` on standard error. Reuse those arguments with the
same executable, model, and search options. Help also prints the initial state.

`opt.rnd()` returns an independent value at the configured initial state. Use it
instead of `Rnd(opt.seed())` to honor full-state input. The numeric accessor
throws for full-state, time, or hardware initialization.

The test runner snapshots immediately before each iteration. Failure and exception
reports print `-state`, `-iter 1`, and `-test-exact` arguments that restore
the iteration directly, bypassing suite seed derivation. Replay requires one
thread and rejects an accompanying seed.

CMake `-DGECODE_RANDOM_ENGINE=xorshift64star` and Autoconf
`--with-random-engine=xorshift64star` select the alternative default; both accept
`splitmix`. The installed configuration header carries that choice to clients.
Both concrete engine types remain available; no runtime registry is required.

## Model-owned randomness and migration

FlatZinc's restart/relaxation generator is an ordinary model member. Its explicit
meta-engine policy splits by restart/portfolio kind and the high/low words of the
logical index. This affects that member only, not the model's branchers.
Photo likewise splits its member before relaxation. The relaxation APIs take
`Rnd&` and advance the caller's value directly.

Gecode 6 shared-handle behavior is removed: ordinary copying now copies state.
Replace the old zero-argument `seed()` snapshot accessor with `state()` and
restore with `state(text)`. Numeric construction is explicit; default construction
initializes seed 1. `copy()` remains a convenience equivalent to ordinary copying.

Seeded sequences and randomized choice archives change. There is no legacy sequence
mode. The old FlatZinc 31-bit sampling workaround is removed. Release version and
ABI-number changes remain release preparation; this feature must not ship in a
compatibility-preserving release.

## Measurements

Use the existing bounded benchmark harness after building Gecode:

```sh
clang++ -O3 -DNDEBUG -std=c++17 -DRANDOM_NEW -Ibuild/random -I. \
  tools/random-benchmark.cpp -Lbuild/random -lgecodeint -lgecodesearch \
  -lgecodekernel -lgecodesupport -Wl,-rpath,build/random \
  -o build/random/random-benchmark
python3 tools/random-benchmark.py build/random/random-benchmark \
  --baseline /path/to/baseline/random-benchmark --repeat 5 --output results.json
```

Compile the same source against baseline headers/libraries without `-DRANDOM_NEW`.
The harness alternates run order, discards one warmup, and reports medians. The
controlled tree always has 32767 nodes and 16384 solutions. Queens also reports
node count because changes in its tree can affect timing. Avoid concurrent
compilation while measuring.

Measurements after the ownership correction, on arm64 macOS with Apple Clang 21
in Release mode, use five measured repetitions, one warmup, seed 42, and main
at `6b7de57b04` as the baseline. Sizes are bytes:

| Object | Main | SplitMix | Xorshift64* |
| --- | ---: | ---: | ---: |
| `Rnd` | 8 (handle) | 16 | 8 |
| Random variable selector | 16 | 24 | 16 |
| Random value selector | 8 | 16 | 8 |
| Integer variable description | 112 | 120 | 112 |
| Integer value description | 80 | 88 | 80 |
| `Space` | 288 | 288 | 288 |
| Nonrandom position/value choice | 24 | 24 | 24 |
| Random choice, one selector | 24 | 48 | 40 |
| Nonrandom choice archive | 12 | 12 | 12 |
| Random choice archive, one selector | 12 | 28 | 20 |

Main's handle size excludes its separately allocated shared implementation; the
new values contain all engine state. Random choice sizes include the inline
payload. Adding another random selector adds only its state words to that payload.

Median controlled random-tree time relative to main was 1.02x with cloning and
1.16x with recomputation for SplitMix, versus 1.63x and 2.88x for xorshift64*.
The plain tree was 1.03x and 1.05x in both builds. Indexed binary split-plus-draw
cost about 15.7 ns for SplitMix and 237 ns for xorshift64*. Queens took 20.1 ms
with SplitMix (approximately main's time) and 21.2 ms with xorshift64* (1.06x).
Its node counts were 9333 on main, 9325 with SplitMix, and 9323 with xorshift64*.
These are local measurements, not general performance guarantees. They replace
the earlier space-local results and leave the default-engine decision provisional.
