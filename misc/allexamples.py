#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Run all examples and expand supported -propagation/-model combinations."""

import os
import subprocess
import sys
from pathlib import Path
from typing import Iterable, Sequence


def run_cmd_lines(argv: Sequence[str]) -> Iterable[str]:
    proc = subprocess.Popen(list(argv), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    assert proc.stdout is not None
    for line in proc.stdout:
        yield line
    proc.wait()


def runexample(executable: Path, display_name: str, args: list[str]) -> None:
    sys.stdout.write("------------------------------------------------------------\n")
    if args:
        sys.stdout.write(f"Running {display_name} {' '.join(args)}\n")
    else:
        sys.stdout.write(f"Running {display_name}\n")
    for line in run_cmd_lines([str(executable), *args, "-time", "120000"]):
        sys.stdout.write(line)
    sys.stdout.write("------------------------------------------------------------\n")


def main() -> int:
    directory = sys.argv[1] if len(sys.argv) > 1 else ""
    examples_dir = Path(f"{directory}/examples")
    for entry in os.scandir(examples_dir):
        if not entry.is_file() or "." in entry.name:
            continue
        executable = Path(entry.path)
        filename = executable.name

        prop: list[str] = []
        model: list[str] = []
        for line in run_cmd_lines([str(executable), "-help"]):
            if "-propagation (" in line:
                l1 = line
                l1 = l1.split("-propagation (", 1)[1]
                l1 = l1.split(")", 1)[0]
                l1 = l1.replace(" ", "")
                prop = l1.split(",") if l1 else []
            elif "-model (" in line:
                l1 = line
                l1 = l1.split("-model (", 1)[1]
                l1 = l1.split(")", 1)[0]
                l1 = l1.replace(" ", "")
                model = l1.split(",") if l1 else []

        if len(prop) == 0:
            if len(model) == 0:
                runexample(executable, filename, [])
            else:
                for m in model:
                    runexample(executable, filename, ["-model", m])
        else:
            for p in prop:
                if len(model) == 0:
                    runexample(executable, filename, ["-propagation", p])
                else:
                    for m in model:
                        runexample(executable, filename, ["-propagation", p, "-model", m])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
