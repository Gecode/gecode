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
        (void) new FlatZincTest("jobshop2x2",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..4] of int: d = [2, 5, 3, 4];\n\
var bool: BOOL____00001 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00002 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00004 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00005 :: is_defined_var :: var_is_introduced;\n\
var 5..14: end;\n\
array [1..4] of var 0..14: s :: output_array([1..2, 1..2]);\n\
constraint array_bool_or([BOOL____00001, BOOL____00002], true);\n\
constraint array_bool_or([BOOL____00004, BOOL____00005], true);\n\
constraint int_lin_le([-1, 1], [end, s[2]], -5);\n\
constraint int_lin_le([-1, 1], [end, s[4]], -4);\n\
constraint int_lin_le([1, -1], [s[1], s[2]], -2);\n\
constraint int_lin_le([1, -1], [s[3], s[4]], -3);\n\
constraint int_lin_le_reif([-1, 1], [s[1], s[3]], -3, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_lin_le_reif([-1, 1], [s[2], s[4]], -4, BOOL____00005) :: defines_var(BOOL____00005);\n\
constraint int_lin_le_reif([1, -1], [s[1], s[3]], -2, BOOL____00001) :: defines_var(BOOL____00001);\n\
constraint int_lin_le_reif([1, -1], [s[2], s[4]], -5, BOOL____00004) :: defines_var(BOOL____00004);\n\
solve minimize end;\n\
", "s = array2d(1..2, 1..2, [0, 2, 2, 7]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
