+++
schema_version = 1
id = "word-061"
key = "autoconf-word-test-directory"
area = "word"
status = "done"
blocked_by = []
+++
# Fix clean Autoconf Word test builds

## Outcome

A clean out-of-tree Autoconf build creates the test/word object directory before compiling registered Word tests.

## Context

WORDTESTOBJ emits test/word/*.o targets, but TESTBUILDDIRS omits test/word. In-tree CI masks the omission because the source directory exists.

## Boundaries

- Do not change source inventories or introduce a new build harness.
- Follow the existing mkcompiledirs and Autoconf smoke-test patterns.

## Done when

- [x] test/word is included in the conventional test build-directory set.
- [x] A clean VPATH build reaches and compiles Word test objects without relying on source-tree directories.
- [x] Existing in-tree build behavior remains unchanged.

## Validation

- Run the focused clean out-of-tree Autoconf configure and Word test build smoke.
- Run the existing generated-source/build checks affected by Makefile.in.
- Run git diff --check.

## Result

Added the missing test/word build directory to the conventional Autoconf VPATH directory set.

Validation:

- Clean VPATH Autoconf configure and make test compiled all registered Word test objects and linked test/test.
- VPATH Word::TestFramework passed.
- In-tree mkcompiledirs remained a no-op and generated-source freshness passed.
- Independent Spec and Standards verification and git diff --check passed.
