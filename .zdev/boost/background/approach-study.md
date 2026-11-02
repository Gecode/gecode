# Refreshed copy, improved copy, or support library

## Result

This answers boost-003. It compares three ways to handle the vendored
Boost.Interval copy in `gecode/third-party/boost`:

1. **Refresh**: regenerate the copy from Boost 1.92.0 with a script.
2. **Improve**: keep Boost 1.47.0, the 2012 base, but regenerate it with
   the same script and drop the dead files.
3. **Support library**: replace the copy with Gecode code.

| measure | HEAD | refresh (1.92.0) | improve (1.47.0) | command |
|---|---|---|---|---|
| files in the copy | 105 | 53 | 48 | [C3], [C14], [C15] |
| headers the 11 emulated targets reach, files / lines | 47 / 6543 | 52 / 9155 | 47 / 6543 | [C8], [C9] |
| patches on top of upstream: patches / hunks / lines | 10 / 20 / 63 | 2 / 2 / 4 | 8 / 17 / 43 | [C13] |
| script reproduces HEAD byte for byte | yes | - | - | [C12] |
| emulated targets with `#error`, of 11 | 1 | 0 | 1 | [C8] |
| compiler warnings, all builds | 0 | 0 | 0 | [C16] |
| rounding probe configurations with 0 unsound rows and HEAD's bounds, of 9 | 9 | 9 | 9 | [C19] |
| `nth_root` rows that leave the caller's mode changed, of 240 | 0 | 180 (0 with fix) | 0 | [C18] |
| `nth_root` rows whose bounds differ from HEAD, of 240 | - | 15 (0 with fix) | 0 | [C18] |
| comparison truth table as HEAD | - | yes, with patch 03 | yes | [C17] |
| examples whose search equals `build/local`, of 4 | 4 | 2 (4 with fix) | 4 | [C19] |
| Float:: suite, failed of 1561 | 0 | 0 (0 with fix) | 0 | [C19] |

The fix in the refresh column is upstream commit 6a59a02 (October 2026, after
1.92.0). Without it, 1.92.0's `nth_root` runs without a rounding scope.

For approach 3, the measured Boost code behind Gecode's float API is 1316
source lines without MPFR, plus 215 with MPFR [C22]. Without template
plumbing and empty-operand checks it is 974 lines, plus 139 with MPFR.
Rounding control adds 290 Boost lines on top [C22]. The P2 nextafter
alternative replaces that with about 105 added lines [C23].

**Recommendation (for the user to decide in boost-004): approach 3, the
support library, with the current switched-mode rounding.** Reasons:

- Gecode 7 is the breaking release in which the foreign types can leave the
  public API.
- Gecode already owns the arithmetic since boost-006.
- The rest is about 1000 lines of known algorithms [C22], and the suite,
  the probes here, and the examples are ready oracles.
- Refreshing brings nothing Gecode uses and needs a patch that is not in any
  release yet.

Second choice: approach 2. The section Recommendation gives the reasons and
what would change the choice.

## Question and scope

The brief's open questions for this task:

- What does current Boost.Interval add over the 2012 copy, and how large is
  its dependency closure?
- How large is approach 3?

The task also asks:

- whether a script can reproduce the copy exactly;
- what renaming and patching that takes;
- the cost and risk of each approach;
- one recommendation.

Covered:

- Boost 1.92.0, the latest release, against the 2012 copy.
- Closures of `boost/numeric/interval.hpp`, both textual and per target.
- A scratch vendoring script and its byte-exact check against HEAD.
- Builds of the refreshed and improved copies in scratch source copies.
- The rounding probe, the DirectedRounding test, the Float:: suite, and the
  four float examples.
- A comparison truth table and an `nth_root` probe.
- A symbol inventory of what Gecode instantiates from the copy.
- Source line counts for the support-library estimate.

Not covered:

- Running anything on x86-64 or Windows. This machine is arm64 macOS.
- MSVC at all. Its rounding code is listed from source only.
- Real Linux, MinGW, and Windows system headers. The per-target closures use
  the preprocessor without a sysroot, with the standard library chosen by a
  predefined macro (see Setup).
- Runtime. Other processes loaded the machine heavily during the builds
  and the suite. The coordinator asked for no suite timing.

## Setup

- Revision 0786b2312ff298a098ac95258a5f9c51d510289a on branch
  feature/breaking-7, clean tree [C1]. `.zdev/` is listed in
  `.git/info/exclude`, so the study shows only with `--ignored`.
- Apple M1 Max, 10 cores, macOS 27.0.1 [C1].
- Compilers [C1]:
  - for builds, `/Library/Developer/CommandLineTools/usr/bin/c++`, Apple
    clang 21.0.0 (clang-2100.3.34.2);
  - for probes and closures, `/usr/bin/clang++`, the same compiler;
  - the rounding probe adds Homebrew LLVM 22 and GCC 14, as in
    rounding-probe.md.
- CMake 3.27.6, Python 3.15.1, MPFR 4.2.2 [C1].
- Upstream pins [C2]:
  - boostorg/boost tag boost-1.92.0 is the latest release tag. Its
    superproject commit is afdfa32505af73e3d208144b3f623f0096cb62b6, dated
    2026-08-05.
  - boostorg/interval: 2939307e2797fda9df945ab25ab5d6b258d776d4, the same
    commit as boost-1.91.0. Author date 2026-01-22, "Remove dependencies on
    Boost.StaticAssert".
  - boostorg/config: 115e718e0fd72329e69fc776dac99811385d6f77.
  - boostorg/detail: 965826dc374165d71530d9814ecd5f4628365522.
  - boostorg/logic: 9b8703a2d6623405323b892f1d126a6b17ca1651. It was
    fetched to check the closure, which does not reach it.
  - Each module commit equals the superproject's submodule entry.
  - interval master, 3a640dfe901f, is 29 commits ahead of the release.
- The 2012 base: `git archive 561102a86c` ("Added Boost as third party
  software", 2012-03-27). It is identical to the planner's snapshot
  [C3].
- Downloads are in `build/zdev-impl/boost-003/dl/`, and the scripts only read
  them. Python ran with `python3 -I`.
- Build options: `scripts/build.sh` uses build/local's compiler and
  `GECODE_ENABLE_GIST=OFF`. It sets `CMAKE_BUILD_TYPE` and
  `GECODE_ENABLE_MPFR`, with Unix Makefiles for Release and Ninja for
  Debug. `build/local` is a Release build with MPFR [C16].
- Per-target emulation (`scripts/target_closure.py`). All targets run
  `clang++ -std=c++23 -M` on a one-line file that includes the root header.
  - macOS arm64 and x86_64 use the real SDK.
  - The other targets use `-nostdinc -MG`, so missing system headers pass.
    Each picks its standard library with a predefined macro (`__GLIBCXX__`,
    `_LIBCPP_VERSION`, or `_CPPLIB_VER`).
  - GCC and MinGW targets also undefine `__clang__`.
  - MSVC targets use `-fms-compatibility -fmsc-version=1940` and undefine
    `__clang__`, `__x86_64__`, and `__aarch64__`. Two of them also set
    `__cplusplus` to 199711L, as cl.exe reports it without
    `/Zc:__cplusplus`.

All paths below are relative to `build/zdev-impl/boost-003`, which git
ignores.

## Inventory at HEAD

### Source references

Outside the copy, `gecode_boost`, `GECODE_BOOST`, or `third-party/boost`
appear on 38 lines in four files [C20]:

- `gecode/float.hh`, 6 lines:
  - the include;
  - `RoundingControl`: `rounded_arith_std` on MSVC with SSE, otherwise
    `rounded_arith_opp`;
  - `Rounding` as `save_state<RoundingBase>`;
  - `checking_strict`;
  - `interval<FloatNum, policies<R, P>>` as `FloatValImpType`.
- `gecode/float/num.hpp`, 6 lines: the `pi_half`, `pi`, and `pi_twice`
  lower and upper constants.
- `gecode/float/val.hpp`, 20 lines:
  - `width`, `singleton`, `in`, `zero_in`;
  - the `pi_half`, `pi`, and `pi_twice` intervals;
  - 11 `catch` clauses for `comparison_error`;
  - `checking_strict::is_empty`.
- `test/float/arithmetic.cpp`, 6 lines: the DirectedRounding test's
  `RoundingScope` uses `interval`, `policies`, `checking_strict`, `fmod`,
  `sin`, and `save_state<Float::Rounding>` directly.

`val.hpp` also calls these unqualified on `FloatValImpType`, found by
argument-dependent lookup:

- arithmetic: unary `+` (unary `-` is Gecode's own code), the four
  operators and their compound forms, `abs`, `sqrt`, `square`, `pow`,
  `nth_root`, `max`, `min`;
- set operations: `subset`, `proper_subset`, `overlap`, `intersect`,
  `hull`;
- under `GECODE_HAS_MPFR`: `exp`, `log`, `fmod`, `sin`, `cos`, `tan`,
  `asin`, `acos`, `atan`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`.

### Symbols

Debug builds at -O0 (`CMAKE_CXX_FLAGS_DEBUG=-g`) were inspected with
`nm -C`. The binaries are all `libgecode*.6.5.0.dylib`, `gecode-test`, and
the four float examples [C21]. They define:

- with MPFR: 99 distinct `gecode_boost` functions;
- without MPFR: 82. The difference is the transcendental functions and the
  pi constants. `archimedean-spiral` and `golden-spiral` are not built
  without MPFR (`GECODE_MPFR_FLOAT_EXAMPLES`).

Every API function above appears except `proper_subset`, `sinh`, `cosh`,
`tanh`, `asinh`, `acosh`, and `atanh`, which nothing instantiates [C21].
They are still public `FloatVal` functions, so a replacement must provide
them. Beyond the API, the symbols show what the copy uses internally:

- `checking_base`, `checking_no_empty`, and `checking_no_nan`:
  `checking_strict` is built from them;
- `exception_create_empty`;
- `test_input`;
- `user::is_neg`, `is_zero`, and `is_pos`;
- `div_positive`, `div_negative`, `div_zero`, `div_non_zero`, and
  `multiplicative_inverse`;
- `pow_dn` and `pow_up`; `root_aux`, `root_aux_dn`, and `root_aux_up`;
- the C99 rounding control: `upward`, `downward`, `to_nearest`, and
  `get_rounding_mode` and `set_rounding_mode`;
- `save_state` and `save_state_unprotected`;
- `rounded_arith_opp::init` and `median`;
- `rounding_control::force_rounding`;
- the pi constants;
- interval members such as `set_empty`, `whole`, and the holder types of
  the comparisons.

The full lists are `results/inv/head-dbg.txt` and
`results/inv/head-dbg-nompfr.txt`.

### Corrections to the brief's Current state

- `user::is_neg`, `is_zero`, and `is_pos` are used only inside the copy.
  Gecode code outside the copy has no reference to them [C20], [C21].
- `fmod` and `sin` reach Gecode code in two ways: through `val.hpp`'s MPFR
  block and directly from the DirectedRounding test. The other
  transcendental functions are in the same MPFR block.
- The list misses the calls found by argument-dependent lookup (see Source
  references), `median`, the pi intervals, and the compound assignments.
- Since boost-006, `RoundingControl` supplies only mode control (`init`,
  `upward`, `downward`, and the mode get and set used by `save_state`) and
  `median`. Gecode's `gecode/float/rounding.hpp` owns the 10 arithmetic
  operations, and `rounding.cpp` the MPFR functions.

## (1) Upstream against the 2012 copy

### The 2012 base is Boost 1.47.0

The 2012 import holds 99 files [C3]. Besides `extract.perl`, `patch.txt`,
and `LICENSE_1_0.txt`, it has 96 Boost headers. Compared with CR stripped,
it matches Boost 1.47.0 best: 94 headers are the same, 1 differs, and 1 is
not upstream [C4]. Other tags differ more:

| tag | same | differ | not upstream |
|---|---|---|---|
| 1.46.0 | 49 | 45 | 2 |
| 1.46.1 | 61 | 34 | 1 |
| 1.47.0 | 94 | 1 | 1 |
| 1.48.0 | 74 | 21 | 1 |

These counts are from [C4].

- The differing file is `detail/msvc_rounding_control.hpp`. It carries
  `WIN64` changed to `_WIN64` (upstream 2d5a340, first released in Boost
  1.56.0 [C10]) and the
  `patch.txt` change, a `_control87` call for x64 [C5].
- The file not upstream is `msvc_rounding_control.hpp.orig`, upstream 1.47.0
  with only `_WIN64` [C5].
- `LICENSE_1_0.txt` equals Boost's [C5].
- The import used CRLF line endings, for example 32 CR in the 32 lines of
  `numeric/interval.hpp` [C5].

### Upstream changes from 1.47.0 to 1.92.0

1.47.0 is not an ancestor of 1.92.0 in boostorg/interval. 35 commits
reach 1.92.0's `include/` that are not in 1.47.0, and 17 of them are dated
on or after 2011-07-11, the day 1.47.0 was tagged [C10].

Changed lines in interval, ignoring line endings, encoding, and trailing
whitespace, total 281 [C10]:

| file | lines | change |
|---|---|---|
| `utility_fwd.hpp` | 173 | new: forward declarations (802aef1) |
| `detail/bugs.hpp` | 31 | old-compiler workarounds removed |
| `detail/msvc_rounding_control.hpp` | 17 | `_WIN64` (2d5a340), x64 `_control87` (362724b), MSVC ARM64 (b4b190a) |
| `limits.hpp` | 16 | `BOOST_NOEXCEPT_OR_NOTHROW` (f90d1fa) and cleanup |
| `compare/set.hpp` | 12 | unused parameters unnamed (94e41db) |
| `hw_rounding.hpp` | 9 | MSVC ARM64; `BOOST_NO_FENV_H` (1b95966) |
| `rounded_arith.hpp` | 8 | `this->to_int` (538299d) |
| `utility.hpp` | 4 | unused typedef in `norm` (000f50f) |
| `detail/x86gcc_rounding_control.hpp` | 4 | |
| `detail/c99sub_rounding_control.hpp` | 2 | `<fenv.h>` replaced by `boost/detail/fenv.hpp` |
| `detail/bcc_rounding_control.hpp` | 2 | `BOOST_BORLANDC` (ed01cda) |
| `detail/x86_rounding_control.hpp` | 2 | |
| `arith2.hpp` | 1 | `typename Policies::rounding rnd;` removed from `nth_root` (eee9745, 2022) |

These counts are from [C10].

- `interval.hpp`, `arith.hpp`, `division.hpp`, `transc.hpp`,
  `checking.hpp`, `constants.hpp`, and `rounding.hpp` are unchanged apart
  from line endings and whitespace [C10].
- Boost.Config changed by 10903 lines over the same files [C10].

What Gecode gets from this:

- No change to arithmetic, division, the transcendental functions,
  comparisons, or checking.
- Five of Gecode's changes are now upstream [C14]: `_WIN64`, `_control87`,
  `this->to_int`, the `norm` typedef, and the hardware selection that
  `BOOST_NO_FENV_H` replaces. See part (2).
- One regression. Removing the "unused" `rnd` in eee9745 leaves
  `nth_root` without a rounding scope. The `nth_root` probe [C18] shows it:
  - Called in nearest, downward, or toward-zero mode, 1.92.0's `nth_root`
    returns with the mode set to upward in 180 of 240 rows.
  - 15 rows get different bounds from HEAD's, all of them still sound.
  - Upstream fixed this in 6a59a02, "Fix nth_root not setting and restoring
    the rounding mode", on 2026-10-14. No release has the fix yet [C11].

After 1.92.0, 18 of interval master's 29 commits [C2] change `include/`:
14 files, 213 lines in and 52 out [C11]. They include rounding fixes for
optimized builds:

- g++ merging square roots (8f074a4);
- x87 and `fldcw` ordering (2bb1db2, 738ca11);
- `reload_operand` in the std arithmetic (b2a1019);
- `int_down` and `int_up` across mode changes (adc3850).

Also: constrained converting constructors (39f1d56), `comparison_error`
in its own header (f88338e, 321cc2b), and PowerPC fixes. Upstream is
active again and works on the problem boost-006 solved for Gecode.

### Closure

`scripts/vendor.py closure` is extract.py's walk with these fixes:

- macro includes resolve for any macro name;
- quoted includes resolve relative to the including file;
- `#include_next` and spacing after `#` are handled;
- comments are ignored;
- a missing file is an error.

It follows every preprocessor branch, so it is an upper bound. Results:

- On 1.47.0: 95 files, no unresolved include, 29 system headers. That is
  the 2012 import minus `extract.perl`, `patch.txt`, `LICENSE_1_0.txt`, and
  the `.orig` file [C6].
- On 1.92.0: 107 files, no unresolved include, 43 system headers [C6].
- 1.92.0 reaches `boost/detail/fenv.hpp` (Boost.Detail), but no logic
  header [C6].

The tracked `extract.py` [C7]:

- on 1.47.0, stops with a `UnicodeDecodeError` on a Latin-1 header;
- on 1.92.0, emits 103 copies. It misses `config/compiler/diab.hpp` and
  `config/platform/cloudabi.hpp`, `haiku.hpp`, and `wasm.hpp`. Upstream
  lists those only in an `#if 0` block written for dependency scanners.

The preprocessor gives each target's closure [C8]:

| copy | files per target | union | targets with `#error` |
|---|---|---|---|
| upstream 1.47.0 | 36 to 39 | 47 | 8 of 11 |
| HEAD | 36 to 39 | 47 | 1 of 11 |
| 1.92.0, and the refreshed copy | 41 to 43 | 52 | 0 of 11 |
| improved copy | 36 to 39 | 47 | 1 of 11 |

- Unpatched 1.47.0 stops with "Please specify rounding control mechanism"
  on every target except macOS x86_64 and the two MSVC x64 variants. This is
  why Gecode patched `hw_rounding.hpp` (patch 09).
- HEAD and the improved copy hit the same `#error` on MSVC arm64 when
  `__cplusplus` is 199711L, which cl.exe reports without
  `/Zc:__cplusplus`. Nothing in the CMake files sets that flag. This is
  emulation only, and MSVC arm64 is not a CI target [C8].
- 1.92.0's union adds the `config/detail/` selector headers,
  `clang_version.hpp`, `helper_macros.hpp`, `pragma_message.hpp`,
  `detail/fenv.hpp`, and `utility_fwd.hpp`. It no longer needs HEAD's
  top-level `config/` selectors or `no_tr1/utility.hpp` [C8].

Sizes [C9]:

| header set | files | lines |
|---|---|---|
| HEAD, tracked | 105 | 11308 |
| HEAD, installed | 96 | 10978 |
| 1.47.0, textual closure | 95 | 10926 |
| 1.92.0, textual closure | 107 | 15821 |
| HEAD, per-target union | 47 | 6543 |
| 1.47.0, per-target union | 47 | 6522 |
| 1.92.0, per-target union | 52 | 9155 |

Notes on these sizes:

- The install rule, `CMakeLists.txt:1639-1654`, installs `*.hh`, `*.hpp`,
  and `LICENSE_1_0.txt` and excludes `abi*`. So the six `config/abi*`
  headers (f813edf841), `extract.py`, `patch.txt`, and the `.orig` file
  ship in the source tree only [C9].
- No closure reaches them [C6].

Files the per-target union drops, for both copies [C14], [C15]:

- compiler configurations for Borland, CodeGear, Comeau, Compaq,
  Digital Mars, GCC-XML, Green Hills, HP aCC, Intel, KAI, Metrowerks, MPW,
  NVCC, PathScale, PGI, SGI MIPSpro, Sun, and VisualAge (1.92.0 also Cray,
  Diab, XL C++);
- platform configurations for AIX, AmigaOS, BeOS, BSD, Cygwin, HP-UX,
  IRIX, QNX, Solaris, Symbian, VMS, and VxWorks (1.92.0 also CloudABI,
  Cray, Haiku, WASM, z/OS);
- old standard libraries;
- the Alpha, Borland, IA-64, PowerPC, SPARC, and x86 GCC-asm rounding
  controls;
- for 1.47.0 also `detail/endian.hpp`, `detail/limits.hpp`,
  `non_type.hpp`, and `type.hpp`.

Dropping platforms has a cost. With the same emulation, FreeBSD reaches
`config/platform/bsd.hpp` and Cygwin reaches `config/platform/cygwin.hpp`
[C25]. HEAD has both. Both pruned copies lack them, so a FreeBSD or Cygwin
build would stop at a missing include. `CMakeLists.txt:209` mentions
`CYGWIN`.

## (2) Renaming, patching, and exact reproduction

### Gecode's edits

24 commits touched the copy [C24]. Each edit falls in one of three groups;
the file lists are from [C24].

Mechanical transforms:

- CRLF to LF (c71469e4ab);
- include paths (4ff2dfc197) and renaming (72bf3fd0f7);
- trailing whitespace (448fcee00f, in 11 headers; 87fa9d96fc in
  `rounded_arith.hpp`);
- Latin-1 to UTF-8 (3641a30d25).

Four commits touched only Gecode's own files: `extract.perl`, `extract.py`,
or the `.orig` file (8207f7fd29, 9a282b12ed, 2a9c43036e, 19bff4fb9b).

Semantic patches, kept as files `a/patches/*.patch` [C13]:

| patch | from | hunks | lines |
|---|---|---|---|
| 01 MSVC `_WIN64` and x64 `_control87` | 2012 import | 2 | 6 |
| 03 `interval::operator==` on equal bounds | 3e448c0d5c | 1 | 3 |
| 04 `this->to_int` | 3e448c0d5c, 87fa9d96fc | 2 | 8 |
| 05 MSVC version limit | 6e0efb1dec, 13232ee695, 4d14667bf7, 90bb5ddd80 | 1 | 3 |
| 06 libstdc++ thread detection | d78bb0b1d1 | 1 | 4 |
| 07 unused parameters in `compare/set.hpp` | 3cb43dc1d6 | 6 | 6 |
| 08 `abi*` headers keep `boost/` include paths | f813edf841 | 2 | 4 |
| 09 hardware selection without glibc, PowerPC | c36470ad8e, cde7a6c1cf | 3 | 12 |
| 10 unused typedef in `norm` | 14857e39d9 | 1 | 1 |
| 11 PowerPC `long long` | f753986df1, be7f5d7d1c | 1 | 16 |

There is no patch 02. Patch 03 changes the comparison semantics, and
patches 01 and 09 decide how the rounding mode is controlled. The others
adjust configuration or silence warnings.

Files that are not upstream: `extract.py`, `patch.txt`, the `.orig` file,
and the six `abi*` headers (added by f813edf841).

`patch.txt` records 1 of the 10 patches [C13].

### Test A: reproducing HEAD

`scripts/test-a.sh` runs `vendor.py build` [C12]:

1. Copy the 101-file manifest: the 1.47.0 closure plus the six `abi*`
   headers.
2. Convert CRLF to LF and Latin-1 to UTF-8.
3. Rename with four rules, in order:
   - `([<"])boost/` to `\1gecode/third-party/boost/`;
   - `BOOST_` to `GECODE_BOOST_`, when not preceded by an identifier
     character;
   - `boost` followed by `::` to `gecode_boost`;
   - `namespace boost` to `namespace gecode_boost`.
4. Strip trailing whitespace in the 12 files listed in `a/strip-ws.txt`.
5. Apply the 10 patches.
6. Add `LICENSE_1_0.txt`.
7. With `--check`, compare the result with `gecode/third-party/boost`.

Result: `check ok`, exit 0 [C12]. The script reproduces HEAD byte for byte
with 10 patches of 20 hunks and 63 changed lines [C13]. Three files are
excluded:

- `extract.py` and `patch.txt` are Gecode's tool and note, not upstream;
- the `.orig` file is a stray copy of the unpatched file that nothing
  includes [C5], [C6].

The renaming is not fully consistent. The `abi*` headers keep `boost/`
include paths, which patch 08 records. That is harmless because nothing
includes or installs them.

### Test B: Boost 1.92.0

`vendor.py build` on 1.92.0 with the 55 files outside the per-target union
dropped gives 52 headers plus `LICENSE_1_0.txt` [C14]. The 2012 patches
apply as follows [C14]:

| patch | on 1.92.0 |
|---|---|
| 01 | already upstream |
| 03 | applies; still needed |
| 04 | already upstream |
| 05 | fails; obsolete, upstream knows current MSVC versions |
| 06 | fails; obsolete, upstream has the same condition |
| 07 | applies, but obsolete: upstream unnamed the parameters |
| 08 | no file; the `abi*` headers are not copied |
| 09 | 2 of 3 hunks fail; obsolete, upstream uses `BOOST_NO_FENV_H` |
| 10 | already upstream |
| 11 | no file; PowerPC is dropped |

- The refreshed copy needs patch 03, and the `nth_root` fix as patch 12,
  taken from 6a59a02: 2 patches, 2 hunks, 4 lines [C13].
- After renaming, no `boost::`, `namespace boost`, `<boost/`, or bare
  `BOOST_` is left [C14].

### Test B: behaviour

Scratch source copies of HEAD had the copy replaced: `src/refresh` with
1.92.0 and patch 03, `src/refreshfix` with patch 12 added, and `src/improve`
with the improved copy [C16].

- Release with MPFR, Release without MPFR, and the Debug builds of HEAD
  compiled with 0 warnings [C16]. Nothing outside the copy needed a change.
- `bld/refresh-rel`'s dependency files name the scratch copy's
  `detail/fenv.hpp`. The source tree comes before
  `/opt/homebrew/include` on the include path [C16].

Behaviour [C17], [C18], [C19]:

- **Rounding probe.** The rounding-probe.md matrix (AppleClang, LLVM 22,
  GCC 14 at -O0, -O2, -O3) against each install gives 0 unsound rows. The
  9 result files are byte-identical to HEAD's for refresh, improve, and
  refreshfix [C19].
- **DirectedRounding.** Passes for every variant [C19].
- **Comparisons.** The truth table equals HEAD's for 1.92.0 with patch 03,
  for refreshfix, and for 1.47.0. Without patch 03, two rows change:
  `[1,2]` against `[1,2]`, and `[1,1+ulp]` against itself. The raw interval
  `==` then throws, and `FloatVal ==` returns false instead of true [C17].
- **`nth_root`.** 0 mode changes and 0 differing rows for refreshfix and
  1.47.0. For refresh: 180 mode changes and 15 differing rows, all sound
  [C18].
- **Examples.** Solution counts are the same everywhere, 189, 572, 390,
  and 98 [C19]. Compared with `build/local`, ignoring the runtime line:
  - HEAD and improve print the same solutions and statistics in all four.
  - Refresh differs in cartesian-heart (28118 nodes against 28112;
    3558952 propagations against 3556954) and descartes-folium (19041
    nodes against 19070; 3332 failures against 3343). Both examples use
    `nroot`, directly or through `pow`'s propagator. Their printed
    solutions differ in 914 and 594 lines [C19].
  - Refreshfix prints the same solutions and statistics as `build/local`
    in all four. Refresh and refreshfix differ in one line, the restored
    `rnd` in `nth_root` [C14], so the search changes in refresh come from
    the `nth_root` regression.
- **Float:: suite**, 8 threads. Head, refresh, refreshfix, and improve
  each pass all 1561 tests [C19]. The suite does not catch the `nth_root`
  regression; the `nth_root` probe [C18] and the examples [C19] do.

## (3) Support library

### Boost code behind the API

`scripts/sloc.py` counts source lines without comments or blank lines, per
definition. It also counts two subsets [C22]:

- plumbing: template heads, typedefs, `using`, and the
  `typename Policies::rounding rnd;` line;
- checks: lines that test for empty or NaN operands.

`scripts/groups.py` sums the definitions of every function the inventory
reaches, grouped as a support library would need them [C22]:

| group | definitions | lines | plumbing | checks | rest |
|---|---|---|---|---|---|
| class: constructors, assignment, bounds, empty, whole | 23 | 241 | 41 | 22 | 178 |
| comparisons and `comparison_error` | 13 | 116 | 12 | 12 | 92 |
| checking and `test_input` | 16 | 135 | 21 | 15 | 99 |
| `+ - * /`, compound forms, division, inverse | 33 | 420 | 72 | 33 | 315 |
| `sqrt`, `square` | 2 | 26 | 6 | 2 | 18 |
| `pow`, `nth_root` and helpers | 7 | 128 | 21 | 2 | 105 |
| utility: `width`, `median`, set operations, `abs`, `max`, `min` | 22 | 201 | 32 | 21 | 148 |
| constants | 31 | 49 | 30 | 0 | 19 |
| **interval code, no MPFR** | | **1316** | | | **974** |
| MPFR: `fmod` | 3 | 33 | 9 | 3 | 21 |
| MPFR: transcendental functions | 14 | 182 | 47 | 17 | 118 |
| rounding control, all supported targets | 34 | 290 | 41 | 0 | 249 |

The interval-code totals are the sum of the eight rows above them. The
measured total of all definitions is 1821 lines, or 1362 without plumbing
and checks [C22].

The estimate for a double-only Gecode implementation is between the rest
column and the lines column:

- interval code: 974 to 1316 lines [C22];
- with MPFR: another 139 to 215 [C22].

The rest column assumes that every check can go. `FloatVal` never holds an
empty interval: its constructor throws on `l > u` [C17]. But the NaN checks
behind `test_input` still matter for operands that are NaN.

Gecode code already in place [C22]:

| file | lines |
|---|---|
| `gecode/float/rounding.hpp`, 10 operations, `barrier`, and `median` | 134 |
| `rounding.cpp`, the MPFR functions | 39 |
| `val.hpp` | 457 |
| `num.hpp` | 26 |

Of `val.hpp`, the 18 comparison operators take 123 lines [C22].

### Rounding layer

Two choices.

**Switched modes**, as today. Gecode would own:

- `init`, `upward`, `downward`, and `to_nearest`;
- mode get and set over `<cfenv>`;
- the MSVC `_control87` path;
- a scope that saves and restores the caller's mode, replacing
  `save_state`.

Boost's code for this on the supported targets is the 290-line rounding
group: `rounding.hpp` 50 lines, `rounded_arith.hpp` 87, `hw_rounding.hpp`
12, the C99 controls 33, and the MSVC and x86 controls 108 [C22]. The
arithmetic part of `rounded_arith.hpp` is already replaced by Gecode's.

**P2, nextafter with error terms** (nextafter-study.md):

- About 105 added lines replace the arithmetic and the mode switch
  [C23].
- The mode scope then protects only the mode-dependent code that
  nextafter-study.md lists: `tanatan.hpp:109-110`, `sincos.hpp:44-45`, and
  the test harness cuts.
- P2 changes bounds in edge cases near zero, underflow, and overflow, which
  the brief requires the user to accept.

### Comparison semantics

The truth table from `probe/cmp.cpp` [C17]. Rows have `x = [1,2]` unless
named. Columns are `< <= > >= == !=`. "raw" is Boost's interval operator,
where X means it throws `comparison_error`. "FloatVal" is Gecode's result.

| operands | raw | FloatVal |
|---|---|---|
| y = [3,4], disjoint above | TTFFFT | TTFFFT |
| y = [2,3], touching above | XTFXXX | FTFFFF |
| y = [1.5,3], overlapping | XXXXXX | FFFFFF |
| y = [1,2], equal bounds | XXXXTX | FFFFTF |
| y = [1.2,1.8], inside | XXXXXX | FFFFFF |
| y = [0,1], touching below | FXXTXX | FFFTFF |
| y = [-1,0], disjoint below | FFTTFT | FFTTFT |
| x = y = [1,1] | FTFTTF | FTFTTF |
| x = [1,1], y = [2,2] | TTFFFT | TTFFFT |
| x = [1,1], y = [1,2] | XTFXXX | FTFFFF |
| x = y = [1,1+ulp] | XXXXTX | FFFFTF |
| y = 0.5 | FFTTFT | FFTTFT |
| y = 1, lower bound | FXXTXX | FFFTFF |
| y = 1.5, inside | XXXXXX | FFFFFF |
| y = 2, upper bound | XTFXXX | FTFFFF |
| y = 3 | TTFFFT | TTFFFT |
| x = [1,1], y = 1 | FTFTTF | FTFTTF |
| x = [1,1+ulp], y = 1 | FXXTXX | FFFTTF |
| x = [1,1+ulp], y = 1+ulp | XTFXXX | FTFFTF |

With a number on the left, `FloatVal` gives the mirror image:

- `1 <= [1,2]` is true;
- `1 == [1,1+ulp]` is true.

An empty interval cannot be built: `FloatVal(2,1)` throws
`std::runtime_error` "gecode_boost::interval: empty interval created"
[C17].

The behaviour to keep:

- `<`, `<=`, `>`, `>=`, and `!=` are true only if they hold for every pair
  of values ("certainly"). Gecode turns Boost's exception into false.
- `==` between two `FloatVal`s is true when the bounds are equal, which is
  set equality. This is patch 03, not Boost's semantics. Upstream returns
  true only for two equal singletons and otherwise throws.
- `==` with a number is true for the singleton and for an interval one ulp
  wide that has the number as a bound (`val.hpp:315-324`).

A direct implementation states these rules without exceptions: one or two
conditions per operator, for 18 operators. It replaces Boost's 116 lines
and `val.hpp`'s 123 [C22]. The empty-interval message names
`gecode_boost` and would change.

### API and ABI changes

Approach 3 changes:

- `gecode/float.hh:73`: the include goes.
- `RoundingControl`: today a typedef of a Boost class (`float.hh:130`,
  `:135`).
- `RoundingBase`: its base class.
- `Rounding`: today `save_state<RoundingBase>` (`float.hh:305`).
- `FloatVal::FloatValImpType` (`float.hh:445-452`): it changes, and with
  it the type of `FloatVal`'s member.
- `num.hpp`: the pi constants.
- `val.hpp`: the comparison operators.
- `test/float/arithmetic.cpp`: the `RoundingScope` test uses `interval`,
  `policies`, `checking_strict`, `fmod`, `sin`, and `save_state` directly.

The brief allows these changes in Gecode 7.

### Soundness-critical parts

Boost lines per definition, from [C22]:

- division by an interval containing zero: `detail/division.hpp`, 147
  lines, and the `operator/` overloads in `arith.hpp`, 52;
- `pow` with negative bases, and `pow(x, 0)`: `pow` 35, `pow_dn` 13,
  `pow_up` 13;
- `nth_root` and the interval Newton steps: `nth_root` 32, `root_aux` 13,
  `root_aux_dn` 11, `root_aux_up` 11. 1.92.0 already got this wrong once
  [C18].
- `fmod` and the range reduction in `cos` and `tan`: `fmod` 33, `cos` 24,
  `sin` 12, `tan` 17;
- the domain edges of `asin` 16, `acos` 16, and `acosh` 10.

## Cost and risk

### Approach 1: refresh to 1.92.0

**Initial effort.** Small:

- the script (`vendor.py`, 292 lines in scratch [C13]) as a tracked tool
  with a freshness check;
- 2 patches, a 55-file drop list, and the copy itself [C13], [C14].

It compiled without other changes [C16].

**Maintenance.** Someone must review every Boost release:

- 18 commits are already waiting on master, some in rounding [C11];
- each release can regress, as eee9745 did [C18];
- the freshness check covers only that the copy matches the script.

**Soundness risk.** Moderate. 1.92.0 as released changes `nth_root` and
the search in two examples [C18], [C19]. Gecode's arithmetic since boost-006
overrides Boost's, so upstream's arithmetic changes do not reach Gecode.

**API and ABI.** Unchanged.

**Installed footprint.** 52 headers, 9155 lines [C9]. That is more than
HEAD's union but less than the 96 installed today.

**MSVC and CI.** Every emulated target passes the preprocessing check with
no unresolved Boost header and no `#error`, including MSVC arm64 with the
old `__cplusplus` [C8]. This is `clang++ -M`, not a compile; real platform
builds are not verified. FreeBSD and Cygwin lose their platform header
unless the drop list keeps it [C25].

**Fit with the brief.**

- Met: no dependency; scripted copy; sound under optimization (rounding
  probe 9 of 9 [C19]).
- Met only with the unreleased fix: unchanged float semantics [C19].
- Gecode keeps shipping Boost headers and `gecode_boost` types in its
  public API. The brief allows a scripted copy, so this is a cost, not an
  unmet requirement.

### Approach 2: improve the 2012 copy

**Initial effort.** Smallest:

- the same script;
- 8 patches of 43 lines and a 48-file drop list [C13], [C15].

The result is a strict subset of HEAD's files, byte for byte [C15].

**Maintenance.** No upstream to track. The freshness check covers
everything. Fixes after 1.47.0 that Gecode lacks: those in Test B that
Gecode has not patched itself. None affects a measured target.

**Soundness risk.** Lowest. Probe, suite, and examples are the same as
HEAD [C17], [C18], [C19].

**API and ABI.** Unchanged.

**Installed footprint.** 47 headers, 6543 lines [C9].

**MSVC and CI.** As HEAD, including the emulated MSVC arm64 `#error`
[C8]. Same FreeBSD and Cygwin loss as approach 1 [C25].

**Fit with the brief.** Same as approach 1, but the semantics are
unchanged without any extra fix.

### Approach 3: support library

**Initial effort.** Largest:

- 974 to 1316 lines of interval code, plus 139 to 215 with MPFR [C22];
- a rounding layer;
- rewriting the comparisons and `RoundingScope`;
- Test::Base tests for the interval core.

The probes here (`cmp.cpp`, `nroot.cpp`) and rounding-probe.md's probe are
ready oracles. So is the existing copy, which can be diffed against while
both exist.

**Maintenance.** Gecode owns the code, with no upstream.

**Soundness risk.** Highest during the port. The critical parts are listed
in part (3). The Float:: suite, the examples' statistics, the rounding
probe, and the `nth_root` probe catch differences from today's behaviour.

**API and ABI.** The foreign types leave `float.hh`; see part (3). Only
Gecode 7 allows this.

**Installed footprint.** No third-party headers.

**MSVC and CI.** Gecode's code carries the `_control87` path, which today
lives in Boost. CI's Windows jobs all run on x64 `windows-2022` runners
(`.github/workflows/build.yml:497`, `:567`, `:618`), so MSVC arm64 stays
untested.

**Fit with the brief.**

- Met: no dependency, no copy.
- Directed rounding: as sound as Gecode's own layer.
- Semantics: unchanged, if the algorithms are ported as they are.

## Recommendation

This is a recommendation; the user decides in boost-004.

**Recommended: approach 3, the support library, keeping switched-mode
rounding.** Reasons:

- **The window.** The brief allows changing `Float::Rounding`,
  `RoundingBase`, and `FloatValImpType` because Gecode 7 is a breaking
  release. Approaches 1 and 2 keep `gecode_boost` types in the public API.
  Removing them later needs another breaking release.
- **What is left is small and fixed.** Since boost-006, Gecode owns the
  arithmetic, and `RoundingControl` supplies only mode control and
  `median`. What remains is 974 to 1316 lines of interval code, plus 139
  to 215 with MPFR [C22]. These are well-known case analyses that can be
  ported as they are, which keeps the bounds and the search unchanged.
- **The oracles exist.** A difference in the behaviour they exercise shows
  up in:
  - the Float:: suite;
  - the examples' statistics (identical search, as improve shows [C19]);
  - the rounding probe [C19];
  - the comparison truth table [C17];
  - the `nth_root` probe [C18].

  They do not cover everything: `proper_subset` and the hyperbolic
  functions are never instantiated [C21], so a port of those needs its own
  tests, and coverage gaps stay a risk of the port.
- **Refreshing buys nothing Gecode uses.** 1.92.0 changes no arithmetic,
  division, transcendental, comparison, or checking code [C10]. It needs a
  patch that no release has yet [C18], and it changes the search without
  that patch [C19].
- **Gecode stops shipping Boost.** The brief allows a scripted copy, but
  approach 3 is the only one that ships no Boost code and removes the
  foreign types from the public API.

nextafter-study.md bears on the rounding layer, not on the choice:

- P2 removes the mode switch from every `FloatVal` operation: 199.9 ns to
  60.5 ns in its microbenchmark (nextafter-study.md, its [C9]).
- But descartes-folium runs 1.471 times as long, inside Boost's `pow` and
  `root_aux` (nextafter-study.md, its [C8] and [C11]).
- P2 also changes edge-case bounds and needs the test harness's cuts
  clamped.
- P1 widens almost every bound and needs the user's acceptance.

So the recommendation keeps switched modes, which keeps today's bounds.
P2 can follow as a separate, measured change, on any of the three
approaches, since the nextafter prototypes replaced only `RoundingBase`'s
arithmetic.

**Second choice: approach 2.** It is the cheapest and the only one with no
measured behaviour change. It removes 57 files from the tree and 48 from
the install [C9], [C15]. Its script reproduces the copy, and it needs no
upstream tracking.

What would change the choice to approach 2:

- if the user is not willing to own 1113 to 1531 lines of
  soundness-critical code (the interval and MPFR ranges above) [C22];
- if Gecode 7 cannot wait for that port;
- if keeping `gecode_boost` types in the public API is acceptable.

What would favour approach 1: a Boost release with 6a59a02 and the October
2026 rounding fixes [C11], if Gecode wants to follow upstream. Even then it
adds 2612 lines to the installed headers over approach 2 [C9].

Whichever copy is kept, the drop list decides FreeBSD and Cygwin support
[C25]. Keeping `config/platform/bsd.hpp` and `cygwin.hpp` costs two files.

## Brief open questions

- **Directed rounding under optimization.** Answered in rounding-probe.md.
  It was unsound before boost-006, and its addendum reports all nine
  configurations exact after it. This study reran the probe: 0 unsound
  rows for every variant [C19].
- **What current Boost adds, and its closure.** Part (1) of this study.
- **Size of approach 3.** Part (3) of this study.
- **nextafter against switched modes.** Answered in nextafter-study.md.
- **Which approach.** The user decides in boost-004; this study only
  recommends.

## Commands

All commands run from `build/zdev-impl/boost-003`. `scripts/facts.sh <Cn>`
reruns the fact commands; its source is in Appendix A. The clones and
extracted trees come from `scripts/prepare.sh fetch`, then `trees`, and
then, after [C6] and [C8], `copies` and `sources`.

- **[C1] Setup.** `scripts/facts.sh C1`: HEAD, `git status --short`, tool
  versions, machine.
- **[C2] Pins.** `scripts/facts.sh C2`:
  - `git ls-remote --tags` on boostorg/boost;
  - the superproject commit and its submodule entries;
  - each module's `boost-1.92.0` and `boost-1.91.0` commits;
  - interval master's lead.
- **[C3] 2012 import.** `scripts/facts.sh C3`, after `prepare.sh trees`
  extracted `git archive 561102a86c`.
- **[C4] Base release.** `scripts/facts.sh C4`: `basematch.py` on 1.46.0,
  1.46.1, 1.47.0, and 1.48.0.
- **[C5] MSVC file, `.orig`, LICENSE, line endings.** `scripts/facts.sh
  C5`.
- **[C6] Textual closure.** `scripts/facts.sh C6`: `vendor.py closure` on
  the 1.47.0 and 1.92.0 trees.
- **[C7] Tracked extract.py.** `scripts/facts.sh C7`.
- **[C8] Per-target closure.** `scripts/facts.sh C8`: `target_closure.py`
  on upstream 1.47.0 and 1.92.0 and on `src/head`, `src/refresh`, and
  `src/improve`.
- **[C9] Sizes.** `scripts/facts.sh C9`, which runs `scripts/sizes.sh`.
- **[C10] Upstream change.** `scripts/facts.sh C10`: `upstream_diff.py` on
  interval and on Boost.Config, and the commit counts.
- **[C11] Upstream after 1.92.0.** `scripts/facts.sh C11`.
- **[C12] Test A.** `scripts/facts.sh C12`, which runs `scripts/test-a.sh`.
  `scripts/test-a.sh mkpatch` regenerates `a/patches` from HEAD and
  `a/patch-groups.txt`.
- **[C13] Patch sizes.** `scripts/facts.sh C13`: hunks and changed lines
  per patch and per patch set, and `wc -l scripts/vendor.py`.
- **[C14] Test B, copy and patches.** `scripts/facts.sh C14`. The copies
  `b/out0` (no patch), `b/out` (patch 03), and `b/out-fix` (patches 03 and
  12) come from `prepare.sh copies`.
- **[C15] Improved copy.** `scripts/facts.sh C15`.
- **[C16] Builds.** `scripts/prepare.sh sources`, then
  `scripts/build-all.sh > logs/build-all.log`. It builds head-rel,
  refresh-rel, improve-rel, refresh-nompfr, improve-nompfr, head-dbg, and
  head-dbg-nompfr. `scripts/refreshfix.sh` builds refreshfix-rel. Warnings
  are counted with `grep -c 'warning:'` in each build log. build/local's
  type: `grep -E '^(CMAKE_BUILD_TYPE|GECODE_ENABLE_MPFR)'
  build/local/CMakeCache.txt` from the repository root. The include
  check: `grep -l detail/fenv.hpp -r
  bld/refresh-rel/CMakeFiles/gecodefloat_shared.dir`.
- **[C17] Comparison probe.** `cd probe && ./run-probes.sh`. It builds
  `cmp.cpp` header-only against `src/head` and against overlays of
  `b/out0`, `b/out`, `b/out-fix`, and `c/out`, then diffs the outputs.
- **[C18] `nth_root` probe.** The same `run-probes.sh` for `nroot.cpp` at
  -O0 and -O2. `probe/build-nroot-all.sh` gives the per-row bound
  differences.
- **[C19] Behaviour.** `scripts/behaviour.sh` for head, refresh, and
  improve, and `scripts/refreshfix.sh` for refreshfix:
  - install;
  - `probe-<v>/run.sh`, with unchanged copies of
    `build/rounding-probe/rounding-probe.cpp` (SHA-256
    0704233d2257ee2245664dedf9c8cd9dc494e846e000efbc93ae0364fce50390) and
    `run.sh` (SHA-256
    f94e29820cc7cbb9b47e3ba302957e00f344392f86032072b779f41090de42dc);
  - `gecode-test -test Float::Arithmetic::DirectedRounding`;
  - each example with `-mode solution` and `-mode stat`, also from
    `build/local/bin`;
  - `gecode-test -test Float:: -stop false -threads 8`. The improve run
    hit the background time limit, so the improve and refreshfix suites
    ran again with `scripts/suites.sh improve refreshfix`.

  Comparisons:
  - `cmp probe-head/results/<f>.tsv probe-<v>/results/<f>.tsv` for the 9
    files;
  - `scripts/compare-examples.sh head refresh improve refreshfix`;
  - `grep 'Test summary' results/suite/<v>-t8.log`.
- **[C20] Source references.** `scripts/facts.sh C20`.
- **[C21] Symbol inventory.** `scripts/inventory.sh head-dbg` and
  `scripts/inventory.sh head-dbg-nompfr`. For the API check:
  `grep -P "^gecode_boost::numeric::<name>\t" results/inv/head-dbg.txt`
  for each function `val.hpp` calls.
- **[C22] Source lines.** `scripts/facts.sh C22`, then `python3 -I
  scripts/groups.py scripts/groups.txt results/sloc-boost-head.txt`.
  Per-definition lines are in `results/sloc-boost-head.txt` and
  `results/sloc-gecode-float.txt`.
- **[C23] P2 size.** `scripts/facts.sh C23`: added lines in
  nextafter-study.md Appendix A.2.
- **[C24] History of the copy.** `scripts/facts.sh C24`: the commits that
  touched the copy, and the files six of them touched.
- **[C25] FreeBSD and Cygwin.** `scripts/extra_targets.sh`.
- **[C26] Final state.** `git status --short` and
  `git status --short --ignored .zdev`, from the repository root.

## Not verified

- MSVC, x86-64, and MinGW builds and runs. The closures for them are
  emulated, without system headers. The MSVC arm64 `#error` is therefore
  only a likely finding.
- Real FreeBSD and Cygwin builds; the platform-header check is emulated
  [C25].
- Linux builds with GCC. CI covers them; this study did not run them.
- Boost interval master beyond the commit list. Only 6a59a02 was applied
  and tested.
- The support-library estimate is a line count of Boost's code. No port was
  written.
- Runtime of any variant.
- Float tests outside `Float::`, such as the FlatZinc float tests.

## Appendix A: scripts

### A.1 `scripts/facts.sh`

```sh
#!/bin/bash
# Recompute the study's facts, one section per command tag: facts.sh [Cn...]
# Run from build/zdev-impl/boost-003 after the clones in dl/, the extracted
# trees in up/, snap2012/, and the a/, b/, c/ outputs exist (see C3 to C15).
export LC_ALL=C
R=$(cd ../../.. && pwd)
export TMPDIR=$PWD/tmp  # scratch stays under build/zdev-impl/boost-003
H=$R/gecode/third-party/boost
UP47="up/interval-1.47.0/include up/config-1.47.0/include up/detail-1.47.0/include"
UP92="up/interval-1.92.0/include up/config-1.92.0/include up/detail-1.92.0/include up/logic-1.92.0/include"
LIC=dl/superproject/LICENSE_1_0.txt

C1() { # revision, tree state, tools
  git -C "$R" rev-parse HEAD; git -C "$R" status --short | wc -l
  /Library/Developer/CommandLineTools/usr/bin/c++ --version | head -1
  /usr/bin/clang++ --version | head -1; cmake --version | head -1; python3 --version
  grep -h 'define MPFR_VERSION_STRING' /opt/homebrew/include/mpfr.h
  sw_vers -productVersion; sysctl -n machdep.cpu.brand_string hw.ncpu
}
C2() { # upstream pins
  git ls-remote --tags https://github.com/boostorg/boost.git | grep -v '\^{}' |
    sed 's|.*refs/tags/||' | grep -E '^boost-1\.[0-9]+\.[0-9]+$' | sort -t. -k2,2n -k3,3n | tail -1
  git -C dl/superproject log -1 --format='superproject %H %ci'
  git -C dl/superproject ls-tree HEAD libs/numeric/interval libs/config libs/detail libs/logic
  for m in interval config detail logic; do
    echo "$m $(git -C dl/$m rev-parse boost-1.92.0^{commit}) 1.91.0=$(git -C dl/$m rev-parse boost-1.91.0^{commit})"
  done
  git -C dl/interval log -1 --format='interval %h a:%ai c:%ci %s' boost-1.92.0
  echo "interval master $(git -C dl/interval rev-parse --short=12 origin/HEAD)" \
    "ahead $(git -C dl/interval rev-list --count boost-1.92.0..origin/HEAD)"
}
C3() { # 2012 snapshot
  git -C "$R" log -1 --format='%H %ai %s' 561102a86c
  diff -r snap2012 "$R/build/zdev-plan/boost-003/snap2012" && echo same-as-planner
  echo "snap2012 files $(find snap2012 -type f | wc -l)"
  echo "HEAD files $(git -C "$R" ls-files gecode/third-party/boost | wc -l)"
}
C4() { # base release: per-file match of the 2012 import against upstream tags
  for t in 1.46.0 1.46.1 1.47.0 1.48.0; do
    echo "$t $(python3 -I scripts/basematch.py snap2012/gecode/third-party/boost up/interval-$t up/config-$t \
      up/detail-$t | tee results/basematch-$t.txt | tail -1)"
  done
  grep -v '^summary' results/basematch-1.47.0.txt
}
C5() { # the .orig file, the MSVC difference, LICENSE, line endings
  S=snap2012/gecode/third-party/boost
  U=up/interval-1.47.0/include/boost/numeric/interval/detail/msvc_rounding_control.hpp
  diff <(tr -d '\r' < $U) <(tr -d '\r' < $S/numeric/interval/detail/msvc_rounding_control.hpp.orig)
  diff <(tr -d '\r' < $U) <(tr -d '\r' < $S/numeric/interval/detail/msvc_rounding_control.hpp)
  cmp <(tr -d '\r' < $S/LICENSE_1_0.txt) $LIC && echo license-same
  printf 'interval.hpp CR=%s LF=%s\n' $(tr -cd '\r' < $S/numeric/interval.hpp | wc -c) \
    $(tr -cd '\n' < $S/numeric/interval.hpp | wc -c)
}
C6() { # textual closure, fixed resolver
  python3 -I scripts/vendor.py closure $UP47 > results/closure-1.47.0.txt; echo "1.47.0 rc=$?"
  grep -v '^file' results/closure-1.47.0.txt
  grep '^file' results/closure-1.47.0.txt | cut -f2 | sed 's|^boost/||' | sort > results/closure-1.47.0.files
  echo "1.47.0 closure vs 2012 import less extract.perl, patch.txt, LICENSE:"
  comm -3 results/closure-1.47.0.files \
    <(grep -vE '^(extract.perl|patch.txt|LICENSE_1_0.txt)$' results/snap2012-files.txt | sort)
  python3 -I scripts/vendor.py closure $UP92 > results/closure-1.92.0.txt; echo "1.92.0 rc=$?"
  grep -v '^file\|^system' results/closure-1.92.0.txt
  grep -c '^system' results/closure-1.92.0.txt
  grep '^file' results/closure-1.92.0.txt | cut -f2 | sed 's|^boost/||' | sort > results/closure-1.92.0.files
  grep -E 'logic|detail/fenv' results/closure-1.92.0.files
}
C7() { # the tracked extract.py on unrenamed upstream
  for t in 1.47.0 1.92.0; do
    (cd up/merged-$t && python3 -I "$H/extract.py" boost/numeric/interval.hpp /dst/ \
      > ../../results/extractpy-$t.txt 2>&1; echo "extract.py $t rc=$?")
    echo "cp lines $(grep -c '^cp ' results/extractpy-$t.txt)"; grep -v '^cp \|^mkdir' results/extractpy-$t.txt | tail -1
  done
  sed -n 's|^cp "boost/\([^"]*\)".*|\1|p' results/extractpy-1.92.0.txt | sort > results/extractpy-1.92.0.files
  echo "in the fixed closure only:"; comm -23 results/closure-1.92.0.files results/extractpy-1.92.0.files
}
C8() { # per-target closure through the preprocessor
  mkdir -p results/tc
  for t in 1.47.0 1.92.0; do echo "## $t"; python3 -I scripts/target_closure.py up/merged-$t \
    boost/numeric/interval.hpp results/tc/up-$t | cut -c1-200; done
  for v in head refresh improve; do echo "## src/$v"; python3 -I scripts/target_closure.py src/$v \
    gecode/third-party/boost/numeric/interval.hpp results/tc/src-$v | cut -c1-200; done
  sed 's|^gecode/third-party/boost/||' results/tc/src-head-union.files > results/tc/head-union.rel
  sed 's|^boost/||' results/tc/up-1.92.0-union.files > results/tc/up192-union.rel
  echo "1.92.0 union only:"; comm -13 results/tc/head-union.rel results/tc/up192-union.rel
  echo "HEAD union only:"; comm -23 results/tc/head-union.rel results/tc/up192-union.rel
}
C9() { # header set sizes
  scripts/sizes.sh
}
C10() { # upstream change 1.47.0 to 1.92.0
  python3 -I scripts/upstream_diff.py up/interval-1.47.0/include up/interval-1.92.0/include
  python3 -I scripts/upstream_diff.py up/merged-1.47.0 up/merged-1.92.0 boost/config | tail -1
  git -C dl/interval merge-base --is-ancestor boost-1.47.0 boost-1.92.0 && echo ancestor || echo not-ancestor
  echo "commits 1.47.0..1.92.0 $(git -C dl/interval rev-list --count boost-1.47.0..boost-1.92.0 -- include)"
  echo "commits since 2011-07-11 $(git -C dl/interval rev-list --count --since=2011-07-11 boost-1.92.0 -- include)"
  git -C dl/interval log --format='%h %ad %s' --date=short --since=2011-07-11 boost-1.92.0 -- include
  git -C dl/interval log -1 --format='boost-1.47.0 tagged %ci' boost-1.47.0
  echo "2d5a340 first in $(git -C dl/interval tag --contains 2d5a340 | sort -V | head -1)"
}
C11() { # upstream after 1.92.0
  git -C dl/interval log --format='%h %ad %s' --date=short boost-1.92.0..origin/HEAD -- include
  git -C dl/interval diff --shortstat boost-1.92.0 origin/HEAD -- include
}
C12() { # test A: byte-exact reproduction of HEAD from 1.47.0
  wc -l < a/manifest-1.47.0.txt; grep -vc '^#' a/strip-ws.txt
  scripts/test-a.sh > results/testA.txt 2>&1; echo "rc=$?"; tail -1 results/testA.txt
}
C13() { # patch sizes
  scripts/patchstats.sh a/patches c/patches b/patches b/patches-fix
  wc -l < scripts/vendor.py
  for p in a/patches/*.patch; do
    echo "$(basename $p) hunks=$(grep -c '^@@' $p) lines=$(grep -cE '^[-+]([^-+]|$)' $p)"
  done
}
C14() { # test B: 1.92.0 copy, patch status, renaming leftovers
  wc -l < b/drop-1.92.0.txt
  for p in a/patches/*.patch; do echo "== $(basename $p)"; patch --dry-run -p1 -N -d b/out0 -i "$PWD/$p" 2>&1 |
    grep -E 'hunks? (FAILED|failed|ignored)|No file|previously applied' | head -2; done
  echo "b/out files $(find b/out -type f | wc -l)"
  grep -rnE '(^|[^A-Za-z0-9_])boost::|namespace boost([^A-Za-z0-9_]|$)|[<"]boost/|(^|[^A-Za-z0-9_])BOOST_' b/out |
    wc -l
  diff -r b/out b/out-fix | grep -c '^[<>]'
}
C15() { # approach 2: the pruned 1.47.0 copy
  wc -l < c/drop-1.47.0.txt; ls c/patches
  echo "c/out files $(find c/out -type f | wc -l)"
  echo "differing $(diff -rq c/out "$H" | grep -vc '^Only in /')"
  echo "only in HEAD $(diff -rq c/out "$H" | grep -c '^Only in /')"
  tr '\n' ' ' < c/drop-1.47.0.txt; echo
}
C24() { # Gecode's history of the copy
  git -C "$R" log --format='%h %ad %s' --date=short -- gecode/third-party/boost | tee /dev/stderr | wc -l
  for c in 448fcee00f 87fa9d96fc 8207f7fd29 9a282b12ed 2a9c43036e 19bff4fb9b; do
    echo "$c: $(git -C "$R" show --stat=200 --format= $c -- gecode/third-party/boost |
      sed 's|gecode/third-party/boost/||' | awk 'NF > 2 && $2 == "|" {printf "%s ", $1}')"
  done
}
C20() { # source references outside the copy
  git -C "$R" grep -n -E 'gecode_boost|GECODE_BOOST|third-party/boost' -- ':!gecode/third-party/boost' ':!.zdev' |
    cut -d: -f1 | sort | uniq -c
  grep -c 'comparison_error' "$R/gecode/float/val.hpp"
  git -C "$R" grep -n -E 'is_neg|is_zero|is_pos' -- gecode test ':!gecode/third-party/boost' | wc -l
}
C22() { # source lines
  (cd "$H/numeric/interval" && python3 -I "$OLDPWD/scripts/sloc.py" interval.hpp checking.hpp policies.hpp \
    arith.hpp arith2.hpp utility.hpp transc.hpp constants.hpp rounding.hpp hw_rounding.hpp rounded_arith.hpp \
    rounded_transc.hpp detail/division.hpp detail/test_input.hpp detail/bugs.hpp detail/interval_prototype.hpp \
    detail/c99_rounding_control.hpp detail/c99sub_rounding_control.hpp detail/msvc_rounding_control.hpp \
    detail/x86_rounding_control.hpp) > results/sloc-boost-head.txt
  awk -F'\t' '$2 == "FILE" {s += $3; print} END {print "sum", s}' results/sloc-boost-head.txt
  (cd "$R" && python3 -I "$OLDPWD/scripts/sloc.py" gecode/float/rounding.hpp gecode/float/rounding.cpp \
    gecode/float/val.hpp gecode/float/num.hpp) > results/sloc-gecode-float.txt
  grep FILE results/sloc-gecode-float.txt
}
C23() { # P2 patch size from nextafter-study.md Appendix A.2
  sed -n '/^### A.2 P2/,/^### A.3/p' "$R/.zdev/boost/background/nextafter-study.md" | grep -c '^+[^+]'
}

if [ $# -eq 0 ]; then set -- C1 C2 C3 C4 C5 C6 C7 C8 C9 C10 C11 C12 C13 C14 C15 C20 C22 C23 C24; fi
for c in "$@"; do echo "##### $c"; $c; done
```

### A.2 `scripts/prepare.sh`

```sh
#!/bin/bash
# Fetch upstream and build the vendored trees the study measures:
# prepare.sh fetch|trees|copies|sources. Run from build/zdev-impl/boost-003.
# fetch needs the network; the other steps only read dl/ and the checkout.
set -e
export LC_ALL=C
R=$(cd ../../.. && pwd)
export TMPDIR=$PWD/tmp  # scratch stays under build/zdev-impl/boost-003
H=$R/gecode/third-party/boost
UP47="up/interval-1.47.0/include up/config-1.47.0/include up/detail-1.47.0/include"
UP92="up/interval-1.92.0/include up/config-1.92.0/include up/detail-1.92.0/include up/logic-1.92.0/include"
LIC=dl/superproject/LICENSE_1_0.txt
V="python3 -I scripts/vendor.py"

fetch() { # clones (untrusted data, only read by scripts)
  mkdir -p dl
  git clone -q https://github.com/boostorg/interval.git dl/interval
  for m in config logic detail; do
    git clone -q --filter=blob:none --no-checkout https://github.com/boostorg/$m.git dl/$m
  done
  git clone -q --depth 1 --branch boost-1.92.0 https://github.com/boostorg/boost.git dl/superproject
}
trees() { # per-tag include trees, merged trees, the 2012 import
  for t in 1.46.0 1.46.1 1.47.0 1.48.0 1.92.0; do
    for m in interval config detail logic; do
      d=up/$m-$t; rm -rf $d; mkdir -p $d
      git -C dl/$m archive boost-$t include 2>/dev/null | tar -x -C $d || echo "no $m $t"
    done
  done
  for t in 1.47.0 1.92.0; do
    m=up/merged-$t; rm -rf $m; mkdir -p $m
    for mod in interval config detail logic; do cp -R up/$mod-$t/include/. $m/; done
  done
  rm -rf snap2012; mkdir -p snap2012
  git -C "$R" archive 561102a86c gecode/third-party/boost | tar -x -C snap2012
  (cd snap2012/gecode/third-party/boost && find . -type f | sed 's|^\./||' | sort) > results/snap2012-files.txt
}
copies() { # needs results/closure-*.files (facts.sh C6) and results/tc/*-union.rel (facts.sh C8)
  # Test A input: the 1.47.0 closure plus the abi headers HEAD carries.
  (cat results/closure-1.47.0.files; cd "$H" && find config/abi config/abi_*.hpp -type f) | sort \
    > a/manifest-1.47.0.txt
  scripts/test-a.sh mkpatch > /dev/null
  # Approach 1: 1.92.0, files outside the per-target union dropped.
  comm -23 results/closure-1.92.0.files results/tc/up192-union.rel > b/drop-1.92.0.txt
  rm -rf b/out0 b/out b/out-fix b/patches b/patches-fix; mkdir -p b/patches b/patches-fix
  $V build --drop b/drop-1.92.0.txt --license $LIC --out b/out0 $UP92 > /dev/null
  cp a/patches/03-interval-eq-3e448c0d5c.patch b/patches/
  $V build --drop b/drop-1.92.0.txt --license $LIC --patches b/patches --out b/out $UP92 > /dev/null
  cp b/patches/*.patch b/patches-fix/
  git -C dl/interval show --format= 6a59a02 -- include/boost/numeric/interval/arith2.hpp |
    sed -e 's|^--- a/include/boost/|--- a/|' -e 's|^+++ b/include/boost/|+++ b/|' |
    grep -v '^diff --git\|^index ' > b/patches-fix/12-nth-root-rounding-upstream-6a59a02.patch
  $V build --drop b/drop-1.92.0.txt --license $LIC --patches b/patches-fix --out b/out-fix $UP92 > /dev/null
  # Approach 2: 1.47.0, files outside HEAD's per-target union dropped.
  comm -23 results/closure-1.47.0.files results/tc/head-union.rel > c/drop-1.47.0.txt
  rm -rf c/out c/patches; mkdir -p c/patches
  for p in 01 03 04 05 06 07 09 10; do cp a/patches/$p-*.patch c/patches/; done
  $V build --drop c/drop-1.47.0.txt --license $LIC --strip-ws a/strip-ws.txt --patches c/patches --out c/out \
    $UP47 > /dev/null
}
sources() { # scratch source copies of HEAD with the copy replaced
  for v in head refresh improve refreshfix; do
    rm -rf src/$v; mkdir -p src/$v; git -C "$R" archive HEAD | tar -x -C src/$v
  done
  for v in refresh:b/out improve:c/out refreshfix:b/out-fix; do
    rm -rf src/${v%%:*}/gecode/third-party/boost; cp -R ${v#*:} src/${v%%:*}/gecode/third-party/boost
  done
}
"$@"
```

### A.3 `scripts/vendor.py`

```python
"""Compute a Boost header closure and produce a renamed, patched copy.

Scratch prototype for boost-003. Not part of the Gecode build.

  vendor.py closure --prefix P --root R [--allow FILE] INCDIR...
      Textual closure of P/R over every preprocessor branch. INCDIR holds
      P/... (an upstream module's include/ directory, or a vendored root's
      parent). Prints the files, the system headers, and every include it
      could not resolve, with file and line. Exits 1 if any include is
      unresolved and not allowlisted, or if a P/ include names a missing file.

  vendor.py build --root R --out DIR [--manifest FILE] [--drop FILE]
                  [--patches DIR] [--license FILE] [--rename gecode]
                  [--check DIR] INCDIR...
      Copy the closure (or the files listed in --manifest), minus the files
      in --drop, transform them (line endings, encoding, renaming), apply the
      unified diffs in --patches in name order, and write LICENSE_1_0.txt.
      With --check DIR, compare the result with DIR and exit 1 on any
      difference.
"""

import argparse
import filecmp
import re
import subprocess
import sys
import tempfile
from pathlib import Path

INCLUDE = re.compile(r"^[ \t]*#[ \t]*include(?:_next)?[ \t]*(.*)$")
DEFINE = re.compile(r"^[ \t]*#[ \t]*define[ \t]+([A-Za-z0-9_]+)[ \t]+(.*)$")


def strip_comments(text: str) -> str:
    """Remove C and C++ comments, keeping line structure and strings."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == '"' or c == "'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("\n" * text.count("\n", i, j))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def read_text(p: Path) -> str:
    data = p.read_bytes()
    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        return data.decode("latin-1")


def logical_lines(text: str):
    """Yield (line number, line) with backslash continuations joined."""
    lines = strip_comments(text).replace("\r", "").split("\n")
    i = 0
    while i < len(lines):
        start = i
        cur = lines[i]
        while cur.endswith("\\") and i + 1 < len(lines):
            i += 1
            cur = cur[:-1] + lines[i]
        yield start + 1, cur
        i += 1


def closure(prefix: str, root: str, incdirs: list[Path]):
    """Return (files, system, unresolved, missing) for prefix/root."""

    def find(rel: str):
        for d in incdirs:
            if (d / rel).is_file():
                return d / rel
        return None

    files: dict[str, Path] = {}
    system: dict[str, set[str]] = {}
    pending: list[tuple[str, str, int, str]] = []  # (macro, file, line, text)
    defs: dict[str, set[str]] = {}
    missing: list[tuple[str, str, int]] = []
    todo = [f"{prefix}/{root}"]
    seen = set(todo)

    def add(target: str, src: str, line: int):
        if target in seen:
            return
        seen.add(target)
        if find(target) is None:
            missing.append((target, src, line))
        else:
            todo.append(target)

    def header_name(spec: str):
        m = re.match(r'^<([^>]+)>', spec) or re.match(r'^"([^"]+)"', spec)
        return (m.group(1), spec[0]) if m else (None, None)

    while todo:
        rel = todo.pop()
        path = find(rel)
        files[rel] = path
        here = rel.rsplit("/", 1)[0]
        for ln, line in logical_lines(read_text(path)):
            m = DEFINE.match(line)
            if m:
                name, spec = header_name(m.group(2).strip())
                if name is not None:
                    defs.setdefault(m.group(1), set()).add(name)
                continue
            m = INCLUDE.match(line)
            if not m:
                continue
            spec = m.group(1).strip()
            name, kind = header_name(spec)
            if name is None:
                pending.append((spec.split()[0], rel, ln, spec))
                continue
            if name.startswith(prefix + "/"):
                add(name, rel, ln)
            elif kind == '"' and find(f"{here}/{name}") is not None:
                add(f"{here}/{name}", rel, ln)
            else:
                system.setdefault(name, set()).add(rel)
        if not todo:
            # Resolve macro includes against every definition seen so far.
            for macro, src, ln, spec in pending:
                for name in sorted(defs.get(macro, ())):
                    if name.startswith(prefix + "/"):
                        add(name, src, ln)
                    else:
                        system.setdefault(name, set()).add(src)
    unresolved = sorted({(macro, src, ln) for macro, src, ln, _ in pending
                         if macro not in defs})
    return files, system, unresolved, missing


def load_allow(path):
    allow = {}
    if path:
        for line in Path(path).read_text().splitlines():
            if line.strip() and not line.startswith("#"):
                macro, reason = line.split(None, 1)
                allow[macro] = reason
    return allow


def cmd_closure(a) -> int:
    files, system, unresolved, missing = closure(a.prefix, a.root, a.incdirs)
    allow = load_allow(a.allow)
    print(f"# files {len(files)}")
    for rel in sorted(files):
        print(f"file\t{rel}")
    for name in sorted(system):
        print(f"system\t{name}\t{','.join(sorted(system[name]))}")
    bad = 0
    for macro, src, ln in unresolved:
        tag = "allowed" if macro in allow else "unresolved"
        bad += macro not in allow
        print(f"{tag}\t{macro}\t{src}:{ln}\t{allow.get(macro, '')}")
    for target, src, ln in missing:
        print(f"missing\t{target}\t{src}:{ln}")
    print(f"# unresolved {bad} allowed {len(unresolved) - bad} "
          f"missing {len(missing)} system {len(system)}")
    return 1 if bad or missing else 0


# Renaming rules, applied in this order to every copied file.
RENAME = [
    # Include paths, in directives and in macro values naming headers.
    (re.compile(r'([<"])boost/'), r"\1gecode/third-party/boost/"),
    # Macros: BOOST_X -> GECODE_BOOST_X, unless already prefixed.
    (re.compile(r"(?<![A-Za-z0-9_])BOOST_"), "GECODE_BOOST_"),
    # Namespace: boost:: and namespace boost.
    (re.compile(r"(?<![A-Za-z0-9_])boost(?=\s*::)"), "gecode_boost"),
    (re.compile(r"(namespace\s+)boost(?![A-Za-z0-9_])"), r"\1gecode_boost"),
]


def transform(data: bytes, rename: bool, strip_ws: bool = False) -> bytes:
    text = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    try:
        s = text.decode("utf-8")
    except UnicodeDecodeError:
        s = text.decode("latin-1")
    if rename:
        for pat, rep in RENAME:
            s = pat.sub(rep, s)
    if strip_ws:
        s = re.sub(r"[ \t]+$", "", s, flags=re.M)
    return s.encode("utf-8")


def cmd_build(a) -> int:
    pfx = "boost"
    if a.manifest:
        rels = [line.strip() for line in Path(a.manifest).read_text().splitlines()
                if line.strip() and not line.startswith("#")]
        rels = [f"{pfx}/{r}" for r in rels]
    else:
        files, _, unresolved, missing = closure(pfx, a.root, a.incdirs)
        allow = load_allow(a.allow)
        if missing or any(m not in allow for m, _, _ in unresolved):
            sys.exit("closure incomplete; run the closure command")
        rels = sorted(files)
    drop = set()
    if a.drop:
        drop = {line.split()[0] for line in Path(a.drop).read_text().splitlines()
                if line.strip() and not line.startswith("#")}
    strip = set()
    if a.strip_ws:
        strip = {line.split()[0] for line in Path(a.strip_ws).read_text().splitlines()
                 if line.strip() and not line.startswith("#")}
    out = Path(tempfile.mkdtemp()) if a.check and not a.out else Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    copied = 0
    for rel in rels:
        sub = rel[len(pfx) + 1:]
        if sub in drop:
            continue
        src = next((d / rel for d in a.incdirs if (d / rel).is_file()), None)
        if src is None:
            sys.exit(f"missing {rel}")
        dst = out / sub
        dst.parent.mkdir(parents=True, exist_ok=True)
        dst.write_bytes(transform(src.read_bytes(), a.rename == "gecode", sub in strip))
        copied += 1
    if a.license:
        (out / "LICENSE_1_0.txt").write_bytes(Path(a.license).read_bytes())
    applied = 0
    if a.patches:
        for p in sorted(Path(a.patches).glob("*.patch")):
            r = subprocess.run(["patch", "-s", "-p1", "-N", "-r", "-", "-d", str(out),
                                "-i", str(p.resolve())],
                               capture_output=True, text=True)
            if r.returncode != 0:
                print(f"patch failed\t{p.name}\t{r.stdout.strip()} {r.stderr.strip()}")
                return 2
            applied += 1
        for orig in out.rglob("*.orig"):
            orig.unlink()
    print(f"# copied {copied} patches {applied} out {out}")
    if a.check:
        r = subprocess.run(["diff", "-r", str(a.check), str(out)] +
                           sum((["-x", x] for x in a.exclude), []),
                           capture_output=True, text=True)
        sys.stdout.write(r.stdout)
        print(f"# check {'ok' if r.returncode == 0 else 'DIFFERS'}")
        return r.returncode
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    sp = ap.add_subparsers(dest="cmd", required=True)
    c = sp.add_parser("closure")
    c.add_argument("--prefix", default="boost")
    c.add_argument("--root", default="numeric/interval.hpp")
    c.add_argument("--allow")
    c.add_argument("incdirs", nargs="+", type=Path)
    b = sp.add_parser("build")
    b.add_argument("--root", default="numeric/interval.hpp")
    b.add_argument("--out")
    b.add_argument("--manifest")
    b.add_argument("--drop")
    b.add_argument("--strip-ws")
    b.add_argument("--allow")
    b.add_argument("--patches")
    b.add_argument("--license")
    b.add_argument("--rename", choices=["gecode", "none"], default="gecode")
    b.add_argument("--check")
    b.add_argument("--exclude", action="append", default=[])
    b.add_argument("incdirs", nargs="+", type=Path)
    a = ap.parse_args()
    return cmd_closure(a) if a.cmd == "closure" else cmd_build(a)


if __name__ == "__main__":
    raise SystemExit(main())
```

### A.4 `scripts/test-a.sh`

```sh
#!/bin/sh
# Test A: reproduce HEAD's gecode/third-party/boost from Boost 1.47.0.
# Run from build/zdev-impl/boost-003. mkpatch regenerates a/patches from HEAD.
set -e
R=$(cd ../../.. && pwd)
export TMPDIR=$PWD/tmp  # scratch stays under build/zdev-impl/boost-003
H=$R/gecode/third-party/boost
UP="up/interval-1.47.0/include up/config-1.47.0/include up/detail-1.47.0/include"
V="python3 -I scripts/vendor.py build --manifest a/manifest-1.47.0.txt --strip-ws a/strip-ws.txt --license dl/superproject/LICENSE_1_0.txt"
if [ "$1" = mkpatch ]; then
  rm -rf a/out1 a/patches; mkdir -p a/patches
  $V --out a/out1 $UP
  while read -r name files; do
    : > a/patches/$name.patch
    for f in $files; do
      (cd a && diff -u --label a/$f --label b/$f out1/$f "$H/$f" >> patches/$name.patch || true)
    done
  done < a/patch-groups.txt
fi
$V --patches a/patches --check "$H" --exclude extract.py --exclude patch.txt --exclude '*.orig' $UP
```

### A.5 `a/patch-groups.txt`

```text
01-msvc-rounding-2012import numeric/interval/detail/msvc_rounding_control.hpp
03-interval-eq-3e448c0d5c numeric/interval/interval.hpp
04-rounded-arith-this-3e448c0d5c-87fa9d96fc numeric/interval/rounded_arith.hpp
05-visualc-version-6e0efb1dec-13232ee695-4d14667bf7-90bb5ddd80 config/compiler/visualc.hpp
06-libstdcpp3-threads-d78bb0b1d1 config/stdlib/libstdcpp3.hpp
07-compare-set-unused-3cb43dc1d6 numeric/interval/compare/set.hpp
08-abi-unrenamed-f813edf841 config/abi_prefix.hpp config/abi_suffix.hpp
09-hw-rounding-c36470ad8e-cde7a6c1cf numeric/interval/hw_rounding.hpp
10-utility-unused-typedef-14857e39d9 numeric/interval/utility.hpp
11-ppc-long-long-f753986df1-be7f5d7d1c numeric/interval/detail/ppc_rounding_control.hpp
```

### A.6 `a/strip-ws.txt`

```text
# Trailing whitespace removed by Gecode commits 448fcee00f (11 headers) and 87fa9d96fc (rounded_arith.hpp).
config.hpp
config/abi_prefix.hpp
config/abi_suffix.hpp
config/posix_features.hpp
config/select_compiler_config.hpp
config/select_platform_config.hpp
config/select_stdlib_config.hpp
config/suffix.hpp
config/user.hpp
detail/limits.hpp
limits.hpp
numeric/interval/rounded_arith.hpp
```

### A.7 `scripts/basematch.py`

```python
"""Compare a vendored Boost tree with upstream module trees, per file.

Usage: python3 -I basematch.py <vendored-root> <module-root>...

Each module root holds include/boost/. A vendored file X matches when some
module has include/boost/X with the same bytes after removing CR characters.
Prints one line per vendored file: same, differ (with changed line count),
or missing, and a summary.
"""

import difflib
import sys
from pathlib import Path

SKIP = {"extract.perl", "extract.py", "patch.txt", "LICENSE_1_0.txt"}


def norm(p: Path) -> list[str]:
    return p.read_bytes().replace(b"\r", b"").decode("latin-1").splitlines()


def main() -> int:
    root = Path(sys.argv[1])
    mods = [Path(m) / "include" / "boost" for m in sys.argv[2:]]
    counts = {"same": 0, "differ": 0, "missing": 0}
    for f in sorted(p for p in root.rglob("*") if p.is_file()):
        rel = f.relative_to(root).as_posix()
        if rel in SKIP:
            continue
        cands = [m / rel for m in mods if (m / rel).is_file()]
        if not cands:
            print(f"missing\t{rel}")
            counts["missing"] += 1
            continue
        a = norm(f)
        b = norm(cands[0])
        if a == b:
            counts["same"] += 1
            continue
        n = sum(1 for d in difflib.unified_diff(b, a, lineterm="", n=0)
                if d[:1] in "+-" and d[:3] not in ("+++", "---"))
        print(f"differ\t{rel}\t{n}")
        counts["differ"] += 1
    print("summary\t" + "\t".join(f"{k}={v}" for k, v in counts.items()))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

### A.8 `scripts/target_closure.py`

```python
"""Per-target Boost header closure through the preprocessor.

Usage: python3 -I target_closure.py <include-root> <header> <out-prefix>

<include-root> is the directory passed with -I, and <header> the include
path below it (boost/numeric/interval.hpp or
gecode/third-party/boost/numeric/interval.hpp). For each target, runs
`clang++ -M -MG` on a one-line translation unit and keeps the dependencies
inside <include-root>. Writes <out-prefix>-<target>.files and prints one
summary line per target, then the union.

macOS targets use the real SDK. The others have no sysroot: -nostdinc and
-MG let missing system headers through, and the standard library is
identified by predefining its detection macro. Compiler emulation (-U
__clang__) makes Boost.Config select its GCC or MSVC configuration.
"""

import subprocess
import sys
import tempfile
from pathlib import Path

CLANG = "/usr/bin/clang++"
NOSYS = ["-nostdinc", "-nostdinc++", "-MG"]
MSVC = [*NOSYS, "-fms-compatibility", "-fms-extensions", "-fmsc-version=1940",
        "-U__clang__", "-U__clang_major__", "-U__clang_minor__",
        "-D_CPPLIB_VER=650", "-D_MSVC_STL_VERSION=143"]
TARGETS = {
    "macos-arm64-appleclang": ["--target=arm64-apple-macos13"],
    "macos-x86_64-appleclang": ["--target=x86_64-apple-macos13"],
    "linux-x86_64-gcc": ["--target=x86_64-linux-gnu", *NOSYS, "-U__clang__",
                         "-D__GLIBCXX__=20240801", "-D__GLIBC__=2"],
    "linux-aarch64-gcc": ["--target=aarch64-linux-gnu", *NOSYS, "-U__clang__",
                          "-D__GLIBCXX__=20240801", "-D__GLIBC__=2"],
    "linux-x86_64-clang-libstdcxx": ["--target=x86_64-linux-gnu", *NOSYS,
                                     "-D__GLIBCXX__=20240801", "-D__GLIBC__=2"],
    "linux-aarch64-clang-libcxx": ["--target=aarch64-linux-gnu", *NOSYS,
                                   "-D_LIBCPP_VERSION=200000", "-D__GLIBC__=2"],
    "mingw-x86_64-gcc": ["--target=x86_64-w64-windows-gnu", *NOSYS, "-U__clang__",
                         "-D__GLIBCXX__=20240801"],
    "msvc-x64": [*MSVC, "--target=x86_64-pc-windows-msvc", "-U__x86_64__",
                 "-U__x86_64", "-U__amd64__", "-U__amd64"],
    "msvc-arm64": [*MSVC, "--target=aarch64-pc-windows-msvc", "-U__aarch64__"],
    # cl.exe reports __cplusplus 199711L unless /Zc:__cplusplus is given.
    "msvc-x64-cplusplus199711": [*MSVC, "--target=x86_64-pc-windows-msvc",
                                 "-U__x86_64__", "-U__x86_64", "-U__amd64__",
                                 "-U__amd64", "-U__cplusplus", "-D__cplusplus=199711L"],
    "msvc-arm64-cplusplus199711": [*MSVC, "--target=aarch64-pc-windows-msvc",
                                   "-U__aarch64__", "-U__cplusplus",
                                   "-D__cplusplus=199711L"],
}


def deps(text: str) -> list[str]:
    body = text.split(":", 1)[1].replace("\\\n", " ")
    return body.split()


def main() -> int:
    root = Path(sys.argv[1]).resolve()
    header = sys.argv[2]
    out = sys.argv[3]
    union: set[str] = set()
    tu = Path(tempfile.mkdtemp()) / "tu.cpp"
    tu.write_text(f"#include <{header}>\n")
    for name, flags in TARGETS.items():
        cmd = [CLANG, "-std=c++23", "-M", *flags, "-Wno-builtin-macro-redefined",
               "-I", str(root), str(tu)]
        r = subprocess.run(cmd, capture_output=True, text=True)
        files = set()
        missing = set()
        for d in deps(r.stdout) if r.stdout else []:
            p = Path(d)
            if p.is_absolute():
                try:
                    files.add(p.resolve().relative_to(root).as_posix())
                except ValueError:
                    pass
            elif "boost/" in d:
                missing.add(d)
        errors = [line for line in r.stderr.splitlines() if "error" in line]
        Path(f"{out}-{name}.files").write_text("".join(f"{f}\n" for f in sorted(files)))
        union |= files
        print(f"{name}\trc={r.returncode}\tfiles={len(files)}\tmissing_boost={len(missing)}"
              f"\terrors={len(errors)}\t{'; '.join(errors)[:300]}")
        for m in sorted(missing):
            print(f"  missing {m}")
    Path(f"{out}-union.files").write_text("".join(f"{f}\n" for f in sorted(union)))
    print(f"union\tfiles={len(union)}")
    print("cmd", " ".join([CLANG, "-std=c++23", "-M", "<flags>", "-I", str(root), "tu.cpp"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

### A.9 `scripts/sizes.sh`

```sh
#!/bin/sh
# File and line counts of the Boost header sets. Run from build/zdev-impl/boost-003.
R=$(cd ../../.. && pwd)
H=$R/gecode/third-party/boost
count() { # label base listfile
  n=0; l=0
  while read -r f; do n=$((n+1)); l=$((l + $(wc -l < "$2/$f"))); done < "$3"
  printf '%s\tfiles=%d\tlines=%d\n' "$1" $n $l
}
(cd $H && find . -type f | sed 's|^\./||' | sort) > results/head-all.files
count "HEAD tracked" $H results/head-all.files
(cd $H && find . -type f \( -name '*.hpp' -o -name LICENSE_1_0.txt \) | grep -v '/abi' | sed 's|^\./||' | sort) > results/head-installed.files
count "HEAD installed (*.hpp, LICENSE, not abi*)" $H results/head-installed.files
count "1.47.0 textual closure" up/merged-1.47.0/boost results/closure-1.47.0.files
count "1.92.0 textual closure" up/merged-1.92.0/boost results/closure-1.92.0.files
count "HEAD per-target union" $H results/tc/head-union.rel
count "1.92.0 per-target union" up/merged-1.92.0/boost results/tc/up192-union.rel
sed 's|^boost/||' results/tc/up-1.47.0-union.files > results/tc/up147-union.rel
count "1.47.0 per-target union" up/merged-1.47.0/boost results/tc/up147-union.rel
```

### A.10 `scripts/upstream_diff.py`

```python
"""Count changed lines per file between two upstream include trees.

Usage: python3 -I upstream_diff.py <old-include> <new-include> [<subdir>]

Compares every file under <subdir> (default boost/numeric) present in either
tree. Lines are compared after removing CR and trailing whitespace; lines
that differ only in their byte encoding (Latin-1 against UTF-8) are not
counted. Prints changed (added + removed) lines per file and a total.
"""

import difflib
import sys
from pathlib import Path


def lines(p: Path) -> list[str]:
    if not p.is_file():
        return []
    data = p.read_bytes().replace(b"\r", b"")
    try:
        s = data.decode("utf-8")
    except UnicodeDecodeError:
        s = data.decode("latin-1")
    return [line.rstrip() for line in s.split("\n")]


def main() -> int:
    old, new = Path(sys.argv[1]), Path(sys.argv[2])
    sub = sys.argv[3] if len(sys.argv) > 3 else "boost/numeric"
    rels = sorted({p.relative_to(old).as_posix() for p in (old / sub).rglob("*") if p.is_file()} |
                  {p.relative_to(new).as_posix() for p in (new / sub).rglob("*") if p.is_file()})
    total = 0
    for rel in rels:
        a, b = lines(old / rel), lines(new / rel)
        n = sum(1 for d in difflib.unified_diff(a, b, lineterm="", n=0)
                if d[:1] in "+-" and d[:3] not in ("+++", "---"))
        status = "added" if not a else "removed" if not b else "changed"
        if n:
            print(f"{n}\t{status}\t{rel}")
            total += n
    print(f"{total}\ttotal")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

### A.11 `scripts/patchstats.sh`

```sh
#!/bin/sh
# Hunks and changed lines per patch directory. Run from build/zdev-impl/boost-003.
for d in "$@"; do
  h=0; l=0; n=0
  for p in $d/*.patch; do
    n=$((n+1)); h=$((h + $(grep -c '^@@' $p))); l=$((l + $(grep -cE '^[-+]([^-+]|$)' $p)))
  done
  echo "$d patches=$n hunks=$h changed_lines=$l"
done
```

### A.12 `scripts/extra_targets.sh`

```sh
#!/bin/sh
# Platform headers that FreeBSD and Cygwin builds reach, and whether each copy has them.
# Same method as target_closure.py (-M -MG, no sysroot). Run from build/zdev-impl/boost-003.
mkdir -p tmp; echo '#include <gecode/third-party/boost/numeric/interval.hpp>' > tmp/tu.cpp
for v in head refresh improve; do
  for t in x86_64-unknown-freebsd14 x86_64-pc-cygwin; do
    d=$(/usr/bin/clang++ -std=c++23 -M -nostdinc -nostdinc++ -MG --target=$t -I src/$v tmp/tu.cpp 2>/dev/null |
        tr ' \\' '\n\n' | grep 'config/platform/' | sed 's|.*boost/config/||')
    ex=""
    for f in $d; do
      if [ -f src/$v/gecode/third-party/boost/config/$f ]; then ex="$ex $f:present"; else ex="$ex $f:MISSING"; fi
    done
    echo "$v $t:$ex"
  done
done
```

### A.13 `scripts/build.sh`

```sh
#!/bin/sh
# Configure and build one scratch variant: build.sh <src> <name> <Release|Debug> <ON|OFF mpfr> [targets...]
# Options match build/local except build type and MPFR. Run from build/zdev-impl/boost-003.
set -e
src=$1; name=$2; type=$3; mpfr=$4; shift 4
gen="Unix Makefiles"; [ "$type" = Debug ] && gen=Ninja
cmake -S src/$src -B bld/$name -G "$gen" -DCMAKE_BUILD_TYPE=$type \
  -DCMAKE_CXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/c++ \
  -DGECODE_ENABLE_MPFR=$mpfr -DGECODE_ENABLE_GIST=OFF > logs/$name.configure.log 2>&1
start=$(date +%s)
if [ $# -eq 0 ]; then
  cmake --build bld/$name -j 10 > logs/$name.build.log 2>&1
  cmake --build bld/$name --target gecode-test -j 10 >> logs/$name.build.log 2>&1
else
  cmake --build bld/$name -j 10 --target "$@" > logs/$name.build.log 2>&1
fi
echo "$name built in $(( $(date +%s) - start )) s; warnings: $(grep -c 'warning:' logs/$name.build.log)"
```

### A.14 `scripts/build-all.sh`

```sh
#!/bin/sh
# All scratch builds, one after another. Run from build/zdev-impl/boost-003.
scripts/build.sh head head-rel Release ON
scripts/build.sh refresh refresh-rel Release ON
scripts/build.sh improve improve-rel Release ON
scripts/build.sh refresh refresh-nompfr Release OFF
scripts/build.sh improve improve-nompfr Release OFF
scripts/build.sh head head-dbg Debug ON
scripts/build.sh head head-dbg-nompfr Debug OFF
echo ALL-DONE
```

### A.15 `scripts/behaviour.sh`

```sh
#!/bin/sh
# Behaviour checks for head-rel, refresh-rel, improve-rel. Run from build/zdev-impl/boost-003.
# 1. install each build; 2. rounding probe matrix; 3. DirectedRounding test;
# 4. float examples (solutions and statistics); 5. Float:: suite with 8 threads.
R=$(cd ../../.. && pwd)
V="head refresh improve"
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
mkdir -p inst results/probe results/examples results/suite
for v in $V; do
  cmake --install bld/$v-rel --prefix "$PWD/inst/$v" > logs/$v.install.log 2>&1
  rm -rf probe-$v; mkdir -p probe-$v
  cp "$R/build/rounding-probe/rounding-probe.cpp" "$R/build/rounding-probe/run.sh" probe-$v/
  ln -s "$PWD/inst/$v" probe-$v/install
  probe-$v/run.sh > results/probe/$v.log 2>&1
  bld/$v-rel/bin/gecode-test -test Float::Arithmetic::DirectedRounding > results/suite/$v-directed.log 2>&1
  echo "exit $?" >> results/suite/$v-directed.log
done
for v in $V local; do
  b=bld/$v-rel/bin; [ $v = local ] && b=$R/build/local/bin
  for e in $EX; do
    $b/$e -mode solution > results/examples/$e-$v.solution 2>&1
    $b/$e -mode stat > results/examples/$e-$v.stat 2>&1
  done
done
for v in $V; do
  bld/$v-rel/bin/gecode-test -test Float:: -stop false -threads 8 > results/suite/$v-t8.log 2>&1
  echo "exit $?" >> results/suite/$v-t8.log
done
echo BEHAVIOUR-DONE
```

### A.16 `scripts/refreshfix.sh`

```sh
#!/bin/sh
# Refresh with the upstream nth_root fix (b/out-fix): build after build-all.sh,
# then the same behaviour checks as behaviour.sh. Run from build/zdev-impl/boost-003.
R=$(cd ../../.. && pwd)
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
while ! grep -q ALL-DONE logs/build-all.log; do sleep 20; done
scripts/build.sh refreshfix refreshfix-rel Release ON
v=refreshfix
cmake --install bld/$v-rel --prefix "$PWD/inst/$v" > logs/$v.install.log 2>&1
rm -rf probe-$v; mkdir -p probe-$v
cp "$R/build/rounding-probe/rounding-probe.cpp" "$R/build/rounding-probe/run.sh" probe-$v/
ln -s "$PWD/inst/$v" probe-$v/install
probe-$v/run.sh > results/probe/$v.log 2>&1
bld/$v-rel/bin/gecode-test -test Float::Arithmetic::DirectedRounding > results/suite/$v-directed.log 2>&1
echo "exit $?" >> results/suite/$v-directed.log
for e in $EX; do
  bld/$v-rel/bin/$e -mode solution > results/examples/$e-$v.solution 2>&1
  bld/$v-rel/bin/$e -mode stat > results/examples/$e-$v.stat 2>&1
done
while ! grep -q BEHAVIOUR-DONE logs/behaviour.log; do sleep 20; done
bld/$v-rel/bin/gecode-test -test Float:: -stop false -threads 8 > results/suite/$v-t8.log 2>&1
echo "exit $?" >> results/suite/$v-t8.log
echo REFRESHFIX-DONE
```

### A.17 `scripts/compare-examples.sh`

```sh
#!/bin/bash
# Compare each variant's example output with build/local's, without runtime lines.
# Usage: compare-examples.sh <variant>... Run from build/zdev-impl/boost-003.
EX="archimedean-spiral cartesian-heart descartes-folium golden-spiral"
for v in "$@"; do
  for e in $EX; do
    for k in solution stat; do
      a=results/examples/$e-local.$k; b=results/examples/$e-$v.$k
      n=$(diff <(grep -v 'runtime:' $a) <(grep -v 'runtime:' $b) | grep -c '^[<>]')
      echo "$v $e $k differing_lines=$n"
    done
  done
done
for e in $EX; do printf '%s local: ' $e; grep -E 'solutions|propagations|nodes|failures' results/examples/$e-local.stat | tr -s ' \t' ' ' | tr '\n' ';'; echo; done
```

### A.18 `scripts/inventory.sh`

```sh
#!/bin/sh
# Symbol inventory of the vendored Boost code in one Debug (-O0) build:
# inventory.sh <build-name>. Run from build/zdev-impl/boost-003.
# Writes results/inv/<build>/<binary>.nm and <build>.txt (distinct functions).
b=$1; o=results/inv/$b; rm -rf $o; mkdir -p $o
grep -E '^CMAKE_BUILD_TYPE|^CMAKE_CXX_FLAGS_DEBUG|^GECODE_ENABLE_MPFR' bld/$b/CMakeCache.txt
for f in bld/$b/libgecode*.6.5.0.dylib bld/$b/bin/gecode-test \
         bld/$b/bin/archimedean-spiral bld/$b/bin/cartesian-heart bld/$b/bin/descartes-folium bld/$b/bin/golden-spiral; do
  [ -f "$f" ] || { echo "$(basename "$f") not built"; continue; }
  nm -C "$f" > $o/$(basename "$f").nm
  echo "$(basename "$f") $(grep -c 'gecode_boost::' $o/$(basename "$f").nm)"
done
python3 -I scripts/inventory.py $o/*.nm > results/inv/$b.txt
tail -1 results/inv/$b.txt
```

### A.19 `scripts/inventory.py`

```python
"""Distinct gecode_boost functions instantiated in binaries.

Usage: python3 -I inventory.py <nm-output>...

Each input is the output of `nm -C` on one binary. Keeps defined symbols
(any type letter except U) whose demangled name contains gecode_boost::,
removes template arguments and parameter lists, and prints each distinct
qualified name with the inputs it occurs in.
"""

import sys
from collections import defaultdict
from pathlib import Path


def strip_nested(s: str, open_c: str, close_c: str) -> str:
    out, depth = [], 0
    for c in s:
        if c == open_c:
            depth += 1
        elif c == close_c:
            depth -= 1
        elif depth == 0:
            out.append(c)
    return "".join(out)


def base_name(sym: str) -> str:
    # Drop the parameter list: the last top-level "(...)", keeping operator().
    s = sym.replace("operator()", "operator@CALL@")
    s = s.replace("operator<=", "operator@LE@").replace("operator>=", "operator@GE@")
    s = s.replace("operator<", "operator@LT@").replace("operator>", "operator@GT@")
    s = strip_nested(s, "<", ">")
    s = strip_nested(s, "(", ")")
    for a, b in (("@CALL@", "()"), ("@LE@", "<="), ("@GE@", ">="), ("@LT@", "<"), ("@GT@", ">")):
        s = s.replace(a, b)
    s = s.replace(" const", "").strip()
    # Return type: keep from the last space before the qualified name.
    if " " in s and not s.startswith("operator"):
        parts = s.split(" ")
        cand = [p for p in parts if "::" in p]
        s = cand[-1] if cand else parts[-1]
    return s


def main() -> int:
    seen: dict[str, set[str]] = defaultdict(set)
    for f in sys.argv[1:]:
        for line in Path(f).read_text(errors="replace").splitlines():
            parts = line.split(None, 2)
            if len(parts) < 3 or parts[1] in ("U",) or "gecode_boost::" not in parts[2]:
                continue
            name = base_name(parts[2])
            if "gecode_boost::" in name:
                seen[name].add(Path(f).stem)
    for name in sorted(seen):
        print(f"{name}\t{','.join(sorted(seen[name]))}")
    print(f"# distinct {len(seen)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

### A.20 `scripts/sloc.py`

```python
"""Source lines per definition in C++ headers.

Usage: python3 -I sloc.py <file>...

SLOC counts lines that are neither blank nor comment-only, after removing
comments. Each file is split into definitions: a run of lines from the end of
the previous definition up to a line where the brace depth returns to the
enclosing namespace level after having opened a brace, or to a `;` at that
level. Namespace braces do not count as definitions. Prints, per file, each
definition's first line, its name (the identifier before the first `(` or the
class/struct name), its SLOC, and two subsets of its SLOC: PLUMB, lines a
double-only implementation would not need (template heads, typedefs, the
`typename Policies::rounding rnd;` declaration), and CHECK, lines that test
for empty or invalid operands (test_input, checking::, is_empty). Then the
sum over definitions (TOTAL) and the file's SLOC including preprocessor
lines (FILE).
"""

import re
import sys
from pathlib import Path


def strip_comments(text: str) -> str:
    out, i, n = [], 0, len(text)
    while i < n:
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("\n" * text.count("\n", i, j))
            i = j
        elif text[i] in "\"'":
            q, j = text[i], i + 1
            while j < n and text[j] != q and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1])
            i = j + 1
        else:
            out.append(text[i])
            i += 1
    return "".join(out)


PLUMB = re.compile(r"^(template\s*<|typedef\b|typename\s+Policies::rounding\s+rnd\s*;|using\b)")
CHECK = re.compile(r"test_input|checking::|is_empty")
NAME = re.compile(r"(operator\s*\S+?|~?[A-Za-z_][A-Za-z0-9_]*)\s*\(")
CLASS = re.compile(r"\b(?:class|struct|union)\s+([A-Za-z_][A-Za-z0-9_]*)")


def definitions(path: Path):
    lines = strip_comments(path.read_bytes().decode("utf-8", "replace")).split("\n")
    ns_depth = 0  # braces opened by namespace or extern "C"
    depth = 0
    cur: list[tuple[int, str]] = []
    opened = False
    for no, line in enumerate(lines, 1):
        s = line.strip()
        if not s or s.startswith("#"):
            if s.startswith("#") and depth == ns_depth and not cur:
                continue
            if s and cur:
                cur.append((no, s))
            continue
        if depth == ns_depth and re.match(r'^(namespace\b[^{;]*|extern\s+"C"\s*)\{', s):
            ns_depth += s.count("{") - s.count("}")
            depth = ns_depth
            continue
        if depth == ns_depth and s.startswith("}") and not cur:
            ns_depth -= 1
            depth = ns_depth
            continue
        cur.append((no, s))
        depth += s.count("{") - s.count("}")
        opened = opened or "{" in s
        if depth == ns_depth and (opened or s.endswith(";")):
            text = " ".join(t for _, t in cur)
            m = CLASS.search(text.split("{")[0]) if not NAME.search(text.split("{")[0]) else None
            name = m.group(1) if m else (NAME.search(text).group(1) if NAME.search(text) else text[:40])
            plumb = sum(1 for _, t in cur if PLUMB.match(t))
            check = sum(1 for _, t in cur if CHECK.search(t))
            yield cur[0][0], name, len(cur), plumb, check
            cur, opened = [], False


def main() -> int:
    for f in sys.argv[1:]:
        total = 0
        for start, name, n, plumb, check in definitions(Path(f)):
            print(f"{f}:{start}\t{name}\t{n}\t{plumb}\t{check}")
            total += n
        text = strip_comments(Path(f).read_bytes().decode("utf-8", "replace"))
        nonblank = sum(1 for line in text.split("\n") if line.strip())
        print(f"{f}\tTOTAL\t{total}")
        print(f"{f}\tFILE\t{nonblank}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

### A.21 `scripts/groups.py`

```python
"""Sum sloc.py output by group: python3 -I groups.py <groups.txt> <sloc-output>

Prints, per group, the number of definitions, their SLOC, the PLUMB and
CHECK subsets, and SLOC - PLUMB - CHECK, then the totals with and without
the MPFR-only groups (names starting with mpfr_). Exits 1 if a pattern
matches no definition.
"""

import re
import sys
from collections import defaultdict


def main() -> int:
    rules = []
    for line in open(sys.argv[1]):
        if line.strip() and not line.startswith("#"):
            g, f, pat = line.rstrip("\n").split("\t")
            rules.append((g, f, re.compile(pat), [0]))
    sums = defaultdict(lambda: [0, 0, 0, 0])
    for line in open(sys.argv[2]):
        parts = line.rstrip("\n").split("\t")
        if len(parts) != 5:
            continue
        loc, name, n, plumb, check = parts
        f = loc.rsplit(":", 1)[0]
        for g, rf, pat, hits in rules:
            if f == rf and pat.fullmatch(name):
                s = sums[g]
                s[0] += 1; s[1] += int(n); s[2] += int(plumb); s[3] += int(check)
                hits[0] += 1
                break
    bad = [(g, f, p.pattern) for g, f, p, h in rules if h[0] == 0]
    total, core = [0, 0, 0, 0], [0, 0, 0, 0]
    print("group\tdefs\tsloc\tplumb\tcheck\trest")
    for g, s in sums.items():
        print(f"{g}\t{s[0]}\t{s[1]}\t{s[2]}\t{s[3]}\t{s[1] - s[2] - s[3]}")
        for t in (total,) if g.startswith("mpfr_") else (total, core):
            for i in range(4):
                t[i] += s[i]
    for label, t in (("total-without-mpfr", core), ("total", total)):
        print(f"{label}\t{t[0]}\t{t[1]}\t{t[2]}\t{t[3]}\t{t[1] - t[2] - t[3]}")
    for b in bad:
        print("no match", *b)
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
```

### A.22 `scripts/groups.txt`

```text
# group	file	definition names (regular expression, full match)
# Every Boost definition the inventory (nm, Debug builds) or FloatVal's
# source (val.hpp, num.hpp) reaches, grouped by what a support library needs.
class	interval.hpp	T|interval|operator=|assign|set|set_empty|set_whole|hull|empty|whole|lower|upper
compare	interval.hpp	comparison_error|operator<|operator>|operator<=|operator>=|operator==|operator!=
checking	checking.hpp	.*
checking	detail/test_input.hpp	.*
arith	arith.hpp	.*
arith	detail/division.hpp	.*
arith	arith2.hpp	multiplicative_inverse
sqrt_square	arith2.hpp	sqrt|square
pow_root	arith2.hpp	pow|pow_dn|pow_up|nth_root|root_aux|root_aux_dn|root_aux_up
utility	utility.hpp	width|median|empty|zero_in|in|subset|proper_subset|overlap|singleton|equal|intersect|hull|abs|GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION
constants	constants.hpp	.*
mpfr_fmod	arith2.hpp	fmod
mpfr_transc	transc.hpp	.*
rounding	rounding.hpp	.*
rounding	rounded_arith.hpp	.*
rounding	hw_rounding.hpp	.*
rounding	detail/c99_rounding_control.hpp	.*
rounding	detail/c99sub_rounding_control.hpp	.*
rounding	detail/msvc_rounding_control.hpp	.*
rounding	detail/x86_rounding_control.hpp	.*
```

### A.23 `probe/run-probes.sh`

```sh
#!/bin/sh
# Comparison and nth_root probes against HEAD's copy and each candidate copy.
# Run from build/zdev-impl/boost-003/probe after prepare.sh copies.
# r192np: 1.92.0 without patches (b/out0); r192: with patch 03 (b/out);
# r192fix: with patch 03 and upstream 6a59a02 (b/out-fix); r147: c/out.
W=$(cd .. && pwd)
for v in r192np:b/out0 r192:b/out r192fix:b/out-fix r147:c/out; do
  mkdir -p ovl-${v%%:*}/gecode/third-party
  ln -sfn "$W/${v#*:}" ovl-${v%%:*}/gecode/third-party/boost
done
./build-cmp.sh && ./build-nroot.sh || exit 1
for v in head r192np r192 r192fix r147; do
  ./cmp-$v > cmp-$v.txt
  if cmp -s cmp-head.txt cmp-$v.txt; then echo "cmp $v: same as head"; else echo "cmp $v: differs"; diff cmp-head.txt cmp-$v.txt; fi
done
for O in -O0 -O2; do
  for v in head r192np r192 r192fix r147; do
    ./nroot-$v$O > nroot-$v$O.txt; echo "nroot $v$O rc=$? $(tail -1 nroot-$v$O.txt)"
  done
done
```

### A.24 `probe/build-cmp.sh`

```sh
#!/bin/sh
# Build cmp.cpp header-only against HEAD and each vendored copy (overlay first on the include path).
# Run from build/zdev-impl/boost-003/probe.
set -e
CXX="/usr/bin/clang++ -std=c++23 -O2"
INC="-I ../src/head -I ../bld/head-rel -I /opt/homebrew/include"
$CXX $INC cmp.cpp -o cmp-head
for v in r192np r192 r192fix r147; do $CXX -I ovl-$v $INC cmp.cpp -o cmp-$v; done
```

### A.25 `probe/cmp.cpp`

```cpp
// Comparison semantics of FloatVal and of the interval type behind it.
// Prints one row per operand pair: the raw interval result (T, F, or X for
// comparison_error) and the FloatVal result (T or F), for < <= > >= == !=.
#include <gecode/float.hh>

#include <cmath>
#include <cstdio>

using Gecode::FloatNum;
using Gecode::FloatVal;
// The same type as the protected FloatVal::FloatValImpType.
using I = gecode_boost::numeric::interval<
    FloatNum, gecode_boost::numeric::interval_lib::policies<
                  Gecode::Float::Rounding, gecode_boost::numeric::interval_lib::checking_strict<FloatNum>>>;

namespace {

    template <class F>
    char raw(F f) {
        try {
            return f() ? 'T' : 'F';
        } catch (gecode_boost::numeric::interval_lib::comparison_error&) {
            return 'X';
        }
    }

    char tf(bool b) {
        return b ? 'T' : 'F';
    }

    void row_vv(const char* name, FloatNum xl, FloatNum xu, FloatNum yl, FloatNum yu) {
        I x(xl, xu), y(yl, yu);
        FloatVal a(xl, xu), b(yl, yu);
        std::printf("%-28s raw %c%c%c%c%c%c  FloatVal %c%c%c%c%c%c\n", name, raw([&] { return x < y; }),
                    raw([&] { return x <= y; }), raw([&] { return x > y; }), raw([&] { return x >= y; }),
                    raw([&] { return x == y; }), raw([&] { return x != y; }), tf(a < b), tf(a <= b), tf(a > b),
                    tf(a >= b), tf(a == b), tf(a != b));
    }

    void row_vn(const char* name, FloatNum xl, FloatNum xu, FloatNum y) {
        I x(xl, xu);
        FloatVal a(xl, xu);
        std::printf("%-28s raw %c%c%c%c%c%c  FloatVal %c%c%c%c%c%c  num-left %c%c%c%c%c%c\n", name,
                    raw([&] { return x < y; }), raw([&] { return x <= y; }), raw([&] { return x > y; }),
                    raw([&] { return x >= y; }), raw([&] { return x == y; }), raw([&] { return x != y; }),
                    tf(a < y), tf(a <= y), tf(a > y), tf(a >= y), tf(a == y), tf(a != y), tf(y < a), tf(y <= a),
                    tf(y > a), tf(y >= a), tf(y == a), tf(y != a));
    }

} // namespace

int main() {
    const FloatNum one_up = std::nextafter(1.0, 2.0);
    std::printf("columns: < <= > >= == !=\n");
    std::printf("interval-interval, x = [1,2] unless named\n");
    row_vv("y=[3,4] disjoint above", 1, 2, 3, 4);
    row_vv("y=[2,3] touching above", 1, 2, 2, 3);
    row_vv("y=[1.5,3] overlapping", 1, 2, 1.5, 3);
    row_vv("y=[1,2] equal bounds", 1, 2, 1, 2);
    row_vv("y=[1.2,1.8] inside", 1, 2, 1.2, 1.8);
    row_vv("y=[0,1] touching below", 1, 2, 0, 1);
    row_vv("y=[-1,0] disjoint below", 1, 2, -1, 0);
    row_vv("x=y=[1,1] same singleton", 1, 1, 1, 1);
    row_vv("x=[1,1] y=[2,2]", 1, 1, 2, 2);
    row_vv("x=[1,1] y=[1,2]", 1, 1, 1, 2);
    row_vv("x=y=[1,1+ulp] tight", 1, one_up, 1, one_up);
    std::printf("interval-number, x = [1,2] unless named\n");
    row_vn("y=0.5 below", 1, 2, 0.5);
    row_vn("y=1 lower bound", 1, 2, 1);
    row_vn("y=1.5 inside", 1, 2, 1.5);
    row_vn("y=2 upper bound", 1, 2, 2);
    row_vn("y=3 above", 1, 2, 3);
    row_vn("x=[1,1] y=1 singleton", 1, 1, 1);
    row_vn("x=[1,1+ulp] y=1 tight", 1, one_up, 1);
    row_vn("x=[1,1+ulp] y=1+ulp tight", 1, one_up, one_up);
    std::printf("empty interval\n");
    try {
        I e(2, 1);
        std::printf("I(2,1) constructed: [%g,%g]\n", e.lower(), e.upper());
    } catch (std::exception& ex) {
        std::printf("I(2,1) throws %s\n", ex.what());
    }
    try {
        FloatVal e(2, 1);
        std::printf("FloatVal(2,1) constructed: [%g,%g]\n", e.min(), e.max());
    } catch (std::exception& ex) {
        std::printf("FloatVal(2,1) throws %s\n", ex.what());
    }
    return 0;
}
```

### A.26 `probe/build-nroot.sh`

```sh
#!/bin/sh
# Build nroot.cpp header-only against HEAD and each vendored copy, at -O0 and -O2.
# Run from build/zdev-impl/boost-003/probe.
set -e
INC="-I ../src/head -I ../bld/head-rel -I /opt/homebrew/include"
LIB="-L /opt/homebrew/lib -lmpfr -lgmp"
for O in -O0 -O2; do
  /usr/bin/clang++ -std=c++23 $O $INC nroot.cpp -o nroot-head$O $LIB
  for v in r192np r192 r192fix r147; do /usr/bin/clang++ -std=c++23 $O -I ovl-$v $INC nroot.cpp -o nroot-$v$O $LIB; done
done
```

### A.27 `probe/build-nroot-all.sh`

```sh
#!/bin/sh
# All rows of the nth_root probe (-DPRINT_ALL, -O2) for HEAD and each copy; counts
# the rows whose bounds differ from HEAD's. Run from build/zdev-impl/boost-003/probe.
INC="-I ../src/head -I ../bld/head-rel -I /opt/homebrew/include"
LIB="-L /opt/homebrew/lib -lmpfr -lgmp"
/usr/bin/clang++ -std=c++23 -O2 -DPRINT_ALL $INC nroot.cpp -o nroot-all-head $LIB
./nroot-all-head > /dev/null 2> nroot-all-head.txt
for v in r192np r192 r192fix r147; do
  /usr/bin/clang++ -std=c++23 -O2 -DPRINT_ALL -I ovl-$v $INC nroot.cpp -o nroot-all-$v $LIB
  ./nroot-all-$v > /dev/null 2> nroot-all-$v.txt
  echo "$v rows $(wc -l < nroot-all-$v.txt) differing_from_head $(diff nroot-all-head.txt nroot-all-$v.txt | grep -c '^>')"
  diff nroot-all-head.txt nroot-all-$v.txt | grep '^>' | cut -d' ' -f2 | sort | uniq -c
done
```

### A.28 `probe/nroot.cpp`

```cpp
// Soundness of Gecode::nroot (Boost nth_root) and mode restoration.
// For each caller mode, operand x in (0,1) and above 1, and k in 2..5:
// compares nroot(FloatVal(x)) with MPFR's directed k-th roots and checks
// that the caller's mode is unchanged afterwards. Prints counts and the
// first unsound rows; exits 1 if any row is unsound or changes the mode.
// Built with -DPRINT_ALL it also prints every row's bounds on stderr.
#include <gecode/float.hh>

#include <mpfr.h>

#include <cfenv>
#include <cmath>
#include <cstdio>

namespace {

    double mpfr_root(double x, int k, mpfr_rnd_t r) {
        mpfr_t m;
        mpfr_init2(m, 53);
        mpfr_set_d(m, x, MPFR_RNDN);
        mpfr_rootn_ui(m, m, static_cast<unsigned long>(k), r);
        double d = mpfr_get_d(m, r);
        mpfr_clear(m);
        return d;
    }

} // namespace

int main() {
    const int modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    const char* names[] = {"nearest", "downward", "upward", "towardzero"};
    const double xs[] = {0.1, 0.2, 0.3, 0.37, 0.5, 0.7, 0.9, 0.99, 2.0, 3.0, 5.0, 7.0, 10.0, 1e-5, 1e5};
    int rows = 0, unsound = 0, changed = 0, printed = 0;
    for (int mi = 0; mi < 4; mi++) {
        for (double x : xs) {
            for (int k = 2; k <= 5; k++) {
                std::fesetround(modes[mi]);
                Gecode::FloatVal r = Gecode::nroot(Gecode::FloatVal(x), k);
                int after = std::fegetround();
                std::fesetround(FE_TONEAREST);
                double lo = mpfr_root(x, k, MPFR_RNDD), up = mpfr_root(x, k, MPFR_RNDU);
                bool bad = (r.min() > lo) || (r.max() < up);
                rows++;
#ifdef PRINT_ALL
                std::fprintf(stderr, "%s %a %d %a %a\n", names[mi], x, k, r.min(), r.max());
#endif
                unsound += bad;
                changed += (after != modes[mi]);
                if ((bad || after != modes[mi]) && printed < 12) {
                    printed++;
                    std::printf("%s x=%a k=%d got [%a,%a] exact [%a,%a]%s%s\n", names[mi], x, k, r.min(), r.max(),
                                lo, up, bad ? " UNSOUND" : "", after != modes[mi] ? " MODE-CHANGED" : "");
                }
            }
        }
    }
    std::printf("rows %d unsound %d mode_changed %d\n", rows, unsound, changed);
    return (unsound || changed) ? 1 : 0;
}
```

### A.29 `scripts/suites.sh`

```sh
#!/bin/sh
# Rerun the Float:: suite for the given variants, one after the other:
# suites.sh <variant>... Run from build/zdev-impl/boost-003. Used for improve
# and refreshfix after behaviour.sh hit the background time limit.
for v in "$@"; do
  bld/$v-rel/bin/gecode-test -test Float:: -stop false -threads 8 > results/suite/$v-t8.log 2>&1
  echo "exit $?" >> results/suite/$v-t8.log
done
echo SUITES-DONE
```

## Appendix B: patches

`a/patches` comes from `scripts/test-a.sh mkpatch`; patch 12 from
`prepare.sh copies`. Paths are relative to the copy's root.

### B.1 `a/patches/01-msvc-rounding-2012import.patch`

```diff
--- a/numeric/interval/detail/msvc_rounding_control.hpp
+++ b/numeric/interval/detail/msvc_rounding_control.hpp
@@ -25,7 +25,7 @@
 namespace interval_lib {
 namespace detail {
 
-#if GECODE_BOOST_MSVC < 1400 || defined(WIN64)
+#if GECODE_BOOST_MSVC < 1400 || defined(_WIN64)
 extern "C" { double rint(double); }
 #else
 inline double rint(double x)
@@ -88,7 +88,11 @@
   static void get_rounding_mode(rounding_mode& mode)
   { mode = msvc2hard(_control87(0, 0)); }
   static void set_rounding_mode(const rounding_mode mode)
+#if defined(_M_X64) || defined(_M_IA64)
+  { _control87(hard2msvc(mode), _MCW_EM | _MCW_RC ); }
+#else
   { _control87(hard2msvc(mode), _MCW_EM | _MCW_RC | _MCW_PC | _MCW_IC); }
+#endif
   static double to_int(const double& x) { return rint(x); }
 };
```

### B.2 `a/patches/03-interval-eq-3e448c0d5c.patch`

```diff
--- a/numeric/interval/interval.hpp
+++ b/numeric/interval/interval.hpp
@@ -364,7 +364,8 @@
 bool interval<T, Policies>::operator== (const interval_holder& r) const
 {
   if (!checking::is_empty(low, up)) {
-    if (up == r.low && low == r.up) return true;
+//Boost bug ?    if (up == r.low && low == r.up) return true;
+    if (up == r.up && low == r.low) return true;
     else if (up < r.low || low > r.up) return false;
   }
   throw interval_lib::comparison_error();
```

### B.3 `a/patches/04-rounded-arith-this-3e448c0d5c-87fa9d96fc.patch`

```diff
--- a/numeric/interval/rounded_arith.hpp
+++ b/numeric/interval/rounded_arith.hpp
@@ -68,8 +68,8 @@
   { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); GECODE_BOOST_DN(sqrt(x)); }
   T sqrt_up  (const T& x)
   { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); GECODE_BOOST_UP(sqrt(x)); }
-  T int_down(const T& x) { this->downward(); return to_int(x); }
-  T int_up  (const T& x) { this->upward();   return to_int(x); }
+  T int_down(const T& x) { this->downward(); return this->to_int(x); }
+  T int_up  (const T& x) { this->upward();   return this->to_int(x); }
 # undef GECODE_BOOST_DN
 # undef GECODE_BOOST_NR
 # undef GECODE_BOOST_UP
@@ -105,8 +105,8 @@
   { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); GECODE_BOOST_DN(sqrt(x)); }
   T sqrt_up  (const T& x)
   { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); GECODE_BOOST_UP(sqrt(x)); }
-  T int_down(const T& x) { return -to_int(-x); }
-  T int_up  (const T& x) { return to_int(x); }
+  T int_down(const T& x) { return -this->to_int(-x); }
+  T int_up  (const T& x) { return this->to_int(x); }
 # undef GECODE_BOOST_DN
 # undef GECODE_BOOST_NR
 # undef GECODE_BOOST_UP
```

### B.4 `a/patches/05-visualc-version-6e0efb1dec-13232ee695-4d14667bf7-90bb5ddd80.patch`

```diff
--- a/config/compiler/visualc.hpp
+++ b/config/compiler/visualc.hpp
@@ -273,7 +273,8 @@
 #endif
 //
 // last known and checked version is 1600 (VC10, aka 2010):
-#if (_MSC_VER > 1600)
+// Changed to VC15, aka 2017
+#if (_MSC_VER > 2000)
 #  if defined(GECODE_BOOST_ASSERT_CONFIG)
 #     error "Unknown compiler version - please run the configure tests and report the results"
 #  else
```

### B.5 `a/patches/06-libstdcpp3-threads-d78bb0b1d1.patch`

```diff
--- a/config/stdlib/libstdcpp3.hpp
+++ b/config/stdlib/libstdcpp3.hpp
@@ -33,7 +33,9 @@
 
 #ifdef __GLIBCXX__ // gcc 3.4 and greater:
 #  if defined(_GLIBCXX_HAVE_GTHR_DEFAULT) \
-        || defined(_GLIBCXX__PTHREADS)
+        || defined(_GLIBCXX__PTHREADS) \
+        || defined(_GLIBCXX_HAS_GTHREADS) \
+        || defined(_WIN32)
       //
       // If the std lib has thread support turned on, then turn it on in Boost
       // as well.  We do this because some gcc-3.4 std lib headers define _REENTANT
```

### B.6 `a/patches/07-compare-set-unused-3cb43dc1d6.patch`

```diff
--- a/numeric/interval/compare/set.hpp
+++ b/numeric/interval/compare/set.hpp
@@ -29,6 +29,7 @@
 template<class T, class Policies> inline
 bool operator<(const interval<T, Policies>& x, const T& y)
 {
+  (void)x; (void)y;
   throw comparison_error();
 }
 
@@ -41,6 +42,7 @@
 template<class T, class Policies> inline
 bool operator<=(const interval<T, Policies>& x, const T& y)
 {
+  (void)x; (void)y;
   throw comparison_error();
 }
 
@@ -53,6 +55,7 @@
 template<class T, class Policies> inline
 bool operator>(const interval<T, Policies>& x, const T& y)
 {
+  (void)x; (void)y;
   throw comparison_error();
 }
 
@@ -65,6 +68,7 @@
 template<class T, class Policies> inline
 bool operator>=(const interval<T, Policies>& x, const T& y)
 {
+  (void)x; (void)y;
   throw comparison_error();
 }
 
@@ -77,6 +81,7 @@
 template<class T, class Policies> inline
 bool operator==(const interval<T, Policies>& x, const T& y)
 {
+  (void)x; (void)y;
   throw comparison_error();
 }
 
@@ -89,6 +94,7 @@
 template<class T, class Policies> inline
 bool operator!=(const interval<T, Policies>& x, const T& y)
 {
+  (void)x; (void)y;
   throw comparison_error();
 }
```

### B.7 `a/patches/08-abi-unrenamed-f813edf841.patch`

```diff
--- a/config/abi_prefix.hpp
+++ b/config/abi_prefix.hpp
@@ -12,7 +12,7 @@
 # error double inclusion of header boost/config/abi_prefix.hpp is an error
 #endif
 
-#include <gecode/third-party/boost/config.hpp>
+#include <boost/config.hpp>
 
 // this must occur after all other includes and before any code appears:
 #ifdef GECODE_BOOST_HAS_ABI_HEADERS
--- a/config/abi_suffix.hpp
+++ b/config/abi_suffix.hpp
@@ -7,7 +7,7 @@
 // http://www.boost.org/LICENSE_1_0.txt).
 
 // This header should be #included AFTER code that was preceded by a #include
-// <gecode/third-party/boost/config/abi_prefix.hpp>.
+// <boost/config/abi_prefix.hpp>.
 
 #ifndef GECODE_BOOST_CONFIG_ABI_PREFIX_HPP
 # error Header boost/config/abi_suffix.hpp must only be used after boost/config/abi_prefix.hpp
```

### B.8 `a/patches/09-hw-rounding-c36470ad8e-cde7a6c1cf.patch`

```diff
--- a/numeric/interval/hw_rounding.hpp
+++ b/numeric/interval/hw_rounding.hpp
@@ -16,13 +16,19 @@
 
 #define GECODE_BOOST_NUMERIC_INTERVAL_NO_HARDWARE
 
+#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (defined(__cplusplus) && __cplusplus >= 201103L)
+#  define GECODE_BOOST_C99_ENABLED
+#endif
+
 // define appropriate specialization of rounding_control for built-in types
-#if defined(__x86_64__) && (defined(__USE_ISOC99) || defined(__APPLE__))
+#if defined(__x86_64__) && (defined(GECODE_BOOST_C99_ENABLED) || defined(__APPLE__))
 #  include <gecode/third-party/boost/numeric/interval/detail/c99_rounding_control.hpp>
 #elif defined(__i386__) || defined(_M_IX86) || defined(__BORLANDC__) || defined(_M_X64)
 #  include <gecode/third-party/boost/numeric/interval/detail/x86_rounding_control.hpp>
 #elif defined(powerpc) || defined(__powerpc__) || defined(__ppc__)
+#ifndef __NO_FPRS__
 #  include <gecode/third-party/boost/numeric/interval/detail/ppc_rounding_control.hpp>
+#endif
 #elif defined(sparc) || defined(__sparc__)
 #  include <gecode/third-party/boost/numeric/interval/detail/sparc_rounding_control.hpp>
 #elif defined(alpha) || defined(__alpha__)
@@ -31,7 +37,7 @@
 #  include <gecode/third-party/boost/numeric/interval/detail/ia64_rounding_control.hpp>
 #endif
 
-#if defined(GECODE_BOOST_NUMERIC_INTERVAL_NO_HARDWARE) && (defined(__USE_ISOC99) || defined(__MSL__))
+#if defined(GECODE_BOOST_NUMERIC_INTERVAL_NO_HARDWARE) && (defined(GECODE_BOOST_C99_ENABLED) || defined(__MSL__))
 #  include <gecode/third-party/boost/numeric/interval/detail/c99_rounding_control.hpp>
 #endif
 
@@ -40,6 +46,8 @@
 #  error Boost.Numeric.Interval: Please specify rounding control mechanism.
 #endif
 
+#undef GECODE_BOOST_C99_ENABLED
+
 namespace gecode_boost {
 namespace numeric {
 namespace interval_lib {
```

### B.9 `a/patches/10-utility-unused-typedef-14857e39d9.patch`

```diff
--- a/numeric/interval/utility.hpp
+++ b/numeric/interval/utility.hpp
@@ -248,7 +248,6 @@
 template<class T, class Policies> inline
 T norm(const interval<T, Policies>& x)
 {
-  typedef interval<T, Policies> I;
   if (interval_lib::detail::test_input(x)) {
     typedef typename Policies::checking checking;
     return checking::nan();
```

### B.10 `a/patches/11-ppc-long-long-f753986df1-be7f5d7d1c.patch`

```diff
--- a/numeric/interval/detail/ppc_rounding_control.hpp
+++ b/numeric/interval/detail/ppc_rounding_control.hpp
@@ -28,10 +28,18 @@
   double dmode;
 } rounding_mode_struct;
 
-static const rounding_mode_struct mode_upward      = { 0xFFF8000000000002LL };
-static const rounding_mode_struct mode_downward    = { 0xFFF8000000000003LL };
-static const rounding_mode_struct mode_to_nearest  = { 0xFFF8000000000000LL };
-static const rounding_mode_struct mode_toward_zero = { 0xFFF8000000000001LL };
+static const rounding_mode_struct mode_upward      = { 
+  static_cast< ::gecode_boost::long_long_type >(0xFFF8000000000002LL) 
+};
+static const rounding_mode_struct mode_downward    = { 
+  static_cast< ::gecode_boost::long_long_type >(0xFFF8000000000003LL) 
+};
+static const rounding_mode_struct mode_to_nearest  = { 
+  static_cast< ::gecode_boost::long_long_type >(0xFFF8000000000000LL) 
+};
+static const rounding_mode_struct mode_toward_zero = { 
+  static_cast< ::gecode_boost::long_long_type >(0xFFF8000000000001LL) 
+};
 
 struct ppc_rounding_control
 {
```

### B.11 `b/patches-fix/12-nth-root-rounding-upstream-6a59a02.patch`

```diff
--- a/numeric/interval/arith2.hpp
+++ b/numeric/interval/arith2.hpp
@@ -273,6 +273,7 @@ interval<T, Policies> nth_root(interval<T, Policies> const &x, int k)
   if (interval_lib::detail::test_input(x)) return I::empty();
   assert(k > 0);
   if (k == 1) return x;
+  typename Policies::rounding rnd;
   typedef typename interval_lib::unprotect<I>::type R;
   if (!interval_lib::user::is_pos(x.upper())) {
     if (interval_lib::user::is_zero(x.upper())) {
```
