/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Jip J. Dekker, 2026
 */
#if defined(_WIN32)
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#if !defined(WINVER) || (WINVER < 0x0600)
#undef WINVER
#define WINVER 0x0600
#endif

#include <gecode/flatzinc/blackbox-process.hh>
#include <gecode/flatzinc.hh>
#include <cassert>
#include <sstream>
#include <windows.h>

namespace Gecode { namespace FlatZinc {
    namespace {

        const size_t max_exec_response_size = 1024 * 1024;

        std::wstring
        utf8_to_wide(const std::string& s) {
            if (s.empty()) {
                return std::wstring();
            }
            int n =
                MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
            if (n == 0) {
                throw Error("Blackbox", "Invalid UTF-8 string in blackbox path or argument");
            }
            std::wstring w(static_cast<size_t>(n), L'\0');
            if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), static_cast<int>(s.size()), &w[0], n) ==
                0) {
                throw Error("Blackbox", "Invalid UTF-8 string in blackbox path or argument");
            }
            return w;
        }

        std::string
        windows_error(const std::string& prefix, DWORD err) {
            return prefix + " (Windows error " + std::to_string(err) + ")";
        }

        class WindowsHandle {
        private:
            HANDLE handle;

        public:
            explicit WindowsHandle(HANDLE handle0 = nullptr) : handle(handle0) {}
            ~WindowsHandle() {
                reset();
            }

            WindowsHandle(const WindowsHandle&) = delete;
            WindowsHandle& operator=(const WindowsHandle&) = delete;

            HANDLE
            get() const {
                return handle;
            }
            HANDLE*
            put() {
                reset();
                return &handle;
            }
            HANDLE
            release() {
                HANDLE handle0 = handle;
                handle = nullptr;
                return handle0;
            }
            bool
            valid() const {
                return (handle != nullptr) && (handle != INVALID_HANDLE_VALUE);
            }
            void
            reset(HANDLE handle0 = nullptr) {
                if (valid()) {
                    CloseHandle(handle);
                }
                handle = handle0;
            }
        };

        class WindowsAttributeList {
        private:
            std::vector<char> buffer;
            LPPROC_THREAD_ATTRIBUTE_LIST list;
            bool initialized;

        public:
            WindowsAttributeList() : list(nullptr), initialized(false) {}
            ~WindowsAttributeList() {
                if (initialized) {
                    DeleteProcThreadAttributeList(list);
                }
            }

            void
            init() {
                SIZE_T size = 0;
                InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
                if (size == 0) {
                    throw Error("BlackBoxExec",
                                windows_error("ProcThreadAttributeList size query failed", GetLastError()));
                }
                buffer.resize(size);
                list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
                if (!InitializeProcThreadAttributeList(list, 1, 0, &size)) {
                    throw Error("BlackBoxExec",
                                windows_error("InitializeProcThreadAttributeList failed", GetLastError()));
                }
                initialized = true;
            }

            void
            set_inherited_handles(HANDLE* handles, DWORD count) {
                if (!UpdateProcThreadAttribute(list, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles,
                                               sizeof(HANDLE) * count, nullptr, nullptr)) {
                    throw Error("BlackBoxExec",
                                windows_error("PROC_THREAD_ATTRIBUTE_HANDLE_LIST failed", GetLastError()));
                }
            }

            LPPROC_THREAD_ATTRIBUTE_LIST
            get() const {
                return list;
            }
        };
        bool
        qualified_path(const std::wstring& program) {
            return (program.find_first_of(L"\\/") != std::wstring::npos) ||
                   ((program.size() > 1) && (program[1] == L':'));
        }

        class WindowsProcessSession : public BlackBoxProcessSession {
        protected:
            HANDLE job;
            HANDLE process;
            HANDLE pipe_send;
            HANDLE pipe_receive;

            static std::string
            last_error(const std::string& prefix) {
                return prefix + " (Windows error " + std::to_string(GetLastError()) + ")";
            }

            static void
            close_handle(HANDLE& h) {
                if (h != nullptr) {
                    CloseHandle(h);
                    h = nullptr;
                }
            }

            static std::wstring
            quote_argument(const std::wstring& arg) {
                std::wstring q(L"\"");
                unsigned int backslashes = 0;
                for (wchar_t ch : arg) {
                    if (ch == L'\\') {
                        backslashes++;
                    } else if (ch == L'"') {
                        q.append(backslashes * 2 + 1, L'\\');
                        q += ch;
                        backslashes = 0;
                    } else {
                        q.append(backslashes, L'\\');
                        q += ch;
                        backslashes = 0;
                    }
                }
                q.append(backslashes * 2, L'\\');
                q += L'"';
                return q;
            }

            void open_windows(const std::string& program, const std::vector<std::string>& args);
            void close_windows();

        public:
            WindowsProcessSession(const std::string& program, const std::vector<std::string>& args)
                : job(nullptr), process(nullptr), pipe_send(nullptr), pipe_receive(nullptr) {
                open_windows(program, args);
            }

            ~WindowsProcessSession() override {
                close();
            }

            std::string
            exchange(const std::string& out_buf) override {
                size_t written = 0;
                while (written < out_buf.size()) {
                    DWORD count = 0;
                    DWORD remaining = static_cast<DWORD>(out_buf.size() - written);
                    BOOL success = WriteFile(pipe_send, out_buf.data() + written, remaining, &count, nullptr);
                    if (!success || count == 0) {
                        throw Error("BlackBoxExec", last_error("Writing blackbox process input failed"));
                    }
                    written += count;
                }

                char c[2] = {0, 0};
                std::ostringstream oss;
                size_t response_size = 0;
                while (c[0] != '\n') {
                    DWORD count = 0;
                    BOOL success = ReadFile(pipe_receive, c, sizeof(c) - 1, &count, nullptr);
                    if (!success) {
                        if (GetLastError() == ERROR_BROKEN_PIPE) {
                            throw Error("BlackBoxExec", "Blackbox process provided an incomplete response");
                        }
                        throw Error("BlackBoxExec", "Failed to read blackbox process output from pipe");
                    } else if (count == 0) {
                        throw Error("BlackBoxExec", "Blackbox process provided an incomplete response");
                    }
                    assert(count == 1);
                    if (++response_size > max_exec_response_size) {
                        throw Error("BlackBoxExec", "Blackbox process response exceeds the size limit");
                    }
                    oss << c[0];
                }
                return oss.str();
            }

            void
            close() {
                close_windows();
            }
        };

        void
        WindowsProcessSession::open_windows(const std::string& program, const std::vector<std::string>& args) {
            // Build the command line before opening OS handles so allocation/conversion
            // failures cannot leak partially constructed process state.
            std::wstring program_w = utf8_to_wide(program);
            std::wstring prog = quote_argument(program_w);
            for (const std::string& a : args) {
                prog += L" ";
                prog += quote_argument(utf8_to_wide(a));
            }
            std::vector<wchar_t> command_line(prog.begin(), prog.end());
            command_line.push_back(L'\0');

            SECURITY_ATTRIBUTES security_attributes;
            security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
            security_attributes.bInheritHandle = TRUE;
            security_attributes.lpSecurityDescriptor = nullptr;

            WindowsHandle child_stdin_read;
            WindowsHandle child_stdin_write;
            WindowsHandle child_stdout_read;
            WindowsHandle child_stdout_write;
            WindowsHandle child_stderr_write;
            if (!CreatePipe(child_stdout_read.put(), child_stdout_write.put(), &security_attributes, 0)) {
                throw Error("BlackBoxExec", last_error("Stdout CreatePipe failed"));
            }
            if (!SetHandleInformation(child_stdout_read.get(), HANDLE_FLAG_INHERIT, 0)) {
                throw Error("BlackBoxExec", last_error("Stdout SetHandleInformation failed"));
            }
            if (!CreatePipe(child_stdin_read.put(), child_stdin_write.put(), &security_attributes, 0)) {
                throw Error("BlackBoxExec", last_error("Stdin CreatePipe failed"));
            }
            if (!SetHandleInformation(child_stdin_write.get(), HANDLE_FLAG_INHERIT, 0)) {
                throw Error("BlackBoxExec", last_error("Stdin SetHandleInformation failed"));
            }

            HANDLE parent_stderr = GetStdHandle(STD_ERROR_HANDLE);
            if ((parent_stderr != nullptr) && (parent_stderr != INVALID_HANDLE_VALUE)) {
                if (!DuplicateHandle(GetCurrentProcess(), parent_stderr, GetCurrentProcess(), child_stderr_write.put(),
                                     0, TRUE, DUPLICATE_SAME_ACCESS)) {
                    throw Error("BlackBoxExec", last_error("stderr DuplicateHandle failed"));
                }
            } else {
                HANDLE nul = CreateFileW(L"NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         &security_attributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (nul == INVALID_HANDLE_VALUE) {
                    throw Error("BlackBoxExec", last_error("stderr NUL CreateFile failed"));
                }
                child_stderr_write.reset(nul);
            }

            WindowsAttributeList attr_list;
            attr_list.init();
            PROCESS_INFORMATION process_info;
            STARTUPINFOEXW startup_info;
            ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));
            ZeroMemory(&startup_info, sizeof(STARTUPINFOEXW));
            startup_info.StartupInfo.cb = sizeof(STARTUPINFOEXW);
            startup_info.StartupInfo.hStdOutput = child_stdout_write.get();
            startup_info.StartupInfo.hStdInput = child_stdin_read.get();
            startup_info.StartupInfo.hStdError = child_stderr_write.get();
            startup_info.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

            HANDLE inherit_handles[3] = {child_stdin_read.get(), child_stdout_write.get(), child_stderr_write.get()};
            attr_list.set_inherited_handles(inherit_handles, 3);
            startup_info.lpAttributeList = attr_list.get();

            WindowsHandle process_job(CreateJobObjectW(nullptr, nullptr));
            if (!process_job.valid()) {
                throw Error("BlackBoxExec", last_error("CreateJobObject failed"));
            }
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_info;
            ZeroMemory(&job_info, sizeof(job_info));
            job_info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            if (!SetInformationJobObject(process_job.get(), JobObjectExtendedLimitInformation, &job_info,
                                         sizeof(job_info))) {
                throw Error("BlackBoxExec", last_error("SetInformationJobObject failed"));
            }

            BOOL process_started = CreateProcessW(qualified_path(program_w) ? program_w.c_str() : nullptr,
                                                  command_line.data(), // command line
                                                  nullptr,             // process security attributes
                                                  nullptr,             // primary thread security attributes
                                                  TRUE,                // handles from attribute list
                                                  EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED,
                                                  nullptr, // use parent's environment
                                                  nullptr, // use parent's current directory
                                                  &startup_info.StartupInfo,
                                                  &process_info); // receives PROCESS_INFORMATION

            if (!process_started) {
                throw Error(
                    "BlackBoxExec",
                    windows_error("starting blackbox process failed for program `" + program + "'", GetLastError()));
            }
            WindowsHandle process_handle(process_info.hProcess);
            WindowsHandle thread_handle(process_info.hThread);
            if (!AssignProcessToJobObject(process_job.get(), process_handle.get())) {
                DWORD err = GetLastError();
                DWORD terminate_err = ERROR_SUCCESS;
                if (!TerminateProcess(process_handle.get(), 1)) {
                    terminate_err = GetLastError();
                }
                DWORD wait = WaitForSingleObject(process_handle.get(), 5000);
                std::string message = windows_error("Unable to assign blackbox process to required job", err);
                if (terminate_err != ERROR_SUCCESS) {
                    message += "; " + windows_error("TerminateProcess cleanup failed", terminate_err);
                }
                if (wait == WAIT_FAILED) {
                    message += "; " + last_error("process cleanup wait failed");
                } else if (wait == WAIT_TIMEOUT) {
                    message += "; process cleanup timed out";
                }
                throw Error("BlackBoxExec", message);
            }

            if (ResumeThread(thread_handle.get()) == static_cast<DWORD>(-1)) {
                DWORD err = GetLastError();
                DWORD terminate_err = ERROR_SUCCESS;
                if (!TerminateJobObject(process_job.get(), 1)) {
                    terminate_err = GetLastError();
                }
                HANDLE assigned_job = process_job.release();
                DWORD close_err = ERROR_SUCCESS;
                if (!CloseHandle(assigned_job)) {
                    close_err = GetLastError();
                }
                DWORD wait = WaitForSingleObject(process_handle.get(), 5000);
                std::string message = windows_error("ResumeThread failed for blackbox process", err);
                if (terminate_err != ERROR_SUCCESS) {
                    message += "; " + windows_error("TerminateJobObject cleanup failed", terminate_err);
                }
                if (close_err != ERROR_SUCCESS) {
                    message += "; " + windows_error("job cleanup close failed", close_err);
                }
                if (wait == WAIT_FAILED) {
                    message += "; " + last_error("process cleanup wait failed");
                } else if (wait == WAIT_TIMEOUT) {
                    message += "; process cleanup timed out";
                }
                throw Error("BlackBoxExec", message);
            }

            pipe_send = child_stdin_write.release();
            pipe_receive = child_stdout_read.release();
            process = process_handle.release();
            job = process_job.release();
        }

        void
        WindowsProcessSession::close_windows() {
            close_handle(pipe_send);
            close_handle(pipe_receive);
            if (process != nullptr) {
                DWORD wait = WaitForSingleObject(process, 1000);
                if (wait == WAIT_TIMEOUT) {
                    if (job != nullptr) {
                        TerminateJobObject(job, 1);
                    } else {
                        TerminateProcess(process, 1);
                    }
                    WaitForSingleObject(process, 5000);
                }
                close_handle(process);
            }
            close_handle(job);
        }
    } // namespace

    BlackBoxProcessSession*
    create_blackbox_process(const std::string& program, const std::vector<std::string>& args) {
        return new WindowsProcessSession(program, args);
    }

}} // namespace Gecode::FlatZinc
#endif

// STATISTICS: flatzinc-other
