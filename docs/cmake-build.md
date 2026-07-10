# CMake Build and Package Guide

This document describes the CMake build, install, and package-consumption flow
for Gecode.

## Requirements

- CMake 3.21 or newer
- A C++17-capable compiler
- `uv` on `PATH` for generated-script paths: documentation generation,
  `GECODE_REGENERATE_VARIMP=ON`, and manual helper execution. Ordinary CMake
  builds can use the checked-in generated headers without `uv`.
- Python 3.11 or newer for helper scripts run through `uv`.
- Optional dependencies based on enabled modules:
  - MPFR for float support with MPFR
  - Qt 6 Core, or Qt 5.15+ Core, for FlatZinc plugin support when Qt support
    is enabled or auto-detected
  - Qt 6 Gui, Widgets, and PrintSupport, or the matching Qt 5.15+ components,
    for Gist

## Quick Start

Single-config generators (Unix Makefiles / Ninja):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --build build --target check
cmake --install build --prefix /path/to/install
```

Multi-config generators (Visual Studio / Xcode):

```bash
cmake -S . -B build
cmake --build build --config Release
cmake --build build --config Release --target check
cmake --install build --config Release --prefix /path/to/install
```

## Visual Studio + vcpkg (MPFR)

This repository includes a vcpkg manifest (`vcpkg.json`) and CMake presets
for a ready-to-run Visual Studio path with MPFR enabled.

Prerequisites:

- Visual Studio 2022 with C++ toolchain
- CMake 3.21 or newer
- `VCPKG_ROOT` set to your vcpkg checkout

Preset flow:

```powershell
cmake --preset vs2022-vcpkg
cmake --build --preset vs2022-vcpkg-release
cmake --build --preset vs2022-vcpkg-check
```

Equivalent command-line flow (without presets):

```powershell
cmake -S . -B build/vs2022-vcpkg -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DGECODE_ENABLE_MPFR=ON -DGECODE_ENABLE_QT=OFF -DGECODE_ENABLE_GIST=OFF
cmake --build build/vs2022-vcpkg --config Release
cmake --build build/vs2022-vcpkg --config Release --target check
cmake --install build/vs2022-vcpkg --config Release --prefix C:/path/to/install
```

Visual Studio is a multi-config generator, so use `--config Release` (or
`Debug`) for build/install/check commands rather than `CMAKE_BUILD_TYPE`.

## Build Conventions and Key Options

### Common CMake options

- `BUILD_SHARED_LIBS`: conventional default selector
  - `ON` default behavior: `GECODE_BUILD_SHARED=ON`, `GECODE_BUILD_STATIC=OFF`
  - `OFF` default behavior: `GECODE_BUILD_SHARED=OFF`, `GECODE_BUILD_STATIC=ON`
- `BUILD_TESTING`: controls whether `gecode-test` is built and whether `check`
  runs the test subset. The `check` target still exists when tests are disabled,
  but reports that checks are skipped.
- `CMAKE_BUILD_TYPE` / multi-config `--config`: standard build mode control

### Gecode-specific options

- `GECODE_BUILD_SHARED`, `GECODE_BUILD_STATIC`: explicit library variants
  - If set directly, these override `BUILD_SHARED_LIBS`-based defaults.
- `GECODE_INSTALL`: enables install/export/package rules
  - Defaults to `ON` when Gecode is top-level; defaults to `OFF` as subproject.
- `GECODE_ENABLE_*`: module toggles (search/int/set/float/minimodel/driver/flatzinc/gist, etc.)
  - Higher-level modules force their required lower-level modules on. For
    example, examples and FlatZinc require the core modeling modules, and
    minimodel requires int, set, and search support.
  - `GECODE_REGENERATE_VARIMP`: regenerate checked-in var-imp headers during build
- `GECODE_ENABLE_QT`: `AUTO`, `ON`, or `OFF`. `AUTO` uses Qt-backed FlatZinc
  plugin support when Qt Core is found. `ON` requires Qt and fails
  configuration if supported Qt is missing. `OFF` disables Qt. Qt 6 is
  preferred; Qt 5.15 or newer is accepted for legacy environments. This can add
  a public Qt Core dependency to `Gecode::gecodeflatzinc`.
- `GECODE_ENABLE_GIST`: `AUTO`, `ON`, or `OFF`. `AUTO` builds Gist when a
  usable Qt GUI stack is available. `ON` requires Gist dependencies and fails
  configuration if they are unavailable. Gist requires Qt Gui, Widgets, and
  PrintSupport in addition to Qt Core.
- `GECODE_ENABLE_FAULT_INJECTION`: builds deterministic allocation and copy
  failpoints together with the `Fault::*` regression suite. This facility is
  test-only, process-global, isolated to the dedicated single-threaded fault
  test executable, CMake-only, and disabled by default.
- `GECODE_SANITIZER`: selects `address`, `undefined`, `address-undefined`, or
  `thread` instrumentation. These modes currently require a compiler that
  accepts GCC/Clang sanitizer flags.
- `GECODE_DOC_DOT`, `GECODE_DOC_SEARCH`, `GECODE_DOC_TAGFILE`: configure the
  Doxygen `doc` target. Building docs requires Doxygen 1.17.0 or newer and
  `uv`; Graphviz `dot` is used when available and `GECODE_DOC_DOT=ON`.

Top-level defaults:

- `GECODE_ENABLE_EXAMPLES=ON`
- `BUILD_TESTING=ON`
- `GECODE_INSTALL=ON`

Subproject defaults (`add_subdirectory`):

- `GECODE_ENABLE_EXAMPLES=OFF`
- `BUILD_TESTING=OFF` (unless enabled by parent project)
- `GECODE_INSTALL=OFF`

### Fault injection and sanitizers

Fault injection is test-only and process-global. Its tests run in a dedicated,
isolated single-threaded executable and must not share a test process with
ordinary tests. The CMake targets enforce this:

```bash
cmake -S . -B build-fault -DGECODE_ENABLE_FAULT_INJECTION=ON
cmake --build build-fault --target check-fault
```

The ordinary `check` target also runs `check-fault` when fault injection is
enabled. CTest registers the fault suite as a separate process with
`-threads 1` and serializes it with the fault-injection resource lock. The
FailPoint facility is not a supported application API.

For a sanitizer build, configure a separate build directory. For example:

```bash
cmake -S . -B build-asan-ubsan \
  -DGECODE_SANITIZER=address-undefined
cmake --build build-asan-ubsan --target check
```

## Using Gecode From Other CMake Projects

### Recommended aggregate target

`Gecode::gecode` links the exported aggregate target. Use it unless the
consumer needs explicit module linkage.

```cmake
find_package(Gecode CONFIG REQUIRED)
add_executable(app main.cpp)
target_link_libraries(app PRIVATE Gecode::gecode)
```

### Component targets

Canonical component names:

- `support`, `kernel`, `search`, `int`, `set`, `float`, `minimodel`, `driver`,
  `flatzinc`, `gist`

Example:

```cmake
find_package(Gecode CONFIG REQUIRED COMPONENTS driver)
add_executable(app main.cpp)
target_link_libraries(app PRIVATE Gecode::gecodedriver)
```

Legacy component spellings are also accepted in `COMPONENTS`:

- `gecodesupport`, `gecodekernel`, `gecodesearch`, `gecodeint`, `gecodeset`,
  `gecodefloat`, `gecodeminimodel`, `gecodedriver`, `gecodeflatzinc`,
  `gecodegist`

### Package location hints

Use one of:

- `-DCMAKE_PREFIX_PATH=/path/to/install`
- `-DGecode_ROOT=/path/to/install`

Packagers that want to avoid any Qt dependency in the exported package should
configure with:

```bash
cmake -S . -B build -DGECODE_ENABLE_QT=OFF -DGECODE_ENABLE_GIST=OFF
```

### Version checks

Use `Gecode_VERSION` from package config:

```cmake
find_package(Gecode CONFIG REQUIRED)
message(STATUS "Gecode version: ${Gecode_VERSION}")
```

Do not rely on parsing installed headers such as `config.hpp` for version checks.

## Migration Notes

### Deprecated cache variable aliases

| Deprecated | Replacement |
|---|---|
| `ENABLE_THREADS` | `GECODE_ENABLE_THREAD` |
| `ENABLE_GIST` | `GECODE_ENABLE_GIST` |
| `BUILD_EXAMPLES` | `GECODE_ENABLE_EXAMPLES` |
| `ENABLE_CPPROFILER` | `GECODE_ENABLE_CPPROFILER` |

Deprecation horizon:

- These aliases are accepted for the 6.x line to preserve migration compatibility.
- New configurations should use the `GECODE_*` names.

## Troubleshooting

- `Target "..." links to Gecode::gecode but the target was not found`:
  - Ensure you are using an installation built with this CMake export layout.
  - Check `CMAKE_PREFIX_PATH` / `Gecode_ROOT` points to the intended install.
- `find_package(Gecode COMPONENTS ...)` fails:
  - Verify requested component is enabled in the installed build.
  - For optional modules (`flatzinc`, `gist`, `float` with MPFR), ensure dependencies were available.
- Qt/Gist issues:
  - `GECODE_ENABLE_QT=ON` and `GECODE_ENABLE_GIST=ON` are requirement modes;
    use `AUTO` when dependency discovery should be best-effort.
  - `GECODE_ENABLE_GIST=ON` requires Qt discovery and a usable Qt link
    interface.
  - Gecode requires Qt 5.15 or newer. Qt 6 is preferred for new builds.
  - On macOS, Gist defaults to `OFF`; enable it explicitly once the local
    Qt/OpenGL framework setup is known to link. Current macOS SDKs require a
    current Qt 6 build; older Qt packages can still reference removed system
    frameworks.
- MPFR issues:
  - Use `CMAKE_PREFIX_PATH`, `MPFR_ROOT`, or toolchain include/link paths so
    `find_package(MPFR)` succeeds.
  - Installed packages built with MPFR first try config-mode MPFR discovery,
    then the installed `FindMPFR.cmake`. vcpkg consumers can rely on the active
    vcpkg toolchain or `VCPKG_ROOT`/`VCPKG_TARGET_TRIPLET` to provide the MPFR
    and GMP roots.
