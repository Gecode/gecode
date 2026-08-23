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


def run(binary: Path, formulation: str) -> dict:
    command = [str(binary), "-formulation", formulation, "-solutions", "0"]
    start = time.perf_counter()
    completed = subprocess.run(command, check=True, text=True,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
    elapsed = time.perf_counter()-start
    lines = [line for line in completed.stdout.splitlines() if line.strip()]
    if len(lines) != 1:
      raise ValueError(f"unexpected solver output: {completed.stdout!r}")
    result = json.loads(lines[0])
    if result.get("status") != "ok":
      raise ValueError(f"solver did not report success: {result!r}")
    result["elapsed_seconds"] = elapsed
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--parent-binary", type=Path, required=True)
    parser.add_argument("--candidate-binary", type=Path, required=True)
    parser.add_argument("--repetitions", type=int, default=20)
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
    for binary, formulation in cases.values():
      run(binary,formulation)

    runs = []
    base_order = tuple(cases)
    for repetition in range(args.repetitions):
      offset=repetition % len(base_order)
      order=base_order[offset:]+base_order[:offset]
      if repetition % 2:
        order=tuple(reversed(order))
      for name in order:
        result=run(*cases[name])
        result.update(case=name,repetition=repetition+1)
        runs.append(result)

    summary = {}
    for name in cases:
      selected=[result for result in runs if result["case"] == name]
      first=selected[0]
      if any(any(result[field] != first[field] for field in COUNTERS)
             for result in selected):
        raise ValueError(f"unstable counters for {name}")
      summary[name] = {
        "median_seconds": statistics.median(
          result["elapsed_seconds"] for result in selected),
        "counters": {field: first[field] for field in COUNTERS},
      }

    parent=summary["parent-compact"]["counters"]
    compact=summary["candidate-compact"]["counters"]
    if parent != compact:
      raise ValueError("candidate compact counters differ from exact parent")
    bounded=summary["candidate-bounded"]["counters"]
    if (bounded["solutions"],bounded["checksum"]) != \
       (compact["solutions"],compact["checksum"]):
      raise ValueError("compact and bounded semantics differ")

    artifact = {
      "schema_version": 1,
      "repetitions": args.repetitions,
      "order": "rotated and reversed across repetitions",
      "binaries": {name: str(binary) for name, (binary, _) in cases.items()},
      "summary": summary,
      "runs": runs,
    }
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(artifact,indent=2,sort_keys=True)+"\n")
    print(json.dumps(summary,indent=2,sort_keys=True))


if __name__ == "__main__":
    main()
