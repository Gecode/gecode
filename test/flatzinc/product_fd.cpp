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
        (void) new FlatZincTest("product::fd",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..2] of int: capacity = [200, 400];\n\
array [1..6] of int: consumption = [5, 2, 4, 4, 3, 6];\n\
array [1..3] of int: demand = [100, 200, 300];\n\
array [1..3] of int: insideCost = [60, 80, 30];\n\
array [1..3] of int: outsideCost = [80, 90, 40];\n\
var 0..131000: INT____00001 :: is_defined_var :: var_is_introduced;\n\
array [1..3] of var 0..400: inside :: output_array([1..3]);\n\
array [1..3] of var 0..300: outside :: output_array([1..3]);\n\
constraint int_lin_eq([-1, 60, 80, 30, 80, 90, 40], [INT____00001, inside[1], inside[2], inside[3], outside[1], outside[2], outside[3]], 0) :: defines_var(INT____00001);\n\
constraint int_lin_le([-1, -1], [inside[1], outside[1]], -100);\n\
constraint int_lin_le([-1, -1], [inside[2], outside[2]], -200);\n\
constraint int_lin_le([-1, -1], [inside[3], outside[3]], -300);\n\
constraint int_lin_le([2, 4, 6], [inside[1], inside[2], inside[3]], 400);\n\
constraint int_lin_le([5, 4, 3], [inside[1], inside[2], inside[3]], 200);\n\
solve minimize INT____00001;\n\
", "inside = array1d(1..3, [40, 0, 0]);\n\
outside = array1d(1..3, [60, 200, 300]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
