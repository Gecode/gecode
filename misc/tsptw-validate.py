#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.10"
# dependencies = []
# ///
"""Focused offline TSPTW data checks and optional native solver validation."""

import argparse
import hashlib
import json
import runpy
import subprocess
import sys
import tempfile
from collections.abc import Callable, Sequence
from fractions import Fraction
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "examples/data/tsptw"
CHECKER = runpy.run_path(str(Path(__file__).with_name("tsptw-check.py")))


def reject(action: Callable[[], object], expected: str) -> None:
    try:
        action()
    except ValueError as error:
        assert expected in str(error), str(error)
    else:
        raise AssertionError(f"accepted invalid input: {expected}")


def validate_native(reader: Path, path: Path) -> None:
    matrix, windows, scale = CHECKER["read_instance"](path)
    result = subprocess.run(
        [str(reader), str(path)], capture_output=True, text=True, check=True
    )
    integers = [int(x) for x in result.stdout.split()]
    n = len(matrix)
    cost_limit = n * max(x for row in matrix for x in row)
    time_limit = cost_limit + max(due for _, due in windows)
    expected = [n, scale, int(cost_limit * scale), int(time_limit * scale)]
    expected += [int(x * scale) for row in matrix for x in row]
    expected += [int(x * scale) for window in windows for x in window]
    assert integers == expected, str(path)


def run_solver(
    solver: Path,
    path: Path,
    engine: str,
    options: Sequence[str] = (),
    seed: Path | None = None,
    formulation: str = "closed",
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    data = CHECKER["read_instance"](path)
    command = [
        str(solver),
        str(path),
        "--engine",
        engine,
        "--formulation",
        formulation,
        *options,
    ]
    seed_result = None
    if seed:
        tour = [int(x) for x in seed.read_text().split()]
        # Check the seed with the Python evaluator before allowing the native
        # engine to use its cost to prune search.
        seed_result = CHECKER["check_tour"](data, tour)
        command += ["--seed", str(seed)]
    output = subprocess.run(
        command, capture_output=True, text=True, check=True, timeout=30
    )
    records = [json.loads(line) for line in output.stdout.splitlines()]
    assert records and records[-1]["type"] == "summary", output.stdout
    summary = records[-1]
    assert summary["engine"] == engine
    assert summary["formulation"] == formulation
    bound = options[options.index("--bound") + 1] if "--bound" in options else "prefix"
    propagation = (
        options[options.index("--residual-propagation") + 1]
        if "--residual-propagation" in options
        else "off"
    )
    assert summary["bound"] == ("cp-total" if formulation == "closed" else bound)
    assert summary["residual_propagation"] == propagation
    assert summary["termination"] in {"optimal", "infeasible", "stopped"}
    assert all(isinstance(x, int) and x >= 0 for x in summary["statistics"].values())
    last = None
    for record in records[:-1]:
        assert record["type"] == "solution", record
        assert all(type(node) is int for node in record["tour"]), record
        checked = CHECKER["check_tour"](data, record["tour"], record["cost"])
        for field in ("scaled_cost", "scale", "cost", "return_time"):
            assert record[field] == checked[field], (path, engine, field, record)
        assert last is None or checked["scaled_cost"] < last["scaled_cost"], records
        last = {
            field: record[field]
            for field in ("tour", "scaled_cost", "scale", "cost", "return_time")
        }
    assert summary["incumbent"] == last, records
    if seed_result:
        assert records[0]["tour"] == tour
        assert records[0]["scaled_cost"] == seed_result["scaled_cost"]
    if summary["termination"] == "optimal":
        assert last is not None, records
    if summary["termination"] == "infeasible":
        assert last is None, records
    return summary, records[:-1]


def validate_strengthening(
    solver: Path, path: Path, configurations: Sequence[tuple[str, str]]
) -> None:
    optimum = CHECKER["enumerate_tours"](CHECKER["read_instance"](path))["optimum"]
    for bound, propagation in configurations:
        for engine in ("bab", "astar", "cabs"):
            summary, _ = run_solver(
                solver,
                path,
                engine,
                ("--bound", bound, "--residual-propagation", propagation),
                formulation="transition",
            )
            assert summary["termination"] == ("optimal" if optimum else "infeasible")
            if optimum:
                assert summary["incumbent"]["scaled_cost"] == optimum["scaled_cost"]


def validate_solver(solver: Path, manifest: dict[str, Any]) -> None:
    engines = ("bab", "astar", "cabs")
    formulations = ("closed", "transition")
    for case in manifest["tiny"]:
        path = DATA / case["path"]
        optimum = CHECKER["enumerate_tours"](CHECKER["read_instance"](path))["optimum"]
        validate_strengthening(
            solver, path, (("min-arcs", "off"), ("min-arcs", "unary"))
        )
        for engine in engines:
            for formulation in formulations:
                summary, _ = run_solver(
                    solver, path, engine, formulation=formulation
                )
                assert summary["termination"] == (
                    "optimal" if optimum else "infeasible"
                )
                if optimum:
                    assert summary["incumbent"]["scaled_cost"] == optimum["scaled_cost"]
    with tempfile.TemporaryDirectory(prefix="tsptw-solver-") as directory:
        directory = Path(directory)
        nonmetric = directory / "nonmetric.txt"
        nonmetric.write_text(
            (DATA / "tiny/waiting.txt").read_text().replace("0 12", "0 6")
        )
        assert (
            CHECKER["enumerate_tours"](CHECKER["read_instance"](nonmetric))["optimum"][
                "cost"
            ]
            == "5"
        )
        validate_strengthening(solver, nonmetric, (("min-arcs", "unary"),))
        validate_strengthening(
            solver, DATA / "tiny/waiting.txt", (("prefix", "unary"),)
        )
        seed = directory / "seed.tour"
        folded = DATA / "tiny/folded-service.txt"
        for formulation in formulations:
            for engine in engines:
                summary, _ = run_solver(
                    solver, nonmetric, engine, formulation=formulation
                )
                assert (
                    summary["termination"] == "optimal"
                    and summary["incumbent"]["cost"] == "5"
                )
                seed.write_text("2 1\n")
                summary, solutions = run_solver(
                    solver, folded, engine, seed=seed, formulation=formulation
                )
                assert (
                    summary["termination"] == "optimal"
                    and summary["incumbent"]["cost"] == "9"
                )
                assert solutions[0]["cost"] == "13" and len(solutions) >= 2
                seed.write_text("1 2\n")
                summary, solutions = run_solver(
                    solver, folded, engine, seed=seed, formulation=formulation
                )
                assert summary["termination"] == "optimal" and len(solutions) == 1
                for instance, tour in [
                    (folded, "1 1\n"),
                    (DATA / "tiny/waiting.txt", "2 1\n"),
                    (folded, "1 2 999999999999999999999\n"),
                ]:
                    seed.write_text(tour)
                    result = subprocess.run(
                        [
                            str(solver),
                            str(instance),
                            "--engine",
                            engine,
                            "--formulation",
                            formulation,
                            "--seed",
                            str(seed),
                        ],
                        capture_output=True,
                        text=True,
                        check=False,
                    )
                    assert result.returncode == 1 and not result.stdout, result
        # Prefix-cost guidance ignores the return arc. The first complete tour
        # can therefore cost more than a later tour, even with A* search.
        improving = directory / "improving.txt"
        improving.write_text(
            "4\n0 1 2 3\n1 0 1 4\n1 3 0 1\n20 1 3 0\n0 100\n5 100\n0 100\n0 100\n"
        )
        optimum = CHECKER["enumerate_tours"](CHECKER["read_instance"](improving))[
            "optimum"
        ]
        for engine in engines:
            summary, solutions = run_solver(
                solver, improving, engine, formulation="transition"
            )
            assert summary["termination"] == "optimal"
            assert summary["incumbent"]["scaled_cost"] == optimum["scaled_cost"]
            assert len(solutions) >= 2
        # Every arc between distinct nodes costs one, so assigning arc costs
        # leaves route choices. Exercise successor branching and resource stops.
        branching = directory / "branching.txt"
        branching.write_text(
            "5\n"
            + "\n".join(
                " ".join("0" if i == j else "1" for j in range(5)) for i in range(5)
            )
            + "\n"
            + "0 100\n" * 5
        )
        branching_optimum = CHECKER["enumerate_tours"](
            CHECKER["read_instance"](branching)
        )["optimum"]["scaled_cost"]
        for formulation in formulations:
            for engine in engines:
                summary, _ = run_solver(
                    solver,
                    branching,
                    engine,
                    ("--node-limit", "0"),
                    formulation=formulation,
                )
                assert summary["termination"] == "stopped"
                assert summary["statistics"]["node"] > 0
            for limit in ("--max-width", "--frontier-limit"):
                summary, _ = run_solver(
                    solver, branching, "cabs", (limit, "1"), formulation=formulation
                )
                # A CABS cap stops a run only if pending work or a wider beam
                # would exceed it. Equal tour costs can let objective bounds
                # prove optimality before either cap stops search.
                assert summary["termination"] in ("stopped", "optimal"), summary
                if summary["termination"] == "optimal":
                    assert summary["incumbent"]["scaled_cost"] == branching_optimum
    print(
        f"passed closed/transition BAB/A*/CABS: tiny optima/infeasibility, "
        f"nonmetric filtering, "
        f"minimum-arc bounds and residual unary propagation, "
        f"checked seeds and stops"
    )


def validate(reader: Path, solver: Path | None) -> None:
    manifest = json.loads((DATA / "manifest.json").read_text())
    checked = 0
    for case in [*manifest["tiny"], *manifest["published"]]:
        path = DATA / case["path"]
        assert hashlib.sha256(path.read_bytes()).hexdigest() == case["sha256"]
        validate_native(reader, path)
        data = CHECKER["read_instance"](path)
        if "tour_path" in case:
            tour_path = DATA / case["tour_path"]
            assert (
                hashlib.sha256(tour_path.read_bytes()).hexdigest()
                == case["tour_sha256"]
            )
            tour = [int(x) for x in tour_path.read_text().split()]
            result = CHECKER["check_tour"](data, tour, case["exact_cost"])
            displayed = case["published_cost"]
            places = len(displayed.split(".")[1]) if "." in displayed else 0
            assert abs(Fraction(result["cost"]) - Fraction(displayed)) <= Fraction(
                1, 2 * 10**places
            )
        else:
            result = CHECKER["enumerate_tours"](data)
            assert result["feasible_tours"] == case["feasible_tours"]
            optimum = result["optimum"]
            assert (None if optimum is None else optimum["cost"]) == case["optimum"]
        checked += 1
    waiting = CHECKER["read_instance"](DATA / "tiny/waiting.txt")
    result = CHECKER["check_tour"](waiting, [1, 2], "5")
    assert result["visits"][0]["waiting"] == "3" and result["return_time"] == "8"
    reject(lambda: CHECKER["check_tour"](waiting, [1, 1]), "duplicate")
    reject(lambda: CHECKER["check_tour"](waiting, [1]), "omits")
    reject(lambda: CHECKER["check_tour"](waiting, [1, 3]), "invalid")
    reject(lambda: CHECKER["check_tour"](waiting, [1, 2], "8"), "incorrect cost")
    for case, message in [
        ("late-customer", "late customer"),
        ("late-return", "late return"),
    ]:
        data = CHECKER["read_instance"](DATA / f"tiny/{case}.txt")
        reject(lambda data=data: CHECKER["check_tour"](data, [1, 2]), message)
    with tempfile.TemporaryDirectory(prefix="tsptw-validate-") as directory:
        path = Path(directory) / "input.txt"
        good = "2\n0 1\n1 0\n0 10\n0 10\n"
        bad_inputs = [
            good.replace("2\n", "3\n", 1),
            good + "1\n",
            good.replace("0 1", "0 -1", 1),
            good.replace("0 1", "0 1.0.00", 1),
            good.replace("0 1", "0 1e1", 1),
            good.replace("0 1", "0 .", 1),
            good.replace("0 10", "11 10", 1),
            good.replace("0 10", "1 10", 1),
            good.replace("0 1", "0 2147483647", 1),
            good.replace("0 1", "0 1073741824", 1),
            good.replace("0 1", "0 0.0000000001", 1),
            good.replace("0 10", "0 2147483646", 1),
        ]
        for content in bad_inputs:
            path.write_text(content)
            try:
                CHECKER["read_instance"](path)
            except ValueError:
                pass
            else:
                raise AssertionError(f"Python accepted invalid data: {content}")
            assert (
                subprocess.run(
                    [str(reader), str(path)], capture_output=True, check=False
                ).returncode
                == 1
            )
        path.write_text(good.replace("0 1", "0 1.000", 1) + "# optional comment\n")
        validate_native(reader, path)
        for zero in (".0", ".00"):
            path.write_text(good.replace("0 1", f"{zero} 1", 1))
            validate_native(reader, path)
        tour_path = Path(directory) / "tour.txt"
        tour_path.write_text("1 2\n")
        command = [
            sys.executable,
            str(Path(__file__).with_name("tsptw-check.py")),
            "check",
            str(DATA / "tiny/waiting.txt"),
            str(tour_path),
            "--cost",
            "5",
        ]
        assert subprocess.run(command, capture_output=True, check=False).returncode == 0
        tour_path.write_text("1 1\n")
        assert subprocess.run(command, capture_output=True, check=False).returncode == 1
    print(
        f"passed: {checked} bundled instances, five tiny enumerations, four published tours, "
        "reader rejection cases, checker failures and CLI smoke"
    )
    if solver:
        validate_solver(solver, manifest)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--reader", type=Path, required=True, help="compiled misc/tsptw-read.cpp"
    )
    parser.add_argument(
        "--solver", type=Path, help="also validate the native tsptw executable"
    )
    args = parser.parse_args()
    validate(args.reader.resolve(), args.solver.resolve() if args.solver else None)
