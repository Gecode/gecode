#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Generate a minimal Xcode project listing the repository C/C++ sources."""

import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Tuple, Union

Tree = Dict[str, Union[str, List[object]]]


def uid() -> str:
    raw = subprocess.check_output(["uuidgen"], text=True).strip().upper()
    m = re.search(r"[A-Z0-9]+-([A-Z0-9]+)-([A-Z0-9]+)-([A-Z0-9]+)-([A-Z0-9]+)", raw)
    if not m:
        raise SystemExit("uuidgen failed")
    return "".join(m.groups())


def iter_files(root: Path):
    with os.scandir(root) as entries:
        for entry in entries:
            if entry.is_dir(follow_symlinks=False):
                yield from iter_files(Path(entry.path))
            elif entry.is_file(follow_symlinks=False):
                yield Path(entry.path)


def find_files(extension: str, ftype: str, files: Dict[str, Tuple[str, str, str]], source_tree: Tree, header_tree: Tree) -> None:
    for path_obj in iter_files(Path(".")):
        if path_obj.suffix != f".{extension}":
            continue
        rel_path = os.path.relpath(path_obj, ".").replace(os.sep, "/")
        f = f"./{rel_path}"
        filename = path_obj.name

        dirs = f.split("/")
        if dirs:
            dirs = dirs[1:]
        if dirs:
            dirs = dirs[:-1]

        tree_p: Tree
        if re.search(r"sourcecode\..*\.h", ftype):
            tree_p = header_tree
        else:
            tree_p = source_tree

        for d in dirs:
            if d not in tree_p:
                subtree: Tree = {}
                tree_p[d] = [uid(), subtree]
            tree_p = tree_p[d][1]  # type: ignore[index]

        tree_p[filename] = ""
        files[f] = (uid(), filename, ftype)


def print_group(g: str, gid: str, tree_p: Tree, path: str, files: Dict[str, Tuple[str, str, str]]) -> None:
    sys.stdout.write(f"    {gid} /* {g} */ = {{\n")
    sys.stdout.write("      isa = PBXGroup;\n")
    sys.stdout.write("      children = (\n")
    for k in sorted(tree_p.keys()):
        v = tree_p[k]
        if isinstance(v, list):
            sys.stdout.write(f"        {v[0]} /* {k} */,\n")
        else:
            sys.stdout.write(f"        {files[path + k][0]} /* {k} */,\n")
    sys.stdout.write("      );\n")
    if path == "./":
        sys.stdout.write('      path = ".";\n')
        sys.stdout.write(f'      name = "{g}";\n')
    else:
        sys.stdout.write(f'      path = "{g}";\n')
    sys.stdout.write('      sourceTree = "<group>";\n    };\n')
    for k, v in tree_p.items():
        if isinstance(v, list):
            print_group(k, v[0], v[1], path + k + "/", files)  # type: ignore[index]


def main() -> int:
    sys.stdout.write(
        """// !$*UTF8*$!
{
  archiveVersion = 1;
  classes = {};
  objectVersion = 44;
  objects = {

/* Begin PBXFileReference section */
"""
    )

    files: Dict[str, Tuple[str, str, str]] = {}
    source_tree: Tree = {}
    header_tree: Tree = {}

    find_files("h", "sourcecode.c.h", files, source_tree, header_tree)
    find_files("c", "sourcecode.c.c", files, source_tree, header_tree)
    find_files("hpp", "sourcecode.cpp.h", files, source_tree, header_tree)
    find_files("hh", "sourcecode.cpp.h", files, source_tree, header_tree)
    find_files("cpp", "sourcecode.cpp.cpp", files, source_tree, header_tree)

    for k in files.keys():
        sys.stdout.write(f"    {files[k][0]} /* {files[k][1]} */ = ")
        sys.stdout.write("{isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = ")
        sys.stdout.write(f"{files[k][2]}; path =\"{files[k][1]}")
        sys.stdout.write("\"; sourceTree = \"<group>\"; };\n")

    sys.stdout.write("/* End PBXFileReference section */\n\n")
    sys.stdout.write("/* Begin PBXGroup section */\n")

    sources_uid = uid()
    print_group("Sources", sources_uid, source_tree, "./", files)

    headers_uid = uid()
    print_group("Headers", headers_uid, header_tree, "./", files)

    main_uid = uid()
    sys.stdout.write(
        f"""    {main_uid} = {{
      isa = PBXGroup;
      children = (
        {sources_uid} /* Sources */,
        {headers_uid} /* Headers */,
      );
      sourceTree = "<group>";
    }};
"""
    )

    root_uid = uid()

    sys.stdout.write("/* End PBXGroup section */\n")
    sys.stdout.write("/* Begin PBXProject section */\n")
    sys.stdout.write(f"    {root_uid} /* Project object */ = {{\n")
    sys.stdout.write("      isa = PBXProject;\n")
    sys.stdout.write('      compatibilityVersion = "XCode 2.4";\n')
    sys.stdout.write(f"      mainGroup = {main_uid} /* Gecode */;\n")
    sys.stdout.write('      projectDirPath = "";\n')
    sys.stdout.write('      projectRoot = "";\n')
    sys.stdout.write("    };\n")
    sys.stdout.write("/* End PBXProject section */\n")
    sys.stdout.write("  };\n")
    sys.stdout.write(f"  rootObject = {root_uid} /* Project object */;\n")
    sys.stdout.write("}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
