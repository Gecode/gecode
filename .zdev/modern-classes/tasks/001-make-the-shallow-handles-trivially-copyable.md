+++
schema_version = 1
id = "modern-classes-001"
key = "trivial-handles"
area = "modern-classes"
status = "open"
complexity = "standard"
afk = true
priority = "high"
blocked_by = []
+++
# Make the shallow handles trivially copyable

## Outcome

Gecode's shallow handles are trivially copyable, a compile-time test keeps them so, and AllowedTypes no longer lists them.

## Context

The brief's Current state lists the handles that the compile-time probe build/allowed-probe/probe.cpp reports as not trivially copyable, and why: each has a hand-written copy constructor, copy assignment, or both that copy members one by one (Group at gecode/kernel/core.hpp:5276, Pos at gecode/kernel/branch/view.hpp:117, IntVar at gecode/int/var/int.hpp:48 and the other variables, VarArray at gecode/kernel/data/array.hpp:902, ViewArray at array.hpp:1123, AFC at gecode/kernel/branch/afc.hpp:147-154 with an empty destructor, LocalHandle at gecode/kernel/core.hpp:4002-4009 with an empty destructor). Reify follows from its BoolVar member. Remove such members where the implicit one is the same, or default them on their first declaration: an out-of-line `= default` after an in-class declaration is user-provided and keeps the class non-trivial. Check every base class and member on the way (VarImpVar, the generated variable classes from misc/genvarimp.py, PropagatorGroup and BrancherGroup), and also the open variable and view lists that AllowedTypes names; rerun the probe for the full list. Keep converting constructors such as IntVar(const Int::IntView&). Home follows the brief's settled decision: it keeps its Space& member (core.hpp:890), its copy constructor becomes implicit or defaulted on its first declaration, its copy assignment is deleted (= delete, so the intent is visible), and Space's copy assignment is deleted (core.hpp:2178). Then remove the trivially copyable types from performance-unnecessary-value-param.AllowedTypes in .clang-tidy, update the AllowedTypes paragraph in docs/code-style.md, and add changelog entries under 7.0.0 for the ABI change and the source changes (Home and Space are no longer copy-assignable; VarArray::operator= returns a non-const reference once implicit). Leave update() functions and the cloning constructors alone: they are not copy members, and their source parameters stay non-const (see the brief's cloning-model bullet).

## Boundaries

- Do not change what a copy does, except that Home and Space lose their copy assignment, which the brief settles. If another hand-written member does more than copy members, keep it and record the type in the result.
- Parameters already passed by const& stay as they are; this task changes no other signatures.

## Done when

- [ ] std::is_trivially_copyable_v holds for Home, Group, PropagatorGroup, BrancherGroup, Pos, Reify, IntVar, BoolVar, SetVar, FloatVar, IntVarArray, BoolVarArray, SetVarArray, FloatVarArray, ViewArray of each view kind, LocalHandle, and AFC with IntAFC, BoolAFC, SetAFC, and FloatAFC, checked by static_asserts in an existing test file. Home keeps its Space& member, and std::is_copy_assignable_v is false for Home and Space, checked the same way. Any type that cannot be made trivially copyable is named in the result with the reason.
- [ ] AllowedTypes in .clang-tidy lists no type that is now trivially copyable, and uv run misc/check.py lint passes.
- [ ] docs/code-style.md and changelog.in describe the change.

## Validation

- Build build/full and build/local; run bin/gecode-test -test Kernel:: -test Array:: and check-standard in build/full.
- uv run misc/check.py lint --build-dir build/full.
