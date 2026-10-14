# 15-puzzle search integration

The BAB-only example from `feature/open-cp` was integrated at parent commit
`60325e4801d092ffad42b014bbca7708039b5826`. The MPG measurement matrix is
finished. Task `open-search-037` delivered the extension and focused validation.
Do not change MPG's source, results, or frozen measurement library.

Task `open-search-038` carries forward the deferred goal correction from CP
commit `81fff38e8e843e29c0d26a65e004fca1e926ee87`. The open model waits for board
assignment before closing the path or requiring a successor; Manhattan distance
stays in heuristic guidance. Upstream validation found that open BAB's default
exceeds a 30-second Debug cutoff after losing early backward goal propagation.
The default slowdown was addressed in task `open-search-039`.

Task `open-search-039` carried that follow-up from CP commit
`fc26a2a8fcb78d5b1c617e341a1ef13224f40019`. Assigned boards fail when their
depth plus Manhattan distance exceeds the current move cap. A materialized
board at the final cap is constrained to the goal, including after incumbent
cuts. These rules preserve deferred construction and the heuristic extension.
Focused Release validation completed the open defaults in 9–10 ms, with the
two-board root, clone independence and pending-cap behavior checked separately.

## Prepared extension

Extend upstream `examples/fifteen-puzzle.cpp`, preserving its documentation,
size validation, shared slide/goal rules, and separate open/fixed models.
Upstream owns the example's CMake and Autoconf registration.

- Add `bab`, `astar`, and `cabs` search selections.
- Add open-model depth and both models' total Manhattan bounds:
  `max(depth + Manhattan(current board), minimum length - 1)`.
- Supply the same minimizing bound through `Search::HeuristicGuidance` to
  A* and CABS. Use small engine adapters for `Script::run` and the default
  unrestricted CABS widening policy.
- Guard experimental registrations, guidance, and adapters with
  `GECODE_HAS_EXPERIMENTAL`; keep BAB available in ordinary builds.
- Use upstream `known_length()` in the open-model depth query. Update the
  downstream TSPTW caller in `examples/tsptw-transition.hh` when integrating
  the parent rename.

Preserve standard `INT_VAL_MIN` branching. Length includes the start and moves
are length minus one. The constructor posts the start unconditionally; the
materialization transition returns for index zero. Keep this an educational
example without an embedded oracle, optimum assertions, or benchmark machinery.

Reference implementation:
`/Users/zayenz/.codex/worktrees/bba6/MPG/rst/examples/src/fifteen-puzzle.cpp`.

Build the example with experimental search enabled and disabled, and run
small representative open/fixed cases
through BAB, A*, and CABS. Use the existing example validation conventions;
the extension does not need a new permanent test harness.

## Repeated posting and memory

The MPG/CP handoff reports roughly 698.5 MiB peak RSS for open BAB versus
5.9 MiB for fixed BAB at scramble 16, limit 16, seed 3, with 780737 nodes in
each run. These measurements are preliminary and have not been independently
checked here. The growing result file is
`/Users/zayenz/.codex/worktrees/bba6/MPG/rst/examples/results/fifteen-puzzle-engines-60s.csv`.

Source inspection confirms that freshly posted propagators allocate
`Kernel::GPI::Info` from shared space data (`gecode/kernel/core.hpp`,
`Propagator::Propagator(Home)`). Clones reuse the existing pointer. GPI allocates
entries monotonically in blocks and frees the blocks in its destructor
(`gecode/kernel/gpi.hpp`). Open transitions repeatedly post new constraints,
so this metadata can accumulate across explored states even after those spaces
are deleted. The fixed formulation posts its constraints initially.

Follow up by measuring allocation counts and memory attributable to GPI, and
profiling runtime to distinguish posting/propagation costs from search costs.
The inspected lifetime fits the reported RSS difference; its contribution
still needs measurement. Any change to allocation or AFC sharing needs separate
design work and should leave the ongoing comparison untouched.
