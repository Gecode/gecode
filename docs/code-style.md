# Code style

Use clang-format **22** for first-party C and C++ sources. `.clang-format`
defines the exact formatting; `.editorconfig` supplies the matching editor
defaults. Keep upstream third-party files in their original style.

## Formatting

C++ uses four-space indentation and a 120-column limit. Braces stay on the
opening line, compact nested namespaces remain indented, and pointer and
reference markers bind to the type. Function definitions retain a separate
return-type line. Include order and comment wrapping are preserved. Comment
indentation follows the surrounding code, including copyright notices and
Doxygen text.

Four spaces distinguish nested control flow. The 120-column limit leaves room
for namespace-qualified calls, templates, and generated scheduling expressions.
A comparison across six representative files produced 9,411 lines at 100 columns
and 9,299 at 120, versus 9,665 at 80. Only 146 additional lines exceeded 100
columns at 120, so the wider limit mainly avoids awkward wrapping. Continuation
indentation stays at four spaces; comments retain their existing wrapping.

Python uses four spaces; CMake and YAML use two. Editor settings belong in
`.editorconfig` rather than per-file indentation headers.

## Generated files

The variable-header generator formats its output with clang-format 22. Set
`CLANG_FORMAT` to its executable, or put `clang-format-22` or `clang-format`
on `PATH`. Builds using the checked-in headers do not require this tool.
Apply changes to the generator or its inputs, regenerate the outputs, and
check freshness. Formatting the checked-in outputs alone would make the next
regeneration undo the change.

The generated FlatZinc parser and lexer retain Bison and Flex formatting;
their upstream scaffolding is excluded from clang-format. Regenerate these
files from `.yxx` and `.lxx` inputs rather than editing the output by hand.

## Naming

Functions, variables, and members use `snake_case`. Types retain `PascalCase`;
enum constants and macros retain `UPPER_CASE`. Existing private-member leading
or trailing underscores are accepted when the name itself is snake_case.

Constants may use `UPPER_CASE`, and so may functions that stand for constant
values. Modification events and propagation conditions (`ME_INT_VAL`,
`PC_INT_DOM`), propagator status helpers (`ES_SUBSUMED`, `ES_FIX_PARTIAL`),
and branching selectors (`INT_VAR_SIZE_MIN()`, `INT_VAL_MIN()`) belong to the
same vocabulary as the enum constants they appear with, such as `ES_FIX` and
`IRT_EQ`, and keep their spelling.
Names should separate words and describe their purpose. A casing check cannot
identify word boundaries in names such as `dfsengine` and `modeventdelta`;
those require explicit review.

Representative API changes include:

| Previous spelling | Current spelling |
|---|---|
| `IRT_LE`, `FRT_LE`, `SRT_LE` | `IRT_LT`, `FRT_LT`, `SRT_LT` |
| `nooverlap(...)` | `no_overlap(...)` |
| `binpacking(...)` | `bin_packing(...)` |
| `modeventdelta(...)` | `modification_event_delta(...)` |
| `dfsengine(...)` | `make_dfs_engine(...)` |
| `clearall(value)` | `fill_bits(value)` |
| `arg2intvarargs(...)` | `arg_to_int_var_args(...)` |

Strict-less relation constants use `_LT`; `_LQ` continues to mean less-or-equal.
The former `_LE` names have no compatibility aliases.

Qt overrides and other external interfaces retain the spellings their APIs
require. Parser tokens and protocol strings remain unchanged.

## Checking changes

Put LLVM 22 tools on `PATH`, or set `CLANG_FORMAT` and `CLANG_TIDY` to their
executables. The checker rejects other major versions. Use:

```sh
uv run misc/check.py format
uv run misc/check.py format-check
uv run misc/check.py lint --build-dir build/local
uv run misc/check.py all --build-dir build/local --jobs 4
```

CMake provides `format`, `check-format`, `check-lint`, and `check-extended`
targets for the same commands. Formatting checks need no configured build.
Lint requires a Ninja or Makefiles compilation database configured with
`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`; it checks compiled sources and their
maintained headers. The command reports sources outside that build so optional
modules cannot silently disappear from the coverage report.
Bison/Flex grammar syntax is reviewed manually; clang-tidy checks the owned C++
action ranges in their generated translation units.

The extended command also checks the changelog and generated-source freshness,
builds the configured targets, runs CTest integrity and enabled fault suites,
installs into `check-install` under the build directory, and builds, tests, and lints the
allocator and public-test-component package consumers. It also builds the
documentation (Doxygen 1.18.0 or newer). The full `check-standard` and
`check-sweep` suites remain separate targets: the standard suite alone contains
over 54,000 cases and defaults to five iterations per case.
Besides LLVM 22, the extended check requires uv, Bison 3.8.2, Flex 2.6.4, Ninja,
and Doxygen 1.18.0 or newer. Configure with `BUILD_TESTING`, `GECODE_INSTALL`,
`GECODE_ENABLE_ALLOCATOR`, search, integer variables, minimodel, and driver
enabled. Lint always analyzes assertions by undefining `NDEBUG`, regardless of
the selected build type. Ordinary compilation and runtime tests retain the
configured build settings.
Enable optional and experimental modules in the build when checking them;
platform-specific code also needs its corresponding CI toolchain. Normal
builds use checked-in generated sources and do not require LLVM tools.

Performance checks cover hidden loop copies, associative-container lookup,
vector growth, string operations, and missed moves as well as unnecessary
copies. Use `'\n'` inside a formatted record. Flush explicitly when publishing
a solution, progress update, protocol response, or complete diagnostic; a
printer should not flush each row of a multi-line solution.
The unused-return-value check accepts explicit `(void)` discards for reviewed
operations such as ownership handoff after successful engine construction.

`performance-unnecessary-value-param` is reserved for manual performance audits.
Its suggestions mix Gecode's small, intentionally copied variable/view handles
with ownership-sensitive callbacks and argument arrays. Review actual copy
cost and lifetime requirements before changing those public signatures.

Other performance checks need focused review: `performance-enum-size` changes
layouts and ABI, and `performance-no-int-to-ptr` conflicts with deliberate
pointer tagging. Review `noexcept` guarantees, destructor triviality, and math
promotion against allocation-failure handling and interval-rounding semantics
before adopting those checks. Never add `noexcept` to a potentially throwing
operation just to satisfy a diagnostic.
