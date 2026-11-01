+++
schema_version = 1
id = "config-header-001"
key = "configure-file"
area = "config-header"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = []
+++
# Generate config.hpp with configure_file

## Outcome

CMake generates the installed gecode/support/config.hpp from gecode/support/config.hpp.in with configure_file and #cmakedefine/#cmakedefine01. The hand-written rewriting loop, the appended Autoconf #undef block, and the PACKAGE_* macros are gone. forceinline is defined by compiler detection in the template, with the same replacement tokens as today.

## Context

The current generation is the "Configure-time metadata and config.hpp generation" section of CMakeLists.txt. It reads the template with file(READ), appends extra #undef lines (HAVE_MMAP, GECODE_NO_AUTOLINK, PACKAGE_*), and rewrites each #undef by guessing quoting from the value (`/**/`, quoted string, or 0/1). The forceinline value comes from two check_c_source_compiles probes (HAVE_ALWAYS_INLINE, HAVE_FORCE_INLINE). Read the brief's Objective, Terms and settled decisions, and Testing first. The brief says which macros keep their values and quoting and which may change from `/**/` to 1. GECODE_MIMALLOC is currently appended after the loop. Use `#cmakedefine GECODE_MIMALLOC 1` and leave it undefined when disabled, because gecode/support/allocator.hpp tests it with #ifdef; #cmakedefine01 would define it as 0 and still select mimalloc. Keep the defined/undefined state of every macro that is tested with #ifdef.

## Boundaries

- Do not change the bit-operation probes (GECODE_HAS_BUILTIN_*) or MSVC auto-linking in this task. Later tasks in this area remove them, so keep them as ordinary #cmakedefine lines here.
- Keep the name forceinline and every use of it.

## Done when

- [x] CMakeLists.txt generates config.hpp with a single configure_file call, and no CMake code rewrites template lines.
- [x] The template defines forceinline as `__forceinline` under _MSC_VER and as `inline __attribute__ ((__always_inline__))` otherwise, the same tokens the probes produce today, and the HAVE_ALWAYS_INLINE and HAVE_FORCE_INLINE probes are removed.
- [x] PACKAGE_BUGREPORT, PACKAGE_NAME, PACKAGE_STRING, PACKAGE_TARNAME, PACKAGE_URL, and PACKAGE_VERSION no longer appear in the generated header.
- [x] For each configuration listed in the brief's Testing section, the macros defined in the generated header and their values match the baseline taken before the change. The only differences are the removed PACKAGE_* macros and `/**/` becoming 1. forceinline and GECODE_MIMALLOC are unchanged.
- [x] changelog.in has an entry in the 7.0.0 section that lists the removed PACKAGE_* macros.

## Validation

- Compare preprocessor macro dumps (for example `c++ -dM -E` of a file that includes the generated header) for each configuration, before and after, using a throwaway script under build/.
- Build the default configuration and run ctest. The installed-package fixtures run through the area's terminal `misc/check.py all` validation in remove-autolink.
- uv run misc/tidy.py --check

## Result

config.hpp is generated from config.hpp.in with configure_file and #cmakedefine; PACKAGE_* macros removed; freelist size macros are now numeric

Validation:

- Six configurations (default, mimalloc, threads off, no set/float, fault injection, audit) give macro dumps identical to baseline except PACKAGE_* removal and empty-to-1
- Default Release build with 0 warnings; ctest 7/7 passed
- uv run misc/tidy.py --check and format-check passed
- Independent Codex verifier: pass, no findings
