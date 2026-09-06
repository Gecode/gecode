#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

VERIFIER_PREFIX = "[verify-installed-legacy-test-component]"
EXPECTED_TEST_NAME = "Package::Equality"
LIBRARIES = [
    "gecodetestint",
    "gecodetest",
    "gecodesearch",
    "gecodeint",
    "gecodekernel",
    "gecodesupport",
]


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
    env_summary: dict[str, str] | None = None,
) -> "NoReturn":
    sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: FAIL - {message}\n")
    if command is not None:
        sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: command: {format_command(command)}\n")
    if cwd is not None:
        sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: cwd: {cwd}\n")
    if env_summary is not None:
        sys.stderr.write(
            f"{VERIFIER_PREFIX} {phase}: runtime-env: {json.dumps(env_summary, sort_keys=True)}\n"
        )
    if result is not None:
        sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: exit: {result.returncode}\n")
        write_stream(sys.stderr, f"{VERIFIER_PREFIX} {phase}: stdout:\n{result.stdout}")
        write_stream(sys.stderr, f"{VERIFIER_PREFIX} {phase}: stderr:\n{result.stderr}")
    raise SystemExit(1)



def assert_phase(condition: bool, phase: str, message: str) -> None:
    if not condition:
        fail_phase(phase, message)



def run_phase(
    phase: str,
    command: list[str],
    *,
    cwd: Path,
    env: dict[str, str] | None = None,
    expect_success: bool = True,
    env_summary: dict[str, str] | None = None,
) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        command,
        cwd=cwd,
        env=env,
        text=True,
        capture_output=True,
    )
    if expect_success and result.returncode != 0:
        fail_phase(
            phase,
            "command failed",
            command=command,
            cwd=cwd,
            result=result,
            env_summary=env_summary,
        )
    if not expect_success and result.returncode == 0:
        fail_phase(
            phase,
            "command unexpectedly succeeded",
            command=command,
            cwd=cwd,
            result=result,
            env_summary=env_summary,
        )
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: ok\n")
    return result



def resolve_library_dir(prefix: Path) -> Path:
    phase = "inputs"
    required = ["libgecodetest.a", "libgecodetestint.a"]
    candidates: dict[str, list[Path]] = {}
    for filename in required:
        matches = sorted(path.resolve() for path in prefix.rglob(filename) if path.is_file())
        assert_phase(matches, phase, f"missing installed library: {prefix / filename}")
        candidates[filename] = matches

    lib_dirs = {path.parent for matches in candidates.values() for path in matches}
    assert_phase(len(lib_dirs) == 1, phase, f"expected one installed library directory, found: {sorted(str(path) for path in lib_dirs)}")
    return next(iter(lib_dirs))



def build_runtime_env(prefix_lib: Path) -> tuple[dict[str, str], dict[str, str]]:
    env = os.environ.copy()
    prefix_lib_str = str(prefix_lib)
    env["DYLD_LIBRARY_PATH"] = prefix_lib_str
    env["LD_LIBRARY_PATH"] = prefix_lib_str
    env_summary = {
        "DYLD_LIBRARY_PATH": env["DYLD_LIBRARY_PATH"],
        "LD_LIBRARY_PATH": env["LD_LIBRARY_PATH"],
    }
    return env, env_summary



def create_workspace(build_root: Path, source: Path, *, mode: str) -> tuple[tempfile.TemporaryDirectory[str], Path, Path]:
    artifact_dir = build_root / "installed-legacy-verifier"
    artifact_dir.mkdir(parents=True, exist_ok=True)
    temp_dir = tempfile.TemporaryDirectory(prefix=f"{build_root.name}-{mode}-")
    workspace = Path(temp_dir.name).resolve()
    assert_phase(not workspace.is_relative_to(source), "workspace", f"workspace must stay outside the source tree: {workspace}")
    (artifact_dir / "last-workspace.txt").write_text(f"{workspace}\n")
    (artifact_dir / "last-mode.txt").write_text(f"{mode}\n")
    sys.stdout.write(f"{VERIFIER_PREFIX} workspace: path={workspace}\n")
    return temp_dir, workspace, artifact_dir



def resolve_consumer_binary(binary: Path) -> Path:
    if binary.exists():
        return binary
    exe_binary = binary.with_suffix(binary.suffix + ".exe")
    if exe_binary.exists():
        return exe_binary
    fail_phase("build", f"missing built consumer binary: {binary}")



def assert_compiler_command_surface(
    command: list[str],
    *,
    source: Path,
    workspace: Path,
    prefix: Path,
    prefix_include: Path,
    expected_library_dirs: list[Path],
) -> None:
    phase = "include-leakage"
    workspace = workspace.resolve()
    prefix = prefix.resolve()
    prefix_include = prefix_include.resolve()
    source = source.resolve()
    expected_library_dirs = [path.resolve() for path in expected_library_dirs]

    include_dirs: list[Path] = []
    library_dirs: list[Path] = []
    workspace_sources: list[Path] = []
    output_paths: list[Path] = []
    i = 0
    while i < len(command):
        argument = command[i]
        include_path: str | None = None
        library_path: str | None = None
        output_path: str | None = None
        if argument == "-I":
            i += 1
            include_path = command[i]
        elif argument.startswith("-I") and argument != "-I":
            include_path = argument[2:]
        elif argument == "-L":
            i += 1
            library_path = command[i]
        elif argument.startswith("-L") and argument != "-L":
            library_path = argument[2:]
        elif argument == "-o":
            i += 1
            output_path = command[i]
        elif argument.endswith((".cpp", ".cc", ".cxx", ".c")):
            candidate = Path(argument)
            candidate = (workspace / candidate).resolve() if not candidate.is_absolute() else candidate.resolve()
            workspace_sources.append(candidate)
        i += 1

        if include_path is not None:
            candidate = Path(include_path)
            include_dirs.append((workspace / candidate).resolve() if not candidate.is_absolute() else candidate.resolve())
        if library_path is not None:
            candidate = Path(library_path)
            library_dirs.append((workspace / candidate).resolve() if not candidate.is_absolute() else candidate.resolve())
        if output_path is not None:
            candidate = Path(output_path)
            output_paths.append((workspace / candidate).resolve() if not candidate.is_absolute() else candidate.resolve())

    assert_phase(include_dirs == [prefix_include], phase, f"unexpected include directories in compiler command: {[str(path) for path in include_dirs]}")
    assert_phase(library_dirs == expected_library_dirs, phase, f"unexpected library directories in compiler command: {[str(path) for path in library_dirs]}")
    assert_phase(workspace_sources, phase, "compiler command did not reference any workspace source files")
    assert_phase(output_paths, phase, "compiler command did not set an output path")
    assert_phase(
        all(path.is_relative_to(workspace) for path in [*workspace_sources, *output_paths]),
        phase,
        f"compiler command referenced non-workspace source/output paths: {[str(path) for path in [*workspace_sources, *output_paths]]}",
    )

    candidate_paths = [*include_dirs, *library_dirs, *workspace_sources, *output_paths]
    disallowed_source_paths = [
        path
        for path in candidate_paths
        if path.is_relative_to(source) and not path.is_relative_to(prefix) and not path.is_relative_to(workspace)
    ]
    assert_phase(
        not disallowed_source_paths,
        phase,
        f"compiler command referenced disallowed source-tree paths: {[str(path) for path in disallowed_source_paths]}",
    )
    sys.stdout.write(
        f"{VERIFIER_PREFIX} {phase}: command={format_command(command)} include={prefix_include} lib-dirs={[str(path) for path in expected_library_dirs]}\n"
    )



def run_prefix_surface(source: Path, prefix: Path) -> None:
    command = [
        sys.executable,
        str((source / "test/verify-legacy-install-surface.py").resolve()),
        "--prefix",
        str(prefix),
    ]
    result = run_phase("prefix-surface", command, cwd=source)
    write_stream(sys.stdout, result.stdout)
    write_stream(sys.stderr, result.stderr)



def write_positive_consumer(source: Path, workspace: Path) -> Path:
    source_fixture = (source / "test/package/public-test-component/consumer-smoke.cpp").resolve()
    destination = workspace / "consumer-smoke.cpp"
    shutil.copy2(source_fixture, destination)
    return destination



def compiler_command_prefix() -> list[str]:
    cxx = os.environ.get("CXX", "c++")
    return shlex.split(cxx)



def build_positive_consumer(
    source: Path,
    workspace: Path,
    prefix: Path,
    prefix_lib: Path,
    artifact_dir: Path,
) -> Path:
    consumer_source = write_positive_consumer(source, workspace)
    consumer_binary = workspace / "consumer-smoke"
    command = [
        *compiler_command_prefix(),
        "-std=c++17",
        f"-I{prefix / 'include'}",
        str(consumer_source),
        f"-L{prefix_lib}",
        *[f"-l{name}" for name in LIBRARIES],
        "-o",
        str(consumer_binary),
    ]
    assert_compiler_command_surface(
        command,
        source=source,
        workspace=workspace,
        prefix=prefix,
        prefix_include=prefix / "include",
        expected_library_dirs=[prefix_lib],
    )
    (artifact_dir / "last-compile-command.txt").write_text(f"{format_command(command)}\n")
    result = run_phase("build", command, cwd=workspace)
    write_stream(sys.stdout, result.stdout)
    write_stream(sys.stderr, result.stderr)
    binary = resolve_consumer_binary(consumer_binary)
    sys.stdout.write(f"{VERIFIER_PREFIX} build: binary={binary}\n")
    return binary



def run_list_phase(consumer_binary: Path, workspace: Path, runtime_env: dict[str, str], env_summary: dict[str, str]) -> None:
    result = run_phase(
        "list",
        [str(consumer_binary), "-list"],
        cwd=workspace,
        env=runtime_env,
        env_summary=env_summary,
    )
    assert_phase(EXPECTED_TEST_NAME in result.stdout, "list", f"-list output missing {EXPECTED_TEST_NAME!r}")
    sys.stdout.write(f"{VERIFIER_PREFIX} list: discovered={EXPECTED_TEST_NAME}\n")



def run_filtered_phase(
    consumer_binary: Path,
    workspace: Path,
    runtime_env: dict[str, str],
    env_summary: dict[str, str],
) -> None:
    result = run_phase(
        "filtered-run",
        [str(consumer_binary), "-test", EXPECTED_TEST_NAME, "-iter", "1", "-stop", "true"],
        cwd=workspace,
        env=runtime_env,
        env_summary=env_summary,
    )
    assert_phase(EXPECTED_TEST_NAME in result.stdout, "filtered-run", "filtered run did not print the selected downstream test")
    assert_phase("+" in result.stdout, "filtered-run", "filtered run did not report success")
    sys.stdout.write(f"{VERIFIER_PREFIX} filtered-run: executed={EXPECTED_TEST_NAME}\n")



def write_unsupported_consumer(workspace: Path) -> Path:
    source = workspace / "unsupported-header.cpp"
    source.write_text(
        "#include <test/set.hh>\n\n"
        "int main() {\n"
        "  return 0;\n"
        "}\n"
    )
    return source



def assert_missing_header_diagnostic(result: subprocess.CompletedProcess[str]) -> None:
    phase = "unsupported-header"
    combined_output = f"{result.stdout}\n{result.stderr}"
    assert_phase("test/set.hh" in combined_output, phase, "missing expected unsupported-header diagnostic target")
    assert_phase(
        "file not found" in combined_output or "No such file or directory" in combined_output,
        phase,
        "missing missing-header diagnostic wording",
    )
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: diagnostic=test/set.hh missing\n")



def run_unsupported_header_mode(
    source: Path,
    workspace: Path,
    prefix: Path,
    artifact_dir: Path,
) -> None:
    unsupported_source = write_unsupported_consumer(workspace)
    unsupported_object = workspace / "unsupported-header.o"
    command = [
        *compiler_command_prefix(),
        "-std=c++17",
        f"-I{prefix / 'include'}",
        str(unsupported_source),
        "-c",
        "-o",
        str(unsupported_object),
    ]
    assert_compiler_command_surface(
        command,
        source=source,
        workspace=workspace,
        prefix=prefix,
        prefix_include=prefix / "include",
        expected_library_dirs=[],
    )
    (artifact_dir / "last-runtime-env.json").write_text("{}\n")
    (artifact_dir / "last-compile-command.txt").write_text(f"{format_command(command)}\n")
    result = run_phase("unsupported-header", command, cwd=workspace, expect_success=False)
    assert_missing_header_diagnostic(result)



def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True)
    parser.add_argument("--build-root", required=True)
    parser.add_argument("--prefix", required=True)
    parser.add_argument("--mode", choices=["positive", "unsupported-header"], default="positive")
    return parser.parse_args()



def main() -> int:
    args = parse_args()
    source = Path(args.source).resolve()
    build_root = Path(args.build_root).resolve()
    prefix = Path(args.prefix).resolve()

    assert_phase(source.is_dir(), "inputs", f"missing source tree: {source}")
    assert_phase(prefix.is_dir(), "inputs", f"missing installed prefix: {prefix}")
    build_root.mkdir(parents=True, exist_ok=True)

    prefix_lib = resolve_library_dir(prefix)
    run_prefix_surface(source, prefix)

    temp_dir, workspace, artifact_dir = create_workspace(build_root, source, mode=args.mode)
    with temp_dir:
        if args.mode == "unsupported-header":
            run_unsupported_header_mode(source, workspace, prefix, artifact_dir)
            return 0

        consumer_binary = build_positive_consumer(source, workspace, prefix, prefix_lib, artifact_dir)
        runtime_env, env_summary = build_runtime_env(prefix_lib)
        (artifact_dir / "last-runtime-env.json").write_text(json.dumps(env_summary, indent=2, sort_keys=True) + "\n")
        sys.stdout.write(
            f"{VERIFIER_PREFIX} runtime-env: {json.dumps(env_summary, sort_keys=True)}\n"
        )
        run_list_phase(consumer_binary, workspace, runtime_env, env_summary)
        run_filtered_phase(consumer_binary, workspace, runtime_env, env_summary)
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
