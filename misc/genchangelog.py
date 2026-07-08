#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Generate doxygen changelog.hh from changelog.in release and entry blocks."""

import re
import sys
from collections import defaultdict


def trim_trailing_ws(text: str) -> str:
    return re.sub(r"[ \t\n\r]*$", "", text)


def main() -> int:
    first = True
    version = ""
    date = ""
    moreinfocnt = 1
    info = ""

    modclear = {
        "kernel": "Kernel",
        "search": "Search engines",
        "int": "Finite domain integers",
        "set": "Finite integer sets",
        "float": "Floats",
        "cpltset": "Finite integer sets with complete representation",
        "minimodel": "Minimal modeling support",
        "graph": "%Graph constraints",
        "scheduling": "Scheduling constraints",
        "driver": "Script commandline driver",
        "iter": "Range and value iterators",
        "support": "Support algorithms and datastructures",
        "example": "%Example scripts",
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

    hastext = defaultdict(int)
    text = defaultdict(str)

    versions = ""
    body = ""

    def flush_current_release() -> None:
        nonlocal versions, body

        sid = f"SectionChanges_{version}"
        sid = sid.replace(".", "_")
        versions = versions + f'\n - \\ref {sid} "Gecode {version} ({date})"'
        body = body + f"\\section {sid} Changes in Version {version} ({date})\n\n"
        body = body + f"{info}\n\n"

        for mod in modorder:
            if hastext[mod]:
                body = body + " - " + modclear[mod] + "\n"
                hastext[mod] = 0
                for what in whatorder:
                    k = f"{mod}-{what}"
                    if text[k] != "":
                        body = body + "   - " + whatclear[what] + "\n"
                        body = body + text[k]
                        text[k] = ""
        body = body + "\n\n"

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
                m = re.search(r"Version:[\t ]*(.*)$", l)
                if m:
                    version = m.group(1)
                m = re.search(r"Date:[\t ]*(.*)$", l)
                if m:
                    date = m.group(1)
                if re.search(r"\[DESCRIPTION\]", l):
                    break

            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.search(r"\[ENTRY\]", l):
                    current = l
                    break
                info = info + l
            continue

        if re.match(r"^\[ENTRY\]", l):
            mod = ""
            what = ""
            rank = ""
            bug = ""
            issue = ""
            thanks = ""
            author = ""
            desc = ""
            more = ""

            while idx < len(lines):
                l = lines[idx]
                idx += 1
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
                m = re.search(r"Author:[\t ]*(.*)$", l)
                if m:
                    author = m.group(1)
                if re.search(r"\[DESCRIPTION\]", l):
                    break

            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.search(r"\[ENTRY\]", l) or re.search(r"\[RELEASE\]", l) or re.search(r"\[MORE\]", l):
                    break
                desc = desc + "        " + l

            desc = re.sub(r"^        ", "", desc)
            desc = trim_trailing_ws(desc)
            desc = desc.replace("<", "\\<")
            desc = desc.replace(">", "\\>")
            desc = desc.replace("&", "\\&")
            desc = re.sub(r"\n([ \t]*)\n", r"\n\1<br>\n", desc)

            if re.match(r"^\[MORE\]", l):
                while idx < len(lines):
                    l = lines[idx]
                    idx += 1
                    if re.search(r"\[ENTRY\]", l) or re.search(r"\[RELEASE\]", l):
                        break
                    more = more + "        " + l

            more = trim_trailing_ws(more)
            more = more.replace("<", "\\<")
            more = more.replace(">", "\\>")
            more = more.replace("&", "\\&")
            more = re.sub(r"\n([ \t]*)\n", r"\n\1<br>\n", more)

            hastext[mod] = 1
            rb = rankclear.get(rank, "")
            if bug != "":
                rb = rb + f", bugzilla entry {bug}"
            if issue != "":
                rb = rb + f', <a href="https://github.com/gecode/gecode/issues/{issue}">issue {issue}</a>'
            if thanks != "":
                rb = rb + f", thanks to {thanks}"
            if author != "":
                rb = rb + f", contributed by {author}"

            if more != "":
                more = re.sub(r"^        ", "", more)
                more = trim_trailing_ws(more)
                more = (
                    f"        <span id=lesslink{moreinfocnt} style=\"display: inline\">\n"
                    f"        <a href=\"javascript:showInfo({moreinfocnt}, true)\">Details</a></span>\n"
                    f"        <span id=morelink{moreinfocnt} style=\"display: none\">\n"
                    f"        <a href=\"javascript:showInfo({moreinfocnt}, false)\">Hide details</a></span>\n"
                    f"        <div id=moreinfo{moreinfocnt} style=\"display: none\">\n"
                    f"        {more}\n</div>\n"
                )
                moreinfocnt += 1

            k = f"{mod}-{what}"
            text[k] = text[k] + f"      - {desc} ({rb})\n" + more
            current = l
            continue

    if not first:
        flush_current_release()

    sys.stdout.write(
        f"""/**

\\page PageChange Changelog

\\section SectionChangeList Changes in Gecode Versions

{versions}

\\section SectionChangeWhat Gecode Version Numbers

%Gecode version numbers <em>x</em>.<em>y</em>.<em>z</em> change
according to the following rules (of thumb):

 - when \\e z changes, the programming interfaces for
   \\ref TaskModel, and \\ref TaskSearch remain
   stable and only minor additions or improvements are included.
 - when \\e y changes, the above mentioned interfaces might have changed
   and medium to major additions or improvements are included.
 - the change of \\e x is reserved for radical changes to %Gecode.

"""
    )

    sys.stdout.write(body)
    sys.stdout.write("\n*/\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
