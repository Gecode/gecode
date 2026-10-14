+++
schema_version = 1
id = "open-search-041"
key = "no-gpi-method-docs"
area = "open-search"
status = "open"
complexity = "routine"
afk = true
blocked_by = []
+++
# Document NoGPI on open-list materialization methods

## Outcome

Make the NoGPI default visible in the Doxygen comments for materialize and the methods that invoke it, on feature/open-cp and its search descendant.

## Context

Factory, Transition and append already document NoGPI. materialize currently omits it; get and close invoke materialize. Edit the parent checkout public header and refresh the child through the managed rebase. Preserve the unrelated open-route edit.

## Boundaries

- Documentation only. State the default scope and callback-local UseGPI override at materialize; use short references for get and close. Do not repeat the full kernel metadata contract or change behavior.
- No build or runtime tests for this comment-only change; independent source review and whitespace checks suffice. Coordination owns parent source commit, child rebase and publication.

## Done when

- [ ] materialize documents NoGPI and the selected UseGPI override; get and close make their materialization scope discoverable.
- [ ] Both branches contain the documentation and remain correctly stacked; the parent user edit is unchanged and independent review passes.

## Validation

- Review comments against materialize/get/close call paths, existing Factory/Transition documentation and Gecode Doxygen conventions. Run git diff --check and compare branch source deltas and the saved parent user edit.
