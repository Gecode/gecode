# Random generators in Gecode 7

Random branching uses a stream local to each space. Each recorded choice contains
the state needed to derive a different stream for each alternative. Recomputing
that choice installs the same stream as the original commit. Cloning copies state;
it does not split or consume random values.

## Engines and full state

`Support::Random<Engine>` is a value type. Copying it makes an independent, exact
copy. It supplies bounded integer generation and state text on top of the engine.
An engine provides:

- `State`, a nonempty `std::array<uint64_t,N>` containing all mutable state and
  per-stream parameters;
- construction and `seed(uint64_t)`, with a documented rule for every seed;
- `next()`, `min()`, and `max()`, with raw output in `[0,UINT64_MAX]` or
  `[1,UINT64_MAX]`;
- `state()` and `state(const State&)` for exact capture and validated restoration;
- `name()`, a stable identifier for its state format and algorithm;
- `split(uint32_t) const`, returning the selected child without changing its
  parent, for use in search.

All valid alternative indices of a parent must produce distinct child states.
An engine must document that property; different first output values are neither
required nor sufficient. Copying and restoring state must reproduce subsequent
draws and splits. A state setter must reject invalid input before mutating state.
Keep engine-owned resources exception-safe; copying an engine may occur while
cloning a space. Raw pointers to external mutable state are unsuitable snapshots.

The built-in engines are:

| Engine | State | Splitting |
| --- | ---: | --- |
| `Support::SplitMix` | 16 bytes | Indexed sequential SplitMix splits, constant time |
| `Support::Xorshift64Star` | 8 bytes | Indexed jumps in the native recurrence |

SplitMix uses the published two-word splittable design: a state and an odd
increment. It is the preferred default. For parent `(s,g)`, child `a` is
`(Mix13(s+(2a+1)g), mixGamma(s+(2a+2)g))` modulo 2^64. The first word is distinct
for every 32-bit index because `g` is odd and Mix13 is a permutation. This is
exactly the child obtained by sequentially splitting `a+1` times, without doing
the intervening work.

Xorshift64* uses shifts 12, 25, and 27, with multiplier 2685821657736338717.
Child `a` starts `(a+1)*2^32` recurrence steps ahead. Since 2^32 is coprime to
the period 2^64-1, sibling states are distinct. Binary powers of the transition
matrix compute jumps in logarithmic time, with a shared 16 KiB table initialized
once. That table is not part of individual states or choices. For the maximum
32-bit index, 2^64 steps reduce to one step modulo the period. These jumps keep
the original recurrence; they are not a new seed-hashing scheme.

Neither engine guarantees globally disjoint streams throughout an unbounded
search tree. Xorshift64* also has known statistical weaknesses in its low bits;
its smaller state and different splitting cost should be considered together.
See [Vigna's discussion](https://prng.di.unimi.it/xorshift.php) and the
[SplitMix paper](https://gee.cs.oswego.edu/dl/papers/oopsla14.pdf).

Bounded draws use integer rejection sampling. Bounds zero, one, or negative
signed bounds return zero without consuming output. Full-range engines reject
the incomplete bucket at the bottom of the raw range; nonzero engines first
subtract one and reject the incomplete bucket at the top. Thus xorshift's missing
zero is accounted for. No distribution state is cached. The conversion and its
draw consumption are part of Gecode's reproducible sequence.

Full state text is an identifier followed by fixed-width hexadecimal words, for
example `splitmix-v1:000000000000002a:9e3779b97f4a7c15`. Word order is explicit
and independent of host byte order. Restoration does not run seed expansion or
repair invalid states. A normal 64-bit seed is a convenience for initialization,
not a substitute for a complete snapshot.

## Handles, binding, and callbacks

`Rnd(seed)` creates a standalone default stream. `Rnd(Support::Random<MyEngine>(seed))`
creates a user-defined one. Copying a standalone `Rnd` shares its stream;
`r.copy()` creates an independent exact copy, and `r.split(a)` creates a child.
Concurrent draws through shared standalone handles require caller synchronization.

`Rnd(home, source)` binds a stream to a space. Binding the same stream again
returns the same local stream. Different initialization handles remain different
streams even when their initial states are equal. Binding retains stream identity
across clones, so callbacks can resolve an ancestor's handle in their own space.
Reseeding an external initialization handle after binding does not reseed the
space-local copy; reseed the bound handle explicitly if that is intended.

Built-in random selectors bind during posting and remap during cloning. A model
that retains a bound handle should likewise use `rnd(*this, s.rnd)` in its copy
constructor. User callbacks that capture a handle must resolve it through the
space passed to the callback rather than draw directly from the captured handle:

```cpp
Rnd source(*this, 42);
branch(*this, x, INT_VAR_NONE(),
       INT_VAL([source](const Space& home, IntVar v, int) {
         Rnd local(home, source);
         unsigned int offset = local(v.size());
         IntVarValues values(v);
         while (offset--) ++values;
         return values.val();
       }));
```

The const-space overload only looks up an already bound stream. Bind streams
before taking choices or clones that will replay their use. In particular, a
choice-selection callback must not first register a new stream that is absent
from an earlier clone. Dynamically posted branchers can reuse existing streams;
new streams introduced during commit must be introduced consistently on replay.
An explicitly new stream starts at its specified initialization point; it does
not retrospectively consume earlier alternatives.

Each choice snapshots all bound streams, including streams belonging to later
branchers. Therefore a deterministic branch before a random branch still gives
the later branch different states for its alternatives. Multiple selectors sharing
one stream incur only one snapshot. Custom branchers participate through
`Space::choice()`, `Space::commit()`/`trycommit()`, and the base `Choice::archive()`;
their choice payload still contains their usual position/value data. Choices own
their snapshots and cannot be copied by C++ copy construction; use archiving when
an independent choice representation is needed.

## Restarts, portfolios, and reproducibility limits

Ordinary space cloning preserves stream states exactly. Generic meta-engines do
not add implicit RNG splits when creating clones. Models can explicitly call
`random_split(index)` in their `slave()` callback to derive all bound streams
from a logical restart or asset index. The callback must follow a fixed policy,
independent of which worker happens to execute it.

FlatZinc uses a fixed three-step policy: split by 0 for restart or 1 for portfolio,
then by the high and low 32-bit words of the logical index. The Photo example
uses the high and low restart-index words before relaxation. The common relaxation
helper binds its input stream to its destination space, avoiding shared draws
between sibling spaces.

The guarantee is the RNG state for a recorded path, not identical scheduling or
solution order in parallel search. Adaptive heuristics, restart constraints, and
weakly monotonic propagation can still change the search tree. Reproducing a
whole run also requires the same model, relevant options, and compatible Gecode
code and random algorithm. Standard-library distributions are outside Gecode's
bounded-sequence contract.

## Test failure replay

The test runner records complete state immediately before each iteration. On a
failure or exception it prints arguments using `-state`, `-iter 1`, and
`-test-exact`. Run the same test executable with those arguments. This restores
the iteration directly, bypassing suite seed derivation. A state replay requires
one thread and rejects an accompanying `-seed` or an incompatible state format.

## Measurements

Build the benchmark against the candidate using the same release compiler flags
as Gecode (adjust library search paths for your platform):

```sh
clang++ -O3 -DNDEBUG -std=c++17 -DRANDOM_NEW -Ibuild/random -I. \
  tools/random-benchmark.cpp -Lbuild/random -lgecodeint -lgecodesearch \
  -lgecodekernel -lgecodesupport -Wl,-rpath,build/random \
  -o build/random/random-benchmark
python3 tools/random-benchmark.py build/random/random-benchmark \
  --baseline /path/to/baseline/random-benchmark --repeat 5 --output results.json
```

For the baseline, compile the same benchmark source against baseline headers and
libraries without `-DRANDOM_NEW`. The script alternates execution order, discards
one warmup, and reports medians while retaining individual measurements in JSON.
Use a new output filename for another run. Avoid concurrent compilation or other
heavy work while collecting timings.

The controlled search enumerates a complete binary tree of 32767 nodes and 16384
solutions for every engine and seed, so timings compare overhead without changes
in tree size. The queens case also reports node count: changed random choices can
change its search tree, and wall time alone is not an overhead comparison. Raw
baseline draws emit 32-bit words while the new engines emit 64-bit words. Raw
throughput can also benefit from compiler optimizations that do not apply inside
branchers; the search measurements matter more for the default decision.

On arm64 macOS 26.6.2 with Apple Clang 21, baseline 6b7de57b04, and seed 42,
five measured repetitions after one warmup gave these medians:

| Measurement | Baseline | SplitMix with recorded streams |
| --- | ---: | ---: |
| Bounded draw through Rnd | 14.85 ns | 2.96 ns |
| Clone a random space | 178.70 ns | 345.95 ns |
| Binary-tree node, frequent cloning | 153.07 ns | 254.32 ns |
| Binary-tree node, recomputation | 226.76 ns | 368.53 ns |
| Enumerate 10-queens | 20.04 ms | 20.96 ms |

The controlled random tree is about 1.6x slower: local copying and recorded state
have a cost that faster drawing does not eliminate. Nonrandom tree overhead was
1–3%. Seeds 1 and 1337 gave similar results; queens time increased about 4–5%,
with node counts within 0.1% of baseline. These measurements do not establish a
general speedup or statistical confidence beyond this machine and these cases.

A binary SplitMix split followed by a draw measured about 15.7 ns; xorshift's
native jump followed by a draw measured about 237 ns. This favors SplitMix as the
default while retaining xorshift for its 8-byte state. The default integer choice
occupies 32 bytes plus a 24-byte snapshot allocation, compared with 24 bytes on
baseline. Archives occupy 32 rather than 12 bytes. No snapshot is allocated for
a nonrandom choice, although its optional pointer costs 8 bytes. Allocator
overhead is additional to these logical sizes.
