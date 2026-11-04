+++
schema_version = 1
id = "found-bugs-006"
key = "diagnostics"
area = "found-bugs"
status = "open"
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

- [ ] The overflow has a stated cause and, if a bug, a fix.
- [ ] The doc target gives no warnings.

## Validation

- Build the doc target.
- uv run misc/check.py all --build-dir build/full --jobs 10.
