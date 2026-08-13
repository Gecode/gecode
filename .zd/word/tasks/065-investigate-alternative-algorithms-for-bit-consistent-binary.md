+++
schema_version = 1
id = "word-065"
key = "binary-add-algorithms"
area = "word"
status = "done"
blocked_by = []
+++
# Investigate alternative algorithms for bit-consistent binary Word addition

## Outcome

Determine whether a different native algorithm can materially outperform the current bit-consistent binary Add carry automaton, or establish that its linear support scan is close to the necessary floor.

## Context

Speck profiling identifies binary Add as the dominant algorithmic frame. The current actor is already a native two-state carry automaton with per-bit forward reachability, backward reachability, and support projection. Earlier tasks optimized transition lookup, publication, and n-ary addition; this task investigates algorithmic alternatives rather than more local code tuning.

## Boundaries

- Do not reopen NaryAdd, propagation-cost tuning, or ordinary changed-only publication work.
- Keep prototypes and benchmark artifacts outside the repository.
- Investigate broadword or block carry-transfer composition, precomputed transfer/support summaries, and fused reachability/support scans.
- Consider staged or advisor-based execution only if avoided work repays its subscription, cloning, and disposal costs.
- A weaker but faster prototype may be measured, but retaining weaker propagation requires a separate user-approved change to the bit-consistency contract.
- Do not add normal tests unless a production candidate is retained.

## Done when

- [x] The current actor's work is decomposed into measured phases, including calls, bit scans, transitions, support projection, publications, and useful changes.
- [x] At least two credible alternative binary-Add algorithms are implemented or analytically rejected with concrete complexity and Gecode-lifecycle reasoning.
- [x] Temporary candidates are compared against the exact current baseline on symbolic ALU, Speck32/64, focused forward and inverse Add cases, and MD5 as a secondary regression guard.
- [x] Exact candidates preserve assigned semantics, bit consistency, aliases, AddCarry terminal-carry behavior, actor lifecycle, and search solution parity.
- [x] The result establishes either a retained material improvement or a documented negative conclusion that the required support projection makes the current linear scan close to the practical floor.

## Validation

- Use Release builds and preserve temporary commands, raw results, profiles, and exact baseline identity.
- Run focused Word Arithmetic Add, Lifecycle, and TestFramework tests for any retained production change.
- Report wall time, propagations, nodes, failures, solutions, actor counts, phase counts, and best-effort memory.
- Run zd check word and git diff --check.

## Result

Replaced repeated binary Add tuple scans with exact immutable transition/support summaries and fused directional passes, materially improving ALU, Speck, and MD5 while preserving bit consistency and lifecycle.

Validation:

- Independent Spec and Standards verification passed, including exhaustive table algebra, aliases, AddCarry terminal states, width 1/64, lifecycle, and cross-propagator audit.
- Release Arithmetic, Overflow carry/lifecycle, and TestFramework tests passed; exact benchmark parity, provenance hashes, zd check word, and git diff --check passed.
