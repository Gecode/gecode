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

#include "test/flatzinc.hh"

#include <gecode/flatzinc/blackbox-backend.hh>

#include <cstdint>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <set>

#ifdef GECODE_HAS_THREADS
#include <thread>
#endif

#if defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
#include <signal.h>
#include <unistd.h>
#endif

namespace Test { namespace FlatZinc {

  namespace {
    const char* blackbox_decl =
      "predicate gecode_blackbox("
      "array[int] of var int: int_input, "
      "array[int] of var float: float_input, "
      "array[int] of var int: int_output, "
      "array[int] of var float: float_output);\n";

    const char* blackbox_bounds_decl =
      "predicate gecode_blackbox_bounds("
      "array[int] of var int: int_input, "
      "array[int] of var float: float_input, "
      "array[int] of int: flat_reason);\n";

    std::string
    fzn_string(const std::string& value) {
      std::string quoted("\"");
      for (char c : value) {
        if ((c == '\\') || (c == '\"')) {
          quoted += '\\';
        }
        quoted += c;
      }
      return quoted + "\"";
    }

    std::string
    fixture_annotation(const char* mode, const std::string& fixture,
                       const std::vector<std::string>& args) {
      std::string annotation("blackbox_");
      annotation += mode;
      annotation += "(" + fzn_string(fixture);
      if (!args.empty()) {
        annotation += ", [";
        for (size_t i = 0; i < args.size(); ++i) {
          if (i != 0) {
            annotation += ", ";
          }
          annotation += fzn_string(args[i]);
        }
        annotation += "]";
      }
      return annotation + ")";
    }

    std::string
    fixture_log(const std::string& name) {
      const char* base = std::getenv("GECODE_TEST_BLACKBOX_LOG");
      return (base == nullptr) ? std::string() : std::string(base) + "." + name;
    }

    void
    reset_log(const std::string& log) {
      std::remove(log.c_str());
    }

    std::vector<std::pair<std::string, std::string> >
    read_log(const std::string& log) {
      std::vector<std::pair<std::string, std::string> > entries;
      std::ifstream in(log.c_str());
      std::string event;
      std::string id;
      while (in >> event >> id) {
        entries.push_back(std::make_pair(event, id));
      }
      return entries;
    }

    bool
    concurrent_log(const std::string& log) {
      const std::vector<std::pair<std::string, std::string> > entries =
        read_log(log);
      std::set<std::string> starts;
      std::set<std::string> ready;
      if ((entries.size() != 4) || (entries[0].first != "start") ||
          (entries[1].first != "start") || (entries[2].first != "ready") ||
          (entries[3].first != "ready")) {
        return false;
      }
      for (const auto& entry : entries) {
        (entry.first == "start" ? starts : ready).insert(entry.second);
      }
      return (starts.size() == 2) && (ready == starts);
    }

    bool
    dll_parallel_lifecycle(const std::string& log) {
      const std::vector<std::pair<std::string, std::string> > entries =
        read_log(log);
      std::set<std::string> clones;
      std::set<std::string> starts;
      std::set<std::string> ready;
      std::set<std::string> freed;
      std::vector<std::string> calls;
      std::string prototype;
      unsigned int inits = 0;
      unsigned int clone_count = 0;
      unsigned int start_count = 0;
      unsigned int ready_count = 0;
      unsigned int free_count = 0;
      bool frees_after_calls = true;
      for (const auto& entry : entries) {
        if (entry.first == "init") {
          ++inits;
          prototype = entry.second;
        } else if (entry.first == "clone") {
          ++clone_count;
          clones.insert(entry.second);
        } else if (entry.first == "start") {
          ++start_count;
          starts.insert(entry.second);
          calls.push_back(entry.first);
        } else if (entry.first == "ready") {
          ++ready_count;
          ready.insert(entry.second);
          calls.push_back(entry.first);
        } else if (entry.first == "free") {
          ++free_count;
          freed.insert(entry.second);
          frees_after_calls = frees_after_calls && (calls.size() == 4);
        } else {
          return false;
        }
      }
      std::set<std::string> instances(clones);
      instances.insert(prototype);
      return (entries.size() == 10) && (entries[0].first == "init") &&
        (inits == 1) && (clone_count == 2) && (start_count == 2) &&
        (ready_count == 2) && (free_count == 3) && (clones.size() == 2) &&
        (clones.find(prototype) == clones.end()) && (starts == clones) &&
        (ready == clones) && (freed == instances) && frees_after_calls &&
        (calls == std::vector<std::string>{"start", "start", "ready", "ready"});
    }

    bool
    dll_model_lifecycle(const std::string& log) {
      const std::vector<std::pair<std::string, std::string> > entries =
        read_log(log);
#ifdef GECODE_HAS_THREADS
      if (entries.size() != 8) {
        return false;
      }
      for (size_t i = 0; i < entries.size(); i += 4) {
        if ((entries[i].first != "init") ||
            (entries[i+1].first != "clone") ||
            (entries[i+2] != std::make_pair(std::string("free"),
                                             entries[i+1].second)) ||
            (entries[i+3] != std::make_pair(std::string("free"),
                                             entries[i].second))) {
          return false;
        }
      }
#else
      if (entries.size() != 4) {
        return false;
      }
      for (size_t i = 0; i < entries.size(); i += 2) {
        if ((entries[i].first != "init") ||
            (entries[i+1] != std::make_pair(std::string("free"),
                                             entries[i].second))) {
          return false;
        }
      }
#endif
      return true;
    }

#if defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
    bool
    descendant_gone(const std::string& output, const std::string& log) {
      const std::vector<std::pair<std::string, std::string> > entries =
        read_log(log);
      reset_log(log);
      if (output.find("y = 1;") == std::string::npos) {
        return false;
      }
      for (const auto& entry : entries) {
        if (entry.first == "descendant") {
          const long pid = std::strtol(entry.second.c_str(), nullptr, 10);
          if (pid <= 0) {
            return false;
          }
          for (unsigned int i = 0; i < 500; ++i) {
            if ((kill(static_cast<pid_t>(pid), 0) == -1) && (errno == ESRCH)) {
              return true;
            }
            usleep(10000);
          }
          return false;
        }
      }
      return false;
    }
#endif
  }

  namespace Blackbox {
    class NativeProtocol : public Base {
    public:
      NativeProtocol(void) : Base("FlatZinc::blackbox::native_protocol") {}
      virtual bool run(void) {
        std::vector<int64_t> int_input{-2};
        std::vector<double> float_input{1.25};
        std::vector<int64_t> int_output(1);
        std::vector<double> float_output(1);
        Gecode::FlatZinc::BlackBoxCall call = {
          int_input, float_input, int_output, float_output
        };
        try {
          if (Gecode::FlatZinc::encode_blackbox_request(call) !=
              "-2;1.25\n") {
            return false;
          }
          Gecode::FlatZinc::decode_blackbox_response("7;2.5\n", call);
        } catch (...) {
          return false;
        }
        return (int_output[0] == 7) && (float_output[0] == 2.5);
      }
    };

#ifdef GECODE_HAS_THREADS
    template<class T>
    bool
    parallel_runs(T& black_box) {
      bool echoed[2] = {false, false};
      std::exception_ptr exception[2];
      auto run = [&black_box, &echoed, &exception](int i) {
        try {
          const int64_t input = (i == 0) ? -1 : i + 1;
          std::vector<int64_t> int_out(1);
          std::vector<double> float_out;
          black_box.run({input}, {}, int_out, float_out);
          echoed[i] = (int_out[0] == input);
        } catch (...) {
          exception[i] = std::current_exception();
        }
      };
      std::thread first;
      std::thread second;
      auto join = [&first, &second](void) {
        if (first.joinable()) {
          first.join();
        }
        if (second.joinable()) {
          second.join();
        }
      };
      try {
        first = std::thread(run, 0);
        second = std::thread(run, 1);
      } catch (...) {
        join();
        return false;
      }
      join();
      return (exception[0] == nullptr) && (exception[1] == nullptr) &&
        echoed[0] && echoed[1];
    }

#if defined(_WIN32) || defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
    class NativeExecParallelSessions : public Base {
    private:
      std::string executable;
      std::string log;
    public:
      NativeExecParallelSessions(const std::string& executable0,
                                 const std::string& log0)
        : Base("FlatZinc::blackbox::native_exec_parallel_sessions"),
          executable(executable0), log(log0) {}
      virtual bool run(void) {
        reset_log(log);
        bool ok = false;
        try {
          Gecode::FlatZinc::BlackBoxExec black_box(executable,
                                                   {"normal", log});
          ok = parallel_runs(black_box) && concurrent_log(log);
        } catch (...) {}
        reset_log(log);
        return ok;
      }
    };
#endif

    class NativeDllParallelCalls : public Base {
    private:
      std::string library;
      std::string log;
    public:
      NativeDllParallelCalls(const std::string& library0,
                             const std::string& log0)
        : Base("FlatZinc::blackbox::native_dll_parallel_calls"),
          library(library0), log(log0) {}
      virtual bool run(void) {
        reset_log(log);
        bool ok = false;
        try {
          {
            Gecode::FlatZinc::BlackBoxLibrary black_box(library,
                                                        {"normal", log});
            ok = parallel_runs(black_box);
          }
          ok = ok && dll_parallel_lifecycle(log);
        } catch (...) {}
        reset_log(log);
        return ok;
      }
    };
#endif

    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        (void) new NativeProtocol;
        (void) new FlatZincErrorTest("blackbox::malformed_annotation",
          std::string(blackbox_decl) +
          "var 0..1: y;\n"
          "constraint gecode_blackbox([], [], [y], []) :: "
          "blackbox_exec([]);\n"
          "solve satisfy;\n", {},
          "expected a target string and an argument array");

#if defined(_WIN32) || defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
        const char* exec = std::getenv("GECODE_TEST_BLACKBOX_EXEC");
        if (exec != nullptr) {
          const std::string executable(exec);
          (void) new FlatZincTest("blackbox::constant_value",
            std::string(blackbox_decl) +
            "var 7..7: y :: output_var;\n"
            "constraint gecode_blackbox([], [], [y], []) :: " +
            fixture_annotation("exec", executable, {"value7"}) + ";\n"
            "solve satisfy;\n",
            "y = 7;\n----------\n");

          (void) new FlatZincTest("blackbox::constant_value_unsat",
            std::string(blackbox_decl) +
            "var 8..8: y;\n"
            "constraint gecode_blackbox([], [], [y], []) :: " +
            fixture_annotation("exec", executable, {"value7"}) + ";\n"
            "solve satisfy;\n",
            "=====UNSATISFIABLE=====\n");

          (void) new FlatZincTest("blackbox::reason_independent_bounds",
            std::string(blackbox_bounds_decl) +
            "var 5..5: x :: output_var;\n"
            "constraint gecode_blackbox_bounds([x], [], [1,0,0]) :: " +
            fixture_annotation("exec", executable, {"bounds2"}) + ";\n"
            "solve satisfy;\n",
            "x = 5;\n----------\n");

          (void) new FlatZincTest("blackbox::reason_independent_bounds_unsat",
            std::string(blackbox_bounds_decl) +
            "var 6..6: x;\n"
            "constraint gecode_blackbox_bounds([x], [], [1,0,0]) :: " +
            fixture_annotation("exec", executable, {"bounds2"}) + ";\n"
            "solve satisfy;\n",
            "=====UNSATISFIABLE=====\n");

          (void) new FlatZincTest("blackbox::reason_dependent_bounds",
            std::string(blackbox_bounds_decl) +
            "var 5..5: x :: output_var;\n"
            "constraint gecode_blackbox_bounds([x], [], [1,1,1,1,0]) :: " +
            fixture_annotation("exec", executable, {"bounds2"}) + ";\n"
            "solve satisfy;\n",
            "x = 5;\n----------\n");

          (void) new FlatZincTest("blackbox::bounds_rescheduled_after_branch",
            std::string(blackbox_bounds_decl) +
            "var 0..1: x :: output_var;\n"
            "var 0..5: y :: output_var;\n"
            "constraint gecode_blackbox_bounds([x,y], [], "
            "[1,0,0,2,1,1,1,1,1,2]) :: " +
            fixture_annotation("exec", executable, {"dependent_bounds"}) +
            ";\nsolve :: int_search([x], input_order, indomain_min, complete) "
            "satisfy;\n",
            "x = 0;\ny = 0;\n----------\n"
            "x = 1;\ny = 5;\n----------\n==========\n",
            false, {"-a"});

#ifdef GECODE_HAS_FLOAT_VARS
          (void) new FlatZincErrorTest("blackbox::missing_bounds_reason_entry",
            std::string(blackbox_bounds_decl) +
            "var 0..10: x;\n"
            "var 0.0..10.0: y;\n"
            "constraint gecode_blackbox_bounds([x], [y], [1,0,0]) :: " +
            fixture_annotation("exec", executable, {"mixed"}) + ";\n"
            "solve satisfy;\n", {}, "missing explained variable entry");
#endif

          (void) new FlatZincErrorTest("blackbox::duplicate_bounds_reason_entry",
            std::string(blackbox_bounds_decl) +
            "var 0..10: x;\n"
            "var 0..10: y;\n"
            "constraint gecode_blackbox_bounds([x,y], [], [1,0,0,1,0,0]) :: " +
            fixture_annotation("exec", executable, {"bounds4"}) + ";\n"
            "solve satisfy;\n", {}, "duplicate explained variable index");

          (void) new FlatZincErrorTest("blackbox::invalid_bounds_reason_code",
            std::string(blackbox_bounds_decl) +
            "var 0..10: x;\n"
            "constraint gecode_blackbox_bounds([x], [], [1,1,1,0,0]) :: " +
            fixture_annotation("exec", executable, {"bounds2"}) + ";\n"
            "solve satisfy;\n", {}, "dependency bound code is out of range");

#ifdef GECODE_HAS_FLOAT_VARS
          (void) new FlatZincErrorTest("blackbox::invalid_float_output",
            std::string(blackbox_decl) +
            "var 0.0..10.0: y;\n"
            "constraint gecode_blackbox([], [], [], [y]) :: " +
            fixture_annotation("exec", executable, {"nan"}) + ";\n"
            "solve satisfy;\n", {}, "Failed to read output float 0");
#endif

          (void) new FlatZincErrorTest("blackbox::nul_output",
            std::string(blackbox_decl) +
            "constraint gecode_blackbox([], [], [], []) :: " +
            fixture_annotation("exec", executable, {"nul"}) + ";\n"
            "solve satisfy;\n", {}, "response contains NUL data");

          (void) new FlatZincErrorTest("blackbox::malformed_exec_parallel",
            std::string(blackbox_decl) +
            "var 0..1: x :: output_var;\n"
            "var 0..1: y :: output_var;\n"
            "constraint gecode_blackbox([x], [], [y], []) :: " +
            fixture_annotation("exec", executable, {"malformed"}) + ";\n"
            "solve :: int_search([x], first_fail, indomain_min, complete) "
            "satisfy;\n",
            {"-p", "2"}, "Failed to read output integer 0");

          for (int kind = 1; kind <= 5; ++kind) {
            const char* expected = nullptr;
            switch (kind) {
            case 1:
              expected = "Failed to read output integer 0";
              break;
            case 2:
              expected = "provided an incomplete response";
              break;
            case 3:
              expected = "response contains NUL data";
              break;
            case 4:
              expected = "response exceeds the size limit";
              break;
            default:
              expected = "integer 0 is outside Gecode's integer range";
              break;
            }
            (void) new FlatZincErrorTest(
              "blackbox::native_exec_fault_" + std::to_string(kind),
              std::string(blackbox_decl) +
              "var " + std::to_string(kind) + ".." + std::to_string(kind) +
              ": x;\nvar 0..1: y;\n"
              "constraint gecode_blackbox([x], [], [y], []) :: " +
              fixture_annotation("exec", executable, {"fault"}) + ";\n"
              "solve satisfy;\n", {}, expected);
          }

#ifdef GECODE_HAS_THREADS
          const std::string exec_log = fixture_log("exec_parallel");
          if (!exec_log.empty()) {
            (void) new NativeExecParallelSessions(executable, exec_log);
          }
#endif

#if defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
          const std::string descendant_log = fixture_log("exec_descendant");
          if (!descendant_log.empty()) {
            (void) new FlatZincTest("blackbox::native_exec_descendant_cleanup",
              std::string(blackbox_decl) +
              "var 1..1: y :: output_var;\n"
              "constraint gecode_blackbox([], [], [y], []) :: " +
              fixture_annotation("exec", executable,
                                 {"descendant", descendant_log}) + ";\n"
              "solve satisfy;\n",
              "", false, {},
              [descendant_log](const std::string& output) {
                return descendant_gone(output, descendant_log);
              },
              [descendant_log](void) { reset_log(descendant_log); });
          }
#endif
        }

        (void) new FlatZincErrorTest("blackbox::missing_exec_parallel",
          std::string(blackbox_decl) +
          "var 0..1: x :: output_var;\n"
          "var 0..1: y :: output_var;\n"
          "constraint gecode_blackbox([x], [], [y], []) :: "
          "blackbox_exec(\"gecode-blackbox-missing-program\");\n"
          "solve :: int_search([x], first_fail, indomain_min, complete) "
          "satisfy;\n",
          {"-p", "2"}, "starting blackbox process failed");

        (void) new FlatZincErrorTest("blackbox::missing_exec_root_status",
          std::string(blackbox_decl) +
          "var 0..0: x :: output_var;\n"
          "var 0..1: y :: output_var;\n"
          "constraint gecode_blackbox([x], [], [y], []) :: "
          "blackbox_exec(\"gecode-blackbox-missing-program\");\n"
          "solve satisfy;\n",
          {"-p", "2"}, "starting blackbox process failed");
#endif

        const char* dll = std::getenv("GECODE_TEST_BLACKBOX_DLL");
        if (dll != nullptr) {
          const std::string library(dll);
          const std::string dll_model_log = fixture_log("dll_model");
          if (!dll_model_log.empty()) {
            (void) new FlatZincTest("blackbox::native_dll_per_constraint",
              std::string(blackbox_decl) +
              "var 1..1: a :: output_var;\n"
              "var 1..1: b :: output_var;\n"
              "constraint gecode_blackbox([], [], [a], []) :: " +
              fixture_annotation("dll", library, {"normal", dll_model_log}) +
              ";\nconstraint gecode_blackbox([], [], [b], []) :: " +
              fixture_annotation("dll", library, {"normal", dll_model_log}) +
              ";\nsolve satisfy;\n",
              "a = 1;\nb = 1;\n----------\n", false, {},
              [dll_model_log](const std::string& output) {
                const bool ok = (output == "a = 1;\nb = 1;\n----------\n") &&
                  dll_model_lifecycle(dll_model_log);
                reset_log(dll_model_log);
                return ok;
              },
              [dll_model_log](void) { reset_log(dll_model_log); });
          }

#ifdef GECODE_HAS_THREADS
          const std::string dll_log = fixture_log("dll_parallel");
          if (!dll_log.empty()) {
            (void) new NativeDllParallelCalls(library, dll_log);
          }
#endif

#ifdef GECODE_HAS_FLOAT_VARS
          (void) new FlatZincErrorTest("blackbox::native_dll_nonfinite",
            std::string(blackbox_decl) +
            "var 0.0..1.0: y;\n"
            "constraint gecode_blackbox([], [], [], [y]) :: " +
            fixture_annotation("dll", library, {"nan"}) + ";\n"
            "solve satisfy;\n", {},
            "library output float 0 is not a finite value");
#endif
        }
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
