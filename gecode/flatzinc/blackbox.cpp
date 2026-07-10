/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Copyright:
 *     Jip J. Dekker, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/kernel.hh>
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/blackbox.hh>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <locale>
#include <memory>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX 1 // Ensure the words min/max remain available
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
#endif

#ifdef GECODE_HAS_THREADS
#include <thread>
#endif

namespace Gecode {
namespace FlatZinc {

namespace {

#ifdef _WIN32
std::wstring
utf8_to_wide(const std::string &s) {
  if (s.empty()) {
    return std::wstring();
  }
  int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(),
                              static_cast<int>(s.size()), NULL, 0);
  if (n == 0) {
    throw Error("Blackbox", "Invalid UTF-8 string in blackbox path or argument");
  }
  std::wstring w(static_cast<size_t>(n), L'\0');
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(),
                          static_cast<int>(s.size()), &w[0], n) == 0) {
    throw Error("Blackbox", "Invalid UTF-8 string in blackbox path or argument");
  }
  return w;
}

std::string
windows_error(const std::string &prefix, DWORD err) {
  return prefix + " (Windows error " + std::to_string(err) + ")";
}

void
close_library(void *library) {
  if (library != nullptr) {
    FreeLibrary(static_cast<HMODULE>(library));
  }
}
#else
void
close_library(void *library) {
  if (library != nullptr) {
    dlclose(library);
  }
}

int
set_cloexec(int fd) {
  int flags = fcntl(fd, F_GETFD);
  if (flags == -1) {
    return -1;
  }
  return fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

int
dup_cloexec(int fd, int min_fd) {
  int nfd;
#ifdef F_DUPFD_CLOEXEC
  nfd = fcntl(fd, F_DUPFD_CLOEXEC, min_fd);
  if (nfd != -1) {
    return nfd;
  }
  if (errno != EINVAL) {
    return -1;
  }
#endif
  nfd = fcntl(fd, F_DUPFD, min_fd);
  if (nfd == -1) {
    return -1;
  }
  if (set_cloexec(nfd) != 0) {
    int e = errno;
    ::close(nfd);
    errno = e;
    return -1;
  }
  return nfd;
}

int
move_from_standard_fd(int fd) {
  if (fd > STDERR_FILENO) {
    return fd;
  }
  int nfd = dup_cloexec(fd, STDERR_FILENO + 1);
  if (nfd == -1) {
    return -1;
  }
  ::close(fd);
  return nfd;
}

int
create_socketpair(int sv[2]) {
#ifdef SOCK_CLOEXEC
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv) == 0) {
    return 0;
  }
  if (errno != EINVAL) {
    return -1;
  }
#endif
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
    return -1;
  }
  if ((set_cloexec(sv[0]) != 0) || (set_cloexec(sv[1]) != 0)) {
    int e = errno;
    ::close(sv[0]);
    ::close(sv[1]);
    errno = e;
    return -1;
  }
  return 0;
}

ssize_t
send_no_sigpipe(int fd, const char *data, size_t size) {
#ifdef MSG_NOSIGNAL
  return send(fd, data, size, MSG_NOSIGNAL);
#else
#ifdef SO_NOSIGPIPE
  return send(fd, data, size, 0);
#else
  sigset_t block;
  sigset_t old;
  sigset_t pending;
  sigemptyset(&block);
  sigaddset(&block, SIGPIPE);
  bool blocked = false;
  bool was_pending = false;
  if (pthread_sigmask(SIG_BLOCK, &block, &old) == 0) {
    blocked = true;
    if (sigpending(&pending) == 0) {
      was_pending = sigismember(&pending, SIGPIPE) == 1;
    }
  }
  ssize_t n = send(fd, data, size, 0);
  if ((n == -1) && (errno == EPIPE) && !was_pending) {
    const struct timespec timeout = {0, 0};
    sigtimedwait(&block, NULL, &timeout);
  }
  if (blocked) {
    pthread_sigmask(SIG_SETMASK, &old, NULL);
  }
  return n;
#endif
#endif
}
#endif

int
checked_int(long long v, const char *source, size_t i) {
  if ((v < Int::Limits::min) || (v > Int::Limits::max) ||
      (v < std::numeric_limits<int>::min()) ||
      (v > std::numeric_limits<int>::max())) {
    throw Error("Blackbox", std::string(source) + " integer " +
                              std::to_string(i) +
                              " is outside Gecode's integer range");
  }
  return static_cast<int>(v);
}

#ifdef GECODE_HAS_FLOAT_VARS
void
check_float(double v, const char *source, size_t i) {
  static_assert(sizeof(double) == sizeof(std::uint64_t) &&
                std::numeric_limits<double>::is_iec559,
                "blackbox floats must use IEEE-754 binary64");
  std::uint64_t bits = 0;
  const volatile unsigned char *raw =
      reinterpret_cast<const volatile unsigned char *>(&v);
  unsigned char *target = reinterpret_cast<unsigned char *>(&bits);
  for (size_t j = 0; j < sizeof(bits); j++) {
    target[j] = raw[j];
  }
  if (((bits & UINT64_C(0x7ff0000000000000)) ==
       UINT64_C(0x7ff0000000000000)) ||
      (v < Float::Limits::min) || (v > Float::Limits::max)) {
    throw Error("Blackbox", std::string(source) + " float " +
                              std::to_string(i) +
                              " is not a finite value in Gecode's floating "
                              "point range");
  }
}

void
check_floats(const std::vector<double> &v, const char *source) {
  for (size_t i = 0; i < v.size(); i++) {
    check_float(v[i], source, i);
  }
}
#endif

const size_t max_exec_response_size = 1024 * 1024;

} // namespace

BlackBoxLibrary::BlackBoxLibrary(const std::string &name,
                                 const std::vector<std::string> &args)
    : library(nullptr), library_fzn_blackbox(nullptr) {
  std::string loadError;
  void *loaded = nullptr;
#ifdef _WIN32
  DWORD err = 0;
  std::wstring wname = utf8_to_wide(name);
  loaded = LoadLibraryW(wname.c_str());
  if (!loaded) {
    err = GetLastError();
    loadError = std::string("unable to locate library `") + name + "'";
    std::wstring wdll = utf8_to_wide(name + ".dll");
    loaded = LoadLibraryW(wdll.c_str());
    if (!loaded) {
      err = GetLastError();
    }
  }
  if (!loaded) {
    std::wstring wlibdll = utf8_to_wide(std::string("lib") + name + ".dll");
    loaded = LoadLibraryW(wlibdll.c_str());
    if (!loaded) {
      loadError += " (" + windows_error("LoadLibraryW failed", err) + ")";
    }
  }
#else
  loaded = dlopen(name.c_str(), RTLD_LAZY);
  if (!loaded) {
    loadError = std::string(dlerror());
    loaded = dlopen((name + ".so").c_str(), RTLD_NOW);
  }
  if (!loaded) {
    loaded = dlopen((std::string("lib") + name + ".so").c_str(), RTLD_NOW);
  }
#ifdef __APPLE__
  if (!loaded) {
    loaded = dlopen((name + ".dylib").c_str(), RTLD_NOW);
  }
  if (!loaded) {
    loaded = dlopen((std::string("lib") + name + ".dylib").c_str(), RTLD_NOW);
  }
#endif
#endif
  if (!loaded) {
    throw Error("Blackbox", "Unable to open dynamic library: " + loadError);
  }

  // find symbol for blackbox function
#ifdef _WIN32
  library_fzn_blackbox = reinterpret_cast<decltype(library_fzn_blackbox)>(
      GetProcAddress((HMODULE)loaded, "fzn_blackbox"));
#if defined(_M_IX86) || defined(__i386__)
  if (!library_fzn_blackbox) {
    library_fzn_blackbox = reinterpret_cast<decltype(library_fzn_blackbox)>(
        GetProcAddress((HMODULE)loaded, "_fzn_blackbox@32"));
  }
  if (!library_fzn_blackbox) {
    library_fzn_blackbox = reinterpret_cast<decltype(library_fzn_blackbox)>(
        GetProcAddress((HMODULE)loaded, "fzn_blackbox@32"));
  }
#endif
  std::string symError = ".";
#else
  *(void **)(&library_fzn_blackbox) = dlsym(loaded, "fzn_blackbox");
  std::string symError(": ");
  if (!library_fzn_blackbox) {
    symError += std::string(dlerror());
  }
#endif
  if (!library_fzn_blackbox) {
    close_library(loaded);
    throw Error("Blackbox",
                "Unable to find symbol `fzn_blackbox` in dynamic library" +
                    symError);
  }

  // Optionally call the initialisation function with the given arguments. It is
  // not an error for the library to omit `fzn_initialize`.
  void(GECODE_BLACKBOX_CALL *dll_fzn_initialize)(const char **, size_t) =
      nullptr;
#ifdef _WIN32
  dll_fzn_initialize = reinterpret_cast<decltype(dll_fzn_initialize)>(
      GetProcAddress((HMODULE)loaded, "fzn_initialize"));
#if defined(_M_IX86) || defined(__i386__)
  if (!dll_fzn_initialize) {
    dll_fzn_initialize = reinterpret_cast<decltype(dll_fzn_initialize)>(
        GetProcAddress((HMODULE)loaded, "_fzn_initialize@8"));
  }
  if (!dll_fzn_initialize) {
    dll_fzn_initialize = reinterpret_cast<decltype(dll_fzn_initialize)>(
        GetProcAddress((HMODULE)loaded, "fzn_initialize@8"));
  }
#endif
#else
  *(void **)(&dll_fzn_initialize) = dlsym(loaded, "fzn_initialize");
#endif
  try {
    if (dll_fzn_initialize != nullptr) {
      std::vector<const char *> argv;
      argv.reserve(args.size());
      for (const std::string &a : args) {
        argv.push_back(a.c_str());
      }
      dll_fzn_initialize(argv.data(), argv.size());
    }
  } catch (...) {
    close_library(loaded);
    throw;
  }
  library = loaded;
}

BlackBoxLibrary::~BlackBoxLibrary() {
  close_library(library);
}

void
BlackBoxLibrary::run(const std::vector<int64_t> &int_in,
                     const std::vector<double> &float_in,
                     std::vector<int64_t> &int_out,
                     std::vector<double> &float_out) {
  library_fzn_blackbox(int_in.data(), int_in.size(), float_in.data(),
                       float_in.size(), int_out.data(), int_out.size(),
                       float_out.data(), float_out.size());
  for (size_t i = 0; i < int_out.size(); ++i) {
    int_out[i] = checked_int(int_out[i], "library output", i);
  }
#ifdef GECODE_HAS_FLOAT_VARS
  check_floats(float_out, "library output");
#endif
}

class BlackBoxExec::Session {
protected:
#ifdef _WIN32
  HANDLE job;
  HANDLE process;
  HANDLE pipe_send;
  HANDLE pipe_receive;
#else
  pid_t child;
  int pipe_send;
  FILE *file_receive;
#endif
#ifdef GECODE_HAS_THREADS
  std::thread::id owner;
#endif

  static std::string last_error(const std::string &prefix) {
#ifdef _WIN32
    return prefix + " (Windows error " + std::to_string(GetLastError()) + ")";
#else
    return prefix + " (errno " + std::to_string(errno) + ")";
#endif
  }

#ifdef _WIN32
  static void close_handle(HANDLE &h) {
    if (h != NULL) {
      CloseHandle(h);
      h = NULL;
    }
  }

  static std::wstring quote_argument(const std::wstring &arg) {
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

  void open_windows(const std::string &program,
                    const std::vector<std::string> &args);
  void close_windows(void);
#else
  static bool reap_child(pid_t pid, int &status) {
    pid_t r;
    do {
      r = waitpid(pid, &status, WNOHANG);
    } while ((r == -1) && (errno == EINTR));
    return (r == pid) || ((r == -1) && (errno == ECHILD));
  }

  static bool wait_child(pid_t pid, int &status, int attempts) {
    for (int i = 0; i < attempts; i++) {
      if (reap_child(pid, status)) {
        return true;
      }
      usleep(10000);
    }
    return reap_child(pid, status);
  }

  static void terminate_child(pid_t pid) {
    if (pid <= 0) {
      return;
    }
    int status = 0;
    if (wait_child(pid, status, 100)) {
      return;
    }
    if (kill(-pid, SIGTERM) != 0) {
      kill(pid, SIGTERM);
    }
    if (wait_child(pid, status, 100)) {
      return;
    }
    if (kill(-pid, SIGKILL) != 0) {
      kill(pid, SIGKILL);
    }
    do {
      if (waitpid(pid, &status, 0) != -1) {
        return;
      }
    } while (errno == EINTR);
  }

  void open_posix(const std::string &program,
                  const std::vector<std::string> &args);
  void close_posix(void);
#endif

public:
  Session(const std::string &program, const std::vector<std::string> &args)
#ifdef GECODE_HAS_THREADS
#ifdef _WIN32
      : job(NULL), process(NULL), pipe_send(NULL), pipe_receive(NULL),
        owner(std::this_thread::get_id())
#else
      : child(-1), pipe_send(-1), file_receive(NULL),
        owner(std::this_thread::get_id())
#endif
#else
#ifdef _WIN32
      : job(NULL), process(NULL), pipe_send(NULL), pipe_receive(NULL)
#else
      : child(-1), pipe_send(-1), file_receive(NULL)
#endif
#endif
  {
#ifdef _WIN32
    open_windows(program, args);
#else
    open_posix(program, args);
#endif
  }

  ~Session(void) { close(); }

  bool owned_by_current_thread(void) const {
#ifdef GECODE_HAS_THREADS
    return owner == std::this_thread::get_id();
#else
    return true;
#endif
  }

  std::string run(const std::string &out_buf) {
#ifdef _WIN32
    size_t written = 0;
    while (written < out_buf.size()) {
      DWORD count = 0;
      DWORD remaining =
          static_cast<DWORD>(out_buf.size() - written);
      BOOL success =
          WriteFile(pipe_send, out_buf.data() + written, remaining, &count,
                    nullptr);
      if (!success || count == 0) {
        throw Error("BlackBoxExec",
                    last_error("Writing blackbox process input failed"));
      }
      written += count;
    }

    char c[2] = {0, 0};
    std::ostringstream oss;
    size_t response_size = 0;
    while (c[0] != '\n') {
      DWORD count = 0;
      BOOL success = ReadFile(pipe_receive, c, sizeof(c) - 1, &count, NULL);
      if (!success) {
        throw Error(
            "BlackBoxExec",
            "Failed to read blackbox process output from pipe");
      } else if (count == 0) {
        throw Error("BlackBoxExec",
                    "Blackbox process provided an incomplete response");
      }
      assert(count == 1);
      if (++response_size > max_exec_response_size) {
        throw Error("BlackBoxExec",
                    "Blackbox process response exceeds the size limit");
      }
      oss << c[0];
    }
    return oss.str();
#else
    const char *p = out_buf.c_str();
    size_t remaining = out_buf.size();
    while (remaining > 0) {
      ssize_t n = send_no_sigpipe(pipe_send, p, remaining);
      if (n < 0) {
        if (errno == EINTR) {
          continue;
        }
        throw Error("BlackBoxExec",
                    "Writing blackbox process input failed with errno " +
                        std::to_string(errno));
      }
      if (n == 0) {
        throw Error("BlackBoxExec",
                    "Writing blackbox process input wrote zero bytes");
      }
      p += n;
      remaining -= static_cast<size_t>(n);
    }

    std::string in_buffer;
    while (true) {
      errno = 0;
      int ch = fgetc(file_receive);
      if (ch == EOF) {
        if (feof(file_receive)) {
          throw Error("BlackBoxExec",
                      "Blackbox process provided an incomplete response");
        }
        throw Error("BlackBoxExec",
                    std::string("Reading blackbox process output from pipe "
                                "failed with errno ") +
                        std::to_string(errno));
      }
      in_buffer += static_cast<char>(ch);
      if (in_buffer.size() > max_exec_response_size) {
        throw Error("BlackBoxExec",
                    "Blackbox process response exceeds the size limit");
      }
      if (ch == '\n') {
        break;
      }
    }
    return in_buffer;
#endif
  }

  void close(void) {
#ifdef _WIN32
    close_windows();
#else
    close_posix();
#endif
  }
};

#ifdef _WIN32
void
BlackBoxExec::Session::open_windows(const std::string &program,
                                    const std::vector<std::string> &args) {
  // Build the command line before opening OS handles so allocation/conversion
  // failures cannot leak partially constructed process state.
  std::wstring program_w = utf8_to_wide(program);
  std::wstring prog = quote_argument(program_w);
  for (const std::string &a : args) {
    prog += L" ";
    prog += quote_argument(utf8_to_wide(a));
  }
  std::vector<wchar_t> cmdline(prog.begin(), prog.end());
  cmdline.push_back(L'\0');

  SIZE_T attr_size = 0;
  InitializeProcThreadAttributeList(NULL, 1, 0, &attr_size);
  std::vector<char> attr_buf(attr_size);

  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  HANDLE child_stdin_read = NULL;
  HANDLE child_stdin_write = NULL;
  HANDLE child_stdout_read = NULL;
  HANDLE child_stdout_write = NULL;
  HANDLE child_stderr_write = NULL;
  LPPROC_THREAD_ATTRIBUTE_LIST attr_list = NULL;

  auto close_startup_handles = [&]() {
    close_handle(child_stdin_read);
    close_handle(child_stdin_write);
    close_handle(child_stdout_read);
    close_handle(child_stdout_write);
    close_handle(child_stderr_write);
  };
  auto destroy_attr_list = [&]() {
    if (attr_list != NULL) {
      DeleteProcThreadAttributeList(attr_list);
      attr_list = NULL;
    }
  };

  if (!CreatePipe(&child_stdout_read, &child_stdout_write, &saAttr, 0)) {
    throw Error("BlackBoxExec", last_error("Stdout CreatePipe failed"));
  }
  if (!SetHandleInformation(child_stdout_read, HANDLE_FLAG_INHERIT, 0)) {
    DWORD err = GetLastError();
    close_startup_handles();
    throw Error("BlackBoxExec",
                windows_error("Stdout SetHandleInformation failed", err));
  }
  if (!CreatePipe(&child_stdin_read, &child_stdin_write, &saAttr, 0)) {
    DWORD err = GetLastError();
    close_startup_handles();
    throw Error("BlackBoxExec", windows_error("Stdin CreatePipe failed", err));
  }
  if (!SetHandleInformation(child_stdin_write, HANDLE_FLAG_INHERIT, 0)) {
    DWORD err = GetLastError();
    close_startup_handles();
    throw Error("BlackBoxExec",
                windows_error("Stdin SetHandleInformation failed", err));
  }

  HANDLE parent_stderr = GetStdHandle(STD_ERROR_HANDLE);
  if ((parent_stderr != NULL) && (parent_stderr != INVALID_HANDLE_VALUE)) {
    if (!DuplicateHandle(GetCurrentProcess(), parent_stderr,
                         GetCurrentProcess(), &child_stderr_write, 0, TRUE,
                         DUPLICATE_SAME_ACCESS)) {
      DWORD err = GetLastError();
      close_startup_handles();
      throw Error("BlackBoxExec",
                  windows_error("stderr DuplicateHandle failed", err));
    }
  }

  PROCESS_INFORMATION piProcInfo;
  STARTUPINFOEXW siStartInfo;
  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFOEXW));
  siStartInfo.StartupInfo.cb = sizeof(STARTUPINFOEXW);
  siStartInfo.StartupInfo.hStdOutput = child_stdout_write;
  siStartInfo.StartupInfo.hStdInput = child_stdin_read;
  siStartInfo.StartupInfo.hStdError = child_stderr_write;
  siStartInfo.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

  HANDLE inherit_handles[3] = {child_stdin_read, child_stdout_write, NULL};
  DWORD inherit_count = 2;
  if (child_stderr_write != NULL) {
    inherit_handles[inherit_count++] = child_stderr_write;
  }

  attr_list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attr_buf.data());
  siStartInfo.lpAttributeList = attr_list;
  if (!InitializeProcThreadAttributeList(attr_list, 1, 0, &attr_size)) {
    DWORD err = GetLastError();
    close_startup_handles();
    throw Error("BlackBoxExec",
                windows_error("InitializeProcThreadAttributeList failed", err));
  }
  if (!UpdateProcThreadAttribute(attr_list, 0,
                                 PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                 inherit_handles,
                                 sizeof(HANDLE) * inherit_count,
                                 NULL, NULL)) {
    DWORD err = GetLastError();
    destroy_attr_list();
    close_startup_handles();
    throw Error("BlackBoxExec",
                windows_error("PROC_THREAD_ATTRIBUTE_HANDLE_LIST failed", err));
  }

  BOOL processStarted =
      CreateProcessW(nullptr,
                     cmdline.data(),        // command line
                     nullptr,               // process security attributes
                     nullptr,               // primary thread security attributes
                     TRUE,                  // handles from attribute list
                     EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED,
                     nullptr,               // use parent's environment
                     nullptr,               // use parent's current directory
                     &siStartInfo.StartupInfo,
                     &piProcInfo);          // receives PROCESS_INFORMATION
  destroy_attr_list();

  if (!processStarted) {
    DWORD err = GetLastError();
    close_startup_handles();
    throw Error("BlackBoxExec", windows_error("Unable to start program `" +
                                             program + "'", err));
  }

  HANDLE process_job = CreateJobObjectW(NULL, NULL);
  if (process_job != NULL) {
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_info;
    ZeroMemory(&job_info, sizeof(job_info));
    job_info.BasicLimitInformation.LimitFlags =
        JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!SetInformationJobObject(process_job, JobObjectExtendedLimitInformation,
                                 &job_info, sizeof(job_info)) ||
        !AssignProcessToJobObject(process_job, piProcInfo.hProcess)) {
      CloseHandle(process_job);
      process_job = NULL;
    }
  }

  if (ResumeThread(piProcInfo.hThread) == static_cast<DWORD>(-1)) {
    DWORD err = GetLastError();
    if (process_job != NULL) {
      TerminateJobObject(process_job, 1);
    } else {
      TerminateProcess(piProcInfo.hProcess, 1);
    }
    WaitForSingleObject(piProcInfo.hProcess, 5000);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(piProcInfo.hProcess);
    close_handle(process_job);
    close_startup_handles();
    throw Error("BlackBoxExec",
                windows_error("ResumeThread failed for blackbox process", err));
  }
  CloseHandle(piProcInfo.hThread);

  close_handle(child_stdout_write);
  close_handle(child_stdin_read);
  close_handle(child_stderr_write);

  pipe_send = child_stdin_write;
  pipe_receive = child_stdout_read;
  process = piProcInfo.hProcess;
  job = process_job;
}

void
BlackBoxExec::Session::close_windows(void) {
  close_handle(pipe_send);
  close_handle(pipe_receive);
  if (process != NULL) {
    DWORD wait = WaitForSingleObject(process, 1000);
    if (wait == WAIT_TIMEOUT) {
      if (job != NULL) {
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
#else
void
BlackBoxExec::Session::open_posix(const std::string &program,
                                  const std::vector<std::string> &args) {
  const int READ = 0;
  const int WRITE = 1;
  int child_in[2] = {-1, -1};
  int child_out[2] = {-1, -1};
  if (create_socketpair(child_in) != 0) {
    throw Error("BlackBoxExec", last_error("stdin socket creation failed"));
  }
  if (create_socketpair(child_out) != 0) {
    ::close(child_in[READ]);
    ::close(child_in[WRITE]);
    throw Error("BlackBoxExec", last_error("stdout socket creation failed"));
  }
  int fds[4] = {child_in[READ], child_in[WRITE],
                child_out[READ], child_out[WRITE]};
  for (int i = 0; i < 4; i++) {
    int moved = move_from_standard_fd(fds[i]);
    if (moved == -1) {
      int e = errno;
      for (int j = 0; j < 4; j++)
        ::close(fds[j]);
      errno = e;
      throw Error("BlackBoxExec",
                  last_error("moving session descriptors away from stdio "
                             "failed"));
    }
    fds[i] = moved;
  }
  child_in[READ] = fds[0];
  child_in[WRITE] = fds[1];
  child_out[READ] = fds[2];
  child_out[WRITE] = fds[3];

  std::vector<char *> argv;
  argv.reserve(args.size() + 2);
  argv.push_back(const_cast<char *>(program.c_str()));
  for (const std::string &a : args) {
    argv.push_back(const_cast<char *>(a.c_str()));
  }
  argv.push_back(nullptr);

  posix_spawn_file_actions_t actions;
  int err = posix_spawn_file_actions_init(&actions);
  if (err != 0) {
    ::close(child_in[READ]);
    ::close(child_in[WRITE]);
    ::close(child_out[READ]);
    ::close(child_out[WRITE]);
    errno = err;
    throw Error("BlackBoxExec", last_error("spawn file action init failed"));
  }

  posix_spawnattr_t attr;
  err = posix_spawnattr_init(&attr);
  if (err != 0) {
    posix_spawn_file_actions_destroy(&actions);
    ::close(child_in[READ]);
    ::close(child_in[WRITE]);
    ::close(child_out[READ]);
    ::close(child_out[WRITE]);
    errno = err;
    throw Error("BlackBoxExec", last_error("spawn attribute init failed"));
  }

  err = posix_spawnattr_setpgroup(&attr, 0);
  if (err == 0) {
    err = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
  }
  if (err == 0) {
    err = posix_spawn_file_actions_adddup2(&actions, child_in[READ],
                                           STDIN_FILENO);
  }
  if (err == 0) {
    err = posix_spawn_file_actions_adddup2(&actions, child_out[WRITE],
                                           STDOUT_FILENO);
  }
  if (err == 0) {
    err = posix_spawn_file_actions_addclose(&actions, child_in[READ]);
  }
  if (err == 0) {
    err = posix_spawn_file_actions_addclose(&actions, child_in[WRITE]);
  }
  if (err == 0) {
    err = posix_spawn_file_actions_addclose(&actions, child_out[READ]);
  }
  if (err == 0) {
    err = posix_spawn_file_actions_addclose(&actions, child_out[WRITE]);
  }
  if (err == 0) {
    err = posix_spawnp(&child, program.c_str(), &actions, &attr, argv.data(),
                       environ);
  }
  posix_spawnattr_destroy(&attr);
  posix_spawn_file_actions_destroy(&actions);
  if (err != 0) {
    ::close(child_in[READ]);
    ::close(child_in[WRITE]);
    ::close(child_out[READ]);
    ::close(child_out[WRITE]);
    child = -1;
    errno = err;
    throw Error("BlackBoxExec", last_error("starting blackbox process failed"));
  }

  ::close(child_in[READ]);
  ::close(child_out[WRITE]);

  pipe_send = child_in[WRITE];
#ifdef SO_NOSIGPIPE
  int nosigpipe = 1;
  if (setsockopt(pipe_send, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe,
                 sizeof(nosigpipe)) != 0) {
    ::close(pipe_send);
    pipe_send = -1;
    ::close(child_out[READ]);
    terminate_child(child);
    child = -1;
    throw Error("BlackBoxExec", last_error("SO_NOSIGPIPE setup failed"));
  }
#endif
  file_receive = fdopen(child_out[READ], "r");
  if (file_receive == NULL) {
    ::close(pipe_send);
    pipe_send = -1;
    ::close(child_out[READ]);
    terminate_child(child);
    child = -1;
    throw Error("BlackBoxExec", last_error("fdopen failed"));
  }
}

void
BlackBoxExec::Session::close_posix(void) {
  if (pipe_send != -1) {
    ::close(pipe_send);
    pipe_send = -1;
  }
  if (file_receive != NULL) {
    fclose(file_receive);
    file_receive = NULL;
  }
  if (child > 0) {
    terminate_child(child);
    child = -1;
  }
}
#endif

BlackBoxExec::BlackBoxExec(const std::string &program0,
                           const std::vector<std::string> &args0)
    : program(program0), args(args0) {}

BlackBoxExec::~BlackBoxExec(void) {
  Support::Lock lock(mutex);
  for (Session *s : sessions) {
    delete s;
  }
  sessions.clear();
}

BlackBoxExec::Session &BlackBoxExec::session(void) {
  Support::Lock lock(mutex);
  for (Session *s : sessions) {
    if (s->owned_by_current_thread()) {
      return *s;
    }
  }
  std::unique_ptr<Session> s(new Session(program, args));
  Session *r = s.get();
  sessions.push_back(r);
  s.release();
  return *r;
}

void BlackBoxExec::run(const std::vector<int64_t> &int_in,
                       const std::vector<double> &float_in,
                       std::vector<int64_t> &int_out,
                       std::vector<double> &float_out) {
  // Construct program input: comma-separated integers, a semicolon, then
  // comma-separated floats, terminated by a newline (e.g. "5,-7;2.5,1.125\n").
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out.precision(std::numeric_limits<double>::max_digits10);
  for (size_t i = 0; i < int_in.size(); ++i) {
    if (i != 0) {
      out << ",";
    }
    out << int_in[i];
  }
  out << ";";
  for (size_t i = 0; i < float_in.size(); ++i) {
    if (i != 0) {
      out << ",";
    }
    out << float_in[i];
  }
  out << "\n";
  std::string out_buf = out.str();
  std::string in_buffer = session().run(out_buf);
  if (in_buffer.find('\0') != std::string::npos) {
    throw Error("BlackBoxExec",
                "Blackbox process response contains NUL data.");
  }
  // Parse the response in a single left-to-right pass: comma-separated
  // integers, a semicolon, then comma-separated floats (e.g. "5,-7;2.5,1.125\n").
  const char *p = in_buffer.c_str();
  auto skip_ws = [](const char *&q) {
    while (*q == ' ' || *q == '\t' || *q == '\r') {
      ++q;
    }
  };
  auto skip_final_ws = [](const char *&q) {
    while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') {
      ++q;
    }
  };
  auto value_end = [](const char *q) {
    while (*q != ',' && *q != ';' && *q != '\n' && *q != '\0') {
      ++q;
    }
    return q;
  };
  auto check_number_tail = [](std::istringstream &in) {
    char c;
    while (in.get(c)) {
      if (c != ' ' && c != '\t' && c != '\r') {
        return false;
      }
    }
    return true;
  };
  for (size_t i = 0; i < int_out.size(); ++i) {
    skip_ws(p);
    const char *end = value_end(p);
    std::istringstream in(std::string(p, end));
    in.imbue(std::locale::classic());
    long long v;
    if (!(in >> v) || !check_number_tail(in)) {
      throw Error("BlackBoxExec", "Failed to read output integer " +
                                      std::to_string(i) +
                                      " from blackbox process output, " +
                                      std::to_string(int_out.size()) +
                                      " integer values were expected.");
    }
    int_out[i] = checked_int(v, "blackbox process output", i);
    p = end;
    skip_ws(p);
    if (i + 1 < int_out.size()) {
      if (*p != ',') {
        throw Error("BlackBoxExec",
                    "Blackbox process response is missing an integer output "
                    "separator.");
      }
      ++p;
    }
  }
  skip_ws(p);
  if (*p != ';') {
    throw Error("BlackBoxExec",
                "Blackbox process response is missing the `;' separator between "
                "the integer and floating point outputs.");
  }
  ++p;
  for (size_t i = 0; i < float_out.size(); ++i) {
    skip_ws(p);
    const char *end = value_end(p);
    std::istringstream in(std::string(p, end));
    in.imbue(std::locale::classic());
    double v;
    if (!(in >> v) || !check_number_tail(in)) {
      throw Error("BlackBoxExec", "Failed to read output float " +
                                      std::to_string(i) +
                                      " from blackbox process output, " +
                                      std::to_string(float_out.size()) +
                                      " floating point values were expected.");
    }
#ifdef GECODE_HAS_FLOAT_VARS
    check_float(v, "blackbox process output", i);
#endif
    float_out[i] = v;
    p = end;
    skip_ws(p);
    if (i + 1 < float_out.size()) {
      if (*p != ',') {
        throw Error("BlackBoxExec",
                    "Blackbox process response is missing a floating point "
                    "output separator.");
      }
      ++p;
    }
  }
  skip_final_ws(p);
  if (*p != '\0') {
    throw Error("BlackBoxExec",
                "Blackbox process response contains trailing data.");
  }
}

ExecStatus BlackBox::propagate(Space &home, const ModEventDelta &) {
  if (int_input.assigned()
#ifdef GECODE_HAS_FLOAT_VARS
      && float_input.assigned()
#endif
  ) {
    std::vector<int64_t> int_in(int_input.size());
    std::vector<int64_t> int_out(int_output.size());
    // std::cerr << "Black Box Fn input: ";
    for (int i = 0; i < int_in.size(); i++) {
      int_in[i] = int_input[i].val();
    }
    std::vector<double> float_in;
    std::vector<double> float_out;
#ifdef GECODE_HAS_FLOAT_VARS
    float_in.resize(float_input.size());
    float_out.resize(float_output.size());
    for (int i = 0; i < float_in.size(); i++) {
      float_in[i] = float_input[i].val().med();
    }
#endif

    black_box()->run(int_in, float_in, int_out, float_out);

    for (int i = 0; i < int_out.size(); i++) {
      // std::cerr << int_out[i] << " ";
      GECODE_ME_CHECK(int_output[i].eq(home, static_cast<int>(int_out[i])));
    }
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < float_out.size(); i++) {
      GECODE_ME_CHECK(float_output[i].eq(home, float_out[i]));
    }
#endif

    return home.ES_SUBSUMED(*this);
  }
  return ES_FIX;
}

ExecStatus BlackBoxBounds::propagate(Space &home, const ModEventDelta &) {
  std::vector<int64_t> int_in(ivar.size() * 2);
  std::vector<int64_t> int_out(ivar.size() * 2);
  // std::cerr << "Black Box Bounds Fn input: ";
  for (int i = 0; i < ivar.size(); i++) {
    int_in[i*2] = ivar[i].min();
    int_in[i*2+1] = ivar[i].max();
  }
  std::vector<double> float_in;
  std::vector<double> float_out;
#ifdef GECODE_HAS_FLOAT_VARS
  float_in.resize(fvar.size() * 2);
  float_out.resize(fvar.size() * 2);
  for (int i = 0; i < fvar.size(); i++) {
    float_in[i*2] = fvar[i].min();
    float_in[i*2+1] = fvar[i].max();
  }
#endif

  black_box()->run(int_in, float_in, int_out, float_out);

  for (int i = 0; i < ivar.size(); i++) {
    // std::cerr << int_out[i*2] << ".." << int_out[i*2+1] << " ";
    GECODE_ME_CHECK(ivar[i].gq(home, static_cast<int>(int_out[i*2])));
    GECODE_ME_CHECK(ivar[i].lq(home, static_cast<int>(int_out[i*2+1])));
  }
#ifdef GECODE_HAS_FLOAT_VARS
  for (int i = 0; i < fvar.size(); i++) {
    GECODE_ME_CHECK(fvar[i].gq(home, float_out[i*2]));
    GECODE_ME_CHECK(fvar[i].lq(home, float_out[i*2+1]));
  }
#endif

  return ES_NOFIX;
}

void blackbox(Home home, const IntVarArgs &int_in, const IntVarArgs &int_out,
#ifdef GECODE_HAS_FLOAT_VARS
              const FloatVarArgs &float_in, const FloatVarArgs &float_out,
#endif
              const std::string &mode, const std::string &instantiation,
              const std::vector<std::string> &args) {
  ViewArray<Int::IntView> int_input(home, int_in);
  ViewArray<Int::IntView> int_output(home, int_out);
#ifdef GECODE_HAS_FLOAT_VARS
  ViewArray<Float::FloatView> float_input(home, float_in);
  ViewArray<Float::FloatView> float_output(home, float_out);
#endif

  if (home.failed())
    return;
  PostInfo pi(home);
  ExecStatus es = BlackBox::post(home, int_input, int_output,
#ifdef GECODE_HAS_FLOAT_VARS
                                 float_input, float_output,
#endif
                                 mode, instantiation, args);
  GECODE_ES_FAIL(es);
}

/// Parse the flat reason and mark, per channel, the variables whose bounds the
/// propagator depends on (the variables that appear as literals in any reason).
/// \a sub_int / \a sub_float are filled with one boolean per variable. Variable
/// indices in the reason are 1-based over the combined variable list, integer
/// variables first, then float variables.
///
/// The flat reason is a concatenation of one entry per variable, each entry
/// being `[idx, |R_lb|, (var, bnd)..., |R_ub|, (var, bnd)...]`.
static void reason_subscriptions(const std::vector<int> &reason, int n_int,
                                 int n_float, SharedArray<bool> &sub_int,
                                 SharedArray<bool> &sub_float) {
  for (int i = 0; i < n_int; i++) {
    sub_int[i] = false;
  }
  for (int i = 0; i < n_float; i++) {
    sub_float[i] = false;
  }

  const int n_total = n_int + n_float;
  std::vector<bool> explained(n_total, false);
  size_t pos = 0;
  auto require = [&](size_t n, const char *what) {
    if (reason.size() - pos < n) {
      throw Error("Blackbox", std::string("Malformed blackbox bounds reason: ") +
                                  what + ".");
    }
  };
  while (pos < reason.size()) {
    require(1, "missing explained variable index");
    int idx = reason[pos++];
    if ((idx < 1) || (idx > n_total)) {
      throw Error("Blackbox",
                  "Malformed blackbox bounds reason: explained variable index "
                  "is out of range.");
    }
    if (explained[idx - 1]) {
      throw Error("Blackbox",
                  "Malformed blackbox bounds reason: duplicate explained "
                  "variable index.");
    }
    explained[idx - 1] = true;
    for (int side = 0; side < 2; side++) { // lower- then upper-bound literals
      require(1, "missing reason literal count");
      int count = reason[pos++];
      if (count < 0) {
        throw Error("Blackbox",
                    "Malformed blackbox bounds reason: negative literal count.");
      }
      if (static_cast<size_t>(count) > (reason.size() - pos) / 2) {
        throw Error("Blackbox",
                    "Malformed blackbox bounds reason: truncated reason "
                    "literals.");
      }
      for (int k = 0; k < count; k++) {
        int var = reason[pos++]; // 1-based combined variable index
        int bnd = reason[pos++];
        if (var >= 1 && var <= n_int) {
          sub_int[var - 1] = true;
        } else if (var > n_int && var <= n_int + n_float) {
          sub_float[var - 1 - n_int] = true;
        } else {
          throw Error("Blackbox",
                      "Malformed blackbox bounds reason: dependency variable "
                      "index is out of range.");
        }
        if (bnd != 1 && bnd != 2) { // MiniZinc PropBnd: PR_LB, PR_UB
          throw Error("Blackbox",
                      "Malformed blackbox bounds reason: dependency bound "
                      "code is out of range.");
        }
      }
    }
  }
  for (int i = 0; i < n_total; i++) {
    if (!explained[i]) {
      throw Error("Blackbox",
                  "Malformed blackbox bounds reason: missing explained "
                  "variable entry.");
    }
  }
}

void blackbox_bounds(Home home, const IntVarArgs &ivar,
#ifdef GECODE_HAS_FLOAT_VARS
              const FloatVarArgs &fvar,
#endif
              const std::string &mode, const std::string &instantiation,
              const std::vector<std::string> &args,
              const std::vector<int> &reason) {
  ViewArray<Int::IntView> int_var(home, ivar);
#ifdef GECODE_HAS_FLOAT_VARS
  ViewArray<Float::FloatView> float_var(home, fvar);
  int n_float = fvar.size();
#else
  int n_float = 0;
#endif

  // Determine which variables the propagator depends on, so it is only
  // subscribed (and thus scheduled) on the bounds mentioned in the reason. The
  // marking is constant and shared between all copies of the propagator.
  SharedArray<bool> sub_int(ivar.size());
  SharedArray<bool> sub_float(n_float);
  reason_subscriptions(reason, ivar.size(), n_float, sub_int, sub_float);

  if (home.failed())
    return;
  PostInfo pi(home);
  ExecStatus es = BlackBoxBounds::post(home, int_var,
#ifdef GECODE_HAS_FLOAT_VARS
                                 float_var,
#endif
                                 sub_int,
#ifdef GECODE_HAS_FLOAT_VARS
                                 sub_float,
#endif
                                 mode, instantiation, args);
  GECODE_ES_FAIL(es);
}

} // namespace FlatZinc
} // namespace Gecode
