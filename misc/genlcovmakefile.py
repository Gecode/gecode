#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Generate a Makefile that runs tests/examples and collects lcov coverage."""

import os
import subprocess
import sys
from pathlib import Path
from typing import Iterable, Sequence


def cmd_lines(argv: Sequence[str]) -> Iterable[str]:
    proc = subprocess.Popen(list(argv), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    assert proc.stdout is not None
    for line in proc.stdout:
        yield line
    proc.wait()


def parse_help_options(executable: str):
    prop: list[str] = []
    model: list[str] = []
    for line in cmd_lines([executable, "-help"]):
        if "-propagation (" in line:
            s = line.split("-propagation (", 1)[1].split(")", 1)[0].replace(" ", "")
            prop = s.split(",") if s else []
        elif "-model (" in line:
            s = line.split("-model (", 1)[1].split(")", 1)[0].replace(" ", "")
            model = s.split(",") if s else []
    return prop, model


def main() -> int:
    tests: dict[str, str] = {}
    block_size = 40

    for line in cmd_lines(["./test/test", "-list"]):
        tests[line.rstrip("\n")] = ""

    size = 0
    for _ in tests.keys():
        size += 1

    targets = size / block_size

    examples: list[str] = []
    for entry in os.scandir(Path("./examples")):
        if not entry.is_file() or "." in entry.name:
            continue
        filename = entry.name
        prop, model = parse_help_options(entry.path)
        if len(prop) == 0:
            if len(model) == 0:
                examples.append(filename)
            else:
                for m in model:
                    examples.append(f"{filename} -model {m}")
        else:
            for p in prop:
                if len(model) == 0:
                    examples.append(f"{filename} -propagation {p}")
                else:
                    for m in model:
                        examples.append(f"{filename} -propagation {p} -model {m}")

    sys.stdout.write("LCOVOUTDIR = /srv/gecode/httpd/html/Internal/gcov-trunk\n\n")
    sys.stdout.write("all: tests examples\n")
    sys.stdout.write("\tlcov --directory $(PWD) --base-directory $(PWD) --capture \\\n")
    sys.stdout.write("\t     --output-file testsandexamples.info\n")
    sys.stdout.write("\tgenhtml -t \"Gecode tests and examples\" testsandexamples.info -o $(LCOVOUTDIR) -p $(PWD)\n\n")

    sys.stdout.write("tests: \\\n")
    tcount = 0
    while tcount <= targets:
        sys.stdout.write(f"\ttest{tcount} \\\n")
        tcount += 1
    sys.stdout.write("\tdone\n\n")
    sys.stdout.write("examples: \\\n")
    for e in examples:
        etarget = re_target(e)
        sys.stdout.write(f"\t{etarget} \\\n")
    sys.stdout.write("\tdone\n\n")

    tcount = 1
    count = 0
    sys.stdout.write("test0:\n")
    sys.stdout.write("\t./test/test \\\n")
    for k in tests.keys():
        if count == block_size:
            sys.stdout.write("\t\t$(TESTOPTIONS)\n\n")
            sys.stdout.write(f"test{tcount}:\n")
            sys.stdout.write("\t./test/test \\\n")
            count = 0
            tcount += 1
        sys.stdout.write(f"\t\t-test {k} \\\n")
        count += 1
    if count < block_size:
        sys.stdout.write("\t\t$(TESTOPTIONS)\n\n")

    for e in examples:
        etarget = re_target(e)
        sys.stdout.write(f"{etarget}:\n")
        sys.stdout.write(f"\t./examples/{e} -time 240000\n\n")

    sys.stdout.write("done:\n")
    return 0


def re_target(value: str) -> str:
    return value.replace(" ", "_").replace("-", "_")


if __name__ == "__main__":
    raise SystemExit(main())
