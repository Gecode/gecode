+++
schema_version = 1
id = "word-002"
key = "test-framework"
area = "word"
status = "done"
blocked_by = ["word-001"]
+++
# Extend the Gecode testing framework for WordVar

## Outcome

The generic Gecode test framework can describe partial word domains, enumerate concrete and partial assignments, construct and clone WordVar test spaces, compare results with a reference oracle, and supply the hooks needed by later direct, shared, and reified word-propagator tests.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Do not implement or test any word posting propagator in this task.
- Keep word-specific enumeration and formatting in the shared testing framework rather than copying helpers into later test files.
- Avoid exhaustive configurations whose size grows beyond controlled small-width test domains.

## Done when

- [x] The test framework has WordVar assignment, domain, formatting, and TestSpace support analogous to the applicable Int, Set, and Float testing facilities.
- [x] Framework APIs support direct, shared-variable, clone/recomputation, and all applicable reification test shapes required by later tasks.
- [x] A reference enumerator can determine the concrete values represented by a lo/hi word domain and compare expected supported values without using a word propagator.
- [x] Framework self-tests cover assignment enumeration, partial-domain generation, width handling, cloning, formatting, deterministic sampling, and tractable exhaustive limits.

## Validation

- Run the WordVar testing-framework self-tests without enabling any word propagator test suite.
- Run representative existing Int, Set, and Float framework tests to detect regressions in shared test infrastructure.
- Review later task interfaces against the framework and confirm they do not require private duplicate assignment or oracle machinery.

## Result

Extended the normal Gecode test framework with reusable Word domains, bounded concrete and partial enumeration, complete and deterministic sampled assignments, formatting, clone-safe TestSpace support, and direct/shared/reified posting hooks.

Validation:

- Independent pattern verification passed; Word::TestFramework and representative Int, Set, and Float tests passed in the normal gecode-test binary; Word-disabled CMake filtering and git diff checks passed. No bespoke executable or Word propagator test was added.
