/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
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
#include <gecode/flatzinc/blackbox-backend.hh>
#include <gecode/flatzinc/blackbox-process.hh>

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

} // namespace

#ifdef GECODE_HAS_THREADS
class BlackBoxLibrary::Instance {
public:
  std::thread::id owner;
  void *value;

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
  // Each live thread has exclusive access to its selected instance.
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
BlackBoxLibrary::run(BlackBoxCall& call) {
#ifdef GECODE_HAS_THREADS
  if (library_fzn_init != nullptr) {
    Instance *selected = this->instance();
    library_fzn_blackbox(selected->value,
                         call.int_input.data(), call.int_input.size(),
                         call.float_input.data(), call.float_input.size(),
                         call.int_output.data(), call.int_output.size(),
                         call.float_output.data(), call.float_output.size());
  } else {
    library_fzn_blackbox(nullptr,
                         call.int_input.data(), call.int_input.size(),
                         call.float_input.data(), call.float_input.size(),
                         call.int_output.data(), call.int_output.size(),
                         call.float_output.data(), call.float_output.size());
  }
#else
  library_fzn_blackbox(root_instance,
                       call.int_input.data(), call.int_input.size(),
                       call.float_input.data(), call.float_input.size(),
                       call.int_output.data(), call.int_output.size(),
                       call.float_output.data(), call.float_output.size());
#endif
  for (size_t i = 0; i < call.int_output.size(); ++i) {
    check_int(call.int_output[i], "library output", i);
  }
#ifdef GECODE_HAS_FLOAT_VARS
  check_floats(call.float_output, "library output");
#endif
}


BlackBoxExec::BlackBoxExec(const std::string &program0,
                           const std::vector<std::string> &args0)
    : program(program0), args(args0) {}

BlackBoxExec::~BlackBoxExec(void) {
  Support::Lock lock(mutex);
  for (BlackBoxProcessSession *s : sessions) {
    delete s;
  }
  sessions.clear();
}


BlackBoxProcessSession& BlackBoxExec::session(void) {
  Support::Lock lock(mutex);
  for (BlackBoxProcessSession *s : sessions) {
    if (s->owned_by_current_thread()) {
      return *s;
    }
  }
  std::unique_ptr<BlackBoxProcessSession> s(
    create_blackbox_process(program, args));
  BlackBoxProcessSession *r = s.get();
  sessions.push_back(r);
  s.release();
  return *r;
}

std::string
encode_blackbox_request(const BlackBoxCall& call) {
  // Construct program input: comma-separated integers, a semicolon, then
  // comma-separated floats, terminated by a newline (e.g. "5,-7;2.5,1.125\n").
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out.precision(std::numeric_limits<double>::max_digits10);
  for (size_t i = 0; i < call.int_input.size(); ++i) {
    if (i != 0) {
      out << ",";
    }
    out << call.int_input[i];
  }
  out << ";";
  for (size_t i = 0; i < call.float_input.size(); ++i) {
    if (i != 0) {
      out << ",";
    }
    out << call.float_input[i];
  }
  out << "\n";
  return out.str();
}

void
decode_blackbox_response(const std::string& response, BlackBoxCall& call) {
  if (response.find('\0') != std::string::npos) {
    throw Error("BlackBoxExec",
                "Blackbox process response contains NUL data.");
  }
  // Parse the response in a single left-to-right pass: comma-separated
  // integers, a semicolon, then comma-separated floats (e.g. "5,-7;2.5,1.125\n").
  const char *p = response.c_str();
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
  for (size_t i = 0; i < call.int_output.size(); ++i) {
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
                                      std::to_string(call.int_output.size()) +
                                      " integer values were expected.");
    }
    check_int(value, "blackbox process output", i);
    call.int_output[i] = value;
    p = end;
    skip_ws(p);
    if (i + 1 < call.int_output.size()) {
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
  for (size_t i = 0; i < call.float_output.size(); ++i) {
    skip_ws(p);
    const char *end = value_end(p);
    std::istringstream in(std::string(p, end));
    in.imbue(std::locale::classic());
    double v;
    if (!(in >> v) || !check_number_tail(in)) {
      throw Error("BlackBoxExec", "Failed to read output float " +
                                      std::to_string(i) +
                                      " from blackbox process output, " +
                                      std::to_string(call.float_output.size()) +
                                      " floating point values were expected.");
    }
#ifdef GECODE_HAS_FLOAT_VARS
    check_float(v, "blackbox process output", i);
#endif
    call.float_output[i] = v;
    p = end;
    skip_ws(p);
    if (i + 1 < call.float_output.size()) {
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

void
BlackBoxExec::run(BlackBoxCall& call) {
  const std::string response =
    session().exchange(encode_blackbox_request(call));
  decode_blackbox_response(response, call);
}

} // namespace FlatZinc
} // namespace Gecode
