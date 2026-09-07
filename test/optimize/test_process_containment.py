"""Windows ABI/failure mocks plus real Windows containment gates.

Passing mocks on POSIX is NOT evidence that the Windows runtime port works.
RealWindowsTests must execute (not skip) on both Windows Python architectures.
"""
import ctypes as C
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest
from unittest import mock

SPEC = importlib.util.spec_from_file_location("process_containment", Path(__file__).with_name("process_containment.py"))
P = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(P)


class FakeAPI:
    """Track kernel ownership and inject BOOL/DWORD failures without launching."""
    def __init__(self, fail=None):
        self.fail = fail or {}
        self.calls = []
        self.counts = {}
        self.live = set()
        self.closed = []
        self.attributes = {}
        self.error = 5
        self.next_handle = (1 << 40) if C.sizeof(P.HANDLE) == 8 else 100
        self.process = self.thread = self.job = None
        self.member = True
        self.exited = False
        self.active = 0
        self.keep_active = False
        self.exit_code = 259  # This is a valid exit value AFTER a signalled wait.
        self.resume_result = 1
        self.wait_result = None

    def ok(self, name):
        self.calls.append(name)
        self.counts[name] = self.counts.get(name, 0) + 1
        return self.fail.get(name) != self.counts[name]

    def handle(self):
        self.next_handle += 1
        self.live.add(self.next_handle)
        return self.next_handle

    def last_error(self):
        return self.error

    def GetCurrentProcess(self):
        return -1

    def CreateJobObjectW(self, security, name):
        assert security is None and name is None
        if not self.ok("create_job"):
            return None
        self.job = self.handle()
        return self.job

    def SetInformationJobObject(self, job, kind, data, size):
        assert job == self.job and kind == 9
        assert size == C.sizeof(P.JOBOBJECT_EXTENDED_LIMIT_INFORMATION)
        assert data._obj.BasicLimitInformation.LimitFlags == P.JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
        return self.ok("set_limits")

    def DuplicateHandle(self, source, handle, target, result, access, inherit, options):
        assert source == target == -1 and access == 0 and inherit and options == 2
        if not self.ok("duplicate"):
            return False
        result._obj.value = self.handle()
        return True

    def InitializeProcThreadAttributeList(self, storage, count, flags, size):
        assert count == 2 and flags == 0
        if storage is None:
            self.error = P.ERROR_INSUFFICIENT_BUFFER if self.ok("attribute_size") else 5
            size._obj.value = 128
            return False
        self.error = 5
        return self.ok("attribute_init")

    def UpdateProcThreadAttribute(self, storage, flags, key, data, size, previous, returned):
        assert flags == 0 and previous is None and returned is None
        assert size == C.sizeof(data._obj)
        self.attributes[key] = list(data._obj)
        return self.ok("attribute_handles" if key == P.PROC_THREAD_ATTRIBUTE_HANDLE_LIST else "attribute_jobs")

    def DeleteProcThreadAttributeList(self, storage):
        self.ok("attribute_delete")

    def CreateProcessW(self, application, command, ps, ts, inherit, flags, env, cwd, startup, result):
        assert application == r"C:\python.exe" and command.value.startswith(r"C:\python.exe")
        assert ps is None and ts is None and env is None and inherit
        assert flags == P.CREATE_SUSPENDED | P.CREATE_NO_WINDOW | P.EXTENDED_STARTUPINFO_PRESENT
        assert self.attributes[P.PROC_THREAD_ATTRIBUTE_JOB_LIST] == [self.job]
        handles = self.attributes[P.PROC_THREAD_ATTRIBUTE_HANDLE_LIST]
        assert len(handles) == 3 and self.job not in handles
        si = startup._obj.StartupInfo
        assert si.cb == C.sizeof(P.STARTUPINFOEXW) and si.dwFlags == P.STARTF_USESTDHANDLES
        assert handles == [si.hStdInput, si.hStdOutput, si.hStdError]
        if not self.ok("create_process"):
            return False
        self.process, self.thread = self.handle(), self.handle()
        self.active = 1
        result._obj.hProcess, result._obj.hThread = self.process, self.thread
        result._obj.dwProcessId, result._obj.dwThreadId = 73, 74
        return True

    def IsProcessInJob(self, process, job, result):
        assert process == self.process and job == self.job
        result._obj.value = self.member
        return self.ok("membership")

    def ResumeThread(self, thread):
        assert thread == self.thread
        if not self.ok("resume"):
            return P.WAIT_FAILED
        self.exited = True
        return self.resume_result

    def WaitForSingleObject(self, process, milliseconds):
        assert process == self.process and 0 <= milliseconds < P.WAIT_FAILED
        if not self.ok("wait"):
            return P.WAIT_FAILED
        if self.wait_result is not None:
            return self.wait_result
        return P.WAIT_OBJECT_0 if self.exited else P.WAIT_TIMEOUT

    def GetExitCodeProcess(self, process, result):
        assert process == self.process
        result._obj.value = self.exit_code
        return self.ok("exit_code")

    def TerminateJobObject(self, job, code):
        assert job == self.job and code == 124
        if not self.ok("terminate_job"):
            return False
        self.exited = True
        if not self.keep_active:
            self.active = 0
        return True

    def TerminateProcess(self, process, code):
        assert process == self.process and code == 124
        if not self.ok("terminate_process"):
            return False
        self.exited = True
        return True

    def QueryInformationJobObject(self, job, kind, data, size, returned):
        assert job == self.job and kind == 1 and returned is None
        assert size == C.sizeof(P.JOBOBJECT_BASIC_ACCOUNTING_INFORMATION)
        data._obj.ActiveProcesses = self.active
        return self.ok("query")

    def CloseHandle(self, handle):
        assert handle in self.live, "double close or borrowed-handle close"
        if not self.ok("close"):
            return False
        self.live.remove(handle)
        self.closed.append(handle)
        if handle == self.job:
            self.active = 0  # KILL_ON_JOB_CLOSE fallback.
            self.exited = True
        return True


class MockWindowsTests(unittest.TestCase):
    def start(self, api):
        process = P.WindowsJobProcess(api=api, fd_to_handle=lambda fd: fd + 1000)
        with tempfile.TemporaryFile() as output:
            process.start([r"C:\python.exe", "-c", "pass"], output, output, r"C:\work")
        return process

    def test_native_width_structures(self):
        wide = C.sizeof(P.HANDLE) == 8
        self.assertEqual(C.sizeof(P.DWORD), 4)
        self.assertEqual(C.sizeof(P.BOOL), 4)
        self.assertEqual(C.sizeof(P.STARTUPINFOW), 104 if wide else 68)
        self.assertEqual(P.STARTUPINFOW.dwFlags.offset, 60 if wide else 44)
        self.assertEqual(P.STARTUPINFOW.hStdInput.offset, 80 if wide else 56)
        self.assertEqual(C.sizeof(P.STARTUPINFOEXW), 112 if wide else 72)
        self.assertEqual(C.sizeof(P.PROCESS_INFORMATION), 24 if wide else 16)
        self.assertEqual(C.sizeof(P.JOBOBJECT_BASIC_LIMIT_INFORMATION), 64 if wide else 48)
        self.assertEqual(C.sizeof(P.JOBOBJECT_EXTENDED_LIMIT_INFORMATION), 144 if wide else 112)
        self.assertEqual(C.sizeof(P.IO_COUNTERS), 48)
        self.assertEqual(C.sizeof(P.JOBOBJECT_BASIC_ACCOUNTING_INFORMATION), 48)
        self.assertEqual(P.JOBOBJECT_BASIC_ACCOUNTING_INFORMATION.ActiveProcesses.offset, 40)

    def test_atomic_assignment_precedes_resume_and_handles_are_owned(self):
        api = FakeAPI()
        process = self.start(api)
        order = [api.calls.index(name) for name in (
            "set_limits", "attribute_handles", "attribute_jobs", "create_process", "membership", "resume")]
        self.assertEqual(order, sorted(order))
        self.assertEqual(api.live, {api.job, api.process})
        self.assertEqual(process.poll(), 259)
        process.cleanup(0.1)
        self.assertFalse(api.live)
        self.assertEqual(api.closed[-1], api.job)
        with self.assertRaises(ValueError):
            process.start([], None, None, r"C:\work")

    def test_setup_failures_do_not_run_or_leave_children(self):
        scenarios = [(name, 1) for name in ("create_job", "set_limits", "attribute_size", "attribute_init",
                     "attribute_handles", "attribute_jobs", "create_process", "membership", "resume")]
        scenarios += [("duplicate", number) for number in (1, 2, 3)]
        scenarios += [("close", number) for number in (1, 2, 3, 4)]
        for operation, count in scenarios:
            with self.subTest(operation=operation, count=count):
                api = FakeAPI({operation: count})
                with self.assertRaises(OSError):
                    self.start(api)
                self.assertFalse(api.live, api.calls)
                self.assertEqual(api.active, 0)
                if operation not in ("resume", "close"):
                    self.assertNotIn("resume", api.calls)
                if api.process is not None:
                    self.assertIn("terminate_job", api.calls)

    def test_failed_membership_or_suspend_count_kills_created_process(self):
        for member, count in ((False, 1), (True, 0), (True, 2)):
            with self.subTest(member=member, count=count):
                api = FakeAPI()
                api.member, api.resume_result = member, count
                with self.assertRaises(OSError):
                    self.start(api)
                if not member:
                    self.assertNotIn("resume", api.calls)
                    self.assertIn("terminate_process", api.calls)
                self.assertFalse(api.live)

    def test_cleanup_failures_are_reported_and_all_handles_attempted(self):
        for operation in ("terminate_job", "wait", "exit_code", "query", "close"):
            with self.subTest(operation=operation):
                api = FakeAPI()
                process = self.start(api)
                api.fail[operation] = api.counts.get(operation, 0) + 1
                with self.assertRaises(OSError):
                    process.cleanup(0.01)
                self.assertIsNone(process.job)
                self.assertEqual(api.active, 0)
                if operation == "terminate_job":
                    self.assertIn("terminate_process", api.calls)
                if operation == "close":
                    # Retain a handle whose close failed; retry is explicit.
                    self.assertEqual(api.live, {api.process})
                    process.cleanup(0.01)
                self.assertFalse(api.live)

    def test_job_termination_failure_still_uses_kill_on_close(self):
        api = FakeAPI()
        process = self.start(api)
        api.fail.update(terminate_job=1, terminate_process=1)
        with self.assertRaises(OSError) as error:
            process.cleanup(0)
        self.assertIn("TerminateJobObject", str(error.exception))
        self.assertIn("TerminateProcess", str(error.exception))
        self.assertFalse(api.live)
        self.assertEqual(api.active, 0)

    def test_failed_job_close_is_retained_for_watchdog_retry(self):
        api = FakeAPI()
        process = self.start(api)
        api.fail["close"] = api.counts["close"] + 2  # process closes; job close fails.
        with self.assertRaisesRegex(OSError, "CloseHandle\\(job\\)"):
            process.cleanup(.01)
        self.assertEqual(process.job, api.job)
        self.assertEqual(api.live, {api.job})
        process.terminate()
        process.cleanup(.01)
        self.assertFalse(api.live)

    def test_descendant_wait_is_bounded_and_closes_job_on_timeout(self):
        api = FakeAPI()
        process = self.start(api)
        api.keep_active = True
        with mock.patch.object(P.time, "monotonic", side_effect=[10, 10, 10.1]):
            with self.assertRaisesRegex(OSError, "descendants"):
                process.cleanup(0.01)
        self.assertFalse(api.live)

    def test_waits_never_use_infinite_and_failed_wait_is_not_completion(self):
        api = FakeAPI()
        process = self.start(api)
        api.wait_result = P.WAIT_TIMEOUT
        self.assertIsNone(process.poll())
        for timeout in (0, 0.0001, 1e308):
            with self.assertRaises(subprocess.TimeoutExpired):
                process.wait(timeout)
        api.wait_result = P.WAIT_FAILED
        with self.assertRaises(OSError):
            process.poll()
        for timeout in (-1, float("inf"), float("nan")):
            with self.assertRaises(ValueError):
                process.wait(timeout)
        api.wait_result = None
        process.cleanup(0.01)

    def test_command_validation_prevents_shell_and_length_ambiguity(self):
        bad = ([], ["python.exe"], [r"\python.exe"], [r"C:python.exe"],
               [r"C:\python.exe", "x\0y"], [r"C:\python.exe", "😀" * 16384])
        for command in bad:
            with self.subTest(command=repr(command)[:60]), self.assertRaises(ValueError):
                P.checked_command(command, r"C:\work")
        with self.assertRaises(ValueError):
            P.checked_command([r"C:\python.exe"], "relative")
        arguments = [r"C:\Program Files\python.exe", "", "x y", 'a"b', "unicode-λ"]
        _, _, line = P.checked_command(arguments, r"C:\work")
        self.assertEqual(line, subprocess.list2cmdline(arguments))


@unittest.skipUnless(os.name == "nt", "requires actual Windows APIs; mocks do not establish runtime support")
class RealWindowsTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="optimize-containment-")
        self.directory = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def launch(self, source, args=(), api=None):
        output, error = tempfile.TemporaryFile(dir=self.directory), tempfile.TemporaryFile(dir=self.directory)
        self.addCleanup(output.close)
        self.addCleanup(error.close)
        process = P.WindowsJobProcess(api=api)
        self.addCleanup(process.cleanup, 1)
        process.start([sys.executable, "-c", source, *args], output, error, self.directory)
        return process, output, error

    def test_suspended_membership_before_resume_and_unicode_stdio(self):
        marker = self.directory/"first-instruction"
        api = P.WinAPI()
        original = api.ResumeThread
        observed = []
        def resume(thread):
            self.assertFalse(marker.exists())
            observed.append(True)
            return original(thread)
        api.ResumeThread = resume
        arguments = ("", "space here", 'a"b', "trailing\\", "λ😀")
        source = ("import pathlib,sys,json; pathlib.Path(sys.argv[1]).write_text('ran'); "
                  "print(json.dumps(sys.argv[2:])); print('diagnostic',file=sys.stderr); sys.exit(3)")
        process, output, error = self.launch(source, (str(marker), *arguments), api)
        self.assertEqual(process.wait(5), 3)
        process.cleanup(1)
        output.seek(0); error.seek(0)
        self.assertEqual(json.loads(output.read()), list(arguments))
        self.assertIn(b"diagnostic", error.read())
        self.assertEqual(observed, [True])
        self.assertTrue(marker.exists())

    def test_postcreation_verification_failure_never_runs_child(self):
        for failure in ("membership_false", "membership_api", "resume_api"):
            with self.subTest(failure=failure):
                marker = self.directory/failure
                api = P.WinAPI()
                if failure == "membership_false":
                    api.IsProcessInJob = lambda process, job, result: 1
                elif failure == "membership_api":
                    api.IsProcessInJob = lambda process, job, result: 0
                else:
                    api.ResumeThread = lambda thread: P.WAIT_FAILED
                with self.assertRaises(OSError):
                    self.launch("import pathlib; pathlib.Path("+repr(str(marker))+").write_text('ran')", api=api)
                self.assertFalse(marker.exists())

    def test_successful_root_cleanup_kills_descendant(self):
        marker = self.directory/"descendant"
        ready = self.directory/"child-ready"
        child = ("import pathlib,time; pathlib.Path("+repr(str(ready))+").write_text('ready'); "
                 "time.sleep(.6); pathlib.Path("+repr(str(marker))+").write_text('escaped')")
        source = ("import subprocess,sys,pathlib,time; subprocess.Popen([sys.executable,'-c',"+repr(child)+"]); "
                  "p=pathlib.Path("+repr(str(ready))+"); "
                  "exec('while not p.exists(): time.sleep(.005)')")
        process, _, _ = self.launch(source)
        self.assertEqual(process.wait(5), 0)
        process.cleanup(1)
        self.assertTrue(ready.exists(), "the descendant must actually have run")
        time.sleep(.8)
        self.assertFalse(marker.exists())

    def test_unlisted_inheritable_handle_is_not_inherited(self):
        import msvcrt
        with (self.directory/"unlisted").open("wb") as borrowed:
            handle = msvcrt.get_osfhandle(borrowed.fileno())
            os.set_handle_inheritable(handle, True)
            # BY_HANDLE_FILE_INFORMATION is thirteen DWORDs: its volume serial
            # and high/low file index identify the underlying file. A numeric
            # handle can legitimately be reused for another object in the child.
            kernel = C.WinDLL("kernel32", use_last_error=True)
            info_function = kernel.GetFileInformationByHandle
            info_function.argtypes, info_function.restype = [C.c_void_p, C.c_void_p], C.c_int32
            info = (C.c_uint32 * 13)()
            self.assertTrue(info_function(handle, C.byref(info)))
            identity = [info[7], info[11], info[12]]
            source = ("import ctypes as C,sys,json; k=C.WinDLL('kernel32',use_last_error=True); "
                      "k.GetFileInformationByHandle.argtypes=[C.c_void_p,C.c_void_p]; "
                      "k.GetFileInformationByHandle.restype=C.c_int32; info=(C.c_uint32*13)(); "
                      "ok=k.GetFileInformationByHandle(int(sys.argv[1]),C.byref(info)); "
                      "print(json.dumps([info[7],info[11],info[12]] if ok else None))")
            process, output, _ = self.launch(source, (str(handle),))
            self.assertEqual(process.wait(5), 0)
            process.cleanup(1)
            output.seek(0)
            self.assertNotEqual(json.loads(output.read()), identity,
                                "the unrelated inheritable file escaped the explicit handle allowlist")

    def test_nested_containment_and_breakaway_rejection(self):
        module_dir = str(Path(__file__).resolve().parent)
        inner = ("import sys,tempfile,subprocess; sys.path.insert(0,"+repr(module_dir)+"); "
                 "from process_containment import WindowsJobProcess; "
                 "p=WindowsJobProcess(); out=tempfile.TemporaryFile(); "
                 "p.start([sys.executable,'-c','print(42)'],out,out,"+repr(str(self.directory))+"); "
                 "assert p.wait(5)==0; p.cleanup(1)")
        process, _, error = self.launch(inner)
        self.assertEqual(process.wait(8), 0)
        process.cleanup(1)
        error.seek(0)
        self.assertEqual(error.read(), b"")
        breaker = """import subprocess,sys
try:
    subprocess.Popen([sys.executable,'-c','pass'],creationflags=0x01000000)
except OSError:
    sys.exit(0)
else:
    sys.exit(9)
"""
        process, _, _ = self.launch(breaker)
        self.assertEqual(process.wait(5), 0, "a child was allowed to break away from the job")
        process.cleanup(1)

    def test_owner_abrupt_exit_leaves_no_descendant(self):
        # The owner closes its final job handle on exit, terminating the child.
        module_dir = str(Path(__file__).resolve().parent)
        marker = self.directory/"descendant"
        ready = self.directory/"ready"
        child = ("import pathlib,time; pathlib.Path("+repr(str(ready))+").write_text('ready'); "
                 "time.sleep(1.5); pathlib.Path("+repr(str(marker))+").write_text('escaped')")
        source = ("import os,sys,time,tempfile,pathlib; sys.path.insert(0,"+repr(module_dir)+"); "
                  "from process_containment import WindowsJobProcess; p=WindowsJobProcess(); "
                  "out=tempfile.TemporaryFile(); p.start([sys.executable,'-c',"+repr(child)+"],out,out,"
                  +repr(str(self.directory))+"); ready=pathlib.Path("+repr(str(ready))+"); "
                  "exec('while not ready.exists(): time.sleep(.005)'); os._exit(17)")
        result = subprocess.run([sys.executable, "-c", source], capture_output=True, timeout=4)
        self.assertEqual(result.returncode, 17, result.stderr)
        self.assertTrue(ready.exists())
        time.sleep(1.7)
        self.assertFalse(marker.exists(), "job outlived its final owner handle")


if __name__ == "__main__":
    required = "--require-windows" in sys.argv
    if required:
        sys.argv.remove("--require-windows")
        if os.name != "nt":
            raise SystemExit("Windows verification requires actual Windows; skipped tests cannot pass")
        if unittest.defaultTestLoader.loadTestsFromTestCase(RealWindowsTests).countTestCases() < 6:
            raise SystemExit("Windows containment fixtures are missing")
    outcome = unittest.main(exit=False).result
    raise SystemExit(0 if outcome.wasSuccessful() and not (required and outcome.skipped) else 1)
