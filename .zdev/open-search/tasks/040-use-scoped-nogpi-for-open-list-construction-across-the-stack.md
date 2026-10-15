+++
schema_version = 1
id = "open-search-040"
key = "no-gpi-stack"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = []
+++
# Use scoped NoGPI for open-list construction across the stack

## Outcome

Integrate the reviewed NoGPI kernel feature into feature/open-cp, suppress GPI by default for open-list factories and transitions, then update and validate feature/open-search on that parent.

## Context

The user explicitly authorized updating both branches. Import only the three reviewed feature/no-gpi commits, without the newer main baseline. Preserve backup refs and the unrelated open-route edits in the parent checkout. Use a temporary parent checkout for implementation; coordination owns commits, parent fast-forward, descendant rebase and publication.

## Boundaries

- Scope suppression in materialize and the transition invocation in append, including direct append. Callback-local Space::UseGPI permits selected postings; document AFC and callback scopes. No new configuration toggle.
- Preserve known_length, experimental search guards and ordinary posting outside open-list calls. Avoid benchmark campaigns and unrelated cleanup. Existing kernel tests travel with the feature; add only a focused Gecode-style open-list contract check if useful.

## Done when

- [x] The reviewed kernel commits and default open-list NoGPI scopes are on feature/open-cp, with public callback documentation and selected UseGPI supported.
- [x] feature/open-search is rebased onto the updated parent with its heuristic changes preserved and backups available.
- [x] Both branches pass focused existing open-list/AFC tests and example checks; the search branch also passes heuristic checks with experimental ON/OFF and fully instrumented ASan/UBSan where applicable. Independent review passes.

## Validation

- Build affected parent and child code; run existing AFC/open-list checks and representative open/fixed fifteen-puzzle runs. Verify factory, direct append transition, nested growth, opt-in and restoration through a proportionate Gecode-style check.
- Run focused native heuristic tests and experimental-disabled BAB on the child. Run a fully instrumented ASan/UBSan build and selected checks; inspect diagnostics.
- Review branch ancestry, ranges and diffs, source style, generated-source freshness if changed, and unchanged user edits.

## Result

Imported reviewed NoGPI kernel support into feature/open-cp and made open-list factories and transitions use NoGPI by default, including direct append. Documented callback-local UseGPI and added one focused Gecode-style contract fixture. Rebased feature/open-search through zdev, preserving search changes and backup branches. Independent verification passed without findings; unrelated parent edit preserved.

Validation:

- Parent Release: 45 AFC/open-list/open-regular checks and 10 puzzle cases passed. Child Release ON: 53 checks and 30 puzzle cases; OFF: 49 checks and 10 BAB cases. Independent optimum calculations and path replay passed.
- Fully instrumented ASan/UBSan with CBS: 54 checks and 12 puzzle cases passed with empty stderr. Selected clang-tidy analyzer checks passed. Kernel commit/range comparison, ancestry, scope restoration, cloning, guards, whitespace and parent edit preservation checked.
