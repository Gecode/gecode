# Origin and Variant terminology inventory

This inventory retains the result of a case-insensitive, whole-word scan for
the two Gecode 6 meta-search role terms, including their plurals. The verifier
walks the entire repository. It excludes only `.git/`, `.zd/`,
`autom4te.cache/`, directories whose names begin with `build`, binary files,
and the manifest itself. Root build files are therefore in scope.

`test/search/verify-origin-variant-terminology.cpp` builds a standalone
verifier for the Autoconf and CMake `check` targets. The occurrence-level
manifest is
[`test/search/origin-variant-terminology.expected`](../test/search/origin-variant-terminology.expected).
Each tab-separated row records `path:line:column`, the exact spelling, a
classification, and the escaped complete source line as its excerpt. Thus
two terms on one line are two independently classified records. A new,
removed, moved, respelled, or contextually changed occurrence fails the
check.

Run the exact checked scan through either build surface:

```sh
make verify-origin-variant-terminology
cmake --build BUILD_DIRECTORY --target verify-origin-variant-terminology
```

To print the whole-repository occurrence list in manifest format, with each
classification marked `UNCLASSIFIED`, run:

```sh
BUILD_DIRECTORY/bin/gecode-verify-origin-variant-terminology REPOSITORY_ROOT -
```

## Classification guide

The manifest contains one row per occurrence. These groups explain why its
classifications are retained:

| Files | Classification |
| --- | --- | --- |
| `CMakeLists.txt`, `Makefile.in`, `test/search/origin-variant-api-fail.cpp` | Intentional negative API validation. The declarations must fail to compile. |
| `examples/origin-variant.cpp`, `docs/gecode-7-migration.md` | Historical migration mapping. |
| `docs/origin-variant-terminology.md` | Rejected terminology retained as design rationale. |
| `changelog.in` | Historical release record, not current API documentation. |
| `examples/scowl.hpp` | Third-party word-list data and licence. |
| `examples/perfect-square.cpp` | Unrelated geometric domain language. |
| `gecode/support/run-jobs.hpp` | Unrelated internal scheduler topology. |
| `gecode/int/cumulative/tree.hpp` | Bibliographic title. |

The checked scan contains no current RBS/PBS identifier or descriptive prose
using either removed role term. A source change that adds one must either fix
the stale terminology or update this inventory and the checked manifest with a
specific unrelated or historical classification.
