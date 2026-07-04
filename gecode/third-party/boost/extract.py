#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Emit shell commands to copy the Boost header closure for vendoring."""

import re
import sys


def main() -> int:
    library = sys.argv[1] if len(sys.argv) > 1 else ""
    dst = sys.argv[2] if len(sys.argv) > 2 else ""

    todo = [library]
    done: dict[str, int] = {library: 1}
    unresolved: dict[str, int] = {}
    defs: dict[str, str] = {}
    other: dict[str, int] = {}

    while todo:
        f = todo.pop()
        try:
            fh = open(f, "r", encoding="utf-8")
        except OSError:
            raise SystemExit(f"File missing: {f}")
        with fh:
            for line in fh:
                m = re.match(r"^\#( )*include <(boost/.*)>", line)
                if m:
                    g = re.sub(r"^\./", "", m.group(2))
                    if g not in done:
                        todo.append(g)
                        done[g] = 1
                    continue
                m = re.match(r'^\#( )*include "(boost/.*)"', line)
                if m:
                    g = re.sub(r"^\./", "", m.group(2))
                    if g not in done:
                        todo.append(g)
                        done[g] = 1
                    continue
                m = re.match(r"^\#( )*include <(.*)>", line)
                if m:
                    other[m.group(2)] = 1
                    continue
                m = re.match(r"^\#( )*include (.*)", line)
                if m:
                    k = m.group(2).rstrip("\n")
                    unresolved[k] = 1
                    continue
                m = re.match(r"^\#( )*define (GECODE_BOOST_[A-Z_]*) (.*)", line)
                if m:
                    k = m.group(2)
                    v = m.group(3).rstrip("\n")
                    qm = re.search(r'\"(.*)\"', v)
                    if qm:
                        v = qm.group(1)
                    am = re.search(r"<(.*)>", v)
                    if am:
                        v = am.group(1)
                    if k in defs:
                        defs[k] = defs[k] + "," + v
                    else:
                        defs[k] = v

        for u in list(unresolved.keys()):
            if u in defs:
                for d in defs[u].split(","):
                    if d not in done:
                        todo.append(d)
                        done[d] = 1

    dirs: dict[str, int] = {}
    for g in sorted(done.keys()):
        m = re.match(r"(.*)/.*", g)
        if m:
            dirs[m.group(1)] = 1

    for d in sorted(dirs.keys()):
        sys.stdout.write(f"mkdir -p \"{dst}{d}\"\n")
    for g in sorted(done.keys()):
        m = re.match(r"(.*)/.*", g)
        if m:
            d = m.group(1)
            sys.stdout.write(f"cp \"{g}\" \"{dst}{d}\"\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
