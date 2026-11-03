# Plan: Extensible random generators and reproducible splitting

> Provisional draft for Gecode 7 or another future breaking-change release only.
> Status: implemented and locally verified; design remains under review in draft PR #241.

## Goal

Replace the old default generator, provide user-extensible alternatives and exact
state replay, and split randomized branching states by alternative. Keep states
small enough to store directly in the model, selector, or brancher that uses them.

## Required behavior

1. A generator is a value containing its engine state inline. Copying it produces
   independent state, with no allocation, registry, or aliasing for built-in engines.
2. Model members and selectors own their own copies. Posting or cloning copies
   state without drawing. Posting splits each selector by brancher id and role,
   so passing the same value to two consumers gives them independent streams.
3. Selection draws from a copy, so computing a choice leaves selector state
   unchanged. Committing alternative a splits the state of every selector of
   that brancher by a. The state at a node is thus a function of the posted
   state and the path, and cloning, recomputation, and archived replay agree.
4. Each sibling index produces a distinct successor state. Cloning does not
   split. Late alternatives do not replay preceding siblings. Choices store no
   generator state.
5. Deterministic branchers, unrelated branchers, and model-owned RNGs are not
   implicitly advanced. Custom owners explicitly record/split their own state
   when required; hidden callback randomness is not automatically enrolled.
6. Space and Choice representations have no additional RNG fields or archive
   words, for randomized and deterministic branchers alike.
7. Users can define engines with different state sizes and use generic branching
   machinery. No runtime engine registry or fixed custom-state-size cap is needed.
8. Checked 64-bit seeds and canonical full-state text remain distinct interfaces.
   Drivers use one configured engine. Test failures and exceptions report exact
   iteration-state replay commands.
9. Restart and portfolio engines split every brancher of each slave by the
   restart or asset number, behind separate separators, so slaves explore
   different random paths even when the engines nest.
10. This remains a provisional breaking-change design, not a compatibility release.

## Implementation

### Value ownership

- [x] Replace shared-handle Rnd with RndGenerator<Engine> and a configured Rnd alias.
- [x] Copy selector and model generators normally; remove random-handle disposal
      overhead for trivially destructible built-in engines.
- [x] Make relaxation take a generator reference, explicitly advancing its owner.

### Brancher-local replay

- [x] Add a selector random_split hook with a no-op default, and a virtual
      Brancher::random_split that splits all selectors of a brancher.
- [x] Draw from copies in selection; split on commit only.
- [x] Key selectors at posting by brancher id and role (value 0, variable
      selector i at 1+i).
- [x] Add Space::random_split, called by RBS and PBS on each slave.
- [x] Cover binary, multiway, assignment, reversed alternatives, LDSB, open-list,
      and FlatZinc int-bool branchers.
- [x] Support user-parameterized variable and integer/Boolean value selectors
      through the existing generic brancher machinery.
- [x] Keep model/custom-brancher state transitions explicit.

### Tests and documentation

- [x] Test independent copies, untouched model/later-selector state, and
      consumer-local splitting.
- [x] Test that branchers sharing a source differ, that open-list and int-bool
      results do not depend on the recomputation distance, and that restarts
      find different solutions.
- [x] Check direct/archived choices, perturbed destination selector state,
      sibling exploration, cloning/recomputation, and parallel solution agreement.
- [x] Exercise a three-word external engine in selectors and a runnable example.
- [x] Retain engine vectors, full-state failure replay, CLI validation, and
      failed-clone resource checks.
- [x] Document engine extension, splitting, full-state replay, and migration.
- [x] Run full relevant checks for both defaults and the reduced static/no-thread build.
- [x] Measure compactness and representative costs against main.
- [x] Review the implementation and update the provisional draft PR.

## Verification strategy

Use the focused Random::Contract, Random::BranchReplay, Random::CommitBoundary,
Random::Streams, and FlatZinc::random_intbool_branch tests. Fault::Random::CloneFailures counts inline
custom-engine instances across failed clones. Existing Boolean, set, float,
assignment, LDSB, and FlatZinc restart cases cover their integration paths.

Run random-options and random-state-replay CTests with both configured engines.
Verify that the custom example's initial state replays its output. Keep direct
assertions on state and choice archives; different first outputs alone do not
prove sibling-state distinction.

Measure against the same main baseline (6b7de57b04), using the existing controlled
tree and queens harness. Report actual generator/description/selector/choice sizes
and any performance costs.

## Algorithm and CLI decisions

Splittable SplitMix has two state words and constant-time indexed splitting.
Xorshift64* has one state word and splits by hashing s + (2a+1)g with Mix13.
Jumps along its recurrence were rejected: they add up, so split(0) of split(0)
equals split(1), correlating subtrees. A zero input hashes (2a+1)g instead, so
each split is a bijection on nonzero states. Seeds are hashed the same way, as
related raw seeds of the linear recurrence give related streams. Bounded draws
use the high bits, since xorshift64*'s low bits are linear. Both engines have
documented sibling-state arguments in gecode/support/random.hpp. The default remains
provisional SplitMix; xorshift64* remains configurable through CMake and Autoconf.

Support::Random<Engine> defines bounded integer conversion and canonical full-state
encoding. RndGenerator<Engine> provides the modeling value interface. Full state
includes every word needed for future draws and splits. Seed expansion never
substitutes for state restoration.

Drivers retain -seed (examples), -r (FlatZinc), and -state. Time/hardware
initialization reports concrete state. Incompatible, malformed, and conflicting
input is rejected. No legacy sequence mode is required.

## Split analysis

PractRand 0.95 (expanded tests, full folding) passes 32 GB of first draws for
both engines along leftmost, random binary, and random 10-way paths, in
depth-first order over complete trees, for all 2^32 siblings of one node, for
siblings under many parents, for parent and child draws side by side, for
posting keys, and for restart slaves; binary draws pass 16 GB. Splits without
the hash fail all of these at 32-64 MB, and the old jump split fails the
depth-first stream at 32 MB. Single-bit avalanche of every split derivation is
at the noise floor, and trees of 1.1-1.3 * 10^8 nodes repeat no state.
Unsplit xorshift64* low bits fail at 128 MB, which motivated high-bit draws.
Repeated states at about 2^32 nodes are expected for the one-word engine and
harmless: two nodes then make the same random choices.

## Verification results

With the split-on-commit design, both engine configurations (xorshift64* built
with experimental engines) pass CMake check and all CTests. The Random and
FlatZinc random tests pass with both engines. The SplitMix build also passes
all Branch, Assign, FlatZinc, Search, and NoGoods tests. The custom-engine
example's full-state replay agrees with its output.

The Space and base Choice implementation matches main exactly, and randomized
choices are as large as deterministic ones. Rnd is 16 bytes with SplitMix and
8 with xorshift64*, without shared allocation. The cost measurements in the PR
description predate this design and must be repeated. Draft PR #241 remains
provisional, for a future breaking-change release only.
