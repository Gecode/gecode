#!/usr/bin/env -S python3
"""Compare compact and bounded Word register-file formulations."""

import argparse
import json
import statistics
import subprocess
import time
from pathlib import Path


COUNTERS = ("solutions", "checksum", "index_pruned", "nodes", "failures",
            "propagations", "root_propagators", "root_branchers")

def captured(value: bytes | str | None) -> str:
    return value.decode(errors="replace") if isinstance(value,bytes) else value or ""


def run(binary: Path, formulation: str, timeout: float) -> dict:
    command = [str(binary), "-formulation", formulation, "-solutions", "0"]
    start = time.perf_counter()
    try:
      completed = subprocess.run(command, check=False, text=True,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, timeout=timeout)
    except subprocess.TimeoutExpired as error:
      return {"formulation": formulation, "status": "timeout",
              "elapsed_seconds": timeout, "command": command,
              "returncode": None, "stdout": captured(error.stdout),
              "stderr": captured(error.stderr), "error": str(error)}
    except OSError as error:
      return {"formulation": formulation, "status": "error",
              "elapsed_seconds": time.perf_counter()-start, "command": command,
              "returncode": None, "stdout": "", "stderr": "",
              "error": str(error)}
    elapsed = time.perf_counter()-start
    lines = [line for line in completed.stdout.splitlines() if line.strip()]
    try:
      if completed.returncode != 0:
        raise ValueError(f"exit status {completed.returncode}")
      if len(lines) != 1:
        raise ValueError(f"unexpected solver output: {completed.stdout!r}")
      result = json.loads(lines[0])
      if not isinstance(result,dict):
        raise ValueError("solver output must be a JSON object")
      if result.get("status") != "ok":
        raise ValueError(f"solver did not report success: {result!r}")
    except (json.JSONDecodeError, ValueError) as error:
      return {"formulation": formulation, "status": "error",
              "elapsed_seconds": elapsed, "command": command,
              "returncode": completed.returncode, "stdout": completed.stdout,
              "stderr": completed.stderr, "error": str(error)}
    result.update(elapsed_seconds=elapsed,command=command,
                  returncode=completed.returncode,stdout=completed.stdout,
                  stderr=completed.stderr,error=None)
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--parent-binary", type=Path, required=True)
    parser.add_argument("--candidate-binary", type=Path, required=True)
    parser.add_argument("--repetitions", type=int, default=20)
    parser.add_argument("--timeout", type=float, default=60.0)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.repetitions <= 0:
      parser.error("--repetitions must be positive")
    for binary in (args.parent_binary,args.candidate_binary):
      if not binary.is_file():
        parser.error(f"binary does not exist: {binary}")

    cases = {
      "parent-compact": (args.parent_binary,"compact-word"),
      "candidate-compact": (args.candidate_binary,"compact-word"),
      "candidate-bounded": (args.candidate_binary,"bounded-word"),
    }
    runs = []
    base_order = tuple(cases)
    for repetition in range(args.repetitions):
      offset=repetition % len(base_order)
      order=base_order[offset:]+base_order[:offset]
      if repetition % 2:
        order=tuple(reversed(order))
      for name in order:
        result=run(*cases[name],args.timeout)
        result.update(case=name,repetition=repetition+1)
        runs.append(result)

    summary = {}
    issues = []
    for name in cases:
      selected=[result for result in runs if result["case"] == name]
      statuses={result["status"] for result in selected}
      successful=[result for result in selected if result["status"] == "ok"]
      if statuses != {"ok"}:
        issues.append(f"{name}: statuses={sorted(statuses)}")
      if successful:
        first=successful[0]
        if any(any((field in result) != (field in first) or
                   result.get(field) != first.get(field) for field in COUNTERS)
               for result in successful[1:]):
          issues.append(f"{name}: unstable status/counters")
      seconds=[result["elapsed_seconds"] for result in successful]
      summary[name] = {
        "statuses": {status:sum(result["status"] == status for result in selected)
                     for status in ("ok","timeout","error")},
        "median_seconds": statistics.median(seconds) if seconds else None,
        "min_seconds": min(seconds) if seconds else None,
        "max_seconds": max(seconds) if seconds else None,
        "counters": {field:successful[0].get(field) for field in COUNTERS}
                    if successful else None,
      }

    parent=summary["parent-compact"]["counters"]
    compact=summary["candidate-compact"]["counters"]
    bounded=summary["candidate-bounded"]["counters"]
    if parent is not None and compact is not None and parent != compact:
      issues.append("candidate compact counters differ from exact parent")
    if bounded is not None and compact is not None and \
       (bounded["solutions"],bounded["checksum"]) != \
       (compact["solutions"],compact["checksum"]):
      issues.append("compact and bounded semantics differ")

    artifact = {
      "schema_version": 1,
      "repetitions": args.repetitions,
      "order": "rotated and reversed across repetitions",
      "binaries": {name: str(binary.resolve())
                   for name, (binary, _) in cases.items()},
      "summary": summary,
      "runs": runs,
      "validation": {"issues": issues},
    }
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(artifact,indent=2,sort_keys=True)+"\n")
    print(json.dumps({"summary":summary,"validation":{"issues":issues}},
                     indent=2,sort_keys=True))
    return 1 if issues else 0


if __name__ == "__main__":
    raise SystemExit(main())
