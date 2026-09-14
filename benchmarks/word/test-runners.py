#!/usr/bin/env python3
"""Focused regressions for Word runner validation and process cleanup."""
import importlib.util
import json
import os
from pathlib import Path
import signal
import selectors
import subprocess
import sys
import time
import tempfile
from types import SimpleNamespace
import unittest
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

def imported(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module

campaign = imported("word_campaign_test", HERE / "comparison-campaign.py")
benchmark = imported("word_benchmark_test", HERE / "benchmark.py")

SMT_SCENARIOS = {
    "partial": "timeout", "multiline": "timeout", "truncated": "error",
    "partial-eof": "error", "malformed": "error", "split": "ok",
    "wrong-projection": "wrong-result", "unsat": "ok", "input-stall": "timeout",
    "request-stall": "timeout", "blocking-stall": "timeout", "stderr": "ok",
    "cumulative": "timeout", "setup-error": "error", "unsat-eof": "ok",
    "write-eof": "error",
}

# The child speaks just enough SMT-LIB to exercise the adapters' public seam.
SOLVER_PROGRAM = r"""
import os, sys, time
mode = sys.argv[1]
if mode == "input-stall":
    time.sleep(60)
if mode == "write-eof":
    os.close(1)
    time.sleep(60)
if mode == "stderr":
    for _ in range(128):
        os.write(2, b"diagnostic" * 8192)
checks = 0
for line in sys.stdin:
    if line.startswith("(check-sat)"):
        checks += 1
        if mode == "partial":
            os.write(1, b"sa")
            time.sleep(60)
        if mode == "partial-eof":
            os.write(1, b"sa")
            break
        if mode == "malformed":
            print("unexpected", flush=True)
            break
        if mode == "unsat-eof":
            os.write(1, b"unsat")
            break
        if mode == "unsat" or checks > 1:
            os.write(1, b"un")
            time.sleep(0.02)
            os.write(1, b"sat\n")
            continue
        if mode == "cumulative":
            time.sleep(0.2)
        os.write(1, b"sa")
        time.sleep(0.02)
        os.write(1, b"t\n")
        if mode == "request-stall":
            time.sleep(60)
    elif line.startswith("(get-value"):
        os.write(1, b"(\n")
        if mode == "multiline":
            time.sleep(60)
        if mode == "truncated":
            break
        if mode == "cumulative":
            time.sleep(0.2)
        time.sleep(0.02)
        os.write(1, b"(x #x")
        time.sleep(0.02)
        os.write(1, b"07)\n)\n")
        if mode == "blocking-stall":
            time.sleep(60)
"""


def run_smt_probe(adapter: str, scenario: str) -> dict:
    """Run one exchange in an outer-capped process and inspect owned resources."""
    module = campaign.MIXED if adapter == "mixed" else campaign.BITS
    case = {"id": "transport-test", "decision_variables": ["x"],
            "family": "mult", "parameters": {"width": 8}}
    info = {"status": "ready", "command": sys.executable,
            "options": ["-u", "-c", SOLVER_PROGRAM, scenario]}
    body = [";" + "x" * (2 * 1024 * 1024)] if scenario in ("input-stall", "write-eof") else []
    name = "x" * (2 * 1024 * 1024) if scenario == "request-stall" else "x"
    names = [name] if adapter == "mixed" else [(name, 8)]
    expected = [] if scenario in ("unsat", "unsat-eof") else [[8 if scenario == "wrong-projection" else 7]]
    timeout = 0.35 if SMT_SCENARIOS[scenario] == "timeout" else 1.5
    processes, selectors_owned = [], []
    spawn, create_selector = subprocess.Popen, selectors.DefaultSelector

    def record_process(*args, **kwargs):
        process = spawn(*args, **kwargs)
        processes.append(process)
        return process

    def record_selector():
        selector = create_selector()
        selectors_owned.append(selector)
        if scenario == "setup-error":
            selector.register = mock.Mock(side_effect=OSError("test setup failure"))
        return selector

    with mock.patch.object(module, "smt_case" if adapter == "mixed" else "smt",
                           return_value=(body, names)), \
         mock.patch.object(subprocess, "Popen", side_effect=record_process), \
         mock.patch("selectors.DefaultSelector", side_effect=record_selector):
        if scenario == "blocking-stall":
            with mock.patch.object(module, "bv", return_value="x" * (2 * 1024 * 1024)):
                row = (module.run_solver(info, case, timeout, expected) if adapter == "mixed"
                       else module.run_solver(info, case, expected, timeout))
        else:
            row = (module.run_solver(info, case, timeout, expected) if adapter == "mixed"
                   else module.run_solver(info, case, expected, timeout))
    was_reaped = True
    for process in processes:
        try:
            os.waitpid(process.pid, os.WNOHANG)
            was_reaped = False
        except ChildProcessError:
            pass
    return {"row": row, "timeout": timeout, "was_reaped": was_reaped,
            "returncodes": [process.returncode for process in processes],
            "pipes_closed": all(pipe.closed for process in processes
                                for pipe in (process.stdin, process.stdout, process.stderr)
                                if pipe is not None),
            "selectors_closed": all(selector.get_map() is None for selector in selectors_owned)}


class RunnerTests(unittest.TestCase):
    def test_gecode_wrong_projection_overrules_child_status(self):
        case = {"id": "collision", "decision_variables": ["x"]}
        child = {"case": "child", "status": "ok", "decision_variables": ["x"],
                 "projections": [[999]], "root": {"diagnostic": True}}
        with mock.patch.object(subprocess, "run", return_value=mock.Mock(returncode=0, stdout=json.dumps(child))):
            row = campaign.BITS.run_gecode(Path("child"), case, [[90]], 1)
        self.assertEqual(row["case"], "collision")
        self.assertEqual(row["status"], "wrong-result")
        self.assertEqual(row["projections"], [[999]])
        self.assertEqual(row["root"], {"diagnostic": True})

    def test_gecode_correct_projection_ignores_conflicting_child_fields(self):
        case = {"id": "collision", "decision_variables": ["x"]}
        child = {"case": "child", "status": "wrong-result", "decision_variables": ["x"],
                 "projections": [[90]], "diagnostic": "kept"}
        with mock.patch.object(subprocess, "run", return_value=mock.Mock(returncode=0, stdout=json.dumps(child))):
            row = campaign.BITS.run_gecode(Path("child"), case, [[90]], 1)
        self.assertEqual(row["case"], "collision")
        self.assertEqual(row["status"], "ok")
        self.assertEqual(row["projections"], [[90]])
        self.assertEqual(row["diagnostic"], "kept")

    @unittest.skipUnless(os.name == "posix", "POSIX pipe readiness and process groups")
    def test_interactive_smt_deadlines(self) -> None:
        for adapter in ("mixed", "bits"):
            for scenario, status in SMT_SCENARIOS.items():
                with self.subTest(adapter=adapter, scenario=scenario):
                    result = self.run_capped_probe(adapter, scenario)
                    row = result["row"]
                    self.assertEqual(row["status"], status, row)
                    self.assertLess(row["elapsed_seconds"], result["timeout"] + 1.25)
                    if scenario in ("truncated", "partial-eof", "malformed", "setup-error", "write-eof"):
                        self.assertLess(row["elapsed_seconds"], result["timeout"])
                    if status in ("ok", "wrong-result"):
                        expected = [] if scenario in ("unsat", "unsat-eof") else [[7]]
                        self.assertEqual(row["projections"], expected)
                        self.assertEqual(row["solutions"], len(expected))
                        self.assertEqual(row["semantic_status"], "sat" if expected else "unsat")
                    self.assertTrue(result["was_reaped"], result)
                    self.assertTrue(result["pipes_closed"], result)
                    self.assertTrue(result["selectors_closed"], result)
                    self.assertEqual(len(result["returncodes"]), 1, result)
                    self.assertIsNotNone(result["returncodes"][0], result)

    def run_capped_probe(self, adapter: str, scenario: str) -> dict:
        command = [sys.executable, str(Path(__file__).resolve()), "--smt-probe",
                   adapter, scenario]
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   text=True, start_new_session=True,
                                   env={**os.environ, "PYTHONDONTWRITEBYTECODE": "1"})
        try:
            stdout, stderr = process.communicate(timeout=4)
        except subprocess.TimeoutExpired:
            self.fail("interactive SMT exchange exceeded the 4-second outer cap")
        finally:
            # Kill the probe's group even if its parent exited before a solver did.
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            process.communicate(timeout=2)
            process.stdout.close()
            process.stderr.close()
        self.assertEqual(process.returncode, 0, stderr)
        return json.loads(stdout)

    def test_batch_validation(self):
        expected = {"status": "sat", "solutions": 1, "projections": [[7]]}
        result = {"semantic_status": "sat", "solutions": 1,
                  "projections": [[7]], "batch": 2, "batch_solutions": 2}
        self.assertTrue(campaign.semantic_native(json.dumps(result), expected, 2)[0])
        result["batch_solutions"] = 1
        self.assertFalse(campaign.semantic_native(json.dumps(result), expected, 2)[0])
        result = {"semantic_status": "sat", "solutions": 2,
                  "projections": [[7]], "iterations": 2}
        self.assertTrue(campaign.semantic_native(json.dumps(result), expected, 2)[0])
        result["iterations"] = 1
        self.assertFalse(campaign.semantic_native(json.dumps(result), expected, 2)[0])
        result = {"semantic_status": "sat", "solutions": 1,
                  "projections": [[7]], "measurement": "search-batch",
                  "batch_requested": 2, "batch_iterations": 2}
        self.assertTrue(campaign.semantic_native(json.dumps(result), expected, 2)[0])
        self.assertFalse(campaign.semantic_native("[]", expected, 2)[0])
        result["projections"] = None
        self.assertFalse(campaign.semantic_native(json.dumps(result), expected, 2)[0])
        result["projections"] = [[True]]
        self.assertFalse(campaign.semantic_native(json.dumps(result), expected, 2)[0])

    def test_dma_batch_validation(self):
        expected = {"status": "unsat", "solutions": 0, "projections": []}
        root = {"semantic_status": "unsat", "solutions": 0, "projections": [],
                "measurement": "root-only", "batch_requested": 16,
                "batch_iterations": 0, "nodes": 0, "failures": 0, "checksum": 0}
        self.assertTrue(campaign.semantic_native(json.dumps(root), expected, 16)[0])
        for field, value in (("batch_requested", 2), ("batch_iterations", 16),
                             ("batch_iterations", False), ("nodes", 1),
                             ("failures", 1), ("solutions", 1), ("checksum", 1),
                             ("measurement", "search-batch")):
            with self.subTest(field=field, value=value):
                self.assertFalse(campaign.semantic_native(
                    json.dumps({**root, field: value}), expected, 16)[0])
        searched = {**root, "measurement": "search-batch", "batch_iterations": 16}
        self.assertTrue(campaign.semantic_native(json.dumps(searched), expected, 16)[0])
        for field, value in (("batch_iterations", 0), ("batch_iterations", 2),
                             ("batch_requested", 2), ("measurement", "unknown")):
            with self.subTest(field=field, value=value):
                self.assertFalse(campaign.semantic_native(
                    json.dumps({**searched, field: value}), expected, 16)[0])
        del searched["measurement"]
        self.assertFalse(campaign.semantic_native(json.dumps(searched), expected, 16)[0])

    def test_root_only_requires_dma_contract(self):
        expected = {"status": "sat", "solutions": 1, "projections": [[7]]}
        for counts in ({"batch": 2, "batch_solutions": 2, "solutions": 1},
                       {"iterations": 2, "solutions": 2}):
            result = {"semantic_status": "sat", "projections": [[7]], **counts}
            self.assertTrue(campaign.semantic_native(json.dumps(result), expected, 2)[0])
            result["measurement"] = "root-only"
            self.assertFalse(campaign.semantic_native(json.dumps(result), expected, 2)[0])

    def test_dma_normalization(self):
        case = {"id": "dma-test", "campaign_family": "dma", "level": "small"}
        for kind, status, rows in (("root-only", "unsat", []),
                                   ("search-batch", "sat", [[7]]),
                                   ("search-batch", "unsat", [])):
            expected = {"status": status, "solutions": len(rows), "projections": rows}
            for batch in (1, 2, 16):
                with self.subTest(kind=kind, status=status, batch=batch), tempfile.TemporaryDirectory() as directory:
                    args = SimpleNamespace(root=Path(directory), cpu_budget=100)
                    native = {"semantic_status": status, "solutions": len(rows),
                              "projections": rows, "measurement": kind,
                              "batch_requested": batch,
                              "batch_iterations": 0 if kind == "root-only" else batch,
                              "nodes": 0, "failures": 0, "checksum": 0}
                    measured = {"status": "measured", "stdout": json.dumps(native),
                                "cpu_seconds": 1.0}
                    with mock.patch.object(campaign, "native", return_value=[]), \
                         mock.patch.object(campaign, "local_run", return_value=measured):
                        result = campaign.execute(args, case, "gecode-baseline", expected,
                                                  "screen", 0, 30, batch)
                    self.assertEqual(result["status"], "measured")
                    denominator = 1 if kind == "root-only" else batch
                    self.assertEqual(result["normalization_iterations"], denominator)
                    self.assertEqual(result["seconds_per_problem"], 1.0 / denominator)

    def test_dma_adaptive_batching(self):
        case = {"id": "dma-test", "campaign_family": "dma", "level": "small"}
        expected = {"status": "unsat", "solutions": 0, "projections": []}
        for kind, batches in (("root-only", [1]), ("search-batch", [1, 2, 4])):
            with self.subTest(kind=kind), tempfile.TemporaryDirectory() as directory:
                args = SimpleNamespace(root=Path(directory), cpu_budget=1000)
                measurements = [
                    {"status": "measured", "cpu_seconds": 0.1 * batch,
                     "stdout": json.dumps({"semantic_status": "unsat", "solutions": 0,
                                           "projections": [], "measurement": kind,
                                           "batch_requested": batch,
                                           "batch_iterations": 0 if kind == "root-only" else batch,
                                           "nodes": 0, "failures": 0, "checksum": 0})}
                    for batch in batches]
                with mock.patch.object(campaign, "native", return_value=[]), \
                     mock.patch.object(campaign, "local_run", side_effect=measurements) as runs:
                    self.assertEqual(campaign.batch_for(args, case, "gecode-baseline", expected),
                                     batches[-1])
                self.assertEqual(runs.call_count, len(batches))
                for path in (args.root / "records").glob("*.json"):
                    self.assertEqual(campaign.load(path)["status"], "measured")

    def test_smt_error_is_not_success(self):
        self.assertFalse(campaign.semantic_smt(
            '(error "unsupported assertion")\nsat\n', {"status": "sat"}, 1)[0])
        self.assertTrue(campaign.semantic_smt(
            "unsat\nunsat\n", {"status": "unsat"}, 2)[0])

    def test_successful_process(self):
        with mock.patch.object(benchmark, "time_wrapper", return_value=None):
            result, elapsed, rss, source = benchmark.timed_command(
                [sys.executable, "-c", "print('ok')"], 5)
        self.assertEqual((result.returncode, result.stdout), (0, "ok\n"))
        self.assertGreater(elapsed, 0)
        self.assertIsNone(rss)
        self.assertEqual(source, "unavailable")

    @unittest.skipUnless(os.name == "posix", "POSIX process groups")
    def test_timeout_kills_wrapper_and_solver(self):
        # A forwarding wrapper stands in for /usr/bin/time.
        wrapper = [sys.executable, "-c",
                   "import subprocess,sys; sys.exit(subprocess.call(sys.argv[1:]))"]
        command = [sys.executable, "-c",
                   "import os,time; print(os.getpid(),flush=True); time.sleep(30)"]
        with mock.patch.object(benchmark, "time_wrapper", return_value=wrapper):
            with self.assertRaises(subprocess.TimeoutExpired) as caught:
                benchmark.timed_command(command, 0.5)
        pid = int(caught.exception.stdout.strip())
        def alive():
            # A killed, not-yet-reaped orphan is harmless; check process state.
            row = subprocess.run(["ps", "-o", "stat=", "-p", str(pid)],
                                 capture_output=True, text=True).stdout.strip()
            return bool(row) and not row.startswith("Z")
        try:
            deadline = time.monotonic() + 2
            while alive() and time.monotonic() < deadline:
                time.sleep(0.01)
            self.assertFalse(alive(), "timed-out solver survived its wrapper")
        finally:
            if alive():
                os.kill(pid, signal.SIGKILL)

if __name__ == "__main__":
    if len(sys.argv) == 4 and sys.argv[1] == "--smt-probe":
        print(json.dumps(run_smt_probe(sys.argv[2], sys.argv[3])))
    else:
        unittest.main()
