+++
schema_version = 1
id = "word-029"
key = "remove-nary-alias-rescans"
area = "word"
status = "open"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["word-021"]
+++
# Skip repeated alias discovery for distinct bounded sum operands

## Outcome

Bound-only n-ary Add avoids quadratic role comparisons when operands are known to be distinct.

## Context

BoundNaryAdd::narrow at bounded.hpp:577 scans each role against every earlier role on every invocation. The actor already stores aliased; cost():558 advertises linear bound-only work for non-aliased inputs. Small hash arities mask this overhead.

## Boundaries

- Use the existing non-aliased fact or an appropriate reusable mapping; keep alias semantics unchanged.
- Do not add a general alias registry or change public search policy.

## Done when

- [ ] Non-aliased bound-only propagation has no quadratic alias-discovery setup.
- [ ] Aliased inputs/result remain correct and clone-safe.
- [ ] Release scaling for n=8,32,128,512 distinguishes bound-only and cube events.

## Validation

- Run existing n-ary alias tests and TestFramework; add no tests that only assert implementation shape.
- Compare solution parity and bound-only work/runtime with the exact parent.
