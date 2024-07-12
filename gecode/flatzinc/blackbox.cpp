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
#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <cstdlib>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#endif

namespace Gecode {
namespace FlatZinc {

BlackBoxDLL::BlackBoxDLL(const std::string &name,
                         const std::vector<std::string> &args) {
  std::string loadError;
#ifdef _WIN32
  library = LoadLibraryA(name.c_str());
  if (!library) {
    loadError = std::string("unable to locate library `") + name + "'";
    library = LoadLibraryA((std::string(name) + ".dll").c_str());
  }
  if (!library) {
    library = LoadLibraryA((std::string("lib") + name + ".dll").c_str());
  }
#else
  library = dlopen(name.c_str(), RTLD_LAZY);
  if (!library) {
    loadError = std::string(dlerror());
    library = dlopen((name + ".so").c_str(), RTLD_NOW);
  }
  if (!library) {
    library = dlopen((std::string("lib") + name + ".so").c_str(), RTLD_NOW);
  }
#endif
  if (!library) {
    throw Error("Blackbox", "Unable to open dynamic library: " + loadError);
  }

  // find symbol for blacbox function
#ifdef _WIN32
  dll_fzn_blackbox = reinterpret_cast<decltype(dll_fzn_blackbox)>(
      GetProcAddress((HMODULE)library, "fzn_blackbox"));
  std::string symError = ".";
#else
  *(void **)(&dll_fzn_blackbox) = dlsym(library, "fzn_blackbox");
  std::string symError(": ");
  if (!dll_fzn_blackbox) {
    symError += std::string(dlerror());
  }
#endif
  if (!dll_fzn_blackbox) {
    throw Error("Blackbox",
                "Unable to find symbol `fzn_blackbox` in dynamic library" +
                    symError);
  }

  // Optionally call the initialisation function with the given arguments. It is
  // not an error for the library to omit `fzn_initialize`.
  void(__stdcall *dll_fzn_initialize)(const char **, size_t) = nullptr;
#ifdef _WIN32
  dll_fzn_initialize = reinterpret_cast<decltype(dll_fzn_initialize)>(
      GetProcAddress((HMODULE)library, "fzn_initialize"));
#else
  *(void **)(&dll_fzn_initialize) = dlsym(library, "fzn_initialize");
#endif
  if (dll_fzn_initialize != nullptr) {
    std::vector<const char *> argv;
    argv.reserve(args.size());
    for (const std::string &a : args) {
      argv.push_back(a.c_str());
    }
    dll_fzn_initialize(argv.data(), argv.size());
  }
}

BlackBoxDLL::~BlackBoxDLL() {
  if (library) {
#ifdef _WIN32
    FreeLibrary((HMODULE)library);
#else
    dlclose(library);
#endif
  }
}

BlackBoxExec::BlackBoxExec(const std::string &program,
                           const std::vector<std::string> &args) {
#ifdef _WIN32
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  HANDLE g_hChildStd_IN_Rd = NULL;
  HANDLE g_hChildStd_IN_Wr = NULL;
  HANDLE g_hChildStd_OUT_Rd = NULL;
  HANDLE g_hChildStd_OUT_Wr = NULL;

  // Create a pipe for the child process's STDOUT.
  if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
    std::cerr << "Stdout CreatePipe" << std::endl;
  // Ensure the read handle to the pipe for STDOUT is not inherited.
  if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
    std::cerr << "Stdout SetHandleInformation" << std::endl;

  // Create a pipe for the child process's STDIN
  if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
    std::cerr << "Stdin CreatePipe" << std::endl;
  // Ensure the write handle to the pipe for STDIN is not inherited.
  if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
    std::cerr << "Stdin SetHandleInformation" << std::endl;

  PROCESS_INFORMATION piProcInfo;
  STARTUPINFOA siStartInfo;

  // Set up members of the PROCESS_INFORMATION structure.
  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

  // Set up members of the STARTUPINFO structure.
  // This structure specifies the STDIN and STDOUT handles for redirection.
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
  siStartInfo.cb = sizeof(STARTUPINFOA);
  siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
  siStartInfo.hStdInput = g_hChildStd_IN_Rd;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  // Build the command line: the program followed by the (quoted) arguments.
  std::string prog = program;
  for (const std::string &a : args) {
    prog += " \"";
    for (char ch : a) {
      if (ch == '"' || ch == '\\') {
        prog += '\\';
      }
      prog += ch;
    }
    prog += '"';
  }
  BOOL processStarted =
      CreateProcessA(nullptr,
                     prog.data(),  // command line
                     nullptr,      // process security attributes
                     nullptr,      // primary thread security attributes
                     TRUE,         // handles are inherited
                     0,            // creation flags
                     nullptr,      // use parent's environment
                     nullptr,      // use parent's current directory
                     &siStartInfo, // STARTUPINFO pointer
                     &piProcInfo); // receives PROCESS_INFORMATION

  if (!processStarted) {
    throw Error("BlackBoxExec", "Unable to start program `" + program + "'");
  }

  CloseHandle(piProcInfo.hThread);
  // Stop ReadFile from blocking
  CloseHandle(g_hChildStd_OUT_Wr);
  // Just close the child's in pipe here
  CloseHandle(g_hChildStd_IN_Rd);

  pipe_send = g_hChildStd_IN_Wr;
  pipe_receive = g_hChildStd_OUT_Rd;
#else
  const int READ = 0;
  const int WRITE = 1;
  int child_in[2];
  int child_out[2];
  pipe(child_in);
  pipe(child_out);

  if (fork() != 0) {
    close(child_in[READ]);
    close(child_out[WRITE]);

    pipe_send = child_in[WRITE];
    int pipe_receive = child_out[READ];
    file_receive = fdopen(pipe_receive, "r");
    return;
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  dup2(child_in[READ], STDIN_FILENO);
  dup2(child_out[WRITE], STDOUT_FILENO);
  close(child_in[WRITE]);
  close(child_out[READ]);

  // Launch the program directly (no shell), passing the annotation arguments as
  // its command-line arguments.
  std::vector<char *> argv;
  argv.push_back(const_cast<char *>(program.c_str()));
  for (const std::string &a : args) {
    argv.push_back(const_cast<char *>(a.c_str()));
  }
  argv.push_back(nullptr);
  execvp(program.c_str(), argv.data());
  // execvp only returns on failure.
  std::exit(127);
#endif
};

BlackBoxExec::~BlackBoxExec() {
#ifdef _WIN32
  CloseHandle(pipe_send);
  CloseHandle(pipe_receive);
#else
  close(pipe_send);
  fclose(file_receive);
#endif
}

void BlackBoxExec::run(const std::vector<int64_t> &int_in,
                       const std::vector<double> &float_in,
                       std::vector<int64_t> &int_out,
                       std::vector<double> &float_out) {
  // Construct program input: comma-separated integers, a semicolon, then
  // comma-separated floats, terminated by a newline (e.g. "5,-7;2.5,1.125\n").
  std::stringstream out;
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
#ifdef _WIN32
  // Write to process input pipe
  BOOL success =
      WriteFile(pipe_send, out_buf.c_str(), out_buf.size(), nullptr, nullptr);
  assert(success);

  // Read output from process by pipe
  char c[2] = {0, 0};
  std::ostringstream oss;
  while (c[0] != '\n') {
    DWORD count = 0;
    BOOL success = ReadFile(pipe_receive, c, sizeof(c) - 1, &count, NULL);
    if (!success) {
      throw Error(
          "BlackBoxExec",
          "Reading blackbox process output from pipe resulted did not succeed");
    } else if (count == 0) {
      throw Error("BlackBoxExec",
                  "Blackbox process provided an incomplete response");
    }
    assert(count == 1);
    oss << c[0];
  }
  std::string in_buffer(oss.str());
#else
  // Write to process input pipe
  ssize_t bytes_written = write(pipe_send, out_buf.c_str(), out_buf.size());
  if (bytes_written != static_cast<ssize_t>(out_buf.size())) {
    throw Error("BlackBoxExec",
                "Failed to write the full request to the blackbox process.");
  }

  // Read from process output pipe
  char *str = NULL;
  size_t size = 0;

  if (getline(&str, &size, file_receive) == -1) {
    throw Error(
        "BlackBoxExec",
        "Reading blackbox process output from pipe resulted in error no. " +
            std::to_string(errno));
  }
  std::string in_buffer(str);
  free(str);
#endif
  // Parse the response in a single left-to-right pass: comma-separated
  // integers, a semicolon, then comma-separated floats (e.g. "5,-7;2.5,1.125\n").
  const char *p = in_buffer.c_str();
  char *end = nullptr;
  auto skip_ws = [](const char *&q) {
    while (*q == ' ' || *q == '\t' || *q == '\r') {
      ++q;
    }
  };
  for (size_t i = 0; i < int_out.size(); ++i) {
    long long v = std::strtoll(p, &end, 10);
    if (end == p) {
      throw Error("BlackBoxExec", "Failed to read output integer " +
                                      std::to_string(i) +
                                      " from blackbox process output, " +
                                      std::to_string(int_out.size()) +
                                      " integer values where expected.");
    }
    int_out[i] = static_cast<int64_t>(v);
    p = end;
    skip_ws(p);
    if (*p == ',') {
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
    double v = std::strtod(p, &end);
    if (end == p) {
      throw Error("BlackBoxExec", "Failed to read output float " +
                                      std::to_string(i) +
                                      " from blackbox process output, " +
                                      std::to_string(float_out.size()) +
                                      " floating point values where expected.");
    }
    float_out[i] = v;
    p = end;
    skip_ws(p);
    if (*p == ',') {
      ++p;
    }
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
      // std::cerr << int_input[i].val() << " ";
      int_in[i] = int_input[i].val();
    }
    std::vector<double> float_in;
    std::vector<double> float_out;
#ifdef GECODE_HAS_FLOAT_VARS
    float_in.resize(float_input.size());
    float_out.resize(float_output.size());
    for (int i = 0; i < float_in.size(); i++) {
      // std::cerr << float_input[i].val() << " ";
      float_in[i] = float_input[i].val().med();
    }
#endif
    // std::cerr << std::endl;

    black_box()->run(int_in, float_in, int_out, float_out);

    // std::cerr << "Black Box Fn output: ";
    for (int i = 0; i < int_out.size(); i++) {
      // std::cerr << int_out[i] << " ";
      GECODE_ME_CHECK(int_output[i].eq(home, static_cast<int>(int_out[i])));
    }
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < float_out.size(); i++) {
      // std::cerr << float_out[i] << " ";
      GECODE_ME_CHECK(float_output[i].eq(home, float_out[i]));
    }
#endif
    // std::cerr << std::endl;

    return home.ES_SUBSUMED(*this);
  }
  return ES_FIX;
}

ExecStatus BlackBoxBounds::propagate(Space &home, const ModEventDelta &) {
  std::vector<int64_t> int_in(ivar.size() * 2);
  std::vector<int64_t> int_out(ivar.size() * 2);
  // std::cerr << "Black Box Bounds Fn input: ";
  for (int i = 0; i < ivar.size(); i++) {
    // std::cerr << ivar[i].min() << " " << ivar[i].max() << " ";
    int_in[i*2] = ivar[i].min();
    int_in[i*2+1] = ivar[i].max();
  }
  std::vector<double> float_in;
  std::vector<double> float_out;
#ifdef GECODE_HAS_FLOAT_VARS
  float_in.resize(fvar.size() * 2);
  float_out.resize(fvar.size() * 2);
  for (int i = 0; i < fvar.size(); i++) {
    // std::cerr << fvar[i].min() << " " << fvar[i].max() << " ";
    float_in[i*2] = fvar[i].min();
    float_in[i*2+1] = fvar[i].max();
  }
#endif
  // std::cerr << std::endl;

  black_box()->run(int_in, float_in, int_out, float_out);

  // std::cerr << "Black Box Fn output: ";
  for (int i = 0; i < ivar.size(); i++) {
    // std::cerr << int_out[i*2] << ".." << int_out[i*2+1] << " ";
    GECODE_ME_CHECK(ivar[i].gq(home, static_cast<int>(int_out[i*2])));
    GECODE_ME_CHECK(ivar[i].lq(home, static_cast<int>(int_out[i*2+1])));
  }
#ifdef GECODE_HAS_FLOAT_VARS
  for (int i = 0; i < fvar.size(); i++) {
    // std::cerr << float_out[i*2] << ".." << float_out[i*2+1] << " ";
    GECODE_ME_CHECK(fvar[i].gq(home, float_out[i*2]));
    GECODE_ME_CHECK(fvar[i].lq(home, float_out[i*2+1]));
  }
#endif
  // std::cerr << std::endl;

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
/// being `[idx, |R_lb|, (var, bnd)..., |R_ub|, (var, bnd)...]`. An empty reason
/// falls back to subscribing to every variable.
static void reason_subscriptions(const std::vector<int> &reason, int n_int,
                                 int n_float, SharedArray<bool> &sub_int,
                                 SharedArray<bool> &sub_float) {
  const bool all = reason.empty();
  for (int i = 0; i < n_int; i++) {
    sub_int[i] = all;
  }
  for (int i = 0; i < n_float; i++) {
    sub_float[i] = all;
  }
  if (all) {
    return;
  }

  size_t pos = 0;
  while (pos < reason.size()) {
    pos++; // idx: the variable being explained (not needed for subscription)
    for (int side = 0; side < 2; side++) { // lower- then upper-bound literals
      int count = reason[pos++];
      for (int k = 0; k < count; k++) {
        int var = reason[pos++]; // 1-based combined variable index
        pos++;                   // bound code (per-variable granularity only)
        if (var >= 1 && var <= n_int) {
          sub_int[var - 1] = true;
        } else if (var > n_int && var <= n_int + n_float) {
          sub_float[var - 1 - n_int] = true;
        }
      }
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
