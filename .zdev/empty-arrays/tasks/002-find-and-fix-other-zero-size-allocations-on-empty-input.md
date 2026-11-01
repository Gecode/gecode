+++
schema_version = 1
id = "empty-arrays-002"
key = "audit"
area = "empty-arrays"
status = "open"
complexity = "advanced"
afk = true
blocked_by = ["empty-arrays-001", "empty-arrays-003", "empty-arrays-004", "empty-arrays-005", "empty-arrays-006"]
+++
# Find and fix other zero-size allocations on empty input

## Outcome

Every public posting function in the int, set, float, and minimodel modules that can reach a zero-size space, region, or heap allocation from empty array input is found and fixed with a regression test, and FlatZinc and the instance reader are checked. This is the area's terminal task: later areas depend on it.

## Context

Use the brief's Terms and settled decisions: fix at the posting site, and check whether ViewArray and SharedArray allocate for size zero before changing their callers. At the area base there are about 250 `alloc<T>(n)`-style calls in gecode/. Start with a throwaway Debug driver under build/ that posts every array-taking overload declared in gecode/int.hh, set.hh, float.hh, and minimodel.hh with empty arrays and otherwise minimal valid arguments, then calls status(). The driver alone is not enough: Region::ralloc (gecode/kernel/memory/region.hpp) has no positive-size assertion, so trace allocation sizes in source for each checked path as well. Documented exceptions for empty input, such as element with an empty array, are correct behaviour. Check FlatZinc and instance-reader calls that a model can make with empty arrays, and fix frontend code only under the brief's reachability condition. If the inventory finds more than a few families to fix, record it first, then split the repairs before editing into children grouped by module or posting family through the derived-work contract. This task stays open until they are done, then completes with the final validation.

## Boundaries

- Do not change behaviour for non-empty input.

## Done when

- [ ] .zdev/empty-arrays/background/empty-array-audit.md lists every posting function checked, how it was checked (driver, source trace, or both), and the result for empty input (posts nothing, fails, throws, or was fixed), plus the FlatZinc and instance-reader findings, and the brief indexes it.
- [ ] Each repaired posting family has focused coverage of its documented empty-input result. Where the original path aborts or gives a wrong result, that coverage fails before the fix in a Debug build. For zero-size Region or heap requests that already give the correct result (for example the Region allocation in gecode/float/linear.cpp), the audit records the before and after allocation sizes from a source trace instead; do not add allocator instrumentation only to make a test fail.
- [ ] changelog.in has a 7.0.0 entry that names the affected constraints.

## Validation

- Run the focused tests in build/full, then `ctest` and the check-standard suite in Debug.
- Last task in the area: run `uv run misc/check.py all --build-dir build/full --jobs N`. Fix failures that this area caused. Report failures that already exist at the area base.
