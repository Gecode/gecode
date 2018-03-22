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
        (void) new FlatZincTest("trucking",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..4] of int: Cost = [30, 27, 23, 20];\n\
array [1..6] of int: Demand = [27, 11, 14, 19, 25, 22];\n\
array [1..4] of int: Loads = [20, 18, 15, 13];\n\
var 0..600: INT____00001 :: is_defined_var :: var_is_introduced;\n\
var 0..600: obj :: output_var = INT____00001;\n\
array [1..24] of var 0..1: x :: output_array([1..4, 1..6]);\n\
constraint int_lin_eq([-1, 30, 30, 30, 30, 30, 30, 27, 27, 27, 27, 27, 27, 23, 23, 23, 23, 23, 23, 20, 20, 20, 20, 20, 20], [INT____00001, x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], x[9], x[10], x[11], x[12], x[13], x[14], x[15], x[16], x[17], x[18], x[19], x[20], x[21], x[22], x[23], x[24]], 0) :: defines_var(INT____00001);\n\
constraint int_lin_le([1, 1], [x[19], x[20]], 1);\n\
constraint int_lin_le([1, 1], [x[20], x[21]], 1);\n\
constraint int_lin_le([1, 1], [x[21], x[22]], 1);\n\
constraint int_lin_le([1, 1], [x[22], x[23]], 1);\n\
constraint int_lin_le([1, 1], [x[23], x[24]], 1);\n\
constraint int_lin_le([1, 1, 1], [x[13], x[14], x[15]], 1);\n\
constraint int_lin_le([1, 1, 1], [x[14], x[15], x[16]], 1);\n\
constraint int_lin_le([1, 1, 1], [x[15], x[16], x[17]], 1);\n\
constraint int_lin_le([1, 1, 1], [x[16], x[17], x[18]], 1);\n\
constraint int_lin_le([-20, -18, -15, -13], [x[1], x[7], x[13], x[19]], -27);\n\
constraint int_lin_le([-20, -18, -15, -13], [x[2], x[8], x[14], x[20]], -11);\n\
constraint int_lin_le([-20, -18, -15, -13], [x[3], x[9], x[15], x[21]], -14);\n\
constraint int_lin_le([-20, -18, -15, -13], [x[4], x[10], x[16], x[22]], -19);\n\
constraint int_lin_le([-20, -18, -15, -13], [x[5], x[11], x[17], x[23]], -25);\n\
constraint int_lin_le([-20, -18, -15, -13], [x[6], x[12], x[18], x[24]], -22);\n\
solve ::int_search(x, first_fail, indomain_min, complete) minimize INT____00001;\n\
", "obj = 224;\n\
x = array2d(1..4, 1..6, [0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
