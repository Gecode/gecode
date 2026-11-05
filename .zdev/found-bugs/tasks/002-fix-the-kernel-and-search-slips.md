+++
schema_version = 1
id = "found-bugs-002"
key = "kernel"
area = "found-bugs"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = ["found-bugs-001"]
+++
# Fix the kernel and search slips

## Outcome

val-commit returns ModEvent values, Space::realloc for pointer arrays uses the pointer size, TraceFilter::TFO frees only what it owns, and Search::relax fails the space.

## Context

gecode/kernel/branch/val-commit.hpp:130 returns ES_FAILED/ES_OK from a ModEvent function; return ME_GEN_FAILED/ME_GEN_NONE. Space::realloc(T**) in gecode/kernel/core.hpp uses sizeof(T) where the elements are T*, so the rrealloc shrink frees the wrong count. TraceFilter::TFO::~TFO in gecode/kernel/trace/filter.hpp frees null: confirm what happens and guard or correct it. gecode/search/relax.hh:62 calls home.failed() where home.fail() is meant, so a failed post leaves the space unfailed.

## Done when

- [x] Each of the four is fixed, with a test where the framework can express it.

## Validation

- Focused kernel and search tests, then check-standard in build/full.

## Result

val-commit returns ModEvent values, a type cleanup with no behaviour change. Space::realloc(T**) measures the old size with sizeof(T*). Shrinking through Space::rrealloc, Space::realloc and Region::realloc computed the tail as m - n, which underflowed, and now uses n - m. TFO guards its free, which has no effect in default builds. Search::relax fails the space when a post fails.

Validation:

- New tests Space::Realloc::Pointers, Space::Realloc::Shrink, Region::Realloc::Shrink and Search::Relax::Failed fail or crash on unfixed code and pass now.
- check 565/565; standard suite 6838/6838, run directly with -threads 10 because check-standard is single-threaded; -test Search 25818/25818.
- Fresh verifier (gpt-6.1-sol high) PASS with no findings.
