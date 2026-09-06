#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path

VERIFIER_PREFIX = "[verify-legacy-install-surface]"
EXPECTED_HEADERS = ["int.hh", "int.hpp", "test.hh", "test.hpp"]
UNSUPPORTED_HEADERS = [
    "assign.hh",
    "branch.hh",
    "float.hh",
    "float.hpp",
    "flatzinc.hh",
    "set.hh",
    "set.hpp",
]
UNSUPPORTED_BINARIES = [
    "gecode-test",
    "public-int-smoke",
    "public-runner-smoke",
]
EXPECTED_STATIC_LIBS = {
    "gecodetest": "libgecodetest.a",
    "gecodetestint": "libgecodetestint.a",
}
def fail_phase(phase: str, message: str) -> "NoReturn":
    sys.stderr.write(f"{VERIFIER_PREFIX} {phase}: FAIL - {message}\n")
    raise SystemExit(1)



def assert_phase(condition: bool, phase: str, message: str) -> None:
    if not condition:
        fail_phase(phase, message)



def is_installed_library_path(prefix: Path, path: Path) -> bool:
    relative_path = path.relative_to(prefix)
    return any(part.startswith("lib") for part in relative_path.parts[:-1]) and "cmake" not in relative_path.parts



def verify_prefix_surface(prefix: Path) -> None:
    phase = "prefix-surface"
    include_test_dir = prefix / "include" / "test"
    assert_phase(include_test_dir.is_dir(), phase, f"missing installed test include dir: {include_test_dir}")

    installed_headers = sorted(path.name for path in include_test_dir.iterdir() if path.is_file())
    missing_headers = [name for name in EXPECTED_HEADERS if name not in installed_headers]
    unexpected_headers = [name for name in installed_headers if name not in EXPECTED_HEADERS]
    assert_phase(
        not missing_headers and not unexpected_headers,
        phase,
        f"missing expected headers: {missing_headers}; unexpected installed headers: {unexpected_headers}",
    )
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: headers={installed_headers}\n")



def verify_harness_libs(prefix: Path) -> None:
    phase = "harness-libs"
    expected_locations: list[str] = []

    for library_name, expected_filename in EXPECTED_STATIC_LIBS.items():
        matches = sorted(
            path.relative_to(prefix).as_posix()
            for path in prefix.rglob(expected_filename)
            if path.is_file() and is_installed_library_path(prefix, path)
        )
        assert_phase(matches, phase, f"missing installed {library_name} static library: {expected_filename}")
        assert_phase(
            len(matches) == 1,
            phase,
            f"expected exactly one installed {library_name} static library, found: {matches}",
        )
        expected_locations.extend(matches)

    unexpected_harness_artifacts = sorted(
        path.relative_to(prefix).as_posix()
        for path in prefix.rglob("*")
        if path.is_file()
        and is_installed_library_path(prefix, path)
        and path.name.startswith(("libgecodetest", "libgecodetestint"))
        and path.name not in EXPECTED_STATIC_LIBS.values()
    )
    assert_phase(
        not unexpected_harness_artifacts,
        phase,
        f"unexpected installed harness artifacts: {unexpected_harness_artifacts}",
    )
    sys.stdout.write(f"{VERIFIER_PREFIX} {phase}: artifacts={sorted(expected_locations)}\n")



def verify_unsupported_surface(prefix: Path) -> None:
    phase = "unsupported-surface"
    include_test_dir = prefix / "include" / "test"
    unexpected_headers = [name for name in UNSUPPORTED_HEADERS if (include_test_dir / name).exists()]
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

    verify_prefix_surface(prefix)
    verify_harness_libs(prefix)
    verify_unsupported_surface(prefix)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
