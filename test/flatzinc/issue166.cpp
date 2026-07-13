/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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
 */

#include "test/flatzinc.hh"

namespace Test { namespace FlatZinc {

  namespace {
    /// Check that recomputation does not lose solutions
    bool check(const std::string& output, int expected,
               const std::string& representative) {
      const std::string separator = "----------\n";
      int solutions = 0;
      for (std::string::size_type p = 0;
           (p = output.find(separator,p)) != std::string::npos;
           p += separator.size())
        solutions++;
      return (solutions == expected) &&
        (output.find(representative) != std::string::npos);
    }

    /// Helper class to create and register test
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        (void) new FlatZincTest("Issue166::Original",
"predicate gecode_global_cardinality(array [int] of var int: x, array [int] of int: cover, array [int] of var int: counts);\n\
var {1,3}: A :: output_var;\n\
var 2..3: B :: output_var;\n\
var 2..3: C :: output_var;\n\
var 2..3: D :: output_var;\n\
var 2..5: E :: output_var;\n\
var 1..4: F :: output_var;\n\
var 2..3: G :: output_var;\n\
var 2..3: H :: output_var;\n\
constraint gecode_global_cardinality([A,D,1,3,3,C,1,H,B],[1,2,3],[G,F,E]) :: domain;\n\
solve :: int_search([A,B,C,D,E,F,G,H],anti_first_fail,indomain_min,complete) satisfy;\n",
          "", true, {"-a"}, [] (const std::string& output) {
            return check(output, 26,
                         "A = 1;\nB = 2;\nC = 3;\nD = 3;\nE = 5;\n"
                         "F = 1;\nG = 3;\nH = 3;\n");
          });
        (void) new FlatZincTest("Issue166::Minimized",
"predicate gecode_global_cardinality(array [int] of var int: x, array [int] of int: cover, array [int] of var int: counts);\n\
var 1..3: A :: output_var;\n\
var 1..3: B :: output_var;\n\
var 1..3: C :: output_var;\n\
var 1..2: D :: output_var;\n\
var 2..3: E :: output_var;\n\
var 2..3: G :: output_var;\n\
constraint gecode_global_cardinality([3,A,D,G,E],[1,2,3],[1,B,C]) :: domain;\n\
solve :: int_search([D,A,E,B],input_order,indomain_random,complete) satisfy;\n",
          "", true, {"-a", "-r", "0", "-c-d", "8"},
          [] (const std::string& output) {
            return check(output, 11,
                         "A = 3;\nB = 1;\nC = 3;\nD = 1;\nE = 2;\nG = 3;\n");
          });
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
