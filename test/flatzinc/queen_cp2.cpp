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
        (void) new FlatZincTest("queens::cp2",
"predicate all_different_int(array [int] of var int: x);\n\
predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
var 2..9: INT____00001 :: is_defined_var :: var_is_introduced;\n\
var 3..10: INT____00002 :: is_defined_var :: var_is_introduced;\n\
var 4..11: INT____00003 :: is_defined_var :: var_is_introduced;\n\
var 5..12: INT____00004 :: is_defined_var :: var_is_introduced;\n\
var 6..13: INT____00005 :: is_defined_var :: var_is_introduced;\n\
var 7..14: INT____00006 :: is_defined_var :: var_is_introduced;\n\
var 8..15: INT____00007 :: is_defined_var :: var_is_introduced;\n\
var 8..15: INT____00008 :: is_defined_var :: var_is_introduced;\n\
var 7..14: INT____00009 :: is_defined_var :: var_is_introduced;\n\
var 6..13: INT____00010 :: is_defined_var :: var_is_introduced;\n\
var 5..12: INT____00011 :: is_defined_var :: var_is_introduced;\n\
var 4..11: INT____00012 :: is_defined_var :: var_is_introduced;\n\
var 3..10: INT____00013 :: is_defined_var :: var_is_introduced;\n\
var 2..9: INT____00014 :: is_defined_var :: var_is_introduced;\n\
array [1..8] of var 1..8: q :: output_array([1..8]);\n\
constraint all_different_int([INT____00008, INT____00009, INT____00010, INT____00011, INT____00012, INT____00013, INT____00014, q[8]]);\n\
constraint all_different_int([q[1], INT____00001, INT____00002, INT____00003, INT____00004, INT____00005, INT____00006, INT____00007]);\n\
constraint all_different_int(q);\n\
constraint int_lin_eq([-1, 1], [INT____00001, q[2]], -1) :: defines_var(INT____00001);\n\
constraint int_lin_eq([-1, 1], [INT____00002, q[3]], -2) :: defines_var(INT____00002);\n\
constraint int_lin_eq([-1, 1], [INT____00003, q[4]], -3) :: defines_var(INT____00003);\n\
constraint int_lin_eq([-1, 1], [INT____00004, q[5]], -4) :: defines_var(INT____00004);\n\
constraint int_lin_eq([-1, 1], [INT____00005, q[6]], -5) :: defines_var(INT____00005);\n\
constraint int_lin_eq([-1, 1], [INT____00006, q[7]], -6) :: defines_var(INT____00006);\n\
constraint int_lin_eq([-1, 1], [INT____00007, q[8]], -7) :: defines_var(INT____00007);\n\
constraint int_lin_eq([-1, 1], [INT____00008, q[1]], -7) :: defines_var(INT____00008);\n\
constraint int_lin_eq([-1, 1], [INT____00009, q[2]], -6) :: defines_var(INT____00009);\n\
constraint int_lin_eq([-1, 1], [INT____00010, q[3]], -5) :: defines_var(INT____00010);\n\
constraint int_lin_eq([-1, 1], [INT____00011, q[4]], -4) :: defines_var(INT____00011);\n\
constraint int_lin_eq([-1, 1], [INT____00012, q[5]], -3) :: defines_var(INT____00012);\n\
constraint int_lin_eq([-1, 1], [INT____00013, q[6]], -2) :: defines_var(INT____00013);\n\
constraint int_lin_eq([-1, 1], [INT____00014, q[7]], -1) :: defines_var(INT____00014);\n\
solve  :: int_search(q, first_fail, indomain_min, complete) satisfy;\n\
", "q = array1d(1..8, [1, 5, 8, 6, 3, 7, 2, 4]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
