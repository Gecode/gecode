# Optimization Build and Test Guide

`Gecode::Optimize` provides owning sparse models, numerical LP/MILP solving with
HiGHS, and bounded integer search with Gecode. The component is optional and
requires CMake; `GECODE_ENABLE_OPTIMIZE` defaults to `OFF`.

The [reference documentation](../doxygen/optimize.hh) describes model ownership,
backend restrictions, result guarantees, and the available C++, C, and Python
interfaces. The [model exchange guide](optimize-io.md) describes LP/MPS syntax.

## Requirements

- CMake 3.21 or newer and a C++17-capable compiler.
- HiGHS 1.15 or newer for the numerical backend and checked native LP deductions.
  For a reproducible configuration, use HiGHS 1.15.1 at commit
  `04024d701f79feb8e2f18bc3df0dffc04ef05088` from
  [ERGO-Code/HiGHS](https://github.com/ERGO-Code/HiGHS).
- Python 3.9 or newer for binding and frontend tests.
- MiniZinc 2.10.1 with its matching standard library for MiniZinc tests.

Dependencies must be installed separately. CMake accepts an installed HiGHS
package through `CMAKE_PREFIX_PATH`, or a source checkout through
`GECODE_OPTIMIZE_HIGHS_SOURCE`. It does not download dependencies.
See the [CMake build guide](cmake-build.md) for other Gecode build options.

## Build and Test

From the repository root, with a HiGHS source checkout:

```sh
cmake -S . -B build/optimize -DCMAKE_BUILD_TYPE=Release \
  -DGECODE_ENABLE_OPTIMIZE=ON \
  -DGECODE_OPTIMIZE_HIGHS_SOURCE=/path/to/HiGHS \
  -DGECODE_OPTIMIZE_BUILD_TESTS=ON -DBUILD_TESTING=ON \
  -DGECODE_ENABLE_QT=OFF -DGECODE_ENABLE_GIST=OFF
cmake --build build/optimize --config Release --parallel 4
ctest --test-dir build/optimize -C Release --output-on-failure
```

The integer and search components enable the native optimization backend.
CTest runs the optimization tests and the configured Gecode test subset.
`GECODE_OPTIMIZE_TEST_PYTHON=OFF` disables Python binding tests when the host
Python cannot load an instrumented library, such as in a sanitizer build.

To run the native algorithm and frontend tests separately:

```sh
ctest --test-dir build/optimize -C Release --output-on-failure \
  -R '^optimize-(native|flatzinc|minizinc)'
```

To build without HiGHS, set `GECODE_OPTIMIZE_WITH_HIGHS=OFF` and omit
`GECODE_OPTIMIZE_HIGHS_SOURCE`. Native integer solving remains available;
numerical solving and checked native LP deductions report unsupported status.

## MiniZinc

The experimental `fzn-gecode-optimize` registration supports finite integer
models admitted by the optimization frontend. It accepts linear constraints,
Boolean relations, and supported all-different, element, table, cumulative,
circuit, and regular constraints. Unsupported model features produce an error.
Numerical LP/MILP/QP MiniZinc models and general search annotations are unsupported.

Enable the registration and provide the MiniZinc compiler to register its tests:

```sh
cmake -S . -B build/optimize \
  -DGECODE_OPTIMIZE_MINIZINC_REGISTRATION=ON \
  -DGECODE_OPTIMIZE_MINIZINC_EXECUTABLE=/path/to/minizinc
cmake --build build/optimize --config Release --parallel 4
ctest --test-dir build/optimize -C Release --output-on-failure
```

This reuses the build configured above. The tests run the compiler, driver, and
output processing against the source fixtures. To repeat that test directly:

```sh
python3 -B test/optimize/minizinc_registration.py \
  --minizinc /path/to/minizinc \
  --binary build/optimize/bin/fzn-gecode-optimize \
  --registration build/optimize/minizinc/Release/gecode-optimize.msc
```

For generators that put executables in configuration directories, use the
corresponding `Release` executable path. CTest selects that path automatically.

The native strategy can be selected with `--native-mode auto`, `race`, `plain`,
or `configured`. `--native-diagnostics on` prints the selected route and work
counters as protocol comments. For example:

```sh
/path/to/minizinc \
  --solver build/optimize/minizinc/Release/gecode-optimize.msc \
  --native-mode auto --native-diagnostics on \
  test/optimize/minizinc-fixtures/mzn-native-knapsack.mzn
```

## Standalone Numerical Build

The numerical component can also be built without the native Gecode libraries:

```sh
cmake -S gecode/optimize -B build/optimize-numerical \
  -DCMAKE_BUILD_TYPE=Release \
  -DGECODE_OPTIMIZE_HIGHS_SOURCE=/path/to/HiGHS
cmake --build build/optimize-numerical --config Release --parallel 4
ctest --test-dir build/optimize-numerical -C Release --output-on-failure
```

A standalone build cannot enable `GECODE_OPTIMIZE_WITH_NATIVE`.

## Installation and Package Consumption

```sh
cmake --install build/optimize --config Release --prefix /path/to/install
```

For a combined installation:

```cmake
find_package(Gecode CONFIG REQUIRED COMPONENTS optimize)
target_link_libraries(my_target PRIVATE Gecode::optimize)
```

For a standalone installation, use `find_package(GecodeOptimize CONFIG REQUIRED)`.
Both packages provide `Gecode::optimize` and the alias `Gecode::gecodeoptimize`.
The C interface uses `gecode/optimize/c_api.h` and `Gecode::optimize_c`.

The Python package in `python/` loads the shared C library. Set `PYTHONPATH` to
that directory and `GECODE_OPTIMIZE_LIBRARY` to the built or installed shared
library's absolute path. The package does not download a solver at runtime.
