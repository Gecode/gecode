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
        (void) new FlatZincTest("oss",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..9] of int: duration = [661, 6, 333, 168, 489, 343, 171, 505, 324];\n\
var bool: BOOL____00001 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00002 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00003 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00004 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00005 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00006 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00010 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00011 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00012 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00013 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00014 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00015 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00019 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00020 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00021 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00022 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00023 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00024 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00028 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00029 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00030 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00031 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00032 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00033 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00037 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00038 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00039 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00040 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00041 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00042 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00046 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00047 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00048 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00049 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00050 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00051 :: is_defined_var :: var_is_introduced;\n\
var 661..1509: makespan :: output_var;\n\
array [1..9] of var 0..1509: start :: output_array([1..3, 1..3]);\n\
constraint array_bool_or([BOOL____00001, BOOL____00002], true);\n\
constraint array_bool_or([BOOL____00003, BOOL____00004], true);\n\
constraint array_bool_or([BOOL____00005, BOOL____00006], true);\n\
constraint array_bool_or([BOOL____00010, BOOL____00011], true);\n\
constraint array_bool_or([BOOL____00012, BOOL____00013], true);\n\
constraint array_bool_or([BOOL____00014, BOOL____00015], true);\n\
constraint array_bool_or([BOOL____00019, BOOL____00020], true);\n\
constraint array_bool_or([BOOL____00021, BOOL____00022], true);\n\
constraint array_bool_or([BOOL____00023, BOOL____00024], true);\n\
constraint array_bool_or([BOOL____00028, BOOL____00029], true);\n\
constraint array_bool_or([BOOL____00030, BOOL____00031], true);\n\
constraint array_bool_or([BOOL____00032, BOOL____00033], true);\n\
constraint array_bool_or([BOOL____00037, BOOL____00038], true);\n\
constraint array_bool_or([BOOL____00039, BOOL____00040], true);\n\
constraint array_bool_or([BOOL____00041, BOOL____00042], true);\n\
constraint array_bool_or([BOOL____00046, BOOL____00047], true);\n\
constraint array_bool_or([BOOL____00048, BOOL____00049], true);\n\
constraint array_bool_or([BOOL____00050, BOOL____00051], true);\n\
constraint int_lin_le([-1, 1], [makespan, start[1]], -661);\n\
constraint int_lin_le([-1, 1], [makespan, start[2]], -6);\n\
constraint int_lin_le([-1, 1], [makespan, start[3]], -333);\n\
constraint int_lin_le([-1, 1], [makespan, start[4]], -168);\n\
constraint int_lin_le([-1, 1], [makespan, start[5]], -489);\n\
constraint int_lin_le([-1, 1], [makespan, start[6]], -343);\n\
constraint int_lin_le([-1, 1], [makespan, start[7]], -171);\n\
constraint int_lin_le([-1, 1], [makespan, start[8]], -505);\n\
constraint int_lin_le([-1, 1], [makespan, start[9]], -324);\n\
constraint int_lin_le_reif([-1, 1], [start[1], start[2]], -6, BOOL____00029) :: defines_var(BOOL____00029);\n\
constraint int_lin_le_reif([-1, 1], [start[1], start[3]], -333, BOOL____00031) :: defines_var(BOOL____00031);\n\
constraint int_lin_le_reif([-1, 1], [start[1], start[4]], -168, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_lin_le_reif([-1, 1], [start[1], start[7]], -171, BOOL____00004) :: defines_var(BOOL____00004);\n\
constraint int_lin_le_reif([-1, 1], [start[2], start[3]], -333, BOOL____00033) :: defines_var(BOOL____00033);\n\
constraint int_lin_le_reif([-1, 1], [start[2], start[5]], -489, BOOL____00011) :: defines_var(BOOL____00011);\n\
constraint int_lin_le_reif([-1, 1], [start[2], start[8]], -505, BOOL____00013) :: defines_var(BOOL____00013);\n\
constraint int_lin_le_reif([-1, 1], [start[3], start[6]], -343, BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_lin_le_reif([-1, 1], [start[3], start[9]], -324, BOOL____00022) :: defines_var(BOOL____00022);\n\
constraint int_lin_le_reif([-1, 1], [start[4], start[5]], -489, BOOL____00038) :: defines_var(BOOL____00038);\n\
constraint int_lin_le_reif([-1, 1], [start[4], start[6]], -343, BOOL____00040) :: defines_var(BOOL____00040);\n\
constraint int_lin_le_reif([-1, 1], [start[4], start[7]], -171, BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_lin_le_reif([-1, 1], [start[5], start[6]], -343, BOOL____00042) :: defines_var(BOOL____00042);\n\
constraint int_lin_le_reif([-1, 1], [start[5], start[8]], -505, BOOL____00015) :: defines_var(BOOL____00015);\n\
constraint int_lin_le_reif([-1, 1], [start[6], start[9]], -324, BOOL____00024) :: defines_var(BOOL____00024);\n\
constraint int_lin_le_reif([-1, 1], [start[7], start[8]], -505, BOOL____00047) :: defines_var(BOOL____00047);\n\
constraint int_lin_le_reif([-1, 1], [start[7], start[9]], -324, BOOL____00049) :: defines_var(BOOL____00049);\n\
constraint int_lin_le_reif([-1, 1], [start[8], start[9]], -324, BOOL____00051) :: defines_var(BOOL____00051);\n\
constraint int_lin_le_reif([1, -1], [start[1], start[2]], -661, BOOL____00028) :: defines_var(BOOL____00028);\n\
constraint int_lin_le_reif([1, -1], [start[1], start[3]], -661, BOOL____00030) :: defines_var(BOOL____00030);\n\
constraint int_lin_le_reif([1, -1], [start[1], start[4]], -661, BOOL____00001) :: defines_var(BOOL____00001);\n\
constraint int_lin_le_reif([1, -1], [start[1], start[7]], -661, BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_lin_le_reif([1, -1], [start[2], start[3]], -6, BOOL____00032) :: defines_var(BOOL____00032);\n\
constraint int_lin_le_reif([1, -1], [start[2], start[5]], -6, BOOL____00010) :: defines_var(BOOL____00010);\n\
constraint int_lin_le_reif([1, -1], [start[2], start[8]], -6, BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint int_lin_le_reif([1, -1], [start[3], start[6]], -333, BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint int_lin_le_reif([1, -1], [start[3], start[9]], -333, BOOL____00021) :: defines_var(BOOL____00021);\n\
constraint int_lin_le_reif([1, -1], [start[4], start[5]], -168, BOOL____00037) :: defines_var(BOOL____00037);\n\
constraint int_lin_le_reif([1, -1], [start[4], start[6]], -168, BOOL____00039) :: defines_var(BOOL____00039);\n\
constraint int_lin_le_reif([1, -1], [start[4], start[7]], -168, BOOL____00005) :: defines_var(BOOL____00005);\n\
constraint int_lin_le_reif([1, -1], [start[5], start[6]], -489, BOOL____00041) :: defines_var(BOOL____00041);\n\
constraint int_lin_le_reif([1, -1], [start[5], start[8]], -489, BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint int_lin_le_reif([1, -1], [start[6], start[9]], -343, BOOL____00023) :: defines_var(BOOL____00023);\n\
constraint int_lin_le_reif([1, -1], [start[7], start[8]], -171, BOOL____00046) :: defines_var(BOOL____00046);\n\
constraint int_lin_le_reif([1, -1], [start[7], start[9]], -171, BOOL____00048) :: defines_var(BOOL____00048);\n\
constraint int_lin_le_reif([1, -1], [start[8], start[9]], -505, BOOL____00050) :: defines_var(BOOL____00050);\n\
solve minimize makespan;\n\
", "makespan = 1168;\n\
start = array2d(1..3, 1..3, [0, 1162, 829, 1000, 505, 0, 829, 0, 505]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
