# Copyright (c) 2026 Mikael Zayenz Lagerkvist
# SPDX-License-Identifier: MIT
import sys
import statistics
import subprocess

if len(sys.argv) != 2:
    raise SystemExit("usage: bench-number-theory.py BENCHMARK_EXECUTABLE")

print("arity,pattern,implementation,median_us,nodes,solutions")
for n in (4,12,64,256):
    for pattern in ("distinct", "repeated", "alias"):
        solutions = []
        for implementation in ("product", "chain"):
            samples = [subprocess.check_output(
                [sys.argv[1], str(n), pattern, implementation, "500"],
                text=True, timeout=60).strip().split(",") for _ in range(3)]
            solutions.append(samples[0][2])
            print(n, pattern, implementation,
                  round(statistics.median(float(s[0]) for s in samples), 3),
                  samples[0][1], samples[0][2], sep=",")
        assert solutions[0] == solutions[1]
