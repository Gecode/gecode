/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
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

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace {

  std::string
  process_id(void) {
#ifdef _WIN32
    return std::to_string(static_cast<int64_t>(GetCurrentProcessId()));
#else
    return std::to_string(static_cast<int64_t>(getpid()));
#endif
  }

  void
  record(const std::string& log, const std::string& event,
         const std::string& id) {
    if (log.empty()) {
      return;
    }
    const std::string line = event + " " + id + "\n";
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
    std::set<std::string> ids;
    std::string event;
    std::string id;
    while (in >> event >> id) {
      if (event == "start") {
        ids.insert(id);
      }
    }
    return ids.size() >= 2;
  }

  void
  wait_for_peer(const std::string& log, const std::string& id) {
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

  bool
  first_integer(const std::string& request, int64_t& value) {
    const std::string::size_type end = request.find(';');
    if ((end == std::string::npos) || (end == 0)) {
      return false;
    }
    const std::string integer = request.substr(0, end);
    char* last = nullptr;
    errno = 0;
    const long long parsed = std::strtoll(integer.c_str(), &last, 10);
    if ((last == integer.c_str()) || (*last != '\0') || (errno == ERANGE) ||
        (parsed < (std::numeric_limits<int64_t>::min)()) ||
        (parsed > (std::numeric_limits<int64_t>::max)())) {
      return false;
    }
    value = static_cast<int64_t>(parsed);
    return true;
  }

  void
  write_response(const std::string& value) {
    std::cout.write(value.data(), static_cast<std::streamsize>(value.size()));
    std::cout << std::endl;
    std::cout.flush();
  }

  bool
  dependent_bounds(const std::string& request) {
    long long xmin, xmax, ymin, ymax;
    if (std::sscanf(request.c_str(), "%lld,%lld,%lld,%lld;",
                    &xmin, &xmax, &ymin, &ymax) != 4) {
      return false;
    }
    (void) ymin;
    (void) ymax;
    write_response(std::to_string(xmin) + "," + std::to_string(xmax) + "," +
                   std::to_string(5 * xmin) + "," +
                   std::to_string(5 * xmax) + ";");
    return true;
  }

  const std::string*
  fixed_response(const std::string& mode) {
    static const std::string value7("7;");
    static const std::string bounds2("5,5;");
    static const std::string bounds4("5,5,5,5;");
    static const std::string mixed("5,5;5,5");
    static const std::string nan(";nan");
    static const std::string nul(";\0x", 3);
    static const std::string malformed("x;");
    if (mode == "value7") {
      return &value7;
    } else if (mode == "bounds2") {
      return &bounds2;
    } else if (mode == "bounds4") {
      return &bounds4;
    } else if (mode == "mixed") {
      return &mixed;
    } else if (mode == "nan") {
      return &nan;
    } else if (mode == "nul") {
      return &nul;
    } else if (mode == "malformed") {
      return &malformed;
    }
    return nullptr;
  }

  void
  fault(int64_t kind) {
    if (kind == 1) {
      write_response("invalid");
    } else if (kind == 2) {
      return;
    } else if (kind == 3) {
      write_response(std::string(";\0x", 3));
    } else if (kind == 5) {
      write_response(std::to_string((std::numeric_limits<int64_t>::max)()) +
                     ";");
    } else {
      write_response(std::string(1024U * 1024U + 1U, 'x'));
    }
  }

}

int
main(int argc, char* argv[]) {
  const std::string mode = (argc > 1) ? argv[1] : "normal";
  const std::string log = (argc > 2) ? argv[2] : "";
  unsigned int round = 0;
#ifndef _WIN32
  bool descendant_started = false;
#endif
  std::string request;
  while (std::getline(std::cin, request)) {
    ++round;
    int64_t value = 0;
    const bool has_value = first_integer(request, value);
    if (mode == "fault") {
      fault(has_value ? value : 1);
      return 0;
    }
    if (mode == "descendant") {
#ifndef _WIN32
      if (!descendant_started) {
        int ready[2];
        if (pipe(ready) != 0) {
          return 1;
        }
        const pid_t descendant = fork();
        if (descendant == 0) {
          (void)close(ready[0]);
          record(log, "descendant", process_id());
          const char started = '1';
          (void)write(ready[1], &started, 1);
          (void)close(ready[1]);
          for (;;) {
            pause();
          }
        }
        (void)close(ready[1]);
        if (descendant <= 0) {
          (void)close(ready[0]);
          return 1;
        }
        char started = '\0';
        ssize_t n;
        do {
          n = read(ready[0], &started, 1);
        } while ((n == -1) && (errno == EINTR));
        (void)close(ready[0]);
        if ((n != 1) || (started != '1')) {
          return 1;
        }
        descendant_started = true;
      }
#endif
      write_response("1;");
      continue;
    }

    if (mode == "dependent_bounds") {
      if (!dependent_bounds(request)) {
        return 1;
      }
      continue;
    }

    if (const std::string* value = fixed_response(mode)) {
      write_response(*value);
      continue;
    }

    if (!log.empty() && has_value) {
      const std::string id = process_id();
      record(log, "start", id);
      wait_for_peer(log, id);
      record(log, "ready", id);
    }
    write_response(std::to_string(has_value ? value :
                                  static_cast<int64_t>(round)) + ";");
  }
  return 0;
}

// STATISTICS: test-flatzinc
