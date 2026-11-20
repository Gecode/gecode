+++
schema_version = 1
id = "boost-010"
key = "interval-removal"
area = "boost"
status = "done"
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

- [x] gecode/third-party and the differential test are deleted, and a grep of the tree outside .zdev for third-party/boost, gecode_boost, GECODE_BOOST, and LICENSE_1_0 finds nothing.
- [x] An install contains no Boost header and no LICENSE_1_0.txt.

## Validation

- Release and Debug builds with and without MPFR, ctest, an install, and bin/gecode-test -test Float::.
- uv run misc/check.py lint --build-dir build/full, format-check, and the doc target.

## Result

Deleted gecode/third-party (105 files) and the temporary differential test, and removed every reference from CMakeLists.txt (install pattern, doc-input filter), misc/check.py, .clang-tidy, doxygen.conf.in, docs/code-style.md and docs/breaking-7.md; only interval.hpp's decision comment and acknowledgement mention Boost. Float::ScaleView is asserted trivially copyable, so performance-unnecessary-value-param.AllowedTypes is gone and code-style.md says FloatVal is trivially copyable but still passed by const&.

Validation:

- Release/Debug x MPFR on/off builds without warnings; ctest 7/7; Float:: Release 1566/1566 and 1428/1428; install has no Boost header or LICENSE_1_0.txt; format-check and lint (LLVM 22) clean; doc 0 warnings. Fresh verifier (codex gpt-6.1-sol high) configured and built from scratch, ran the focused float tests (151/151) and a scratch install, and found nothing.
