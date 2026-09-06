# Testing custom propagators

Gecode's test harness can check a custom integer propagator against a small,
independent specification. The test supplies two things: a predicate that says
which complete assignments are valid, and a function that posts the propagator.
The harness compares them while exercising propagation, cloning, pruning, and
search.

The installed API covers the core runner and the integer test helpers:

- `Gecode::gecodetest` provides test registration and the runner.
- `Gecode::gecodetestint` provides `Test::Int::Test` and links to the core runner.
- `test/test.hh` and `test/test.hpp` declare the core API.
- `test/int.hh` and `test/int.hpp` declare the integer helpers.

Other helper families under `test/`, including the set, float, branch, assign,
and FlatZinc helpers, are not part of the installed API.

## Install the test component

The test component requires the search and integer modules. With CMake, enable
testing when configuring Gecode:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
cmake --install build --prefix /path/to/gecode
```

The Autoconf build installs the same headers and static libraries when search
and integer variables are enabled:

```bash
mkdir -p build/legacy
cd build/legacy
../../configure --disable-qt --disable-gist --disable-doc-search --disable-examples
make -j4
make install prefix=/path/to/gecode
```

See [the CMake build guide](./cmake-build.md) for package lookup and other build
options.

## Link a test executable with CMake

An installed-package consumer needs the `test` component and the integer helper
target:

```cmake
cmake_minimum_required(VERSION 3.21)
project(custom_propagator_tests LANGUAGES CXX)

find_package(Gecode CONFIG REQUIRED COMPONENTS test)

add_executable(custom-propagator-test custom-propagator-test.cpp)
target_compile_features(custom-propagator-test PRIVATE cxx_std_17)
target_link_libraries(custom-propagator-test PRIVATE Gecode::gecodetestint)
```

The target names are identical when Gecode is brought in with
`add_subdirectory` or `FetchContent`. Set `BUILD_TESTING` to a true CMake value
before adding Gecode so that the test targets exist.

If CMake cannot locate an installed package, set one of these variables:

- `CMAKE_PREFIX_PATH=/path/to/gecode`
- `Gecode_ROOT=/path/to/gecode`
- `Gecode_DIR=/path/to/gecode/lib/cmake/Gecode`

## Write the test

The following test checks an equality propagator over two variables with domain
`0..1`. Two assignments satisfy equality and two violate it, so an empty or
incorrect posting function cannot pass.

```c++
#include <test/int.hh>

#include <gecode/int.hh>

namespace {

void post_equal(Gecode::Space& home, Gecode::IntVar x, Gecode::IntVar y) {
  // Replace this body with the posting function for the propagator under test.
  Gecode::rel(home, x, Gecode::IRT_EQ, y);
}

class EqualityTest final : public ::Test::Int::Test {
public:
  EqualityTest()
    : ::Test::Int::Test("Package::Equality", 2, 0, 1) {}

  bool solution(const ::Test::Int::Assignment& assignment) const override {
    return assignment[0] == assignment[1];
  }

  void post(Gecode::Space& home, Gecode::IntVarArray& x) override {
    post_equal(home, x[0], x[1]);
  }
} equality_test;

} // namespace

int main(int argc, char* argv[]) {
  return Test::run_registered_tests(argc, argv);
}
```

`solution()` is the specification. Keep it independent of the propagator code.
For a complicated constraint, a direct calculation over the assigned values is
usually a better oracle than calling another version of the same propagator.

The constructor arguments give the test its name, arity, and common variable
domain. Override `assignment()` when the variables need different domains or a
custom assignment generator.

`post()` receives fresh variables in a test space. It should call the same
posting function that users of the propagator call. The harness checks the
posted constraint against the oracle across complete assignments and under
partial-domain modifications.

The test object registers during construction. Give it static lifetime, as in
the example. Destruction does not unregister it. Registered objects must remain
alive until all calls to `run_registered_tests` have returned, and runner calls
must not overlap.

## Consistency and reification

The final constructor arguments select reification and the integer propagation
level:

```c++
EqualityTest()
  : ::Test::Int::Test("Package::Equality", 2, 0, 1,
                      false, Gecode::IPL_DOM) {}
```

`IPL_DOM` also selects the domain-consistency check. A subclass can set the
protected `contest` member to `CTL_NONE`, `CTL_BOUNDS_D`, or `CTL_BOUNDS_Z` when
the propagator promises a different consistency level.

Pass `true` as the reification argument for a reified propagator and override
the reified posting function:

```c++
void post(Gecode::Space& home, Gecode::IntVarArray& x,
          Gecode::Reify r) override {
  Gecode::rel(home, x[0], Gecode::IRT_EQ, x[1], r);
}
```

The protected `rms` bit mask restricts the tested modes to `RM_EQV`, `RM_IMP`,
or `RM_PMI` when the propagator supports only part of the reification API.
`testsearch` and `testfix` can disable the corresponding checks for constraints
where those checks do not apply.

## Run and reproduce tests

List the registered tests:

```bash
./custom-propagator-test -list
```

Run the equality test once and stop on its first error:

```bash
./custom-propagator-test -test Package::Equality -iter 1 -stop true
```

A failure reports the random seed and test name. Use both values to reproduce
the run:

```bash
./custom-propagator-test \
  -test Package::Equality \
  -seed 12345 \
  -iter 1 \
  -threads 1 \
  -log \
  -stop true
```

`-log` prints the buffered test log on failure and cannot be combined with a
multi-threaded run. `-help` prints the complete option list. Help and malformed
options terminate the process, so the runner belongs in a test executable
rather than an embedding library.

## Link without CMake package metadata

The Autoconf installation provides static `gecodetest` and `gecodetestint`
libraries. A direct compiler invocation must name their dependency closure:

```bash
c++ -std=c++17 -I<prefix>/include custom-propagator-test.cpp \
  -L<prefix>/lib \
  -lgecodetestint -lgecodetest \
  -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport
```

Use the configured `libdir` or `sharedlibdir` instead of `<prefix>/lib` when the
installation uses a different library directory.

The installation does not include `gecode-test`, `public-runner-smoke`, or
`public-int-smoke`. Those executables check Gecode's own build and are not part
of the downstream interface.

## Check an installation

The package checks build and run the sample consumer against an installed
prefix:

```bash
python test/package/verify-installed-test-component.py \
  --source . \
  --build-root build/package-check \
  --prefix /path/to/gecode
```

For a nonstandard layout, pass `--include-dir` and `--lib-dir`. The Autoconf
package check uses `verify-installed-legacy-test-component.py` with the same
`--source`, `--build-root`, and `--prefix` arguments.
