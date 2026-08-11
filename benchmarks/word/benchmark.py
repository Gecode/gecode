#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///
"""Run and summarize the focused Gecode Word differential benchmark."""

from __future__ import annotations

import argparse
import json
import os
import re
import statistics
import subprocess
import sys
import tarfile
import time
import zipfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any

try:
    import resource
except ImportError:  # Not available on every Python platform.
    resource = None  # type: ignore[assignment]


SCHEMA_VERSION = 1
VARIANTS = ("native-word", "bool-decomposition")
TERMINAL_STATUSES = frozenset(("ok", "failed", "timeout", "error"))
INSTANCE_FIELDS = (
    "schema_version",
    "id",
    "width",
    "rounds",
    "key",
    "shift",
    "target_lo",
    "target_hi",
    "expected_solutions",
    "expected_input",
)
SAFE_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]*$")


@dataclass(frozen=True)
class Layout:
    root: Path

    @property
    def runs(self) -> Path:
        return self.root / "runs"

    @property
    def analysis(self) -> Path:
        return self.root / "analysis"

    @property
    def reports(self) -> Path:
        return self.root / "reports"

    @property
    def plots(self) -> Path:
        return self.root / "plots"

    def create(self) -> None:
        for path in (self.runs, self.analysis, self.reports, self.plots):
            path.mkdir(parents=True, exist_ok=True)


@dataclass(frozen=True)
class Case:
    corpus: str
    instance: dict[str, Any]
    variant: str
    repetition: int

    @property
    def run_id(self) -> str:
        return (
            f"{self.corpus}-{self.instance['id']}-{self.variant}"
            f"-r{self.repetition:03d}"
        )


def atomic_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
    temporary.replace(path)


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ValueError(f"expected a JSON object in {path}")
    return value


def validate_instance(value: dict[str, Any], path: Path) -> dict[str, Any]:
    missing = [field for field in INSTANCE_FIELDS if field not in value]
    if missing:
        raise ValueError(f"{path}: missing fields: {', '.join(missing)}")
    if value["schema_version"] != SCHEMA_VERSION:
        raise ValueError(f"{path}: unsupported schema_version")
    if not isinstance(value["id"], str) or not SAFE_NAME.fullmatch(value["id"]):
        raise ValueError(f"{path}: id must be a safe artifact name")
    numeric = INSTANCE_FIELDS[2:]
    if any(not isinstance(value[field], int) or isinstance(value[field], bool)
           for field in numeric):
        raise ValueError(f"{path}: instance numeric fields must be integers")
    width = value["width"]
    mask = (1 << width) - 1 if 0 < width <= 64 else 0
    if not 0 < width <= 64 or value["rounds"] <= 0:
        raise ValueError(f"{path}: width must be 1..64 and rounds positive")
    if not 0 <= value["shift"] < width:
        raise ValueError(f"{path}: shift must be smaller than width")
    for field in ("key", "target_lo", "target_hi", "expected_input"):
        if not 0 <= value[field] <= mask:
            raise ValueError(f"{path}: {field} does not fit width")
    if value["target_lo"] & ~value["target_hi"]:
        raise ValueError(f"{path}: target_lo is not a subset of target_hi")
    if value["expected_solutions"] < 0:
        raise ValueError(f"{path}: expected_solutions must be nonnegative")
    return value


def load_manifest(
    path: Path, expected_corpus: str
) -> tuple[list[dict[str, Any]], list[str]]:
    manifest = load_json(path)
    if manifest.get("schema_version") != SCHEMA_VERSION:
        raise ValueError(f"{path}: unsupported schema_version")
    if manifest.get("corpus") != expected_corpus:
        raise ValueError(f"{path}: corpus must be {expected_corpus}")
    if expected_corpus == "public" and manifest.get("redistributable") is not True:
        raise ValueError(f"{path}: public manifest must be redistributable")
    if expected_corpus == "private" and manifest.get("redistributable") is not False:
        raise ValueError(f"{path}: private manifest must not be redistributable")
    entries = manifest.get("instances")
    if not isinstance(entries, list):
        raise ValueError(f"{path}: instances must be an array")

    instances: list[dict[str, Any]] = []
    issues: list[str] = []
    base = path.parent.resolve()
    for entry in entries:
        if not isinstance(entry, dict) or not isinstance(entry.get("id"), str) \
                or not isinstance(entry.get("path"), str):
            issues.append("manifest entry lacks string id/path")
            continue
        relative = Path(entry["path"])
        instance_path = (base / relative).resolve()
        if relative.is_absolute() or base not in instance_path.parents:
            issues.append(f"{entry['id']}: path escapes corpus root")
            continue
        if not instance_path.is_file():
            issues.append(f"{entry['id']}: missing {relative}")
            continue
        try:
            instance = validate_instance(load_json(instance_path), instance_path)
        except ValueError as error:
            issues.append(str(error))
            continue
        if instance["id"] != entry["id"]:
            issues.append(f"{entry['id']}: manifest and instance ids differ")
            continue
        if expected_corpus == "public":
            if entry.get("redistributable") is not True or \
                    instance.get("redistributable") is not True:
                issues.append(f"{entry['id']}: public instance is not redistributable")
                continue
            if not entry.get("license") or not instance.get("license"):
                issues.append(f"{entry['id']}: public instance lacks license metadata")
                continue
        instances.append(instance)
    return instances, issues


def discover_corpora(
    repo_root: Path, private_root: Path | None
) -> tuple[list[tuple[str, dict[str, Any]]], dict[str, Any]]:
    public_path = repo_root / "benchmarks" / "word" / "public-manifest.json"
    public, public_issues = load_manifest(public_path, "public")
    if public_issues:
        raise ValueError("invalid checked-in public corpus: " + "; ".join(public_issues))
    discovered = [("public", instance) for instance in public]
    private_status: dict[str, Any] = {
        "requested": private_root is not None,
        "status": "not-requested",
        "root": str(private_root) if private_root else None,
        "loaded_instances": 0,
        "issues": [],
    }
    if private_root is None:
        return discovered, private_status
    if not private_root.is_dir():
        private_status["status"] = "missing-root"
        private_status["issues"] = ["private corpus root does not exist"]
        return discovered, private_status
    manifest_path = private_root / "manifest.json"
    if not manifest_path.is_file():
        private_status["status"] = "missing-manifest"
        private_status["issues"] = ["private corpus manifest.json is missing"]
        return discovered, private_status
    try:
        private, issues = load_manifest(manifest_path, "private")
    except ValueError as error:
        private_status["status"] = "invalid-manifest"
        private_status["issues"] = [str(error)]
        return discovered, private_status
    discovered.extend(("private", instance) for instance in private)
    private_status["loaded_instances"] = len(private)
    private_status["issues"] = issues
    private_status["status"] = "partial" if issues else "ready"
    return discovered, private_status


def boundary_scan(repo_root: Path) -> dict[str, Any]:
    completed = subprocess.run(
        ["git", "ls-files"], cwd=repo_root, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
    )
    tracked = completed.stdout.splitlines() if completed.returncode == 0 else []
    forbidden_prefixes = (
        "results/",
        "benchmarks/word/private-corpus/",
        "benchmarks/word/private-results/",
    )
    payloads = [path for path in tracked if path.startswith(forbidden_prefixes)]
    return {
        "git_scan_status": "ok" if completed.returncode == 0 else "unavailable",
        "tracked_private_or_result_payloads": payloads,
    }


def is_private_or_result_payload(name: str) -> bool:
    normalized = "/" + name.replace("\\", "/").strip("/") + "/"
    return any(marker in normalized for marker in (
        "/results/",
        "/benchmarks/word/private-corpus/",
        "/benchmarks/word/private-results/",
    ))


def package_names(path: Path) -> tuple[list[str], str]:
    if path.is_dir():
        return (
            [str(file.relative_to(path)) for file in path.rglob("*")
             if file.is_file()],
            "directory",
        )
    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as archive:
            return ([entry.filename for entry in archive.infolist()
                     if not entry.is_dir()], "zip")
    if tarfile.is_tarfile(path):
        with tarfile.open(path) as archive:
            return ([entry.name for entry in archive.getmembers()
                     if entry.isfile()], "tar")
    raise ValueError("unsupported package output; expected directory, tar, or zip")


def package_scan(paths: list[Path]) -> dict[str, Any]:
    if not paths:
        return {
            "status": "not-requested",
            "outputs": [],
            "private_or_result_payload_candidates": [],
        }
    outputs: list[dict[str, Any]] = []
    candidates: list[str] = []
    for requested in paths:
        path = requested.resolve()
        if not path.exists():
            outputs.append({
                "path": str(path), "status": "unavailable",
                "error": "package output does not exist",
            })
            continue
        try:
            names, kind = package_names(path)
            found = sorted(name for name in names
                           if is_private_or_result_payload(name))
            candidates.extend(f"{path}:{name}" for name in found)
            outputs.append({
                "path": str(path), "status": "ok", "kind": kind,
                "file_count": len(names), "candidates": found,
            })
        except (OSError, ValueError, tarfile.TarError, zipfile.BadZipFile) as error:
            outputs.append({
                "path": str(path), "status": "scan-error", "error": str(error),
            })
    statuses = {output["status"] for output in outputs}
    if "scan-error" in statuses:
        status = "scan-error"
    elif statuses == {"unavailable"}:
        status = "unavailable"
    elif "unavailable" in statuses:
        status = "partial"
    else:
        status = "ok"
    return {
        "status": status,
        "outputs": outputs,
        "private_or_result_payload_candidates": sorted(candidates),
    }


def resolve_binary(repo_root: Path, requested: Path | None) -> Path:
    candidates = [requested] if requested else [
        repo_root / "build" / "bin" / "word-benchmark",
        repo_root / "examples" / "word-benchmark",
    ]
    for candidate in candidates:
        if candidate is not None and candidate.is_file() and \
                os.access(candidate, os.X_OK):
            return candidate.resolve()
    raise ValueError("word-benchmark executable not found; pass --binary")


def command_for(binary: Path, case: Case) -> list[str]:
    instance = case.instance
    arguments = [str(binary), "--variant", case.variant,
                 "--instance-id", instance["id"]]
    for field in ("width", "rounds", "key", "shift", "target_lo",
                  "target_hi", "expected_solutions", "expected_input"):
        arguments.extend(["--" + field.replace("_", "-"), str(instance[field])])
    return arguments


def timed_command(
    command: list[str], timeout: float
) -> tuple[subprocess.CompletedProcess[str], float, int | None, str]:
    if sys.platform != "darwin" and Path("/usr/bin/time").is_file():
        wrapped = ["/usr/bin/time", "-v"] + command
        source = "usr-bin-time"
    else:
        wrapped = command
        source = "python-resource-children-max" if resource else "unavailable"
    started = time.perf_counter()
    completed = subprocess.run(
        wrapped, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        timeout=timeout, check=False,
    )
    elapsed = time.perf_counter() - started
    peak_kib: int | None = None
    patterns = (
        (r"Maximum resident set size \(kbytes\):\s*(\d+)", 1),
        (r"(\d+)\s+maximum resident set size", 1 / 1024),
    )
    for pattern, factor in patterns:
        match = re.search(pattern, completed.stderr, re.IGNORECASE)
        if match:
            peak_kib = int(int(match.group(1)) * factor)
            break
    if peak_kib is None and source == "python-resource-children-max" and resource:
        child_max = resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
        # ru_maxrss is bytes on macOS and KiB on other supported platforms.
        peak_kib = int(child_max / 1024) if sys.platform == "darwin" else int(child_max)
    return completed, elapsed, peak_kib, source


def parse_fixture_output(stdout: str) -> dict[str, Any]:
    lines = [line for line in stdout.splitlines() if line.strip()]
    if len(lines) != 1:
        raise ValueError("fixture output must contain exactly one JSON line")
    value = json.loads(lines[0])
    if not isinstance(value, dict) or value.get("schema_version") != SCHEMA_VERSION:
        raise ValueError("fixture returned an unsupported result")
    return value


def run_case(
    layout: Layout, binary: Path, case: Case, timeout: float, force: bool
) -> str:
    result_path = layout.runs / f"{case.run_id}.json"
    stdout_path = layout.runs / f"{case.run_id}.stdout"
    stderr_path = layout.runs / f"{case.run_id}.stderr"
    if result_path.is_file() and not force:
        try:
            previous = load_json(result_path)
            if previous.get("status") in TERMINAL_STATUSES:
                return "resumed"
        except ValueError:
            pass
    layout.runs.mkdir(parents=True, exist_ok=True)
    command = command_for(binary, case)
    try:
        completed, elapsed, peak_kib, memory_source = timed_command(command, timeout)
        stdout = completed.stdout
        stderr = completed.stderr
        fixture = parse_fixture_output(stdout)
        status = (
            "ok" if completed.returncode == 0 and fixture.get("status") == "ok"
            else "failed"
        )
        error = None
    except (subprocess.TimeoutExpired, ValueError, json.JSONDecodeError) as exception:
        elapsed = timeout if isinstance(exception, subprocess.TimeoutExpired) else 0.0
        peak_kib = None
        memory_source = "unavailable"
        timeout_stdout = exception.stdout if isinstance(
            exception, subprocess.TimeoutExpired
        ) else ""
        timeout_stderr = exception.stderr if isinstance(
            exception, subprocess.TimeoutExpired
        ) else ""
        stdout = timeout_stdout.decode(errors="replace") \
            if isinstance(timeout_stdout, bytes) else (timeout_stdout or "")
        stderr = timeout_stderr.decode(errors="replace") \
            if isinstance(timeout_stderr, bytes) else (timeout_stderr or "")
        fixture = None
        status = (
            "timeout" if isinstance(exception, subprocess.TimeoutExpired)
            else "failed"
        )
        error = str(exception)
        completed = None
    stdout_path.write_text(stdout)
    stderr_path.write_text(stderr)
    result = {
        "schema_version": SCHEMA_VERSION,
        "run_id": case.run_id,
        "status": status,
        "corpus": case.corpus,
        "instance_id": case.instance["id"],
        "solver_variant": case.variant,
        "repetition": case.repetition,
        "command": command,
        "returncode": completed.returncode if completed else None,
        "runtime_seconds": elapsed,
        "peak_memory_kib": peak_kib,
        "peak_memory_source": memory_source,
        "solver_metrics": fixture,
        "error": error,
        "artifacts": {
            "stdout": stdout_path.name,
            "stderr": stderr_path.name,
        },
    }
    atomic_json(result_path, result)
    return status


def common_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--name", required=True)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument("--results-root", type=Path)


def layout_from(args: argparse.Namespace) -> Layout:
    if not SAFE_NAME.fullmatch(args.name):
        raise ValueError("--name must be a safe artifact name")
    repo_root = args.repo_root.resolve()
    results_root = args.results_root.resolve() if args.results_root else repo_root / "results"
    return Layout(results_root / args.name)


def run_command(args: argparse.Namespace) -> int:
    repo_root = args.repo_root.resolve()
    layout = layout_from(args)
    private_root = args.private_corpus_root.resolve() if args.private_corpus_root else None
    discovered, private_status = discover_corpora(repo_root, private_root)
    cases = [Case(corpus, instance, variant, repetition)
             for corpus, instance in discovered
             for variant in VARIANTS
             for repetition in range(1, args.repetitions + 1)]
    if args.limit is not None:
        cases = cases[:args.limit]
    scan = boundary_scan(repo_root)
    packages = package_scan(args.package_output)
    plan = {
        "schema_version": SCHEMA_VERSION,
        "name": args.name,
        "case_count": len(cases),
        "run_ids": [case.run_id for case in cases],
        "private_corpus": private_status,
        "repository_boundary": scan,
        "package_scan": packages,
    }
    if args.dry_run:
        print(json.dumps(plan, indent=2, sort_keys=True))
        return 0
    binary = resolve_binary(repo_root, args.binary.resolve() if args.binary else None)
    layout.create()
    plan["binary"] = str(binary)
    atomic_json(layout.root / "run-plan.json", plan)
    states = [run_case(layout, binary, case, args.timeout, args.force) for case in cases]
    print(json.dumps({"result_root": str(layout.root), "states": states,
                      "private_corpus": private_status}, sort_keys=True))
    return 1 if any(state in ("failed", "timeout") for state in states) else 0


def load_runs(layout: Layout) -> list[dict[str, Any]]:
    if not layout.runs.is_dir():
        raise ValueError(f"run directory does not exist: {layout.runs}")
    return [load_json(path) for path in sorted(layout.runs.glob("*.json"))]


def analyze_command(args: argparse.Namespace) -> int:
    layout = layout_from(args)
    runs = load_runs(layout)
    plan_path = layout.root / "run-plan.json"
    plan = load_json(plan_path) if plan_path.is_file() else {}
    grouped: dict[tuple[str, str, str], list[dict[str, Any]]] = {}
    for run in runs:
        key = (run["corpus"], run["instance_id"], run["solver_variant"])
        grouped.setdefault(key, []).append(run)
    groups = []
    for (corpus, instance_id, variant), values in sorted(grouped.items()):
        successful = [value for value in values if value["status"] == "ok"]
        runtimes = [value["runtime_seconds"] for value in successful]
        memories = [value["peak_memory_kib"] for value in successful
                    if value["peak_memory_kib"] is not None]
        metric_rows = [value["solver_metrics"] for value in successful]
        def average(field: str) -> float | None:
            observed = [row[field] for row in metric_rows if field in row]
            return statistics.fmean(observed) if observed else None
        groups.append({
            "corpus": corpus,
            "instance_id": instance_id,
            "solver_variant": variant,
            "runs": len(values),
            "successful_runs": len(successful),
            "runtime_seconds_mean": statistics.fmean(runtimes) if runtimes else None,
            "runtime_seconds_median": statistics.median(runtimes) if runtimes else None,
            "peak_memory_kib_mean": statistics.fmean(memories) if memories else None,
            "allocations_mean": average("allocations"),
            "allocation_metric": "model-variable-implementations",
            "propagation_calls_mean": average("propagation_calls"),
            "clone_footprint_mean": average("clone_footprint"),
            "clone_footprint_metric": "model-variable-implementations-plus-stable-clone-actors",
            "nodes_mean": average("nodes"),
            "failures_mean": average("failures"),
        })
    summary = {
        "schema_version": SCHEMA_VERSION,
        "name": args.name,
        "run_count": len(runs),
        "successful_run_count": sum(run["status"] == "ok" for run in runs),
        "private_corpus": plan.get("private_corpus", {}),
        "repository_boundary": plan.get("repository_boundary", {}),
        "package_scan": plan.get("package_scan", {}),
        "groups": groups,
    }
    layout.analysis.mkdir(parents=True, exist_ok=True)
    atomic_json(layout.analysis / "summary.json", summary)
    print(layout.analysis / "summary.json")
    return 0


def format_metric(value: Any, digits: int = 3) -> str:
    if value is None:
        return "n/a"
    if isinstance(value, float):
        return f"{value:.{digits}f}"
    return str(value)


def report_command(args: argparse.Namespace) -> int:
    layout = layout_from(args)
    summary_path = layout.analysis / "summary.json"
    summary = load_json(summary_path)
    lines = [
        f"# Word benchmark: {args.name}",
        "",
        (f"Successful runs: {summary['successful_run_count']} / "
         f"{summary['run_count']}."),
        "",
        "| corpus | instance | solver variant | runs | runtime mean (s) | peak RSS mean (KiB) | allocations | propagation calls | clone footprint | nodes | failures |",
        "|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for group in summary["groups"]:
        lines.append("| " + " | ".join((
            group["corpus"], group["instance_id"], group["solver_variant"],
            str(group["successful_runs"]),
            format_metric(group["runtime_seconds_mean"], 6),
            format_metric(group["peak_memory_kib_mean"], 1),
            format_metric(group["allocations_mean"], 1),
            format_metric(group["propagation_calls_mean"], 1),
            format_metric(group["clone_footprint_mean"], 1),
            format_metric(group["nodes_mean"], 1),
            format_metric(group["failures_mean"], 1),
        )) + " |")
    private_status = summary.get("private_corpus", {})
    package_status = summary.get("package_scan", {})
    lines.extend([
        "",
        "## Interpretation boundaries",
        "",
        "This compares native Gecode Word constraints with an equivalent Gecode Boolean decomposition. It is not an SMT comparison.",
        "",
        "Allocation and clone-footprint values are structural model counters, not kernel allocation counts or byte measurements. Peak RSS is best effort.",
        "",
        f"Private corpus discovery: `{private_status.get('status', 'not-recorded')}` with {private_status.get('loaded_instances', 0)} loaded instances.",
        "",
        f"Supplied package-output scan: `{package_status.get('status', 'not-recorded')}` with {len(package_status.get('private_or_result_payload_candidates', []))} private/result candidates.",
        "",
    ])
    issues = private_status.get("issues", [])
    if issues:
        lines.append("Discovery issues: " + "; ".join(issues) + ".")
        lines.append("")
    layout.reports.mkdir(parents=True, exist_ok=True)
    report_path = layout.reports / "benchmark.md"
    report_path.write_text("\n".join(lines))
    print(report_path)
    return 0


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    subparsers = result.add_subparsers(dest="command", required=True)
    run = subparsers.add_parser("run", help="run or resume benchmark cases")
    common_parser(run)
    run.add_argument("--binary", type=Path)
    run.add_argument("--private-corpus-root", type=Path)
    run.add_argument("--package-output", type=Path, action="append", default=[])
    run.add_argument("--repetitions", type=int, default=1)
    run.add_argument("--timeout", type=float, default=60.0)
    run.add_argument("--limit", type=int)
    run.add_argument("--force", action="store_true")
    run.add_argument("--dry-run", action="store_true")
    run.set_defaults(handler=run_command)
    analyze = subparsers.add_parser("analyze", help="aggregate stable run artifacts")
    common_parser(analyze)
    analyze.set_defaults(handler=analyze_command)
    report = subparsers.add_parser("report", help="write a concise engineering report")
    common_parser(report)
    report.set_defaults(handler=report_command)
    return result


def main() -> int:
    args = parser().parse_args()
    if getattr(args, "repetitions", 1) <= 0:
        print("--repetitions must be positive", file=sys.stderr)
        return 2
    if getattr(args, "limit", 1) is not None and getattr(args, "limit", 1) <= 0:
        print("--limit must be positive", file=sys.stderr)
        return 2
    try:
        return args.handler(args)
    except ValueError as error:
        print(f"benchmark error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
