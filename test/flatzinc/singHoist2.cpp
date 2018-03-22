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
        (void) new FlatZincTest("singHoist2",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..16] of int: Empty = [0, 2, 3, 3, 2, 0, 2, 3, 3, 2, 0, 2, 3, 3, 2, 0];\n\
array [1..4] of int: Full = [4, 4, 4, 5];\n\
array [1..3] of int: MaxTime = [10, 25, 10];\n\
array [1..3] of int: MinTime = [10, 25, 10];\n\
var bool: BOOL____00001 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00002 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00003 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00004 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00005 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00006 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00007 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00008 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00009 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00010 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00011 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00012 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00013 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00014 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00015 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00016 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00017 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00018 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00019 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00020 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00021 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00022 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00023 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00024 :: is_defined_var :: var_is_introduced;\n\
array [1..4] of var 0..135: Entry :: output_array([0..3]);\n\
var 0..45: Period :: output_var;\n\
array [1..4] of var 0..135: Removal :: output_array([0..3]);\n\
constraint array_bool_or([BOOL____00001, BOOL____00002], true);\n\
constraint array_bool_or([BOOL____00003, BOOL____00004], true);\n\
constraint array_bool_or([BOOL____00005, BOOL____00006], true);\n\
constraint array_bool_or([BOOL____00007, BOOL____00008], true);\n\
constraint array_bool_or([BOOL____00009, BOOL____00010], true);\n\
constraint array_bool_or([BOOL____00011, BOOL____00012], true);\n\
constraint array_bool_or([BOOL____00013, BOOL____00014], true);\n\
constraint array_bool_or([BOOL____00015, BOOL____00016], true);\n\
constraint array_bool_or([BOOL____00017, BOOL____00018], true);\n\
constraint array_bool_or([BOOL____00019, BOOL____00020], true);\n\
constraint array_bool_or([BOOL____00021, BOOL____00022], true);\n\
constraint array_bool_or([BOOL____00023, BOOL____00024], true);\n\
constraint int_eq(Removal[1], 0);\n\
constraint int_lin_eq([-1, 1], [Entry[1], Removal[4]], -5);\n\
constraint int_lin_eq([-1, 1], [Entry[2], Removal[1]], -4);\n\
constraint int_lin_eq([-1, 1], [Entry[3], Removal[2]], -4);\n\
constraint int_lin_eq([-1, 1], [Entry[4], Removal[3]], -4);\n\
constraint int_lin_le([-3, 1], [Period, Removal[4]], -5);\n\
constraint int_lin_le([-1, 1], [Entry[2], Removal[2]], 10);\n\
constraint int_lin_le([-1, 1], [Entry[3], Removal[3]], 25);\n\
constraint int_lin_le([-1, 1], [Entry[4], Removal[4]], 10);\n\
constraint int_lin_le([1, -1], [Entry[2], Removal[2]], -10);\n\
constraint int_lin_le([1, -1], [Entry[3], Removal[3]], -25);\n\
constraint int_lin_le([1, -1], [Entry[4], Removal[4]], -10);\n\
constraint int_lin_le_reif([-2, 1, -1], [Period, Entry[1], Removal[1]], 0, BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint int_lin_le_reif([-2, 1, -1], [Period, Entry[1], Removal[2]], -2, BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_lin_le_reif([-2, 1, -1], [Period, Entry[1], Removal[3]], -3, BOOL____00024) :: defines_var(BOOL____00024);\n\
constraint int_lin_le_reif([-2, 1, -1], [Period, Entry[3], Removal[1]], -3, BOOL____00004) :: defines_var(BOOL____00004);\n\
constraint int_lin_le_reif([-2, 1, -1], [Period, Entry[4], Removal[1]], -3, BOOL____00008) :: defines_var(BOOL____00008);\n\
constraint int_lin_le_reif([-2, 1, -1], [Period, Entry[4], Removal[2]], -3, BOOL____00016) :: defines_var(BOOL____00016);\n\
constraint int_lin_le_reif([-1, 1, -1], [Period, Entry[1], Removal[1]], 0, BOOL____00010) :: defines_var(BOOL____00010);\n\
constraint int_lin_le_reif([-1, 1, -1], [Period, Entry[1], Removal[2]], -2, BOOL____00018) :: defines_var(BOOL____00018);\n\
constraint int_lin_le_reif([-1, 1, -1], [Period, Entry[1], Removal[3]], -3, BOOL____00022) :: defines_var(BOOL____00022);\n\
constraint int_lin_le_reif([-1, 1, -1], [Period, Entry[3], Removal[1]], -3, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_lin_le_reif([-1, 1, -1], [Period, Entry[4], Removal[1]], -3, BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_lin_le_reif([-1, 1, -1], [Period, Entry[4], Removal[2]], -3, BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint int_lin_le_reif([1, 1, -1], [Period, Entry[2], Removal[2]], 0, BOOL____00001) :: defines_var(BOOL____00001);\n\
constraint int_lin_le_reif([1, 1, -1], [Period, Entry[2], Removal[3]], -2, BOOL____00005) :: defines_var(BOOL____00005);\n\
constraint int_lin_le_reif([1, 1, -1], [Period, Entry[2], Removal[4]], -3, BOOL____00009) :: defines_var(BOOL____00009);\n\
constraint int_lin_le_reif([1, 1, -1], [Period, Entry[3], Removal[3]], 0, BOOL____00013) :: defines_var(BOOL____00013);\n\
constraint int_lin_le_reif([1, 1, -1], [Period, Entry[3], Removal[4]], -2, BOOL____00017) :: defines_var(BOOL____00017);\n\
constraint int_lin_le_reif([1, 1, -1], [Period, Entry[4], Removal[4]], 0, BOOL____00021) :: defines_var(BOOL____00021);\n\
constraint int_lin_le_reif([2, 1, -1], [Period, Entry[2], Removal[2]], 0, BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_lin_le_reif([2, 1, -1], [Period, Entry[2], Removal[3]], -2, BOOL____00007) :: defines_var(BOOL____00007);\n\
constraint int_lin_le_reif([2, 1, -1], [Period, Entry[2], Removal[4]], -3, BOOL____00011) :: defines_var(BOOL____00011);\n\
constraint int_lin_le_reif([2, 1, -1], [Period, Entry[3], Removal[3]], 0, BOOL____00015) :: defines_var(BOOL____00015);\n\
constraint int_lin_le_reif([2, 1, -1], [Period, Entry[3], Removal[4]], -2, BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint int_lin_le_reif([2, 1, -1], [Period, Entry[4], Removal[4]], 0, BOOL____00023) :: defines_var(BOOL____00023);\n\
solve minimize Period;\n\
", "Entry = array1d(0..3, [62, 4, 18, 47]);\n\
Period = 25;\n\
Removal = array1d(0..3, [0, 14, 43, 57]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
