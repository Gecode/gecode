#!/usr/bin/env python3
"""Cross-check clang-tidy's coverage of empty special members with a text scan.

Usage (from a git checkout's source root): coverage.py
Lists every default constructor, copy or cloning constructor, and destructor
in an owned C++ file whose definition is one line with no initializer list
and an empty body (`X(void) {}`, `X::X(const X&) {}`, `~X(void) {}`), unless
the line before carries the modernize-use-equals-default NOLINTNEXTLINE.
clang-tidy reaches only code that the build compiles; this scan also covers
headers no translation unit includes and branches of other configurations.
Review what it prints by hand.
"""
import re
import subprocess
from pathlib import Path

OWNED = ("gecode/", "test/", "examples/", "tools/")
GENERATED = {"gecode/flatzinc/parser.tab.cpp", "gecode/flatzinc/parser.tab.hpp", "gecode/flatzinc/lexer.yy.cpp"}
SUFFIXES = {".hh", ".hpp", ".h", ".cpp", ".cc"}
# An optional qualifier X<...>:: followed by X( or ~X( with the same name, then an empty body.
MEMBER = re.compile(r"^\s*(?:(?:\w+(?:<[^>]*>)?::)*?(\w+)(?:<[^>]*>)?::)?(~?)(\w+)\(([^()]*)\)\s*\{\s*\}\s*$")
MARK = "NOLINTNEXTLINE(modernize-use-equals-default)"


def main() -> None:
    names = subprocess.check_output(["git", "ls-files", "-z"], text=True).split("\0")
    for name in sorted(names):
        if not name.startswith(OWNED) or name.startswith("gecode/third-party/") or name in GENERATED \
                or Path(name).suffix not in SUFFIXES:
            continue
        lines = Path(name).read_text(errors="replace").splitlines()
        for number, line in enumerate(lines, 1):
            match = MEMBER.match(line)
            if not match or (match[1] and match[1] != match[3]):
                continue
            parameters = match[4].strip()
            if parameters not in ("", "void") and not re.fullmatch(rf"(const\s+)?{match[3]}(<[^>]*>)?\s*&\s*\w*",
                                                                     parameters):
                continue
            if number > 1 and MARK in lines[number - 2]:
                continue
            print(f"{name}:{number}: {line.strip()}")


if __name__ == "__main__":
    main()
