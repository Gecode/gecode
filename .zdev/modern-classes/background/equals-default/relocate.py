#!/usr/bin/env python3
"""Move out-of-line `= default` special members to their in-class declaration.

Usage (from the source root): relocate.py BUILD_DIR JOBS

clang-tidy's modernize-use-equals-default fix rewrites a body where it
stands, so an out-of-line definition becomes an out-of-line `= default`,
which is user-provided and keeps the class non-trivial. This script parses
the owned translation units of BUILD_DIR's compilation database with libclang
(LLVM 22), finds every out-of-line defaulted constructor, destructor, or
assignment in an owned file, deletes the definition, and adds `= default` to
the declaration in the class. Virtual members stay where they are, so no
class changes the translation unit that emits its vtable.
"""
import glob
import json
import multiprocessing
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path.cwd().resolve()
LLVM = Path("/opt/homebrew/opt/llvm@22")
sys.path.insert(0, sorted(glob.glob(str(LLVM / "lib/python3*/site-packages")))[-1])
import clang.cindex as ci  # noqa: E402

ci.Config.set_library_file(str(LLVM / "lib/libclang.dylib"))
RESOURCE_DIR = subprocess.check_output([str(LLVM / "bin/clang"), "-print-resource-dir"], text=True).strip()
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx"}
OWNED = ("gecode/", "test/", "examples/", "tools/")
K = ci.CursorKind
MEMBERS = {K.CONSTRUCTOR, K.DESTRUCTOR, K.CXX_METHOD}

compile_args: dict[str, list[str]] = {}


def is_owned(name: str) -> bool:
    if not name.startswith(str(ROOT) + "/"):
        return False
    relative = name[len(str(ROOT)) + 1:]
    return relative.startswith(OWNED) and not relative.startswith("gecode/third-party/")


def load_database(build_dir: Path) -> None:
    for entry in json.loads((build_dir / "compile_commands.json").read_text()):
        path = str((Path(entry["directory"]) / entry["file"]).resolve())
        args = entry["arguments"] if "arguments" in entry else entry["command"].split()
        result, skip = [], False
        for a in args[1:]:
            if skip:
                skip = False
            elif a in ("-o", "-MF", "-MT", "-MQ"):
                skip = True
            elif a not in ("-c", entry["file"], path):
                result.append(a)
        compile_args.setdefault(path, result + ["-UNDEBUG", "-resource-dir", RESOURCE_DIR])


def walk(cursor, found: set) -> None:
    for child in cursor.get_children():
        if child.location.file is None or not is_owned(child.location.file.name):
            continue
        # An out-of-line member declaration is always a definition, but libclang does not call a defaulted
        # member of a class template a definition, so is_definition() would miss those.
        if child.kind in MEMBERS and child.is_default_method() and child.lexical_parent != child.semantic_parent \
                and not child.is_virtual_method():
            declaration = child.canonical
            found.add((child.location.file.name, child.extent.start.offset, child.extent.end.offset,
                       declaration.location.file.name, declaration.extent.end.offset))
        elif child.kind not in MEMBERS:
            walk(child, found)


def collect(path: str) -> set | str:
    unit = ci.Index.create().parse(path, args=compile_args[path])
    errors = [str(d) for d in unit.diagnostics if d.severity >= ci.Diagnostic.Error]
    if errors:
        return f"{path}: {errors[0]}"
    found: set = set()
    walk(unit.cursor, found)
    return found


def start_worker(build_dir: Path) -> None:
    load_database(build_dir)


def main() -> None:
    build_dir = Path(sys.argv[1]).resolve()
    jobs = int(sys.argv[2])
    load_database(build_dir)
    paths = sorted(p for p in compile_args if Path(p).suffix in SOURCE_SUFFIXES and is_owned(p))
    found: set = set()
    with multiprocessing.Pool(jobs, start_worker, (build_dir,)) as pool:
        for result in pool.imap(collect, paths):
            if isinstance(result, str):
                sys.exit(f"libclang cannot parse {result}")
            found |= result
    # file -> list of (start, end, replacement), applied from the end of the file.
    edits: dict[str, list[tuple[int, int, str]]] = {}
    declarations = {}
    for file, start, end, declaration_file, declaration_end in sorted(found):
        text = Path(file).read_bytes()
        semicolon = text.index(b";", end)
        if not re.search(rb"=\s*default\s*$", text[start:semicolon]):
            sys.exit(f"{file}:{start}: definition does not end in = default")
        line_start = text.rfind(b"\n", 0, start) + 1
        if text[line_start:start].strip() == b"":
            start = line_start
        stop = semicolon + 1
        if text[stop:stop + 1] == b"\n":
            stop += 1
        edits.setdefault(file, []).append((start, stop, ""))
        key = (declaration_file, declaration_end)
        if key in declarations:
            sys.exit(f"{declaration_file}:{declaration_end}: declaration has two out-of-line definitions")
        declarations[key] = file
        after = Path(declaration_file).read_bytes()[declaration_end:]
        if not after.lstrip().startswith(b";"):
            sys.exit(f"{declaration_file}:{declaration_end}: declaration does not end before a semicolon")
        edits.setdefault(declaration_file, []).append((declaration_end, declaration_end, " = default"))
    for file, changes in sorted(edits.items()):
        text = Path(file).read_bytes()
        for start, stop, replacement in sorted(changes, reverse=True):
            text = text[:start] + replacement.encode() + text[stop:]
        Path(file).write_bytes(text)
    print(f"relocated {len(found)} definitions; changed {len(edits)} files")


if __name__ == "__main__":
    main()
