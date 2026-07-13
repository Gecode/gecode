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
    /// Check that propagation terminates after finding all solutions
    bool check(const std::string& output) {
      const std::string separator = "----------\n";
      int solutions = 0;
      for (std::string::size_type p = 0;
           (p = output.find(separator,p)) != std::string::npos;
           p += separator.size())
        solutions++;
      return (solutions == 3) &&
        (output.find("==========\n") != std::string::npos);
    }

    /// Helper class to create and register test
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        (void) new FlatZincTest("Issue168",
"predicate fzn_sort(array [int] of var int: x, array [int] of var int: y);\n\
var 2..4: D :: output_var;\n\
var {2,4}: E :: output_var;\n\
var {1,4}: F :: output_var;\n\
var 3..4: G :: output_var;\n\
var {2,4}: H :: output_var;\n\
var 3..4: J :: output_var;\n\
var {1,4}: K :: output_var;\n\
constraint fzn_sort([F,K,G,D],[F,E,H,J]);\n\
solve :: int_search([1,D,F,K,F,E,G,H,J],input_order,indomain,complete) satisfy;\n",
          "", true, {"-a"}, check);
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
