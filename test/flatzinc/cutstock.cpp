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
        (void) new FlatZincTest("cutstock",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..3] of int: i_demand = [2, 2, 4];\n\
array [1..3] of int: i_length = [7, 5, 3];\n\
var 0..8: INT____00001 :: is_defined_var :: var_is_introduced;\n\
array [1..24] of var int: items :: output_array([1..8, 1..3]);\n\
var 0..8: obj :: output_var = INT____00001;\n\
array [1..8] of var 0..1: pieces :: output_array([1..8]);\n\
constraint int_le(0, items[1]);\n\
constraint int_le(0, items[2]);\n\
constraint int_le(0, items[3]);\n\
constraint int_le(0, items[4]);\n\
constraint int_le(0, items[5]);\n\
constraint int_le(0, items[6]);\n\
constraint int_le(0, items[7]);\n\
constraint int_le(0, items[8]);\n\
constraint int_le(0, items[9]);\n\
constraint int_le(0, items[10]);\n\
constraint int_le(0, items[11]);\n\
constraint int_le(0, items[12]);\n\
constraint int_le(0, items[13]);\n\
constraint int_le(0, items[14]);\n\
constraint int_le(0, items[15]);\n\
constraint int_le(0, items[16]);\n\
constraint int_le(0, items[17]);\n\
constraint int_le(0, items[18]);\n\
constraint int_le(0, items[19]);\n\
constraint int_le(0, items[20]);\n\
constraint int_le(0, items[21]);\n\
constraint int_le(0, items[22]);\n\
constraint int_le(0, items[23]);\n\
constraint int_le(0, items[24]);\n\
constraint int_lin_eq([-1, 1, 1, 1, 1, 1, 1, 1, 1], [INT____00001, pieces[1], pieces[2], pieces[3], pieces[4], pieces[5], pieces[6], pieces[7], pieces[8]], 0) :: defines_var(INT____00001);\n\
constraint int_lin_le([7, 5, 3, -10], [items[1], items[2], items[3], pieces[1]], 0);\n\
constraint int_lin_le([7, 5, 3, -10], [items[4], items[5], items[6], pieces[2]], 0);\n\
constraint int_lin_le([7, 5, 3, -10], [items[7], items[8], items[9], pieces[3]], 0);\n\
constraint int_lin_le([7, 5, 3, -10], [items[10], items[11], items[12], pieces[4]], 0);\n\
constraint int_lin_le([7, 5, 3, -10], [items[13], items[14], items[15], pieces[5]], 0);\n\
constraint int_lin_le([7, 5, 3, -10], [items[16], items[17], items[18], pieces[6]], 0);\n\
constraint int_lin_le([7, 5, 3, -10], [items[19], items[20], items[21], pieces[7]], 0);\n\
constraint int_lin_le([7, 5, 3, -10], [items[22], items[23], items[24], pieces[8]], 0);\n\
constraint int_lin_le([-1, -1, -1, -1, -1, -1, -1, -1], [items[1], items[4], items[7], items[10], items[13], items[16], items[19], items[22]], -2);\n\
constraint int_lin_le([-1, -1, -1, -1, -1, -1, -1, -1], [items[2], items[5], items[8], items[11], items[14], items[17], items[20], items[23]], -2);\n\
constraint int_lin_le([-1, -1, -1, -1, -1, -1, -1, -1], [items[3], items[6], items[9], items[12], items[15], items[18], items[21], items[24]], -4);\n\
solve  :: int_search([items[1], items[2], items[3], items[4], items[5], items[6], items[7], items[8], items[9], items[10], items[11], items[12], items[13], items[14], items[15], items[16], items[17], items[18], items[19], items[20], items[21], items[22], items[23], items[24], pieces[1], pieces[2], pieces[3], pieces[4], pieces[5], pieces[6], pieces[7], pieces[8]], input_order, indomain_min, complete) minimize INT____00001;\n\
", "items = array2d(1..8, 1..3, [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 1, 0, 1, 1, 0, 1]);\n\
obj = 4;\n\
pieces = array1d(1..8, [0, 0, 0, 0, 1, 1, 1, 1]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
