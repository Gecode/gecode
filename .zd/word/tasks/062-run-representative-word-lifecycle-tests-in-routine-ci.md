+++
schema_version = 1
id = "word-062"
key = "word-ci-check-coverage"
area = "word"
status = "done"
blocked_by = ["word-059", "word-060"]
+++
# Run representative Word lifecycle tests in routine CI

## Outcome

CMake and Autoconf check targets execute a proportionate registered Word test set spanning the module’s important actor and lifecycle seams.

## Context

Both routine check gates currently execute only Word::TestFramework although fourteen Word test translation units are registered. Add an established-style required test list and registration integrity guard analogous to Float.

## Boundaries

- Do not execute every exhaustive Word test in routine CI.
- Do not create a new test runner or duplicate existing tests.
- Keep CMake and Make check selections equivalent.

## Done when

- [x] Routine checks include representative kernel/channel, relation, logic, structural, arithmetic/division, mixed Int/Word, branch/trace, and MiniModel tests.
- [x] Both build systems fail clearly if a required Word test is missing or unregistered.
- [x] The selected set remains proportionate for ordinary CI.

## Validation

- Build and run the CMake check target with Word enabled.
- Build and run the Autoconf make check target with Word enabled.
- Confirm the required-registration guards pass and run git diff --check.

## Result

Added equivalent representative Word lifecycle selections and required-registration guards to CMake and Autoconf routine checks.

Validation:

- CMake Release check passed with Word and Float registration verification.
- Clean VPATH Autoconf make check passed with the same eleven Word tests.
- Selected Word subset ran in 0.69 seconds.
- Negative CMake and Make guard tests rejected bogus names with clear diagnostics.
- Independent Spec and Standards verification and git diff --check passed.
