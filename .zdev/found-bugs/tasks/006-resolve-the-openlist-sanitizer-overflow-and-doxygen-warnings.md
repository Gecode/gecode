+++
schema_version = 1
id = "found-bugs-006"
key = "diagnostics"
area = "found-bugs"
status = "done"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["found-bugs-005"]
+++
# Resolve the OpenList sanitizer overflow and Doxygen warnings

## Outcome

The OpenList worker-stack overflow under ASan is explained and fixed if it is a Gecode bug, and the Doxygen build has no warnings.

## Context

Under ASan, OpenList with parallel workers overflowed a worker stack in linear propagation and passed serially. ASan frames are large, so decide whether this is a real defect or a stack-size limit of the sanitizer build. The doc target gave 91 warnings earlier; count them from Doxygen's own warning output at HEAD and fix them.

## Done when

- [x] The overflow has a stated cause and, if a bug, a fix.
- [x] The doc target gives no warnings.

## Validation

- Build the doc target.
- uv run misc/check.py all --build-dir build/full --jobs 10.

## Result

The OpenList overflow is a stack limit of the macOS Debug sanitizer build, documented in docs/cmake-build.md; Doxygen warnings drop from 91 to 0 by matching definitions to declarations.

Validation:

- check.py all exit 0 (format, tidy, freshness, build, lint, CTest 8/8, packages, doc); doxygen.log empty
- gecode-test -threads 8: Int::OpenList 61/61, Set::Var 9/9, -iter 2 -fixprob 1 -tag check 546/546; optimized ASan+UBSan OpenList 61/61 with -threads 4
