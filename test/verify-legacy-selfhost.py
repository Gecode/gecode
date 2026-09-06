#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import shlex
import subprocess
import sys
from pathlib import Path

VERIFIER_PREFIX = "[verify-legacy-selfhost]"


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


def build_runtime_env(build_dir: Path) -> dict[str, str]:
    env = os.environ.copy()
    build_dir_str = str(build_dir)
    for key in ("DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"):
        existing = env.get(key)
        env[key] = build_dir_str if not existing else os.pathsep.join([build_dir_str, existing])
    existing_path = env.get("PATH")
    env["PATH"] = build_dir_str if not existing_path else os.pathsep.join([build_dir_str, existing_path])
    return env


def run_phase(
    phase: str,
    command: list[str],
    *,
    cwd: Path,
    env: dict[str, str],
) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(command, cwd=cwd, env=env, text=True, capture_output=True)
    if result.returncode != 0:
        fail_phase(phase, "command failed", command=command, cwd=cwd, result=result)
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: ok\n")
    return result


def assert_phase(condition: bool, phase: str, message: str) -> None:
    if not condition:
        fail_phase(phase, message)


def resolve_binary(build_dir: Path, relative_path: str) -> Path:
    binary = build_dir / relative_path
    if binary.exists():
        return binary
    exe_binary = binary.with_suffix(binary.suffix + ".exe")
    if exe_binary.exists():
        return exe_binary
    fail_phase("build-surface", f"missing built binary: {binary}")


def verify_runner_smoke(build_dir: Path, runtime_env: dict[str, str]) -> None:
    phase = "runner-smoke"
    binary = resolve_binary(build_dir, "test/public-runner-smoke")
    run_phase(phase, [str(binary)], cwd=build_dir, env=runtime_env)
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: binary={binary}\n")


def verify_int_smoke(build_dir: Path, runtime_env: dict[str, str]) -> None:
    phase = "int-smoke"
    binary = resolve_binary(build_dir, "test/public-int-smoke")
    run_phase(phase, [str(binary)], cwd=build_dir, env=runtime_env)
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: binary={binary}\n")


def verify_list_phase(build_dir: Path, expected_filter: str, runtime_env: dict[str, str]) -> Path:
    phase = "list"
    binary = resolve_binary(build_dir, "test/test")
    result = run_phase(phase, [str(binary), "-list"], cwd=build_dir, env=runtime_env)
    listed = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    assert_phase(bool(listed), phase, "-list produced no registered test names")
    assert_phase(
        any(expected_filter in name for name in listed),
        phase,
        f"missing expected filter in -list output: {expected_filter}",
    )
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: discovered-prefix={expected_filter}\n")
    return binary


def verify_filtered_phase(
    build_dir: Path,
    test_binary: Path,
    expected_filter: str,
    runtime_env: dict[str, str],
) -> None:
    phase = "filtered-run"
    result = run_phase(
        phase,
        [str(test_binary), "-test", expected_filter, "-iter", "1", "-stop", "true"],
        cwd=build_dir,
        env=runtime_env,
    )
    assert_phase(expected_filter in result.stdout, phase, "filtered run did not print the selected test")
    assert_phase("+" in result.stdout, phase, "filtered run did not report success")
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: executed={expected_filter}\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--expected-filter", required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    build_dir = Path(args.build_dir).resolve()
    assert_phase(build_dir.exists(), "build-surface", f"missing build directory: {build_dir}")
    runtime_env = build_runtime_env(build_dir)
    sys.stdout.write(f"{VERIFIER_PREFIX} runtime-env: library-path={build_dir}\n")

    verify_runner_smoke(build_dir, runtime_env)
    verify_int_smoke(build_dir, runtime_env)
    test_binary = verify_list_phase(build_dir, args.expected_filter, runtime_env)
    verify_filtered_phase(build_dir, test_binary, args.expected_filter, runtime_env)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
