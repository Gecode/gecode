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
        (void) new FlatZincTest("2DPacking",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..4] of int: ItemHeight = [4, 4, 4, 3];\n\
array [1..4] of int: ItemWidth = [1, 1, 2, 3];\n\
var 0..2: INT____00001 :: is_defined_var :: var_is_introduced;\n\
array [1..2] of var 0..1: bin;\n\
array [1..8] of var 0..1: item :: output_array([1..2, 1..4]);\n\
var 0..2: obj :: output_var = INT____00001;\n\
array [1..16] of var 0..1: x____00002;\n\
array [1..16] of var 0..1: x____00003;\n\
constraint int_eq(x____00002[5], 0);\n\
constraint int_eq(x____00002[9], 0);\n\
constraint int_eq(x____00002[10], 0);\n\
constraint int_eq(x____00002[13], 0);\n\
constraint int_eq(x____00002[14], 0);\n\
constraint int_eq(x____00002[15], 0);\n\
constraint int_eq(x____00003[5], 0);\n\
constraint int_eq(x____00003[9], 0);\n\
constraint int_eq(x____00003[10], 0);\n\
constraint int_eq(x____00003[13], 0);\n\
constraint int_eq(x____00003[14], 0);\n\
constraint int_eq(x____00003[15], 0);\n\
constraint int_lin_eq([1, -1], [item[1], x____00002[1]], 0);\n\
constraint int_lin_eq([1, -1], [item[5], x____00003[1]], 0);\n\
constraint int_lin_eq([1, 1], [item[1], item[5]], 1);\n\
constraint int_lin_eq([1, 1], [item[2], item[6]], 1);\n\
constraint int_lin_eq([1, 1], [item[3], item[7]], 1);\n\
constraint int_lin_eq([1, 1], [item[4], item[8]], 1);\n\
constraint int_lin_eq([1, -1, -1], [item[1], x____00002[2], x____00002[6]], 0);\n\
constraint int_lin_eq([1, -1, -1], [item[5], x____00003[2], x____00003[6]], 0);\n\
constraint int_lin_eq([1, -1, -1, -1], [item[1], x____00002[3], x____00002[7], x____00002[11]], 0);\n\
constraint int_lin_eq([1, -1, -1, -1], [item[5], x____00003[3], x____00003[7], x____00003[11]], 0);\n\
constraint int_lin_eq([1, -1, -1, -1, -1], [item[1], x____00002[4], x____00002[8], x____00002[12], x____00002[16]], 0);\n\
constraint int_lin_eq([1, -1, -1, -1, -1], [item[5], x____00003[4], x____00003[8], x____00003[12], x____00003[16]], 0);\n\
constraint int_lin_le([-2], [x____00002[16]], 0);\n\
constraint int_lin_le([-2], [x____00003[16]], 0);\n\
constraint int_lin_le([-3, 3], [x____00002[11], x____00002[12]], 0);\n\
constraint int_lin_le([-3, 3], [x____00003[11], x____00003[12]], 0);\n\
constraint int_lin_le([-4, 2, 3], [x____00002[6], x____00002[7], x____00002[8]], 0);\n\
constraint int_lin_le([-4, 2, 3], [x____00003[6], x____00003[7], x____00003[8]], 0);\n\
constraint int_lin_le([-4, 1, 2, 3], [x____00002[1], x____00002[2], x____00002[3], x____00002[4]], 0);\n\
constraint int_lin_le([-4, 1, 2, 3], [x____00003[1], x____00003[2], x____00003[3], x____00003[4]], 0);\n\
constraint int_lin_le([-10, 4, 4, 4, 3], [bin[1], x____00002[1], x____00002[6], x____00002[11], x____00002[16]], 0);\n\
constraint int_lin_le([-10, 4, 4, 4, 3], [bin[2], x____00003[1], x____00003[6], x____00003[11], x____00003[16]], 0);\n\
constraint int_plus(bin[1], bin[2], INT____00001) :: defines_var(INT____00001);\n\
solve minimize INT____00001;\n\
",
"item = array2d(1..2, 1..4, [1, 1, 1, 1, 0, 0, 0, 0]);\n\
obj = 1;\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
