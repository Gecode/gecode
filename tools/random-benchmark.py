#!/usr/bin/env python3
"""Compare compiled random-benchmark executables with bounded repeated runs.

Example: python3 tools/random-benchmark.py build/random/random-benchmark \
  --baseline /tmp/baseline/random-benchmark --repeat 5 --output results.json
Build instructions and the controls are recorded in docs/random.md.
"""
import argparse
import json
import platform
import statistics
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--repeat", type=int, default=5)
    parser.add_argument("--draws", type=int, default=1_000_000)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if args.repeat < 1 or args.draws < 1:
        parser.error("repeat and draws must be positive")
    if not 0 <= args.seed <= 0xffffffff:
        parser.error("the baseline comparison requires a 32-bit unsigned seed")
    binaries = {"candidate": args.candidate.resolve()}
    if args.baseline:
        binaries["baseline"] = args.baseline.resolve()
    records = {name: [] for name in binaries}
    for iteration in range(args.repeat + 1):
        # Reverse order on alternating repetitions; first repetition is warmup.
        names = list(binaries)
        if iteration % 2:
            names.reverse()
        for name in names:
            run = subprocess.run(
                [str(binaries[name]), str(args.draws), str(args.seed)],
                capture_output=True, text=True, timeout=120,
            )
            if run.returncode:
                parser.exit(1, f"{binaries[name]} failed:\n{run.stdout}\n{run.stderr}")
            rows = {}
            for line in run.stdout.splitlines():
                case, value, unit, checksum = line.split("\t")
                rows[case] = {"value": float(value), "unit": unit, "checksum": checksum}
            if iteration:
                records[name].append(rows)
    medians = {
        name: {case: statistics.median(run[case]["value"] for run in runs)
               for case in runs[0]}
        for name, runs in records.items()
    }
    for case, value in medians["candidate"].items():
        unit = records["candidate"][0][case]["unit"]
        previous = medians.get("baseline", {}).get(case)
        comparison = f" (baseline {previous:.2f}, ratio {value / previous:.3f})" if previous else ""
        print(f"{case}: {value:.2f} {unit}{comparison}")
    if args.output:
        # Preserve earlier runs; choose a new output name to collect another run.
        with args.output.open("x") as out:
            json.dump({"platform": platform.platform(), "machine": platform.machine(),
                       "draws": args.draws, "seed": args.seed,
                       "binaries": {k: str(v) for k, v in binaries.items()},
                       "runs": records, "medians": medians}, out, indent=2)
            out.write("\n")


if __name__ == "__main__":
    main()
