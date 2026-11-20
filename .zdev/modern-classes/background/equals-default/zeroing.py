#!/usr/bin/env python3
"""List the class types that owned code value-initializes with zeroing.

Usage (from the source root): zeroing.py BUILD_DIR JOBS OUT
Runs clang-query (LLVM 22) on every owned source of BUILD_DIR's compilation
database and writes to OUT one line per class type that some expression
value-initializes by zeroing it first, with one location. A class gets such
an expression only if its default constructor is not user-provided, so
comparing the output before and after defaulting a default constructor shows
where the change makes `T()`, `T{}`, or an allocator's value-initialization
zero memory that the hand-written constructor left alone.
"""
import json
import multiprocessing
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path.cwd().resolve()
QUERY = "/opt/homebrew/opt/llvm@22/bin/clang-query"
OWNED = ("gecode/", "test/", "examples/", "tools/")
# The type is printed as 'written' or 'written':'canonical'; the second names a template argument's class.
ZEROING = re.compile(r"^CXX(?:Construct|TemporaryObject)Expr 0x\w+ <(.*?)> '([^']*)'(?::'([^']*)')? .*\bzeroing\b")
build_dir = Path()


def start_worker(directory: Path) -> None:
    global build_dir
    build_dir = directory


def query(path: str) -> list[tuple[str, str]]:
    result = subprocess.run([QUERY, "-p", str(build_dir), "--extra-arg=-UNDEBUG", "-c", "set traversal AsIs",
                             "-c", "set output dump", "-c", "match cxxConstructExpr(requiresZeroInitialization())",
                             path], capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"clang-query failed on {path}: {result.stderr[:2000]}")
    return [(m[3] or m[2], m[1]) for m in map(ZEROING.match, result.stdout.splitlines()) if m]


def main() -> None:
    build_dir = Path(sys.argv[1]).resolve()
    jobs = int(sys.argv[2])
    entries = json.loads((build_dir / "compile_commands.json").read_text())
    paths = sorted({str((Path(e["directory"]) / e["file"]).resolve()) for e in entries})
    paths = [p for p in paths if p.startswith(str(ROOT) + "/") and p[len(str(ROOT)) + 1:].startswith(OWNED)
             and not p[len(str(ROOT)) + 1:].startswith("gecode/third-party/")]
    found: dict[str, str] = {}
    with multiprocessing.Pool(jobs, start_worker, (build_dir,)) as pool:
        for matches in pool.imap(query, paths):
            for type_name, where in matches:
                found.setdefault(type_name, where.replace(str(ROOT) + "/", ""))
    with open(sys.argv[3], "w") as out:
        for type_name in sorted(found):
            out.write(f"{type_name}\t{found[type_name]}\n")
    print(f"{len(paths)} sources, {len(found)} zero-initialized class types")


if __name__ == "__main__":
    main()
