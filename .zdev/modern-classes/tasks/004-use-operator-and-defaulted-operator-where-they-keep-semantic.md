+++
schema_version = 1
id = "modern-classes-004"
key = "comparisons"
area = "modern-classes"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["modern-classes-002"]
+++
# Use operator<=> and defaulted operator== where they keep semantics

## Outcome

Every comparison the inventory marks as convertible uses defaulted operator==, operator<=>, or both, and redundant operator!= and relational operators are gone; excluded operators are unchanged.

## Context

Follow the comparison rows in background/inventory.md. C++20 rewrites the operator expressions a != b from operator== and a < b and the other relational operators from operator<=>, so removing a hand-written operator!= or < is safe only when the rewritten candidate gives the same result and the same overload resolution for existing callers. Check for each converted class: mixed-type and reversed-argument calls; explicit calls such as operator!=(a, b) or a.operator<(b), which rewriting does not cover; that a rewritten operator== returns bool; and generic code that takes the operator's address. Check callers in gecode/, examples/, test/, and tools/. Add a focused Test::Base test only for a converted class that no existing test compares. State the rule in docs/code-style.md, and name every change to a public callable in changelog.in under 7.0.0, or keep the public operator.

## Boundaries

- Expression-building operators, FloatVal interval comparisons, and rows the inventory excludes stay unchanged.

## Done when

- [x] Each convertible inventory row is converted or the result names why it was kept.
- [x] Every converted class is compared by at least one test, and the suites pass.
- [x] docs/code-style.md states when to use operator<=> and defaulted operator==.

## Validation

- check-standard in build/full and ctest in build/local.
- uv run misc/check.py lint --build-dir build/full.
- uv run misc/check.py all --build-dir build/full --jobs N before the commit.

## Result

Converted the 14 convertible comparison rows: Support::BitSetData has a defaulted operator==, Int::LDSB::Literal a defaulted operator<=> in place of operator<, and operator!= is removed from PropagatorGroup, BrancherGroup, DFA, IntSet, BinPacking::Item, space_allocator, region_allocator, and 16 int/set/float views. TupleSet keeps operator!= because its implicit int constructor and bool conversion would otherwise make true != t ambiguous. The 13 excluded rows are unchanged. Int::CachedView::operator== called a nonexistent offset() and now compares base(). 11 comparison tests, a docs/code-style.md rule, and changelog entries.

Validation:

- Independent verifier: full review found one issue (TupleSet::operator!= removal made true != t ambiguous), fixed in one correction pass, focused recheck PASS with baseline/current compile probes on AppleClang and LLVM 22. Evidence: check-standard 6831/6831 (before the last lint and TupleSet edits); after them focused Int::Extensional and Compare 260/260, Release ctest 7/7, check.py all rc=0; no compiler warnings; Doxygen 91 warnings as at baseline.
