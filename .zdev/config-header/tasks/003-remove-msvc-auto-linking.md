+++
schema_version = 1
id = "config-header-003"
key = "remove-autolink"
area = "config-header"
status = "open"
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

- [ ] `grep -rn 'auto-link\|GECODE_NO_AUTOLINK\|GECODE_LIBRARY_NAME\|GECODE_DLL_USER\|GECODE_LIBRARY_VERSION' gecode test examples tools cmake CMakeLists.txt` and the same search over the generated config.hpp find nothing.
- [ ] A fresh installation contains no auto-link.hpp, and the exported targets carry no GECODE_NO_AUTOLINK compile definition.
- [ ] changelog.in has a 7.0.0 entry that says MSVC auto-linking was removed, names the removed macros, and says consumers link through the Gecode:: CMake targets.

## Validation

- uv run misc/tidy.py --check
- List the removed MSVC-only code in the task result so the PR description can ask Windows CI to confirm.
- Last task in the area: run `uv run misc/check.py all --build-dir build/full --jobs N` (Debug with experimental, CBS, fault injection, audit, and Gist). This includes ctest and both installed-package fixtures. Fix failures that this area caused. Report failures that already exist at the area base without fixing them.
