/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Copyright:
 *     Jip J. Dekker, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <set>
#include <string>

#ifdef _WIN32
#include <windows.h>
#define GECODE_BLACKBOX_EXPORT __declspec(dllexport)
#define GECODE_BLACKBOX_CALL __stdcall
#else
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#if defined(__GNUC__) || defined(__clang__)
#define GECODE_BLACKBOX_EXPORT __attribute__((visibility("default")))
#else
#define GECODE_BLACKBOX_EXPORT
#endif
#define GECODE_BLACKBOX_CALL
#endif

namespace {

  enum class Mode { normal, nan };

  struct Instance {
    const Mode mode;
    const std::string log;
    const unsigned int id;

    Instance(const char** args, size_t n_args, unsigned int id0)
      : mode((n_args > 0) && (std::strcmp(args[0], "nan") == 0)
               ? Mode::nan : Mode::normal),
        log((n_args > 1) ? args[1] : ""), id(id0) {}
    Instance(const Instance& other, unsigned int id0)
      : mode(other.mode), log(other.log), id(id0) {}
  };

  std::atomic<unsigned int> next_id(0);

  unsigned int
  instance_id(void) {
    return next_id.fetch_add(1, std::memory_order_relaxed) + 1;
  }

  void
  record(const std::string& log, const std::string& event, unsigned int id) {
    if (log.empty()) {
      return;
    }
    const std::string line = event + " " + std::to_string(id) + "\n";
#ifdef _WIN32
    HANDLE file = CreateFileA(log.c_str(), FILE_APPEND_DATA,
                              FILE_SHARE_READ | FILE_SHARE_WRITE |
                                FILE_SHARE_DELETE,
                              nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (file == INVALID_HANDLE_VALUE) {
      return;
    }
    DWORD written;
    (void)WriteFile(file, line.data(), static_cast<DWORD>(line.size()),
                    &written, nullptr);
    (void)CloseHandle(file);
#else
    int fd;
    do {
      fd = open(log.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0600);
    } while ((fd == -1) && (errno == EINTR));
    if (fd == -1) {
      return;
    }
    ssize_t n;
    do {
      n = write(fd, line.data(), line.size());
    } while ((n == -1) && (errno == EINTR));
    (void)close(fd);
#endif
  }

  bool
  has_two_starts(const std::string& log) {
    std::ifstream in(log.c_str());
    std::set<unsigned int> ids;
    std::string event;
    unsigned int id;
    while (in >> event >> id) {
      if (event == "start") {
        ids.insert(id);
      }
    }
    return ids.size() >= 2;
  }

  void
  wait_for_peer(const std::string& log, unsigned int id) {
    for (unsigned int i = 0; i < 500; ++i) {
      if (has_two_starts(log)) {
        return;
      }
#ifdef _WIN32
      Sleep(10);
#else
      usleep(10000);
#endif
    }
    record(log, "timeout", id);
  }

}

extern "C" GECODE_BLACKBOX_EXPORT void* GECODE_BLACKBOX_CALL
fzn_init(const char** args, size_t n_args) {
  Instance* instance = new Instance(args, n_args, instance_id());
  record(instance->log, "init", instance->id);
  return instance;
}

extern "C" GECODE_BLACKBOX_EXPORT void* GECODE_BLACKBOX_CALL
fzn_clone(void* value) {
  Instance* instance = new Instance(*static_cast<Instance*>(value),
                                    instance_id());
  record(instance->log, "clone", instance->id);
  return instance;
}

extern "C" GECODE_BLACKBOX_EXPORT void GECODE_BLACKBOX_CALL
fzn_free(void* value) {
  Instance* instance = static_cast<Instance*>(value);
  record(instance->log, "free", instance->id);
  delete instance;
}

extern "C" GECODE_BLACKBOX_EXPORT void GECODE_BLACKBOX_CALL
fzn_blackbox(void* value, const int64_t* int_input, size_t n_int_input,
             const double* float_input, size_t n_float_input,
             int64_t* int_output, size_t n_int_output,
             double* float_output, size_t n_float_output) {
  const Instance* instance = static_cast<const Instance*>(value);
  if (!instance->log.empty() && (n_int_input > 0)) {
    record(instance->log, "start", instance->id);
    wait_for_peer(instance->log, instance->id);
    record(instance->log, "ready", instance->id);
  }
  for (size_t i = 0; i < n_int_output; ++i) {
    int_output[i] = (n_int_input == 0)
      ? 1
      : int_input[i % n_int_input];
  }
  for (size_t i = 0; i < n_float_output; ++i) {
    if (instance->mode == Mode::nan) {
      const uint64_t nan = UINT64_C(0x7ff8000000000000);
      std::memcpy(&float_output[i], &nan, sizeof(nan));
    } else {
      float_output[i] = (n_float_input == 0)
        ? 0.0 : float_input[i % n_float_input];
    }
  }
}

// STATISTICS: test-flatzinc
