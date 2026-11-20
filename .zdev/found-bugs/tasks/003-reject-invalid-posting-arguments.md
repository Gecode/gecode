+++
schema_version = 1
id = "found-bugs-003"
key = "post-arguments"
area = "found-bugs"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = ["found-bugs-002"]
+++
# Reject invalid posting arguments

## Outcome

bin_packing rejects d<1, 2D element rejects an empty matrix with a documented exception, and the exceptions of set sequence and set element are documented.

## Context

gecode/int/bin-packing.cpp:72 divides by zero when d==0 (SIGFPE on x86). Throw a documented existing exception before the division. 2D int element with w*h==0 throws an undocumented Int::VariableEmptyDomain from its internal IntVar(0,w*h-1) (gecode/int/element.cpp, near :259 and :267), where 1D element throws TooFewArguments. set/sequence.cpp:44 and set/element.cpp:150-197 throw TooFewArguments, which set.hh does not document. Check IntSet(std::initializer_list) with an empty list (gecode/int/int-set.cpp); fix it if it still asserts n>0.

## Done when

- [x] Each case throws a documented exception, covered by a focused test.

## Validation

- Focused tests in build/full.

## Result

Reject invalid bin_packing d and malformed 2D element matrices with documented exceptions; IntSet empty list was already fixed by df72e330c6.

Validation:

- gecode-test -threads 8: Int::BinPacking::NoDimension 1/1, Int::Element::Matrix 9/9, Set::Element 11/11, Set::Sequence 5/5
- check 565/565, fault 21/21, doc 0 warnings, tidy and format pass
