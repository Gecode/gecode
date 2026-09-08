#!/usr/bin/env python3
"""Focused regressions for Word runner validation and process cleanup."""
import importlib.util
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import time
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

class RunnerTests(unittest.TestCase):
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
                  "projections": [[7]], "batch_iterations": 2}
        self.assertTrue(campaign.semantic_native(json.dumps(result), expected, 2)[0])
        self.assertFalse(campaign.semantic_native("[]", expected, 2)[0])
        result["projections"] = None
        self.assertFalse(campaign.semantic_native(json.dumps(result), expected, 2)[0])
        result["projections"] = [[True]]
        self.assertFalse(campaign.semantic_native(json.dumps(result), expected, 2)[0])

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
    unittest.main()
