# Plan: Compact random values and reproducible splitting

> Provisional draft for Gecode 7 or another future breaking-change release only.
> Ownership corrected after review: state belongs to the consumer, not Space.
> Status: implementation corrected; final verification and measurements in progress.

## Goal

Replace the old default generator, provide user-extensible alternatives and exact
state replay, and split randomized branching states by alternative. Keep states
small enough to store directly in the model, selector, or brancher that uses them.

The original implementation introduced a space-managed context and shared stream
identity. That was an incorrect expansion of the requirement. The earlier phase
reviews and performance claims are superseded where they depend on that ownership
model. Their history remains in commits 626fb2d307, b4ec5dfe99, 2130ef3850,
3665a1bed2, and a1ff8e3385.

## Required behavior

1. A generator is a value containing its engine state inline. Copying it produces
   independent state, with no allocation, registry, or aliasing for built-in engines.
2. Model members and selectors own their own copies. Posting or cloning copies
   state without drawing. Passing the same value to two consumers does not share
   their future state.
3. After variable/value selection, the active randomized brancher captures its
   own selectors' states in its choice. Committing alternative a derives each
   next state from that snapshot and a, not from destination mutable state.
4. Each sibling index produces a distinct successor state. Direct exploration
   and archived replay of the same choice/alternative reproduce state exactly.
   Cloning does not split. Late alternatives do not replay preceding siblings.
5. Deterministic branchers, unrelated branchers, and model-owned RNGs are not
   implicitly advanced. Custom owners explicitly record/split their own state
   when required; hidden callback randomness is not automatically enrolled.
6. Nonrandom Space and Choice representations have no additional RNG fields or
   archive words. Randomized brancher choices store only the states they need,
   in the same allocation as the choice.
7. Users can define engines with different state sizes and use generic branching
   machinery. No runtime engine registry or fixed custom-state-size cap is needed.
8. Checked 64-bit seeds and canonical full-state text remain distinct interfaces.
   Drivers use one configured engine. Test failures and exceptions report exact
   iteration-state replay commands.
9. This remains a provisional breaking-change design, not a compatibility release.

## Implementation

### Value ownership

- [x] Replace shared-handle Rnd with RndGenerator<Engine> and a configured Rnd alias.
- [x] Remove RandomContext, origin identities, binding, Space::random(), and
      Space::random_split().
- [x] Restore kernel/core.hpp and core.cpp to their pre-feature state.
- [x] Copy selector and model generators normally; remove random-handle disposal
      overhead for trivially destructible built-in engines.
- [x] Make relaxation take a generator reference, explicitly advancing its owner.

### Brancher-local replay

- [x] Add selector state save/restore/split hooks with no-op defaults.
- [x] Capture only the active brancher's selectors, in tie-break then value order.
- [x] Store words inline in a randomized choice subtype, without per-engine
      identifiers, a state array per alternative, or a separate payload allocation.
- [x] Cover binary, multiway, assignment, reversed alternatives, and LDSB choices.
- [x] Support user-parameterized variable and integer/Boolean value selectors
      through the existing generic brancher machinery.
- [x] Keep model/custom-brancher state transitions explicit.

### Tests and documentation

- [x] Replace tests of global stream coordination with tests of independent copies,
      untouched model/later-selector state, and consumer-local splitting.
- [x] Check direct/archived choices, perturbed destination selector state,
      sibling exploration, cloning/recomputation, and parallel solution agreement.
- [x] Exercise a three-word external engine in selectors and a runnable example.
- [x] Retain engine vectors, full-state failure replay, CLI validation, and
      failed-clone resource checks.
- [x] Rewrite docs and release notes to describe value ownership accurately.
- [x] Run full relevant checks for both defaults and the reduced static/no-thread build.
- [x] Remeasure compactness and representative costs after removing the context.
- [x] Review the correction and update the existing provisional draft PR.

## Verification strategy

Use the existing focused Random::Contract, Random::BranchReplay, and
Random::CommitBoundary tests. The latter now checks consumer ownership rather than
the removed kernel-wide commit hook. Fault::Random::CloneFailures counts inline
custom-engine instances across failed clones. Existing Boolean, set, float,
assignment, LDSB, and FlatZinc restart cases cover their integration paths.

Run random-options and random-state-replay CTests with both configured engines.
Verify that the custom example's initial state replays its output. Keep direct
assertions on state and choice archives; different first outputs alone do not
prove sibling-state distinction.

Measure against the same main baseline (6b7de57b04), using the existing controlled
tree and queens harness. Report actual generator/description/selector/choice sizes
and any performance costs. Do not reuse the space-local implementation's results
as evidence for the corrected design.

## Algorithm and CLI decisions retained

Splittable SplitMix has two state words and constant-time indexed splitting.
Xorshift64* has one state word and indexed native jumps. Both have documented
sibling-state arguments in docs/random.md. The default remains provisional
SplitMix; xorshift64* remains configurable through CMake and Autoconf.

Support::Random<Engine> defines bounded integer conversion and canonical full-state
encoding. RndGenerator<Engine> provides the modeling value interface. Full state
includes every word needed for future draws and splits. Seed expansion never
substitutes for state restoration.

Drivers retain -seed (examples), -r (FlatZinc), and -state. Time/hardware
initialization reports concrete state. Incompatible, malformed, and conflicting
input is rejected. No legacy sequence mode is required.

## Correction review

The ownership error was architectural, not a bug in state copying. The previous
tests verified an expanded contract that the user did not intend. This correction
removes that contract and its infrastructure instead of optimizing it.

Both engine configurations pass CMake check and all five CTests, including
command-line state replay and fault injection. The reduced static/no-thread build
passes check and the focused Random tests. Additional Boolean, set, float,
assignment, filtered-tie, LDSB, and FlatZinc restart checks pass with both defaults.
Random::BranchReplay now also exercises randomized LDSB choice archives. The
custom-engine example's full-state replay and output agree across all three builds.

The corrected Space and base Choice implementation matches main exactly. Rnd is
16 bytes with SplitMix and 8 with xorshift64*, without shared allocation. Fresh
five-run measurements are recorded in docs/random.md: SplitMix's controlled
random tree costs 1.02x with cloning and 1.16x with recomputation relative to main;
xorshift64* costs 1.63x and 2.88x. No result from the removed space-local design is
used to justify this correction. Draft PR #241 describes this corrected contract
and remains provisional, for a future breaking-change release only.
