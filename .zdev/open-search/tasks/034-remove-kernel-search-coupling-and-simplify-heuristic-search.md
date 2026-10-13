+++
schema_version = 1
id = "open-search-034"
key = "restore-kernel-search-boundaries"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = ["open-search-033"]
+++
# Remove kernel search coupling and simplify heuristic search examples

## Outcome

The kernel preparation protocol is independent of search policy; brancher notification uses no extra Space field; examples explain the public search interfaces plainly.

## Context

The user reviewed draft PR 256 and requested concrete wording, removal of kernel/core.hpp's Search dependency, a better brancher lifecycle mechanism, and clearer examples. Commits b8bb70a475 and 202474992b cover wording and example simplification. Continue the boundary correction without losing task 033's completion validity, nested stops/statistics or prepared-terminal fixes.

## Boundaries

- Keep nested search policy and accounting in the search layer. Use a small kernel callback for preparation rather than opaque context, unchecked downcasts or a general search policy framework.
- Remove the standalone brancher-notification boolean without making ordinary commits scan every brancher. Check propagation dispatch and brancher identity allocation when using existing kernel control bits.
- Preserve ordinary/native search correctness, replay compatibility, separate ranking and bounds, and the Gecode 6.5 experimental scope.
- Use concrete source comments and private helper names. Preserve license text, dictionary data and historical approved task records. No new benchmark campaign or redundant test harness.

## Done when

- [x] Kernel headers and core.cpp reference no Search types and kernel-only compilation/linkage succeeds.
- [x] Preparation still propagates newly active branchers and safely classifies terminal outcomes; nested completion observes outer stop limits, resumes and accounts for actual work.
- [x] The standalone notification boolean is removed, lifecycle hooks describe their actual purpose, and ordinary commit notification remains a cheap conditional.
- [x] Small examples use public search interfaces and print improvements and completion status without embedded regression assertions or timing machinery.
- [x] The source scan leaves retain variants only in unchanged dictionary/license text; independent review and required validation pass.

## Validation

- Build experimental ON/OFF and run existing focused search, completion and worker-control checks; run current ASan+UBSan checks when practical.
- Build and run the AStar and best-first examples; independently check tiny TSPTW formulations after preparation caller adaptation.
- Check kernel-only include/linkage, all build registrations, report measurements unchanged, and git diff whitespace.

## Result

Separated kernel preparation from search control; removed the per-Space notification boolean using packed control state; renamed lifecycle hooks and simplified public examples and wording.

Validation:

- Independent Astra high review passed against equal snapshot We013cfe7156fe882; kernel-only and restriction/disable/trace probes, source inventory and whitespace checks passed.
- Experimental ON 479/479 and OFF 475/475; focused ON 24/24 and OFF 20/20; current ASan+UBSan 24/24. Independent release and sanitizer replays passed; macOS leak detection unsupported.
- Both simplified examples, 60 independently checked tiny TSPTW combinations and CADDS witness passed; targeted clang-tidy had no diagnostics. Packed notification reduces theoretical brancher IDs from 30 to 29 bits.
