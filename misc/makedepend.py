#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Compute include dependencies for Makefile.dep from C/C++ source files."""

import os
import re
import sys
from pathlib import Path


def open_with_root(root: str, rel: str):
    path1 = Path(root) / rel
    try:
        return open(path1, "r", encoding="utf-8")
    except OSError:
        return open(rel, "r", encoding="utf-8")


def main() -> int:
    args = sys.argv[1:]
    if not args:
        return 0
    i = 0
    root = args[i]
    i += 1

    while i < len(args):
        target = args[i]
        i += 1
        todo = [target]
        done: dict[str, str] = {}

        while todo:
            f = todo.pop()
            try:
                fh = open_with_root(root, f)
            except OSError:
                raise SystemExit(f"File missing: {root}/{f}")
            with fh:
                for line in fh:
                    m = re.match(r"^\#include <(gecode/.*)>", line)
                    if m:
                        g = re.sub(r"^\./", "", m.group(1))
                        if not re.match(r"^gecode/third-party", g) and g not in done:
                            todo.append(g)
                            if (Path(root) / g).exists():
                                done[g] = f"{root}/"
                            else:
                                done[g] = ""
                        continue
                    m = re.match(r'^\#include "(.*)"', line)
                    if m:
                        g = re.sub(r"^\./", "", m.group(1))
                        if not Path(g).exists() and not (Path(root) / g).exists():
                            d = os.path.dirname(f) or "."
                            g = f"{d}/{g}"
                        if g not in done:
                            todo.append(g)
                            if (Path(root) / g).exists():
                                done[g] = f"{root}/"
                            else:
                                done[g] = ""

        target_out = re.sub(r"\.cpp?", "", target)
        sys.stdout.write(f"{target_out}$(OBJSUFFIX) {target_out}$(SBJSUFFIX): ")
        l = 3
        for g in sorted(done.keys()):
            if l == 3:
                sys.stdout.write("\\\n\t")
                l = 0
            sys.stdout.write(f"{done[g]}{g} ")
            l += 1
        sys.stdout.write("\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
