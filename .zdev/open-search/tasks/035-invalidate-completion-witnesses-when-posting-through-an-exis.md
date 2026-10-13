+++
schema_version = 1
id = "open-search-035"
key = "invalidate-completion-on-reused-home-posting"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = ["open-search-034"]
+++
# Invalidate completion witnesses when posting through an existing Home

## Outcome

Constraints posted through fresh, copied or reused Home objects cannot leave an invalid completion witness that reports a false solution.

## Context

The independent whole-branch review and coordinator reproduced a false solution on four pairwise-distinct variables initially in 1..4: cache Home before completion; prepare and commit completion; post rel(cached_home,x,IRT_LQ,3). Status still reports SS_SOLVED although fresh completion proves infeasibility. Notification currently occurs only in Space-to-Home construction. Inspect Home, PostInfo/GECODE_POST, Search::Completion and the existing CompletionNative fixture. Reproducer: /private/tmp/open-search-audit-cached-home.cpp.

## Boundaries

- Fix the actual posting boundary without reintroducing a kernel dependency on search policy or types.
- Preserve completion preparation, nested stop/statistics, choice replay, cloning, cuts and the cheap ordinary-space notification path. Do not indiscriminately cancel completion on incidental Home copies.
- Extend the existing search fixture with the exhibited behavior; no new harness, API framework or benchmark campaign.

## Done when

- [x] A regression through the public posting API fails before the fix and passes after it: reused/copied Home restrictions trigger completion revalidation and reject the infeasible model.
- [x] Fresh Home posting, completion suspension/resume, nested accounting, replay and prepared-terminal behavior continue to pass existing checks.
- [x] Posting notification stays within the kernel's existing lifecycle abstraction, with no additional per-Space field or higher-layer coupling.

## Validation

- Run existing focused search/completion/worker-control checks and experimental ON/OFF check targets.
- Run focused current ASan+UBSan checks; run the small search examples and existing tiny TSPTW checker after the shared kernel change.
- Verify the regression fails on the original code, review preparation-internal posting for cancellation hazards, and check diff whitespace.

## Result

Invalidate completion witnesses when constraints are posted through cached Home objects.

Validation:

- One existing CompletionNative case failed before the fix and passed afterward; independent Gecode style and lifecycle review passed.
- Full check targets: experimental ON 479/479, OFF 475/475. Focused ON 14/14 and OFF 10/10, five iterations.
- Current ASan+UBSan focused 14/14, five iterations; macOS leak detection unavailable. Small examples and 60 independently checked tiny TSPTW combinations passed.
