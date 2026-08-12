+++
schema_version = 1
id = "word-044"
key = "add-realistic-word-examples"
area = "word"
status = "done"
blocked_by = ["word-043"]
+++
# Add realistic Word modeling and search examples

## Outcome

Users have small normal Gecode examples for ARX preimages, Word-array lookup/register files, and bitboard-style counting that demonstrate effective information-flow-aware Word branching.

## Boundaries

- Add ordinary Script examples through the established example inventories; do not add a durable benchmark runner, raw results, profiling code, or a new documentation subsystem.
- Use only public Word APIs and standard search options. Do not add an ARX-specific propagator or encode one global branching/recomputation policy into WordVar.
- Keep each example deterministic, compact, and useful as modeling documentation rather than as an exhaustive performance suite.

## Done when

- [x] An ARX preimage example, a lookup/register-file example, and a bitboard/counting example build and run with stable documented solutions or solution counts.
- [x] The examples branch all relevant decision variables in an information-flow-aware order and concisely document when search copy/recomputation settings can matter.
- [x] Normal focused example validation confirms the intended models without adding bespoke testing infrastructure.

## Validation

- Build the three focused example targets through the normal CMake example inventory.
- Run each example with its small deterministic settings and check its stable output.
- Run git diff --check and inspect CMake/Make example integration.

## Result

Added and independently verified three normal Word Script examples for ARX preimages, register-file lookup, and bitboard counting.

Validation:

- All three focused Release example targets built and each produced exactly one stable solution.
- CMake/Make Word-conditional inventories, zdev checks, and git diff --check passed.
