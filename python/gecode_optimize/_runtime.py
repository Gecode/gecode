"""Locate an optional bundled binary without searching the working directory."""
from pathlib import Path
import sys


def bundled_library_path():
    directory = Path(__file__).resolve().parent / "_native"
    if not directory.exists():
        return None
    names = {"darwin": "libgecodeoptimize_c.dylib", "win32": "gecodeoptimize_c.dll",
             "linux": "libgecodeoptimize_c.so"}
    if sys.platform not in names:
        raise RuntimeError("the installed native bundle does not support this platform")
    candidate = directory / names[sys.platform]
    if not directory.is_dir() or not candidate.is_file():
        raise RuntimeError("the installed gecode_optimize native bundle is incomplete")
    resolved = candidate.resolve()
    if resolved.parent != directory.resolve():
        raise RuntimeError("bundled solver library resolves outside its native directory")
    return str(resolved)
