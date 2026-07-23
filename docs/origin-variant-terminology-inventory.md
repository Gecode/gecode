# Origin and Variant terminology inventory

This inventory retains the result of a case-insensitive, whole-word scan for
the two Gecode 6 meta-search role terms, including their plurals. The checked
scope is `gecode/`, `test/`, `examples/`, `docs/`, and `changelog.in`.
Generated build trees, Git data, and local zdev state are outside that scope.

`test/search/verify-origin-variant-terminology.cpp` builds a standalone
verifier for the Autoconf and CMake `check` targets. It compares `path:line`
results with
`test/search/origin-variant-terminology.expected`; a new occurrence or a
removed expected occurrence fails the check.

Run the gate directly with either build surface:

```sh
make verify-origin-variant-terminology
cmake --build BUILD_DIRECTORY --target verify-origin-variant-terminology
```

## Classified occurrences

| Files | Exact excerpt | Classification |
| --- | --- | --- |
| `test/search/origin-variant-api-fail.cpp` | `bool mast&#101;r(...) override`, `bool slav&#101;(...) override` | Intentional negative API fixture. These declarations must fail to compile. |
| `examples/origin-variant.cpp`, `docs/gecode-7-migration.md` | Gecode 6 `mast&#101;r` to `origin` and `slav&#101;` to `variant` declarations | Historical migration material. |
| `docs/origin-variant-terminology.md` | `Mast&#101;r / Slav&#101;` | Explicitly rejected terminology retained as design rationale. |
| `changelog.in` | Historical callback names and descriptions, including singular and plural role nouns | Historical release record; changelog wording is not current API documentation. |
| `examples/scowl.hpp` | Dictionary entries for both role nouns and their plurals; ENABLE word-list licence text | Third-party word-list data and licence. |
| `examples/perfect-square.cpp` | `mast&#101;r square` | Unrelated geometric domain language. |
| `gecode/support/run-jobs.hpp` | `RunJobs::Mast&#101;r`, its `mast&#101;r` pointer, and scheduler comments | Unrelated internal scheduler topology. |
| `gecode/int/cumulative/tree.hpp` | `Mast&#101;r Thesis` | Bibliographic title. |

The checked scan contains no current RBS/PBS identifier or descriptive prose
using either removed role term. A source change that adds one must either fix
the stale terminology or update this inventory and the checked manifest with a
specific unrelated or historical classification.
