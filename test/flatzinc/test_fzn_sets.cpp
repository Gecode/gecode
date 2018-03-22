/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2014
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

#include "test/flatzinc.hh"

namespace Test { namespace FlatZinc {

  namespace {
    /// Helper class to create and register tests
    class Create {
    public:

      /// Perform creation and registration
      Create(void) {
        (void) new FlatZincTest("fzn_sets",
"var set of 1..10: s1 :: output_var;\n\
var set of 1..10: s2 :: output_var;\n\
var set of 1..10: s3 :: output_var;\n\
var set of 1..10: s4 :: output_var;\n\
var set of 1..10: s5 :: output_var;\n\
var set of 1..10: s6 :: output_var;\n\
var set of 1..10: s7 :: output_var;\n\
var set of 1..10: s8 :: output_var;\n\
\n\
var bool: rs1 :: output_var;\n\
var bool: rs2 :: output_var;\n\
var bool: rs3 :: output_var;\n\
\n\
constraint set_in(1, {1, 2, 3});\n\
constraint set_in(1, s1);\n\
constraint set_in_reif(2, s1, rs1);\n\
\n\
constraint set_subset({1, 2}, {1, 2, 3});\n\
constraint set_subset({1, 2}, s2);\n\
constraint set_subset_reif({1, 2, 3}, s2, rs2);\n\
\n\
constraint set_subset({1, 2}, {1, 2, 3});\n\
constraint set_subset({1, 2, 3}, s3);\n\
constraint set_subset_reif({1, 2, 3, 4}, s3, rs3);\n\
\n\
constraint set_intersect({1, 2, 3}, {1, 3, 5}, {1, 3});\n\
constraint set_intersect(s4, {1, 2, 3}, {1, 3});\n\
\n\
constraint set_diff({1, 2, 3}, {1, 3, 5}, {2});\n\
constraint set_diff(s5, {2}, {1, 3});\n\
\n\
constraint set_symdiff({1, 2, 3}, {1, 3, 5}, {2, 5});\n\
constraint set_symdiff(s6, {2, 5}, {1, 5});\n\
\n\
constraint set_union({1, 2, 3}, {1, 3, 5}, {1, 2, 3, 5});\n\
constraint set_union(s7, {2, 5}, {1, 2, 3, 4, 5});\n\
\n\
constraint set_card({1, 2, 3}, 3);\n\
constraint set_card(s8, 3);\n\
\n\
solve satisfy;\n\
",
"rs1 = false;\n\
rs2 = false;\n\
rs3 = false;\n\
s1 = {1, 3, 4, 5, 6, 7, 8, 9, 10};\n\
s2 = {1, 2, 4, 5, 6, 7, 8, 9, 10};\n\
s3 = {1, 2, 3, 5, 6, 7, 8, 9, 10};\n\
s4 = {1, 3, 4, 5, 6, 7, 8, 9, 10};\n\
s5 = 1..3;\n\
s6 = 1..2;\n\
s7 = 1..5;\n\
s8 = 1..3;\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
