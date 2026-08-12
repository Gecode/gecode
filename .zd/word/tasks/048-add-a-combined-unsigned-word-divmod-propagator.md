+++
schema_version = 1
id = "word-048"
key = "add-combined-unsigned-divmod"
area = "word"
status = "open"
blocked_by = ["word-047"]
+++
# Add a combined unsigned Word divmod propagator

## Outcome

Models needing both unsigned quotient and remainder can post one conventional Word relation that shares divisor reasoning and intermediate state.

## Boundaries

- Add one direct divmod API for same-width Word dividend, divisor, quotient, and remainder with the existing WS_SMTLIB policy; do not add an overload matrix or MiniModel node in this slice.
- Keep existing separate div and mod APIs and actors unchanged unless focused measurements justify a semantics-preserving internal reuse.
- Use one ordinary actor and normal test/word coverage; no widening, Boolean decomposition, or durable benchmark harness.

## Done when

- [ ] The combined relation implements exact assigned quotient/remainder semantics, including divisor zero, and provides sound useful shared forward and inverse propagation.
- [ ] Normal registered tests cover assigned small widths, zero divisor, representative partial pruning, aliases, failure, clone/recomputation, and subsumption, and compare solutions with separate div plus mod postings.
- [ ] A focused exact-baseline quotient/remainder model demonstrates whether shared reasoning improves actors, propagation calls, runtime, search, or memory without changing solutions.

## Validation

- Build the focused Release Word library and gecode-test and run Div, Mod, Divmod, MiniModel arithmetic regression, and Word TestFramework filters.
- Run temporary separate-versus-combined quotient/remainder benchmarks and profiles with exact solution/checksum parity.
- Run git diff --check.
