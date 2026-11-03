# Modern class definitions

## Objective

Gecode's classes use the C++20 and C++23 forms for special members,
comparisons, and const/non-const overload pairs wherever they keep behaviour:
`= default` and the rule of zero, `operator<=>` and defaulted `operator==`,
and deducing `this`. Where clang-tidy has a check for a form, the check is
enabled so the rule stays enforced.

Success:

- Special members that only copy, move, or destroy members are defaulted or
  omitted. `modernize-use-equals-default` and `modernize-use-equals-delete`
  are enabled in `.clang-tidy`, and `uv run misc/check.py lint` passes.
- Gecode's shallow handles are trivially copyable, and a compile-time test
  keeps them so. The handles are `Home`, the groups, `Pos`, `Reify`, the
  variables, `VarArray`, `ViewArray`, `LocalHandle`, and `AFC` with its
  subclasses, plus any others the inventory finds. `performance-unnecessary-value-param`'s
  `AllowedTypes` then lists only the shallow types that still have a
  non-trivial copy.
- Comparison operators and const/non-const overload pairs use `operator<=>`,
  defaulted `operator==`, or deducing `this` where the inventory shows that the
  semantics stay the same and the code gets simpler.
- `docs/code-style.md` states each rule beside the existing performance
  notes.

## Current state

These facts were read from the tree at feature/breaking-7 8a43781198.

- `.clang-tidy` enables `modernize-deprecated-headers`, `modernize-use-nullptr`,
  and `modernize-use-override`. No `modernize-use-equals-*` check is enabled.
- Library code (`gecode/`, without `third-party`) has 27 `= default`, 0
  `operator<=>`, 0 uses of deducing `this`, about 127 `operator==` and 125
  `operator!=` lines, about 285 lines declaring or defining `<`, `<=`, `>`, or
  `>=`, 54 const `operator[]` lines, and 42 empty destructor bodies (grep
  counts, not a clang-tidy measurement).
- A compile-time probe (`build/allowed-probe/probe.cpp`,
  `std::is_trivially_copyable_v`) shows that `Home`, `Group`,
  `PropagatorGroup`, `BrancherGroup`, `Pos`, `Reify`, `IntVar`, `BoolVar`,
  `SetVar`, `FloatVar`, `IntVarArray`, `ViewArray<IntView>`, and `AFC` and
  its subclasses are not trivially copyable. Each has a hand-written copy
  constructor, copy assignment, or both that copy the members one by one; `AFC` also has
  an empty destructor, and `Reify` follows from its `BoolVar` member. The
  views (`IntView`, `BoolView`, `MinusView`, `OffsetView`, `ConstIntView`,
  `SetView`, `FloatView`) already are.
- Parameter passing: the Itanium C++ ABI (Linux, macOS) passes a class that
  is *non-trivial for the purposes of calls* (a non-trivial copy or move
  constructor or destructor, or all of them deleted) through a temporary in
  memory. Other classes follow the platform C ABI, which may still pass
  large ones indirectly; on AArch64 a composite above 16 bytes, such as
  `Home` (24 bytes), goes through memory either way. So a hand-written copy
  constructor makes an 8-byte `IntVar` go through memory today; defaulting it
  makes register passing possible but does not promise it.
- `Home` stores `Space& s`, and its hand-written assignment (core.hpp:3488)
  assigns through that reference, which copy-assigns one whole `Space` onto
  another: `Space`'s copy assignment is defaulted (core.hpp:2178). An implicit
  `Home` assignment is deleted because of the reference member.
- `FloatVal` has two causes: its own hand-written copy constructor and copy
  assignment (gecode/float/val.hpp:55, 64), and the Boost interval member,
  which declares a copy constructor. Its hand-written default constructor
  (val.hpp:44) does not affect the trait but is also defaultable. The boost area
  decides whether that member type is replaced (boost-004 and boost-005).

## Boundaries and non-goals

- Owned: class definitions in `gecode/`, `examples/`, `test/`, and `tools/`;
  `misc/genvarimp.py` and the other generators for generated class code;
  `.clang-tidy`; `docs/code-style.md`; and `changelog.in`.
- Behaviour does not change. Where a defaulted member would differ from the
  hand-written one (it copies a member the original did not, or skips work
  the original did), keep the hand-written member.
- Operators that build modelling expressions are not comparisons and stay as
  they are. For example, `IntVar < IntVar` builds a `LinIntRel`, and the
  minimodel relations on Boolean and set expressions build `BoolExpr`.
  Interval comparisons on `FloatVal` have possibly/certainly semantics and
  stay as they are. Set inclusion and other partial orders get
  `operator<=>` only when every existing operator agrees with it.
- `gecode/third-party` is excluded. `FloatVal`'s hand-written members are
  defaulted like any other class; whether `FloatVal` becomes trivially
  copyable depends on its member type and so on boost-005. The value-params
  rule that passes `FloatVal` by `const&` (value-params-008) stays unless the
  user decides to revisit it.
- Version and ABI numbers stay at 6.5.0 / 52.

## Terms and settled decisions

- The user decided on 2026-11-03 to use `= default` extensively where it
  makes sense, as part of the Gecode 7 modernization, to enforce it with
  clang-tidy if possible, and to use deducing `this` and spaceship operators
  where they make sense.
- A member defaulted out of line (`X::X(const X&) = default;` after an
  in-class declaration) is user-provided, so the class stays non-trivial.
  Where triviality matters, default the member on its first declaration or
  omit it.
- clang-tidy 22's `modernize-use-equals-default` does not handle move
  constructors or move assignments, and `modernize-use-equals-delete` offers
  only partial fixes. Clean diagnostics alone therefore do not prove the
  objective; the inventory lists the manual candidates as well.
- `Home` (user, 2026-11-03): its `Space&` member is intentional. A home can
  never be null and must never be repointed, so `Home` keeps the reference.
  Its copy assignment, which today assigns one whole `Space` onto another
  through the reference, is deleted, and its copy constructor is implicit or
  defaulted on its first declaration. A class with a reference member, a
  trivial copy constructor, and a deleted copy assignment is trivially
  copyable (checked with a static_assert probe,
  build/allowed-probe/p4.cpp). `Space`'s copy assignment is deleted as well:
  assigning spaces is never meaningful, and cloning is the way to copy one.
  A scratch build of the library, examples, FlatZinc, Gist, and tests at
  8a43781198 with both assignments deleted compiles, so nothing in the tree
  assigns either type (build/home-probe/). The changelog names both source
  breaks.
- Gecode's cloning model (MPG: "Managing memory", "Updating variables", and
  the cloning tips in "Getting started"; Tack 2009, section 6.7) fixes what a
  C++ copy of a handle means. Within one space, copying a handle aliases the
  same space-allocated object. Copying into another space happens only during
  cloning, through `update(home, x)` and the cloning constructors
  (`Space(Space&)`, `Propagator(Space&, P&)`, `VarImp(Space&, VarImp&)`),
  which follow forwarding pointers. Space memory is reclaimed with the space,
  so a handle owns nothing and needs no destructor. Implicit, trivial copy
  members and destructors therefore keep the handles' behaviour. The update
  functions and cloning constructors are not C++ copy members and are out of
  scope for `= default`. They take their source by non-const reference
  because cloning writes forwarding pointers into it, so neither this area's
  tools nor deducing `this` may make that parameter const.
- "Shallow handle" has the meaning in the value-params brief: a copy is a few
  pointers or ids, with no reference count or allocation.
- ABI changes are accepted for Gecode 7. Source changes for users are
  accepted when the changelog names them, for example `VarArray::operator=`
  returning a non-const reference once it is defaulted.
- Wide mechanical edits come from a reproducible tool run. The commit message
  gives the exact commands, so a later rebase reruns the tool instead of
  resolving conflicts by hand. The planned landing order puts
  feature/comparison and feature/random on main before feature/breaking-7, so
  these tool runs are rerun over their code.
- The source-cleanup area still runs last.

## Testing

Focused coverage plus existing checks.

- A compile-time test asserts `std::is_trivially_copyable_v` for every
  shallow handle, so a hand-written copy member cannot come back unnoticed.
  Put it in an existing kernel or array test file as `static_assert`s.
- Where a comparison changes form, existing tests that compare the type must
  keep passing. Add a focused `Test::Base` test only where no existing test
  compares the type.
- Mechanical `= default` edits rely on the build, lint, and the integrity
  suite.

## Validation

- `uv run misc/check.py lint --build-dir <dir>` on a build with experimental,
  CBS, and Gist so that optional code is linted.
- `uv run misc/check.py all --build-dir build/full --jobs N` before the last
  commit.
