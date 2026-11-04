+++
schema_version = 1
id = "boost-010"
key = "interval-removal"
area = "boost"
status = "open"
complexity = "standard"
afk = true
blocked_by = ["boost-009"]
+++
# Remove the vendored Boost copy

## Outcome

gecode/third-party is gone, nothing in the build, checks, documentation, or installed headers refers to it, and the temporary differential test is removed.

## Context

The spike's 'CMake, documentation, and checks' section lists every reference: the LICENSE_1_0.txt install pattern and the third-party doc-input filter in CMakeLists.txt, the third-party exclusions in misc/check.py, .clang-tidy's ExcludeHeaderFilterRegex, doxygen/doxygen.conf.in's exclude pattern, and the mentions in docs/code-style.md and docs/breaking-7.md. Recheck them with a grep for third-party, gecode_boost, GECODE_BOOST, and LICENSE_1_0 at the time of the task.

## Boundaries

- No behaviour change: the float suite and examples give the same results as after interval-switch.
- Initial task-owned path allocation: ["gecode/third-party/boost/LICENSE_1_0.txt","CMakeLists.txt","misc/check.py",".clang-tidy","doxygen/doxygen.conf.in","docs/code-style.md","docs/breaking-7.md"]. Coordination may extend it only after checking retained parent edits and sibling assignments.

## Done when

- [ ] gecode/third-party and the differential test are deleted, and a grep of the tree outside .zdev for third-party/boost, gecode_boost, GECODE_BOOST, and LICENSE_1_0 finds nothing.
- [ ] An install contains no Boost header and no LICENSE_1_0.txt.

## Validation

- Release and Debug builds with and without MPFR, ctest, an install, and bin/gecode-test -test Float::.
- uv run misc/check.py lint --build-dir build/full, format-check, and the doc target.
