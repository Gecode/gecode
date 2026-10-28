# Working on the Gecode sources

Gecode is a C++ toolkit for constraint programming. Applications use it to
model and solve combinatorial problems, and extend it with their own
propagators, branchers, variable types, and search engines. This repository
holds the library modules under `gecode/` (support, kernel, search, the int,
set, and float variables, minimodel, driver, instance, FlatZinc, and Gist),
example models in `examples/`, the test suite in `test/`, and the FlatZinc and
instance tools in `tools/`.

## Constraint programming

A propagator is a weakly monotonic, contracting function on variable domains
(it only removes values). Propagation runs the propagators of a space until they
reach a common fixpoint, or a domain becomes empty and the space fails. The
kernel chooses the order, and different orders can reach different fixpoints,
so correctness never depends on propagation order or on which fixpoint is
reached. Posting may prune a little, but full propagation happens in
`Space::status()`.

Search explores the alternatives of a space at fixpoint, on clones; branchers
run in posting order. An optimizing search must be exhausted to find the best
solution and prove it optimal. A search stopped early has only the best
solution so far.

## Code conventions

New code follows these rules, even where older code does not:

- Name functions and variables in snake_case (`read_file`, `parse_int`).
- Use `enum class`, unless an existing API requires a plain `enum`.
- Write helpers as real functions (anonymous-namespace, static, or member
  functions) rather than lambdas stored in local variables. Lambdas passed
  directly as arguments, such as callbacks and test bodies, are fine.
- Lines are up to 120 columns. Keep comment and Doxygen prose to about 80
  characters, measured from where the text starts.

The core library (support, kernel, search, and the int, set, float, and
minimodel modules) stores data in Gecode's own containers and memory:
`Support::DynamicArray`, `DynamicStack` and `DynamicQueue`, `SharedArray`,
`Region`, and heap or space allocation. Other standard library facilities, such
as `std::optional`, `std::unique_ptr`, and `<algorithm>`, are fine there. The
instance and FlatZinc modules, tools, examples, and tests may use STL containers
when that is clearer. Gecode builds as C++17.

Add abstractions only when they solve a concrete maintenance or correctness
problem.

## Implementing propagators, branchers, and engines

Propagators:

- Return an honest `ExecStatus`: `ES_FIX` only at a fixpoint, otherwise
  `ES_NOFIX`. Report subsumption with `home.ES_SUBSUMED(*this)` and touch the
  actor no further.
- Check the modification event of every view update (`GECODE_ME_CHECK`), and
  finish iterating a view's domain before modifying that view.
- Subscribe with the weakest sound propagation condition (`*_VAL`, `*_BND`,
  `*_DOM`). Do cheap pruning in the post function, and post nothing when the
  constraint is already decided.

Every actor:

- `copy()` updates every view and handle; `dispose()` cancels every
  subscription and frees what the actor owns.
- An actor owning heap or external resources registers with
  `home.notice(*this, AP_DISPOSE)` and pairs it with `home.ignore` in
  `dispose()`.
- Allocation can throw in `copy()` and while posting. Leave both the source and
  a partial clone in a state that `dispose()` can clean up.

Memory: use space memory for fixed-size actor state, the heap for buffers that
grow (freed in `dispose()`), `Region` for temporaries, and `SharedArray` or a
`SharedHandle` for read-only data shared across clones. Every clone copies the
actor state, so build expensive state lazily.

Branchers: a choice holds only space-independent data, with no views or
pointers into the space, and it must archive deterministically. `commit()` uses
only the choice, since it also runs on clones and recomputed spaces. Skip
assigned views. A brancher whose `status()` is false must still commit choices
it created earlier.

Search engines own every `Space*` and `Choice*` they create. A call to
`choice()` invalidates earlier choices on that space. Parallel search order and
timing are nondeterministic; shared stop state is atomic.

## Testing

Tests use the native framework and run in its shared executable
(`bin/gecode-test` with CMake, `test/test` with Autoconf). Add a test next to
the most similar existing one and use its fixture: `Test::Int::Test`,
`Test::Set::SetTest`, or `Test::Float::Test` for constraints, `Test::Branch` and
`Test::Assign` for branching, `Test::FlatZinc::FlatZincTest` for FlatZinc, and
`Test::Base` for search and kernel behaviour. Keep to the existing executable,
CTest setup, and CI jobs; if the framework cannot test something, explain why
and ask.

A constraint test gives an independent `solution()` predicate over assignments
and a `post()` that calls the public posting function. The harness enumerates
assignments and checks propagation, cloning, reification, failure, and
subsumption against that predicate. Keep the arity and domains small, and set
`contest` only for a consistency the propagator promises. The contracts are in
`test/test.hh`, `test/int.hh`, `test/set.hh`, and `test/float.hh`.

Add the smallest test that fails without the fix. Documentation-only changes
need none.

Run a focused selection first with `-test <pattern>`, then a suite in
proportion to the change: `check` (the small `TestTags::check()` set),
`check-standard` (all but sweeps; new tests are `standard` by default), or
`check-sweep` (exhaustive). A failure prints `-replay <seed> -test <name>`;
rerun with those options to reproduce it. For deeper checks, configure with
`GECODE_SANITIZER`, and for changes to copying, disposal, or ownership, with
`GECODE_ENABLE_FAULT_INJECTION` and run `check-fault`.

## Building

Keep build outputs under `build/`, for example
`cmake -S . -B build/local -DCMAKE_BUILD_TYPE=Release`. Configure with
`GECODE_ENABLE_EXPERIMENTAL=ON` when changing AStar or CABS. If the change
crosses their feature guard, build the default configuration too. The
[CMake build reference](docs/cmake-build.md) covers dependency and sanitizer
options.

When adding or removing sources, update both `Makefile.in` and
`cmake/GecodeSources.cmake`, plus `examples/CMakeLists.txt` for examples.

When include dependencies change, regenerate `Makefile.dep` with `make depend`
in an Autoconf build. It needs Bison 3.8 and Flex 2.6 on `PATH`; the macOS
system Bison is too old. The checked-in paths are relative to the source root
(`./gecode/...`), so an out-of-tree build's `../../` prefixes must be rewritten.
The rules for the changed includes should be the only differences.

`configure.ac` is the source for `configure`. After changing a generator input,
run `make regenerate` in a configured Autoconf build. Ordinary C++ edits need no
regeneration. `.github/ci/generated-source-freshness.sh` checks freshness.

## Documentation

Modelling concepts belong in the tutorial (MPG), outside this repository. API
contracts and implementation invariants belong in Doxygen comments beside the
declarations or algorithms they describe. Example data formats and attribution
go beside their data.

When changing Doxygen comments or documentation inputs, build the `doc` target.
It requires Doxygen 1.18 or newer and `uv`. Each new C++ file needs a
`// STATISTICS: module-category` line matching a neighbouring file.

## Changelog and releases

Describe user-visible changes in the unreleased section of `changelog.in`, and
mark preview APIs with `Experimental: true`. Validate it with
`uv run misc/tidy.py --check`.

The changelog release and the build version are separate. The build version
and library ABI live in `gecode-version.m4`, and change only when preparing a
release.
