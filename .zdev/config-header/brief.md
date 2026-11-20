# Native config header generation

## Objective

The build generates the installed `gecode/support/config.hpp` with CMake's
`configure_file` and `#cmakedefine`/`#cmakedefine01`. The hand-written loop in
`CMakeLists.txt` (the "Configure-time metadata and config.hpp generation"
section) is removed: it reads `gecode/support/config.hpp.in`, rewrites each
`#undef`, guesses quoting from the value, and appends extra Autoconf `#undef`
lines. Probes that the C++23 compiler baseline makes unnecessary are gone.

Success:

- `config.hpp.in` is a normal `configure_file` template. The value of each
  macro, including quoting, is written in the template, not chosen by value
  heuristics in CMake.
- `__builtin_ffsll`, `__builtin_popcountll`, and the MSVC
  `_BitScanForward`/`__popcnt` intrinsics in `gecode/support/bitset-base.hpp`
  give way to `<bit>` (`std::countr_zero`, `std::popcount`). The
  `GECODE_HAS_BUILTIN_*` probes and macros are removed.
- `forceinline` is defined from compiler detection (`_MSC_VER` →
  `__forceinline`, GCC/Clang → `inline __attribute__((always_inline))`)
  instead of `check_c_source_compiles`. It keeps its lowercase name, so that
  it reads like `inline`.
- The Autoconf leftovers `PACKAGE_BUGREPORT`, `PACKAGE_NAME`,
  `PACKAGE_STRING`, `PACKAGE_TARNAME`, `PACKAGE_URL`, and `PACKAGE_VERSION`
  are no longer written to the installed header. Nothing in the tree uses
  them, and they pollute every consumer's macro namespace.
- MSVC auto-linking is removed: `gecode/support/auto-link.hpp`, its includes
  and the `GECODE_LIBRARY_NAME` defines in the module headers,
  `GECODE_NO_AUTOLINK` in the template and in the exported targets' compile
  definitions, and the auto-link-only macros `GECODE_DLL_USERPREFIX`,
  `GECODE_DLL_USERSUFFIX`, and `GECODE_LIBRARY_VERSION`. The CMake options
  `GECODE_LIB_PREFIX` and `GECODE_LIB_SUFFIX` stay, because they name the
  built libraries.
- Every other macro keeps its name and its defined/undefined state in every
  configuration. Feature tests such as `#ifdef GECODE_HAS_FLOAT_VARS` must
  behave exactly as before.

## Boundaries and non-goals

- Owned: `CMakeLists.txt` (config generation and the probes it feeds),
  `gecode/support/config.hpp.in`, `gecode/support/bitset-base.hpp`, and other
  `support` files that consume removed macros.
- Do not rename `forceinline` or change its uses.
- Keep `GECODE_VERSION` and `GECODE_VERSION_NUMBER`. Downstream code tests
  them.
- Keep the POSIX spawn, mmap, visibility, mtrace, and peak-heap probes unless a
  probe is dead. They test the platform, not the compiler baseline.
- Do not change the mimalloc integration beyond what moving its
  `GECODE_MIMALLOC` line into the template requires.
- Version and ABI numbers stay at 6.5.0 / 52.

## Terms and settled decisions

- Macros that consumers only test with `#ifdef` may change from
  `#define X /**/` to `#define X 1`. This keeps `#ifdef` behaviour and is not a
  compatibility break. Macros whose value is used, such as `GECODE_MSIZE`,
  `GECODE_MEMORY_ALIGNMENT`, `GECODE_FREELIST_SIZE_MAX*`, and the version
  macros, keep their value and quoting.
- `HAVE_MMAP` is used only by the generated FlatZinc parser
  (`gecode/flatzinc/parser.tab.cpp`). Keep it if the parser still reads it.

- Auto-linking is dead and broken, and nothing replaces it. CMake defines
  `GECODE_NO_AUTOLINK` in both `config.hpp` and the exported targets, so the
  pragma never runs. It would also request `GecodeInt-6-5-0-r-x64.lib`, while
  CMake builds `gecodeint`. Consumers link explicitly through the
  `Gecode::` package targets, which is current CMake practice. Boost's CMake
  package likewise turns off its own auto-linking. The changelog records the
  removed macros.

## Testing

Existing checks only, plus a generated-header comparison that is not committed.
The risk is a silently
changed feature macro in some configuration, which a compile on one machine
would not reveal.

- Before changing anything, generate `config.hpp` for at least these
  configurations into separate `build/` directories and keep the outputs:
  default Release; `GECODE_BUILD_STATIC=ON`; float, set, and MPFR disabled;
  `GECODE_ENABLE_AUDIT=ON` with fault injection; `GECODE_ENABLE_MIMALLOC=ON`;
  `GECODE_ENABLE_EXPERIMENTAL=ON` with CBS. After the change, regenerate them
  and compare the set of macros defined and their values after preprocessing.
  For each configuration the only differences may be the removed macros and
  `/**/` → `1`. A throwaway script under `build/` is enough; do not commit it.
- Bit operations: the integrity suite exercises bitsets heavily through the
  int and set modules. Also inspect the Release assembly of one
  `Support::BitSetData` scan and population count to confirm they still
  compile to single instructions (`tzcnt`/`bsf`/`rbit+clz`, `popcnt`/`cnt`).

## Validation

- `cmake --build build/<dir>` and `ctest` (integrity suite and the installed
  package consumer fixtures) on the default configuration.
- `uv run misc/check.py all --build-dir <debug-full-dir> --jobs N` once, on a
  Debug build with experimental, CBS, fault injection, audit, and Gist, before
  the last commit.
- Windows/MSVC cannot be built locally. Review the `_MSC_VER` branches by hand
  and list them in the PR description for CI to confirm.
