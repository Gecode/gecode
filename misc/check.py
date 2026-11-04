#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Format and check Gecode sources with LLVM 22 and the configured CMake build.

Formatting includes owned headers and generated variable headers. Vendored code
and the Bison/Flex outputs retain their upstream formatting. Lint covers owned
translation units in the selected compilation database and their owned headers;
optional features and platform branches require corresponding build settings.
The all command runs CTest, installed-package fixtures, and documentation
(Doxygen >= 1.18 required). check-standard and check-sweep remain separate
deliberate long runs.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
LLVM_MAJOR = 22
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx"}
HEADER_SUFFIXES = {".h", ".hh", ".hpp", ".hxx"}
GENERATED_PARSER = {
    "gecode/flatzinc/parser.tab.cpp",
    "gecode/flatzinc/parser.tab.hpp",
    "gecode/flatzinc/lexer.yy.cpp",
}


def run(command: list[str], *, env: dict[str, str] | None = None) -> None:
    subprocess.run(command, cwd=ROOT, env=env, check=True)


def llvm_tool(name: str) -> str:
    override = os.environ.get(name.upper().replace("-", "_"))
    path = shutil.which(override) if override else (
        shutil.which(f"{name}-{LLVM_MAJOR}") or shutil.which(name)
    )
    if not path:
        raise ValueError(f"{name} {LLVM_MAJOR} is required; add it to PATH or set "
                         f"{name.upper().replace('-', '_')}")
    version = subprocess.check_output([path, "--version"], text=True)
    if not re.search(rf"\bversion {LLVM_MAJOR}\.", version):
        raise ValueError(f"{name} {LLVM_MAJOR} is required; found {version.strip()}")
    return path


def owned_files() -> list[Path]:
    output = subprocess.check_output(["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"], cwd=ROOT)
    files = []
    for name in sorted(set(output.decode().split("\0"))):
        path = Path(name)
        if (path.suffix in SOURCE_SUFFIXES | HEADER_SUFFIXES
                and name not in GENERATED_PARSER
                and (ROOT / path).is_file()):
            files.append(path)
    return files


def format_sources(fix: bool) -> None:
    formatter = llvm_tool("clang-format")
    files = owned_files()
    if not files:
        raise ValueError("No tracked Gecode sources found")
    print(f"{'Formatting' if fix else 'Checking formatting of'} {len(files)} files", flush=True)
    options = ["-i"] if fix else ["--dry-run", "--Werror", "--ferror-limit=1"]
    # Small batches avoid command-line limits on Windows.
    for start in range(0, len(files), 100):
        run([formatter, "--style=file", "--fail-on-incomplete-format", *options,
             *(str(path) for path in files[start:start + 100])])


def lint(build_dir: Path, jobs: int, *, report_missing: bool = True) -> None:
    configuration = ROOT / ".clang-tidy"
    if not configuration.is_file():
        raise ValueError(f"Missing {configuration}")
    tidy = llvm_tool("clang-tidy")
    runner = os.environ.get("RUN_CLANG_TIDY")
    if not runner:
        directory = Path(tidy).parent
        runner = next((str(directory / name) for name in
                       (f"run-clang-tidy-{LLVM_MAJOR}", "run-clang-tidy", "run-clang-tidy.py")
                       if (directory / name).is_file()), None)
    if not runner:
        raise ValueError("run-clang-tidy from LLVM 22 is required; set RUN_CLANG_TIDY")
    database = build_dir / "compile_commands.json"
    if not database.is_file():
        raise ValueError(f"Missing {database}; configure CMake with "
                         "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON and Ninja or Makefiles")
    entries = json.loads(database.read_text())
    compiled = {(Path(entry["directory"]) / entry["file"]).resolve() for entry in entries}
    sources = {ROOT / path for path in owned_files() if path.suffix in SOURCE_SUFFIXES}
    selected = sorted(sources & compiled)
    if not selected:
        raise ValueError("The compilation database contains no tracked Gecode source files")
    missing = sorted(sources - compiled)
    print(f"Linting {len(selected)} source files and their owned headers", flush=True)
    print("Coverage follows this build's enabled features and platform branches.", flush=True)
    if missing and report_missing:
        print(f"{len(missing)} owned source files are outside this build:", flush=True)
        for path in missing:
            print(f"  {path.relative_to(ROOT)}", flush=True)
    separator = r"[\\/]"

    def path_pattern(path: Path) -> str:
        return separator.join(re.escape(part) for part in path.as_posix().split("/"))

    excluded_sources = "|".join(
        path_pattern(Path(path)) + "$" for path in sorted(GENERATED_PARSER)
    )
    owned_directory = "(gecode|test|examples|tools)" + separator
    source_filter = ("^" + path_pattern(ROOT) + separator + "(?!" + excluded_sources
                     + ")" + owned_directory)
    # LLVM header filters use POSIX regular expressions, without lookarounds.
    header_filter = path_pattern(ROOT) + separator + owned_directory
    generated_headers = path_pattern(build_dir / "gecode/kernel") + separator + r"var-(type|imp)\.hpp$"
    exclude_filter = (separator + path_pattern(Path("gecode/flatzinc"))
                      + separator + r"(parser\.tab\.(cpp|hpp)|lexer\.yy\.cpp)$")
    runner_command = [sys.executable, runner] if os.name == "nt" else [runner]
    run([*runner_command, "-clang-tidy-binary", tidy, "-config-file", str(configuration),
         "-p", str(build_dir), "-j", str(jobs), "-extra-arg=-UNDEBUG",
         "-header-filter", f"({header_filter}|{generated_headers})",
         "-exclude-header-filter", exclude_filter, "-source-filter", source_filter,
         "-warnings-as-errors", "*"])
    # Line filters use physical generated-file locations, not #line filenames.
    for generated, grammar in (("parser.tab.cpp", "parser.yxx"), ("lexer.yy.cpp", "lexer.lxx")):
        source = ROOT / "gecode/flatzinc" / generated
        if source not in compiled:
            continue
        ranges = []
        start = None
        lines = source.read_text().splitlines()
        for number, line in enumerate(lines, 1):
            directive = re.match(r'#line \d+ "([^"]+)"', line)
            if directive:
                if start is not None and number > start:
                    ranges.append([start, number - 1])
                start = number + 1 if Path(directive[1]).name == grammar else None
        if start is not None:
            ranges.append([start, len(lines)])
        if not ranges:
            raise ValueError(f"No owned source mappings found in {source}")
        print(f"Linting C++ actions from {grammar}", flush=True)
        run([tidy, "--extra-arg=-UNDEBUG", "-p", str(build_dir),
             f"--config-file={configuration}",
             "--line-filter=" + json.dumps([{"name": str(source), "lines": ranges}]),
             "--warnings-as-errors=*", str(source)])
    # This owned header is otherwise reached only through excluded Bison/Flex
    # translation units. Clang borrows the parser's compilation command.
    if ROOT / "gecode/flatzinc/parser.tab.cpp" in compiled:
        parser_header = ROOT / "gecode/flatzinc/parser.hh"
        print("Linting parser.hh with the parser compilation settings", flush=True)
        run([tidy, "--extra-arg=-UNDEBUG", "-p", str(build_dir),
             f"--config-file={configuration}",
             f"--header-filter=({header_filter}|{generated_headers})",
             f"--exclude-header-filter={exclude_filter}", "--warnings-as-errors=*",
             str(parser_header)])


def check_packages(build_dir: Path, jobs: int, config: str | None,
                   settings: dict[str, str]) -> None:
    prefix = build_dir / "check-install"
    install = ["cmake", "--install", str(build_dir), "--prefix", str(prefix)]
    if config:
        install += ["--config", config]
    run(install)
    variant = "shared" if settings.get("GECODE_BUILD_SHARED", "").upper() in {
        "ON", "TRUE", "1", "YES"
    } else "static"
    build_type = config or settings.get("CMAKE_BUILD_TYPE", "")
    environment = os.environ.copy()
    if os.name == "nt":
        environment["PATH"] = str(prefix / "bin") + os.pathsep + environment.get("PATH", "")
    for package in ("allocator", "public-test-component"):
        package_build = build_dir / "check-packages" / package
        print(f"Checking installed package: {package}", flush=True)
        configure = ["cmake", "-S", str(ROOT / "test/package" / package),
                     "-B", str(package_build), "-G", "Ninja",
                     "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                     f"-DCMAKE_BUILD_TYPE={build_type}", f"-DCMAKE_PREFIX_PATH={prefix}"]
        for setting in ("CMAKE_C_COMPILER", "CMAKE_CXX_COMPILER", "CMAKE_C_FLAGS",
                        "CMAKE_CXX_FLAGS", "CMAKE_OSX_SYSROOT", "CMAKE_OSX_ARCHITECTURES",
                        "CMAKE_OSX_DEPLOYMENT_TARGET", "CMAKE_TOOLCHAIN_FILE"):
            if settings.get(setting):
                configure.append(f"-D{setting}={settings[setting]}")
        if package == "allocator":
            configure.append(f"-DGECODE_LINK_VARIANT={variant}")
            mimalloc = settings.get("GECODE_ENABLE_MIMALLOC", "").upper() in {"ON", "TRUE", "1", "YES"}
            configure.append(f"-DGECODE_EXPECT_MIMALLOC={'ON' if mimalloc else 'OFF'}")
            if mimalloc:
                source = settings.get("FETCHCONTENT_SOURCE_DIR_GECODE_MIMALLOC") or str(
                    Path(settings.get("FETCHCONTENT_BASE_DIR", str(build_dir / "_deps")))
                    / "gecode_mimalloc-src"
                )
                configure.append(f"-DMIMALLOC_SOURCE_DIR={source}")
        run(configure)
        run(["cmake", "--build", str(package_build), "--parallel", str(jobs)])
        run(["ctest", "--test-dir", str(package_build), "--output-on-failure"], env=environment)
        lint(package_build, jobs, report_missing=False)


def check_all(build_dir: Path, jobs: int, config: str | None) -> None:
    cache = build_dir / "CMakeCache.txt"
    if not cache.is_file():
        raise ValueError(f"Missing {cache}; configure the CMake build first")
    settings = dict(re.findall(r"^([^#/:]+):[^=]+=(.*)$", cache.read_text(), re.MULTILINE))
    for option in ("BUILD_TESTING", "GECODE_ENABLE_SEARCH", "GECODE_ENABLE_INT_VARS",
                   "GECODE_ENABLE_MINIMODEL", "GECODE_ENABLE_DRIVER",
                   "GECODE_INSTALL", "GECODE_ENABLE_ALLOCATOR"):
        if settings.get(option, "").upper() not in {"ON", "TRUE", "1", "YES"}:
            raise ValueError(f"The all command requires {option}=ON for runtime and package checks")
    if settings.get("CMAKE_CONFIGURATION_TYPES") and not config:
        raise ValueError("The all command requires --config for multi-config builds")
    format_sources(False)
    run(["uv", "run", "misc/tidy.py", "--check"])
    run(["bash", ".github/ci/generated-source-freshness.sh"])
    # Build generated headers before clang-tidy parses dependent sources.
    build = ["cmake", "--build", str(build_dir), "--parallel", str(jobs)]
    if config:
        build += ["--config", config]
    run(build)
    lint(build_dir, jobs)
    ctest = ["ctest", "--test-dir", str(build_dir), "--output-on-failure"]
    if config:
        ctest += ["--build-config", config]
    run(ctest)
    check_packages(build_dir, jobs, config, settings)
    run([*build, "--target", "doc"])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("format", "format-check", "lint", "all"))
    parser.add_argument("--build-dir", type=Path, help="configured CMake build (required for lint/all)")
    parser.add_argument("--jobs", "-j", type=int, default=4, help="parallel build/lint jobs (default: 4)")
    parser.add_argument("--config", help="configuration for multi-config CMake builds")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if args.command in {"lint", "all"} and args.build_dir is None:
        parser.error("--build-dir is required for lint/all")
    try:
        if args.command in {"format", "format-check"}:
            format_sources(args.command == "format")
        elif args.command == "lint":
            lint(args.build_dir.resolve(), args.jobs)
        else:
            check_all(args.build_dir.resolve(), args.jobs, args.config)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"check: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
