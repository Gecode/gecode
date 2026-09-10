# Plan: Compact, splittable random generators for Gecode 7

> Source: the feature/random design discussion, 2026-09-10.
> Status: All four phases complete and reviewed.
> Workflow: review, update this plan, and commit after each phase.

## Goal

Replace the old default random generator, provide user-extensible alternatives,
and make random branching streams stable under recomputation. Gecode 7 permits
breaking source, binary, and seeded-sequence compatibility for this change.

Xorshift64* was the original proposed replacement. The subsequent requirement
for alternative-specific splitting must also be satisfied before selecting the
default. The design must stay compact in branching descriptions, branchers,
choices, and search paths.

## User requirements

1. A model author can use a better default generator and supply a custom engine
   through Gecode's supported extension interface.
2. A test failure can be replayed from its complete random state, including state
   required for subsequent splitting, without reconstructing preceding draws.
3. Every alternative of a branching decision gets a distinct successor state.
   Replaying the same recorded decision and alternative gets the same state.
4. Cloning and recomputation do not introduce additional splits or make streams
   depend on exploration order.
5. A command-line executable can use one configured engine. Large internal state
   does not force users to supply equally large numeric seeds.
6. Memory and execution costs remain appropriate for Gecode search.

## Architectural decisions

### Branching and replay contract

Generating a choice may consume random values for variable and value selection.
After those selections, the choice captures the complete state needed to derive
the streams for its alternatives. Committing an alternative derives and installs
its successor state from that immutable choice data and the alternative index.
It must not derive it from whatever mutable generator state happens to be in the
destination space.

The same operation applies during exploration and recomputation. Reconstructing
a choice from an archive restores its splitting data without drawing or splitting.
Cloning preserves generator state without advancing either source or destination.
Sibling spaces must not consume each other's mutable streams.

The sibling-state distinction must hold for all valid alternative indices,
including large multiway choices. A hash with merely a low collision probability
does not establish this requirement. Distinct states do not imply globally
non-overlapping sequences or uniqueness across an unbounded search tree.

Store one common splitting payload per choice for the common single-stream case,
not an array of successor states. Deriving a late alternative must not require
generating all preceding alternatives. If multiple streams need separate state,
their cost must be explicit; do not silently assume one snapshot covers them.

This is a guarantee about random state for a recorded path. It does not promise
identical global scheduling, solution order, adaptive heuristic state, or search
trees under parallel search, restarts, or weakly monotonic propagation.

### Generator operations and extensibility

Specify initialization, raw generation, bounded generation, exact copying,
alternative-indexed splitting, and full-state save/restore separately.
The names and C++ representation remain to be chosen during Phase 1.

Custom engines must be usable by supported branching APIs, not only by callers
of the low-level support library. Define the output range, valid states, state
size, and splitting obligations of an engine. Engines without the required
splitting behavior may be usable as standalone generators but cannot silently
qualify as search generators.

Keep the command-line engine selection independent of the extension mechanism:
one configured engine per executable is sufficient. No runtime plugin registry
or mandatory catalogue of engines is required. If configuration changes public
types or layouts, export the configuration consistently to downstream builds.

Gecode specifies bounded integer conversion and draw consumption, including zero,
one, negative signed bounds, and maximum supported bounds. Use integer conversion
with rejection where required; account for the engine's actual output range.
In particular, a nonzero-state xorshift engine must not be treated as emitting
every 64-bit value with equal frequency over its period. Avoid distribution
caches unless their state is part of snapshots.

### Seeds and complete state

Ordinary initialization accepts a checked 64-bit seed, in decimal or hexadecimal.
Specify seed expansion and the treatment of zero for every supported engine.
Restoring state bypasses seed expansion and never silently repairs invalid state.

Use a canonical text representation containing an algorithm/format identifier
and all state words in a specified order. Stream increments, counters, and other
mutable or per-instance parameters belong in it. An executable rejects an
incompatible state identifier rather than switching engines implicitly.

The test runner records a snapshot immediately before an iteration and prints a
command that restores that snapshot directly for the named test. That command
bypasses normal suite-level seed derivation. Ordinary failure and exception paths
must both report the relevant iteration state, along with existing test options.

The test runner needs full-state input in Phase 1. Public drivers receive the
same seed/state conventions in Phase 4. Use separate seed and state options;
reject conflicting input. A wider seed interface is unnecessary for this scope.

### Compactness

Measure the complete representation, not just the engine's state words: public
descriptions, selectors, local storage, choice objects, archive words, and retained
search paths. Include padding, dispatch data, and allocations.

An 8-byte engine state is preferred; 16 bytes is a candidate budget for a truly
splittable engine, not an already approved limit. Keep runtime dispatch metadata
and textual identifiers out of repeated state payloads where they are implied
by the configured type. Avoid adding random-state storage to models that never
use random branching where practical, and measure any unavoidable common cost.

## Decisions to close during implementation

### Engine and indexed splitting

Compare the original xorshift64* candidate with a published splittable design.
Distinguish fixed-increment SplitMix64 (one state word) from splittable SplitMix
(state plus a per-stream increment). Neither the name "SplitMix" nor the
existence of a sequential split operation establishes our indexed sibling-state
contract. Document the indexed adaptation, its cost, and why sibling states are
distinct before accepting it.

Select a small initial set of alternatives with useful differences. At least one
additional search-capable engine should exercise the extension interface before
release. Preserve xorshift64* as a comparison candidate; do not invent an untested
splitting construction just to keep it as the default. Record the default decision
after correctness, published quality evidence, and Gecode measurements agree.

### Ownership across selectors and branchers

This decision must be closed before the Phase 2 integration is considered done.
Random variable selection and value selection may currently share a handle or
use separate handles; later posted branchers may also retain those handles.
Splitting only the active selector leaves later consumers unchanged.

Start by evaluating a space-local random context shared by cooperating branchers,
using Gecode's existing local-object cloning mechanism if appropriate. Compare it
with compact state held directly in branchers. Choose the smallest design that
meets the contract; neither representation is mandated by this plan.

Specify what reusing a generator in multiple descriptions means, how distinct
generators coexist, when external initialization is bound to a space, and how
later branchers inherit the alternative-specific state. Cover deterministic
choices before a randomized brancher: the later random stream must reflect the
selected alternative even when the earlier choice used no random values.
Also cover one-alternative assignments, dynamically posted branchers, and custom
callbacks. Explain where splitting state is captured and installed so custom
branchers have a clear participation contract.

Use a small worked example with two sequential branchers, random variable and
value selection, and both shared and separate initial generators to settle this.
Do not add a general stream registry or per-node map without demonstrating why
a simpler ownership model cannot meet these cases.

## Phase 1: Extensible generation with exact test replay

**User requirements:** 1, 2, 5, 6.

### What to build

Introduce the engine contract and a compact candidate implementation, connected
to the existing test runner through complete-state input and failure reporting.
Specify raw output, bounded output, initialization, indexed splitting, and state
encoding together. Demonstrate another engine supplied outside the implementation
without editing the engine-selection logic. Resolve the ownership design needed
for the next phase using the worked example above.

### Acceptance criteria

- [x] Saving after mixed bounded draws and splits, restoring, and continuing
      reproduces both future draws and future split states exactly.
- [x] A deliberately failing test iteration can be replayed directly from the
      reported command, including when the failure is an exception.
- [x] Published vectors validate the selected engine where available; focused
      checks cover invalid state, seed expansion, bounds, and state round trips.
- [x] The indexed splitting rule has an argument for sibling-state distinction
      and handles the full valid alternative-index range without linear replay.
- [x] Custom engine state size is not artificially fixed to the default's size.
- [x] The ownership decision and initial memory measurements are recorded.

### Phase 1 review

Implemented `Support::Random<Engine>` with full-state encoding and integer
rejection sampling, a 16-byte splittable SplitMix candidate, and the 8-byte
xorshift64* standalone alternative. The old congruential engine remains named
for comparison, but is no longer the default. Final default selection remains
subject to the Phase 3 measurements.

For SplitMix parent `(s,g)`, alternative `a` gets state
`(Mix13(s+(2a+1)g), mixGamma(s+(2a+2)g))`, using unsigned 64-bit arithmetic.
This is the child of the `(a+1)`th sequential SplitMix split, computed directly.
Since `g` is odd and `a` is 32 bits, the first inputs are distinct modulo 2^64;
Mix13 is a permutation, so the complete sibling states are distinct. No new
generator recurrence or probabilistic collision assumption is introduced.

The chosen ownership direction for Phase 2 is a lazily allocated space-local
collection of bound streams. Reusing the same initialization handle binds the
same local stream; separate handles retain separate streams. Space cloning
duplicates their mutable state. Choices snapshot all bound streams, including
ones belonging to later branchers, and commit derives each from the recorded
alternative. This also handles deterministic choices preceding random ones.
A compact linear collection is sufficient for the normally small number of
streams; there is no general registry or per-node lookup map. Raw engine words
are packed into one optional choice payload, with layout implied by the bound
engines. Phase 2 must measure and review the actual overhead of this design.

Worked example: descriptions for branchers A and B reuse handle R, and A's
variable and value selectors both use R. All three bind one stream in the space.
After A's selections, its choice records R's state. Committing alternative 1
installs R's child 1, which B subsequently uses. If A's value selector instead
uses a separate handle V, the choice stores R and V once each and commits both
child states. Replaying from an earlier clone uses those snapshots even though
A's selection draws were not re-executed.

Validation: `Random::Contract` passed, including known raw-output vectors,
mixed draw/split replay, invalid state/seed input, bounds, and a user engine with
three state words. The dedicated `random-state-replay` CTest passes both ordinary
failure and exception replay by executing the printed command and comparing the
failing draw and state. The exception fixture first completes two iterations,
so this checks an advanced state rather than just initial seeding. Filtered
random tie selection also passes with the maximum 64-bit seed.
The existing `check` target passes, including its fault-injection checks and
selected integer, set, float, FlatZinc, branching, and search regressions.

Measured on arm64 macOS: engine 4 -> 16 bytes; Rnd 8, IntVarBranch 112,
IntValBranch 80, Choice 16, integer PosValChoice 24, and Space 288 bytes remain
unchanged in this phase. Search still has its old shared-handle behavior until
Phase 2; this intermediate limitation is intentional and not a completed search
reproducibility claim.

## Phase 2: Alternative-specific streams through one search path

**User requirements:** 1, 3, 4, 6.

### What to build

Carry the new random state through a representative integer branching path,
choice archiving, commit, and sequential recomputation. Include both random
variable and value selection and the transition to a second brancher. Exercise
the same path with a user-supplied engine to prove that extension reaches search.

### Acceptance criteria

- [x] All alternatives of a recorded choice have distinct successor states.
- [x] Committing an alternative directly, after intervening sibling exploration,
      or after restoring an archived choice produces identical successor state
      and subsequent draws on equivalent spaces.
- [x] The same recorded path yields the same state with frequent cloning and
      substantial recomputation, including last-alternative optimization.
- [x] A deterministic choice followed by random branching, and a transition
      between randomized branchers, both retain the alternative-specific stream.
- [x] Shared and separate variable/value generators follow the documented
      ownership policy; cloning does not mutate the source's generators.
- [x] Choice payload growth is independent of the number of alternatives.
- [x] Measure description, brancher, choice, and archive sizes against baseline.

### Phase 2 review

Implemented the space-local collection and common choice/commit integration.
`Rnd` accepts a user engine through `Support::Random<Engine>` and keeps its
8-byte handle representation. Each space binds initialization handles to local
streams and clones those streams independently. Selector and model handles are
mapped back to the corresponding local stream during copying. Standalone handles
retain handle-style sharing; `copy()` makes an independent exact copy.

All bound state words are captured after choice selection and restored/split
before the brancher's commit. Archives include the complete packed payload.
This works for custom choices that use the standard space choice/commit and base
choice archive interfaces, without per-brancher snapshot code. Binary and multiway
choices have the same random payload for the same registered streams. The small
selector binding change was applied to set and float selectors in this phase too,
because the ownership contract is common; their wider verification is Phase 3.

The global draw mutex is removed: mutable streams are space-local, and standalone
handles are documented as requiring caller synchronization if shared by threads.
Making numeric construction explicit also exposed a FlatZinc decay constructor
that accidentally converted a double to a random seed. It now selects the actual
decay constructor explicitly.

Validation: `Random::BranchReplay` checks reverse-order alternative exploration,
archived choices replayed on pre-selection clones, intentionally perturbed
destination state, shared/separate streams, deterministic-to-random handover,
and a user engine with three state words. For each configuration, all 81 solution
values and final states agree between commit distances 1 and 100. Both random
tests and filtered tie selection pass twice, state-report replay still passes,
and the full existing `check` target (including fault tests) passes.

Arm64 sizes: Rnd 8, IntVarBranch 112, IntValBranch 80, and selectors remain
unchanged. Space grows 288 -> 296 bytes; Choice 16 -> 24 and integer PosValChoice
24 -> 32 bytes. One default stream adds a separately allocated 24-byte snapshot
(8-byte length + 16-byte state), making the logical integer choice footprint
56 bytes before allocator overhead. Its archive grows from 3 to 8 unsigned words.
Two default streams use a 40-byte snapshot; the custom engine uses 8 additional
bytes per stream. Nonrandom choices have no snapshot allocation but pay the
8-byte optional pointer and one archive count word. These costs are explicit
inputs to Phase 3 performance review, not yet a performance acceptance claim.

## Phase 3: Complete branching and search integration

**User requirements:** 1, 3, 4, 6.

### What to build

Apply the verified contract across integer, Boolean, set, and float random
branching, tie breaking, multiway branching, assignment, and supported custom
branchers. Check the common commit boundary and both sequential and parallel
search replay paths. Account explicitly for restarts, relaxation, and model
callbacks that use randomness outside ordinary choice selection.

### Acceptance criteria

- [x] Focused integration cases cover binary, multiway, and one-alternative
      branching, plus handover to later branchers and custom-engine use.
- [x] Alternative identity follows the public choice index, including when a
      brancher reverses the mapping from alternative index to selected value.
- [x] Clone, archive, disposal, failure, traced commit, and conditional commit
      paths preserve the contract without shared mutable state between spaces.
- [x] Sequential and parallel replay of the same recorded path agree on random
      state; checks do not require identical parallel solution order.
- [x] Restart and portfolio initialization policies are documented and do not
      accidentally introduce worker-scheduling-dependent shared streams.
- [x] Existing relevant branching and search correctness tests pass. Add tests
      for the new semantic guarantees rather than duplicating each API wrapper.
- [x] Random generation, bounded draws, splitting, clone cost, and retained-path
      memory are measured in representative Gecode workloads. Use controlled
      paths to separate overhead from changes in the randomized search tree.

### Phase 3 review

Added const-space stream lookup for callbacks and retained stream identity across
ancestor handles. Review showed that mapping only the immediate source space
was insufficient for a callback capturing an already bound ancestor handle.
The collection now holds one handle per stream; each local implementation keeps
its initialization origin alive. Mutable states remain independent across spaces.
The net per-stream collection/implementation storage is unchanged from Phase 2;
the default standalone implementation gains an 8-byte origin handle.

FlatZinc binds its restart/relaxation handle to the space and derives meta-engine
streams from fixed restart/portfolio indices. The shared relaxation helper and
the Photo and JobShop examples now use local handles. Generic cloning still does
not split; models have an explicit `random_split()` operation for their own
meta-engine policy. Callback, registration, and replay boundaries are documented
in `docs/random.md`.

Xorshift64* is now also a search-capable alternative: indexed splitting jumps
`(a+1)*2^32` native recurrence steps using shared transition matrices. This keeps
8-byte engine state, costs a shared 16 KiB table, and preserves the original
generator rather than inventing a new seeding construction. Composition and
period-wrap checks supplement its raw vector and full-state replay checks.
Its sibling-state distinction follows from coprimality with the native period.

Validation passes for three engines (SplitMix, xorshift64*, and an external
three-word engine), shared/separate streams, binary/multiway choices, dynamic
posting through a one-alternative custom brancher, const callbacks, and sequential
versus two-worker search. Complete solution/state sets agree in the parallel
fixture without assuming solution order. Traced and conditional commits, failed
spaces, and illegal alternatives are checked explicitly. A dedicated fault test
counts live engine instances across allocation and brancher-copy failures and
checks that source states survive. Existing Boolean, set, float, FlatZinc restart,
and the full `check` selections also pass.

Release measurements use Apple Clang 21, arm64 macOS 26.6.2, baseline commit
6b7de57b04, one warmup, five repetitions for seed 42, and three repetitions each
for seeds 1 and 1337. Raw records are in `build/random/phase3-benchmark*.json`;
the reusable harness and summary are documented in `docs/random.md`.
Representative seed-42 medians: bounded Rnd draws 14.85 -> 2.96 ns; random-space
clones 178.70 -> 345.95 ns; controlled random binary-tree nodes 153.07 -> 254.32 ns
with frequent cloning and 226.76 -> 368.53 ns with recomputation. Nonrandom tree
overhead was 1–3%. Queens wall time rose about 4–5% across the three seeds, with
similar but not identical node counts. These are local measurements, not a
cross-platform performance guarantee.

The cost of independent streams and recorded state is real: the smallest random
tree is about 1.6x slower, despite much faster individual draws. We retain the
simple optional packed snapshot representation rather than add a second compact
choice hierarchy to hide that cost. SplitMix is the preferred default: binary
split-plus-draw measured about 15.7 ns, versus 237 ns for xorshift's jump-plus-draw.
The latter remains the 8-byte-state alternative. The popcount implementation was
improved after measurement without changing its bit sequence. Phase 4 will make
the executable's default configurable and verify both configurations.

## Phase 4: Configured command lines and Gecode 7 migration

**User requirements:** 1, 2, 5, 6.

### What to build

Complete seed/state handling in the example driver and FlatZinc, propagate the
configured engine through their random consumers, and finalize the default and
initial alternatives using the preceding evidence. Document the extension API,
ownership rules, replay contract, and compatibility changes for Gecode 7.

### Acceptance criteria

- [x] Drivers accept full 64-bit seeds without signed narrowing or truncation
      and use the same initialization/state conventions as the test runner.
- [x] Complete state is accepted and reproduced for the configured engine;
      malformed, incompatible, and conflicting options are rejected clearly.
- [x] Time/hardware initialization can report the concrete initialized state
      needed for a later replay.
- [x] A runnable custom-engine example works through built-in branching; custom
      brancher documentation explains choice snapshots and replay obligations.
- [x] FlatZinc restart sampling no longer relies on the old generator's restricted
      range or sequence-preservation workaround where the new contract replaces it.
- [x] Release notes describe changed seeded sequences, state replay, and copying
      versus sharing semantics. No legacy sequence mode is required.
- [x] Supported build configurations and relevant regression suites pass; the
      default choice and measured memory/performance tradeoffs are documented.

### Phase 4 review

The example driver and FlatZinc share `Driver::RandomOption`: checked 64-bit
decimal/hex seeds, full-state input, and mutually exclusive seed/state arguments.
The existing seed flag names remain `-seed` and `-r`, respectively. `time` and
`hw` initialize once and print the resulting state. Hardware-source failures now
throw rather than return uninitialized data. `opt.rnd()` produces an independent
generator at the configured initial state; all in-tree numeric seed consumers
were migrated. The numeric accessor rejects state-only initialization rather
than silently ignoring it.

Both CMake (`GECODE_RANDOM_ENGINE`) and Autoconf (`--with-random-engine`) select
SplitMix or xorshift64*. The generated, installed configuration header carries
that selection to clients. SplitMix remains the default for the measured splitting
cost; xorshift64* remains the 8-byte alternative. Both concrete engine types and
the public custom-engine interface are available in either build.

`examples/random-engine.cpp` uses a three-word user engine with built-in random
selectors. It delegates generation/splitting to SplitMix and adds a path-local
draw counter, demonstrating full-state extension without another algorithm.
Seeded and full-state runs enumerate the same 24 permutations and solution states.
The output also agrees across both default configurations and the no-thread and
static builds. The old unused FlatZinc restart sampler and its chunk-specific test
are removed. The range-validation and restart integration tests remain. Review
also corrected the FlatZinc test harness to pass its configured generator into
parsing, and replaced an unsupported test `--seed` flag with `-r`.

Validation on arm64 macOS with Apple Clang 21:

- Full CMake `check`, including fault-injection tests, passes with both defaults.
  All enabled example targets also build with the default configuration.
- `random-options` and `random-state-replay` CTests pass with both defaults.
  CLI checks cover maximum-width decimal/hex seeds, arbitrary full states
  (including a non-default SplitMix increment), incompatible identifiers,
  malformed/conflicting input, and replay of reported time/hardware state.
- Boolean, set, and float branching selections, filtered random ties, and all
  FlatZinc restart tests pass with both defaults. A direct FlatZinc run enumerates
  the same 27 assignments from a maximum-width seed and its complete state.
- A reduced static CMake build with threads, set, float, and FlatZinc disabled
  passes the random contract/branch replay/commit tests and both replay/CLI CTests.
- Autoconf builds all libraries and the custom example with xorshift64* and
  threading disabled. Out-of-tree direct example building requires the existing
  `make mkcompiledirs` preparation target. Its output matches the CMake builds.

Logs and comparison outputs are under `build/random/phase4-*`; configurations are
in `build/random`, `build/random-xorshift`, `build/random-static`, and
`build/random-autoconf`. These are local configuration checks, not a claim of
having run every platform's CI. `docs/random.md` and the Gecode 7 changelog section
describe the API, compatibility changes, and measured costs. Release version and
ABI-number changes remain release preparation, outside this feature plan.

### Completion audit

Requirements 1 and 5 are exercised by both configured defaults, the CLI fixture,
and the separately defined custom engine. Requirement 2 is exercised by commands
replayed from actual failure and exception reports, plus driver full-state replay.
Requirements 3 and 4 are exercised by immutable choice snapshots, reverse sibling
exploration, archived replay on pre-selection clones with perturbed state,
multiway and callback handover, and equivalent solution/state sets under cloning,
recomputation, and parallel search. Review confirms that all variable/value
selectors bind and remap local streams and that commit installs them before
callbacks. Requirement 6 is covered by the retained Phase 3 measurements and
explicit description, space, choice, snapshot, archive, and splitting costs.
The final review found no uncompleted acceptance criterion in this plan.

## Validation boundaries

Keep a small set of high-value tests: reference vectors, save/restore including
splits, direct failure replay, and equivalent-path integration tests. Assertions
on state are stronger than expecting different first outputs from siblings:
distinct states can legitimately produce equal individual outputs.

Do not put probabilistic distribution tests into ordinary CI. Use established
statistical tools during engine evaluation when needed, especially for any
indexed-splitting adaptation. Passing a statistical battery alone is not evidence
that sibling states are always distinct or replay is correct.

Implementation proceeds in the phase order above. Each phase review records
its evidence and any adjustment before the phase is committed.

## Initial code observations supporting the plan

- `gecode/support/random.hpp`: current state is one unsigned integer, normally
  32 bits. Bounded output combines low-bit chunks and scaling/modulo operations.
- `gecode/kernel/data/rnd.hpp` and `.cpp`: public random handles share mutable
  implementation state and use a static mutex across implementations.
- `gecode/kernel/branch/var.hpp`, `val.hpp`, and `view-sel.hpp`: branching
  descriptions and selectors retain these handles, including during cloning.
- `gecode/kernel/branch/view-val.hpp` and `gecode/int/branch/view-values.hpp`:
  choices currently store selected positions/values, without random snapshots.
- `gecode/kernel/core.cpp`: exploration and replay reach brancher commit through
  common space operations; archive reconstruction dispatches by brancher identity.
- `gecode/kernel/core.hpp`: space-local objects already support cloning shared
  objects within a space. Their suitability must be weighed against overhead.
- `gecode/kernel/archive.hpp`: archive storage uses unsigned integer words and
  has no existing 64-bit integer overload; explicitly preserve every state bit.
- `gecode/search/seq/path.hpp` and `gecode/search/par/path.hpp`: retained choices
  and alternative indices drive recomputation and make payload size significant.
- `test/test.cpp`: iteration replay currently treats the current generator state
  as an unsigned seed; exceptions report the suite seed instead.
- `gecode/driver.hh` and `gecode/flatzinc.hh`: seed option types differ, with a
  signed seed path in FlatZinc.
- `gecode/flatzinc/restart-random.hpp`: special handling compensates for the
  current bounded generator while preserving established sequences.

## Algorithm references

- [Steele, Lea, and Flood: Fast Splittable Pseudorandom Number Generators](https://gee.cs.oswego.edu/dl/papers/oopsla14.pdf)
  describes the splittable SplitMix design.
- [Vigna: older scrambled linear generators](https://prng.di.unimi.it/xorshift.php)
  discusses limitations of the xorshift family and its low bits.
- [Blackman and Vigna's generator overview](https://prng.di.unimi.it/)
  provides modern alternatives, reference implementations, and seed-expansion
  guidance. Fixed-increment SplitMix64 is distinct from full splittable SplitMix.
