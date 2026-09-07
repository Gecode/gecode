#!/usr/bin/env python3
"""Actual CLI conformance; prebuilt binary, finite fixtures, no timing claims.

Default requires Native and HiGHS. Native-only builds must pass --highs unavailable,
in which case HiGHS requests must explicitly fail rather than silently fall back.
Each child has a deadline, and an aggregate deadline prevents silently skipped cases.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import itertools
import json
import math
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import tempfile
import time


@dataclass(frozen=True)
class Array:
    dimensions: tuple[tuple[int, int], ...]
    values: tuple[int | str, ...]


@dataclass
class Output:
    assignments: dict[str, int | str | Array]
    markers: list[str]
    comments: list[str]


class Failure(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise Failure(message)


def scalar(text: str) -> int | str:
    text = text.strip()
    if text in ("true", "false"):
        return text  # Keep Boolean true distinct from integer 1.
    require(re.fullmatch(r"-?\d+", text) is not None, f"Noninteger source output: {text!r}")
    return int(text)


def parse_output(text: str) -> Output:
    assignments: dict[str, int | str | Array] = {}
    markers: list[str] = []
    comments: list[str] = []
    for line in text.splitlines():
        if not line:
            continue
        if line.startswith("%"):
            comments.append(line)
            continue
        if line in ("----------", "==========", "=====UNKNOWN=====", "=====UNSATISFIABLE====="):
            markers.append(line)
            continue
        require(not markers, f"Assignment after a status/solution marker: {line!r}")
        match = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_]*) = (.*);", line)
        require(match is not None, f"Unexpected stdout content: {line!r}")
        name, raw = match.groups()
        require(name not in assignments, f"Duplicate source output: {name}")
        array = re.fullmatch(r"array(\d+)d\((.*), \[(.*)\]\)", raw)
        if array:
            arity, dimensions, contents = array.groups()
            dims = []
            for dimension in dimensions.split(","):
                bounds = re.fullmatch(r"\s*(-?\d+)\.\.(-?\d+)\s*", dimension)
                require(bounds is not None, f"Unexpected array dimension: {dimension!r}")
                dims.append(tuple(map(int, bounds.groups())))
            require(len(dims) == int(arity), "arrayNd dimension count mismatch")
            values = tuple(scalar(item) for item in contents.split(",")) if contents else ()
            require(math.prod(max(0, hi - lo + 1) for lo, hi in dims) == len(values),
                    "Array output cardinality mismatch")
            assignments[name] = Array(tuple(dims), values)
        else:
            assignments[name] = scalar(raw)
    return Output(assignments, markers, comments)


class Suite:
    def __init__(self, binary: Path, fixtures: Path, timeout: float) -> None:
        self.binary, self.fixtures = binary, fixtures
        self.deadline = time.monotonic() + timeout
        self.results: list[dict[str, object]] = []
        self.sources: dict[str, str] = {}

    def fixture(self, name: str) -> Path:
        path = self.fixtures / name
        require(path.is_file(), f"Required fixture is missing: {path}")
        self.sources[name] = hashlib.sha256(path.read_bytes()).hexdigest()
        return path

    def invoke(self, name: str, args: list[str], data: bytes | None = None) -> tuple[int, str, str]:
        remaining = self.deadline - time.monotonic()
        require(remaining > 0, f"Aggregate CLI test deadline before required case {name}")
        started = time.monotonic()
        command = [str(self.binary), *args]
        process = subprocess.Popen(command, stdin=subprocess.PIPE if data is not None else subprocess.DEVNULL,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   start_new_session=os.name == "posix")
        try:
            stdout, stderr = process.communicate(input=data, timeout=min(8.0, remaining))
        except subprocess.TimeoutExpired as error:
            if os.name == "posix":
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
            else:
                process.kill()  # The trusted driver creates solver threads, not child programs.
            try:
                process.communicate(timeout=1.0)
            except subprocess.TimeoutExpired:
                process.kill()
            raise Failure(f"CLI child deadline: {name}, command={command!r}") from error
        elapsed = time.monotonic() - started
        require(len(stdout) <= 262144 and len(stderr) <= 262144, f"Excessive CLI output: {name}")
        out, err = stdout.decode("utf-8", "strict"), stderr.decode("utf-8", "strict")
        self.results.append({"case": name, "returncode": process.returncode, "elapsed_seconds": elapsed})
        require(time.monotonic() <= self.deadline, f"Aggregate CLI test deadline after {name}")
        return process.returncode, out, err

    def solve(self, name: str, fixture: str, expected: dict[str, int | str | Array],
              *, backend: str = "native", mode: str = "optimal", stdin: bool = False) -> None:
        path = self.fixture(fixture)
        args = ["-" if stdin else str(path), "--backend", backend]
        code, stdout, stderr = self.invoke(name, args, path.read_bytes() if stdin else None)
        require(code == 0 and not stderr, f"{name}: expected success, got {code}; stderr={stderr!r}; stdout={stdout!r}")
        parsed = parse_output(stdout)
        require(parsed.assignments == expected, f"{name}: wrong source assignment: {parsed.assignments!r}")
        markers = {"optimal": ["----------", "=========="], "satisfy": ["----------"],
                   "unsat": ["=====UNSATISFIABLE====="]}[mode]
        require(parsed.markers == markers, f"{name}: wrong completion markers: {parsed.markers!r}")
        comment = "% guarantee: exact integer search" if backend == "native" else "% guarantee: numerical; requested MIP gaps: 0"
        require(parsed.comments == [comment], f"{name}: missing/wrong guarantee attribution: {parsed.comments!r}")

    def error(self, name: str, args: list[str], text: str | None = None, contains: str = "") -> None:
        code, stdout, stderr = self.invoke(name, args, None if text is None else text.encode())
        require(code == 2 and stdout == "" and bool(stderr),
                f"{name}: rejection must emit only stderr, got code={code}, stdout={stdout!r}, stderr={stderr!r}")
        require(contains.casefold() in stderr.casefold(), f"{name}: unexpected error: {stderr!r}")

    def native_controls(self, highs: str) -> None:
        # Original source oracle: two binary items of weight 2 cannot both fit
        # capacity 3; z counts chosen items. No reference result is used.
        source = ("var 0..1: x; var 0..1: y; var 0..2: z :: output_var; "
                  "constraint int_lin_le([2,2],[x,y],3); "
                  "constraint int_lin_eq([1,1,-1],[x,y,z],0); solve maximize z;")
        routes = [
            ("auto", []), ("plain", []),
            ("race", ["--native-race-seconds", "0.02", "--native-race-nodes", "2"]),
            ("auto", ["--native-auto-presolve", "off", "--native-auto-components", "off",
                      "--native-auto-symmetry", "off", "--native-auto-knapsack", "off"]),
            ("configured", ["--native-search", "bab"]),
            ("configured", ["--native-search", "dfs", "--native-branching", "reliability",
                            "--native-branching-probes", "8", "--native-neighborhood", "hamming",
                            "--native-neighborhood-radius", "1", "--native-neighborhood-nodes", "8",
                            "--native-neighborhood-seconds", "0.02", "--native-max-open-nodes", "100"]),
            ("configured", ["--native-search", "best-bound"]),
        ]
        for order in ("bab", "dfs", "best-bound"):
            for lp in ("root", "updated"):
                extra = ["--native-search", order, "--native-lp", lp, "--native-root-cuts", "on",
                         "--native-bound-tightening", "off"]
                if lp == "updated":
                    extra += ["--native-lp-interval", "2"]
                routes.append(("configured", extra))
        for index, (mode, extra) in enumerate(routes):
            args = ["--minizinc", "--native-mode", mode, "-", "--native-diagnostics", "on", *extra]
            code, stdout, stderr = self.invoke(f"native-controls-{index}", args, source.encode())
            lp_requested = "--native-lp" in extra
            if lp_requested and highs == "unavailable":
                require(code == 2 and not stdout and "unsupported" in stderr.lower(),
                        "Requested unavailable checked LP must fail explicitly")
                continue
            parsed = parse_output(stdout)
            require(code == 0 and not stderr and parsed.assignments == {"z": 1} and
                    parsed.markers == ["----------", "=========="],
                    f"Native route {index} failed its independent source oracle: {code}, {stdout!r}, {stderr!r}")
            require(f"% native-mode: {mode}" in parsed.comments, "Requested native route missing from diagnostics")
            backend = next((line for line in parsed.comments if line.startswith("% native-backend: ")), "")
            require("Gecode native" in backend, "Actual native backend attribution missing")
            if mode == "configured" and "bab" not in extra:
                require("native frontier" in backend and "frontier-admitted=" in stdout,
                        "Configured frontier controls did not reach the frontier solver")
            if lp_requested:
                require("checked LP" in backend and re.search(r"lp-calls=[1-9]\d*", stdout),
                        "Checked LP controls did not perform an LP attempt")
            if "--native-neighborhood" in extra:
                require("neighborhood-attempts=" in stdout and "branching-probes=" in stdout,
                        "Requested branching/neighborhood work counters missing")
            if "--native-auto-presolve" in extra:
                require(all(f"auto-{feature}=off" in stdout for feature in ("presolve", "components", "symmetry", "knapsack")),
                        "Automatic transformation flags were not forwarded")
        for mode in ("auto", "plain", "race", "configured"):
            code, stdout, stderr = self.invoke(f"native-controls-zero-{mode}",
                ["--minizinc", "-", "--native-mode", mode, "--native-node-limit", "0"], source.encode())
            parsed = parse_output(stdout)
            require(code == 0 and not parsed.assignments and parsed.markers == ["=====UNKNOWN====="],
                    f"Native mode {mode} ignored the shared zero node budget")
        invalid = [
            ["--native-mode", "race", "--native-race-nodes", "0"],
            ["--native-mode", "race", "--native-race-seconds", "nan"],
            ["--native-mode", "plain", "--native-auto-presolve", "off"],
            ["--native-mode", "configured", "--native-root-cuts", "on"],
            ["--native-mode", "configured", "--native-neighborhood-nodes", "1"],
            ["--native-mode", "configured", "--native-search", "bab", "--native-branching", "reliability"],
            ["--native-mode", "configured", "--native-lp", "updated", "--native-lp-interval", "4294967296"],
            ["--native-node-limit", "18446744073709551616"],
            ["--native-mode", "auto", "--native-mode", "auto"],
            ["--native-diagnostics", "yes"],
        ]
        for index, flags in enumerate(invalid):
            self.error(f"native-controls-reject-{index}", ["--minizinc", "-", *flags], source)
        self.error("native-controls-highs-rejected", ["-", "--backend", "highs", "--native-mode", "auto"], source,
                   "require the native backend")
        self.error("native-controls-node-alias-duplicate", ["-", "--node-limit", "2", "--native-node-limit", "3"], source,
                   "repeated")

    def cases(self, highs: str) -> None:
        # Analytic source oracles, independent of any solver result.
        alias = {"a": Array(((-1, 0), (2, 3)), (1, 1, 2, 3)), "y": 1}
        channel = {"b": "true", "x": 1}
        candidates = [(2*x+y, x, y) for x, y in itertools.product(range(-2, 5), range(-1, 4)) if x-y <= 2]
        objective, x, y = max(candidates)
        require(sum(candidate[0] == objective for candidate in candidates) == 1, "Oracle maximum is not unique")
        maximum = {"x": x, "y": y, "z": objective}
        for backend in ("native", "highs"):
            if backend == "highs" and highs == "unavailable":
                self.error("highs-unavailable", [str(self.fixture("linear-alias.fzn")), "--backend", backend], contains="unsupported")
                continue
            self.solve(f"{backend}-alias-file", "linear-alias.fzn", alias, backend=backend)
            self.solve(f"{backend}-contiguous-domain", "cli-v2-domain-contiguous.fzn", {"x": 0}, backend=backend)
            self.solve(f"{backend}-channel-stdin", "boolean-channel.fzn", channel, backend=backend, stdin=True)
            self.solve(f"{backend}-maximum", "cli-v1-max-linear.fzn", maximum, backend=backend)
            self.solve(f"{backend}-satisfaction", "cli-v1-satisfy-hidden.fzn", {"x": 1}, backend=backend, mode="satisfy")
            self.solve(f"{backend}-infeasible-alias", "cli-v1-infeasible-alias.fzn", {}, backend=backend, mode="unsat")
            self.solve(f"{backend}-constant-goal", "cli-v1-constant-objective.fzn", {"w": 5}, backend=backend)
            self.solve(f"{backend}-empty-array", "cli-v1-empty-output.fzn", {"a": Array(((1, 0),), ())}, backend=backend, mode="satisfy")
            self.solve(f"{backend}-reified-complement", "cli-v1-reified-complement.fzn", {"b": "false", "x": 2}, backend=backend)
            feasible = [(yv, xv) for xv, yv in itertools.product(range(-1, 3), range(-2, 3)) if not (-2*xv+yv-xv <= -1)]
            min_y, min_x = min(feasible)
            require(sum(yv == min_y for yv, xv in feasible) == 1, "Reification oracle is not unique")
            self.solve(f"{backend}-reified-signed-alias", "cli-v1-reified-signed-alias.fzn", {"b": "false", "x": min_x, "y": min_y}, backend=backend)
        # Global source semantics remain present even when another backend lacks them.
        table_candidates = [(yv, xv) for xv in (-3, 1, 3) for yv in range(-2, 3)
                            if (xv, yv) in ((-3, 2), (1, -2), (3, 1))]
        table_y, table_x = min(table_candidates)
        hole_candidates = sorted(set((-3, -1, 1, 4)) & set((-1, 1, 3)))
        hole_min = min(v for v in hole_candidates if v >= 0)
        # Enumerate original successor functions, then walk their labeled graph.
        circuits = []
        for successors in itertools.product(range(3, 6), repeat=3):
            seen, node = set(), 3
            for _ in range(3):
                if node in seen:
                    break
                seen.add(node)
                node = successors[node-3]
            if len(seen) == 3 and node == 3:
                circuits.append(successors)
        circuit = min(circuits)
        require(sum(c[0] == circuit[0] for c in circuits) == 1, "Circuit optimum is not unique")
        # Independent integer-time scheduling oracle. Integer endpoints make
        # these checks exhaustive for half-open continuous-time task usage.
        def cumulative(starts: tuple[int, ...], durations: tuple[int, ...],
                       heights: tuple[int, ...], capacity: int) -> bool:
            require(len(starts) == len(durations) == len(heights), "Malformed scheduling oracle")
            require(capacity >= 0 and all(v >= 0 for v in (*durations, *heights)), "Negative oracle task data")
            first = min(starts, default=0)
            last = max((s+d for s, d in zip(starts, durations)), default=first)
            return all(sum(h for s, d, h in zip(starts, durations, heights) if s <= t < s+d) <= capacity
                       for t in range(first, last))
        zero_starts = [s for s in range(-2, 3) if cumulative((s,), (0,), (2,), 1)]
        require(zero_starts == list(range(-2, 3)), "Zero-duration task incorrectly consumes capacity")
        require(not any(cumulative((s,), (1,), (2,), 1) for s in range(-2, 3)), "Positive-duration excess demand is feasible")
        touching_starts = [s for s in range(-1, 3) if cumulative((-1, s), (1, 2), (2, 2), 2)]
        require(touching_starts == [0, 1, 2], "Shared start/end time has the wrong half-open usage")
        require(not cumulative((-1, -1), (2, 1), (2, 1), 2), "Overlapping demand oracle is feasible")
        fixed_durations = set((0, 2)) & set(range(1, 4))
        fixed_capacities = set((1, 2)) & {2}
        require(fixed_durations == {2} and fixed_capacities == {2}, "Original alias parameter domains are not singleton")
        duration, capacity, height = next(iter(fixed_durations)), next(iter(fixed_capacities)), 1
        fixed_starts = [s for s in range(-1, 4) if cumulative((-1, s), (duration, 1), (height, 2), capacity)]
        require(fixed_starts == [1, 2, 3], "Fixed-alias scheduling oracle changed")
        # Walk raw one-based FlatZinc transition tables. The oracle never uses
        # compiled sparse IR, native DFA objects, or solver output as its input.
        def regular(word: tuple[int, ...], states: int, symbols: int,
                    table: tuple[int, ...], initial: int, finals: tuple[int, ...]) -> bool:
            require(states > 0 and symbols > 0 and len(table) == states*symbols,
                    "Malformed regular oracle dimensions")
            require(1 <= initial <= states and all(0 <= target <= states for target in table)
                    and all(1 <= final <= states for final in finals), "Malformed regular oracle states")
            current = initial
            for symbol in word:
                if not 1 <= symbol <= symbols:
                    return False
                current = table[(current-1)*symbols+symbol-1]
                if current == 0:
                    return False
            return current in finals
        regular_candidates = [word for word in itertools.product(range(4), repeat=2)
                              if regular(word, 3, 2, (2,3,0,3,3,0), 1, (3,))]
        require(regular_candidates == [(1,2), (2,1)], "Regular accepted-word oracle changed")
        regular_word = min(regular_candidates)
        repeated_values = [v for v in (1,2) if regular((v,v,v), 2, 2, (1,2,2,1), 1, (2,))]
        require(repeated_values == [2], "Regular repeated-alias oracle changed")
        require(not regular((1,1), 2, 2, (1,2,2,1), 1, (2,)), "Nonfinal regular word was accepted")
        require(regular((), 2, 2, (0,0,0,0), 2, (2,)) and
                not regular((), 2, 2, (0,0,0,0), 2, (1,)), "Regular empty-word semantics changed")
        require(not regular((1,), 1, 1, (0,), 1, (1,)) and
                not regular((0,), 1, 1, (1,), 1, (1,)) and
                not regular((), 1, 1, (1,), 1, ()), "Regular failure-state/alphabet/final-set semantics changed")
        nonunit_words = [v for v in range(4) if regular((v,), 3, 2, (2,3,0,0,0,0), 1, (3,1,3))]
        interval_words = [v for v in range(4) if regular((v,), 3, 2, (2,3,0,0,0,0), 1, tuple(range(2,4)))]
        require(nonunit_words == [2] and interval_words == [1,2], "Regular literal final-set oracle changed")
        globals_cases = (
            ("cli-v3-regular-word.fzn", dict(zip(("x", "y"), regular_word)), "optimal"),
            ("cli-v3-regular-rejected-word.fzn", {}, "unsat"),
            ("cli-v3-regular-alias-repeat.fzn", {"x": repeated_values[0], "y": repeated_values[0]}, "optimal"),
            ("cli-v3-regular-empty-accept.fzn", {"word": Array(((1,0),), ())}, "satisfy"),
            ("cli-v3-regular-empty-reject.fzn", {}, "unsat"),
            ("cli-v3-regular-dead-transition.fzn", {}, "unsat"),
            ("cli-v3-regular-nonunit-finals.fzn", {"x": max(nonunit_words)}, "optimal"),
            ("cli-v3-regular-final-interval.fzn", {"x": min(interval_words)}, "optimal"),
            ("cli-v3-regular-empty-finals.fzn", {}, "unsat"),
            ("cli-v3-regular-zero-symbol.fzn", {}, "unsat"),
            ("cli-v2-cumulative-zero-duration.fzn", {"s": min(zero_starts)}, "optimal"),
            ("cli-v2-cumulative-positive-duration-unsat.fzn", {}, "unsat"),
            ("cli-v2-cumulative-half-open.fzn", {"a": -1, "b": min(touching_starts)}, "optimal"),
            ("cli-v2-cumulative-overlap-unsat.fzn", {}, "unsat"),
            ("cli-v2-cumulative-fixed-alias.fzn", {"d": duration, "duration_alias": duration, "h": height,
                                                    "capacity": capacity, "capacity_alias": capacity,
                                                    "s": min(fixed_starts)}, "optimal"),
            ("cli-v2-circuit-offset.fzn", dict(zip(("x", "y", "z"), circuit)), "optimal"),
            ("cli-v2-circuit-subtours.fzn", {}, "unsat"),
            ("cli-v2-circuit-singleton.fzn", {}, "satisfy"),
            ("cli-v2-table-holes.fzn", {"x": table_x, "y": table_y}, "optimal"),
            ("cli-v2-table-alias-unsat.fzn", {}, "unsat"),
            ("cli-v2-table-empty.fzn", {}, "unsat"),
            ("cli-v2-holey-alias.fzn", {"x": hole_min, "y": hole_min}, "optimal"),
            ("cli-v1-global-element.fzn", {"i": 2, "v": -4}, "optimal"),
            ("cli-v1-global-distinct.fzn", {"a": Array(((1, 3),), (1, 2, 3))}, "optimal"),
            ("cli-v1-global-repeated-alias.fzn", {}, "unsat"),
        )
        for fixture, expected, mode in globals_cases:
            self.solve(f"native-{fixture}", fixture, expected, mode=mode)
            self.error(f"highs-rejects-{fixture}", [str(self.fixture(fixture)), "--backend", "highs"], contains="unsupported")
        # Default selection must be native/exact, without a fallback flag.
        default = self.fixture("boolean-channel.fzn")
        code, stdout, stderr = self.invoke("default-native", [str(default)])
        parsed = parse_output(stdout)
        require(code == 0 and not stderr and parsed.assignments == channel and
                parsed.comments == ["% guarantee: exact integer search"] and
                parsed.markers == ["----------", "=========="], "Default backend/guarantee/output changed")
        for fixture, fragment in (("cli-v3-regular-malformed-matrix.fzn", "Q*S"),
                                  ("cli-v3-regular-bad-target.fzn", "0..Q"),
                                  ("cli-v3-regular-bad-initial.fzn", "1..Q"),
                                  ("cli-v3-regular-nonliteral-parameter.fzn", "expected an integer literal"),
                                  ("cli-v3-regular-unsupported-set.fzn", "gecode_regular_set"),
                                  ("cli-v3-regular-wrong-arity.fzn", "arity"),
                                  ("cli-v3-regular-bad-count.fzn", "positive"),
                                  ("cli-v2-circuit-empty.fzn", "nonempty"),
                                  ("cli-v2-cumulative-malformed-four.fzn", "equal lengths"),
                                  ("cli-v2-cumulative-wrong-arity.fzn", "arity"),
                                  ("cli-v2-cumulative-unsupported-six.fzn", "multi-machine"),
                                  ("cli-v2-cumulative-unsupported-seven.fzn", "fzn_cumulatives"),
                                  ("cli-v2-cumulative-unfixed-parameter.fzn", "singleton"),
                                  ("cli-v2-table-zero-arity.fzn", "zero-arity"),
                                  ("cli-v1-malformed-arity.fzn", "argument"),
                                  ("cli-v1-unknown-predicate.fzn", "cli_v1_unknown_predicate"),
                                  ("cli-v1-unbounded-domain.fzn", "finite explicit domain"),
                                  ("unsupported.fzn", "int_times")):
            self.error(f"reject-{fixture}", [str(self.fixture(fixture))], contains=fragment)
        self.error("malformed-stdin", ["-"], "var int: x; solve minimize missing;", "missing")
        self.error("unsupported-string-escape", ["-"], "solve :: invalid(\"\\123\") satisfy;", "escape")
        code, stdout, stderr = self.invoke("invalid-utf8-stdin", ["-"], b'solve :: text("\xff") satisfy;')
        require(code == 2 and not stdout and "UTF-8" in stderr, "Invalid UTF-8 input was not rejected")
        self.error("missing-file", [str(self.fixtures / "cli-v1-does-not-exist.fzn")], contains="Cannot open")
        self.error("unknown-flag", [str(default), "--not-a-solver-flag", "1"], contains="Unsupported option")
        self.error("unknown-backend", [str(default), "--backend", "auto"], contains="Backend")
        self.error("missing-option-value", [str(default), "--node-limit"], contains="Missing")
        self.error("duplicate-option", [str(default), "--node-limit", "1", "--node-limit", "2"], contains="repeated")
        self.error("nonfinite-time", [str(default), "--time-limit", "nan"], contains="finite")
        self.error("negative-node-count", [str(default), "--node-limit", "-1"], contains="unsigned")
        self.error("input-byte-limit", [str(default), "--max-input-bytes", "1"], contains="byte limit")
        self.error("no-arguments", [], contains="Usage")
        # Exit status 1 and UNKNOWN are never UNSAT or an exhaustive marker.
        source = self.fixture("cli-v1-max-linear.fzn")
        for key, value in (("--time-limit", "0"), ("--node-limit", "0")):
            code, stdout, stderr = self.invoke(f"zero-{key[2:]}", [str(source), key, value])
            parsed = parse_output(stdout)
            require(code == 1 and not parsed.assignments and parsed.markers == ["=====UNKNOWN====="],
                    f"Zero budget incorrectly completed: {code}, {stdout!r}, {stderr!r}")
        code, stdout, stderr = self.invoke("native-one-node", [str(source), "--node-limit", "1"])
        limited = parse_output(stdout)
        require(code == 1 and "node_limit" in stderr, f"One-node fixture did not actually interrupt: {code}, {stdout!r}, {stderr!r}")
        require(limited.comments == ["% guarantee: exact integer search"], "Limited native result lost attribution")
        if limited.assignments:
            require(limited.markers == ["----------"] and set(limited.assignments) == {"x", "y", "z"}, "Limited witness claimed completion")
            xv, yv, zv = (limited.assignments[key] for key in ("x", "y", "z"))
            require(all(type(v) is int for v in (xv, yv, zv)) and -2 <= xv <= 4 and -1 <= yv <= 3 and
                    xv-yv <= 2 and zv == 2*xv+yv and zv <= objective, "Limited source witness is invalid")
        else:
            require(limited.markers == ["=====UNKNOWN====="], "Limit without witness must be UNKNOWN")
        code, stdout, stderr = self.invoke("help", ["--help"])
        require(code == 0 and not stderr and "Usage:" in stdout, "Help invocation failed")
        # Quoting and file-path handling must not depend on the source checkout name.
        with tempfile.TemporaryDirectory(prefix="fzn driver path ") as directory:
            spaced = Path(directory) / "source model.fzn"
            spaced.write_bytes(default.read_bytes())
            code, stdout, stderr = self.invoke("filename-with-spaces", [str(spaced)])
            require(code == 0 and not stderr and parse_output(stdout).assignments == channel, "Spaced filename failed")
        self.native_controls(highs)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True, type=Path)
    parser.add_argument("--fixtures", type=Path, default=Path(__file__).parent / "flatzinc-fixtures")
    parser.add_argument("--highs", choices=("available", "unavailable"), default="available")
    parser.add_argument("--timeout", type=float, default=60.0)
    args = parser.parse_args()
    try:
        require(args.binary.is_file(), f"Required prebuilt driver is missing: {args.binary}")
        require(math.isfinite(args.timeout) and args.timeout > 0, "Test timeout must be finite and positive")
        digest = hashlib.sha256(args.binary.read_bytes()).hexdigest()
        suite = Suite(args.binary.resolve(), args.fixtures.resolve(), args.timeout)
        suite.cases(args.highs)
        require(len(suite.results) == (135 if args.highs == "available" else 126), "Required CLI cases were skipped")
        print(json.dumps({"status": "passed", "binary_sha256": digest, "cases": len(suite.results),
                          "fixture_sha256": suite.sources, "checks": suite.results}, sort_keys=True))
        return 0
    except (Failure, OSError, UnicodeError) as error:
        print(f"FlatZinc driver CLI conformance failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
