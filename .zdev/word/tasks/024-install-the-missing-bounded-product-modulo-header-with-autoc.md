+++
schema_version = 1
id = "word-024"
key = "repair-word-install-manifest"
area = "word"
status = "open"
complexity = "routine"
afk = true
priority = "normal"
blocked_by = []
+++
# Install the missing bounded product-modulo header with Autoconf

## Outcome

Autoconf-installed Word arithmetic headers are self-contained.

## Context

Makefile.in WORDHDR0 at :561 omits arithmetic/bounded-product-mod.hpp, while installed arithmetic.hh:554 includes it unconditionally. A fresh header install followed by a consumer including <gecode/word/arithmetic.hh> fails. CMake and ordinary <gecode/word.hh> consumers do not expose this omission.

## Boundaries

- Change the actual install manifest and focused existing install smoke coverage; no new test framework.

## Done when

- [ ] The missing header is listed and installed by Autoconf.
- [ ] A consumer using only the installed include prefix compiles arithmetic.hh.
- [ ] The Word header inventory is checked for other omissions using the existing build conventions.

## Validation

- Run a temporary-prefix Autoconf install smoke and installed-header compile.
- Run relevant generated-source/install checks; do not rely on source-tree include fallback.
