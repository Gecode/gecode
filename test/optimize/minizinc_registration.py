#!/usr/bin/env python3
"""Pinned MiniZinc-to-driver compatibility gate; no download, build, or benchmarks.

Requires an actual MiniZinc 2.10.1 compiler with its matching standard library and
an actual native-enabled driver. Each required child and the suite have deadlines.
"""
from __future__ import annotations
import argparse
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

ROOT = Path(__file__).resolve().parents[2]
IDENTITY = "org.gecode.optimize.experimental"
# Exact complete-predicate admission, deliberately independent of registry aliases.
PRIMITIVES = set("int_eq int_le int_lt int_ge int_gt int_plus int_minus int_lin_eq int_lin_le bool_eq bool_le bool_not bool_and bool_or array_bool_and array_bool_or bool_clause bool_lin_eq bool_lin_le bool2int int_in int_le_reif int_le_imp int_eq_imp int_lin_le_reif int_lin_le_imp array_int_element array_var_int_element all_different_int gecode_table_int gecode_regular gecode_circuit gecode_cumulatives cumulatives".split())


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


class Suite:
    def __init__(self, args, directory):
        self.args, self.directory = args, directory
        self.deadline = time.monotonic() + args.timeout
        self.checks, self.sources, self.native_results = [], {}, []
        self.msc = args.registration or directory / "gecode-optimize.msc"
        if args.registration is None:
            config = json.loads((ROOT / "tools/flatzinc/gecode-optimize.msc.in").read_text())
            config["version"] = "6.4.0-experimental-test"
            config["mznlib"] = str((ROOT / "tools/flatzinc/mznlib-optimize").resolve())
            config["executable"] = [str(args.binary), "--minizinc"]
            self.msc.write_text(json.dumps(config, indent=2) + "\n")
        # Read the actual artifact in place, preserving all relative paths.
        self.configuration_bytes = self.msc.read_bytes()
        config = json.loads(self.configuration_bytes)
        require(config.get("id") == IDENTITY, "Wrong experimental solver identity")
        executable = config.get("executable")
        require(isinstance(executable,list) and len(executable) == 2 and
                isinstance(executable[0],str) and executable[1] == "--minizinc",
                "Registration must invoke exactly the driver and --minizinc")
        def relative_path(value):
            require(isinstance(value,str) and value, "Registration path must be a nonempty string")
            path = Path(value)
            return (path if path.is_absolute() else self.msc.parent/path).resolve()
        executable_path = relative_path(executable[0])
        require(executable_path.is_file() and executable_path.samefile(args.binary),
                "Registration executable does not resolve to --binary")
        require(isinstance(config.get("mznlib"),str) and not config["mznlib"].startswith("-G"),
                "Registration must name its dedicated library directory, not a -G alias")
        self.library = relative_path(config["mznlib"])
        require(self.library.is_dir(), "Registration library directory does not exist")
        self.library_hashes = self.hash_library()
        require(self.library_hashes, "Registration library contains no .mzn files")
        # Only this process's environment changes; no system/user solver prefs.
        self.old_solver_path = os.environ.get("MZN_SOLVER_PATH")
        os.environ["MZN_SOLVER_PATH"] = str(self.msc.parent)

    def hash_library(self):
        return {str(path.relative_to(self.library)).replace(os.sep,"/"):digest(path)
                for path in sorted(self.library.rglob("*.mzn"))}

    def close(self):
        if self.old_solver_path is None:
            os.environ.pop("MZN_SOLVER_PATH", None)
        else:
            os.environ["MZN_SOLVER_PATH"] = self.old_solver_path

    def run(self, label, command, *, cwd=None):
        remaining = self.deadline - time.monotonic()
        require(remaining > 1, f"Aggregate deadline before {label}")
        start = time.monotonic()
        with tempfile.TemporaryFile() as out, tempfile.TemporaryFile() as err:
            process = None
            try:
                if os.name == "nt":
                    from process_containment import WindowsJobProcess
                    process = WindowsJobProcess()
                    process.start([str(x) for x in command], out, err, str(cwd or self.directory))
                else:
                    process = subprocess.Popen([str(x) for x in command], cwd=cwd or self.directory,
                                               stdin=subprocess.DEVNULL, stdout=out, stderr=err,
                                               start_new_session=True)
                code = process.wait(timeout=min(10, remaining - 1))
            finally:
                if process is not None:
                    if os.name == "nt":
                        process.cleanup(timeout=1)
                    else:
                        # MiniZinc starts the driver; terminate the whole group,
                        # even if MiniZinc exited while leaving a child behind.
                        try:
                            os.killpg(process.pid, signal.SIGKILL)
                        except ProcessLookupError:
                            pass
                        process.wait(timeout=1)
            require(out.tell() <= 1048576 and err.tell() <= 1048576, f"Excessive output: {label}")
            out.seek(0); err.seek(0)
            stdout, stderr = out.read().decode(), err.read().decode()
        require(time.monotonic() < self.deadline, f"Aggregate deadline after {label}")
        self.checks.append({"case": label, "returncode": code, "elapsed_seconds": time.monotonic()-start})
        return code, stdout, stderr

    def mzn(self, label, *options):
        return self.run(label, [self.args.minizinc, "--solver", self.msc, *options])

    def fixture(self, name):
        path = ROOT / "test/optimize/minizinc-fixtures" / ("mzn-"+name+".mzn")
        require(path.is_file(), f"Missing fixture: {path}")
        self.sources[path.name] = digest(path)
        return path

    def positive(self, name, allowed, *, mode="optimal", required=None):
        source = self.fixture(name)
        fzn = self.directory / (name+".fzn")
        code, stdout, stderr = self.mzn("compile-"+name, "--compile", "--output-fzn-to-file", fzn,
                                      "--output-ozn-to-file", self.directory/(name+".ozn"), source)
        require(code == 0 and not stderr, f"Compile {name}: {code} {stdout!r} {stderr!r}")
        text = fzn.read_text()
        emitted = set(re.findall(r"\bconstraint\s+(\w+)\s*\(", text))
        require(emitted <= PRIMITIVES, f"Unadmitted predicate lowering in {name}: {emitted-PRIMITIVES}")
        if required:
            require(required in emitted, f"Missing native lowering {required} in {name}")
        code, stdout, stderr = self.mzn("solve-"+name, source)
        require(code == 0 and not stderr, f"Solve {name}: {code} {stdout!r} {stderr!r}")
        lines = [line for line in stdout.splitlines() if line and not line.startswith("%")]
        if mode == "unsat":
            require(lines == ["=====UNSATISFIABLE====="], f"False completion in {name}: {lines}")
        else:
            markers = ["----------", "=========="] if mode == "optimal" else ["----------"]
            require(lines[1:] == markers, f"Wrong markers in {name}: {lines}")
            require(tuple(json.loads(lines[0])) in allowed, f"Original-model oracle failed in {name}: {lines[0]}")
            require("% guarantee: exact integer search" in stdout, f"Wrong backend provenance: {name}")

    def rejected(self, name, contains):
        code, stdout, stderr = self.mzn("reject-"+name, self.fixture(name))
        require(code != 0 and contains in stderr, f"Missing rejection {name}: {code} {stdout!r} {stderr!r}")
        require("----------" not in stdout and "=====UNSATISFIABLE=====" not in stdout,
                f"Rejected model published a witness/proof: {name}")


def regular(word, states, alphabet, transitions, initial, finals):
    for symbol in word:
        if not 1 <= symbol <= alphabet or not 1 <= initial <= states:
            return False
        initial = transitions[(initial-1)*alphabet+symbol-1]
        if initial == 0:
            return False
    return initial in finals


def circuit(values, offset):
    visited, node = set(), offset
    for _ in values:
        if node in visited or not offset <= node < offset+len(values):
            return False
        visited.add(node)
        node = values[node-offset]
    return node == offset and len(visited) == len(values)


def cumulative(starts, durations, heights, capacity):
    # Independent half-open integer-time oracle for these tiny source models.
    return all(sum(h for s,d,h in zip(starts,durations,heights) if s <= t < s+d) <= capacity
               for t in range(-2, 6))


def tests(s):
    code, stdout, stderr = s.run("compiler-version", [s.args.minizinc, "--version"])
    require(code == 0 and "version 2.10.1," in stdout and not stderr, "Required pinned MiniZinc 2.10.1 unavailable")
    code, stdout, stderr = s.run("solver-discovery", [s.args.minizinc, "--solvers-json"])
    require(code == 0 and not stderr, "Solver discovery failed")
    # Another installed build may advertise the same solver identity. Validate
    # discovery of this exact artifact, which every solve selects explicitly.
    configs = [x for x in json.loads(stdout) if x["id"] == IDENTITY and
               Path(x["extraInfo"]["configFile"]).resolve() == s.msc.resolve()]
    require(len(configs) == 1, "Configured solver artifact absent or duplicated")
    require(configs[0]["stdFlags"] == ["-t"] and configs[0]["tags"] == ["cp","int","experimental"],
            "Registration overstates supported flags/types")
    require("default" not in configs[0]["tags"], "Experimental registration changed the default")
    advertised = {flag[0]: flag for flag in configs[0]["extraFlags"]}
    expected = set("mode auto-presolve auto-components auto-symmetry auto-knapsack race-seconds race-nodes lp root-cuts bound-tightening lp-interval search branching branching-probes max-open-nodes neighborhood neighborhood-radius neighborhood-nodes neighborhood-seconds node-limit diagnostics".split())
    require(set(advertised) == {"--native-"+name for name in expected}, "Missing or unexpected native controls")
    require(advertised["--native-mode"][2:] == ["opt:auto:race:plain:configured", "auto"], "Wrong mode contract")
    require("CPU" in advertised["--native-race-seconds"][1] and "solve time" in advertised["--native-race-seconds"][1],
            "Registration must disclose race overhead")
    code, stdout, stderr = s.run("solver-native-help", [s.args.minizinc, "--help", s.msc])
    require(code == 0 and not stderr and all(flag in stdout for flag in advertised), "Native controls absent from solver help")
    a = [(x,y,2*x+y-4) for x,y in itertools.product(range(4),repeat=2) if x+y >= 3]
    s.positive("linear-min", {min(a,key=lambda p:p[2])})
    a = [(x,y,3*x-y+2) for x,y in itertools.product(range(-2,3),repeat=2) if x+y <= 1]
    s.positive("linear-max", {max(a,key=lambda p:p[2])})
    s.positive("satisfy", {(1,),(2,)}, mode="satisfy")
    s.positive("unsat", set(), mode="unsat", required="all_different_int")
    s.positive("alias-holes", {(3,3)})
    s.positive("all-different", {(1,2)}, required="all_different_int")
    s.positive("table", {(1,2)}, required="gecode_table_int")
    s.positive("table-alias", {(1,1)}, required="gecode_table_int")
    words = [x for x in itertools.product((1,2),repeat=2) if regular(x,3,2,[2,3,0,3,2,0],1,{3})]
    s.positive("regular", {min(words,key=lambda x:x[0])}, required="gecode_regular")
    words = [(x,x,x) for x in (1,2) if regular((x,x,x),2,2,[2,1,1,2],1,{1})]
    s.positive("regular-alias", set(words), required="gecode_regular")
    s.positive("regular-empty", {()}, mode="satisfy")
    s.positive("regular-dead", set(), mode="unsat", required="gecode_regular")
    for name,offset in (("negative",-2),("offset",2)):
        values = [x for x in itertools.product(range(offset,offset+3),repeat=3) if circuit(x,offset)]
        s.positive("circuit-"+name, {min(values,key=lambda x:x[0])}, required="gecode_circuit")
    s.positive("element-offset", {(0,2)}, required="array_int_element")
    starts = [(x,1) for x in range(3) if cumulative([x,1,1],[1,1,1],[1,1,1],2)]
    s.positive("cumulative-half-open", {min(starts)}, required="gecode_cumulatives")
    s.positive("cumulative-zero", {(0,)}, required="gecode_cumulatives")
    s.positive("cumulative-unsat", set(), mode="unsat", required="gecode_cumulatives")
    s.positive("cumulative-fixed-alias", {(0,1,1)}, required="gecode_cumulatives")
    s.positive("reified-le", {(1,1)})
    accepted = {x for x in itertools.product((1,2),repeat=2) if regular(x,1,2,[1,0],1,{1})}
    s.positive("regular-decomposed-reif", accepted, required="gecode_regular")
    rejected = [x for x in itertools.product((1,2),repeat=2) if not regular(x,1,2,[1,0],1,{1})]
    s.positive("regular-complement", {min(rejected,key=lambda x:2*x[0]+x[1])}, required="gecode_regular")
    for name,text in (("times","multiplication is unsupported"),("reif-eq","equality is unsupported"),
                      ("float","float and set variables"),("set","float and set variables"),("search","search annotations"),
                      ("variable-cumulative","original singleton")):
        s.rejected("reject-"+name,text)
    native_controls(s)
    # Test flags through the actual MiniZinc parser, not only the .msc JSON.
    for flag in ("--all-solutions","--intermediate-solutions"):
        args = [flag]
        code, stdout, stderr = s.mzn("unadvertised-"+flag, *args, s.fixture("satisfy"))
        require(code != 0 and "Unrecognized option" in stderr, f"Unsupported standard flag accepted: {flag}")
    # MiniZinc owns compiler/output statistics and may consume --parallel even
    # when neither solver flag is advertised. Force forwarding to test the
    # driver's strict boundary without misrepresenting the outer compiler.
    for flag in ("-s", "-p"):
        code, stdout, stderr = s.mzn("forwarded-"+flag,"--fzn-flag",flag,s.fixture("satisfy"))
        require(code != 0 and "Unsupported MiniZinc protocol option" in stderr,
                f"Unsupported forwarded solver flag accepted: {flag}")
    for milliseconds in ("0","1000"):
        code, stdout, stderr = s.mzn("time-"+milliseconds,"--solver-time-limit",milliseconds,s.fixture("satisfy"))
        require(code == 0 and "----------" in stdout and not stderr, "MiniZinc time convention mismatch")
    # Filename ordering and zero semantics are isolated from direct CLI behavior.
    fzn = s.directory/"protocol.fzn"
    fzn.write_text("var 0..2: x :: output_var; solve minimize x;\n")
    for args in (("-t","0",str(fzn)),(str(fzn),"-t","0"),("--",str(fzn))):
        code, stdout, stderr = s.run("protocol-order",[s.args.binary,"--minizinc",*args])
        require(code == 0 and "x = 0;" in stdout and "==========" in stdout and not stderr, "Protocol option order/zero failed")
    for args in (("-t",),("-t","-1",str(fzn)),("-t","1.5",str(fzn)),("-t","18446744073709551616",str(fzn)),
                 ("-t","0","-t","1",str(fzn)),(str(fzn),str(fzn)),("--backend","highs",str(fzn)),("-a",str(fzn))):
        code, stdout, stderr = s.run("protocol-rejection",[s.args.binary,"--minizinc",*args])
        require(code == 2 and not stdout and stderr, f"Malformed protocol accepted: {args}")
    code, stdout, stderr = s.run("direct-zero",[s.args.binary,fzn,"--time-limit","0"])
    require(code == 1 and "=====UNKNOWN=====" in stdout, "Direct zero deadline changed")
    # A finite parser workload (no CP benchmark) makes a 1ms budget expire in
    # capture on supported CI machines, verifying UNKNOWN is normal exit0.
    large = s.directory/"capture-deadline.fzn"
    large.write_text("".join(f"var 0..1: x{i};\n" for i in range(20000))+"solve satisfy;\n")
    for prefix in ([s.args.binary,"--minizinc","-t","1"],
                   [s.args.minizinc,"--solver",s.msc,"--solver-time-limit","1"],
                   [s.args.minizinc,"--solver",s.msc,"--native-mode","race","--native-race-seconds","5","--solver-time-limit","1"],
                   [s.args.minizinc,"--solver",s.msc,"--native-mode","configured","--native-branching","reliability","--native-neighborhood","hamming","--solver-time-limit","1"]):
        code, stdout, stderr = s.run("actual-timeout",[*prefix,large])
        require(code == 0 and "=====UNKNOWN=====" in stdout and "=====ERROR=====" not in stdout,
                f"Timeout became solver error: {code} {stdout!r} {stderr!r}")


def native_controls(s):
    # Independent original-model oracle, including all optimal ties. No native
    # result or flattened model is used to calculate this answer.
    weights, profits = (3,3,2,2,4,1), (5,4,3,3,6,1)
    feasible = {x+(sum(p*v for p,v in zip(profits,x)),) for x in itertools.product((0,1), repeat=6)
                if sum(w*v for w,v in zip(weights,x)) <= 8 and 3*x[0]+3*x[1] <= 5 and sum(x[2:5]) <= 2}
    optimum = max(x[-1] for x in feasible)
    optimal = {x for x in feasible if x[-1] == optimum}
    source = s.fixture("native-controls")
    s.positive("native-controls", optimal, required="bool2int")

    def invoke(label, mode="auto", *flags, source=source):
        return s.mzn("native-"+label, "--native-mode", mode, "--native-diagnostics", "on", *flags, source)

    def completed(label, mode="auto", *flags, source=source, allowed=optimal, configuration=(), backend=None, policy=None):
        code, stdout, stderr = invoke(label, mode, *flags, source=source)
        require(code == 0 and not stderr, f"Native {label}: {code} {stdout!r} {stderr!r}")
        lines = [line for line in stdout.splitlines() if line and not line.startswith("%")]
        require(len(lines) == 3 and lines[1:] == ["----------", "=========="], f"Native markers {label}: {lines}")
        require(tuple(json.loads(lines[0])) in allowed, f"Original oracle failed for {label}: {lines[0]}")
        require("% guarantee: exact integer search" in stdout and f"% native-mode: {mode}" in stdout,
                f"Native route provenance absent for {label}: {stdout}")
        config = next((line for line in stdout.splitlines() if line.startswith("% native-configuration: ")), "")
        require(config and all(item in config.split() for item in configuration), f"Settings not forwarded for {label}: {config}")
        if backend:
            require(f"% native-backend: {backend}" in stdout, f"Wrong actual backend for {label}: {stdout}")
        if policy:
            require(any(policy in line for line in stdout.splitlines() if line.startswith("% native-policy: ")),
                    f"Wrong actual policy for {label}: {stdout}")
        work = next((line for line in stdout.splitlines() if line.startswith("% native-work: ")), "")
        counters = {key:int(value) for key,value in re.findall(r"([a-z-]+)=(\d+)", work)}
        s.native_results.append({"case": label, "mode": mode, "solution": json.loads(lines[0]),
                                 "configuration": config[len("% native-configuration: "):], "work": counters,
                                 "policy": next(line[len("% native-policy: "):] for line in stdout.splitlines() if line.startswith("% native-policy: ")),
                                 "backend": next(line[len("% native-backend: "):] for line in stdout.splitlines() if line.startswith("% native-backend: "))})
        return counters, stdout

    completed("auto")
    knapsack_points = {x+(sum(p*v for p,v in zip(profits,x)),) for x in itertools.product((0,1), repeat=6)
                       if sum(w*v for w,v in zip(weights,x)) <= 8}
    knapsack_best = max(x[-1] for x in knapsack_points)
    knapsack_optimal = {x for x in knapsack_points if x[-1] == knapsack_best}
    knapsack = s.fixture("native-knapsack")
    completed("knapsack-auto", source=knapsack, allowed=knapsack_optimal, policy="eligible exact knapsack DP")
    _, stdout = completed("knapsack-disabled", "auto", "--native-auto-knapsack", "off", source=knapsack,
                          allowed=knapsack_optimal, configuration=("auto-knapsack=off",))
    require("eligible exact knapsack DP" not in stdout, "Disabled knapsack DP was still selected")
    for name in ("presolve", "components", "symmetry", "knapsack"):
        completed("auto-no-"+name, "auto", "--native-auto-"+name, "off", configuration=("auto-"+name+"=off",))
    disabled = [item for name in ("presolve", "components", "symmetry", "knapsack")
                for item in ("--native-auto-"+name, "off")]
    completed("auto-all-disabled", "auto", *disabled, configuration=tuple("auto-"+name+"=off" for name in ("presolve", "components", "symmetry", "knapsack")))
    completed("plain", "plain", backend="Gecode native", configuration=("ordinary-native",))
    completed("uint64-limit", "plain", "--native-node-limit", "18446744073709551615", configuration=("node-limit=18446744073709551615",))
    completed("race", "race", "--native-race-seconds", "0.05", "--native-race-nodes", "4",
              configuration=("race-seconds=0.05", "race-nodes=4"), policy="Native sequential race:")
    completed("race-zero", "race", "--native-race-seconds", "0", configuration=("race-seconds=0",), policy="skipped")
    completed("race-global", "race", source=s.fixture("all-different"), allowed={(1,2)}, policy="skipped")
    for order in ("bab", "dfs", "best-bound"):
        completed("search-"+order, "configured", "--native-search", order,
                  configuration=("search="+order,), backend="Gecode native" if order == "bab" else "Gecode native frontier")
    work, _ = completed("reliability", "configured", "--native-branching", "reliability",
                        "--native-branching-probes", "32", "--native-max-open-nodes", "128",
                        configuration=("branching=reliability", "branching-probes=32", "max-open-nodes=128"))
    require(0 < work.get("branching-probes",0) <= 32, "Reliability accepted without executing its probes")
    work, _ = completed("reliability-zero", "configured", "--native-branching", "reliability", "--native-branching-probes", "0")
    require(work.get("branching-probes") == 0, "Zero reliability probe cap ignored")
    work, _ = completed("hamming", "configured", "--native-neighborhood", "hamming", "--native-neighborhood-radius", "1",
                        "--native-neighborhood-nodes", "32", "--native-neighborhood-seconds", "0.1",
                        configuration=("neighborhood=hamming", "neighborhood-radius=1", "neighborhood-nodes=32", "neighborhood-seconds=0.1"))
    require(work.get("neighborhood-attempts",0) > 0, "Hamming accepted without executing an attempt")
    for cap in ("nodes", "seconds"):
        work, _ = completed("hamming-zero-"+cap, "configured", "--native-neighborhood", "hamming", "--native-neighborhood-"+cap, "0")
        require(work.get("neighborhood-attempts") == 0, "Zero neighborhood cap ignored")
    completed("combined-search", "configured", "--native-search", "best-bound", "--native-branching", "reliability", "--native-neighborhood", "hamming")

    # This gate also runs in native-only builds. Discover unavailable checked LP
    # through its explicit rejection, never through a successful silent fallback.
    code, stdout, stderr = invoke("lp-capability", "configured", "--native-search", "bab", "--native-lp", "root")
    checked_lp = code == 0
    if not checked_lp:
        require(code != 0 and "checked LP requires HiGHS" in stderr and "----------" not in stdout,
                f"Unexpected checked LP failure: {code} {stdout!r} {stderr!r}")
    lp_cases = [
        ("lp-root", ("--native-search", "bab", "--native-lp", "root")),
        ("lp-root-covers", ("--native-lp", "root", "--native-root-cuts", "on")),
        ("lp-updated", ("--native-search", "best-bound", "--native-lp", "updated", "--native-lp-interval", "2", "--native-bound-tightening", "off")),
        ("lp-combined", ("--native-lp", "updated", "--native-root-cuts", "on", "--native-branching", "reliability", "--native-neighborhood", "hamming")),
    ]
    for label, flags in lp_cases:
        if checked_lp:
            work, stdout = completed(label, "configured", *flags,
                                     configuration=tuple(flags[i][len("--native-"):]+"="+flags[i+1] for i in range(0,len(flags),2)))
            if label in ("lp-root-covers", "lp-combined"):
                require(work.get("root-cuts",0) > 0, f"Root covers enabled without generating a verified cut: {label}")
            require(work.get("lp-calls",0) > 0 and work.get("checked-bounds",0) > 0,
                    f"Checked LP accepted without checked deductions: {label} {stdout}")
        else:
            code, stdout, stderr = invoke(label, "configured", *flags)
            require(code != 0 and "checked LP requires HiGHS" in stderr and "----------" not in stdout,
                    f"Explicit LP silently fell back: {label}")

    # Shared finite budgets must never publish a false witness or completion.
    # Both the optional algorithms and the ordinary frontier consume this cap.
    for mode in ("auto", "race", "plain", "configured"):
        for cap in ("0", "1"):
            flags = ["--native-node-limit", cap]
            if mode == "configured":
                flags += ["--native-branching", "reliability", "--native-neighborhood", "hamming"]
            code, stdout, stderr = invoke("node-"+mode+"-"+cap, mode, *flags)
            require(code == 0 and "=====ERROR=====" not in stdout and "=====UNSATISFIABLE=====" not in stdout,
                    f"Node cap became error/false infeasibility: {mode} {cap} {stdout!r} {stderr!r}")
            lines = [line for line in stdout.splitlines() if line and not line.startswith("%")]
            if lines and lines[0].startswith("["):
                witness = tuple(json.loads(lines[0]))
                require(witness in feasible, "Limited run published invalid original-model witness")
                require("==========" not in lines or witness in optimal, "Limited run claimed false optimum")
            else:
                require(lines == ["=====UNKNOWN====="], f"Limited run protocol: {lines}")
            if mode == "configured":
                work_line = next((line for line in stdout.splitlines() if line.startswith("% native-work: ")), "")
                work = {key:int(value) for key,value in re.findall(r"([a-z-]+)=(\d+)", work_line)}
                require("budget-nodes" in work and work["budget-nodes"] <= int(cap), f"Shared node cap exceeded: {work}")
                require(work["budget-nodes"] == work.get("frontier-admitted",0)+work.get("branching-probes",0)+work.get("neighborhood-status-attempts",0),
                        f"Frontier/probe/neighborhood accounting mismatch: {work}")
            if cap == "0":
                require("==========" not in lines, "Zero node budget incorrectly completed nontrivial model")
    code, stdout, stderr = invoke("zero-open-spaces", "configured", "--native-max-open-nodes", "0")
    require(code == 0 and "=====UNKNOWN=====" in stdout and "----------" not in stdout and "==========" not in stdout,
            "Zero frontier space cap ignored or became a solver error")

    # Rejected controls must not be ignored by MiniZinc or the driver. Test both
    # malformed values and meaningful but incompatible combinations.
    rejected = [
        ("--native-mode", "bad"), ("--native-diagnostics", "yes"),
        ("--native-node-limit", "-1"), ("--native-node-limit", "18446744073709551616"),
        ("--native-mode", "race", "--native-race-seconds", "nan"),
        ("--native-mode", "race", "--native-race-nodes", "0"),
        ("--native-mode", "plain", "--native-auto-presolve", "off"),
        ("--native-race-seconds", "0.1"), ("--native-lp", "root"),
        ("--native-mode", "configured", "--native-root-cuts", "on"),
        ("--native-mode", "configured", "--native-bound-tightening", "off"),
        ("--native-mode", "configured", "--native-lp", "root", "--native-lp-interval", "2"),
        ("--native-mode", "configured", "--native-lp", "updated", "--native-lp-interval", "0"),
        ("--native-mode", "configured", "--native-search", "bab", "--native-branching", "reliability"),
        ("--native-mode", "configured", "--native-search", "bab", "--native-max-open-nodes", "1"),
        ("--native-mode", "configured", "--native-search", "bab", "--native-neighborhood", "hamming"),
        ("--native-mode", "configured", "--native-branching-probes", "0"),
        ("--native-mode", "configured", "--native-neighborhood-radius", "1"),
        ("--native-mode", "configured", "--native-neighborhood-nodes", "0"),
        ("--native-mode", "configured", "--native-neighborhood-seconds", "0"),
        ("--native-mode", "configured", "--native-neighborhood", "hamming", "--native-neighborhood-seconds", "-1"),
        ("--native-mode", "configured", "--native-search", "bfs"),
    ]
    for i, flags in enumerate(rejected):
        code, stdout, stderr = s.mzn("native-reject-"+str(i), *flags, source)
        require(code != 0 and stderr and "----------" not in stdout and "==========" not in stdout and "=====UNSATISFIABLE=====" not in stdout,
                f"Invalid controls accepted or emitted proof: {flags} {code} {stdout!r} {stderr!r}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--minizinc", type=Path, required=True)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--registration", type=Path, help="Test this configured .msc in place without rewriting it")
    parser.add_argument("--timeout", type=float, default=120)
    args = parser.parse_args()
    require(math.isfinite(args.timeout) and args.timeout > 0, "Finite positive suite timeout required")
    args.minizinc, args.binary = args.minizinc.resolve(), args.binary.resolve()
    args.registration = args.registration.resolve() if args.registration is not None else None
    require(args.minizinc.is_file() and args.binary.is_file(), "Required executable is missing")
    with tempfile.TemporaryDirectory(prefix="gecode-minizinc-") as tmp:
        suite = Suite(args,Path(tmp))
        try:
            tests(suite)
            require(suite.msc.read_bytes() == suite.configuration_bytes, "Registration artifact changed during the test")
            require(suite.hash_library() == suite.library_hashes, "Registration library changed during the test")
            report = {"status":"passed","compiler_version":"2.10.1","compiler_sha256":digest(args.minizinc),
                      "driver_sha256":digest(args.binary),"cases":len(suite.checks),"checks":suite.checks,
                      "configuration_sha256":hashlib.sha256(suite.configuration_bytes).hexdigest(),
                      "configuration_provided":args.registration is not None,
                      "library_files_sha256":suite.library_hashes,
                      "library_sha256":hashlib.sha256(json.dumps(suite.library_hashes,sort_keys=True,separators=(",",":")).encode()).hexdigest(),
                      "source_sha256":suite.sources,"native_controls":suite.native_results}
            print(json.dumps(report,sort_keys=True))
        finally:
            suite.close()


if __name__ == "__main__":
    main()
