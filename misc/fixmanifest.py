#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Inject Gecode assembly identity metadata into Windows manifest files."""

import os
import re
import sys


def main() -> int:
    argv = sys.argv[1:]
    dllsuffix = argv[0] if len(argv) > 0 else ""
    file = argv[1] if len(argv) > 1 else ""

    revx = ""
    revy = ""
    revz = ""
    arch = ""
    m = re.search(r"-([0-9]+)-([0-9]+)-([0-9]+)-[rd]-(x[0-9]+)\.dll", dllsuffix)
    if m:
        revx, revy, revz, arch = m.group(1), m.group(2), m.group(3), m.group(4)

    typ = ""
    name = ""
    if re.search(r"(.*)Gecode([A-Za-z]+)-[0-9]+-[0-9]+-[0-9]+-[rd]-x[0-9]+\.dll\.manifest", file):
        m = re.search(r"(.*)Gecode([A-Za-z]+)-[0-9]+-[0-9]+-[0-9]+-[rd]-x[0-9]+\.dll\.manifest", file)
        typ = "Modules"
        name = m.group(2) if m else ""
    elif re.search(r"(.*)/examples/(.+)\.exe", file):
        m = re.search(r"(.*)/examples/(.+)\.exe", file)
        typ = "Examples"
        name = m.group(2) if m else ""
    elif re.search(r"(.*)fzn-gecode\.exe", file):
        typ = "Tools"
        name = "FlatZinc"
    elif re.search(r"(.*)/test\.exe", file):
        typ = "Tools"
        name = "Test"

    if re.search(r"x64", arch):
        arch = "amd64"

    try:
        with open(file, "r", encoding="utf-8") as inmanifest:
            lines = inmanifest.readlines()
    except OSError:
        return 0

    out_tmp = f"{file}.tmp"
    with open(out_tmp, "w", encoding="utf-8") as outmanifest:
        for line in lines:
            outmanifest.write(line)
            if re.search(r"\<assembly ", line):
                outmanifest.write("  <assemblyIdentity type=\"win32\"\n")
                outmanifest.write(f"                    name=\"Gecode.{typ}.{name}\"\n")
                outmanifest.write(f"                    version=\"{revx}.{revy}.{revz}.0\"\n")
                outmanifest.write(f"                    processorArchitecture=\"{arch}\"\n")
                outmanifest.write("                    publicKeyToken=\"0000000000000000\"\n")
                outmanifest.write("  />\n")

    try:
        os.remove(file)
    except OSError:
        pass
    os.replace(out_tmp, file)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
