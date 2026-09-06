#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import shlex
import shutil
import subprocess
import sys
from pathlib import Path


EXPECTED_HEADERS = [
    "include/test/int.hh",
    "include/test/int.hpp",
    "include/test/test.hh",
    "include/test/test.hpp",
]
EXPECTED_METADATA = [
    "lib/cmake/Gecode/GecodeConfig.cmake",
    "lib/cmake/Gecode/GecodeTargets.cmake",
]
EXPECTED_TEST_NAME = "Int::Package::ConsumerSmoke"
VERIFIER_PREFIX = "[verify-installed-test-component]"


def format_command(command: list[str]) -> str:
    return shlex.join(command)


def write_stream(stream, text: str) -> None:
    if text:
        stream.write(text)
        if not text.endswith("\n"):
            stream.write("\n")


def fail_phase(
    phase: str,
    message: str,
    *,
    command: list[str] | None = None,
    cwd: Path | None = None,
    result: subprocess.CompletedProcess[str] | None = None,
) -> "NoReturn":
    sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: FAIL - {message}\n")
    if command is not None:
        sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: command: {format_command(command)}\n")
    if cwd is not None:
        sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: cwd: {cwd}\n")
    if result is not None:
        sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: exit: {result.returncode}\n")
        write_stream(sys.stderr, f"{VERIFIER_PREFIX} {phase}: stdout:\n{result.stdout}")
        write_stream(sys.stderr, f"{VERIFIER_PREFIX} {phase}: stderr:\n{result.stderr}")
    raise SystemExit(1)


def run_phase(
    phase: str,
    command: list[str],
    *,
    cwd: Path | None = None,
    expect_success: bool = True,
) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        command,
        cwd=cwd,
        text=True,
        capture_output=True,
    )
    if expect_success and result.returncode != 0:
        fail_phase(phase, "command failed", command=command, cwd=cwd, result=result)
    if not expect_success and result.returncode == 0:
        fail_phase(phase, "command unexpectedly succeeded", command=command, cwd=cwd, result=result)
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: ok\n")
    return result


def assert_phase(condition: bool, phase: str, message: str) -> None:
    if not condition:
        fail_phase(phase, message)


def assert_prefix_surface(prefix: Path) -> None:
    phase = "prefix-surface"
    for rel in [*EXPECTED_HEADERS, *EXPECTED_METADATA]:
        path = prefix / rel
        assert_phase(path.exists(), phase, f"missing installed path: {path}")

    installed_test_headers = sorted(
        path.relative_to(prefix).as_posix()
        for path in (prefix / "include" / "test").glob("*")
        if path.is_file()
    )
    assert_phase(installed_test_headers == EXPECTED_HEADERS, phase, f"unexpected installed test headers: {installed_test_headers}")

    targets = (prefix / "lib/cmake/Gecode/GecodeTargets.cmake").read_text()
    assert_phase("Gecode::gecodetest" in targets, phase, "missing Gecode::gecodetest export")
    assert_phase("Gecode::gecodetestint" in targets, phase, "missing Gecode::gecodetestint export")

    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: ok\n")


def iter_include_dirs(entry: dict[str, object]) -> list[Path]:
    directory = Path(str(entry["directory"])).resolve()
    arguments = entry.get("arguments")
    if arguments is None:
        arguments = shlex.split(str(entry["command"]))

    include_dirs: list[Path] = []
    i = 0
    while i < len(arguments):
        argument = str(arguments[i])
        include_path: str | None = None
        if argument == "-I" or argument == "-isystem":
            i += 1
            include_path = str(arguments[i])
        elif argument.startswith("-I") and argument != "-I":
            include_path = argument[2:]
        elif argument.startswith("-isystem") and argument != "-isystem":
            include_path = argument[len("-isystem") :]

        if include_path:
            path = Path(include_path)
            include_dirs.append((directory / path).resolve() if not path.is_absolute() else path.resolve())
        i += 1

    return include_dirs


def assert_no_source_tree_include_leakage(source: Path, consumer_build: Path, prefix: Path) -> None:
    phase = "include-leakage"
    compile_commands_path = consumer_build / "compile_commands.json"
    assert_phase(compile_commands_path.exists(), phase, f"missing compile commands: {compile_commands_path}")

    compile_commands = json.loads(compile_commands_path.read_text())
    assert_phase(bool(compile_commands), phase, "expected compile commands for installed-package consumer")

    source_root = source.resolve()
    consumer_build = consumer_build.resolve()
    prefix = prefix.resolve()
    prefix_include = (prefix / "include").resolve()

    leaked_include_dirs: set[str] = set()
    saw_prefix_include = False
    for entry in compile_commands:
        for include_dir in iter_include_dirs(entry):
            if include_dir == prefix_include or include_dir.is_relative_to(prefix_include):
                saw_prefix_include = True
            if (
                include_dir.is_relative_to(source_root)
                and not include_dir.is_relative_to(consumer_build)
                and not include_dir.is_relative_to(prefix)
            ):
                leaked_include_dirs.add(include_dir.as_posix())

    assert_phase(saw_prefix_include, phase, "consumer compile flags never referenced the installed prefix include directory")
    assert_phase(not leaked_include_dirs, phase, f"source-tree include leakage detected: {sorted(leaked_include_dirs)}")

    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: ok\n")


def configure_consumer(source: Path, build_root: Path, prefix: Path, *, require_unknown_component: bool) -> tuple[Path, Path, subprocess.CompletedProcess[str]]:
    consumer_source = source / "test/package/public-test-component"
    consumer_build = build_root / "consumer"
    cmake_dir = prefix / "lib/cmake/Gecode"

    shutil.rmtree(build_root, ignore_errors=True)
    consumer_build.mkdir(parents=True, exist_ok=True)

    configure_command = [
        "cmake",
        "-S",
        str(consumer_source),
        "-B",
        str(consumer_build),
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        f"-DGecode_DIR={cmake_dir}",
        f"-DCMAKE_PREFIX_PATH={prefix}",
    ]
    if require_unknown_component:
        configure_command.append("-DREQUIRE_UNKNOWN_COMPONENT=ON")

    result = run_phase(
        "configure-missing-component" if require_unknown_component else "configure",
        configure_command,
        expect_success=not require_unknown_component,
    )
    return consumer_source, consumer_build, result


def assert_missing_component_failure(result: subprocess.CompletedProcess[str]) -> None:
    phase = "configure-missing-component"
    combined_output = f"{result.stdout}\n{result.stderr}"
    assert_phase(
        "Unsupported Gecode components requested: unsupported_component" in combined_output,
        phase,
        "missing unsupported-component diagnostic in configure failure output",
    )
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: diagnostic ok\n")


def build_consumer(consumer_build: Path) -> Path:
    run_phase("build", ["cmake", "--build", str(consumer_build), "--target", "consumer-smoke"])
    consumer_binary = consumer_build / "consumer-smoke"
    if not consumer_binary.exists():
        consumer_binary = consumer_build / "consumer-smoke.exe"
    assert_phase(consumer_binary.exists(), "build", f"missing built consumer binary: {consumer_binary}")
    sys.stdout.write(f"{VERIFIER_PREFIX} build: binary={consumer_binary}\n")
    return consumer_binary


def run_list_phase(consumer_binary: Path) -> None:
    result = run_phase("list", [str(consumer_binary), "-list"])
    assert_phase(EXPECTED_TEST_NAME in result.stdout, "list", f"-list output missing {EXPECTED_TEST_NAME!r}")
    sys.stdout.write(f"{VERIFIER_PREFIX} list: discovered={EXPECTED_TEST_NAME}\n")


def run_filtered_phase(consumer_binary: Path) -> None:
    result = run_phase(
        "filtered-run",
        [str(consumer_binary), "-test", EXPECTED_TEST_NAME, "-iter", "1", "-stop", "true"],
    )
    assert_phase(EXPECTED_TEST_NAME in result.stdout, "filtered-run", "filtered run did not print the selected downstream test")
    assert_phase("+" in result.stdout, "filtered-run", "filtered run did not report success")
    sys.stdout.write(f"{VERIFIER_PREFIX} filtered-run: executed={EXPECTED_TEST_NAME}\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True)
    parser.add_argument("--build-root", required=True)
    parser.add_argument("--prefix", required=True)
    parser.add_argument("--expect-missing-component-failure", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    source = Path(args.source).resolve()
    build_root = Path(args.build_root).resolve()
    prefix = Path(args.prefix).resolve()

    assert_prefix_surface(prefix)

    _, consumer_build, configure_result = configure_consumer(
        source,
        build_root,
        prefix,
        require_unknown_component=args.expect_missing_component_failure,
    )

    if args.expect_missing_component_failure:
        assert_missing_component_failure(configure_result)
        return 0

    assert_no_source_tree_include_leakage(source, consumer_build, prefix)
    consumer_binary = build_consumer(consumer_build)
    run_list_phase(consumer_binary)
    run_filtered_phase(consumer_binary)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
