# Code style

Use clang-format **22** for first-party C and C++ sources. `.clang-format`
defines the exact formatting; `.editorconfig` supplies the matching editor
defaults. Keep upstream third-party files in their original style.

## Formatting

C++ uses four-space indentation and a 120-column limit. Braces stay on the
opening line, compact nested namespaces remain indented, and pointer and
reference markers bind to the type. Function definitions retain a separate
return-type line. Include order and comment wrapping are preserved, including
copyright notices and Doxygen text.

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

## Naming migration

The naming migration is pending a decision on which identifier categories
change, including types and namespaces. Names should separate words with
underscores and describe their purpose. A casing check cannot identify word
boundaries in existing names such as `dfsengine` and `modeventdelta`; those
need explicit review.

Changing every category requires resolving collisions before applying fixes.
For example, `Int` cannot become the keyword `int`, and the `DFS` class
template cannot take the existing `dfs` function's name. Review those choices
alongside the public API, generated code, and documentation.

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

The extended command also checks the changelog and generated-source freshness,
builds the configured targets, runs the standard test suite, and runs CTest.
Enable optional and experimental modules in the build when checking them;
platform-specific code also needs its corresponding CI toolchain. Normal
builds use checked-in generated sources and do not require LLVM tools.
