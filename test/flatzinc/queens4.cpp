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
        (void) new FlatZincTest("queens4",
"array [1..4] of var 1..4: q :: output_array([1..4]);\n\
var int: dq01 :: output_var;\n\
var int: dq02 :: output_var;\n\
var int: dq03 :: output_var;\n\
var int: dq12 :: output_var;\n\
var int: dq13 :: output_var;\n\
var int: dq23 :: output_var;\n\
constraint int_plus(dq01, q[2], q[1]);\n\
constraint int_plus(dq02, q[3], q[1]);\n\
constraint int_plus(dq03, q[4], q[1]);\n\
constraint int_plus(dq12, q[3], q[2]);\n\
constraint int_plus(dq13, q[4], q[2]);\n\
constraint int_plus(dq23, q[4], q[3]);\n\
constraint int_ne(q[1], q[2]);\n\
constraint int_ne(q[1], q[3]);\n\
constraint int_ne(q[1], q[4]);\n\
constraint int_ne(q[2], q[3]);\n\
constraint int_ne(q[2], q[4]);\n\
constraint int_ne(q[3], q[4]);\n\
constraint int_ne(dq01, -1);\n\
constraint int_ne(dq01,  1);\n\
constraint int_ne(dq02, -2);\n\
constraint int_ne(dq02,  2);\n\
constraint int_ne(dq03, -3);\n\
constraint int_ne(dq03,  3);\n\
constraint int_ne(dq12, -1);\n\
constraint int_ne(dq12,  1);\n\
constraint int_ne(dq13, -2);\n\
constraint int_ne(dq13,  2);\n\
constraint int_ne(dq23, -1);\n\
constraint int_ne(dq23,  1);\n\
solve satisfy;\n\
", "dq01 = 2;\n\
dq02 = -1;\n\
dq03 = 1;\n\
dq12 = -3;\n\
dq13 = -1;\n\
dq23 = 2;\n\
q = array1d(1..4, [3, 1, 4, 2]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
