# Public Test Harness Guide

This document describes Gecode's supported public testing surface.
It is for downstream users who want to write Gecode-style tests for custom propagators
without developing inside the Gecode source tree.

For general build, install, and package-consumption setup, see
[`docs/cmake-build.md`](./cmake-build.md).

## What this gives you

If Gecode was built and installed with `BUILD_TESTING=ON`, the installed CMake package
exports a `test` component with:

- `Gecode::gecodetest` — core test registration and runner support
- `Gecode::gecodetestint` — integer-test helpers layered over the core runner

The supported installed header surface is intentionally narrow:

- `test/test.hh`
- `test/test.hpp`
- `test/int.hh`
- `test/int.hpp`

The public runner entrypoint is:

```c++
int Test::run_registered_tests(int argc, char* argv[]);
```

This is the same runner seam used by Gecode's own `gecode-test` executable.

## Supported first-release scope

The first public release is intentionally focused.

Supported:

- The core process-global test registry and runner from `test/test.hh`
- Integer-test helpers based on `Test::Int::Test`
- Installed CMake consumption through `find_package(Gecode CONFIG REQUIRED COMPONENTS test)`
- Installed native legacy consumption through the maintained `-I`, `-L`, and `-l...` contract below
- A downstream executable that registers tests and forwards `main(...)` into
  `Test::run_registered_tests(argc, argv)`

Not yet part of the supported installed surface:

- Public installation of the wider helper families under `test/` such as `set`, `float`,
  `assign`, `branch`, or `flatzinc`
- Smoke executables such as `public-runner-smoke`, `public-int-smoke`, or `gecode-test`
  as install artifacts
- A separate replacement framework or a redesigned runner model

## Prerequisite: install Gecode with the public harness enabled

The public harness is only installable when the required search and integer modules are
available.

### CMake install path

The installed CMake package exports the `test` component when the build was configured
with `BUILD_TESTING=ON` and the required harness dependencies were available.

Typical flow:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --target gecodetest gecodetestint gecode-test
cmake --install build --prefix /path/to/install
```

For full build/install details, platform notes, and package-location hints, see
[`docs/cmake-build.md`](./cmake-build.md).

### Legacy Autoconf/Make install path

The legacy install path builds and installs the same narrow public harness boundary when
search and integer support remain enabled.

Typical flow:

```bash
mkdir -p build/legacy && cd build/legacy
../../configure --disable-qt --disable-gist --disable-doc-search --disable-examples
make -j4
make install prefix="$PWD/prefix"
```

## Native legacy install contract

This section is the maintained native contract for downstream consumers using the
Autoconf/Make install path.

Installed public headers:

- `<prefix>/include/test/test.hh`
- `<prefix>/include/test/test.hpp`
- `<prefix>/include/test/int.hh`
- `<prefix>/include/test/int.hpp`

Installed reusable harness libraries:

- `gecodetest` — static core runner/registry seam
- `gecodetestint` — static integer-helper seam layered over `gecodetest`

Use the installed include directory and the normal installed library directory from the
legacy build (`<prefix>/include` and typically `<prefix>/lib`; if you configured a
custom `libdir`/`sharedlibdir`, use that installed path instead).

Supported native compile/link shape:

```bash
c++ -std=c++17 -I<prefix>/include consumer-smoke.cpp \
  -L<prefix>/lib \
  -lgecodetestint -lgecodetest -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport
```

That full link closure is the honest maintained contract for native legacy consumers.
Some shared-library linkers may accept a shorter command line, but downstream proofs and
support assume the explicit closure above rather than implicit transitive behavior.

What the native legacy contract still does **not** install or support:

- `test/set.hh`, `test/set.hpp`, `test/float.hh`, `test/float.hpp`, `test/assign.hh`,
  `test/branch.hh`, or `test/flatzinc.hh`
- proof-only binaries such as `public-runner-smoke`, `public-int-smoke`, or `gecode-test`
- a second discovery layer such as pkg-config

### Maintained installed-proof entrypoint

The canonical installed-proof entrypoint for the native legacy path is:

```bash
python test/package/verify-installed-legacy-test-component.py \
  --source . \
  --build-root build/legacy-test-component-proof \
  --prefix build/legacy-test-component-proof/prefix
```

This is the maintained anti-drift path used by the docs and the Ubuntu Autoconf CI
job. It first re-checks the installed prefix surface, then builds the downstream
consumer in a temp workspace outside the source tree, and finally launches
separate `-list` and filtered-run executions against the installed prefix.

The verifier owns runtime library-path setup for launched processes. It injects
the resolved installed library directory into `LD_LIBRARY_PATH` and
`DYLD_LIBRARY_PATH`, so the proof stays bound to the selected install prefix
instead of ambient system state.

There is also a supported negative mode that proves unsupported helper headers
remain outside the installed legacy surface:

```bash
python test/package/verify-installed-legacy-test-component.py \
  --source . \
  --build-root build/legacy-test-component-proof \
  --prefix build/legacy-test-component-proof/prefix \
  --mode unsupported-header
```

## Minimal downstream CMake consumer

```cmake
cmake_minimum_required(VERSION 3.21)
project(gecode_public_test_consumer LANGUAGES CXX)

find_package(Gecode CONFIG REQUIRED COMPONENTS test)

add_executable(consumer-smoke consumer-smoke.cpp)
target_compile_features(consumer-smoke PRIVATE cxx_std_17)
target_link_libraries(consumer-smoke PRIVATE Gecode::gecodetestint)
```

If CMake does not find the package automatically, point it at the install prefix:

- `-DCMAKE_PREFIX_PATH=/path/to/install`
- or `-DGecode_ROOT=/path/to/install`
- or `-DGecode_DIR=/path/to/install/lib/cmake/Gecode`

## Minimal downstream test

```c++
#include <test/int.hh>

#include <gecode/int.hh>

namespace {

class ConsumerSmoke final : public ::Test::Int::Test {
public:
  ConsumerSmoke()
    : ::Test::Int::Test("Package::ConsumerSmoke", 1, 0, 1) {}

  bool solution(const ::Test::Int::Assignment& assignment) const override {
    return assignment[0] >= 0;
  }

  void post(Gecode::Space& home, Gecode::IntVarArray& x) override {
    Gecode::rel(home, x[0], Gecode::IRT_GQ, 0);
  }
} consumer_smoke;

} // namespace

int main(int argc, char* argv[]) {
  return Test::run_registered_tests(argc, argv);
}
```

This mirrors the maintained sample consumer at
`test/package/public-test-component/consumer-smoke.cpp`.

## Running the downstream test executable

List registered tests:

```bash
./consumer-smoke -list
```

Run just one test:

```bash
./consumer-smoke -test Package::ConsumerSmoke -iter 1 -stop true
```

The runner uses the same option model as Gecode's own `gecode-test` binary.
The supported public seam is the runner function, not a separate alternate CLI.

## Package component behavior

The installed `GecodeConfig.cmake` recognizes `test` as a supported component.
The package treats the component as available only when both of these imported
targets are present:

- `Gecode::gecodetest`
- `Gecode::gecodetestint`

If a consumer requests an unsupported component, the package emits an explicit
configure-time diagnostic rather than failing later at link time.

## Installed CMake proof path

The canonical downstream verifier is:

```bash
python test/package/verify-installed-test-component.py \
  --source . \
  --build-root build/package-proof \
  --prefix /path/to/install
```

What it checks:

- the installed `test/` headers are present and limited to the supported public set
- the package exports `Gecode::gecodetest` and `Gecode::gecodetestint`
- the downstream consumer configures through the installed package metadata
- consumer compile flags use the installed prefix instead of source-tree include leakage
- the consumer binary builds successfully
- `-list` discovers the registered downstream test
- a filtered run executes the selected test successfully

There is also a negative-path mode that confirms unsupported-component diagnostics:

```bash
python test/package/verify-installed-test-component.py \
  --source . \
  --build-root build/package-proof-missing \
  --prefix /path/to/install \
  --expect-missing-component-failure
```

## How this relates to Gecode's own tests

Inside the repo, `gecode-test` now links through the same public harness seam:

- `gecodetest`
- `gecodetestint`

That means the supported downstream path is not a sidecar proof-only API. The
same runner/library boundary is used both for in-tree testing and installed
package consumption.

## Troubleshooting

### `find_package(Gecode CONFIG REQUIRED COMPONENTS test)` fails

Check:

- the installation was built with `BUILD_TESTING=ON`
- the package lookup points at the intended install prefix
- the install tree actually contains `lib/cmake/Gecode/GecodeConfig.cmake`

### `Gecode::gecodetestint` is missing

The `test` component is only considered available when both `Gecode::gecodetest`
and `Gecode::gecodetestint` are exported. Treat a missing companion target as an
installation/configuration issue, not as something to patch around in the consumer.

### The consumer builds but seems to use source-tree headers

Run the verifier. It checks `compile_commands.json` for include-leakage and is the
maintained proof path used by CI as well.

### I need non-integer helper families

That is outside the supported first-release installed surface. The installed public
contract currently stops at the core runner plus integer-test helpers.
