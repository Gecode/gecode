#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass

VERIFIER_PREFIX = "[verify-legacy-check-selection]"
TEST_TOKEN_RE = re.compile(r"(?:^|\s)-test\s+([^\s\\]+)")


@dataclass(frozen=True)
class ModeExpectation:
    required: tuple[str, ...]
    forbidden_prefixes: tuple[str, ...]


MODE_EXPECTATIONS = {
    "no-set-float-flatzinc": ModeExpectation(
        required=(
            "Branch::Int::Dense::3",
            "Int::Arithmetic::Abs",
            "Int::MiniModel::LinExpr::Bool::352",
            "NoGoods::Queens",
            "Search::DFS::Sol::Binary::Nary::Binary::1::1::1",
        ),
        forbidden_prefixes=("Set::", "Float::", "FlatZinc::"),
    ),
}


def fail(message: str) -> "NoReturn":
    sys.stderr.write(f"{VERIFIER_PREFIX} selection: FAIL - {message}\n")
    raise SystemExit(1)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=sorted(MODE_EXPECTATIONS), required=True)
    return parser.parse_args()


def extract_selected_tests(make_output: str) -> list[str]:
    return TEST_TOKEN_RE.findall(make_output.replace("\\\n", " "))


def main() -> int:
    args = parse_args()
    make_output = sys.stdin.read()
    selected_tests = extract_selected_tests(make_output)
    if not selected_tests:
        fail("did not find any '-test <name>' selections in make output")

    expectation = MODE_EXPECTATIONS[args.mode]
    missing_required = [name for name in expectation.required if name not in selected_tests]
    if missing_required:
        fail(f"missing required selections: {', '.join(missing_required)}")

    forbidden = [
        name
        for name in selected_tests
        if any(name.startswith(prefix) for prefix in expectation.forbidden_prefixes)
    ]
    if forbidden:
        fail(f"found forbidden selections: {', '.join(forbidden)}")

    sys.stdout.write(
        f"{VERIFIER_PREFIX} selection: ok - mode={args.mode} selected={len(selected_tests)}\n"
    )
    for name in selected_tests:
        sys.stdout.write(f"{VERIFIER_PREFIX} selection: {name}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
