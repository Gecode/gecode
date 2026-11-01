+++
schema_version = 1
id = "boost-005"
key = "implement"
area = "boost"
status = "open"
complexity = "advanced"
afk = true
blocked_by = ["boost-004"]
+++
# Implement the chosen interval-arithmetic approach

## Outcome

Float variables use the approach the user chose, and the brief's success criteria hold. This is the area's terminal task: later areas depend on it.

## Context

Read the decision in the brief and the background studies. This task is expected to split into children through the derived-work contract, before editing, into coherent steps for the chosen approach (for example: a reproducible extraction script and refreshed copy, or the support-library core, the rounding layer, the FloatVal migration, and removal of gecode/third-party/boost). It stays open until its children are done, then completes with the full validation below.

## Boundaries

- No external dependency (see the brief's Terms and settled decisions).

## Done when

- [ ] Installed headers either contain a vendored copy that a committed script reproduces exactly, with a check like the generated-source freshness check, or no Boost code at all.
- [ ] The build states and applies the compiler flags or pragmas that directed rounding relies on, and the rounding probe or its committed test passes with every compiler from the probe.
- [ ] The decision is documented in a Doxygen comment beside the interval implementation, or in docs/ if it spans several files, and changelog.in describes the change under 7.0.0.
- [ ] The full float suite, including sweep-tagged tests, passes in Debug and Release with and without MPFR.
- [ ] The four float examples give the same solutions as before. Changes in search statistics are explained in the result.

## Validation

- Debug and Release builds with float variables, with and without MPFR, ctest, and `bin/gecode-test -test Float::` including sweeps.
- Last task in the area: run `uv run misc/check.py all --build-dir build/full --jobs N`. Fix failures that this area caused. Report failures that already exist at the area base.
