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
        (void) new FlatZincTest("simple_sat",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..21] of int: Formula = [1, 2, 3, 1, 2, -3, 1, -2, 3, 1, -2, -3, -1, 2, 3, -1, 2, -3, -1, -2, 3];\n\
var bool: BOOL____00001 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00002 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00004 :: is_defined_var :: var_is_introduced;\n\
array [1..3] of var bool: assignment :: output_array([1..3]);\n\
constraint array_bool_or([BOOL____00004, BOOL____00001, assignment[2]], true);\n\
constraint array_bool_or([BOOL____00004, assignment[1], BOOL____00002], true);\n\
constraint array_bool_or([BOOL____00004, assignment[1], assignment[2]], true);\n\
constraint array_bool_or([assignment[3], BOOL____00001, BOOL____00002], true);\n\
constraint array_bool_or([assignment[3], BOOL____00001, assignment[2]], true);\n\
constraint array_bool_or([assignment[3], assignment[1], BOOL____00002], true);\n\
constraint array_bool_or([assignment[3], assignment[1], assignment[2]], true);\n\
constraint bool_eq_reif(assignment[1], false, BOOL____00001) :: defines_var(BOOL____00001);\n\
constraint bool_eq_reif(assignment[2], false, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint bool_eq_reif(assignment[3], false, BOOL____00004) :: defines_var(BOOL____00004);\n\
solve satisfy;\n\
", "assignment = array1d(1..3, [true, true, true]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
