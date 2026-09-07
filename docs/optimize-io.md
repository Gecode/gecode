# Optimization Model Exchange

`Gecode::Optimize::read_model` and `write_model` support a linear LP/free-MPS
subset independently of the numerical backend. They preserve the model's
`double` values. Exact rational parsing, compressed files, and vendor extensions
outside the syntax below are unsupported.

## Import

Only `.lp` and `.mps` suffixes are accepted, case-insensitively. Unreadable files,
malformed records, NaN, coefficient infinities, numeric overflow or underflow,
and unsupported constructs produce `ModelError`. Tiny nonzero coefficients are
preserved during import; a solving backend may reject unsupported magnitudes.

Identifiers begin with an ASCII letter or underscore and continue with letters,
digits, or underscores. Fixed-column MPS files with ordinary whitespace-separated
identifiers are supported. Names with embedded spaces, quoted names, omitted
repeated column names, and SIF extensions are unsupported.

### LP Syntax

- One `Minimize` or `Maximize` objective, a `Subject To` section, optional `Bounds`,
  `Binary`, `General`, and `Semi` sections, and a required `End`. Common aliases
  include `min`, `max`, `st`, `binaries`, `generals`, and `semi-continuous`.
- Signed linear terms, decimal or scientific numbers, repeated terms combined
  additively, objective offsets, and constants on constraint left sides.
  Coefficients and variable names require separating whitespace. Constant
  accumulation retains compensation through right-side subtraction before
  conversion to the final `double` bound.
- One `<=`, `>=`, or `=` comparison with a numeric right side per constraint.
  Objectives can wrap across lines. Constraints can wrap before their comparison
  or right side; complete constraints must start separate lines. Ranged
  expressions with multiple comparisons are unsupported.
- Bounds `lower <= x <= upper`, `x <= upper`, `x >= lower`, `x = value`,
  `lower <= x`, and `x free`, with signed infinity where appropriate. Repeated
  definitions of the same bound side are rejected. Undeclared variables are
  continuous on `[0,+infinity)`.
- Semi-continuous variables and semi-integer variables declared in both `General`
  and `Semi`. Semi domains are `{0} union [lower,upper]`; the nonzero interval
  requires a strictly positive finite lower bound.
- Backslash comments. SOS, indicators, quadratic expressions, piecewise-linear
  sections, and strict comparisons are unsupported.

### MPS Syntax

- `NAME`, optional `OBJSENSE` on the same or next line, `ROWS`, `COLUMNS`, optional
  `RHS`, `RANGES`, and `BOUNDS`, and required `ENDATA`.
- The first `N` row is the objective; later `N` rows remain free constraints.
  `L`, `G`, and `E` rows, one or two row/value pairs per `COLUMNS`, `RHS`, or
  `RANGES` record, decimal or scientific values, and `D` exponents are supported.
- Integer markers `INTORG` and `INTEND`. A marker-only integer column with no
  explicit bounds defaults to `[0,1]`; the writer emits explicit bounds.
- Bound types `LO`, `UP`, `FX`, `FR`, `MI`, `PL`, `BV`, `LI`, `UI`, `SC`, and `SI`.
  Ambiguous overlapping bounds and conflicting type declarations are rejected.
  `BV` sets both bounds and cannot be combined with other bounds for that variable.
  `SC` on an already integral column is rejected; use `SI` to preserve integrality.
  `SI` accepts an integer marker or preceding `LI`, but cannot be combined with
  `BV` or `SC`. Ordinary `LO` with `SC` or `SI` is supported.
- One right-side vector, one range vector, and one bound vector. Multiple vectors,
  unknown row or column references, duplicate right-side or range entries,
  unsupported sections, SOS, quadratic data, and indicators are rejected.

## Export

The writer uses enough significant digits to preserve each `double` value.
Canonical names such as `x0`, `x1`, `r0`, and `r1` avoid identifier collisions and
syntax ambiguities. Original display names are preserved in hex-encoded
`GECODE_NAME` comments and restored by this reader. Other readers use the
canonical names. Malformed metadata and unknown entity references are rejected.

LP ranged rows are written as two inequalities and a `GECODE_RANGE` comment.
On reimport, both expressions and their bounds must match before they are
recombined. Other readers see the equivalent pair of inequalities. MPS uses
`RANGES`; export fails if the range width cannot be represented and reread
without changing the bounds. LP's paired inequalities support that case.

MPS semi-variable export requires a finite upper bound; LP supports an infinite
upper bound. MPS binary export requires exactly `[0,1]` bounds and emits a single
`BV` record. Tighter or fixed binary bounds require LP. Active original indicators
and globals cannot be exported by these linear formats and are rejected.

## Destination Handling

Export validates and compacts the source, writes an exclusive temporary file in
the destination directory, then checks write, flush, and close operations. It
reimports the temporary file and compares every active variable type, bound and
display name, every row and its coefficients, and the objective sense, terms and
offset. Only an exact numerical match permits same-directory atomic replacement.

The existing destination remains unchanged if validation, serialization,
reimport, comparison, writing, or replacement fails. Temporary files are removed
on ordinary failure paths. Replacement uses `rename` on POSIX and `MoveFileExW`
with `MOVEFILE_REPLACE_EXISTING` on Windows. Atomic visibility does not guarantee
power-loss durability. The old destination's inode, permissions, and ACLs are
not retained.

## Tests

After building as described in the [optimization guide](optimize.md), run:

```sh
ctest --test-dir build/optimize -C Release --output-on-failure -R '^optimize-io'
```

The tests cover precision, domains, repeated terms, constants, malformed inputs,
semantic round trips, and preservation of the destination after export failures.
