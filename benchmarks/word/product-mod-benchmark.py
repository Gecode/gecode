#!/usr/bin/env -S uv run --script
"""Run semantically checked Word product-modulo controls."""
from __future__ import annotations

import argparse
import json
import statistics
import subprocess
import time
from pathlib import Path

from semantics import product_mod, validate_contract


def unique_case(case_id: str, width: int, domain: str, x: int, y: int,
                modulus: int) -> dict:
    mask = (1 << width)-1
    result = product_mod(x & mask, y & mask, modulus)
    return {
        "schema_version": 1, "id": case_id, "kind": "product-mod",
        "goal": "unique", "expected_status": "sat",
        "decision_variables": ["x", "y", "result"],
        "parameters": {"width": width, "domain": domain,
                       "x_min": x, "x_max": x, "y_min": y, "y_max": y,
                       "modulus": modulus,
                       "result_min": result, "result_max": result},
    }


CASES = (
    {"schema_version": 1, "id": "reduce-small", "kind": "product-mod",
     "goal": "enumerate", "expected_status": "sat",
     "decision_variables": ["x", "y", "result"],
     "parameters": {"width": 9, "domain": "unsigned",
                    "x_min": 10, "x_max": 30, "y_min": 10, "y_max": 30,
                    "modulus": 17, "result_min": 0, "result_max": 16}},
    unique_case("unsigned-32-nonwrap", 32, "unsigned", 70_000, 3, 65_537),
    unique_case("unsigned-33-wrap", 33, "unsigned", (1 << 32)-1, 3, 1_000_003),
    unique_case("unsigned-63-nonwrap", 63, "unsigned",
                (1 << 31)+7, (1 << 30)+9, 1_000_003),
    unique_case("unsigned-64-wrap", 64, "unsigned", (1 << 63)+1, 2, 1_000_003),
    unique_case("signed-32", 32, "signed", -(1 << 31), -3, 65_537),
    unique_case("signed-33", 33, "signed", -(1 << 32), 2, 1_000_003),
    unique_case("signed-63", 63, "signed", -(1 << 62), 5, 1_000_003),
    unique_case("signed-64", 64, "signed", -(1 << 63), -1, 1_000_003),
    {"schema_version": 1, "id": "wrong-result-unsat", "kind": "product-mod",
     "goal": "unsat", "expected_status": "unsat",
     "decision_variables": ["x", "y", "result"],
     "parameters": {"width": 8, "domain": "unsigned",
                    "x_min": 10, "x_max": 10, "y_min": 20, "y_max": 20,
                    "modulus": 17, "result_min": 12, "result_max": 12}},
)
COUNTERS = ("status", "semantic_status", "solutions", "checksum", "nodes",
            "failures", "propagations", "decision_variables", "projections")


def command(binary: Path, case: dict, variant: str) -> list[str]:
    p = case["parameters"]
    result = [str(binary), "--variant", variant, "--case-id", case["id"]]
    for field in ("width", "domain", "x_min", "x_max", "y_min", "y_max",
                  "modulus", "result_min", "result_max"):
        result.extend(["--" + field.replace("_", "-"), str(p[field])])
    return result


def captured(value: bytes | str | None) -> str:
    return value.decode(errors="replace") if isinstance(value, bytes) else value or ""


def run(binary: Path, case: dict, variant: str, timeout: float) -> dict:
    invocation = command(binary, case, variant)
    started = time.perf_counter()
    try:
        process = subprocess.run(invocation, text=True, capture_output=True,
                                 timeout=timeout, check=False)
    except subprocess.TimeoutExpired as error:
        return {"case_id": case["id"], "variant": variant, "status": "timeout",
                "seconds": timeout, "command": invocation, "returncode": None,
                "stdout": captured(error.stdout), "stderr": captured(error.stderr),
                "error": str(error)}
    except OSError as error:
        return {"case_id": case["id"], "variant": variant, "status": "error",
                "seconds": time.perf_counter()-started, "command": invocation,
                "returncode": None, "stdout": "", "stderr": "", "error": str(error)}
    seconds = time.perf_counter()-started
    try:
        value = json.loads(process.stdout)
        if process.returncode:
            raise ValueError(f"exit status {process.returncode}")
        if not isinstance(value, dict) or value.get("status") != "ok" or \
                value.get("case_id") != case["id"] or value.get("variant") != variant:
            raise ValueError("solver identity/status mismatch")
        validate_contract(case, value)
    except (json.JSONDecodeError, ValueError) as error:
        return {"case_id": case["id"], "variant": variant, "status": "error",
                "seconds": seconds, "command": invocation,
                "returncode": process.returncode, "stdout": process.stdout,
                "stderr": process.stderr, "error": str(error)}
    value.update(seconds=seconds, command=invocation, returncode=process.returncode,
                 stdout=process.stdout, stderr=process.stderr, error=None)
    return value


def validate_case_coverage() -> None:
    widths = {(case["parameters"]["width"], case["parameters"]["domain"])
              for case in CASES}
    required = {(width, domain) for width in (32, 33, 63, 64)
                for domain in ("unsigned", "signed")}
    if not required <= widths:
        raise ValueError("product_mod controls lack width/sign boundaries")
    products = []
    for case in CASES:
        p = case["parameters"]
        mask = (1 << p["width"])-1
        products.append(((p["x_min"] & mask)*(p["y_min"] & mask),
                         p["width"], p["modulus"]))
    if len({product // modulus for product, _, modulus in products}) < 3 or \
            not any(product >= 1 << width for product, width, _ in products) or \
            not any(product < 1 << width for product, width, _ in products):
        raise ValueError("product_mod controls lack quotient and wrap coverage")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--repetitions", type=int, default=20)
    parser.add_argument("--timeout", type=float, default=60.0)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    validate_case_coverage()
    rows = []
    cases = [(case, variant) for case in CASES for variant in ("bounded", "compact")]
    for repetition in range(args.repetitions):
        order = cases[repetition % len(cases):] + cases[:repetition % len(cases)]
        if repetition % 2:
            order = list(reversed(order))
        for case, variant in order:
            row = run(args.binary, case, variant, args.timeout)
            row["repetition"] = repetition+1
            rows.append(row)
    summary = {}
    issues = []
    for case in CASES:
        summary[case["id"]] = {}
        for variant in ("bounded", "compact"):
            selected = [row for row in rows if row["case_id"] == case["id"]
                        and row["variant"] == variant]
            successful = [row for row in selected if row["status"] == "ok"]
            statuses = {row["status"] for row in selected}
            if statuses != {"ok"}:
                issues.append(f"{case['id']}/{variant}: statuses={sorted(statuses)}")
            if successful:
                first = successful[0]
                if any(any((field in row) != (field in first) or
                           row.get(field) != first.get(field) for field in COUNTERS)
                       for row in successful[1:]):
                    issues.append(f"{case['id']}/{variant}: unstable status/counters")
            seconds = [row["seconds"] for row in successful]
            summary[case["id"]][variant] = {
                "statuses": {status: sum(row["status"] == status for row in selected)
                             for status in ("ok", "timeout", "error")},
                "median_seconds": statistics.median(seconds) if seconds else None,
                "min_seconds": min(seconds) if seconds else None,
                "max_seconds": max(seconds) if seconds else None,
                "counters": {field: successful[0].get(field) for field in COUNTERS}
                            if successful else None,
            }
        left, right = (summary[case["id"]][variant]["counters"]
                       for variant in ("bounded", "compact"))
        if left and right and (left["semantic_status"], left["solutions"],
                               left["projections"]) != \
                (right["semantic_status"], right["solutions"], right["projections"]):
            issues.append(f"{case['id']}: formulation projections differ")
    artifact = {"schema_version": 1, "repetitions": args.repetitions,
                "cases": CASES, "order": "rotated and reversed interleaving",
                "summary": summary, "runs": rows, "validation": {"issues": issues}}
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(artifact, indent=2, sort_keys=True)+"\n")
    concise = {
        case_id: {variant: {
            "statuses": row["statuses"],
            "median_seconds": row["median_seconds"],
            "semantic_status": row["counters"]["semantic_status"]
                if row["counters"] else None,
            "solutions": row["counters"]["solutions"]
                if row["counters"] else None,
        } for variant, row in variants.items()}
        for case_id, variants in summary.items()
    }
    print(json.dumps({"repetitions": args.repetitions, "summary": concise,
                      "validation": {"issues": issues}}, indent=2, sort_keys=True))
    return 1 if issues else 0


if __name__ == "__main__":
    raise SystemExit(main())
