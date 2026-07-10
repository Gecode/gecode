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

#ifdef GECODE_HAS_FLOAT_VARS

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

    const char* python_value_7 =
      "blackbox_exec(\"python3\", "
      "[\"-u\", \"-c\", "
      "\"import sys; [print(chr(55)+chr(59), flush=True) "
      "for line in sys.stdin]\"])";

    const char* python_bounds_5 =
      "blackbox_exec(\"python3\", "
      "[\"-u\", \"-c\", "
      "\"import sys; [print(chr(53)+chr(44)+chr(53)+chr(59), "
      "flush=True) for line in sys.stdin]\"])";

    const char* python_bounds_5_twice =
      "blackbox_exec(\"python3\", "
      "[\"-u\", \"-c\", "
      "\"import sys; [print(chr(53)+chr(44)+chr(53)+chr(44)+chr(53)+"
      "chr(44)+chr(53)+chr(59), flush=True) for line in sys.stdin]\"])";

    const char* python_mixed_bounds_5 =
      "blackbox_exec(\"python3\", "
      "[\"-u\", \"-c\", "
      "\"import sys; [print(chr(53)+chr(44)+chr(53)+chr(59)+chr(53)+"
      "chr(44)+chr(53), flush=True) for line in sys.stdin]\"])";

    const char* python_nan =
      "blackbox_exec(\"python3\", "
      "[\"-u\", \"-c\", "
      "\"import sys; [print(chr(59)+chr(110)+chr(97)+chr(110), "
      "flush=True) for line in sys.stdin]\"])";

    const char* python_nul =
      "blackbox_exec(\"python3\", "
      "[\"-u\", \"-c\", "
      "\"import sys; [print(chr(59)+chr(0)+chr(120), "
      "flush=True) for line in sys.stdin]\"])";

    const char* python_malformed =
      "blackbox_exec(\"python3\", "
      "[\"-u\", \"-c\", "
      "\"import sys; [print(chr(120)+chr(59), "
      "flush=True) for line in sys.stdin]\"])";
  }

  namespace Blackbox {
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        (void) new FlatZincTest("blackbox::constant_value",
          std::string(blackbox_decl) +
          "var 7..7: y :: output_var;\n"
          "constraint gecode_blackbox([], [], [y], []) :: " +
          python_value_7 +
          ";\n"
          "solve satisfy;\n",
          "y = 7;\n----------\n");

        (void) new FlatZincTest("blackbox::constant_value_unsat",
          std::string(blackbox_decl) +
          "var 8..8: y;\n"
          "constraint gecode_blackbox([], [], [y], []) :: " +
          python_value_7 +
          ";\n"
          "solve satisfy;\n",
          "=====UNSATISFIABLE=====\n");

        (void) new FlatZincTest("blackbox::reason_independent_bounds",
          std::string(blackbox_bounds_decl) +
          "var 5..5: x :: output_var;\n"
          "constraint gecode_blackbox_bounds([x], [], [1,0,0]) :: " +
          python_bounds_5 +
          ";\n"
          "solve satisfy;\n",
          "x = 5;\n----------\n");

        (void) new FlatZincTest("blackbox::reason_independent_bounds_unsat",
          std::string(blackbox_bounds_decl) +
          "var 6..6: x;\n"
          "constraint gecode_blackbox_bounds([x], [], [1,0,0]) :: " +
          python_bounds_5 +
          ";\n"
          "solve satisfy;\n",
          "=====UNSATISFIABLE=====\n");

        (void) new FlatZincTest("blackbox::reason_dependent_bounds",
          std::string(blackbox_bounds_decl) +
          "var 5..5: x :: output_var;\n"
          "constraint gecode_blackbox_bounds([x], [], [1,1,1,1,0]) :: " +
          python_bounds_5 +
          ";\n"
          "solve satisfy;\n",
          "x = 5;\n----------\n");

        (void) new FlatZincErrorTest("blackbox::missing_bounds_reason_entry",
          std::string(blackbox_bounds_decl) +
          "var 0..10: x;\n"
          "var 0.0..10.0: y;\n"
          "constraint gecode_blackbox_bounds([x], [y], [1,0,0]) :: " +
          python_mixed_bounds_5 +
          ";\n"
          "solve satisfy;\n");

        (void) new FlatZincErrorTest("blackbox::duplicate_bounds_reason_entry",
          std::string(blackbox_bounds_decl) +
          "var 0..10: x;\n"
          "var 0..10: y;\n"
          "constraint gecode_blackbox_bounds([x,y], [], [1,0,0,1,0,0]) :: " +
          python_bounds_5_twice +
          ";\n"
          "solve satisfy;\n");

        (void) new FlatZincErrorTest("blackbox::invalid_bounds_reason_code",
          std::string(blackbox_bounds_decl) +
          "var 0..10: x;\n"
          "constraint gecode_blackbox_bounds([x], [], [1,1,1,0,0]) :: " +
          python_bounds_5 +
          ";\n"
          "solve satisfy;\n");

        (void) new FlatZincErrorTest("blackbox::invalid_float_output",
          std::string(blackbox_decl) +
          "var 0.0..10.0: y;\n"
          "constraint gecode_blackbox([], [], [], [y]) :: " +
          python_nan +
          ";\n"
          "solve satisfy;\n");

        (void) new FlatZincErrorTest("blackbox::nul_output",
          std::string(blackbox_decl) +
          "constraint gecode_blackbox([], [], [], []) :: " +
          python_nul +
          ";\n"
          "solve satisfy;\n");

        (void) new FlatZincErrorTest("blackbox::malformed_annotation",
          std::string(blackbox_decl) +
          "var 0..1: y;\n"
          "constraint gecode_blackbox([], [], [y], []) :: "
          "blackbox_exec([]);\n"
          "solve satisfy;\n");

        (void) new FlatZincErrorTest("blackbox::missing_exec_parallel",
          std::string(blackbox_decl) +
          "var 0..1: x :: output_var;\n"
          "var 0..1: y :: output_var;\n"
          "constraint gecode_blackbox([x], [], [y], []) :: "
          "blackbox_exec(\"gecode-blackbox-missing-program\");\n"
          "solve :: int_search([x], first_fail, indomain_min, complete) "
          "satisfy;\n",
          {"-p", "2"});

        (void) new FlatZincErrorTest("blackbox::missing_exec_root_status",
          std::string(blackbox_decl) +
          "var 0..0: x :: output_var;\n"
          "var 0..1: y :: output_var;\n"
          "constraint gecode_blackbox([x], [], [y], []) :: "
          "blackbox_exec(\"gecode-blackbox-missing-program\");\n"
          "solve satisfy;\n",
          {"-p", "2"});

        (void) new FlatZincErrorTest("blackbox::malformed_exec_parallel",
          std::string(blackbox_decl) +
          "var 0..1: x :: output_var;\n"
          "var 0..1: y :: output_var;\n"
          "constraint gecode_blackbox([x], [], [y], []) :: " +
          python_malformed +
          ";\n"
          "solve :: int_search([x], first_fail, indomain_min, complete) "
          "satisfy;\n",
          {"-p", "2"});
      }
    };

    Create c;
  }

}}

#endif

// STATISTICS: test-flatzinc
