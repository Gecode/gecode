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

`performance-unnecessary-value-param` reports parameters that are copied but
only read. Take such a parameter by `const&`. When the callee keeps the value
and the type has a real move constructor, such as `std::function`,
`std::string`, `std::vector`, or a branching configuration, take it by value
and `std::move` it into place instead.

Gecode's shallow handles are trivially copyable, so the check never reports
them: `Home`, the groups, `Pos`, `Reify`, the variables (`IntVar` and so
on), the variable arrays and `ViewArray`, the open variable and view lists,
`LocalHandle`, `AFC`, and the views. Copying one copies a few pointers or ids,
with no reference count or allocation, and Gecode code relies on their value
semantics. Keep them so: define no copy member, move member, or destructor
for a handle that only copies its members, or default it on its first
declaration. `test/array.cpp` asserts the property. The check's
`AllowedTypes` lists only `Float::ScaleView`, a view that is not trivially
copyable because its `FloatVal` member is not. Owned and reference-counted
types are not exempt: `SharedHandle` types such as `IntSet`, `TupleSet`,
`DFA`, `Rnd`, `Action`, and `CHB`, `SharedArray`, the `*Args` arrays, `REG`,
strings, and callbacks. Their copies touch a reference count or allocate, so
they follow the rule above. `FloatVal` is not exempt either: it is two
doubles, not a handle.

Special members follow the rule of zero: a class whose members copy, move,
and destroy themselves declares no copy member, move member, or destructor.
Where a special member must be declared, for example a default constructor
beside other constructors or a virtual destructor, write `= default` instead
of an empty body or a member-wise copy, on the member's first declaration in
the class. A member defaulted out of line is user-provided and keeps the
class non-trivial. Virtual destructors may stay out of line: their class is
never trivial, and one defined in a .cpp file decides where the vtable is
emitted. A member that must not be called is a public `= delete`, not a
private member without a definition. `modernize-use-equals-default` and
`modernize-use-equals-delete` enforce these rules.

Some hand-written members stay, each after a
`// NOLINTNEXTLINE(modernize-use-equals-default)` line. Cloning constructors
such as `Space(Space&)` are not copy members: they take their source by
non-const reference because cloning writes forwarding pointers into it, and
they update their variables with `update()`. An empty default constructor
stays where defaulting it would change what value-initialization does:
`T()`, `T{}`, and the `alloc<T>(n)` functions of spaces, regions, and the
heap zero a class whose default constructor is not user-provided, while the
empty constructor leaves fields without an initializer untouched. A
constructor whose body is empty only in some configurations stays as well.

Comparisons use the C++20 forms. A class whose equality compares every field
declares `bool operator==(const T&) const = default;`. A class ordered
lexicographically by its fields in declaration order declares
`auto operator<=>(const T&) const = default;` instead, which also provides
`==`; its header includes `<compare>`. Declare no `operator!=` and no `<`,
`<=`, `>`, or `>=` beside them: the compiler rewrites `a != b` as
`!(a == b)` and the relational operators through `operator<=>`, in either
argument order. A hand-written `operator==` stays where equality is not
field-wise, for example a deep comparison of a shared object or a comparison
of the address an allocator refers to, and it gets no `operator!=` either. A
hand-written `operator<` stays where the order is not the field order, such
as a sort key. Operators that build modelling expressions, such as
`IntVar < IntVar`, keep all their operators, and so do the possibly/certainly
interval comparisons of `FloatVal`. The kernel's heterogeneous `ConstView`,
`VarImpView`, and `DerivedView` templates keep `operator!=`: without it, their
reversed `operator==` candidates would join every view comparison. A class
with both an implicit converting constructor and an implicit conversion
operator keeps `operator!=` as well. `TupleSet` is one: without its
`operator!=`, `true != t` is ambiguous between its `int` constructor and its
`bool` conversion. Rewriting
covers only operator expressions, so write `a != b`, not `operator!=(a, b)` or
`a.operator<(b)`. No clang-tidy check enforces this rule.

Other performance checks need focused review: `performance-enum-size` changes
layouts and ABI, and `performance-no-int-to-ptr` conflicts with deliberate
pointer tagging. Review `noexcept` guarantees, destructor triviality, and math
promotion against allocation-failure handling and interval-rounding semantics
before adopting those checks. Never add `noexcept` to a potentially throwing
operation just to satisfy a diagnostic.
