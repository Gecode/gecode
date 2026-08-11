+++
schema_version = 1
id = "word-001"
key = "kernel"
area = "word"
status = "done"
blocked_by = []
+++
# Add a buildable clone-safe WordVar kernel

## Outcome

Gecode provides an optional GecodeWord component whose 1-64 bit WordVar domains use valid lo/hi masks and support construction, narrowing, assignment, arrays, printing, cloning, and recomputation.

## Boundaries

- Do not add nontrivial word constraints, search, MiniModel expressions, or FlatZinc support.
- Wire generated variable specifications and enabled/disabled builds only far enough to make the variable kernel usable; the full packaging matrix belongs to release engineering.
- Follow the established Int, Set, and Float variable implementation patterns. Do not add bespoke test executables or test-only search machinery; generic WordVar testing support belongs to word-002.

## Done when

- [x] The word feature guard, component skeleton, generated variable specification, public variable/value/array surface, implementation, views, deltas, and source inventories compile through CMake and Autoconf/Make.
- [x] Every domain update preserves the width mask and lo/hi validity invariant or fails without publishing a partial update.
- [x] Variable construction, updates, arrays, views, deltas, cloning, recovery, and disposal follow the applicable established Gecode variable patterns, including widths 1 and 64 and rejection outside the supported width/mask range.
- [x] Checked-in generated kernel headers are fresh for the word variable specification.

## Validation

- Review the WordVar implementation against the corresponding Int, Set, and Float variable patterns; defer generic domain/lifecycle test machinery to word-002.
- Run generated-variable-header freshness checks.
- Configure and build focused CMake and Autoconf/Make word-enabled and word-disabled configurations.

## Result

Added the optional GecodeWord component with a 1-64 bit lo/hi WordVar kernel, arrays, basic and explicit-width constant views, deltas, printing, clone/recovery generation, and standard CMake/Autoconf integration.

Validation:

- Independent pattern verification passed; normal CMake and Autoconf word-enabled/disabled component builds passed; canonical generated-header comparisons and git diff checks passed. Generic WordVar testing is explicitly deferred to word-002.
