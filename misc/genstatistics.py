#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Generate stat.hh code metrics from files annotated with // STATISTICS tags."""

import re
import sys
import argparse
from collections import defaultdict


def input_files(args: list[str], input_list: str | None) -> list[str]:
    files = list(args)
    if input_list is not None:
        with open(input_list, "r", encoding="utf-8") as fh:
            files.extend(line.rstrip("\n") for line in fh if line.strip())
    return files


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-list")
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    modorder = [
        "kernel",
        "search",
        "int",
        "set",
        "float",
        "minimodel",
        "gist",
        "driver",
        "iter",
        "support",
        "example",
        "test",
        "flatzinc",
    ]

    catorder = [
        "core",
        "memory",
        "var",
        "prop",
        "branch",
        "post",
        "int",
        "set",
        "float",
        "minimodel",
        "stress",
        "trace",
        "any",
        "seq",
        "par",
        "search",
        "flatzinc",
        "other",
    ]

    modclear = {
        "kernel": "Kernel",
        "search": "Search engines",
        "gist": "Gist",
        "int": "Integer and Boolean variables and constraints",
        "set": "Set variables and constraints",
        "float": "Float variables and constraints",
        "minimodel": "Minimal modeling support",
        "driver": "Script commandline driver",
        "iter": "Range and value iterators",
        "support": "Support algorithms and datastructures",
        "example": "%Example scripts",
        "test": "Systematic tests",
        "flatzinc": "FlatZinc interpreter",
    }

    catclear = {
        "core": "Core functionality",
        "memory": "Memory management",
        "trace": "Tracing support",
        "var": "Variables, views, and variable implementations",
        "prop": "Propagators",
        "branch": "Branchers",
        "seq": "Sequential search engines",
        "par": "Parallel search engines",
        "post": "Posting propagators for constraints",
        "any": "All",
        "other": "Miscellaneous",
        "int": "Integer and Boolean variables and constraints",
        "set": "Set variables and constraints",
        "float": "Float variables and constraints",
        "minimodel": "Minimal modeling support",
        "stress": "System stress",
        "search": "Search",
        "flatzinc": "FlatZinc interpreter",
        "ignore": "ignored",
    }

    class_count = defaultdict(int)
    comment = defaultdict(int)
    blank = defaultdict(int)
    code = defaultdict(int)

    for file in input_files(args.files, args.input_list):
        with open(file, "r", encoding="utf-8") as fh:
            module = "UNKNOWN"
            cat = "UNKNOWN"
            n_blank = 0
            n_comment = 0
            n_code = 0
            n_class = 0
            lines = fh.readlines()
            i = 0
            while i < len(lines):
                line = lines[i]
                i += 1
                m = re.search(r"// STATISTICS: ([A-Za-z]+)-([A-Za-z]+)", line)
                if m:
                    module = m.group(1)
                    cat = m.group(2)
                    if module not in modclear:
                        raise SystemExit(f"Module {module} undefined (file {file})")
                    if cat not in catclear:
                        raise SystemExit(f"Category {cat} undefined (file {file})")
                if re.match(r"^[ \t]*$", line):
                    n_blank += 1
                elif re.match(r"^[ \t]*//", line):
                    n_comment += 1
                elif re.search(r"/\*", line):
                    n_comment += 1
                    while not re.search(r"\*/", line) and i < len(lines):
                        line = lines[i]
                        i += 1
                        n_comment += 1
                else:
                    n_code += 1
                if re.search(r"class [A-Za-z_][A-Za-z0-9_]*.*\{", line):
                    n_class += 1

        if module == "UNKNOWN":
            sys.stdout.write(f"UNKNOWN: {file}\n")
            return 1
        key = f"{module}-{cat}"
        class_count[key] += n_class
        comment[key] += n_comment
        blank[key] += n_blank
        code[key] += n_code

    sys.stdout.write(
        """/**
  \\page PageCodeStat Gecode code statistics

The following approximate breakdown into the different parts of %Gecode
gives some statistics about the amount of code and documentation
contained in the code (as comments) where blank lines are excluded.

The abbreviation \"loc\" means \"lines of code\" and \"lod\" means \"lines of
documentation\".

"""
    )

    an_code = 0
    an_comment = 0
    an_class = 0

    for module in modorder:
        sys.stdout.write(f" - {modclear[module]}: ")
        mn_code = 0
        mn_comment = 0
        mn_class = 0
        n_cat = 0
        doc = ""
        for cat in catorder:
            k = f"{module}-{cat}"
            if code[k]:
                n_cat += 1
                doc += f"   - {catclear[cat]}: "
                mn_code += code[k]
                mn_comment += comment[k]
                mn_class += class_count[k]
                doc += f"{class_count[k]} classes, "
                doc += f"{code[k]} loc, "
                doc += f"{comment[k]} lod\n"
        sys.stdout.write(f"{mn_class} classes, {mn_code} loc, {mn_comment} lod\n")
        if n_cat > 1:
            sys.stdout.write(doc)
        an_code += mn_code
        an_comment += mn_comment
        an_class += mn_class

    sys.stdout.write(
        f"""
The grand total: {an_class} classes, {an_code} loc, {an_comment} lod

*/
"""
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
