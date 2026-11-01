# (void) parameter-list cleanup

## Objective

C++ code no longer uses C-style `(void)` parameter lists. The change comes
from a reproducible tool run and is guarded by a lint check, so a later
history rewrite reruns the tool instead of resolving thousands of conflicts by
hand. `(void)` parameter lists occur on about 5,700 lines at the area base.

Success:

- `(void)` parameter lists are gone from maintained C++ code, including code
  that generators emit. `modernize-redundant-void-arg` is enabled in
  `.clang-tidy`, and `uv run misc/check.py lint` passes.
- Casts to `void` that discard a value, such as `(void)new (p) T()`, are
  untouched.
- `docs/code-style.md` states the rule. It also says that `forceinline` stays
  lowercase so that it reads like `inline`.
- The mass edit is one commit. Its message gives the exact commands that made
  it, so the commit can be regenerated.

## Boundaries and non-goals

- Owned: every maintained C++ file, `misc/genvarimp.py` and the other
  generators, the Bison and Flex inputs, `.clang-tidy`, `docs/code-style.md`,
  and `changelog.in`.
- `gecode/third-party` is excluded. C sources keep `(void)`, because there it
  means something different.
- `forceinline` keeps its name and its uses. The user chose the lowercase
  spelling so that it reads like `inline`.

## Terms and settled decisions

- This area runs last, after the `config-header`, `empty-arrays`,
  `value-params`, `boost`, and `scoped-enums` work is committed, so the mass
  edit covers their code too. Do not hand-merge the mass edit; rerun the
  tool.
- Run the formatter after the tool run, because shorter declarations can
  reflow.

## Testing

Existing checks only. The change is mechanical and keeps behaviour, and the
build, the lint check, and the integrity suite cover it.

- Rebuild the generated sources with the `regenerate` target, then run
  `.github/ci/generated-source-freshness.sh`. This confirms that the
  generators emit the new form.

## Validation

- `uv run misc/check.py all --build-dir <debug-full-dir> --jobs N` on a Debug
  build with experimental, CBS, fault injection, audit, and Gist.
- Build the default Release configuration and run `ctest`.
