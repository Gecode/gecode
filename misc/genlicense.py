#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Generate license.hh by extracting authors and contributor names from inputs."""

import fileinput
import argparse
import re
import sys


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

    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")

    main_authors: dict[str, str] = {}
    fix_authors: dict[str, str] = {}

    lines = list(fileinput.input(files=input_files(args.files, args.input_list),
                                 encoding="utf-8"))
    i = 0
    while i < len(lines):
        line = lines[i]
        i += 1
        if re.search(r"Main authors:", line) or re.search(r"Contributing authors:", line):
            while i < len(lines):
                line = lines[i]
                m = re.search(r"([A-Z].*) <(.*)>", line)
                if not m:
                    break
                main_authors[m.group(1)] = m.group(2)
                i += 1
        elif re.search(r"provided by:", line):
            while i < len(lines):
                line = lines[i]
                m = re.search(r"([A-Z][^<]*)", line)
                if not m:
                    break
                fix_authors[m.group(1)] = ""
                i += 1

    sys.stdout.write(
        """/**
  \\page PageLic %Gecode license and authors

\\section SectLicLic GECODE LICENSE AGREEMENT

This software and its documentation are copyrighted by the
individual authors as listed in each file. All files, unless explicitly disclaimed in individual files, may be used according to the terms of the
<a href=\"http://www.opensource.org/licenses/mit-license.php\">
MIT license</a>, which reads as follows:

<TT>
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
\"Software\"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
</TT>

\\section SecLicAuthors Gecode authors

The following authors have contributed to the %Gecode library
and its documentation (in alphabetical order of firstname):
"""
    )

    for n in sorted(main_authors.keys()):
        e = main_authors[n]
        sys.stdout.write(f" - {n} <{e}>\n")

    sys.stdout.write(
        """
The above authors are grateful to the following people who have provided
bugfixes:
"""
    )

    for n in sorted(fix_authors.keys()):
        sys.stdout.write(f" - {n}\n")

    sys.stdout.write(
        """
We are also grateful for our users reporting bugs and suggesting
improvements.

*/
"""
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
