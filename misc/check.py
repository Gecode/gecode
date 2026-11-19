#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Format and check Gecode sources with LLVM 22 and the configured CMake build.

Formatting includes owned headers and generated variable headers. Vendored code
and the Bison/Flex outputs retain their upstream formatting. Lint covers owned
translation units in the selected compilation database and their owned headers;
optional features and platform branches require corresponding build settings.
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


def run(command: list[str]) -> None:
    subprocess.run(command, cwd=ROOT, check=True)


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
                and not name.startswith("gecode/third-party/")
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


def lint(build_dir: Path, jobs: int) -> None:
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
    if missing:
        print(f"{len(missing)} owned source files are outside this build:", flush=True)
        for path in missing:
            print(f"  {path.relative_to(ROOT)}", flush=True)
    separator = r"[\\/]"

    def path_pattern(path: Path) -> str:
        return separator.join(re.escape(part) for part in path.as_posix().split("/"))

    excluded_sources = path_pattern(Path("gecode/third-party")) + separator + "|" + "|".join(
        path_pattern(Path(path)) + "$" for path in sorted(GENERATED_PARSER)
    )
    owned_directory = "(gecode|test|examples|tools)" + separator
    source_filter = ("^" + path_pattern(ROOT) + separator + "(?!" + excluded_sources
                     + ")" + owned_directory)
    # LLVM header filters use POSIX regular expressions, without lookarounds.
    header_filter = path_pattern(ROOT) + separator + owned_directory
    generated_headers = path_pattern(build_dir / "gecode/kernel") + separator + r"var-(type|imp)\.hpp$"
    exclude_filter = (separator + path_pattern(Path("gecode/third-party")) + separator
                      + "|" + separator + path_pattern(Path("gecode/flatzinc"))
                      + separator + r"(parser\.tab\.(cpp|hpp)|lexer\.yy\.cpp)$")
    runner_command = [sys.executable, runner] if os.name == "nt" else [runner]
    run([*runner_command, "-clang-tidy-binary", tidy, "-config-file", str(configuration),
         "-p", str(build_dir), "-j", str(jobs),
         "-header-filter", f"({header_filter}|{generated_headers})",
         "-exclude-header-filter", exclude_filter, "-source-filter", source_filter,
         "-warnings-as-errors", "*"])


def check_all(build_dir: Path, jobs: int, config: str | None) -> None:
    cache = build_dir / "CMakeCache.txt"
    if not cache.is_file():
        raise ValueError(f"Missing {cache}; configure the CMake build first")
    settings = dict(re.findall(r"^([^#/:]+):[^=]+=(.*)$", cache.read_text(), re.MULTILINE))
    for option in ("BUILD_TESTING", "GECODE_ENABLE_SEARCH", "GECODE_ENABLE_INT_VARS",
                   "GECODE_ENABLE_MINIMODEL", "GECODE_ENABLE_DRIVER"):
        if settings.get(option, "").upper() not in {"ON", "TRUE", "1", "YES"}:
            raise ValueError(f"The all command requires {option}=ON for runtime checks")
    format_sources(False)
    run(["uv", "run", "misc/tidy.py", "--check"])
    run(["bash", ".github/ci/generated-source-freshness.sh"])
    # Build generated headers before clang-tidy parses dependent sources.
    build = ["cmake", "--build", str(build_dir), "--parallel", str(jobs)]
    if config:
        build += ["--config", config]
    run(build)
    lint(build_dir, jobs)
    run([*build, "--target", "check-standard"])
    ctest = ["ctest", "--test-dir", str(build_dir), "--output-on-failure"]
    if config:
        ctest += ["--build-config", config]
    run(ctest)


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
