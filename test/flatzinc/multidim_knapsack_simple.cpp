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
        (void) new FlatZincTest("multidim_knapsack_simple",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..3] of int: Capacities = [5, 10, 15];\n\
array [1..5] of int: Profits = [5, 6, 3, 7, 4];\n\
array [1..15] of int: Weights = [2, 3, 2, 1, 4, 4, 1, 2, 5, 3, 2, 3, 1, 3, 5];\n\
var 0..25: INT____00001 :: is_defined_var :: var_is_introduced;\n\
array [1..5] of var 0..1: x :: output_array([1..5]);\n\
constraint int_lin_eq([-1, 5, 6, 3, 7, 4], [INT____00001, x[1], x[2], x[3], x[4], x[5]], 0) :: defines_var(INT____00001);\n\
constraint int_lin_le([2, 1, 1, 3, 1], [x[1], x[2], x[3], x[4], x[5]], 5);\n\
constraint int_lin_le([2, 4, 5, 3, 5], [x[1], x[2], x[3], x[4], x[5]], 15);\n\
constraint int_lin_le([3, 4, 2, 2, 3], [x[1], x[2], x[3], x[4], x[5]], 10);\n\
solve maximize INT____00001;\n\
", "x = array1d(1..5, [0, 1, 0, 1, 1]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
