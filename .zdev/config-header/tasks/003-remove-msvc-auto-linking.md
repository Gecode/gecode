+++
schema_version = 1
id = "config-header-003"
key = "remove-autolink"
area = "config-header"
status = "done"
afk = true
blocked_by = ["config-header-001", "config-header-002"]
+++
# Remove MSVC auto-linking

## Outcome

gecode/support/auto-link.hpp, its includes, and the GECODE_LIBRARY_NAME defines in the module headers are gone. GECODE_NO_AUTOLINK is no longer defined in the template or in the exported targets' compile definitions. The auto-link-only macros GECODE_DLL_USERPREFIX, GECODE_DLL_USERSUFFIX, and GECODE_LIBRARY_VERSION are removed.

## Context

The brief records why auto-linking is dead and broken (see Terms and settled decisions). GECODE_LIBRARY_NAME is defined before including auto-link.hpp in gecode/kernel.hh, support.hh, search.hh, int.hh, set.hh, float.hh, minimodel.hh, driver.hh, flatzinc.hh, gist.hh, gist/qtgist.hh, and instance.hh. CMakeLists.txt adds `PUBLIC GECODE_NO_AUTOLINK` to the library targets. The CMake options GECODE_LIB_PREFIX and GECODE_LIB_SUFFIX stay, because they set the output names. The installed headers come from `install(DIRECTORY gecode ...)` in CMakeLists.txt, not from cmake/GecodeSources.cmake. Top-level ctest does not run the installed-package fixtures (test/package/allocator and test/package/public-test-component). `uv run misc/check.py all` configures and runs them.

## Boundaries

- Do not change library output names or the GECODE_LIB_PREFIX and GECODE_LIB_SUFFIX options.

## Done when

- [x] `grep -rn 'auto-link\|GECODE_NO_AUTOLINK\|GECODE_LIBRARY_NAME\|GECODE_DLL_USER\|GECODE_LIBRARY_VERSION' gecode test examples tools cmake CMakeLists.txt` and the same search over the generated config.hpp find nothing.
- [x] A fresh installation contains no auto-link.hpp, and the exported targets carry no GECODE_NO_AUTOLINK compile definition.
- [x] changelog.in has a 7.0.0 entry that says MSVC auto-linking was removed, names the removed macros, and says consumers link through the Gecode:: CMake targets.

## Validation

- uv run misc/tidy.py --check
- List the removed MSVC-only code in the task result so the PR description can ask Windows CI to confirm.
- Last task in the area: run `uv run misc/check.py all --build-dir build/full --jobs N` (Debug with experimental, CBS, fault injection, audit, and Gist). This includes ctest and both installed-package fixtures. Fix failures that this area caused. Report failures that already exist at the area base without fixing them.

## Result

MSVC auto-linking removed: auto-link.hpp, the GECODE_LIBRARY_NAME defines in 12 module headers, GECODE_NO_AUTOLINK (template and exported targets), GECODE_DLL_USERPREFIX/SUFFIX and GECODE_LIBRARY_VERSION

Validation:

- Done-when grep over gecode test examples tools cmake CMakeLists.txt and the generated config.hpp finds nothing
- Fresh install has no auto-link.hpp; GecodeTargets*.cmake carry no GECODE_NO_AUTOLINK
- Release build and ctest 7/7; cmake-consumer-smoke (MPFR), gist-static-macro-smoke, tidy and format-check passed
- uv run misc/check.py all --build-dir build/full --jobs 12 (Debug, experimental, CBS, fault injection, audit, Gist): exit 0, ctest 8/8, both package fixtures passed
- Removed MSVC-only code for Windows CI: auto-link.hpp pragma comment(lib) selection (platform x86/ia64/x64, d/r), the 12 GECODE_LIBRARY_NAME blocks, the GECODE_NO_AUTOLINK check in cmake-consumer-smoke.ps1
- Independent Codex verifier: pass, no findings
