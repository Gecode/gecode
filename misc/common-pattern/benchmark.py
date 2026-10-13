#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Compare open/padded DFA models with CP2015 prefix projection at 100% support.

One writer per named run. See README.md for the build and comparison limits.
"""

import argparse
from collections import Counter
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import random
import re
import signal
import statistics
import subprocess
import sys
import time


VARIANTS = ("open-dfa", "padded-dfa", "paper-projection")
LANGUAGES = {
    "none": r"[A-Z]+",
    "motif": r"A[A-Z]*C",
    "re10": r"A*B(B|C)D*EF*(G|H)I*",
    "re14": r"A*(Q|BS*(B|C))D*E(I|S)*(F|H)G*R",
    "re2": r"(S|T)[A-Z](R|K)",
}
METRICS = ("solutions", "nodes", "failures", "propagations",
           "model_search_seconds")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save_json(path: Path, value: object) -> None:
    temporary = path.with_suffix(".tmp")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    temporary.replace(path)


def planted(rng: random.Random, pattern: str, count: int,
            noise: int) -> list[str]:
    """Keep the pattern's order while inserting independently sampled items."""
    sequences = []
    for _ in range(count):
        text = list(pattern)
        for _ in range(noise):
            text.insert(rng.randrange(len(text) + 1), rng.choice("DEFGHIJKLMNOP"))
        sequences.append("".join(text))
    return sequences


def cases(seed: int, suite: str, database: Path | None,
          restriction: str) -> list[dict]:
    if database:
        sequences = database.read_text().splitlines()
        if not sequences or any(not re.fullmatch("[A-Z]+", s) for s in sequences):
            raise ValueError("database must contain one nonempty A..Z sequence per line")
        return [{"id": "input", "sequences": sequences, "language": restriction}]
    rng = random.Random(seed)
    workloads = [
        ("paper-table1", ["ABCBC", "BABC", "AB", "BCD"], ["none"]),
        ("example", ["ABCBC", "BABC", "ABDC"], ["none", "motif"]),
        ("repeated", ["AAAAAA", "AAAAAAAA", "AAAAAAAAAA"], ["none", "motif"]),
        ("no-common", ["ABC", "DEF"], ["none"]),
    ]
    if suite == "scaling":
        for count in (8, 32, 128):
            workloads.append((f"database-{count}",
                              planted(rng, "ABCABC", count, 10), ["none", "motif"]))
        for length in (8, 32, 128):
            workloads.append((f"suffix-{length}",
                              ["ABCBC"] + planted(rng, "ABCBC", 7, length),
                              ["none", "motif"]))
        for length in (4, 8, 12):
            pattern = ("ABC" * 4)[:length]
            workloads.append((f"pattern-{length}",
                              [pattern] + planted(rng, pattern, 7, 12), ["none"]))
        workloads.extend([
            ("paper-re10", ["AABCDDEFFGII"] +
             planted(rng, "AABCDDEFFGII", 7, 8), ["re10"]),
            ("paper-re14", ["AABCDDEISHGGR"] +
             planted(rng, "AABCDDEISHGGR", 7, 8), ["re14"]),
            ("paper-re2", ["SAR"] + planted(rng, "SAR", 7, 16), ["re2"]),
        ])
    return [{"id": f"{name}-{language}", "sequences": sequences,
             "language": language}
            for name, sequences, languages in workloads for language in languages]


def common_patterns(sequences: list[str], restriction: str) -> set[str] | None:
    """Independent exhaustive oracle when the shortest input is small enough."""
    shortest = min(sequences, key=len)
    if len(shortest) > 16:
        return None
    candidates = {""}
    for item in shortest:
        candidates |= {prefix + item for prefix in candidates}
    expression = re.compile(LANGUAGES[restriction])
    return {pattern for pattern in candidates if expression.fullmatch(pattern)
            and all(is_subsequence(pattern, sequence) for sequence in sequences)}


def is_subsequence(pattern: str, sequence: str) -> bool:
    items = iter(sequence)
    return all(item in items for item in pattern)


def execute(command: list[str], stdout: Path, stderr: Path,
            timeout: float) -> dict:
    """Capture one invocation, including startup, and reap its process group."""
    start = time.perf_counter()
    with stdout.open("w") as out, stderr.open("w") as err:
        try:
            process = subprocess.Popen(command, stdout=out, stderr=err,
                                       start_new_session=True)
        except OSError as error:
            return {"status": "error", "detail": str(error),
                    "wall_seconds": time.perf_counter() - start}
        try:
            returncode = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            kill_process_group(process)
            return {"status": "timeout", "wall_seconds": time.perf_counter() - start}
        except BaseException:
            kill_process_group(process)
            raise
    elapsed = time.perf_counter() - start
    if returncode:
        return {"status": "error", "returncode": returncode, "wall_seconds": elapsed}
    try:
        metrics = json.loads(stderr.read_text())
        if not isinstance(metrics, dict) or set(metrics) != set(METRICS):
            raise ValueError("unexpected target metrics")
        for key in METRICS:
            value = metrics[key]
            if (type(value) not in (int, float) or value < 0 or
                    not math.isfinite(value) or
                    (key != "model_search_seconds" and type(value) is not int)):
                raise ValueError(f"invalid {key}")
    except (ValueError, TypeError) as error:
        return {"status": "invalid", "detail": str(error), "wall_seconds": elapsed}
    return {"status": "ok", "wall_seconds": elapsed, "metrics": metrics}


def kill_process_group(process: subprocess.Popen) -> None:
    try:
        os.killpg(process.pid, signal.SIGKILL)
    except ProcessLookupError:
        pass
    process.wait()


def check_patterns(result: dict, output: Path, case: dict,
                   expected: set[str] | None) -> dict:
    if result["status"] != "ok":
        return result
    patterns = output.read_text().splitlines()
    actual = set(patterns)
    expression = re.compile(LANGUAGES[case["language"]])
    if (len(patterns) != result["metrics"]["solutions"] or
            len(actual) != len(patterns) or
            any(not expression.fullmatch(p) or
                not all(is_subsequence(p, s) for s in case["sequences"])
                for p in patterns) or (expected is not None and actual != expected)):
        return {**result, "status": "invalid", "detail": "incorrect pattern enumeration"}
    fingerprint = hashlib.sha256("\n".join(sorted(actual)).encode()).hexdigest()
    return {**result, "patterns_sha256": fingerprint,
            "oracle": "exhaustive" if expected is not None else "feasibility"}


def valid_record(path: Path, identity: dict, validation: bool = False) -> dict | None:
    if not path.exists():
        return None
    try:
        record = json.loads(path.read_text())
        if not isinstance(record, dict):
            raise ValueError("record must be an object")
        if any(record.get(key) != value for key, value in identity.items()):
            raise ValueError("record identity mismatch")
        if record.get("status") not in ("ok", "timeout", "error", "invalid"):
            raise ValueError("missing terminal status")
        elapsed = record["wall_seconds"]
        if type(elapsed) not in (int, float) or not math.isfinite(elapsed) or elapsed < 0:
            raise ValueError("invalid wall time")
        if record["status"] == "ok":
            metrics = record["metrics"]
            if not isinstance(metrics, dict) or set(metrics) != set(METRICS):
                raise ValueError("missing metrics")
            for key, value in metrics.items():
                if (type(value) not in (int, float) or not math.isfinite(value) or
                        value < 0 or (key != "model_search_seconds" and type(value) is not int)):
                    raise ValueError("invalid saved metrics")
            if validation and (not re.fullmatch("[0-9a-f]{64}",
                                               record.get("patterns_sha256", "")) or
                               record.get("oracle") not in ("exhaustive", "feasibility")):
                raise ValueError("missing correctness evidence")
        return record
    except (ValueError, TypeError, KeyError) as error:
        print(f"Unfinished record {path.name}: {error}", file=sys.stderr)
        return None


def command(binary: str, case: dict, variant: str, database: Path,
            output: str, bound: str) -> list[str]:
    return [binary, variant, case["language"], str(database), output, bound]


def collect(args: argparse.Namespace, directory: Path) -> None:
    binary = args.binary.resolve(strict=True)
    workloads = cases(args.seed, args.suite, args.database, args.language)
    # Mutable shared libraries count as part of the target, not just the executable.
    libraries = sorted({p.resolve() for p in binary.parent.parent.glob("libgecode*")
                        if p.suffix in (".so", ".dylib", ".a")})
    if not libraries:
        raise ValueError("expected binary in BUILD/bin and Gecode libraries in BUILD")
    config = {"format": 1, "binary": str(binary), "binary_sha256": digest(binary),
              "libraries": {str(p): digest(p) for p in libraries},
              "build_label": args.build_label,
              "runner_sha256": digest(Path(__file__)),
              "source_sha256": digest(Path(__file__).with_name("compare.cpp")),
              "machine": {"system": platform.platform(), "node": platform.node(),
                          "processor": platform.processor()},
              "cases": workloads, "repetitions": args.repetitions,
              "seed": args.seed, "timeout_seconds": args.timeout,
              "bound": args.bound, "variants": list(VARIANTS)}
    print(f"{len(workloads)} cases; {len(workloads)*3} correctness runs; "
          f"{len(workloads)*3*args.repetitions} measured runs; "
          f"{args.timeout}s timeout per invocation; bound={args.bound}")
    print("Serial, shuffled repetitions; model construction + exhaustive DFS; "
          "measured runs count solutions without printing them.")
    if args.dry_run:
        print(" ".join(command(str(binary), workloads[0], VARIANTS[0],
                               directory / "inputs" / (workloads[0]["id"] + ".txt"),
                               "count", args.bound)))
        return
    if directory.exists():
        if json.loads((directory / "config.json").read_text()) != config:
            raise ValueError("configuration changed; use a new run name")
    else:
        directory.mkdir(parents=True)
        save_json(directory / "config.json", config)
    for part in ("inputs", "validation", "samples"):
        (directory / part).mkdir(exist_ok=True)
    passed = set()
    for case in workloads:
        database = directory / "inputs" / (case["id"] + ".txt")
        database.write_text("\n".join(case["sequences"]) + "\n")
        oracle = common_patterns(case["sequences"], case["language"])
        checks = []
        for variant in VARIANTS:
            identity = {"case": case["id"], "variant": variant}
            path = directory / "validation" / f"{case['id']}-{variant}.json"
            record = valid_record(path, identity, validation=True)
            if record is None:
                cmd = command(str(binary), case, variant, database, "print", args.bound)
                record = execute(cmd, path.with_suffix(".stdout"),
                                 path.with_suffix(".stderr"), args.timeout)
                record = check_patterns(record, path.with_suffix(".stdout"), case, oracle)
                record.update(identity)
                save_json(path, record)
            checks.append(record)
        if (all(r["status"] == "ok" for r in checks) and
                len({r["patterns_sha256"] for r in checks}) == 1):
            passed.add(case["id"])
        else:
            print(f"{case['id']}: correctness failed or incomplete; timings skipped")
    schedule = [(case, variant, repeat) for repeat in range(args.repetitions)
                for case in workloads for variant in VARIANTS]
    random.Random(args.seed).shuffle(schedule)
    executed = 0
    failures = 0
    for case, variant, repeat in schedule:
        if case["id"] not in passed:
            continue
        identity = {"case": case["id"], "variant": variant, "repeat": repeat}
        path = directory / "samples" / f"{case['id']}-{variant}-{repeat}.json"
        saved = valid_record(path, identity)
        if saved is not None:
            failures += saved["status"] != "ok"
            continue
        if args.limit is not None and executed >= args.limit:
            break
        database = directory / "inputs" / (case["id"] + ".txt")
        cmd = command(str(binary), case, variant, database, "count", args.bound)
        record = execute(cmd, path.with_suffix(".stdout"),
                         path.with_suffix(".stderr"), args.timeout)
        checked = json.loads((directory / "validation" /
                              f"{case['id']}-{variant}.json").read_text())
        if record["status"] == "ok" and (
                record["metrics"]["solutions"] != checked["metrics"]["solutions"] or
                path.with_suffix(".stdout").read_text()):
            record.update(status="invalid", detail="count run differs from validation")
        record.update(identity)
        save_json(path, record)
        executed += 1
        failures += record["status"] != "ok"
        print(f"{case['id']} / {variant} / {repeat}: {record['status']}")
    print(f"Collected {executed} new measured samples in {directory}")
    if len(passed) != len(workloads) or failures:
        raise ValueError("comparison has failed or timed-out work; inspect saved records")


def analyze(directory: Path) -> None:
    config = json.loads((directory / "config.json").read_text())
    print(f"Build: {config['build_label']}; bound: {config['bound']}; "
          "support: 100%; all-pattern enumeration")
    print("Model+search seconds: median [min, max]; only validated successful samples.")
    print("End-to-end wall seconds include process startup and teardown.")
    print("case | variant | ok/expected | other | patterns | model+search s | wall s | nodes | propagations")
    totals = Counter()
    validation_counts = Counter()
    mismatched_cases = 0
    for case in config["cases"]:
        checks = [valid_record(directory / "validation" / f"{case['id']}-{v}.json",
                               {"case": case["id"], "variant": v}, validation=True)
                  for v in config["variants"]]
        comparable = (all(r is not None and r["status"] == "ok" for r in checks) and
                      len({r["patterns_sha256"] for r in checks}) == 1)
        validation_counts.update(r["status"] if r else "unfinished" for r in checks)
        if all(r is not None and r["status"] == "ok" for r in checks) and not comparable:
            mismatched_cases += 1
        for variant in config["variants"]:
            records = [valid_record(
                directory / "samples" / f"{case['id']}-{variant}-{repeat}.json",
                {"case": case["id"], "variant": variant, "repeat": repeat})
                for repeat in range(config["repetitions"])]
            counts = Counter(r["status"] if r else "unfinished" for r in records)
            totals.update(counts)
            successful = [r for r in records if r and r["status"] == "ok"] if comparable else []
            other = ",".join(f"{key}:{value}" for key, value in counts.items() if key != "ok") or "-"
            if not successful:
                print(f"{case['id']} | {variant} | 0/{config['repetitions']} | {other} "
                      "| unavailable (correctness or collection incomplete)")
                continue
            seconds = [r["metrics"]["model_search_seconds"] for r in successful]
            wall = statistics.median(r["wall_seconds"] for r in successful)
            metrics = successful[0]["metrics"]
            print(f"{case['id']} | {variant} | {len(successful)}/{config['repetitions']} | "
                  f"{other} | {metrics['solutions']} | {statistics.median(seconds):.6f} "
                  f"[{min(seconds):.6f}, {max(seconds):.6f}] | {wall:.6f} | "
                  f"{metrics['nodes']} | {metrics['propagations']}")
    print(f"Measured sample status: {dict(totals)}")
    print(f"Correctness status: {dict(validation_counts)}; "
          f"cases with disagreeing pattern sets: {mismatched_cases}")
    print("Paper-projection is an Algorithm 2 reimplementation at 100% support, "
          "not the authors' executable or a replication of their dataset timings.")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results-root", type=Path, default=Path("build/common-pattern-results"))
    subparsers = parser.add_subparsers(dest="action", required=True)
    run = subparsers.add_parser("run", help="collect or resume a named comparison")
    run.add_argument("--name", required=True)
    run.add_argument("--binary", type=Path, required=True)
    run.add_argument("--build-label", required=True, help="compiler, flags and build configuration")
    run.add_argument("--suite", choices=("smoke", "scaling"), default="scaling")
    run.add_argument("--database", type=Path, help="custom A..Z database, one sequence per line")
    run.add_argument("--language", choices=tuple(LANGUAGES), default="none",
                     help="restriction for a custom database")
    run.add_argument("--bound", choices=("max", "shortest"), default="max")
    run.add_argument("--repetitions", type=int, default=5)
    run.add_argument("--timeout", type=float, default=30)
    run.add_argument("--seed", type=int, default=20261013)
    run.add_argument("--limit", type=int, help="collect at most this many new timed samples")
    run.add_argument("--dry-run", action="store_true")
    report = subparsers.add_parser("analyze", help="summarize saved measurements without running targets")
    report.add_argument("--name", required=True)
    args = parser.parse_args()
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_.-]*", args.name):
        parser.error("name must be one safe path component")
    directory = args.results_root.resolve() / args.name
    try:
        if args.action == "run":
            if (args.repetitions < 1 or not math.isfinite(args.timeout) or args.timeout <= 0 or
                    (args.limit is not None and args.limit < 0)):
                parser.error("repetitions and timeout must be positive; limit must be nonnegative")
            collect(args, directory)
        else:
            analyze(directory)
    except (ValueError, OSError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
