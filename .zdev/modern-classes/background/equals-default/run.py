#!/usr/bin/env python3
"""Run the special-member checks of one variant with misc/check.py lint coverage.

Copied from the run.py appendix of ../inventory.md, with the equals variant
added and the root taken from the working directory, so that it also runs in
a source tree without git metadata.

Usage (from the source root): run.py BUILD_DIR OUT_DIR JOBS VARIANT
VARIANT is one of default, macros, smf-lenient, equals (see VARIANTS).
Writes OUT_DIR/{config.json,commands.json,tidy.log,parser-actions.log,
lexer-actions.log,parser-hh.log,raw.txt,fixes/,fixes-*.yaml}.
raw.txt has one line per reported diagnostic: path:line:col: check: message.
"""
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path.cwd().resolve()
LLVM = Path("/opt/homebrew/opt/llvm@22/bin")
TIDY = LLVM / "clang-tidy"
RUNNER = LLVM / "run-clang-tidy"
GENERATED_PARSER = {"gecode/flatzinc/parser.tab.cpp", "gecode/flatzinc/parser.tab.hpp",
                    "gecode/flatzinc/lexer.yy.cpp"}
DEFAULTABLE_CHECKS = ["modernize-use-equals-default", "modernize-use-equals-delete",
                      "modernize-use-default-member-init", "cppcoreguidelines-special-member-functions"]
SPECIAL_MEMBERS = "cppcoreguidelines-special-member-functions"
LENIENT_OPTIONS = ["AllowSoleDefaultDtor", "AllowMissingMoveFunctions",
                   "AllowMissingMoveFunctionsWhenCopyIsDeleted", "AllowImplicitlyDeletedCopyOrMove"]
# Each variant: the checks it enables and the CheckOptions it sets (all others keep their defaults).
VARIANTS = {
    "default": (DEFAULTABLE_CHECKS, {}),
    "macros": (DEFAULTABLE_CHECKS, {f"{check}.IgnoreMacros": "false" for check in DEFAULTABLE_CHECKS}),
    "smf-lenient": ([SPECIAL_MEMBERS], {f"{SPECIAL_MEMBERS}.{option}": "true" for option in LENIENT_OPTIONS}),
    "equals": (DEFAULTABLE_CHECKS[:2], {}),
}
DIAGNOSTIC = re.compile(r"^(/[^:\n]+):(\d+):(\d+): warning: (.*) \[([\w.,-]+)\]$")


def path_pattern(path: Path) -> str:
    return r"[\\/]".join(re.escape(part) for part in path.as_posix().split("/"))


def action_ranges(source: Path, grammar: str) -> list[list[int]]:
    """Physical line ranges of source that #line maps to grammar (as misc/check.py lint)."""
    ranges, start = [], None
    lines = source.read_text().splitlines()
    for number, line in enumerate(lines, 1):
        directive = re.match(r'#line \d+ "([^"]+)"', line)
        if directive:
            if start is not None and number > start:
                ranges.append([start, number - 1])
            start = number + 1 if Path(directive[1]).name == grammar else None
    if start is not None:
        ranges.append([start, len(lines)])
    if not ranges:
        sys.exit(f"no {grammar} ranges in {source}")
    return ranges


def lint_commands(build_dir: Path, out: Path, jobs: str, checks: str, config: str) -> list[tuple[str, list[str]]]:
    """The four commands of misc/check.py lint, with checks and config in place of .clang-tidy."""
    sep = r"[\\/]"
    excluded = path_pattern(Path("gecode/third-party")) + sep + "|" + "|".join(
        path_pattern(Path(p)) + "$" for p in sorted(GENERATED_PARSER))
    owned = "(gecode|test|examples|tools)" + sep
    source_filter = "^" + path_pattern(ROOT) + sep + "(?!" + excluded + ")" + owned
    header_filter = path_pattern(ROOT) + sep + owned
    generated_headers = path_pattern(build_dir / "gecode/kernel") + sep + r"var-(type|imp)\.hpp$"
    exclude_filter = (sep + path_pattern(Path("gecode/third-party")) + sep + "|" + sep
                      + path_pattern(Path("gecode/flatzinc")) + sep
                      + r"(parser\.tab\.(cpp|hpp)|lexer\.yy\.cpp)$")
    hf = f"({header_filter}|{generated_headers})"
    common = [f"--checks={checks}", f"--config={config}", "--quiet"]
    commands = [("tidy.log", [str(RUNNER), "-clang-tidy-binary", str(TIDY), f"-checks={checks}",
                 f"-config={config}", "-p", str(build_dir), "-j", jobs, "-quiet", "-extra-arg=-UNDEBUG",
                 "-header-filter", hf, "-exclude-header-filter", exclude_filter,
                 "-source-filter", source_filter, "-export-fixes", str(out / "fixes")])]
    for log, generated, grammar in (("parser-actions.log", "parser.tab.cpp", "parser.yxx"),
                                    ("lexer-actions.log", "lexer.yy.cpp", "lexer.lxx")):
        source = ROOT / "gecode/flatzinc" / generated
        line_filter = json.dumps([{"name": str(source), "lines": action_ranges(source, grammar)}])
        commands.append((log, [str(TIDY), "--extra-arg=-UNDEBUG", "-p", str(build_dir), *common,
                         f"--export-fixes={out / ('fixes-' + log.removesuffix('.log') + '.yaml')}",
                         "--line-filter=" + line_filter, str(source)]))
    commands.append(("parser-hh.log", [str(TIDY), "--extra-arg=-UNDEBUG", "-p", str(build_dir), *common,
                     f"--export-fixes={out / 'fixes-parser-hh.yaml'}", f"--header-filter={hf}",
                     f"--exclude-header-filter={exclude_filter}", str(ROOT / "gecode/flatzinc/parser.hh")]))
    return commands


def relative(path: str) -> str:
    return str(Path(path).resolve().relative_to(ROOT))


def main() -> None:
    build_dir = Path(sys.argv[1]).resolve()
    out = Path(sys.argv[2]).resolve()
    jobs = sys.argv[3]
    check_list, options = VARIANTS[sys.argv[4]]
    out.mkdir(parents=True, exist_ok=True)
    (out / "fixes").mkdir(exist_ok=True)
    checks = "-*," + ",".join(check_list)
    config = json.dumps({"Checks": checks, "CheckOptions": options})
    (out / "config.json").write_text(config + "\n")
    commands = lint_commands(build_dir, out, jobs, checks, config)
    with open(out / "commands.json", "w") as f:
        json.dump([c for _, c in commands], f, indent=1)
    raw = []
    for log, command in commands:
        result = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        (out / log).write_text(result.stdout)
        found = [m.groups() for m in map(DIAGNOSTIC.match, result.stdout.splitlines()) if m]
        print(f"{out.name}/{log}: exit {result.returncode}, {len(found)} diagnostics")
        # The config sets no WarningsAsErrors, so a nonzero exit is a tool or compile failure.
        if result.returncode != 0:
            sys.exit(f"{log} failed; see {out / log}")
        raw += found
    with open(out / "raw.txt", "w") as f:
        for path, line, col, message, check in raw:
            f.write(f"{relative(path)}:{line}:{col}: {check}: {message}\n")
    print(f"{out.name}: raw {len(raw)} unique {len(set(raw))}")


if __name__ == "__main__":
    main()
