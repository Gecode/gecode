#!/usr/bin/env -S python3
"""Exact small DMA/register comparisons for Gecode, Z3, and Bitwuzla.

The SMT adapters deliberately use only the command-line SMT-LIB interface.
Unavailable solvers and unsupported command-line interfaces remain explicit
matrix cells; they are never interpreted as UNSAT.
"""

from __future__ import annotations

import argparse
import itertools
import json
import selectors
import shutil
import subprocess
import time
from pathlib import Path
from typing import Any

LENGTHS = (0x20, 0x30, 0x20, 0x40, 0x30, 0x20)


def dma_rows(p: dict[str, Any]) -> list[list[int]]:
    n, slack, cap = p["descriptor_count"], p["window_slack"], p["selected_cap"]
    end = 0x100 + sum(LENGTHS[i % 6] for i in range(n)) + slack
    bases = range(0x100, end - 0x20 + 1, 16)
    rows = []
    for bs in itertools.product(bases, repeat=n):
        ends = tuple(bs[i] + LENGTHS[i % 6] for i in range(n))
        if any(ends[i] > bs[i + 1] for i in range(n - 1)) or ends[-1] > end:
            continue
        for fs in itertools.product((1, 3, 5), repeat=n):
            if tuple(sorted(fs)) != fs or sum(bool(f & 2) for f in fs) != n // 3 \
                    or sum(bool(f & 4) for f in fs) != n // 3:
                continue
            for index in range(n):
                if ends[index] + (0x10 if fs[index] & 2 else 0) <= min(cap, end):
                    rows.append([index, *bs, *fs])
    return rows


def lookup_rows(p: dict[str, Any]) -> list[list[int]]:
    # This extends the example's overlapping windows by repeating its four
    # entries. The selected lookup value must be 5 or 6.
    size, allowed = p["size"], p["allowed_indices"]
    minima = (3, 4, 6, 7)
    domains = [tuple(range(minima[i % 4], minima[i % 4] + 2)) for i in range(size)]
    return [[index, *values] for index in allowed for values in
            itertools.product(*domains) if values[index] in (5, 6)]


def allocation_rows(p: dict[str, Any]) -> list[list[int]]:
    banks, regs, slots = p["banks"], p["registers_per_bank"], p["slots_per_bank"]
    if regs > slots:
        return []
    choices = [tuple(0x40 * bank + 0x10 * slot for slot in range(slots))
               for bank in range(banks)]
    return [[*sum(parts, ())] for parts in itertools.product(
        *(itertools.permutations(choice, regs) for choice in choices))]


def signed(value: int, width: int) -> int:
    return value - (1 << width) if value & (1 << (width - 1)) else value


def domain(p: dict[str, Any], name: str) -> range:
    return range(p[f"{name}_min"], p[f"{name}_max"] + 1)


def overflow_matches(p: dict[str, Any], x: int, y: int) -> bool:
    kind = p.get("overflow")
    if kind is None: return True
    width = p["width"]
    if kind == "unsigned_mult": actual = x*y > (1 << width)-1
    elif kind == "signed_mult":
        value=signed(x,width)*signed(y,width)
        actual=not -(1 << (width-1)) <= value < (1 << (width-1))
    elif kind == "signed_div":
        actual=signed(x,width) == -(1 << (width-1)) and signed(y,width) == -1
    else: raise ValueError(f"unknown overflow kind {kind!r}")
    return actual == p["overflow_expected"]


def arithmetic_rows(case: dict[str, Any]) -> list[list[int]]:
    p, family = case["parameters"], case["family"]
    width, mask = p["width"], (1 << p["width"]) - 1
    rows: list[list[int]] = []
    if family == "mult":
        for x in domain(p, "x"):
            for y in domain(p, "y"):
                result = (x * y) & mask
                if p["result_min"] <= result <= p["result_max"] and overflow_matches(p,x,y):
                    rows.append([x, y, result])
    elif family == "divmod":
        for x in domain(p, "x"):
            for y in domain(p, "y"):
                if p.get("signed", False):
                    sx, sy = signed(x, width), signed(y, width)
                    if sy == 0:
                        q, r = (1 if sx < 0 else mask), x
                    elif sx == -(1 << (width-1)) and sy == -1:
                        q, r = x, 0
                    else:
                        quotient = abs(sx) // abs(sy)
                        if (sx < 0) != (sy < 0): quotient = -quotient
                        q, r = quotient & mask, (sx - quotient * sy) & mask
                else:
                    q, r = ((mask, x) if y == 0 else (x // y, x % y))
                if p["q_min"] <= q <= p["q_max"] and p["r_min"] <= r <= p["r_max"] and overflow_matches(p,x,y):
                    rows.append([x, y, q, r])
    elif family == "product_mod":
        modulus = p["modulus"]
        if modulus <= 0: return []
        if not p.get("guard", 1):
            return [[x,y,z] for x in domain(p,"x") for y in domain(p,"y")
                    for z in domain(p,"result")]
        for x in domain(p, "x"):
            for y in domain(p, "y"):
                result = (x * y) % modulus
                if p["result_min"] <= result <= p["result_max"]:
                    rows.append([x, y, result])
    return rows


def alu(value: int, width: int) -> int:
    mask = (1 << width) - 1
    amount = value & 3
    s1 = (value << amount) & mask
    total = s1 + 0x1d
    s2, carry = total & mask, total > mask
    negative = bool(s2 & (1 << (width - 1)))
    signed_s2 = signed(s2, width)
    shifted = (signed_s2 >> amount) & mask
    s3 = (s2 ^ (0x15 & mask)) if carry else shifted
    return ((s3 + 1) & mask) if negative else s3


def alu_rows(p: dict[str, Any]) -> list[list[int]]:
    return [[value, alu(value, p["width"])] for value in domain(p, "input")
            if p["output_min"] <= alu(value, p["width"]) <= p["output_max"]]


def expected(case: dict[str, Any]) -> list[list[int]]:
    if case["family"] in ("mult", "divmod", "product_mod"):
        return arithmetic_rows(case)
    if case["family"] == "alu": return alu_rows(case["parameters"])
    return {"dma": dma_rows, "lookup": lookup_rows,
            "allocation": allocation_rows}[case["family"]](case["parameters"])


def expected_names(case: dict[str, Any]) -> list[str]:
    p = case["parameters"]
    if case["family"] == "dma":
        n = p["descriptor_count"]
        return ["index", *(f"base[{i}]" for i in range(n)),
                *(f"flag[{i}]" for i in range(n))]
    if case["family"] == "lookup":
        return ["index", *(f"register[{i}]" for i in range(p["size"]))]
    if case["family"] in ("mult", "product_mod"): return ["x", "y", "result"]
    if case["family"] == "divmod": return ["x", "y", "quotient", "remainder"]
    if case["family"] == "alu": return ["input", "output"]
    count = p["banks"] * p["registers_per_bank"]
    return [f"address[{i}]" for i in range(count)]


def bv(value: int, width: int) -> str:
    return f"(_ bv{value} {width})"


def smt_case(case: dict[str, Any]) -> tuple[list[str], list[str]]:
    family, p = case["family"], case["parameters"]
    declarations: list[str] = []
    assertions: list[str] = []
    names: list[str] = []
    if family == "dma":
        n = p["descriptor_count"]
        end = 0x100 + sum(LENGTHS[i % 6] for i in range(n)) + p["window_slack"]
        declarations.append("(declare-fun index () (_ BitVec 4))")
        names.append("index")
        assertions.append(f"(assert (bvult index {bv(n,4)}))")
        for i in range(n):
            declarations += [f"(declare-fun base{i} () (_ BitVec 12))",
                             f"(declare-fun flag{i} () (_ BitVec 4))"]
            names += [f"base{i}"]
            assertions += [f"(assert (bvuge base{i} {bv(0x100,12)}))",
                           f"(assert (bvule base{i} {bv(end-0x20,12)}))",
                           f"(assert (= (bvand base{i} {bv(15,12)}) {bv(0,12)}))",
                           f"(assert (or (= flag{i} {bv(1,4)}) (= flag{i} {bv(3,4)}) (= flag{i} {bv(5,4)})))"]
            if i + 1 < n:
                assertions += [f"(assert (bvule (bvadd base{i} {bv(LENGTHS[i%6],12)}) base{i+1}))",
                               f"(assert (bvule flag{i} flag{i+1}))"]
            assertions.append(f"(assert (bvule (bvadd base{i} {bv(LENGTHS[i%6],12)}) {bv(end,12)}))")
        writes = " ".join(f"(ite (= flag{i} {bv(3,4)}) {bv(1,4)} {bv(0,4)})" for i in range(n))
        executes = " ".join(f"(ite (= flag{i} {bv(5,4)}) {bv(1,4)} {bv(0,4)})" for i in range(n))
        assertions += [f"(assert (= (bvadd {writes}) {bv(n//3,4)}))",
                       f"(assert (= (bvadd {executes}) {bv(n//3,4)}))"]
        for i in range(n):
            selected = LENGTHS[i % 6] + 0x10
            plain = LENGTHS[i % 6]
            limit = f"(ite (= flag{i} {bv(3,4)}) (bvadd base{i} {bv(selected,12)}) (bvadd base{i} {bv(plain,12)}))"
            assertions.append(f"(assert (=> (= index {bv(i,4)}) (bvule {limit} {bv(min(end,p['selected_cap']),12)})))")
        # Public order is index, all bases, all flags.
        names = ["index", *(f"base{i}" for i in range(n)), *(f"flag{i}" for i in range(n))]
    elif family == "lookup":
        size = p["size"]
        declarations.append("(declare-fun index () (_ BitVec 8))")
        names.append("index")
        allowed = p["allowed_indices"]
        assertions.append("(assert (or " + " ".join(f"(= index {bv(i,8)})" for i in allowed) + "))")
        minima = (3, 4, 6, 7)
        for i in range(size):
            declarations.append(f"(declare-fun register{i} () (_ BitVec 8))")
            minimum = minima[i % 4]
            assertions.append(f"(assert (or (= register{i} {bv(minimum,8)}) (= register{i} {bv(minimum+1,8)})))")
            assertions.append(f"(assert (=> (= index {bv(i,8)}) (or (= register{i} {bv(5,8)}) (= register{i} {bv(6,8)}))))")
            names.append(f"register{i}")
    elif family == "allocation":
        banks, regs, slots = p["banks"], p["registers_per_bank"], p["slots_per_bank"]
        for bank in range(banks):
            bank_names = []
            for register in range(regs):
                name = f"address{bank}_{register}"
                names.append(name); bank_names.append(name)
                declarations.append(f"(declare-fun {name} () (_ BitVec 8))")
                values = " ".join(f"(= {name} {bv(0x40*bank+0x10*slot,8)})" for slot in range(slots))
                assertions.append(f"(assert (or {values}))")
            if len(bank_names) > 1:
                assertions.append(f"(assert (distinct {' '.join(bank_names)}))")
    elif family in ("mult", "divmod", "product_mod", "alu"):
        width = p["width"]
        public = expected_names(case)
        symbols = {"quotient": "q", "remainder": "r"}
        names = [symbols.get(name, name) for name in public]
        for name in names:
            declarations.append(f"(declare-fun {name} () (_ BitVec {width}))")
        ranges = {"x": (p.get("x_min"), p.get("x_max")),
                  "y": (p.get("y_min"), p.get("y_max")),
                  "result": (p.get("result_min"), p.get("result_max")),
                  "q": (p.get("q_min"), p.get("q_max")),
                  "r": (p.get("r_min"), p.get("r_max")),
                  "input": (p.get("input_min"), p.get("input_max")),
                  "output": (p.get("output_min"), p.get("output_max"))}
        for name in names:
            lo, hi = ranges[name]
            assertions += [f"(assert (bvuge {name} {bv(lo,width)}))",
                           f"(assert (bvule {name} {bv(hi,width)}))"]
        if family == "mult": assertions.append("(assert (= result (bvmul x y)))")
        elif family == "divmod":
            opq, opr = ("bvsdiv", "bvsrem") if p.get("signed") else ("bvudiv", "bvurem")
            assertions += [f"(assert (= q ({opq} x y)))", f"(assert (= r ({opr} x y)))"]
        elif family == "product_mod":
            guard, modulus = p.get("guard", 1), p["modulus"]
            assertions.append(f"(assert (bvugt {bv(modulus,width)} {bv(0,width)}))")
            if guard: assertions.append(
                f"(assert (= result ((_ extract {width-1} 0) (bvurem (bvmul ((_ zero_extend {width}) x) ((_ zero_extend {width}) y)) {bv(modulus,2*width)}))))")
        else:
            declarations += [f"(define-fun amount () (_ BitVec {width}) (bvand input {bv(3,width)}))",
                f"(define-fun s1 () (_ BitVec {width}) (bvshl input amount))",
                f"(define-fun s2 () (_ BitVec {width}) (bvadd s1 {bv(0x1d,width)}))",
                f"(define-fun carry () Bool (bvult s2 s1))",
                f"(define-fun negative () Bool (= ((_ extract {width-1} {width-1}) s2) #b1))",
                f"(define-fun s3 () (_ BitVec {width}) (ite carry (bvxor s2 {bv(0x15 & ((1<<width)-1),width)}) (bvashr s2 amount)))"]
            assertions.append(f"(assert (= output (ite negative (bvadd s3 {bv(1,width)}) s3)))")
        overflow = p.get("overflow")
        if overflow:
            if overflow == "unsigned_mult":
                condition=f"(not (= ((_ extract {2*width-1} {width}) (bvmul ((_ zero_extend {width}) x) ((_ zero_extend {width}) y))) {bv(0,width)}))"
            elif overflow == "signed_mult":
                condition=f"(not (= ((_ sign_extend {width}) (bvmul x y)) (bvmul ((_ sign_extend {width}) x) ((_ sign_extend {width}) y))))"
            else:
                condition=f"(and (= x {bv(1 << (width-1),width)}) (= y {bv((1 << width)-1,width)}))"
            assertions.append(f"(assert {' ' if p['overflow_expected'] else '(not '}{condition}{'' if p['overflow_expected'] else ')'})")
    return declarations + assertions, names


def parse_values(line: str) -> list[int]:
    # get-value output contains one #x/#b/numeral value per requested symbol.
    import re
    values = []
    for whole, decimal in ((m.group(0), m.group(1)) for m in re.finditer(
            r"#x[0-9a-fA-F]+|#b[01]+|\(_ bv(\d+) \d+\)", line)):
        values.append(int(decimal) if decimal else int(whole[2:], 16 if whole.startswith("#x") else 2))
    return values


def symbol_width(case: dict[str, Any], name: str) -> int:
    if case["family"] in ("mult", "divmod", "product_mod", "alu"):
        return case["parameters"]["width"]
    if case["family"] == "dma":
        if name == "index" or name.startswith("flag"):
            return 4
        return 12
    return 8


def solver_info(name: str, executable: str | None) -> dict[str, Any]:
    if executable is None:
        return {"solver": name, "status": "missing", "command": None,
                "options": [], "version": None}
    option = "--version"
    try:
        done = subprocess.run([executable, option], text=True, capture_output=True, timeout=2)
        output = (done.stdout or done.stderr).strip().splitlines()
        if done.returncode != 0:
            raise RuntimeError(f"version probe exited with status {done.returncode}")
        if not output:
            raise RuntimeError("version probe produced no output")
        version = output[0]
    except (OSError, subprocess.TimeoutExpired, RuntimeError) as error:
        return {"solver": name, "status": "unsupported", "command": executable,
                "options": [], "version": None, "error": str(error)}
    options = ["-smt2", "-in"] if name == "z3" else ["--lang", "smt2", "--produce-models"]
    return {"solver": name, "status": "ready", "command": executable,
            "options": options, "version": version}


def run_solver(info: dict[str, Any], case: dict[str, Any], timeout: float,
               expected_rows: list[list[int]]) -> dict[str, Any]:
    row = {**info, "case": case["id"], "semantic_status": None,
           "decision_variables": case["decision_variables"],
           "solutions": None, "projections": [], "elapsed_seconds": None}
    if info["status"] != "ready":
        return row
    body, names = smt_case(case)
    started = time.monotonic()
    process = None
    try:
        process = subprocess.Popen([info["command"], *info["options"]], text=True,
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            bufsize=1)
        assert process.stdin is not None and process.stdout is not None
        process.stdin.write("(set-option :produce-models true)\n(set-logic QF_BV)\n" +
                            "\n".join(body) + "\n")
        process.stdin.flush()
        projections: list[list[int]] = []
        selector = selectors.DefaultSelector()
        selector.register(process.stdout, selectors.EVENT_READ)

        def response() -> str:
            remaining = timeout - (time.monotonic() - started)
            if remaining <= 0 or not selector.select(remaining):
                raise subprocess.TimeoutExpired(info["command"], timeout)
            line = process.stdout.readline()
            if not line:
                raise RuntimeError("solver closed its output")
            text = line.strip()
            if text.startswith("("):
                balance = text.count("(") - text.count(")")
                while balance > 0:
                    part = process.stdout.readline().strip()
                    if not part:
                        raise RuntimeError("solver truncated an s-expression")
                    text += " " + part
                    balance += part.count("(") - part.count(")")
            return text

        while True:
            process.stdin.write("(check-sat)\n"); process.stdin.flush()
            status = response()
            if status == "unsat":
                break
            if status != "sat":
                raise RuntimeError(f"unsupported solver response {status!r}")
            process.stdin.write("(get-value (" + " ".join(names) + "))\n")
            process.stdin.flush()
            values = parse_values(response())
            if len(values) != len(names):
                raise RuntimeError("could not parse SMT get-value response")
            projections.append(values)
            process.stdin.write("(assert (not (and " + " ".join(
                f"(= {name} {bv(value, symbol_width(case, name))})"
                for name, value in zip(names, values)) + ")))\n")
            process.stdin.flush()
        row.update(status="ok", semantic_status="sat" if projections else "unsat",
                   solutions=len(projections), projections=sorted(projections))
        if sorted(projections) != sorted(expected_rows):
            row.update(status="wrong-result", error="public projection set differs from independent evaluator")
    except subprocess.TimeoutExpired:
        row.update(status="timeout", error=f"case exceeded {timeout:g}s")
    except (OSError, RuntimeError) as error:
        row.update(status="error", error=str(error))
    finally:
        if process is not None:
            process.kill()
            process.wait()
    row["elapsed_seconds"] = time.monotonic() - started
    return row


def gecode_smoke(binary: Path | None, command: list[str], expected_count: int,
                 timeout: float) -> dict[str, Any]:
    if binary is None:
        return {"status": "missing", "command": command, "solutions": None}
    try:
        done = subprocess.run([str(binary), *command], text=True, capture_output=True,
                              timeout=timeout)
        value = json.loads(done.stdout.strip()) if done.returncode == 0 else {}
        status = "ok" if value.get("solutions") == expected_count else "wrong-result"
        return {"status": status, "command": [str(binary), *command],
                "solutions": value.get("solutions"), "metrics": value,
                "stderr": done.stderr}
    except subprocess.TimeoutExpired:
        return {"status": "timeout", "command": [str(binary), *command]}
    except (OSError, json.JSONDecodeError) as error:
        return {"status": "error", "command": [str(binary), *command], "error": str(error)}


def gecode_exact(binary: Path | None, command: list[str], case: dict[str, Any],
                 rows: list[list[int]], timeout: float) -> dict[str, Any]:
    result = gecode_smoke(binary, command, len(rows), timeout)
    if result["status"] != "ok":
        return result
    metrics = result["metrics"]
    projections = metrics.get("projections")
    if metrics.get("semantic_status") != case["expected_status"] or \
            metrics.get("decision_variables") != case["decision_variables"] or \
            not isinstance(projections, list) or sorted(projections) != sorted(rows):
        result.update(status="wrong-result",
                      error="semantic status, public variables, or projections differ")
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cases", type=Path, default=Path(__file__).with_name("mixed-model-cases.json"))
    parser.add_argument("--z3", default=shutil.which("z3"))
    parser.add_argument("--bitwuzla", default=shutil.which("bitwuzla"))
    parser.add_argument("--dma-binary", type=Path)
    parser.add_argument("--lookup-binary", type=Path)
    parser.add_argument("--allocation-binary", type=Path)
    parser.add_argument("--inverse-binary", type=Path)
    parser.add_argument("--alu-binary", type=Path)
    parser.add_argument("--timeout", type=float, default=10.0)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if not 0 < args.timeout <= 10:
        parser.error("--timeout must be in (0,10]")
    cases = json.loads(args.cases.read_text())["cases"]
    infos = [solver_info("z3", args.z3), solver_info("bitwuzla", args.bitwuzla)]
    results = []
    independent = {}
    gecode_results = []
    for case in cases:
        rows = expected(case)
        if case.get("decision_variables") != expected_names(case):
            raise ValueError(f"{case['id']}: incorrect public decision variables")
        status = "sat" if rows else "unsat"
        if case.get("expected_status") != status or \
                (case.get("goal") == "unsat") != (status == "unsat") or \
                (case.get("goal") == "unique" and len(rows) != 1):
            raise ValueError(f"{case['id']}: goal/status contradict evaluator")
        independent[case["id"]] = {"semantic_status": "sat" if rows else "unsat",
                                   "decision_variables": case["decision_variables"],
                                   "solutions": len(rows), "projections": rows}
        results.extend(run_solver(info, case, args.timeout, rows) for info in infos)
        p = case["parameters"]
        if case["family"] == "dma":
            for formulation in ("compact-word", "bounded-word", "word-int-channel", "int-bool"):
                command = ["-formulation", formulation, "-search-control", "public-min",
                    "-projection", "all", "-size", str(p["descriptor_count"]),
                    "-window-slack", str(p["window_slack"]), "-selected-cap",
                    str(p["selected_cap"]), "-solutions", "0"]
                exact = gecode_exact(args.dma_binary, command, case, rows,
                                     args.timeout)
                exact.update(case=case["id"], formulation=formulation)
                gecode_results.append(exact)
        elif case["family"] == "lookup":
            mask = sum(1 << index for index in p["allowed_indices"])
            for formulation in ("compact-word", "bounded-word"):
                command = ["-formulation", formulation, "-size", str(p["size"]),
                           "-allowed-mask", str(mask), "-projection", "all",
                           "-solutions", "0"]
                exact = gecode_exact(args.lookup_binary, command, case, rows,
                                     args.timeout)
                exact.update(case=case["id"], formulation=formulation)
                gecode_results.append(exact)
        elif case["family"] == "allocation":
            for formulation in ("value", "bounds", "int-channel"):
                command = ["register", formulation, str(p["registers_per_bank"]),
                           "1", str(p["slots_per_bank"]), "projections"]
                exact = gecode_exact(args.allocation_binary, command, case, rows,
                                     args.timeout)
                exact.update(case=case["id"], formulation=formulation)
                gecode_results.append(exact)
        elif case["family"] in ("mult", "divmod", "product_mod"):
            exact = gecode_exact(args.inverse_binary, ["--case",case["id"]],
                                 case, rows, args.timeout)
            exact.update(case=case["id"], formulation="native-word")
            gecode_results.append(exact)
        else:
            command = ["--comparison-width",str(p["width"]),
                       "--input-min",str(p["input_min"]),
                       "--input-max",str(p["input_max"]),
                       "--output-min",str(p["output_min"]),
                       "--output-max",str(p["output_max"])]
            exact = gecode_exact(args.alu_binary, command, case, rows,
                                 args.timeout)
            exact.update(case=case["id"], formulation="native-word")
            gecode_results.append(exact)
    # Native-search smoke is intentionally separate from the aligned SMT rows.
    controls = {
        **{f"dma-{formulation}": gecode_smoke(args.dma_binary,
            ["-formulation",formulation,"-search-control","native",
             "-projection","none","-size","3","-solutions","0"],
            30, args.timeout)
           for formulation in ("compact-word", "bounded-word", "word-int-channel", "int-bool")},
        **{f"lookup-{formulation}": gecode_smoke(args.lookup_binary,
            ["-formulation",formulation,"-solutions","0"], 16, args.timeout)
           for formulation in ("compact-word", "bounded-word")},
        "allocation-ipl-val": gecode_smoke(args.allocation_binary,
            ["register","value","3","1"], 576, args.timeout),
        "allocation-ipl-bnd": gecode_smoke(args.allocation_binary,
            ["register","bounds","3","1"], 576, args.timeout),
        "allocation-int-channel": gecode_smoke(args.allocation_binary,
            ["register","int-channel","3","1"], 576, args.timeout),
    }
    failures = [r for r in results
                if r.get("status") in ("wrong-result", "error", "timeout")]
    failures += [r for r in gecode_results if r.get("status") not in ("ok", "missing")]
    failures += [r for r in controls.values()
                 if r.get("status") not in ("ok", "missing")]
    artifact = {"schema_version": 1, "comparison": "aligned public projections",
                "cases": cases,
                "solver_inventory": infos, "independent": independent,
                "solver_results": results,
                "gecode_semantic_results": gecode_results,
                "gecode_native_branching_controls": controls,
                "validation": {"status": "pass" if not failures else "fail",
                               "failures": failures}}
    rendered = json.dumps(artifact, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered)
    print(rendered, end="")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
