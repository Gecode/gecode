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
        (void) new FlatZincTest("perfect_square::1",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..11] of int: sq = [0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100];\n\
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
var 2..11: INT____00001 :: is_defined_var :: var_is_introduced;\n\
var 1..10: INT____00002 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00023 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00024 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00025 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00026 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00027 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00028 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00029 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00030 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00031 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00032 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00033 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00034 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00035 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00036 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00037 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00038 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00039 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00040 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00041 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00042 :: is_defined_var :: var_is_introduced;\n\
var 1..11: INT____00043 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00044 :: is_defined_var :: var_is_introduced;\n\
var 2..11: INT____00045 :: is_defined_var :: var_is_introduced;\n\
var 0..100: INT____00047 :: is_defined_var :: var_is_introduced;\n\
var 1..10: j;\n\
var 1..10: k :: output_var;\n\
array [1..11] of var 0..10: s :: output_array([0..10]);\n\
constraint array_int_element(INT____00023, sq, INT____00024) :: defines_var(INT____00024);\n\
constraint array_int_element(INT____00025, sq, INT____00026) :: defines_var(INT____00026);\n\
constraint array_int_element(INT____00027, sq, INT____00028) :: defines_var(INT____00028);\n\
constraint array_int_element(INT____00029, sq, INT____00030) :: defines_var(INT____00030);\n\
constraint array_int_element(INT____00031, sq, INT____00032) :: defines_var(INT____00032);\n\
constraint array_int_element(INT____00033, sq, INT____00034) :: defines_var(INT____00034);\n\
constraint array_int_element(INT____00035, sq, INT____00036) :: defines_var(INT____00036);\n\
constraint array_int_element(INT____00037, sq, INT____00038) :: defines_var(INT____00038);\n\
constraint array_int_element(INT____00039, sq, INT____00040) :: defines_var(INT____00040);\n\
constraint array_int_element(INT____00041, sq, INT____00042) :: defines_var(INT____00042);\n\
constraint array_int_element(INT____00043, sq, INT____00044) :: defines_var(INT____00044);\n\
constraint array_int_element(INT____00045, sq, INT____00047);\n\
constraint array_var_int_element(INT____00001, s, INT____00002) :: defines_var(INT____00002);\n\
constraint bool_le(BOOL____00003, BOOL____00004);\n\
constraint bool_le(BOOL____00005, BOOL____00006);\n\
constraint bool_le(BOOL____00007, BOOL____00008);\n\
constraint bool_le(BOOL____00009, BOOL____00010);\n\
constraint bool_le(BOOL____00011, BOOL____00012);\n\
constraint bool_le(BOOL____00013, BOOL____00014);\n\
constraint bool_le(BOOL____00015, BOOL____00016);\n\
constraint bool_le(BOOL____00017, BOOL____00018);\n\
constraint bool_le(BOOL____00019, BOOL____00020);\n\
constraint bool_le(BOOL____00021, BOOL____00022);\n\
constraint int_lin_eq([-1, 1], [INT____00001, j], -1) :: defines_var(INT____00001) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00023, s[1]], -1) :: defines_var(INT____00023) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00025, s[2]], -1) :: defines_var(INT____00025) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00027, s[3]], -1) :: defines_var(INT____00027) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00029, s[4]], -1) :: defines_var(INT____00029) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00031, s[5]], -1) :: defines_var(INT____00031) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00033, s[6]], -1) :: defines_var(INT____00033) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00035, s[7]], -1) :: defines_var(INT____00035) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00037, s[8]], -1) :: defines_var(INT____00037) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00039, s[9]], -1) :: defines_var(INT____00039) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00041, s[10]], -1) :: defines_var(INT____00041) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00043, s[11]], -1) :: defines_var(INT____00043) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00045, k], -1) :: defines_var(INT____00045) :: domain;\n\
constraint int_lin_eq([-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [INT____00047, INT____00024, INT____00026, INT____00028, INT____00030, INT____00032, INT____00034, INT____00036, INT____00038, INT____00040, INT____00042, INT____00044], 0) :: defines_var(INT____00047);\n\
constraint int_lt(s[1], k);\n\
constraint int_lt_reif(0, s[2], BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_lt_reif(0, s[3], BOOL____00005) :: defines_var(BOOL____00005);\n\
constraint int_lt_reif(0, s[4], BOOL____00007) :: defines_var(BOOL____00007);\n\
constraint int_lt_reif(0, s[5], BOOL____00009) :: defines_var(BOOL____00009);\n\
constraint int_lt_reif(0, s[6], BOOL____00011) :: defines_var(BOOL____00011);\n\
constraint int_lt_reif(0, s[7], BOOL____00013) :: defines_var(BOOL____00013);\n\
constraint int_lt_reif(0, s[8], BOOL____00015) :: defines_var(BOOL____00015);\n\
constraint int_lt_reif(0, s[9], BOOL____00017) :: defines_var(BOOL____00017);\n\
constraint int_lt_reif(0, s[10], BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint int_lt_reif(0, s[11], BOOL____00021) :: defines_var(BOOL____00021);\n\
constraint int_lt_reif(s[2], s[1], BOOL____00004) :: defines_var(BOOL____00004);\n\
constraint int_lt_reif(s[3], s[2], BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_lt_reif(s[4], s[3], BOOL____00008) :: defines_var(BOOL____00008);\n\
constraint int_lt_reif(s[5], s[4], BOOL____00010) :: defines_var(BOOL____00010);\n\
constraint int_lt_reif(s[6], s[5], BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint int_lt_reif(s[7], s[6], BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint int_lt_reif(s[8], s[7], BOOL____00016) :: defines_var(BOOL____00016);\n\
constraint int_lt_reif(s[9], s[8], BOOL____00018) :: defines_var(BOOL____00018);\n\
constraint int_lt_reif(s[10], s[9], BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_lt_reif(s[11], s[10], BOOL____00022) :: defines_var(BOOL____00022);\n\
solve maximize j;\n\
", "k = 10;\n\
s = array1d(0..10, [7, 5, 4, 3, 1, 0, 0, 0, 0, 0, 0]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
