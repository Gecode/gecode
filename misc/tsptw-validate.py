#!/usr/bin/env python3
"""Focused offline TSPTW data checks and optional native solver validation."""
import argparse
from collections import Counter
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import runpy
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "examples/data/tsptw"
CHECKER = runpy.run_path(str(Path(__file__).with_name("tsptw-check.py")))


def reject(action, expected):
    try:
        action()
    except ValueError as error:
        assert expected in str(error), str(error)
    else:
        raise AssertionError(f"accepted invalid input: {expected}")


def validate_native(reader, path):
    matrix, windows, scale = CHECKER["read_instance"](path)
    result = subprocess.run([str(reader), str(path)], capture_output=True, text=True, check=True)
    integers = [int(x) for x in result.stdout.split()]
    n = len(matrix)
    cost_limit = n*max(x for row in matrix for x in row)
    time_limit = cost_limit+max(due for _, due in windows)
    expected = [n, scale, int(cost_limit*scale), int(time_limit*scale)]
    expected += [int(x*scale) for row in matrix for x in row]
    expected += [int(x*scale) for window in windows for x in window]
    assert integers == expected, str(path)


def run_solver(solver, path, engine, options=(), seed=None, formulation="closed"):
    data = CHECKER["read_instance"](path)
    command = [str(solver), str(path), "--engine", engine,
               "--formulation", formulation, *options]
    seed_result = None
    if seed:
        tour = [int(x) for x in seed.read_text().split()]
        # Approve the seed independently before the native engine can cut.
        seed_result = CHECKER["check_tour"](data, tour)
        command += ["--seed", str(seed)]
    output = subprocess.run(command, capture_output=True, text=True, check=True, timeout=30)
    records = [json.loads(line) for line in output.stdout.splitlines()]
    assert records and records[-1]["type"] == "summary", output.stdout
    summary = records[-1]
    assert summary["engine"] == engine
    assert summary["formulation"] == formulation
    pruning = options[options.index("--state-pruning")+1] if "--state-pruning" in options else "off"
    assert summary["state_pruning"] == pruning
    bound = options[options.index("--bound")+1] if "--bound" in options else "prefix"
    propagation = options[options.index("--residual-propagation")+1] if "--residual-propagation" in options else "off"
    assert summary["bound"] == ("cp-total" if formulation == "closed" else bound)
    assert summary["residual_propagation"] == propagation
    if pruning == "off":
        assert all(summary["statistics"][field] == 0 for field in
                   ("state_pruned", "state_descriptors", "state_bytes"))
    if formulation != "closed":
        allocation = summary["allocation"]
        n = len(data[0])
        assert allocation["space_bytes"] > 0 and allocation["intvar_handle_bytes"] > 0
        assert allocation["root_positions"] == (0 if formulation == "transition" else n)
        assert allocation["prepared_root_positions"] in ({0, 1} if formulation == "transition" else {n})
        assert allocation["solution_positions"] == (n if summary["incumbent"] else 0)
        measurements = summary["measurements"]
        assert all(isinstance(x, (int, float)) and x >= 0 for x in measurements.values())
        assert measurements["propagation_ms"] <= measurements["inference_ms"]
        if propagation == "off":
            assert measurements["inference_calls"] == 0
            assert measurements["inference_ms"] == measurements["propagation_ms"] == 0
    assert summary["termination"] in {"optimal", "infeasible", "stopped"}
    assert all(isinstance(x, int) and x >= 0 for x in summary["statistics"].values())
    last = None
    elapsed = 0
    for record in records[:-1]:
        assert record["type"] == "solution", record
        assert all(type(node) is int for node in record["tour"]), record
        checked = CHECKER["check_tour"](data, record["tour"], record["cost"])
        for field in ("scaled_cost", "scale", "cost", "return_time"):
            assert record[field] == checked[field], (path, engine, field, record)
        assert last is None or checked["scaled_cost"] < last["scaled_cost"], records
        assert record["elapsed_ms"] >= elapsed
        elapsed = record["elapsed_ms"]
        last = {field: record[field] for field in
                ("tour", "scaled_cost", "scale", "cost", "return_time")}
    assert summary["elapsed_ms"] >= elapsed
    assert summary["incumbent"] == last, records
    if seed_result:
        assert records[0]["tour"] == tour
        assert records[0]["scaled_cost"] == seed_result["scaled_cost"]
    if summary["termination"] == "optimal":
        assert last is not None, records
    if summary["termination"] == "infeasible":
        assert last is None, records
    return summary, records[:-1]


def validate_pruning(solver, path, require_pruning=False, options=(),
                     formulations=("transition", "eager")):
    optimum = CHECKER["enumerate_tours"](CHECKER["read_instance"](path))["optimum"]
    for engine in ("astar", "cabs"):
        lazy_tours = None
        for formulation in formulations:
            for pruning in ("off", "pareto"):
                summary, solutions = run_solver(solver, path, engine,
                    (*options, "--state-pruning", pruning), formulation=formulation)
                assert summary["termination"] == ("optimal" if optimum else "infeasible")
                if optimum:
                    assert summary["incumbent"]["scaled_cost"] == optimum["scaled_cost"]
                if require_pruning and pruning == "pareto":
                    assert summary["statistics"]["state_pruned"] > 0, summary
                    assert summary["statistics"]["state_descriptors"] > 0, summary
                    assert summary["statistics"]["state_bytes"] > 0, summary
                if pruning == "pareto":
                    tours = [record["tour"] for record in solutions]
                    if formulation == "transition":
                        lazy_tours = tours
                    else:
                        assert tours == lazy_tours, (path, engine, lazy_tours, tours)


def validate_strengthening(solver, path, configurations):
    optimum = CHECKER["enumerate_tours"](CHECKER["read_instance"](path))["optimum"]
    for bound, propagation in configurations:
        for engine in ("bab", "astar", "cabs"):
            summary, _ = run_solver(solver, path, engine,
                ("--bound", bound, "--residual-propagation", propagation),
                formulation="transition")
            assert summary["termination"] == ("optimal" if optimum else "infeasible")
            if optimum:
                assert summary["incumbent"]["scaled_cost"] == optimum["scaled_cost"]


def validate_column(solver, path, options=(), seed=None, formulations=("closed", "transition")):
    optimum = CHECKER["enumerate_tours"](CHECKER["read_instance"](path))["optimum"]
    for growth in ("0", "1"):
        for formulation in formulations:
            summary, _ = run_solver(solver, path, "column",
                (*options, "--initial-quota", "1", "--quota-growth", growth),
                seed=seed, formulation=formulation)
            assert summary["initial_quota"] == 1 and summary["quota_growth"] == int(growth)
            assert summary["termination"] == ("optimal" if optimum else "infeasible")
            if optimum:
                assert summary["incumbent"]["scaled_cost"] == optimum["scaled_cost"]


def validate_solver(solver, manifest):
    engines = ("bab", "astar", "cabs")
    formulations = ("closed", "transition", "eager")
    for case in manifest["tiny"]:
        path = DATA/case["path"]
        optimum = CHECKER["enumerate_tours"](CHECKER["read_instance"](path))["optimum"]
        validate_pruning(solver, path)
        validate_column(solver, path)
        validate_strengthening(solver, path, (("min-arcs", "off"), ("min-arcs", "unary")))
        for engine in engines:
            lazy_tours = None
            for formulation in formulations:
                summary, solutions = run_solver(solver, path, engine, formulation=formulation)
                assert summary["termination"] == ("optimal" if optimum else "infeasible")
                if optimum:
                    assert summary["incumbent"]["scaled_cost"] == optimum["scaled_cost"]
                tours = [record["tour"] for record in solutions]
                if formulation == "transition":
                    lazy_tours = tours
                elif formulation == "eager":
                    assert tours == lazy_tours, (case, engine, lazy_tours, tours)
    with tempfile.TemporaryDirectory(prefix="tsptw-solver-") as directory:
        directory = Path(directory)
        nonmetric = directory/"nonmetric.txt"
        nonmetric.write_text((DATA/"tiny/waiting.txt").read_text().replace("0 12", "0 6"))
        assert CHECKER["enumerate_tours"](CHECKER["read_instance"](nonmetric))["optimum"]["cost"] == "5"
        validate_strengthening(solver, nonmetric, (("min-arcs", "unary"),))
        validate_strengthening(solver, DATA/"tiny/waiting.txt", (("prefix", "unary"),))
        seed = directory/"seed.tour"
        folded = DATA/"tiny/folded-service.txt"
        for formulation in formulations:
            for engine in engines:
                summary, _ = run_solver(solver, nonmetric, engine, formulation=formulation)
                assert summary["termination"] == "optimal" and summary["incumbent"]["cost"] == "5"
                seed.write_text("2 1\n")
                summary, solutions = run_solver(solver, folded, engine, seed=seed, formulation=formulation)
                assert summary["termination"] == "optimal" and summary["incumbent"]["cost"] == "9"
                assert solutions[0]["cost"] == "13" and len(solutions) >= 2
                seed.write_text("1 2\n")
                summary, solutions = run_solver(solver, folded, engine, seed=seed, formulation=formulation)
                assert summary["termination"] == "optimal" and len(solutions) == 1
                for instance, tour in [(folded, "1 1\n"), (DATA/"tiny/waiting.txt", "2 1\n"),
                                       (folded, "1 2 999999999999999999999\n")]:
                    seed.write_text(tour)
                    result = subprocess.run([str(solver), str(instance), "--engine", engine,
                                             "--formulation", formulation, "--seed", str(seed)],
                                            capture_output=True, text=True)
                    assert result.returncode == 1 and not result.stdout, result
        # A cheap prefix can publish an expensive return before better tours.
        improving = directory/"improving.txt"
        improving.write_text("4\n0 1 2 3\n1 0 1 4\n1 3 0 1\n20 1 3 0\n"
                             "0 100\n5 100\n0 100\n0 100\n")
        optimum = CHECKER["enumerate_tours"](CHECKER["read_instance"](improving))["optimum"]
        for engine in engines:
            lazy_tours = None
            for formulation in ("transition", "eager"):
                summary, solutions = run_solver(solver, improving, engine, formulation=formulation)
                assert summary["termination"] == "optimal"
                assert summary["incumbent"]["scaled_cost"] == optimum["scaled_cost"]
                assert len(solutions) >= 2
                tours = [record["tour"] for record in solutions]
                if lazy_tours is None:
                    lazy_tours = tours
                else:
                    assert tours == lazy_tours, (engine, lazy_tours, tours)
        validate_pruning(solver, improving)
        validate_column(solver, improving, formulations=("transition",))
        seed.write_text("2 1\n")
        validate_column(solver, folded, seed=seed)
        # Many paths reach equal residual states before any complete tour.
        duplicates = directory/"duplicates.txt"
        duplicates.write_text("6\n" + "\n".join(
            " ".join("0" if i == j else "1" for j in range(6)) for i in range(6)) +
            "\n" + "0 100\n"*6)
        validate_pruning(solver, duplicates, require_pruning=True)
        # Waiting makes arrival time differ from prefix cost. Different visit
        # orders give incomparable labels; enumerate every tour independently.
        pareto = directory/"pareto.txt"
        pareto.write_text("5\n0 3 1 7 4\n4 0 5 1 3\n2 1 0 4 6\n"
                          "3 4 2 0 1\n1 2 3 5 0\n"
                          "0 100\n15 100\n0 100\n0 100\n5 100\n")
        validate_pruning(solver, pareto)
        validate_column(solver, pareto, ("--state-pruning", "pareto",
            "--bound", "min-arcs", "--residual-propagation", "unary"),
            formulations=("transition",))
        validate_pruning(solver, pareto, options=("--bound", "min-arcs",
            "--residual-propagation", "unary"), formulations=("transition",))
        for formulation, engine in (("closed", "astar"), ("transition", "bab")):
            result = subprocess.run([str(solver), str(duplicates), "--engine", engine,
                "--formulation", formulation, "--state-pruning", "pareto"],
                capture_output=True, text=True)
            assert result.returncode == 1 and not result.stdout, result
        # Equal-cost routes still retain decisions after cost branching.
        branching = directory/"branching.txt"
        branching.write_text("5\n" + "\n".join(
            " ".join("0" if i == j else "1" for j in range(5)) for i in range(5)) +
            "\n" + "0 100\n"*5)
        branching_optimum = CHECKER["enumerate_tours"](
            CHECKER["read_instance"](branching))["optimum"]["scaled_cost"]
        for formulation in formulations:
            for engine in engines:
                summary, _ = run_solver(solver, branching, engine, ("--node-limit", "0"),
                                        formulation=formulation)
                assert summary["termination"] == "stopped"
                assert summary["statistics"]["node"] > 0
            for limit in ("--max-width", "--frontier-limit"):
                summary, _ = run_solver(solver, branching, "cabs", (limit, "1"),
                                        formulation=formulation)
                # A ceiling stops only when further retained work or widening
                # is needed. Tight total bounds can prove this equal-cost tree
                # complete before hitting the ceiling.
                assert summary["termination"] in ("stopped", "optimal"), summary
                if summary["termination"] == "optimal":
                    assert summary["incumbent"]["scaled_cost"] == branching_optimum
        for formulation, growth, limit in (("closed", "1", "--node-limit"),
                                           ("transition", "0", "--frontier-limit")):
            summary, _ = run_solver(solver, branching, "column",
                ("--quota-growth", growth, limit, "0" if limit == "--node-limit" else "1"),
                formulation=formulation)
            assert summary["termination"] == "stopped", summary
        for options in (("--initial-quota", "0"), ("--max-width", "1")):
            result = subprocess.run([str(solver), str(branching), "--engine", "column", *options],
                                    capture_output=True, text=True)
            assert result.returncode == 1 and not result.stdout, result
    complete = stopped = 0
    for case in manifest["published"]:
        proven_cost = None
        for formulation in formulations:
            for engine in engines:
                summary, _ = run_solver(solver, DATA/case["path"], engine,
                                        ("--time-ms", "5000"), DATA/case["tour_path"], formulation)
                assert Fraction(summary["incumbent"]["cost"]) <= Fraction(case["exact_cost"])
                assert summary["termination"] != "infeasible"
                if summary["termination"] == "optimal":
                    cost = summary["incumbent"]["scaled_cost"]
                    assert proven_cost is None or cost == proven_cost
                    proven_cost = cost
                    complete += 1
                else:
                    stopped += 1
    print(f"passed closed/lazy/eager BAB/A*/CABS: tiny optima/infeasibility, "
          f"matching transition tours, focused progressive/fixed column checks, "
          f"Pareto off/on enumeration, nonmetric filtering, "
          f"minimum-arc bounds and residual unary propagation, "
          f"checked seeds and stops; "
          f"canonical runs: {complete} proven, {stopped} stopped (5 seconds per run)")


def validate(reader, selected, solver):
    manifest = json.loads((DATA/"manifest.json").read_text())
    cases = manifest["cases"]
    assert Counter(c["split"] for c in cases) == {"development": 12, "held-out": 24}
    assert len({c["id"] for c in cases}) == len(cases)
    assert len({c["path"] for c in cases}) == len(cases)
    for key in (lambda c: (c["family"], c["member"]), lambda c: c["sha256"]):
        dev = {key(c) for c in cases if c["split"] == "development"}
        held = {key(c) for c in cases if c["split"] == "held-out"}
        assert not dev & held, "split overlap"
    for family in manifest["sources"]:
        assert Counter(c["split"] for c in cases if c["family"] == family) == {
            "development": 3, "held-out": 6}
    checked = 0
    for case in [*manifest["tiny"], *manifest["published"]]:
        path = DATA/case["path"]
        assert hashlib.sha256(path.read_bytes()).hexdigest() == case["sha256"]
        validate_native(reader, path)
        data = CHECKER["read_instance"](path)
        if "tour_path" in case:
            tour_path = DATA/case["tour_path"]
            assert hashlib.sha256(tour_path.read_bytes()).hexdigest() == case["tour_sha256"]
            tour = [int(x) for x in tour_path.read_text().split()]
            result = CHECKER["check_tour"](data, tour, case["exact_cost"])
            displayed = case["published_cost"]
            places = len(displayed.split(".")[1]) if "." in displayed else 0
            assert abs(Fraction(result["cost"])-Fraction(displayed)) <= Fraction(1, 2*10**places)
        else:
            result = CHECKER["enumerate_tours"](data)
            assert result["feasible_tours"] == case["feasible_tours"]
            optimum = result["optimum"]
            assert (None if optimum is None else optimum["cost"]) == case["optimum"]
        checked += 1
    waiting = CHECKER["read_instance"](DATA/"tiny/waiting.txt")
    result = CHECKER["check_tour"](waiting, [1, 2], "5")
    assert result["visits"][0]["waiting"] == "3" and result["return_time"] == "8"
    reject(lambda: CHECKER["check_tour"](waiting, [1, 1]), "duplicate")
    reject(lambda: CHECKER["check_tour"](waiting, [1]), "omits")
    reject(lambda: CHECKER["check_tour"](waiting, [1, 3]), "invalid")
    reject(lambda: CHECKER["check_tour"](waiting, [1, 2], "8"), "incorrect cost")
    for case, message in [("late-customer", "late customer"), ("late-return", "late return")]:
        data = CHECKER["read_instance"](DATA/f"tiny/{case}.txt")
        reject(lambda: CHECKER["check_tour"](data, [1, 2]), message)
    with tempfile.TemporaryDirectory(prefix="tsptw-validate-") as directory:
        path = Path(directory)/"input.txt"
        good = "2\n0 1\n1 0\n0 10\n0 10\n"
        bad_inputs = [good.replace("2\n", "3\n", 1), good+"1\n",
                      good.replace("0 1", "0 -1", 1), good.replace("0 1", "0 1.0.00", 1),
                      good.replace("0 1", "0 1e1", 1), good.replace("0 1", "0 .", 1),
                      good.replace("0 10", "11 10", 1),
                      good.replace("0 10", "1 10", 1), good.replace("0 1", "0 2147483647", 1),
                      good.replace("0 1", "0 1073741824", 1),
                      good.replace("0 1", "0 0.0000000001", 1),
                      good.replace("0 10", "0 2147483646", 1)]
        for content in bad_inputs:
            path.write_text(content)
            try:
                CHECKER["read_instance"](path)
            except ValueError:
                pass
            else:
                raise AssertionError(f"Python accepted invalid data: {content}")
            assert subprocess.run([str(reader), str(path)], capture_output=True).returncode == 1
        path.write_text(good.replace("0 1", "0 1.000", 1)+"# optional comment\n")
        validate_native(reader, path)
        for zero in (".0", ".00"):
            path.write_text(good.replace("0 1", f"{zero} 1", 1))
            validate_native(reader, path)
        tour_path = Path(directory)/"tour.txt"
        tour_path.write_text("1 2\n")
        command = ["python3", str(Path(__file__).with_name("tsptw-check.py")), "check",
                   str(DATA/"tiny/waiting.txt"), str(tour_path), "--cost", "5"]
        assert subprocess.run(command, capture_output=True).returncode == 0
        tour_path.write_text("1 1\n")
        assert subprocess.run(command, capture_output=True).returncode == 1
    if selected:
        for case in cases:
            path = selected/case["path"]
            assert hashlib.sha256(path.read_bytes()).hexdigest() == case["sha256"]
            validate_native(reader, path)
            checked += 1
    print(f"passed: {checked} instances, five tiny enumerations, four published tours, "
          "reader rejection cases, checker failures, CLI smoke, and disjoint 12/24 selection")
    if solver:
        validate_solver(solver, manifest)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reader", type=Path, required=True, help="compiled misc/tsptw-read.cpp")
    parser.add_argument("--selected", type=Path, help="also validate explicitly acquired 36-case corpus")
    parser.add_argument("--solver", type=Path, help="also validate the native tsptw executable")
    args = parser.parse_args()
    validate(args.reader.resolve(), args.selected, args.solver.resolve() if args.solver else None)
