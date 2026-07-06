#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Generate Windows .rc resource files for Gecode DLLs and executables."""

import re
import sys


def main() -> int:
    dlldescription = {
        "Kernel": "kernel",
        "Search": "search engines",
        "Int": "finite domain integers",
        "Set": "finite integer sets",
        "Float": "floats",
        "Minimodel": "minimal modeling support",
        "Driver": "script commandline driver",
        "FlatZinc": "FlatZinc interpreter library",
        "Gist": "Gist",
        "Support": "support algorithms and datastructures",
    }

    i = 1
    argv = sys.argv

    dllsuffix = argv[i] if i < len(argv) else ""
    i += 1

    revx = ""
    revy = ""
    revz = ""
    mode = ""
    m = re.search(r"-([0-9]+)-([0-9]+)-([0-9]+)-([rd])-x[0-9]+\.dll", dllsuffix)
    if m:
        revx, revy, revz, mode = m.group(1), m.group(2), m.group(3), m.group(4)

    dir_ = argv[i] if i < len(argv) else ""
    i += 1
    file = argv[i] if i < len(argv) else ""
    i += 1

    srcfiles: list[str] = []
    while i < len(argv):
        srcfiles.append(f"{dir_}/{argv[i]}")
        i += 1

    sys.stdout.write(
        "/*\n"
        " *  CAUTION:\n"
        " *    This file has been automatically generated. Do not edit,\n"
        " *    edit the following files instead:\n"
        f" *     - {dir_}/misc/genrc.py\n"
    )
    for src in srcfiles:
        sys.stdout.write(f" *     - {src}\n")

    sys.stdout.write(
        """ *
 *  This file contains generated code fragments which are
 *  copyrighted as follows:
 *
 *  Main author:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2010
 *
 *  The generated code fragments are part of Gecode, the generic
 *  constraint development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

"""
    )

    authors: dict[str, int] = {}
    for src in srcfiles:
        with open(src, "r", encoding="utf-8") as fh:
            incopyright = False
            for line in fh:
                if incopyright:
                    if re.search(r"\*.*part of Gecode, the", line):
                        incopyright = False
                    else:
                        m = re.search(r"\*\s*([^,]*)(, [0-9]+)+", line)
                        if m:
                            authors[m.group(1)] = 1
                elif re.search(r"\*.*Copyright:", line):
                    incopyright = True

    copyright_text = ", ".join(sorted(authors.keys()))

    filename = ""
    filetype = ""
    icon = 0
    basename = ""
    description = ""

    m = re.search(r"Gecode(.*)-([0-9]+-[0-9]+-[0-9]+-[rd]-.+)\.dll$", file)
    if m:
        filename = file
        filetype = "VFT_DLL"
        icon = 0
        basename = f"Gecode{m.group(1)}-{m.group(2)}"
        description = "Gecode " + dlldescription[m.group(1)]
    else:
        filetype = "VFT_APP"
        icon = 1
        if re.search(r"fzn-gecode\.exe", file):
            filename = "fzn-gecode.exe"
            basename = "fzn-gecode"
            description = "Gecode FlatZinc interpreter"
        elif re.search(r"test\.exe", file):
            filename = "test.exe"
            basename = "test"
            description = "Gecode systematic tests"
        else:
            m = re.search(r"examples/(.+)\.exe", file)
            if m:
                filename = f"{m.group(1)}.exe"
                basename = m.group(1)
                example = ""
                if srcfiles:
                    with open(srcfiles[0], "r", encoding="utf-8") as fh:
                        for line in fh:
                            mm = re.search(r"\\brief.*Example: (.*)", line)
                            if mm:
                                example = mm.group(1)
                example = example.replace("%", "")
                description = f"Gecode example: {example}"
            else:
                raise SystemExit(f"Illegal file {file}")

    debug = 0 if re.search(r"r", mode) else "VS_FF_DEBUG"

    sys.stdout.write("\n#include <windows.h>\n\n")
    if icon:
        sys.stdout.write('0 ICON "misc/gecode-logo.ico"\n')

    sys.stdout.write(
        f"""
VS_VERSION_INFO VERSIONINFO
FILEVERSION    \t{revx},{revy},{revz},0
PRODUCTVERSION \t{revx},{revy},{revz},0
FILEFLAGSMASK  \tVS_FFI_FILEFLAGSMASK
FILEFLAGS      \t{debug}
FILEOS         \tVOS_UNKNOWN
FILETYPE       \t{filetype}
FILESUBTYPE    \tVFT2_UNKNOWN
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"
        BEGIN
            VALUE "CompanyName",      "Gecode team"
            VALUE "FileDescription",  "{description}"
            VALUE "FileVersion",      "{revx}.{revy}.{revz}"
            VALUE "InternalName",     "{basename}"
            VALUE "LegalCopyright",   "{copyright_text} (license information available from www.gecode.dev)"
            VALUE "OriginalFilename", "{filename}"
            VALUE "ProductName",      "Gecode"
            VALUE "ProductVersion",   "{revx}.{revy}.{revz}"
        END
    END

    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END

"""
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
