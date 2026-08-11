+++
schema_version = 1
id = "word-002"
key = "test-framework"
area = "word"
status = "open"
blocked_by = ["word-001"]
+++
# Extend the Gecode testing framework for WordVar

## Outcome

The generic Gecode test framework can describe partial word domains, enumerate concrete and partial assignments, construct and clone WordVar test spaces, compare results with a reference oracle, and supply the hooks needed by later direct, shared, and reified word-propagator tests.

## Boundaries

- Do not implement or test any word posting propagator in this task.
- Keep word-specific enumeration and formatting in the shared testing framework rather than copying helpers into later test files.
- Avoid exhaustive configurations whose size grows beyond controlled small-width test domains.

## Done when

- [ ] The test framework has WordVar assignment, domain, formatting, and TestSpace support analogous to the applicable Int, Set, and Float testing facilities.
- [ ] Framework APIs support direct, shared-variable, clone/recomputation, and all applicable reification test shapes required by later tasks.
- [ ] A reference enumerator can determine the concrete values represented by a lo/hi word domain and compare expected supported values without using a word propagator.
- [ ] Framework self-tests cover assignment enumeration, partial-domain generation, width handling, cloning, formatting, deterministic sampling, and tractable exhaustive limits.

## Validation

- Run the WordVar testing-framework self-tests without enabling any word propagator test suite.
- Run representative existing Int, Set, and Float framework tests to detect regressions in shared test infrastructure.
- Review later task interfaces against the framework and confirm they do not require private duplicate assignment or oracle machinery.
