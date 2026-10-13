#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Generate plain-text ChangeLog content from changelog.in."""

import re
import sys
from collections import defaultdict


def main() -> int:
    sys.stdout.write(
        """Changelog for Gecode
==============================================================================

"""
    )

    first = True
    version = ""
    info = ""
    date = ""

    modclear = {
        "kernel": "Kernel",
        "search": "Search engines",
        "int": "Finite domain integers",
        "set": "Finite integer sets",
        "float": "Floats",
        "cpltset": "Finite integer sets with complete representation",
        "minimodel": "Minimal modeling support",
        "graph": "Graph constraints",
        "scheduling": "Scheduling constraints",
        "driver": "Script commandline driver",
        "iter": "Range and value iterators",
        "support": "Support algorithms and datastructures",
        "example": "Example scripts",
        "test": "Systematic tests",
        "gist": "Gist",
        "flatzinc": "Gecode/FlatZinc",
        "other": "General",
    }

    whatclear = {
        "bug": "Bug fixes",
        "performance": "Performance improvements",
        "documentation": "Documentation fixes",
        "new": "Additions",
        "removed": "Removals",
        "change": "Other changes",
    }

    rankclear = {"minor": "minor", "major": "major"}

    modorder = [
        "kernel",
        "search",
        "int",
        "set",
        "float",
        "cpltset",
        "scheduling",
        "graph",
        "minimodel",
        "driver",
        "iter",
        "support",
        "example",
        "test",
        "gist",
        "flatzinc",
        "other",
    ]

    whatorder = ["new", "removed", "change", "bug", "performance", "documentation"]

    text = defaultdict(str)

    def flush_current_release() -> None:
        sys.stdout.write(f"Changes in Version {version} ({date})\n\n")
        sys.stdout.write(f"Scope:{info}")
        for experimental in (False, True):
            if experimental and any(text[(True, mod, what)] for mod in modorder for what in whatorder):
                sys.stdout.write("Experimental changes\n")
            for mod in modorder:
                if any(text[(experimental, mod, what)] for what in whatorder):
                    sys.stdout.write(f"- {modclear[mod]}\n")
                    for what in whatorder:
                        k = (experimental, mod, what)
                        if text[k]:
                            sys.stdout.write(f"  - {whatclear[what]}\n" + text[k])
                            text[k] = ""
        sys.stdout.write("\n------------------------------------------------------------------------------\n\n")

    lines = sys.stdin.readlines()
    idx = 0
    current = None

    while True:
        if current is None:
            if idx >= len(lines):
                break
            current = lines[idx]
            idx += 1

        l = current
        current = None

        if re.match(r"^\#", l):
            continue
        l = l.replace("%Gecode", "Gecode")

        if re.match(r"^\[RELEASE\]", l):
            if not first:
                flush_current_release()

            first = False
            version = ""
            info = ""
            date = ""

            while idx < len(lines):
                l = lines[idx]
                idx += 1
                l = l.replace("%Gecode", "Gecode")
                if re.search(r"\[DESCRIPTION\]", l):
                    break
                m = re.search(r"Version:[\t ]*(.*)$", l)
                if m:
                    version = m.group(1)
                m = re.search(r"Date:[\t ]*(.*)$", l)
                if m:
                    date = m.group(1)

            while idx < len(lines):
                l = lines[idx]
                idx += 1
                l = l.replace("%Gecode", "Gecode")
                if re.search(r"\[ENTRY\]", l):
                    current = l
                    break
                info = info + " " + l
            continue

        if re.match(r"^\[ENTRY\]", l):
            experimental = False
            desc = ""
            bug = ""
            issue = ""
            rank = ""
            what = ""
            mod = ""
            thanks = ""

            while idx < len(lines):
                l = lines[idx]
                idx += 1
                l = l.replace("%Gecode", "Gecode")
                if re.search(r"\[DESCRIPTION\]", l):
                    break
                m = re.fullmatch(r"Experimental:[\t ]*(.*)\n?", l)
                if m:
                    value = m.group(1).rstrip("\n")
                    if value not in {"true", "false"}:
                        print("gentxtchangelog: invalid Experimental value " + repr(value), file=sys.stderr)
                        return 1
                    experimental = value == "true"
                m = re.search(r"Module:[\t ]*(.*)$", l)
                if m:
                    mod = m.group(1)
                m = re.search(r"What:[\t ]*(.*)$", l)
                if m:
                    what = m.group(1)
                m = re.search(r"Rank:[\t ]*(.*)$", l)
                if m:
                    rank = m.group(1)
                m = re.search(r"Bug:[\t ]*(.*)$", l)
                if m:
                    bug = m.group(1)
                m = re.search(r"Issue:[\t ]*(.*)$", l)
                if m:
                    issue = m.group(1)
                m = re.search(r"Thanks:[\t ]*(.*)$", l)
                if m:
                    thanks = m.group(1)

            while idx < len(lines):
                l = lines[idx]
                idx += 1
                l = l.replace("%Gecode", "Gecode")
                if re.search(r"\[ENTRY\]", l) or re.search(r"\[RELEASE\]", l):
                    current = l
                    break
                if not re.search(r"\[MORE\]", l):
                    if desc == "":
                        desc = l
                    else:
                        desc = desc + "      " + l

            if desc.endswith("\n"):
                desc = desc[:-1]

            rb = rankclear.get(rank, "")
            if bug != "":
                rb = rb + f", bugzilla entry {bug}"
            if issue != "":
                rb = rb + f", github issue {issue}"
            if thanks != "":
                rb = rb + f", thanks to {thanks}"

            k = (experimental, mod, what)
            text[k] = text[k] + f"    - {desc}({rb})\n"
            continue

    if not first and any(text.values()):
        flush_current_release()

    sys.stdout.write(
        """Initial release (2005-12-06)

 No changes, of course.

"""
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
