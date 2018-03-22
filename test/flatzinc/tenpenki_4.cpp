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
        (void) new FlatZincTest("tenpenki::4",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
var bool: BOOL____00003 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00004 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00016 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00017 :: is_defined_var :: var_is_introduced;\n\
array [1..9] of var bool: a :: output_array([1..3, 1..3]);\n\
array [1..1] of var 1..3: s____00001;\n\
array [1..1] of var 1..3: s____00007;\n\
array [1..1] of var 1..3: s____00011;\n\
array [1..1] of var 1..3: s____00014;\n\
array [1..1] of var 1..3: s____00020;\n\
array [1..1] of var 1..3: s____00024;\n\
constraint array_bool_and([BOOL____00003, BOOL____00004], a[6]);\n\
constraint array_bool_and([BOOL____00016, BOOL____00017], a[8]);\n\
constraint bool_eq(a[3], true);\n\
constraint bool_eq(a[7], true);\n\
constraint int_le_reif(s____00001[1], 1, a[3]);\n\
constraint int_le_reif(s____00001[1], 2, BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_le_reif(s____00007[1], 1, a[2]);\n\
constraint int_le_reif(s____00007[1], 2, a[5]);\n\
constraint int_le_reif(s____00011[1], 1, a[1]);\n\
constraint int_le_reif(s____00011[1], 2, a[4]);\n\
constraint int_le_reif(s____00014[1], 1, a[7]);\n\
constraint int_le_reif(s____00014[1], 2, BOOL____00016) :: defines_var(BOOL____00016);\n\
constraint int_le_reif(s____00020[1], 1, a[4]);\n\
constraint int_le_reif(s____00020[1], 2, a[5]);\n\
constraint int_le_reif(s____00024[1], 1, a[1]);\n\
constraint int_le_reif(s____00024[1], 2, a[2]);\n\
constraint int_lin_le([1], [s____00007[1]], 2);\n\
constraint int_lin_le([1], [s____00011[1]], 1);\n\
constraint int_lin_le([1], [s____00020[1]], 2);\n\
constraint int_lin_le([1], [s____00024[1]], 1);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -3, a[9]);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -2, BOOL____00004) :: defines_var(BOOL____00004);\n\
constraint int_lin_le_reif([-1], [s____00007[1]], -2, a[8]);\n\
constraint int_lin_le_reif([-1], [s____00014[1]], -3, a[9]);\n\
constraint int_lin_le_reif([-1], [s____00014[1]], -2, BOOL____00017) :: defines_var(BOOL____00017);\n\
constraint int_lin_le_reif([-1], [s____00020[1]], -2, a[6]);\n\
solve satisfy;\n\
", "a = array2d(1..3, 1..3, [true, true, true, true, true, false, true, false, false]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
