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
        (void) new FlatZincTest("langford2",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
var bool: BOOL____00002 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00006 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00008 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00012 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00014 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00018 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00020 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00024 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00026 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00030 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00032 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00036 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00038 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00042 :: is_defined_var :: var_is_introduced;\n\
array [1..7] of var 1..14: Fst;\n\
var 3..16: INT____00003 :: is_defined_var :: var_is_introduced;\n\
var 1..14: INT____00004 :: is_defined_var :: var_is_introduced;\n\
var 1..7: INT____00005 :: is_defined_var :: var_is_introduced;\n\
var 4..17: INT____00009 :: is_defined_var :: var_is_introduced;\n\
var 1..14: INT____00010 :: is_defined_var :: var_is_introduced;\n\
var 1..7: INT____00011 :: is_defined_var :: var_is_introduced;\n\
var 5..18: INT____00015 :: is_defined_var :: var_is_introduced;\n\
var 1..14: INT____00016 :: is_defined_var :: var_is_introduced;\n\
var 1..7: INT____00017 :: is_defined_var :: var_is_introduced;\n\
var 6..19: INT____00021 :: is_defined_var :: var_is_introduced;\n\
var 1..14: INT____00022 :: is_defined_var :: var_is_introduced;\n\
var 1..7: INT____00023 :: is_defined_var :: var_is_introduced;\n\
var 7..20: INT____00027 :: is_defined_var :: var_is_introduced;\n\
var 1..14: INT____00028 :: is_defined_var :: var_is_introduced;\n\
var 1..7: INT____00029 :: is_defined_var :: var_is_introduced;\n\
var 8..21: INT____00033 :: is_defined_var :: var_is_introduced;\n\
var 1..14: INT____00034 :: is_defined_var :: var_is_introduced;\n\
var 1..7: INT____00035 :: is_defined_var :: var_is_introduced;\n\
var 9..22: INT____00039 :: is_defined_var :: var_is_introduced;\n\
var 1..14: INT____00040 :: is_defined_var :: var_is_introduced;\n\
var 1..7: INT____00041 :: is_defined_var :: var_is_introduced;\n\
array [1..14] of var 1..7: a :: output_array([1..14]);\n\
constraint array_bool_and([BOOL____00006, BOOL____00002, BOOL____00002], true);\n\
constraint array_bool_and([BOOL____00012, BOOL____00008, BOOL____00008], true);\n\
constraint array_bool_and([BOOL____00018, BOOL____00014, BOOL____00014], true);\n\
constraint array_bool_and([BOOL____00024, BOOL____00020, BOOL____00020], true);\n\
constraint array_bool_and([BOOL____00030, BOOL____00026, BOOL____00026], true);\n\
constraint array_bool_and([BOOL____00036, BOOL____00032, BOOL____00032], true);\n\
constraint array_bool_and([BOOL____00042, BOOL____00038, BOOL____00038], true);\n\
constraint array_var_int_element(INT____00004, a, INT____00005) :: defines_var(INT____00005);\n\
constraint array_var_int_element(INT____00010, a, INT____00011) :: defines_var(INT____00011);\n\
constraint array_var_int_element(INT____00016, a, INT____00017) :: defines_var(INT____00017);\n\
constraint array_var_int_element(INT____00022, a, INT____00023) :: defines_var(INT____00023);\n\
constraint array_var_int_element(INT____00028, a, INT____00029) :: defines_var(INT____00029);\n\
constraint array_var_int_element(INT____00034, a, INT____00035) :: defines_var(INT____00035);\n\
constraint array_var_int_element(INT____00040, a, INT____00041) :: defines_var(INT____00041);\n\
constraint array_var_int_element(Fst[1], a, 1);\n\
constraint array_var_int_element(Fst[2], a, 2);\n\
constraint array_var_int_element(Fst[3], a, 3);\n\
constraint array_var_int_element(Fst[4], a, 4);\n\
constraint array_var_int_element(Fst[5], a, 5);\n\
constraint array_var_int_element(Fst[6], a, 6);\n\
constraint array_var_int_element(Fst[7], a, 7);\n\
constraint int_eq_reif(INT____00003, INT____00004, BOOL____00002);\n\
constraint int_eq_reif(INT____00005, 1, BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_eq_reif(INT____00009, INT____00010, BOOL____00008);\n\
constraint int_eq_reif(INT____00011, 2, BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint int_eq_reif(INT____00015, INT____00016, BOOL____00014);\n\
constraint int_eq_reif(INT____00017, 3, BOOL____00018) :: defines_var(BOOL____00018);\n\
constraint int_eq_reif(INT____00021, INT____00022, BOOL____00020);\n\
constraint int_eq_reif(INT____00023, 4, BOOL____00024) :: defines_var(BOOL____00024);\n\
constraint int_eq_reif(INT____00027, INT____00028, BOOL____00026);\n\
constraint int_eq_reif(INT____00029, 5, BOOL____00030) :: defines_var(BOOL____00030);\n\
constraint int_eq_reif(INT____00033, INT____00034, BOOL____00032);\n\
constraint int_eq_reif(INT____00035, 6, BOOL____00036) :: defines_var(BOOL____00036);\n\
constraint int_eq_reif(INT____00039, INT____00040, BOOL____00038);\n\
constraint int_eq_reif(INT____00041, 7, BOOL____00042) :: defines_var(BOOL____00042);\n\
constraint int_le(Fst[1], 12);\n\
constraint int_le(Fst[2], 11);\n\
constraint int_le(Fst[3], 10);\n\
constraint int_le(Fst[4], 9);\n\
constraint int_le(Fst[5], 8);\n\
constraint int_le(Fst[6], 7);\n\
constraint int_le(Fst[7], 6);\n\
constraint int_le(a[1], a[14]);\n\
constraint int_lin_eq([-1, 1], [INT____00003, Fst[1]], -2) :: defines_var(INT____00003) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00009, Fst[2]], -3) :: defines_var(INT____00009) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00015, Fst[3]], -4) :: defines_var(INT____00015) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00021, Fst[4]], -5) :: defines_var(INT____00021) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00027, Fst[5]], -6) :: defines_var(INT____00027) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00033, Fst[6]], -7) :: defines_var(INT____00033) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00039, Fst[7]], -8) :: defines_var(INT____00039) :: domain;\n\
constraint int_lin_le_reif([1], [Fst[1]], 12, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_lin_le_reif([1], [Fst[2]], 11, BOOL____00008) :: defines_var(BOOL____00008);\n\
constraint int_lin_le_reif([1], [Fst[3]], 10, BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint int_lin_le_reif([1], [Fst[4]], 9, BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_lin_le_reif([1], [Fst[5]], 8, BOOL____00026) :: defines_var(BOOL____00026);\n\
constraint int_lin_le_reif([1], [Fst[6]], 7, BOOL____00032) :: defines_var(BOOL____00032);\n\
constraint int_lin_le_reif([1], [Fst[7]], 6, BOOL____00038) :: defines_var(BOOL____00038);\n\
solve  :: int_search(Fst, first_fail, indomain_min, complete) satisfy;\n\
", "a = array1d(1..14, [1, 7, 1, 2, 6, 4, 2, 5, 3, 7, 4, 6, 3, 5]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
