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
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CHANGELOG_IN = ROOT / "changelog.in"
CHANGELOG_HH = ROOT / "changelog.hh"
GEN_CHANGELOG = ROOT / "misc" / "genchangelog.py"

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


def check_changelog(*, fix: bool) -> int:
    generated = generate_changelog()
    errors = validate_changelog_entries(generated)

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
