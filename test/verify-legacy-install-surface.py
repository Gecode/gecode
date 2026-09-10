#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

VERIFIER_PREFIX = "[verify-legacy-install-surface]"
CORE_HEADERS = ["int.hh", "int.hpp", "test.hh", "test.hpp"]
UNSUPPORTED_HEADERS = [
    "assign.hh",
    "branch.hh",
    "flatzinc.hh",
]
UNSUPPORTED_BINARIES = [
    "gecode-test",
    "public-int-smoke",
    "public-runner-smoke",
]
def fail_phase(phase: str, message: str) -> "NoReturn":
    sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: FAIL - {message}\n")
    raise SystemExit(1)



def assert_phase(condition: bool, phase: str, message: str) -> None:
    if not condition:
        fail_phase(phase, message)



def is_installed_library_path(prefix: Path, path: Path) -> bool:
    relative_path = path.relative_to(prefix)
    return any(part.startswith("lib") for part in relative_path.parts[:-1]) and "cmake" not in relative_path.parts



def configured_types(prefix: Path) -> set[str]:
    config = (prefix / "include/gecode/support/config.hpp").read_text()
    return {family for family in ("int", "set", "float")
            if re.search(r"^#define GECODE_HAS_" + family.upper() + r"_VARS\b",
                         config, re.MULTILINE)}


def verify_prefix_surface(prefix: Path, types: set[str]) -> None:
    phase = "prefix-surface"
    include_test_dir = prefix / "include" / "test"
    assert_phase(include_test_dir.is_dir(), phase, f"missing installed test include dir: {include_test_dir}")

    installed_headers = sorted(path.name for path in include_test_dir.iterdir() if path.is_file())
    expected_headers = list(CORE_HEADERS)
    for variable_type in ("set", "float"):
        if variable_type in types:
            expected_headers.extend([f"{variable_type}.hh", f"{variable_type}.hpp"])
    missing_headers = [name for name in expected_headers if name not in installed_headers]
    unexpected_headers = [name for name in installed_headers if name not in expected_headers]
    assert_phase(
        not missing_headers and not unexpected_headers,
        phase,
        f"missing expected headers: {missing_headers}; unexpected installed headers: {unexpected_headers}",
    )
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: headers={installed_headers}\n")



def verify_harness_libs(prefix: Path, types: set[str]) -> None:
    phase = "harness-libs"
    expected_locations: list[str] = []

    libraries = ["gecodetest"] + [f"gecodetest{family}" for family in sorted(types)]
    for library in libraries:
        matches = sorted(path.relative_to(prefix).as_posix()
                         for path in prefix.rglob(f"lib{library}.*")
                         if path.is_file() and is_installed_library_path(prefix, path))
        assert_phase(matches, phase, f"missing installed {library} library")
        expected_locations.extend(matches)
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: artifacts={expected_locations}\n")



def verify_unsupported_surface(prefix: Path, types: set[str]) -> None:
    phase = "unsupported-surface"
    include_test_dir = prefix / "include" / "test"
    unsupported_headers = list(UNSUPPORTED_HEADERS)
    for variable_type in ("set", "float"):
        if variable_type not in types:
            unsupported_headers.extend([f"{variable_type}.hh", f"{variable_type}.hpp"])
    unexpected_headers = [name for name in unsupported_headers if (include_test_dir / name).exists()]
    assert_phase(not unexpected_headers, phase, f"unexpected public helper headers installed: {unexpected_headers}")

    unexpected_binaries = sorted(
        path.relative_to(prefix).as_posix()
        for path in prefix.rglob("*")
        if path.is_file() and (path.name in UNSUPPORTED_BINARIES or path.stem in UNSUPPORTED_BINARIES)
    )
    assert_phase(not unexpected_binaries, phase, f"unexpected internal test binaries installed: {unexpected_binaries}")
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: headers-ok binaries-ok\n")



def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--prefix", required=True)
    return parser.parse_args()



def main() -> int:
    args = parse_args()
    prefix = Path(args.prefix).resolve()

    assert_phase(prefix.is_dir(), "inputs", f"missing installed prefix: {prefix}")

    types = configured_types(prefix)
    verify_prefix_surface(prefix, types)
    verify_harness_libs(prefix, types)
    verify_unsupported_surface(prefix, types)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
