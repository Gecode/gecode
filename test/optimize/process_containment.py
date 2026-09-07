"""Bounded Windows process-tree lifetime for compiler and solver tests.

Requires Windows 10 or later. The helper owns the launched process and its job,
so cleanup also terminates descendants started by the compiler or solver.
"""
import ctypes as C
import math
import ntpath
import os
import subprocess
import time

# Windows uses LLP64, including when these declarations are tested on Unix LP64.
DWORD, BOOL, WORD = C.c_uint32, C.c_int32, C.c_uint16
HANDLE, SIZE_T = C.c_void_p, C.c_size_t
POINTER = C.c_void_p
CREATE_SUSPENDED = 0x00000004
CREATE_NO_WINDOW = 0x08000000
EXTENDED_STARTUPINFO_PRESENT = 0x00080000
STARTF_USESTDHANDLES = 0x00000100
PROC_THREAD_ATTRIBUTE_HANDLE_LIST = 0x00020002
PROC_THREAD_ATTRIBUTE_JOB_LIST = 0x0002000D
JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE = 0x00002000
JOB_OBJECT_EXTENDED_LIMIT_INFORMATION = 9
JOB_OBJECT_BASIC_ACCOUNTING_INFORMATION = 1
DUPLICATE_SAME_ACCESS = 2
ERROR_INSUFFICIENT_BUFFER = 122
WAIT_OBJECT_0, WAIT_TIMEOUT, WAIT_FAILED = 0, 258, 0xFFFFFFFF


class STARTUPINFOW(C.Structure):
    _fields_ = [("cb", DWORD), ("lpReserved", POINTER), ("lpDesktop", POINTER),
                ("lpTitle", POINTER), ("dwX", DWORD), ("dwY", DWORD),
                ("dwXSize", DWORD), ("dwYSize", DWORD), ("dwXCountChars", DWORD),
                ("dwYCountChars", DWORD), ("dwFillAttribute", DWORD), ("dwFlags", DWORD),
                ("wShowWindow", WORD), ("cbReserved2", WORD), ("lpReserved2", POINTER),
                ("hStdInput", HANDLE), ("hStdOutput", HANDLE), ("hStdError", HANDLE)]


class STARTUPINFOEXW(C.Structure):
    _fields_ = [("StartupInfo", STARTUPINFOW), ("lpAttributeList", POINTER)]


class PROCESS_INFORMATION(C.Structure):
    _fields_ = [("hProcess", HANDLE), ("hThread", HANDLE),
                ("dwProcessId", DWORD), ("dwThreadId", DWORD)]


class JOBOBJECT_BASIC_LIMIT_INFORMATION(C.Structure):
    _fields_ = [("PerProcessUserTimeLimit", C.c_int64), ("PerJobUserTimeLimit", C.c_int64),
                ("LimitFlags", DWORD), ("MinimumWorkingSetSize", SIZE_T),
                ("MaximumWorkingSetSize", SIZE_T), ("ActiveProcessLimit", DWORD),
                ("Affinity", SIZE_T), ("PriorityClass", DWORD), ("SchedulingClass", DWORD)]


class IO_COUNTERS(C.Structure):
    _fields_ = [(name, C.c_uint64) for name in (
        "ReadOperationCount", "WriteOperationCount", "OtherOperationCount",
        "ReadTransferCount", "WriteTransferCount", "OtherTransferCount")]


class JOBOBJECT_EXTENDED_LIMIT_INFORMATION(C.Structure):
    _fields_ = [("BasicLimitInformation", JOBOBJECT_BASIC_LIMIT_INFORMATION),
                ("IoInfo", IO_COUNTERS), ("ProcessMemoryLimit", SIZE_T),
                ("JobMemoryLimit", SIZE_T), ("PeakProcessMemoryUsed", SIZE_T),
                ("PeakJobMemoryUsed", SIZE_T)]


class JOBOBJECT_BASIC_ACCOUNTING_INFORMATION(C.Structure):
    _fields_ = [("TotalUserTime", C.c_int64), ("TotalKernelTime", C.c_int64),
                ("ThisPeriodTotalUserTime", C.c_int64), ("ThisPeriodTotalKernelTime", C.c_int64),
                ("TotalPageFaultCount", DWORD), ("TotalProcesses", DWORD),
                ("ActiveProcesses", DWORD), ("TotalTerminatedProcesses", DWORD)]


class WinAPI:
    """Explicit stdcall signatures; kernel32 is never loaded on other systems."""
    def __init__(self):
        if os.name != "nt":
            raise OSError("Windows Job Objects require Windows 10 or newer")
        dll = C.WinDLL("kernel32", use_last_error=True)
        ptr = C.c_void_p
        signatures = {
            "CreateJobObjectW": ([ptr, C.c_wchar_p], HANDLE),
            "SetInformationJobObject": ([HANDLE, C.c_int32, ptr, DWORD], BOOL),
            "QueryInformationJobObject": ([HANDLE, C.c_int32, ptr, DWORD, ptr], BOOL),
            "InitializeProcThreadAttributeList": ([ptr, DWORD, DWORD, C.POINTER(SIZE_T)], BOOL),
            "UpdateProcThreadAttribute": ([ptr, DWORD, SIZE_T, ptr, SIZE_T, ptr, ptr], BOOL),
            "DeleteProcThreadAttributeList": ([ptr], None),
            "CreateProcessW": ([C.c_wchar_p, C.c_wchar_p, ptr, ptr, BOOL, DWORD,
                                ptr, C.c_wchar_p, ptr, C.POINTER(PROCESS_INFORMATION)], BOOL),
            "IsProcessInJob": ([HANDLE, HANDLE, C.POINTER(BOOL)], BOOL),
            "ResumeThread": ([HANDLE], DWORD),
            "WaitForSingleObject": ([HANDLE, DWORD], DWORD),
            "GetExitCodeProcess": ([HANDLE, C.POINTER(DWORD)], BOOL),
            "TerminateJobObject": ([HANDLE, DWORD], BOOL),
            "TerminateProcess": ([HANDLE, DWORD], BOOL),
            "CloseHandle": ([HANDLE], BOOL),
            "GetCurrentProcess": ([], HANDLE),
            "DuplicateHandle": ([HANDLE, HANDLE, HANDLE, C.POINTER(HANDLE),
                                  DWORD, BOOL, DWORD], BOOL),
        }
        for name, (args, result) in signatures.items():
            try:
                function = getattr(dll, name)
            except AttributeError as error:
                raise OSError("required Windows containment API is unavailable: " + name) from error
            function.argtypes, function.restype = args, result
            setattr(self, name, function)

    @staticmethod
    def last_error():
        return C.get_last_error()


def checked_command(command, directory):
    """Use an explicit executable and CRT argument quoting, without a shell."""
    command = [os.fspath(arg) for arg in command]
    directory = os.fspath(directory)
    if (not command or any(not isinstance(arg, str) or "\0" in arg for arg in command)
            or not isinstance(directory, str) or "\0" in directory):
        raise ValueError("Windows command and working directory must be NUL-free strings")
    # ntpath also works in platform-independent conformance tests.
    if any(not ntpath.isabs(path) or not ntpath.splitdrive(path)[0] for path in (command[0], directory)):
        raise ValueError("Windows executable and working directory must be fully qualified absolute paths")
    line = subprocess.list2cmdline(command)
    if len(line.encode("utf-16-le")) // 2 + 1 > 32767:
        raise ValueError("Windows command exceeds the 32767 UTF-16-unit CreateProcessW limit")
    return command, directory, line


class WindowsJobProcess:
    """One launch, owned process/thread/job handles, explicit bounded cleanup.

    Publish this object to the outer watchdog BEFORE start(). Creation-time job
    assignment closes the suspended-process orphan window even if that watchdog
    exits between CreateProcessW and its return to Python. The job handle is never
    inherited. There is no fallback to launching an uncontained process.
    """
    def __init__(self, api=None, fd_to_handle=None):
        self.api = WinAPI() if api is None else api
        if fd_to_handle is None:
            import msvcrt
            fd_to_handle = msvcrt.get_osfhandle
        self.fd_to_handle = fd_to_handle
        self.job = self.process = self.thread = None
        self.pid = self.returncode = None
        self.command = []
        self._stdio = []
        self._attributes = None
        self._attribute_storage = None
        self._attribute_values = []
        self._started = False
        self._membership_verified = False

    def _error(self, operation):
        return OSError("{} failed (Windows error {})".format(operation, self.api.last_error()))

    def _check(self, result, operation):
        if not result:
            raise self._error(operation)

    def _duplicate(self, stream):
        handle = HANDLE()
        current = self.api.GetCurrentProcess()
        self._check(self.api.DuplicateHandle(current, self.fd_to_handle(stream.fileno()), current,
                                             C.byref(handle), 0, True, DUPLICATE_SAME_ACCESS),
                    "DuplicateHandle(stdio)")
        self._stdio.append(handle.value)
        return handle.value

    def start(self, command, stdout, stderr, directory):
        if self._started:
            raise ValueError("WindowsJobProcess permits exactly one start")
        self.command, directory, line = checked_command(command, directory)
        self._started = True
        try:
            try:
                self.job = self.api.CreateJobObjectW(None, None) or None
                self._check(self.job, "CreateJobObjectW")
                limits = JOBOBJECT_EXTENDED_LIMIT_INFORMATION()
                limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
                self._check(self.api.SetInformationJobObject(
                    self.job, JOB_OBJECT_EXTENDED_LIMIT_INFORMATION, C.byref(limits), C.sizeof(limits)),
                    "SetInformationJobObject(KILL_ON_JOB_CLOSE)")
                with open(os.devnull, "rb") as stdin:
                    inherited = (HANDLE * 3)(self._duplicate(stdin), self._duplicate(stdout),
                                              self._duplicate(stderr))
                jobs = (HANDLE * 1)(self.job)
                self._attribute_values = [inherited, jobs]
                size = SIZE_T()
                result = self.api.InitializeProcThreadAttributeList(None, 2, 0, C.byref(size))
                if result or self.api.last_error() != ERROR_INSUFFICIENT_BUFFER or not size.value:
                    raise self._error("InitializeProcThreadAttributeList(size)")
                self._attribute_storage = C.create_string_buffer(size.value)
                self._check(self.api.InitializeProcThreadAttributeList(
                    self._attribute_storage, 2, 0, C.byref(size)), "InitializeProcThreadAttributeList")
                self._attributes = self._attribute_storage
                for key, value in ((PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherited),
                                   (PROC_THREAD_ATTRIBUTE_JOB_LIST, jobs)):
                    self._check(self.api.UpdateProcThreadAttribute(
                        self._attributes, 0, key, C.byref(value), C.sizeof(value), None, None),
                        "UpdateProcThreadAttribute({:#x})".format(key))
                startup = STARTUPINFOEXW()
                startup.StartupInfo.cb = C.sizeof(startup)
                startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES
                startup.StartupInfo.hStdInput, startup.StartupInfo.hStdOutput, startup.StartupInfo.hStdError = inherited
                startup.lpAttributeList = C.cast(self._attributes, POINTER)
                info = PROCESS_INFORMATION()
                self._check(self.api.CreateProcessW(
                    self.command[0], C.create_unicode_buffer(line), None, None, True,
                    CREATE_SUSPENDED | CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT,
                    None, directory, C.byref(startup), C.byref(info)), "CreateProcessW(atomic job assignment)")
                self.process, self.thread, self.pid = info.hProcess, info.hThread, info.dwProcessId
                member = BOOL()
                self._check(self.api.IsProcessInJob(self.process, self.job, C.byref(member)), "IsProcessInJob")
                if not member.value:
                    raise OSError("CreateProcessW did not assign the process to its required Job Object")
                self._membership_verified = True
                previous = self.api.ResumeThread(self.thread)
                if previous == WAIT_FAILED:
                    raise self._error("ResumeThread")
                if previous != 1:
                    raise OSError("ResumeThread returned unexpected suspend count {}".format(previous))
                self._check(self.api.CloseHandle(self.thread), "CloseHandle(primary thread)")
                self.thread = None
            finally:
                self._release_setup()
        except BaseException:
            # This also handles a post-create verification failure: the primary
            # thread remains suspended, and both job and process are terminated.
            self.cleanup(timeout=0.2)
            raise
        return self

    def _release_setup(self):
        if self._attributes is not None:
            self.api.DeleteProcThreadAttributeList(self._attributes)
            self._attributes = None
        self._attribute_values = []
        self._attribute_storage = None
        errors = []
        for handle in self._stdio[:]:
            if self.api.CloseHandle(handle):
                self._stdio.remove(handle)
            else:
                errors.append(str(self._error("CloseHandle(stdio duplicate)")))
        if errors:
            raise OSError("; ".join(errors))

    def _exit_code(self):
        result = DWORD()
        self._check(self.api.GetExitCodeProcess(self.process, C.byref(result)), "GetExitCodeProcess")
        self.returncode = result.value
        return self.returncode

    def poll(self):
        if self.returncode is not None:
            return self.returncode
        if self.process is None:
            raise OSError("process has not been started or has already been closed")
        state = self.api.WaitForSingleObject(self.process, 0)
        if state == WAIT_TIMEOUT:
            return None
        if state != WAIT_OBJECT_0:
            raise self._error("WaitForSingleObject")
        return self._exit_code()

    def wait(self, timeout):
        if not math.isfinite(timeout) or timeout < 0:
            raise ValueError("cleanup timeout must be finite and nonnegative")
        if self.returncode is not None:
            return self.returncode
        if self.process is None:
            raise OSError("process has not been started or has already been closed")
        # Never use INFINITE, nor round a wait beyond its caller's reserve.
        milliseconds = int(min((WAIT_FAILED - 1) / 1000, timeout) * 1000)
        state = self.api.WaitForSingleObject(self.process, milliseconds)
        if state == WAIT_TIMEOUT:
            raise subprocess.TimeoutExpired(self.command, timeout)
        if state != WAIT_OBJECT_0:
            raise self._error("WaitForSingleObject")
        return self._exit_code()

    def terminate(self):
        """Nonwaiting watchdog action; final process exit also closes the job."""
        errors = []
        terminated = False
        if self.job is not None:
            terminated = bool(self.api.TerminateJobObject(self.job, 124))
            if not terminated:
                errors.append(str(self._error("TerminateJobObject")))
        if self.process is not None and (not terminated or not self._membership_verified):
            if not self.api.TerminateProcess(self.process, 124):
                errors.append(str(self._error("TerminateProcess")))
        if errors:
            raise OSError("; ".join(errors))

    def cleanup(self, timeout):
        """Terminate descendants even after root success; close every owned handle.

        Any cleanup failure fails the case. No unbounded wait is used. Keep any
        unsuccessfully closed handles available to the outer watchdog for retry.
        """
        if not math.isfinite(timeout) or timeout < 0:
            raise ValueError("cleanup timeout must be finite and nonnegative")
        deadline = time.monotonic() + timeout
        errors = []
        try:
            try:
                self.terminate()
            except OSError as error:
                errors.append(str(error))
            if self.process is not None:
                try:
                    self.wait(max(0, deadline-time.monotonic()))
                except (OSError, subprocess.TimeoutExpired) as error:
                    errors.append(str(error))
            if self.job is not None:
                # Waiting for the root cannot establish that grandchildren died.
                while True:
                    accounting = JOBOBJECT_BASIC_ACCOUNTING_INFORMATION()
                    if not self.api.QueryInformationJobObject(
                            self.job, JOB_OBJECT_BASIC_ACCOUNTING_INFORMATION,
                            C.byref(accounting), C.sizeof(accounting), None):
                        errors.append(str(self._error("QueryInformationJobObject")))
                        break
                    if accounting.ActiveProcesses == 0:
                        break
                    remaining = deadline-time.monotonic()
                    if remaining <= 0:
                        errors.append("Job Object descendants did not exit within cleanup reserve")
                        break
                    time.sleep(min(0.005, remaining))
        finally:
            try:
                self._release_setup()
            except OSError as error:
                errors.append(str(error))
            # Closing the last, non-inherited job handle is the final kill path.
            for name in ("thread", "process", "job"):
                handle = getattr(self, name)
                if handle is not None:
                    if self.api.CloseHandle(handle):
                        setattr(self, name, None)
                    else:
                        errors.append(str(self._error("CloseHandle({})".format(name))))
        if errors:
            raise OSError("; ".join(errors))
