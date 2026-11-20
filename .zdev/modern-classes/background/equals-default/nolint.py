#!/usr/bin/env python3
"""Mark the special members that stay hand-written before the clang-tidy fix run.

Usage (from the source root): nolint.py KEEP_TSV
Each KEEP_TSV row is: path, the definition line that clang-tidy reports
(without its indentation), the reason the member stays, and, where the line
occurs more than once in the file, which occurrence (counting from 1). The
script puts a NOLINTNEXTLINE comment, indented like it, on its own line before
it, so that the comment survives clang-format and keeps lines within 120
columns.
"""
import sys
from pathlib import Path

COMMENT = "// NOLINTNEXTLINE(modernize-use-equals-default)"


def main() -> None:
    rows = [line.split("\t") for line in Path(sys.argv[1]).read_text().splitlines()
            if line and not line.startswith("#")]
    edits: dict[Path, list[str]] = {}
    for path, text, reason, *occurrence in rows:
        file = Path(path)
        lines = edits.setdefault(file, file.read_text().splitlines(keepends=True))
        matches = [i for i, line in enumerate(lines) if line.strip() == text]
        number = int(occurrence[0]) if occurrence else 1
        if (len(matches) != 1) if not occurrence else not (1 < len(matches) and number <= len(matches)):
            sys.exit(f"{path}: {len(matches)} lines match {text!r} ({reason})")
        index = matches[number - 1]
        line = lines[index]
        lines[index] = line[:len(line) - len(line.lstrip())] + COMMENT + "\n" + line
    for file, lines in edits.items():
        file.write_text("".join(lines))
    print(f"marked {len(rows)} members in {len(edits)} files")


if __name__ == "__main__":
    main()
