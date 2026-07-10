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

#if defined(_WIN32)
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#if !defined(WINVER) || (WINVER < 0x0600)
#undef WINVER
#define WINVER 0x0600
#endif
#elif !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif

#include <gecode/kernel.hh>
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/blackbox.hh>

#include <atomic>
#include <cassert>
#include <cerrno>
#include <charconv>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <exception>
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
#ifdef GECODE_HAS_POSIX_BLACKBOX_EXEC
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
extern char **environ;
#endif
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

class WindowsHandle {
private:
  HANDLE handle;
public:
  explicit WindowsHandle(HANDLE handle0=NULL) : handle(handle0) {}
  ~WindowsHandle(void) { reset(); }

  WindowsHandle(const WindowsHandle &) = delete;
  WindowsHandle &operator=(const WindowsHandle &) = delete;

  HANDLE get(void) const { return handle; }
  HANDLE *put(void) {
    reset();
    return &handle;
  }
  HANDLE release(void) {
    HANDLE handle0 = handle;
    handle = NULL;
    return handle0;
  }
  bool valid(void) const {
    return (handle != NULL) && (handle != INVALID_HANDLE_VALUE);
  }
  void reset(HANDLE handle0=NULL) {
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
  WindowsAttributeList(void) : list(NULL), initialized(false) {}
  ~WindowsAttributeList(void) {
    if (initialized) {
      DeleteProcThreadAttributeList(list);
    }
  }

  void init(void) {
    SIZE_T size = 0;
    InitializeProcThreadAttributeList(NULL, 1, 0, &size);
    if (size == 0) {
      throw Error("BlackBoxExec",
                  windows_error("ProcThreadAttributeList size query failed",
                                GetLastError()));
    }
    buffer.resize(size);
    list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
    if (!InitializeProcThreadAttributeList(list, 1, 0, &size)) {
      throw Error("BlackBoxExec",
                  windows_error("InitializeProcThreadAttributeList failed",
                                GetLastError()));
    }
    initialized = true;
  }

  void set_inherited_handles(HANDLE *handles, DWORD count) {
    if (!UpdateProcThreadAttribute(list, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                   handles, sizeof(HANDLE) * count, NULL,
                                   NULL)) {
      throw Error("BlackBoxExec",
                  windows_error("PROC_THREAD_ATTRIBUTE_HANDLE_LIST failed",
                                GetLastError()));
    }
  }

  LPPROC_THREAD_ATTRIBUTE_LIST get(void) const { return list; }
};

bool
has_dll_suffix(const std::string &name) {
  if (name.size() < 4) {
    return false;
  }
  const char *suffix = ".dll";
  for (size_t i = 0; i < 4; i++) {
    if (std::tolower(static_cast<unsigned char>(name[name.size() - 4 + i])) !=
        suffix[i]) {
      return false;
    }
  }
  return true;
}

std::vector<std::string>
dll_candidates(const std::string &name) {
  std::vector<std::string> candidates;
  candidates.push_back(name);
  if (!has_dll_suffix(name)) {
    candidates.push_back(name + ".dll");
  }
  const size_t separator = name.find_last_of("\\/");
  const std::string directory =
      (separator == std::string::npos) ? std::string() :
      name.substr(0, separator + 1);
  const std::string basename =
      (separator == std::string::npos) ? name : name.substr(separator + 1);
  if (basename.compare(0, 3, "lib") != 0) {
    std::string prefixed = directory + "lib" + basename;
    if (!has_dll_suffix(prefixed)) {
      prefixed += ".dll";
    }
    candidates.push_back(prefixed);
  }
  return candidates;
}

bool
qualified_path(const std::wstring &program) {
  return (program.find_first_of(L"\\/") != std::wstring::npos) ||
         ((program.size() > 1) && (program[1] == L':'));
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

#ifdef GECODE_HAS_POSIX_BLACKBOX_EXEC
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

class FileDescriptor {
private:
  int fd;
public:
  explicit FileDescriptor(int fd0=-1) : fd(fd0) {}
  ~FileDescriptor(void) { reset(); }

  int get(void) const { return fd; }
  int release(void) {
    int fd0 = fd;
    fd = -1;
    return fd0;
  }
  void reset(int fd0=-1) {
    if (fd != -1) {
      ::close(fd);
    }
    fd = fd0;
  }
};

int
move_away_from_standard_fd(FileDescriptor &fd) {
  int old = fd.release();
  int nfd = move_from_standard_fd(old);
  if (nfd == -1) {
    fd.reset(old);
  } else {
    fd.reset(nfd);
  }
  return nfd;
}

class SpawnFileActions {
private:
  posix_spawn_file_actions_t actions;
  bool initialized;
public:
  SpawnFileActions(void) : initialized(false) {}
  ~SpawnFileActions(void) {
    if (initialized) {
      posix_spawn_file_actions_destroy(&actions);
    }
  }

  int init(void) {
    int err = posix_spawn_file_actions_init(&actions);
    initialized = err == 0;
    return err;
  }
  posix_spawn_file_actions_t *get(void) { return &actions; }
};

class SpawnAttributes {
private:
  posix_spawnattr_t attr;
  bool initialized;
public:
  SpawnAttributes(void) : initialized(false) {}
  ~SpawnAttributes(void) {
    if (initialized) {
      posix_spawnattr_destroy(&attr);
    }
  }

  int init(void) {
    int err = posix_spawnattr_init(&attr);
    initialized = err == 0;
    return err;
  }
  posix_spawnattr_t *get(void) { return &attr; }
};

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
#endif

template<class T>
T
library_symbol(void *library, const char *name, unsigned int stdcall_bytes) {
#ifdef _WIN32
  FARPROC symbol = GetProcAddress(static_cast<HMODULE>(library), name);
#if defined(_M_IX86) || defined(__i386__)
  if (symbol == nullptr) {
    const std::string decorated =
        std::string("_") + name + "@" + std::to_string(stdcall_bytes);
    symbol = GetProcAddress(static_cast<HMODULE>(library), decorated.c_str());
  }
  if (symbol == nullptr) {
    const std::string decorated =
        std::string(name) + "@" + std::to_string(stdcall_bytes);
    symbol = GetProcAddress(static_cast<HMODULE>(library), decorated.c_str());
  }
#else
  (void)stdcall_bytes;
#endif
  return reinterpret_cast<T>(symbol);
#else
  (void)stdcall_bytes;
  T symbol = nullptr;
  *(void **)(&symbol) = dlsym(library, name);
  return symbol;
#endif
}

void
check_int(int64_t v, const char *source, size_t i) {
  if (!Int::Limits::valid(static_cast<long long int>(v))) {
    throw Error("Blackbox", std::string(source) + " integer " +
                              std::to_string(i) +
                              " is outside Gecode's integer range");
  }
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

#ifdef GECODE_HAS_THREADS
class BlackBoxLibrary::Instance {
public:
  std::thread::id owner;
  void *value;
  Support::Mutex mutex;

  Instance(const std::thread::id &owner0, void *value0)
      : owner(owner0), value(value0) {}
};
#endif

BlackBoxLibrary::BlackBoxLibrary(const std::string &name,
                                 const std::vector<std::string> &args)
    : library(nullptr), library_fzn_init(nullptr), library_fzn_clone(nullptr),
      library_fzn_blackbox(nullptr), library_fzn_free(nullptr),
      root_instance(nullptr)
{
  std::string loadError;
  void *loaded = nullptr;
#ifdef _WIN32
  DWORD err = ERROR_FILE_NOT_FOUND;
  std::string failed_candidate = name;
  for (const std::string &candidate : dll_candidates(name)) {
    loaded = LoadLibraryW(utf8_to_wide(candidate).c_str());
    if (loaded != nullptr) {
      break;
    }
    failed_candidate = candidate;
    err = GetLastError();
  }
  if (loaded == nullptr) {
    loadError = std::string("unable to locate library `") + name + "' (" +
                windows_error("LoadLibraryW failed for `" + failed_candidate +
                              "'", err) + ")";
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

  bool root_initialized = false;
  try {
    // fzn_blackbox is the only required entry point.
#ifndef _WIN32
    dlerror();
#endif
    library_fzn_blackbox =
        library_symbol<decltype(library_fzn_blackbox)>(loaded, "fzn_blackbox",
                                                        36);
    std::string symError(".");
    if (library_fzn_blackbox == nullptr) {
#ifdef _WIN32
      symError += " (" +
          windows_error("GetProcAddress failed", GetLastError()) + ")";
#else
      const char *error = dlerror();
      if (error != nullptr) {
        symError += std::string(": ") + error;
      }
#endif
      throw Error("Blackbox",
                  "Unable to find symbol `fzn_blackbox` in dynamic library" +
                      symError);
    }

    library_fzn_init =
        library_symbol<decltype(library_fzn_init)>(loaded, "fzn_init", 8);
    library_fzn_clone =
        library_symbol<decltype(library_fzn_clone)>(loaded, "fzn_clone", 4);
    library_fzn_free =
        library_symbol<decltype(library_fzn_free)>(loaded, "fzn_free", 4);
    if ((library_fzn_init != nullptr) && (library_fzn_clone == nullptr)) {
      throw Error("Blackbox",
                  "Dynamic library exports `fzn_init` but not `fzn_clone`");
    }
    if (library_fzn_init != nullptr) {
      std::vector<const char *> argv;
      argv.reserve(args.size());
      for (const std::string &arg : args) {
        argv.push_back(arg.c_str());
      }
      root_instance = library_fzn_init(argv.data(), argv.size());
      root_initialized = true;
    }
    library = loaded;
  } catch (...) {
    if (root_initialized && (library_fzn_free != nullptr)) {
      try {
        library_fzn_free(root_instance);
      } catch (...) {}
    }
    close_library(loaded);
    throw;
  }
}

BlackBoxLibrary::~BlackBoxLibrary() {
  if ((library_fzn_init != nullptr) && (library_fzn_free != nullptr)) {
#ifdef GECODE_HAS_THREADS
    for (Instance *instance : instances) {
      try {
        library_fzn_free(instance->value);
      } catch (...) {}
    }
#endif
    try {
      library_fzn_free(root_instance);
    } catch (...) {}
  }
#ifdef GECODE_HAS_THREADS
  for (Instance *instance : instances) {
    delete instance;
  }
#endif
  close_library(library);
}

#ifdef GECODE_HAS_THREADS
BlackBoxLibrary::Instance *
BlackBoxLibrary::instance(void) {
  const std::thread::id owner = std::this_thread::get_id();
  Support::Lock lock(mutex);
  for (Instance *instance : instances) {
    if (instance->owner == owner) {
      return instance;
    }
  }
  void *value = library_fzn_clone(root_instance);
  Instance *clone = nullptr;
  try {
    clone = new Instance(owner, value);
    instances.push_back(clone);
    return clone;
  } catch (...) {
    delete clone;
    if (library_fzn_free != nullptr) {
      try {
        library_fzn_free(value);
      } catch (...) {}
    }
    throw;
  }
}
#endif

void
BlackBoxLibrary::run(const std::vector<int64_t> &int_in,
                     const std::vector<double> &float_in,
                     std::vector<int64_t> &int_out,
                     std::vector<double> &float_out) {
#ifdef GECODE_HAS_THREADS
  if (library_fzn_init != nullptr) {
    Instance *selected = this->instance();
    Support::Lock lock(selected->mutex);
    library_fzn_blackbox(selected->value, int_in.data(), int_in.size(),
                         float_in.data(), float_in.size(), int_out.data(),
                         int_out.size(), float_out.data(), float_out.size());
  } else {
    library_fzn_blackbox(nullptr, int_in.data(), int_in.size(),
                         float_in.data(), float_in.size(), int_out.data(),
                         int_out.size(),
                         float_out.data(), float_out.size());
  }
#else
  library_fzn_blackbox(root_instance, int_in.data(), int_in.size(),
                       float_in.data(), float_in.size(), int_out.data(),
                       int_out.size(),
                       float_out.data(), float_out.size());
#endif
  for (size_t i = 0; i < int_out.size(); ++i) {
    check_int(int_out[i], "library output", i);
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
#elif defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
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
#elif defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
    return prefix + " (errno " + std::to_string(errno) + ")";
#else
    return prefix;
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
#elif defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
  static void sleep_grace_period(void) {
    struct timespec remaining = {0, 10000000};
    while ((nanosleep(&remaining, &remaining) == -1) && (errno == EINTR)) {}
  }

  static bool child_exited(pid_t pid) {
    siginfo_t info;
    do {
      info.si_pid = 0;
      if (waitid(P_PID, pid, &info, WEXITED | WNOHANG | WNOWAIT) == 0) {
        return info.si_pid != 0;
      }
    } while (errno == EINTR);
    return false;
  }

  static void signal_group(pid_t pid, int signal) {
    if ((kill(-pid, signal) == -1) && (errno == ESRCH)) {
      return;
    }
  }

  static void wait_group(pid_t pid, int attempts) {
    for (int i = 0; i < attempts; i++) {
      if ((kill(-pid, 0) == -1) && (errno == ESRCH)) {
        return;
      }
      if (child_exited(pid)) {
        return;
      }
      sleep_grace_period();
    }
  }

  static void terminate_child(pid_t pid) {
    if (pid <= 0) {
      return;
    }
    int status = 0;
    // Keep the child unreaped until the group has received both signals.
    signal_group(pid, SIGTERM);
    wait_group(pid, 100);
    signal_group(pid, SIGKILL);
    do {
      if (waitpid(pid, &status, 0) != -1) {
        return;
      }
    } while (errno == EINTR);
  }

  static void check_sigchld(void) {
    struct sigaction action;
    if (sigaction(SIGCHLD, NULL, &action) != 0) {
      throw Error("BlackBoxExec", last_error("SIGCHLD query failed"));
    }
    if ((action.sa_handler != SIG_DFL)
#ifdef SA_NOCLDWAIT
        || (action.sa_flags & SA_NOCLDWAIT)
#endif
       ) {
      throw Error("BlackBoxExec",
                  "Cannot start a blackbox process unless SIGCHLD uses "
                  "SIG_DFL without SA_NOCLDWAIT");
    }
  }

  void open_posix(const std::string &program,
                  const std::vector<std::string> &args);
  void close_posix(void);
#endif

public:
  Session(const std::string &program, const std::vector<std::string> &args)
#ifdef _WIN32
      : job(NULL), process(NULL), pipe_send(NULL), pipe_receive(NULL)
#elif defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
      : child(-1), pipe_send(-1), file_receive(NULL)
#endif
  {
#ifdef GECODE_HAS_THREADS
    owner = std::this_thread::get_id();
#endif
#ifdef _WIN32
    open_windows(program, args);
#elif defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
    open_posix(program, args);
#else
    (void)program;
    (void)args;
    throw Error("BlackBoxExec",
                "Persistent process blackboxes are not supported on this "
                "platform");
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
#elif defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
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
        int err = errno;
        if (err == EINTR) {
          clearerr(file_receive);
          continue;
        }
        throw Error("BlackBoxExec",
                    std::string("Reading blackbox process output from pipe "
                                "failed with errno ") +
                        std::to_string(err));
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
#else
    (void)out_buf;
    throw Error("BlackBoxExec",
                "Persistent process blackboxes are not supported on this "
                "platform");
#endif
  }

  void close(void) {
#ifdef _WIN32
    close_windows();
#elif defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
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

  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  WindowsHandle child_stdin_read;
  WindowsHandle child_stdin_write;
  WindowsHandle child_stdout_read;
  WindowsHandle child_stdout_write;
  WindowsHandle child_stderr_write;
  if (!CreatePipe(child_stdout_read.put(), child_stdout_write.put(), &saAttr,
                  0)) {
    throw Error("BlackBoxExec", last_error("Stdout CreatePipe failed"));
  }
  if (!SetHandleInformation(child_stdout_read.get(), HANDLE_FLAG_INHERIT, 0)) {
    throw Error("BlackBoxExec",
                last_error("Stdout SetHandleInformation failed"));
  }
  if (!CreatePipe(child_stdin_read.put(), child_stdin_write.put(), &saAttr,
                  0)) {
    throw Error("BlackBoxExec", last_error("Stdin CreatePipe failed"));
  }
  if (!SetHandleInformation(child_stdin_write.get(), HANDLE_FLAG_INHERIT, 0)) {
    throw Error("BlackBoxExec",
                last_error("Stdin SetHandleInformation failed"));
  }

  HANDLE parent_stderr = GetStdHandle(STD_ERROR_HANDLE);
  if ((parent_stderr != NULL) && (parent_stderr != INVALID_HANDLE_VALUE)) {
    if (!DuplicateHandle(GetCurrentProcess(), parent_stderr,
                         GetCurrentProcess(), child_stderr_write.put(), 0, TRUE,
                         DUPLICATE_SAME_ACCESS)) {
      throw Error("BlackBoxExec",
                  last_error("stderr DuplicateHandle failed"));
    }
  } else {
    HANDLE nul = CreateFileW(L"NUL", GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, &saAttr,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (nul == INVALID_HANDLE_VALUE) {
      throw Error("BlackBoxExec", last_error("stderr NUL CreateFile failed"));
    }
    child_stderr_write.reset(nul);
  }

  WindowsAttributeList attr_list;
  attr_list.init();
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFOEXW siStartInfo;
  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFOEXW));
  siStartInfo.StartupInfo.cb = sizeof(STARTUPINFOEXW);
  siStartInfo.StartupInfo.hStdOutput = child_stdout_write.get();
  siStartInfo.StartupInfo.hStdInput = child_stdin_read.get();
  siStartInfo.StartupInfo.hStdError = child_stderr_write.get();
  siStartInfo.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

  HANDLE inherit_handles[3] = {child_stdin_read.get(), child_stdout_write.get(),
                               child_stderr_write.get()};
  attr_list.set_inherited_handles(inherit_handles, 3);
  siStartInfo.lpAttributeList = attr_list.get();

  WindowsHandle process_job(CreateJobObjectW(NULL, NULL));
  if (!process_job.valid()) {
    throw Error("BlackBoxExec", last_error("CreateJobObject failed"));
  }
  JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_info;
  ZeroMemory(&job_info, sizeof(job_info));
  job_info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
  if (!SetInformationJobObject(process_job.get(),
                               JobObjectExtendedLimitInformation, &job_info,
                               sizeof(job_info))) {
    throw Error("BlackBoxExec", last_error("SetInformationJobObject failed"));
  }

  BOOL processStarted =
      CreateProcessW(qualified_path(program_w) ? program_w.c_str() : NULL,
                     cmdline.data(),        // command line
                     nullptr,               // process security attributes
                     nullptr,               // primary thread security attributes
                     TRUE,                  // handles from attribute list
                     EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED,
                     nullptr,               // use parent's environment
                     nullptr,               // use parent's current directory
                     &siStartInfo.StartupInfo,
                     &piProcInfo);          // receives PROCESS_INFORMATION

  if (!processStarted) {
    throw Error("BlackBoxExec", windows_error("Unable to start program `" +
                                             program + "'", GetLastError()));
  }
  WindowsHandle process_handle(piProcInfo.hProcess);
  WindowsHandle thread_handle(piProcInfo.hThread);
  if (!AssignProcessToJobObject(process_job.get(), process_handle.get())) {
    DWORD err = GetLastError();
    DWORD terminate_err = ERROR_SUCCESS;
    if (!TerminateProcess(process_handle.get(), 1)) {
      terminate_err = GetLastError();
    }
    DWORD wait = WaitForSingleObject(process_handle.get(), 5000);
    std::string message = windows_error(
        "Unable to assign blackbox process to required job", err);
    if (terminate_err != ERROR_SUCCESS) {
      message += "; " + windows_error("TerminateProcess cleanup failed",
                                       terminate_err);
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
    std::string message = windows_error(
        "ResumeThread failed for blackbox process", err);
    if (terminate_err != ERROR_SUCCESS) {
      message += "; " + windows_error("TerminateJobObject cleanup failed",
                                       terminate_err);
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
#elif defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
void
BlackBoxExec::Session::open_posix(const std::string &program,
                                  const std::vector<std::string> &args) {
  const int READ = 0;
  const int WRITE = 1;

  std::vector<char *> argv;
  argv.reserve(args.size() + 2);
  argv.push_back(const_cast<char *>(program.c_str()));
  for (const std::string &a : args) {
    argv.push_back(const_cast<char *>(a.c_str()));
  }
  argv.push_back(nullptr);

  check_sigchld();

  FileDescriptor child_in[2];
  FileDescriptor child_out[2];
  int fds[2];
  if (create_socketpair(fds) != 0) {
    throw Error("BlackBoxExec", last_error("stdin socket creation failed"));
  }
  child_in[READ].reset(fds[READ]);
  child_in[WRITE].reset(fds[WRITE]);
  if (create_socketpair(fds) != 0) {
    throw Error("BlackBoxExec", last_error("stdout socket creation failed"));
  }
  child_out[READ].reset(fds[READ]);
  child_out[WRITE].reset(fds[WRITE]);
  FileDescriptor *session_fds[] = {
    &child_in[READ], &child_in[WRITE],
    &child_out[READ], &child_out[WRITE]
  };
  for (FileDescriptor *fd : session_fds) {
    if (move_away_from_standard_fd(*fd) == -1) {
      throw Error("BlackBoxExec",
                  last_error("moving session descriptors away from stdio "
                             "failed"));
    }
  }

  SpawnFileActions actions;
  int err = actions.init();
  if (err != 0) {
    errno = err;
    throw Error("BlackBoxExec", last_error("spawn file action init failed"));
  }

  SpawnAttributes attr;
  err = attr.init();
  if (err != 0) {
    errno = err;
    throw Error("BlackBoxExec", last_error("spawn attribute init failed"));
  }

  err = posix_spawnattr_setpgroup(attr.get(), 0);
  if (err == 0) {
    short flags = POSIX_SPAWN_SETPGROUP;
#if defined(GECODE_HAS_POSIX_SPAWN_CLOEXEC_DEFAULT) && \
    defined(GECODE_HAS_POSIX_SPAWN_ADDINHERIT_NP)
    flags |= POSIX_SPAWN_CLOEXEC_DEFAULT;
#endif
    err = posix_spawnattr_setflags(attr.get(), flags);
  }
  if (err == 0) {
    err = posix_spawn_file_actions_adddup2(actions.get(),
                                           child_in[READ].get(),
                                           STDIN_FILENO);
  }
  if (err == 0) {
    err = posix_spawn_file_actions_adddup2(actions.get(),
                                           child_out[WRITE].get(),
                                           STDOUT_FILENO);
  }
#if defined(GECODE_HAS_POSIX_SPAWN_CLOEXEC_DEFAULT) && \
    defined(GECODE_HAS_POSIX_SPAWN_ADDINHERIT_NP)
  if (err == 0) {
    err = posix_spawn_file_actions_addinherit_np(actions.get(),
                                                  STDERR_FILENO);
  }
#elif defined(GECODE_HAS_POSIX_SPAWN_ADDCLOSEFROM_NP)
  if (err == 0) {
    err = posix_spawn_file_actions_addclosefrom_np(actions.get(),
                                                    STDERR_FILENO + 1);
  }
#endif
  if (err == 0) {
    err = posix_spawnp(&child, program.c_str(), actions.get(), attr.get(),
                       argv.data(), environ);
  }
  if (err != 0) {
    child = -1;
    errno = err;
    throw Error("BlackBoxExec", last_error("starting blackbox process failed"));
  }

  child_in[READ].reset();
  child_out[WRITE].reset();

#ifdef SO_NOSIGPIPE
  int nosigpipe = 1;
  if (setsockopt(child_in[WRITE].get(), SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe,
                 sizeof(nosigpipe)) != 0) {
    int e = errno;
    terminate_child(child);
    child = -1;
    errno = e;
    throw Error("BlackBoxExec", last_error("SO_NOSIGPIPE setup failed"));
  }
#endif
  FILE *receive = fdopen(child_out[READ].get(), "r");
  if (receive == NULL) {
    int e = errno;
    terminate_child(child);
    child = -1;
    errno = e;
    throw Error("BlackBoxExec", last_error("fdopen failed"));
  }
  file_receive = receive;
  child_out[READ].release();
  pipe_send = child_in[WRITE].release();
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

class BlackBoxState : public SharedHandle::Object {
protected:
  class ExecEntry {
  public:
    std::string program;
    std::vector<std::string> args;
    BlackBoxHandle handle;
    ExecEntry(const std::string &program0, const std::vector<std::string> &args0,
              const BlackBoxHandle &handle0)
        : program(program0), args(args0), handle(handle0) {}
  };
  mutable Support::Mutex mutex;
  std::vector<ExecEntry> exec;
  std::exception_ptr exception;
  std::atomic<bool> error_recorded;

public:
  BlackBoxState(void) : error_recorded(false) {}
  BlackBoxHandle blackBox(const std::string &mode,
                          const std::string &instantiation,
                          const std::vector<std::string> &args);
  void fail(std::exception_ptr e);
  bool failed(void) const;
  void rethrow(void) const;
};

BlackBoxStateHandle
BlackBoxStateHandle::init(SharedHandle &handle) {
  BlackBoxStateHandle state(handle);
  if (!state) {
    state.object(new BlackBoxState);
    handle = state;
  }
  return state;
}

BlackBoxHandle
BlackBoxStateHandle::blackBox(const std::string &mode,
                              const std::string &instantiation,
                              const std::vector<std::string> &args) const {
  return static_cast<BlackBoxState *>(object())->blackBox(mode, instantiation,
                                                           args);
}

void
BlackBoxStateHandle::fail(std::exception_ptr e) const {
  static_cast<BlackBoxState *>(object())->fail(e);
}

bool
BlackBoxStateHandle::failed(void) const {
  return static_cast<bool>(*this) &&
    static_cast<BlackBoxState *>(object())->failed();
}

void
BlackBoxStateHandle::rethrow(void) const {
  if (*this) {
    static_cast<BlackBoxState *>(object())->rethrow();
  }
}

BlackBoxHandle
BlackBoxState::blackBox(const std::string &mode,
                        const std::string &instantiation,
                        const std::vector<std::string> &args) {
  if (mode == "exec") {
    Support::Lock lock(mutex);
    for (const ExecEntry &e : exec) {
      if ((e.program == instantiation) && (e.args == args)) {
        return e.handle;
      }
    }
    BlackBoxHandle handle(new BlackBoxExec(instantiation, args));
    exec.push_back(ExecEntry(instantiation, args, handle));
    return handle;
  }
  if (mode == "dll") {
    return BlackBoxHandle(new BlackBoxLibrary(instantiation, args));
  }
  throw Error("Blackbox", "Unknown blackbox protocol `" + mode + "'");
}

void
BlackBoxState::fail(std::exception_ptr e) {
  Support::Lock lock(mutex);
  if (!error_recorded.load(std::memory_order_relaxed)) {
    exception = e;
    error_recorded.store(true, std::memory_order_release);
  }
}

bool
BlackBoxState::failed(void) const {
  return error_recorded.load(std::memory_order_acquire);
}

void
BlackBoxState::rethrow(void) const {
  if (!error_recorded.load(std::memory_order_acquire)) {
    return;
  }
  std::exception_ptr e;
  {
    Support::Lock lock(mutex);
    e = exception;
  }
  if (e != nullptr) {
    std::rethrow_exception(e);
  }
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
  auto check_integer_tail = [](const char *q, const char *end) {
    while (q != end) {
      if (*q != ' ' && *q != '\t' && *q != '\r') {
        return false;
      }
      ++q;
    }
    return true;
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
    const char *integer = p;
    if (*integer == '+') {
      ++integer;
    }
    int64_t value;
    const std::from_chars_result parsed =
      std::from_chars(integer, end, value);
    if ((parsed.ptr == integer) || (parsed.ec != std::errc()) ||
        !check_integer_tail(parsed.ptr, end)) {
      throw Error("BlackBoxExec", "Failed to read output integer " +
                                      std::to_string(i) +
                                      " from blackbox process output, " +
                                      std::to_string(int_out.size()) +
                                      " integer values were expected.");
    }
    check_int(value, "blackbox process output", i);
    int_out[i] = value;
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
    for (int i = 0; i < int_in.size(); i++) {
      int_in[i] = static_cast<int64_t>(int_input[i].val());
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

    try {
      black_box()->run(int_in, float_in, int_out, float_out);
    } catch (...) {
      black_box_state.fail(std::current_exception());
      return ES_FAILED;
    }

    for (int i = 0; i < int_out.size(); i++) {
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

ExecStatus
BlackBoxBounds::evaluate(Home home, ViewArray<Int::IntView> &ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                         ViewArray<Float::FloatView> &fvar,
#endif
                         BlackBoxHandle &black_box,
                         const BlackBoxStateHandle &black_box_state) {
  std::vector<int64_t> int_in(ivar.size() * 2);
  std::vector<int64_t> int_out(ivar.size() * 2);
  for (int i = 0; i < ivar.size(); i++) {
    int_in[i*2] = static_cast<int64_t>(ivar[i].min());
    int_in[i*2+1] = static_cast<int64_t>(ivar[i].max());
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

  try {
    black_box()->run(int_in, float_in, int_out, float_out);
  } catch (...) {
    black_box_state.fail(std::current_exception());
    return ES_FAILED;
  }

  for (int i = 0; i < ivar.size(); i++) {
    if (me_failed(ivar[i].gq(home, static_cast<int>(int_out[i*2]))) ||
        me_failed(ivar[i].lq(home, static_cast<int>(int_out[i*2+1])))) {
      return ES_FAILED;
    }
  }
#ifdef GECODE_HAS_FLOAT_VARS
  for (int i = 0; i < fvar.size(); i++) {
    if (me_failed(fvar[i].gq(home, float_out[i*2])) ||
        me_failed(fvar[i].lq(home, float_out[i*2+1]))) {
      return ES_FAILED;
    }
  }
#endif

  return ES_OK;
}

ExecStatus BlackBoxBounds::propagate(Space &home, const ModEventDelta &) {
  ExecStatus es = evaluate(home, ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                           fvar,
#endif
                           black_box, black_box_state);
  return (es == ES_OK) ? ES_NOFIX : es;
}

void blackbox(Home home, SharedHandle &black_box_state,
              const IntVarArgs &int_in, const IntVarArgs &int_out,
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
  BlackBoxStateHandle state = BlackBoxStateHandle::init(black_box_state);
  PostInfo pi(home);
  ExecStatus es = BlackBox::post(home, int_input, int_output,
#ifdef GECODE_HAS_FLOAT_VARS
                                 float_input, float_output,
#endif
                                 state,
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

void blackbox_bounds(Home home, SharedHandle &black_box_state,
                     const IntVarArgs &ivar,
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
  BlackBoxStateHandle state = BlackBoxStateHandle::init(black_box_state);
  PostInfo pi(home);
  ExecStatus es = BlackBoxBounds::post(home, int_var,
#ifdef GECODE_HAS_FLOAT_VARS
                                 float_var,
#endif
                                 sub_int,
#ifdef GECODE_HAS_FLOAT_VARS
                                 sub_float,
#endif
                                 state,
                                 mode, instantiation, args);
  GECODE_ES_FAIL(es);
}

} // namespace FlatZinc
} // namespace Gecode
