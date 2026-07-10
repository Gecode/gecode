#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Run repository formatting and generated-file consistency checks."""

from __future__ import annotations

import argparse
import difflib
import re
import subprocess
import sys
from collections import Counter
from dataclasses import dataclass
from html.parser import HTMLParser
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CHANGELOG_IN = ROOT / "changelog.in"
CHANGELOG_HH = ROOT / "changelog.hh"
GEN_CHANGELOG = ROOT / "misc" / "genchangelog.py"
GEN_CURRENT_CHANGELOG = ROOT / "misc" / "gencurrentchangelog.py"

MODULES = {
    "kernel",
    "search",
    "int",
    "set",
    "float",
    "cpltset",
    "minimodel",
    "graph",
    "scheduling",
    "driver",
    "iter",
    "support",
    "example",
    "test",
    "gist",
    "flatzinc",
    "other",
}
WHATS = {"bug", "performance", "documentation", "new", "removed", "change"}
RANKS = {"minor", "major"}


@dataclass
class Release:
    line: int
    version: str
    date: str


@dataclass
class Entry:
    line: int
    release: Release
    module: str
    what: str
    rank: str
    bug: str
    issue: str
    thanks: str
    author: str
    description: str
    more: str


def run(cmd: list[str], *, stdin: str | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        cmd,
        cwd=ROOT,
        input=stdin,
        text=True,
        capture_output=True,
        check=False,
    )


def generate_changelog() -> str:
    source = CHANGELOG_IN.read_text()
    result = run(["uv", "run", str(GEN_CHANGELOG)], stdin=source)
    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        sys.stderr.write(result.stderr)
        raise SystemExit(result.returncode)
    return result.stdout


def generate_current_changelog(source: str) -> subprocess.CompletedProcess[str]:
    return run(["uv", "run", str(GEN_CURRENT_CHANGELOG)], stdin=source)


def is_tracked(path: Path) -> bool:
    rel = str(path.relative_to(ROOT))
    return run(["git", "ls-files", "--error-unmatch", rel]).returncode == 0


def parse_changelog() -> tuple[list[Release], list[Entry]]:
    lines = CHANGELOG_IN.read_text().splitlines(keepends=True)
    releases: list[Release] = []
    entries: list[Entry] = []
    current_release: Release | None = None
    idx = 0

    while idx < len(lines):
        line = lines[idx]
        if line.startswith("#") or not line.strip():
            idx += 1
            continue

        if line.startswith("[RELEASE]"):
            start = idx + 1
            idx += 1
            version = ""
            date = ""
            while idx < len(lines):
                current = lines[idx]
                idx += 1
                match = re.search(r"Version:[\t ]*(.*)$", current)
                if match:
                    version = match.group(1).rstrip("\n")
                match = re.search(r"Date:[\t ]*(.*)$", current)
                if match:
                    date = match.group(1).rstrip("\n")
                if "[DESCRIPTION]" in current:
                    break
            while idx < len(lines) and "[ENTRY]" not in lines[idx]:
                idx += 1
            current_release = Release(start, version, date)
            releases.append(current_release)
            continue

        if line.startswith("[ENTRY]"):
            if current_release is None:
                raise ValueError(f"entry at line {idx + 1} has no preceding release")

            start = idx + 1
            idx += 1
            metadata = {
                "module": "",
                "what": "",
                "rank": "",
                "bug": "",
                "issue": "",
                "thanks": "",
                "author": "",
            }
            fields = {
                "module": "Module",
                "what": "What",
                "rank": "Rank",
                "bug": "Bug",
                "issue": "Issue",
                "thanks": "Thanks",
                "author": "Author",
            }
            while idx < len(lines):
                current = lines[idx]
                idx += 1
                if "[DESCRIPTION]" in current:
                    break
                for key, field in fields.items():
                    match = re.search(rf"{field}:[\t ]*(.*)$", current)
                    if match:
                        metadata[key] = match.group(1).rstrip("\n")

            description: list[str] = []
            more: list[str] = []
            in_more = False
            while idx < len(lines) and "[ENTRY]" not in lines[idx] and "[RELEASE]" not in lines[idx]:
                current = lines[idx]
                idx += 1
                if "[MORE]" in current:
                    in_more = True
                    continue
                (more if in_more else description).append(current)

            entries.append(
                Entry(
                    line=start,
                    release=current_release,
                    description="".join(description),
                    more="".join(more),
                    **metadata,
                )
            )
            continue

        idx += 1

    return releases, entries


def changelog_text(raw: str) -> str:
    text = "".join("        " + line for line in raw.splitlines(keepends=True))
    text = re.sub(r"^        ", "", text)
    text = re.sub(r"[ \t\n\r]*$", "", text)
    text = text.replace("<", "\\<")
    text = text.replace(">", "\\>")
    text = text.replace("&", "\\&")
    return re.sub(r"\n([ \t]*)\n", r"\n\1<br>\n", text)


def generated_section(generated: str, version: str) -> str:
    section_id = version.replace(".", "_")
    marker = f"\\section SectionChanges_{section_id} "
    start = generated.find(marker)
    if start < 0:
        return ""
    end = generated.find("\n\\section SectionChanges_", start + 1)
    return generated[start:] if end < 0 else generated[start:end]


def validate_changelog_entries(generated: str) -> list[str]:
    releases, entries = parse_changelog()
    errors: list[str] = []

    for release in releases:
        section_id = release.version.replace(".", "_")
        heading = (
            f"\\section SectionChanges_{section_id} "
            f"Changes in Version {release.version} ({release.date})"
        )
        if heading not in generated:
            errors.append(f"line {release.line}: missing generated section for {release.version}")

    for entry in entries:
        if entry.module not in MODULES:
            errors.append(f"line {entry.line}: unknown module {entry.module!r}")
        if entry.what not in WHATS:
            errors.append(f"line {entry.line}: unknown change type {entry.what!r}")
        if entry.rank not in RANKS:
            errors.append(f"line {entry.line}: unknown rank {entry.rank!r}")

    if errors:
        return errors

    for entry in entries:
        section = generated_section(generated, entry.release.version)
        description = changelog_text(entry.description)
        position = section.find(description)
        if position < 0:
            errors.append(
                f"line {entry.line}: description missing from generated section "
                f"{entry.release.version}"
            )
            continue

        tail = section[position : position + len(description) + 800]
        expected = [entry.rank]
        if entry.bug:
            expected.append(f"bugzilla entry {entry.bug}")
        if entry.issue:
            expected.append(f"issue {entry.issue}")
        if entry.thanks:
            expected.append(f"thanks to {entry.thanks}")
        if entry.author:
            expected.append(f"contributed by {entry.author}")
        for item in expected:
            if item not in tail:
                errors.append(f"line {entry.line}: missing generated metadata {item!r}")

        if entry.more and changelog_text(entry.more) not in section:
            errors.append(f"line {entry.line}: [MORE] text missing from generated section")

    return errors


def current_changelog_text(raw: str) -> str:
    description = ""
    for line in raw.splitlines(keepends=True):
        description += line if not description else "      " + line
    if description.endswith("\n"):
        description = description[:-1]
    return description.replace("%Gecode", "Gecode")


def normalize_rendered_text(text: str) -> str:
    return " ".join(text.split())


def current_description_from_body(body: str) -> str | None:
    if not body.endswith(")"):
        return None
    depth = 0
    for position in range(len(body) - 1, -1, -1):
        if body[position] == ")":
            depth += 1
        elif body[position] == "(":
            depth -= 1
            if depth == 0:
                metadata = body[position + 1 : -1]
                if re.match(r"^(?:minor|major)(?:,|$)", metadata):
                    return body[:position].rstrip()
                return None
    return None


class CurrentChangelogEntryParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__()
        self._items: list[tuple[list[str], bool]] = []
        self.entries: list[str] = []
        self.errors: list[str] = []

    def handle_starttag(
        self, tag: str, attrs: list[tuple[str, str | None]]
    ) -> None:
        del attrs
        if tag == "li":
            self._items.append(([], False))
        elif tag == "ul" and self._items:
            text, _ = self._items[-1]
            self._items[-1] = (text, True)

    def handle_data(self, data: str) -> None:
        if self._items:
            self._items[-1][0].append(data)

    def handle_endtag(self, tag: str) -> None:
        if tag != "li" or not self._items:
            return
        text, has_nested_list = self._items.pop()
        if has_nested_list:
            return
        body = normalize_rendered_text("".join(text))
        description = current_description_from_body(body)
        if description is None:
            self.errors.append(f"entry has no rank metadata: {body!r}")
            return
        self.entries.append(description)


def rendered_current_descriptions(html: str) -> tuple[Counter[str], list[str]]:
    parser = CurrentChangelogEntryParser()
    parser.feed(html)
    parser.close()
    return Counter(parser.entries), parser.errors


def compare_current_descriptions(
    html: str, expected: list[str], *, context: str
) -> list[str]:
    actual, errors = rendered_current_descriptions(html)
    expected_counts = Counter(normalize_rendered_text(text) for text in expected)
    errors = [f"{context}: {error}" for error in errors]

    for description in sorted(expected_counts.keys() | actual.keys()):
        expected_count = expected_counts[description]
        actual_count = actual[description]
        if actual_count != expected_count:
            errors.append(
                f"{context}: entry {description!r} appears {actual_count} times, "
                f"expected {expected_count}"
            )
    return errors


def validate_current_changelog() -> list[str]:
    source = CHANGELOG_IN.read_text()
    result = generate_current_changelog(source)
    if result.returncode != 0:
        return [
            "current-release changelog generator failed: "
            + (result.stderr.strip() or result.stdout.strip())
        ]

    releases, entries = parse_changelog()
    if not releases:
        return ["changelog has no release for current-release generation"]
    current_entries = [entry for entry in entries if entry.release == releases[0]]
    expected_descriptions = [
        current_changelog_text(entry.description) for entry in current_entries
    ]
    errors = compare_current_descriptions(
        result.stdout, expected_descriptions, context="current-release HTML"
    )

    regression_source = """[RELEASE]
Version: 1.0.0
Date: 2000-01-01
[DESCRIPTION]
Test release.

[ENTRY]
Module: other
What:   change
Rank:   minor
[DESCRIPTION]
Short.

[ENTRY]
Module: other
What:   change
Rank:   minor
[DESCRIPTION]
Short. longer.

[ENTRY]
Module: other
What:   change
Rank:   minor
[DESCRIPTION]
Short.

[RELEASE]
Version: 0.9.0
Date: 1999-01-01
[DESCRIPTION]
Previous release.
"""
    regression_result = generate_current_changelog(regression_source)
    if regression_result.returncode != 0:
        errors.append(
            "current-release exact-entry regression failed to generate: "
            + (regression_result.stderr.strip() or regression_result.stdout.strip())
        )
    else:
        errors.extend(
            compare_current_descriptions(
                regression_result.stdout,
                ["Short.", "Short. longer.", "Short."],
                context="current-release exact-entry regression",
            )
        )

    invalid_fields = {
        "Module": ("unknown module", "invalid-module"),
        "What": ("unknown change type", "invalid-what"),
        "Rank": ("unknown rank", "invalid-rank"),
    }
    for field, (expected_error, value) in invalid_fields.items():
        metadata = {
            "Module": "other",
            "What": "change",
            "Rank": "minor",
        }
        metadata[field] = value
        invalid_source = f"""[RELEASE]
Version: 1.0.0
Date: 2000-01-01
[DESCRIPTION]
Test release.

[ENTRY]
Module: {metadata['Module']}
What:   {metadata['What']}
Rank:   {metadata['Rank']}
[DESCRIPTION]
Test entry.

[RELEASE]
Version: 0.9.0
Date: 1999-01-01
[DESCRIPTION]
Previous release.
"""
        invalid_result = generate_current_changelog(invalid_source)
        if invalid_result.returncode == 0 or expected_error not in invalid_result.stderr:
            errors.append(
                f"current-release generator did not reject invalid {field.lower()}"
            )

    return errors


def check_changelog(*, fix: bool) -> int:
    generated = generate_changelog()
    errors = validate_changelog_entries(generated)
    errors.extend(validate_current_changelog())

    if is_tracked(CHANGELOG_HH):
        if CHANGELOG_HH.exists() and CHANGELOG_HH.read_text() == generated:
            pass
        elif fix:
            CHANGELOG_HH.write_text(generated)
        else:
            current = CHANGELOG_HH.read_text() if CHANGELOG_HH.exists() else ""
            diff = difflib.unified_diff(
                current.splitlines(keepends=True),
                generated.splitlines(keepends=True),
                fromfile="changelog.hh",
                tofile="generated changelog.hh",
            )
            errors.append("changelog.hh is not up to date with changelog.in")
            errors.extend("  " + line.rstrip("\n") for line in diff)
    elif fix:
        CHANGELOG_HH.write_text(generated)

    if errors:
        print("Changelog tidy check failed:", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1

    print("Changelog tidy check passed")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="check without modifying tracked files")
    parser.add_argument("--fix", action="store_true", help="rewrite generated files where applicable")
    args = parser.parse_args()

    if args.check and args.fix:
        parser.error("--check and --fix are mutually exclusive")

    return check_changelog(fix=args.fix)


if __name__ == "__main__":
    raise SystemExit(main())
