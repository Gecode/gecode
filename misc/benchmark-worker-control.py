#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = [
#   "matplotlib>=3.8",
# ]
# ///

"""Benchmark Gecode's externally adjustable parallel-search workers."""

from __future__ import annotations

import argparse
import dataclasses
import datetime as dt
import json
import math
import os
import pathlib
import platform
import random
import re
import shutil
import socket
import statistics
import subprocess
import sys
import time
from collections import Counter, defaultdict
from typing import Any, Iterable, Sequence


SCRIPT_PATH = pathlib.Path(__file__).resolve()
DEFAULT_REPO_ROOT = SCRIPT_PATH.parent.parent
TERMINAL_STATUSES = {"ok", "error", "timeout"}
OVERHEAD_MEDIAN_GATE_PCT = 3.0
OVERHEAD_CI_GATE_PCT = 5.0
BOOTSTRAP_SEED = 20260724
BOOTSTRAP_SAMPLES = 10_000
CASE_GROUPS = ("overhead", "latency", "parked", "portfolio")


class BenchmarkError(RuntimeError):
    pass


@dataclasses.dataclass(frozen=True)
class Layout:
    root: pathlib.Path
    runs: pathlib.Path
    analysis: pathlib.Path
    reports: pathlib.Path
    plots: pathlib.Path


@dataclasses.dataclass(frozen=True)
class Case:
    run_id: str
    command: list[str]
    dimensions: dict[str, Any]
    timeout_sec: int


@dataclasses.dataclass(frozen=True)
class TimeStrategy:
    name: str
    prefix: list[str]


def utc_now() -> str:
    return (
        dt.datetime.now(dt.timezone.utc)
        .replace(microsecond=0)
        .isoformat()
        .replace("+00:00", "Z")
    )


def write_json(path: pathlib.Path, payload: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")


def read_json(path: pathlib.Path) -> Any:
    return json.loads(path.read_text())


def maybe_json(path: pathlib.Path) -> Any | None:
    try:
        return read_json(path)
    except (FileNotFoundError, json.JSONDecodeError, OSError):
        return None


def repo_root(args: argparse.Namespace) -> pathlib.Path:
    raw = getattr(args, "repo_root", None)
    return pathlib.Path(raw).expanduser().resolve() if raw else DEFAULT_REPO_ROOT


def results_root(args: argparse.Namespace, root: pathlib.Path) -> pathlib.Path:
    raw = getattr(args, "results_root", None)
    return pathlib.Path(raw).expanduser().resolve() if raw else root / "results"


def layout(args: argparse.Namespace) -> Layout:
    name = args.name.strip()
    if not name:
        raise BenchmarkError("--name must not be empty")
    root = results_root(args, repo_root(args)) / name
    result = Layout(
        root=root,
        runs=root / "runs",
        analysis=root / "analysis",
        reports=root / "reports",
        plots=root / "plots",
    )
    for path in dataclasses.astuple(result):
        pathlib.Path(path).mkdir(parents=True, exist_ok=True)
    return result


def benchmark_binary(args: argparse.Namespace) -> pathlib.Path:
    build = pathlib.Path(args.build_dir).expanduser().resolve()
    candidates = [
        build / "bin" / "gecode-worker-control-benchmark",
        build / "gecode-worker-control-benchmark",
    ]
    for candidate in candidates:
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return candidate
    raise BenchmarkError(
        "gecode-worker-control-benchmark not found; build the CMake target "
        f"in {build}"
    )


def select_time_strategy() -> TimeStrategy:
    gtime = shutil.which("gtime")
    if gtime:
        return TimeStrategy("gnu-time-v", [gtime, "-v"])
    system_time = pathlib.Path("/usr/bin/time")
    if system_time.exists():
        probe = subprocess.run(
            [str(system_time), "-l", "/usr/bin/true"],
            capture_output=True,
            text=True,
            check=False,
        )
        if probe.returncode == 0:
            return TimeStrategy("bsd-time-l", [str(system_time), "-l"])
        return TimeStrategy("gnu-time-v", [str(system_time), "-v"])
    return TimeStrategy("python-wall", [])


def parse_time(stderr: str, strategy: TimeStrategy) -> dict[str, Any]:
    peak_kib: int | None = None
    reported_sec: float | None = None
    gnu_rss = re.search(r"Maximum resident set size \(kbytes\):\s*(\d+)", stderr)
    if gnu_rss:
        peak_kib = int(gnu_rss.group(1))
    bsd_rss = re.search(
        r"^\s*(\d+)\s+maximum resident set size", stderr, re.MULTILINE
    )
    if bsd_rss:
        peak_kib = int(bsd_rss.group(1)) // 1024
    bsd_real = re.search(r"^\s*([0-9.]+)\s+real\b", stderr, re.MULTILINE)
    if bsd_real:
        reported_sec = float(bsd_real.group(1))
    return {
        "time_strategy": strategy.name,
        "reported_tool_sec": reported_sec,
        "peak_memory_kib": peak_kib,
    }


def case_selected(args: argparse.Namespace, group: str) -> bool:
    selected = getattr(args, "cases", None)
    return not selected or group in selected


def make_case(
    binary: pathlib.Path,
    run_id: str,
    benchmark_case: str,
    variant: str,
    sample: int,
    group: str,
    timeout_sec: int,
    extra: Sequence[str] = (),
) -> Case:
    command = [
        str(binary),
        "--case",
        benchmark_case,
        "--threads",
        "4",
        *extra,
    ]
    if variant:
        command += ["--variant", variant]
    return Case(
        run_id=run_id,
        command=command,
        dimensions={
            "group": group,
            "case": benchmark_case,
            "variant": variant,
            "sample": sample,
        },
        timeout_sec=timeout_sec,
    )


def build_cases(args: argparse.Namespace, binary: pathlib.Path) -> list[Case]:
    cases: list[Case] = []
    timeout = args.timeout_sec
    if case_selected(args, "overhead"):
        for sample in range(args.overhead_samples):
            order = ("baseline", "control") if sample % 2 == 0 else (
                "control",
                "baseline",
            )
            for workload, iterations in (("overhead-dfs", 8), ("overhead-bab", 8)):
                for variant in order:
                    cases.append(
                        make_case(
                            binary,
                            f"{workload}-{sample:03d}-{variant}",
                            workload,
                            variant,
                            sample,
                            "overhead",
                            timeout,
                            ("--iterations", str(iterations)),
                        )
                    )
    if case_selected(args, "latency"):
        latency_cases = (
            "latency-cheap-shrink",
            "latency-cheap-grow",
            "latency-expensive-shrink",
            "latency-expensive-grow",
        )
        for sample in range(args.latency_samples):
            for benchmark_case in latency_cases:
                variant = benchmark_case.rsplit("-", 1)[-1]
                cases.append(
                    make_case(
                        binary,
                        f"{benchmark_case}-{sample:03d}",
                        benchmark_case,
                        variant,
                        sample,
                        "latency",
                        timeout,
                    )
                )
    if case_selected(args, "parked"):
        for sample in range(args.parked_samples):
            order = ("one-worker", "parked") if sample % 2 == 0 else (
                "parked",
                "one-worker",
            )
            for variant in order:
                cases.append(
                    make_case(
                        binary,
                        f"parked-cost-{sample:03d}-{variant}",
                        "parked-cost",
                        variant,
                        sample,
                        "parked",
                        timeout,
                        ("--duration-ms", str(args.parked_duration_ms)),
                    )
                )
    if case_selected(args, "portfolio"):
        for sample in range(args.portfolio_samples):
            cases.append(
                make_case(
                    binary,
                    f"portfolio-{sample:03d}",
                    "portfolio",
                    "reallocate",
                    sample,
                    "portfolio",
                    timeout,
                )
            )
    if args.limit is not None:
        cases = cases[: max(0, args.limit)]
    return cases


def complete_result(path: pathlib.Path) -> bool:
    payload = maybe_json(path)
    return isinstance(payload, dict) and payload.get("status") in TERMINAL_STATUSES


def parse_helper_stdout(stdout: str) -> dict[str, Any]:
    for line in reversed(stdout.splitlines()):
        line = line.strip()
        if not line:
            continue
        try:
            payload = json.loads(line)
        except json.JSONDecodeError:
            continue
        if isinstance(payload, dict):
            return payload
    raise BenchmarkError("benchmark helper produced no JSON object")


def execute(case: Case, paths: Layout, strategy: TimeStrategy) -> dict[str, Any]:
    stdout_path = paths.runs / f"{case.run_id}.stdout"
    stderr_path = paths.runs / f"{case.run_id}.stderr"
    json_path = paths.runs / f"{case.run_id}.json"
    start = time.monotonic()
    started = utc_now()
    timed_out = False
    try:
        proc = subprocess.run(
            [*strategy.prefix, *case.command],
            capture_output=True,
            text=True,
            timeout=case.timeout_sec,
            check=False,
        )
        stdout = proc.stdout or ""
        stderr = proc.stderr or ""
        return_code = proc.returncode
    except subprocess.TimeoutExpired as exc:
        timed_out = True
        stdout = (
            exc.stdout.decode(errors="replace")
            if isinstance(exc.stdout, bytes)
            else (exc.stdout or "")
        )
        stderr = (
            exc.stderr.decode(errors="replace")
            if isinstance(exc.stderr, bytes)
            else (exc.stderr or "")
        )
        stderr += f"\nTIMEOUT after {case.timeout_sec}s\n"
        return_code = -1
    elapsed = time.monotonic() - start
    stdout_path.write_text(stdout)
    stderr_path.write_text(stderr)
    metrics: dict[str, Any] | None = None
    parse_error: str | None = None
    if not timed_out:
        try:
            metrics = parse_helper_stdout(stdout)
        except BenchmarkError as error:
            parse_error = str(error)
    timing = parse_time(stderr, strategy)
    status = "timeout" if timed_out else "ok"
    if not timed_out and (
        return_code != 0
        or metrics is None
        or metrics.get("ok") is not True
    ):
        status = "error"
    result = {
        "run_id": case.run_id,
        "dimensions": case.dimensions,
        "command": case.command,
        "status": status,
        "return_code": return_code,
        "timed_out": timed_out,
        "parse_error": parse_error,
        "elapsed_wall_sec": elapsed,
        "reported_tool_sec": timing["reported_tool_sec"],
        "peak_memory_kib": timing["peak_memory_kib"],
        "time_strategy": timing["time_strategy"],
        "metrics": metrics,
        "stdout_path": str(stdout_path),
        "stderr_path": str(stderr_path),
        "started_at_utc": started,
        "completed_at_utc": utc_now(),
    }
    write_json(json_path, result)
    return result


def git_value(root: pathlib.Path, *args: str) -> str | None:
    proc = subprocess.run(
        ["git", *args], cwd=root, capture_output=True, text=True, check=False
    )
    return proc.stdout.strip() if proc.returncode == 0 else None


def provenance(root: pathlib.Path, strategy: TimeStrategy) -> dict[str, Any]:
    compiler = subprocess.run(
        ["c++", "--version"], capture_output=True, text=True, check=False
    )
    return {
        "generated_at_utc": utc_now(),
        "git_commit": git_value(root, "rev-parse", "HEAD"),
        "git_status": git_value(root, "status", "--short"),
        "platform": platform.platform(),
        "hostname": socket.gethostname(),
        "python": platform.python_version(),
        "compiler": (compiler.stdout or compiler.stderr).splitlines()[0],
        "time_strategy": strategy.name,
        "bootstrap_seed": BOOTSTRAP_SEED,
        "bootstrap_samples": BOOTSTRAP_SAMPLES,
    }


def run_command(args: argparse.Namespace) -> int:
    root = repo_root(args)
    paths = layout(args)
    binary = benchmark_binary(args)
    strategy = select_time_strategy()
    cases = build_cases(args, binary)
    plan = {
        "name": args.name,
        "generated_at_utc": utc_now(),
        "repo_root": str(root),
        "results_root": str(paths.root.parent),
        "build_dir": str(pathlib.Path(args.build_dir).resolve()),
        "binary": str(binary),
        "time_strategy": dataclasses.asdict(strategy),
        "options": {
            "overhead_samples": args.overhead_samples,
            "latency_samples": args.latency_samples,
            "parked_samples": args.parked_samples,
            "portfolio_samples": args.portfolio_samples,
            "parked_duration_ms": args.parked_duration_ms,
            "cases": args.cases,
            "limit": args.limit,
            "timeout_sec": args.timeout_sec,
        },
        "cases": [dataclasses.asdict(case) for case in cases],
    }
    write_json(paths.analysis / "plan.json", plan)
    write_json(
        paths.analysis / "repo-context.json",
        {
            "project_type": "C++ constraint solver",
            "benchmark_shape": [
                "paired DFS/BAB unchanged-limit overhead",
                "cooperative shrink/grow response latency",
                "resident parked-thread process cost",
                "PBS asset budget reallocation",
            ],
            "report_targets": ["markdown", "latex", "png", "svg"],
        },
    )
    write_json(paths.analysis / "provenance.json", provenance(root, strategy))
    print(f"run root: {paths.root}")
    print(f"cases: {len(cases)}")
    completed = 0
    skipped = 0
    for case in cases:
        target = paths.runs / f"{case.run_id}.json"
        if complete_result(target) and not args.force:
            skipped += 1
            continue
        if args.dry_run:
            print("$", " ".join(case.command))
            completed += 1
            continue
        result = execute(case, paths, strategy)
        completed += 1
        marker = "+" if result["status"] == "ok" else "!"
        print(f"{marker} {case.run_id}")
    print(f"completed={completed} skipped={skipped}")
    return 0


def percentile(values: Sequence[float], pct: float) -> float | None:
    if not values:
        return None
    ordered = sorted(values)
    if len(ordered) == 1:
        return ordered[0]
    position = (len(ordered) - 1) * pct / 100.0
    lower = math.floor(position)
    upper = math.ceil(position)
    if lower == upper:
        return ordered[lower]
    fraction = position - lower
    return ordered[lower] * (1.0 - fraction) + ordered[upper] * fraction


def stats(values: Sequence[float]) -> dict[str, Any]:
    return {
        "count": len(values),
        "mean": statistics.fmean(values) if values else None,
        "median": statistics.median(values) if values else None,
        "p50": percentile(values, 50.0),
        "p95": percentile(values, 95.0),
        "min": min(values) if values else None,
        "max": max(values) if values else None,
    }


def bootstrap_median_ci(values: Sequence[float]) -> tuple[float | None, float | None]:
    if not values:
        return None, None
    rng = random.Random(BOOTSTRAP_SEED)
    n = len(values)
    medians = [
        statistics.median(values[rng.randrange(n)] for _ in range(n))
        for _ in range(BOOTSTRAP_SAMPLES)
    ]
    return percentile(medians, 2.5), percentile(medians, 97.5)


def load_runs(paths: Layout) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for path in sorted(paths.runs.glob("*.json")):
        payload = maybe_json(path)
        if isinstance(payload, dict):
            result.append(payload)
    return result


def metric(run: dict[str, Any], name: str) -> float | None:
    metrics = run.get("metrics")
    value = metrics.get(name) if isinstance(metrics, dict) else None
    return float(value) if isinstance(value, (int, float)) else None


def paired_overhead(
    runs: Iterable[dict[str, Any]], workload: str
) -> dict[str, Any]:
    pairs: dict[int, dict[str, float]] = defaultdict(dict)
    for run in runs:
        dimensions = run.get("dimensions", {})
        if (
            run.get("status") != "ok"
            or dimensions.get("case") != workload
        ):
            continue
        elapsed = metric(run, "elapsed_ns")
        if elapsed is not None:
            pairs[int(dimensions["sample"])][str(dimensions["variant"])] = elapsed
    overhead: list[float] = []
    for variants in pairs.values():
        baseline = variants.get("baseline")
        control = variants.get("control")
        if baseline and control:
            overhead.append((control / baseline - 1.0) * 100.0)
    low, high = bootstrap_median_ci(overhead)
    median = statistics.median(overhead) if overhead else None
    gate = bool(
        median is not None
        and high is not None
        and median <= OVERHEAD_MEDIAN_GATE_PCT
        and high <= OVERHEAD_CI_GATE_PCT
    )
    return {
        "workload": workload.removeprefix("overhead-"),
        "pairs": len(overhead),
        "overhead_pct": stats(overhead),
        "bootstrap_95_ci_pct": [low, high],
        "median_gate_pct": OVERHEAD_MEDIAN_GATE_PCT,
        "ci_upper_gate_pct": OVERHEAD_CI_GATE_PCT,
        "gate_pass": gate,
    }


def analyze_command(args: argparse.Namespace) -> int:
    paths = layout(args)
    runs = load_runs(paths)
    statuses = Counter(str(run.get("status", "unknown")) for run in runs)
    failures = [
        {
            "run_id": run.get("run_id"),
            "status": run.get("status"),
            "stderr_path": run.get("stderr_path"),
        }
        for run in runs
        if run.get("status") != "ok"
    ]
    overhead = {
        workload: paired_overhead(runs, f"overhead-{workload}")
        for workload in ("dfs", "bab")
    }
    latency: dict[str, Any] = {}
    for benchmark_case in (
        "latency-cheap-shrink",
        "latency-cheap-grow",
        "latency-expensive-shrink",
        "latency-expensive-grow",
    ):
        values = [
            value / 1000.0
            for run in runs
            if run.get("status") == "ok"
            and run.get("dimensions", {}).get("case") == benchmark_case
            and (value := metric(run, "latency_ns")) is not None
        ]
        latency[benchmark_case.removeprefix("latency-")] = stats(values)

    parked_pairs: dict[int, dict[str, dict[str, float]]] = defaultdict(dict)
    for run in runs:
        dimensions = run.get("dimensions", {})
        if (
            run.get("status") != "ok"
            or dimensions.get("case") != "parked-cost"
        ):
            continue
        sample = int(dimensions["sample"])
        variant = str(dimensions["variant"])
        parked_pairs[sample][variant] = {
            "peak_memory_kib": float(run["peak_memory_kib"])
            if isinstance(run.get("peak_memory_kib"), (int, float))
            else math.nan,
            "cpu_ratio": (
                metric(run, "cpu_ns") or 0.0
            )
            / max(metric(run, "elapsed_ns") or 1.0, 1.0),
        }
    memory_delta: list[float] = []
    one_cpu: list[float] = []
    parked_cpu: list[float] = []
    for pair in parked_pairs.values():
        one = pair.get("one-worker")
        parked = pair.get("parked")
        if not one or not parked:
            continue
        if not math.isnan(one["peak_memory_kib"]) and not math.isnan(
            parked["peak_memory_kib"]
        ):
            memory_delta.append(
                parked["peak_memory_kib"] - one["peak_memory_kib"]
            )
        one_cpu.append(one["cpu_ratio"])
        parked_cpu.append(parked["cpu_ratio"])
    parked_summary = {
        "pairs": sum(
            1
            for pair in parked_pairs.values()
            if "one-worker" in pair and "parked" in pair
        ),
        "peak_memory_delta_kib": stats(memory_delta),
        "one_worker_cpu_to_wall": stats(one_cpu),
        "parked_cpu_to_wall": stats(parked_cpu),
        "interpretation": (
            "Peak process RSS is a best-effort comparison, not an exact "
            "per-thread stack-reservation measurement."
        ),
    }
    portfolio_runs = [
        run
        for run in runs
        if run.get("status") == "ok"
        and run.get("dimensions", {}).get("case") == "portfolio"
    ]
    portfolio_totals = [
        value
        for run in portfolio_runs
        if (value := metric(run, "max_total_admitted")) is not None
    ]
    portfolio = {
        "samples": len(portfolio_runs),
        "budget": 4,
        "max_total_admitted": stats(portfolio_totals),
        "bounded": bool(portfolio_totals)
        and all(value <= 4.0 for value in portfolio_totals),
        "allocations": [[3, 1], [1, 3], [2, 2]],
    }
    gate_pass = (
        not failures
        and all(item["gate_pass"] for item in overhead.values())
        and portfolio["bounded"]
        and all(item["count"] > 0 for item in latency.values())
    )
    summary = {
        "name": args.name,
        "generated_at_utc": utc_now(),
        "total_runs": len(runs),
        "status_counts": dict(statuses),
        "failures": failures[:20],
        "overhead": overhead,
        "latency_us": latency,
        "parked_cost": parked_summary,
        "portfolio": portfolio,
        "provisional_release_gate_pass": gate_pass,
        "human_release_decision": "pending",
        "gate": {
            "median_overhead_pct": OVERHEAD_MEDIAN_GATE_PCT,
            "bootstrap_95_ci_upper_pct": OVERHEAD_CI_GATE_PCT,
        },
        "provenance": maybe_json(paths.analysis / "provenance.json"),
        "plan": maybe_json(paths.analysis / "plan.json"),
    }
    write_json(paths.analysis / "summary.json", summary)
    print(paths.analysis / "summary.json")
    return 0


def fmt(value: Any, digits: int = 3) -> str:
    return "n/a" if value is None else f"{float(value):.{digits}f}"


def markdown_report(summary: dict[str, Any]) -> str:
    gate = "PASS" if summary["provisional_release_gate_pass"] else "FAIL"
    lines = [
        f"# Worker-control benchmark: {summary['name']}",
        "",
        f"- Generated: {summary['generated_at_utc']}",
        f"- Samples: {summary['total_runs']}",
        f"- Provisional release gate: **{gate}**",
        f"- Human release decision: **{summary['human_release_decision']}**",
        "",
        "## Unchanged-limit overhead",
        "",
        "| Workload | Pairs | Median | Bootstrap 95% CI | Gate |",
        "| --- | ---: | ---: | ---: | --- |",
    ]
    for workload in ("dfs", "bab"):
        item = summary["overhead"][workload]
        ci = item["bootstrap_95_ci_pct"]
        lines.append(
            f"| {workload.upper()} | {item['pairs']} | "
            f"{fmt(item['overhead_pct']['median'])}% | "
            f"[{fmt(ci[0])}%, {fmt(ci[1])}%] | "
            f"{'pass' if item['gate_pass'] else 'fail'} |"
        )
    lines += [
        "",
        "The gate requires median paired overhead no greater than 3% and "
        "a bootstrap 95% upper bound no greater than 5% for both workloads.",
        "",
        "## Cooperative resize latency",
        "",
        "| Workload and direction | Samples | P50 (µs) | P95 (µs) |",
        "| --- | ---: | ---: | ---: |",
    ]
    for key, item in summary["latency_us"].items():
        lines.append(
            f"| {key} | {item['count']} | {fmt(item['p50'])} | "
            f"{fmt(item['p95'])} |"
        )
    parked = summary["parked_cost"]
    portfolio = summary["portfolio"]
    lines += [
        "",
        "## Parked-thread process cost",
        "",
        f"- Paired samples: {parked['pairs']}",
        "- Median peak-RSS delta, four resident workers with one active "
        f"versus one physical worker: "
        f"{fmt(parked['peak_memory_delta_kib']['median'])} KiB.",
        "- Median CPU/wall ratio, one physical worker: "
        f"{fmt(parked['one_worker_cpu_to_wall']['median'])}.",
        "- Median CPU/wall ratio, four resident workers with three parked: "
        f"{fmt(parked['parked_cpu_to_wall']['median'])}.",
        f"- Caveat: {parked['interpretation']}",
        "",
        "## Portfolio budget reallocation",
        "",
        f"- Samples: {portfolio['samples']}",
        f"- Fixed active-worker budget: {portfolio['budget']}",
        f"- Allocation phases: {portfolio['allocations']}",
        "- Maximum summed admitted high-water mark: "
        f"{fmt(portfolio['max_total_admitted']['max'], 0)}",
        f"- Bound respected: **{'yes' if portfolio['bounded'] else 'no'}**",
        "",
        "This experiment demonstrates the control seam and the concurrency "
        "bound. It does not claim that this allocation sequence is a "
        "production policy or that it improves every portfolio.",
        "",
        "## Method and provenance",
        "",
    ]
    provenance = summary.get("provenance") or {}
    for key in (
        "git_commit",
        "platform",
        "hostname",
        "compiler",
        "python",
        "time_strategy",
        "bootstrap_seed",
        "bootstrap_samples",
    ):
        lines.append(f"- {key}: {provenance.get(key)}")
    lines += [
        "",
        "Each overhead sample is paired by sample number; execution order "
        "alternates baseline/control to reduce order bias. Timing comes from "
        "the helper's in-process steady clock, excluding process startup. "
        "Raw JSON, stdout, and stderr remain under the named result root.",
    ]
    return "\n".join(lines) + "\n"


def latex_report(summary: dict[str, Any]) -> str:
    rows = []
    for workload in ("dfs", "bab"):
        item = summary["overhead"][workload]
        low, high = item["bootstrap_95_ci_pct"]
        rows.append(
            f"{workload.upper()} & {item['pairs']} & "
            f"{fmt(item['overhead_pct']['median'])}\\% & "
            f"[{fmt(low)}\\%, {fmt(high)}\\%] \\\\"
        )
    return "\n".join(
        [
            "% Generated by misc/benchmark-worker-control.py",
            "\\begin{tabular}{lrrr}",
            "\\hline",
            "Workload & Pairs & Median overhead & Bootstrap 95\\% CI \\\\",
            "\\hline",
            *rows,
            "\\hline",
            "\\end{tabular}",
            "",
        ]
    )


def ensure_summary(args: argparse.Namespace, paths: Layout) -> dict[str, Any]:
    summary = maybe_json(paths.analysis / "summary.json")
    if not isinstance(summary, dict):
        analyze_command(args)
        summary = read_json(paths.analysis / "summary.json")
    return summary


def report_command(args: argparse.Namespace) -> int:
    paths = layout(args)
    summary = ensure_summary(args, paths)
    markdown = paths.reports / "report.md"
    latex = paths.reports / "tables.tex"
    markdown.write_text(markdown_report(summary))
    latex.write_text(latex_report(summary))
    print(markdown)
    print(latex)
    return 0


def plot_command(args: argparse.Namespace) -> int:
    import matplotlib.pyplot as plt

    paths = layout(args)
    summary = ensure_summary(args, paths)
    # Catppuccin Latte colors on a white, documentation-friendly background.
    blue, mauve, green, red, text = (
        "#1e66f5",
        "#8839ef",
        "#40a02b",
        "#d20f39",
        "#4c4f69",
    )
    plt.rcParams.update(
        {
            "figure.facecolor": "white",
            "axes.facecolor": "white",
            "axes.edgecolor": text,
            "axes.labelcolor": text,
            "text.color": text,
            "xtick.color": text,
            "ytick.color": text,
            "font.size": 10,
        }
    )
    fig, axes = plt.subplots(1, 3, figsize=(13.5, 4.2))
    workloads = ["dfs", "bab"]
    medians = [
        summary["overhead"][name]["overhead_pct"]["median"]
        for name in workloads
    ]
    lows = [
        summary["overhead"][name]["bootstrap_95_ci_pct"][0]
        for name in workloads
    ]
    highs = [
        summary["overhead"][name]["bootstrap_95_ci_pct"][1]
        for name in workloads
    ]
    axes[0].bar(
        [name.upper() for name in workloads],
        medians,
        color=[blue, mauve],
        yerr=[
            [median - low for median, low in zip(medians, lows)],
            [high - median for median, high in zip(medians, highs)],
        ],
        capsize=5,
    )
    axes[0].axhline(OVERHEAD_MEDIAN_GATE_PCT, color=red, linestyle="--")
    axes[0].set_title("Fixed-control overhead")
    axes[0].set_ylabel("Paired overhead (%)")

    labels = list(summary["latency_us"])
    p50 = [summary["latency_us"][key]["p50"] for key in labels]
    p95 = [summary["latency_us"][key]["p95"] for key in labels]
    positions = range(len(labels))
    axes[1].bar(
        [position - 0.18 for position in positions],
        p50,
        width=0.36,
        label="p50",
        color=green,
    )
    axes[1].bar(
        [position + 0.18 for position in positions],
        p95,
        width=0.36,
        label="p95",
        color=blue,
    )
    axes[1].set_xticks(list(positions))
    axes[1].set_xticklabels(
        [label.replace("expensive", "prop").replace("-", "\n") for label in labels]
    )
    axes[1].set_title("Cooperative resize latency")
    axes[1].set_ylabel("Microseconds")
    axes[1].legend(frameon=False)

    parked = summary["parked_cost"]
    cpu = [
        parked["one_worker_cpu_to_wall"]["median"],
        parked["parked_cpu_to_wall"]["median"],
    ]
    axes[2].bar(["1 resident", "4 resident\n1 active"], cpu, color=[blue, mauve])
    axes[2].set_title("Search CPU / wall")
    axes[2].set_ylabel("Ratio")
    axes[2].text(
        0.5,
        0.95,
        "Portfolio bound: "
        + ("respected" if summary["portfolio"]["bounded"] else "violated"),
        transform=axes[2].transAxes,
        ha="center",
        va="top",
    )
    fig.suptitle("Gecode adjustable-worker benchmark")
    fig.tight_layout()
    png = paths.plots / "worker-control-benchmark.png"
    svg = paths.plots / "worker-control-benchmark.svg"
    fig.savefig(png, dpi=180)
    fig.savefig(svg)
    plt.close(fig)
    print(png)
    print(svg)
    return 0


def add_paths(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--repo-root")
    parser.add_argument("--results-root")


def parser() -> argparse.ArgumentParser:
    root = argparse.ArgumentParser(
        description="Benchmark externally adjustable Gecode search workers"
    )
    commands = root.add_subparsers(dest="command", required=True)
    run = commands.add_parser("run")
    run.add_argument("--name", required=True)
    add_paths(run)
    run.add_argument("--build-dir", required=True)
    run.add_argument("--force", action="store_true")
    run.add_argument("--dry-run", action="store_true")
    run.add_argument("--limit", type=int)
    run.add_argument("--cases", nargs="+", choices=CASE_GROUPS)
    run.add_argument("--overhead-samples", type=int, default=30)
    run.add_argument("--latency-samples", type=int, default=100)
    run.add_argument("--parked-samples", type=int, default=20)
    run.add_argument("--portfolio-samples", type=int, default=30)
    run.add_argument("--parked-duration-ms", type=int, default=200)
    run.add_argument("--timeout-sec", type=int, default=30)
    run.set_defaults(func=run_command)
    for name, function in (
        ("analyze", analyze_command),
        ("report", report_command),
        ("plot", plot_command),
    ):
        command = commands.add_parser(name)
        command.add_argument("--name", required=True)
        add_paths(command)
        command.set_defaults(func=function)
    return root


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        return args.func(args)
    except BenchmarkError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
