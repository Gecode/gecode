+++
schema_version = 1
id = "source-cleanup-001"
key = "void-args"
area = "source-cleanup"
status = "done"
complexity = "routine"
afk = true
priority = "low"
blocked_by = ["scoped-enums-004", "modern-classes-003", "modern-classes-004", "modern-classes-005"]
+++
# Remove (void) parameter lists by tool run

## Outcome

Maintained C++ code has no (void) parameter lists, generators emit none, and modernize-redundant-void-arg is enabled in .clang-tidy. The mass edit is one commit made by a deterministic command sequence, recorded in full in the commit message, that reproduces it from the parent commit.

## Context

Use the ownership distinctions in misc/check.py: owned sources, third-party headers, generated variable headers, and grammar-owned ranges. Run clang-tidy 22 with `-checks=-*,modernize-redundant-void-arg --fix` over the owned translation units with the lint header filter. Change generated code through its inputs (misc/genvarimp.py and the C++ actions in the FlatZinc Bison and Flex inputs) with a scripted, deterministic edit, leave Bison and Flex scaffolding untouched, then build the regenerate target and run `uv run misc/check.py format`. Cover maintained headers that no translation unit includes in the build, and record how. Verify the sequence by running it again on a fresh checkout of the parent commit under build/ and comparing the result with the commit.

## Boundaries

- Do not hand-merge the mass edit. If it conflicts with other work, rerun the tool.

## Done when

- [x] No `(void)` parameter list remains in maintained C++ files or generator output, checked with a recorded grep that excludes casts to void; casts to void are unchanged.
- [x] .clang-tidy enables modernize-redundant-void-arg, and docs/code-style.md states the rule and that forceinline stays lowercase so it reads like inline.
- [x] The commit message gives the complete command sequence, and rerunning it on the parent commit reproduces the commit exactly.

## Validation

- Build target regenerate, then run .github/ci/generated-source-freshness.sh.
- Run `uv run misc/check.py all --build-dir build/full --jobs N` and build the default Release configuration with ctest.

## Result

Maintained C++ has no (void) parameter lists, modernize-redundant-void-arg guards it, and the mass edit reproduces byte for byte from the parent with the recorded commands.

Validation:

- check.py all exit 0 with the new check over 489 sources; ctest 8/8 and 7/7; generated-source freshness passes; doxygen.log empty; gecode-test -list unchanged
- Independent normalized comparison: 6,239 lists removed, 1,760 casts unchanged; rerun on git archive of the parent matches all 1,332 source files; -tag check 546/546, FlatZinc 155/155
