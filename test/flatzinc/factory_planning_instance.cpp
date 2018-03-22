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
        (void) new FlatZincTest("factory_planning",
std::string("predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..15] of int: can_add_attr = [0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0];\n\
array [1..25] of int: connected = [1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1];\n\
array [1..6] of int: prod_attr_goal = [1, 1, 0, 1, 0, 1];\n\
array [1..2] of int: prod_start_mach = [1, 1];\n\
var bool: BOOL____00065 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00066 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00067 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00068 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00069 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00070 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00071 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00072 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00073 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00074 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00075 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00076 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00077 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00078 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00079 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00080 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00081 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00082 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00083 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00084 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00085 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00086 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00087 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00088 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00089 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00090 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00091 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00092 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00093 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00094 :: is_defined_var :: var_is_introduced;\n\
var 1..13: INT____00001 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00002 :: is_defined_var :: var_is_introduced;\n\
var 2..14: INT____00003 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00004 :: is_defined_var :: var_is_introduced;\n\
var 3..15: INT____00005 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00006 :: is_defined_var :: var_is_introduced;\n\
var 1..13: INT____00007 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00008 :: is_defined_var :: var_is_introduced;\n\
var 2..14: INT____00009 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00010 :: is_defined_var :: var_is_introduced;\n\
var 3..15: INT____00011 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00012 :: is_defined_var :: var_is_introduced;\n\
var 1..13: INT____00013 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00014 :: is_defined_var :: var_is_introduced;\n\
var 2..14: INT____00015 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00016 :: is_defined_var :: var_is_introduced;\n\
var 3..15: INT____00017 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00018 :: is_defined_var :: var_is_introduced;\n\
var 1..13: INT____00019 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00020 :: is_defined_var :: var_is_introduced;\n\
var 2..14: INT____00021 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00022 :: is_defined_var :: var_is_introduced;\n\
var 3..15: INT____00023 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00024 :: is_defined_var :: var_is_introduced;\n\
var 1..13: INT____00025 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00026 :: is_defined_var :: var_is_introduced;\n\
var 2..14: INT____00027 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00028 :: is_defined_var :: var_is_introduced;\n\
var 3..15: INT____00029 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00030 :: is_defined_var :: var_is_introduced;\n\
var 1..13: INT____00031 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00032 :: is_defined_var :: var_is_introduced;\n\
var 2..14: INT____00033 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00034 :: is_defined_var :: var_is_introduced;\n\
var 3..15: INT____00035 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00036 :: is_defined_var :: var_is_introduced;\n\
var 1..13: INT____00037 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00038 :: is_defined_var :: var_is_introduced;\n\
var 2..14: INT____00039 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00040 :: is_defined_var :: var_is_introduced;\n\
var 3..15: INT____00041 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00042 :: is_defined_var :: var_is_introduced;\n\
var 1..13: INT____00043 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00044 :: is_defined_var :: var_is_introduced;\n\
var 2..14: INT____00045 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00046 :: is_defined_var :: var_is_introduced;\n\
var 3..15: INT____00047 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00048 :: is_defined_var :: var_is_introduced;\n\
var 1..25: INT____00049 :: is_defined_var :: var_is_introduced;\n\
var 1..25: INT____00051 :: is_defined_var :: var_is_introduced;\n\
var 1..25: INT____00053 :: is_defined_var :: var_is_introduced;\n\
var 1..25: INT____00055 :: is_defined_var :: var_is_introduced;\n\
var 1..25: INT____00057 :: is_defined_var :: var_is_introduced;\n\
var 1..25: INT____00059 :: is_defined_var :: var_is_introduced;\n\
var 1..25: INT____00061 :: is_defined_var :: var_is_introduced;\n\
var 1..25: INT____00063 :: is_defined_var :: var_is_introduced;\n\
var 5..5: last_step = 5;\n\
array [1..30] of var 0..1: step_prod_attr :: output_array([1..5, 1..2, 1..3]);\n\
array [1..10] of var 1..5: step_prod_mach :: output_array([1..5, 1..2]);\n\
constraint array_int_element(INT____00001, can_add_attr, INT____00002) :: defines_var(INT____00002);\n\
constraint array_int_element(INT____00003, can_add_attr, INT____00004) :: defines_var(INT____00004);\n\
constraint array_int_element(INT____00005, can_add_attr, INT____00006) :: defines_var(INT____00006);\n\
constraint array_int_element(INT____00007, can_add_attr, INT____00008) :: defines_var(INT____00008);\n\
constraint array_int_element(INT____00009, can_add_attr, INT____00010) :: defines_var(INT____00010);\n\
constraint array_int_element(INT____00011, can_add_attr, INT____00012) :: defines_var(INT____00012);\n\
constraint array_int_element(INT____00013, can_add_attr, INT____00014) :: defines_var(INT____00014);\n\
constraint array_int_element(INT____00015, can_add_attr, INT____00016) :: defines_var(INT____00016);\n\
constraint array_int_element(INT____00017, can_add_attr, INT____00018) :: defines_var(INT____00018);\n\
constraint array_int_element(INT____00019, can_add_attr, INT____00020) :: defines_var(INT____00020);\n\
constraint array_int_element(INT____00021, can_add_attr, INT____00022) :: defines_var(INT____00022);\n\
constraint array_int_element(INT____00023, can_add_attr, INT____00024) :: defines_var(INT____00024);\n\
constraint array_int_element(INT____00025, can_add_attr, INT____00026) :: defines_var(INT____00026);\n\
constraint array_int_element(INT____00027, can_add_attr, INT____00028) :: defines_var(INT____00028);\n\
constraint array_int_element(INT____00029, can_add_attr, INT____00030) :: defines_var(INT____00030);\n\
constraint array_int_element(INT____00031, can_add_attr, INT____00032) :: defines_var(INT____00032);\n\
constraint array_int_element(INT____00033, can_add_attr, INT____00034) :: defines_var(INT____00034);\n\
constraint array_int_element(INT____00035, can_add_attr, INT____00036) :: defines_var(INT____00036);\n\
constraint array_int_element(INT____00037, can_add_attr, INT____00038) :: defines_var(INT____00038);\n\
constraint array_int_element(INT____00039, can_add_attr, INT____00040) :: defines_var(INT____00040);\n\
constraint array_int_element(INT____00041, can_add_attr, INT____00042) :: defines_var(INT____00042);\n\
constraint array_int_element(INT____00043, can_add_attr, INT____00044) :: defines_var(INT____00044);\n\
constraint array_int_element(INT____00045, can_add_attr, INT____00046) :: defines_var(INT____00046);\n\
constraint array_int_element(INT____00047, can_add_attr, INT____00048) :: defines_var(INT____00048);\n\
constraint array_int_element(INT____00049, connected, 1);\n\
constraint array_int_element(INT____00051, connected, 1);\n\
constraint array_int_element(INT____00053, connected, 1);\n\
constraint array_int_element(INT____00055, connected, 1);\n\
constraint array_int_element(INT____00057, connected, 1);\n\
constraint array_int_element(INT____00059, connected, 1);\n\
constraint array_int_element(INT____00061, connected, 1);\n\
constraint array_int_element(INT____00063, connected, 1);\n\
constraint bool_le(BOOL____00065, BOOL____00066);\n\
constraint bool_le(BOOL____00067, BOOL____00068);\n\
constraint bool_le(BOOL____00069, BOOL____00070);\n\
constraint bool_le(BOOL____00071, BOOL____00072);\n\
constraint bool_le(BOOL____00073, BOOL____00074);\n\
constraint bool_le(BOOL____00075, BOOL____00076);\n\
constraint bool_le(BOOL____00077, BOOL____00078);\n\
constraint bool_le(BOOL____00079, BOOL____00080);\n\
constraint bool_le(BOOL____00081, BOOL____00082);\n\
constraint bool_le(BOOL____00083, BOOL____00084);\n\
constraint bool_le(BOOL____00085, BOOL____00086);\n\
constraint bool_le(BOOL____00087, BOOL____00088);\n\
constraint bool_le(BOOL____00089, BOOL____00090);\n\
constraint bool_le(BOOL____00091, BOOL____00092);\n\
constraint bool_le(BOOL____00093, BOOL____00094);\n\
constraint int_eq(step_prod_attr[1], 0);\n\
constraint int_eq(step_prod_attr[2], 0);\n\
constraint int_eq(step_prod_attr[3], 0);\n\
constraint int_eq(step_prod_attr[4], 0);\n\
constraint int_eq(step_prod_attr[5], 0);\n\
constraint int_eq(step_prod_attr[6], 0);\n\
constraint int_eq(step_prod_attr[25], 1);\n\
constraint int_eq(step_prod_attr[26], 1);\n\
constraint int_eq(step_prod_attr[27], 0);\n\
constraint int_eq(step_prod_attr[28], 1);\n\
constraint int_eq(step_prod_attr[29], 0);\n\
constraint int_eq(step_prod_attr[30], 1);\n\
constraint int_eq(step_prod_mach[1], 1);\n\
constraint int_eq(step_prod_mach[2], 1);\n\
constraint int_eq(step_prod_mach[9], 5);\n\
constraint int_eq(step_prod_mach[10], 5);\n\
constraint int_eq_reif(step_prod_mach[1], 2, BOOL____00065) :: defines_var(BOOL____00065);\n\
constraint int_eq_reif(step_prod_mach[1], 3, BOOL____00067) :: defines_var(BOOL____00067);\n\
constraint int_eq_reif(step_prod_mach[1], 4, BOOL____00069) :: defines_var(BOOL____00069);\n\
constraint int_eq_reif(step_prod_mach[3], 2, BOOL____00071) :: defines_var(BOOL____00071);\n\
constraint int_eq_reif(step_prod_mach[3], 3, BOOL____00073) :: defines_var(BOOL____00073);\n\
constraint int_eq_reif(step_prod_mach[3], 4, BOOL____00075) :: defines_var(BOOL____00075);\n\
constraint int_eq_reif(step_prod_mach[5], 2, BOOL____00077) :: defines_var(BOOL____00077);\n\
constraint int_eq_reif(step_prod_mach[5], 3, BOOL____00079) :: defines_var(BOOL____00079);\n\
constraint int_eq_reif(step_prod_mach[5], 4, BOOL____00081) :: defines_var(BOOL____00081);\n\
constraint int_eq_reif(step_prod_mach[7], 2, BOOL____00083) :: defines_var(BOOL____00083);\n\
constraint int_eq_reif(step_prod_mach[7], 3, BOOL____00085) :: defines_var(BOOL____00085);\n\
constraint int_eq_reif(step_prod_mach[7], 4, BOOL____00087) :: defines_var(BOOL____00087);\n\
constraint int_eq_reif(step_prod_mach[9], 2, BOOL____00089) :: defines_var(BOOL____00089);\n\
constraint int_eq_reif(step_prod_mach[9], 3, BOOL____00091) :: defines_var(BOOL____00091);\n\
constraint int_eq_reif(step_prod_mach[9], 4, BOOL____00093) :: defines_var(BOOL____00093);\n\
constraint int_le(step_prod_attr[1], step_prod_attr[7]);\n\
constraint int_le(step_prod_attr[2], step_prod_attr[8]);\n\
constraint int_le(step_prod_attr[3], step_prod_attr[9]);\n\
constraint int_le(step_prod_attr[4], step_prod_attr[10]);\n\
constraint int_le(step_prod_attr[5], step_prod_attr[11]);\n\
constraint int_le(step_prod_attr[6], step_prod_attr[12]);\n\
constraint int_le(step_prod_attr[7], step_prod_attr[13]);\n\
constraint int_le(step_prod_attr[8], step_prod_attr[14]);\n\
constraint int_le(step_prod_attr[9], step_prod_attr[15]);\n\
constraint int_le(step_prod_attr[10], step_prod_attr[16]);\n\
constraint int_le(step_prod_attr[11], step_prod_attr[17]);\n\
constraint int_le(step_prod_attr[12], step_prod_attr[18]);\n\
constraint int_le(step_prod_attr[13], step_prod_attr[19]);\n\
constraint int_le(step_prod_attr[14], step_prod_attr[20]);\n\
constraint int_le(step_prod_attr[15], step_prod_attr[21]);\n\
constraint int_le(step_prod_attr[16], step_prod_attr[22]);\n\
constraint int_le(step_prod_attr[17], step_prod_attr[23]);\n\
constraint int_le(step_prod_attr[18], step_prod_attr[24]);\n\
constraint int_le(step_prod_attr[19], step_prod_attr[25]);\n\
constraint int_le(step_prod_attr[20], step_prod_attr[26]);\n\
constraint int_le(step_prod_attr[21], step_prod_attr[27]);\n\
constraint int_le(step_prod_attr[22], step_prod_attr[28]);\n\
constraint int_le(step_prod_attr[23], step_prod_attr[29]);\n\
constraint int_le(step_prod_attr[24], step_prod_attr[30]);\n\
constraint int_lin_eq([-1, 3], [INT____00001, step_prod_mach[1]], 2) :: defines_var(INT____00001) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00003, step_prod_mach[1]], 1) :: defines_var(INT____00003) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00005, step_prod_mach[1]], 0) :: defines_var(INT____00005) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00007, step_prod_mach[2]], 2) :: defines_var(INT____00007) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00009, step_prod_mach[2]], 1) :: defines_var(INT____00009) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00011, step_prod_mach[2]], 0) :: defines_var(INT____00011) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00013, step_prod_mach[3]], 2) :: defines_var(INT____00013) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00015, step_prod_mach[3]], 1) :: defines_var(INT____00015) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00017, step_prod_mach[3]], 0) :: defines_var(INT____00017) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00019, step_prod_mach[4]], 2) :: defines_var(INT____00019) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00021, step_prod_mach[4]], 1) :: defines_var(INT____00021) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00023, step_prod_mach[4]], 0) :: defines_var(INT____00023) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00025, step_prod_mach[5]], 2) :: defines_var(INT____00025) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00027, step_prod_mach[5]], 1) :: defines_var(INT____00027) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00029, step_prod_mach[5]], 0) :: defines_var(INT____00029) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00031, step_prod_mach[6]], 2) :: defines_var(INT____00031) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00033, step_prod_mach[6]], 1) :: defines_var(INT____00033) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00035, step_prod_mach[6]], 0) :: defines_var(INT____00035) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00037, step_prod_mach[7]], 2) :: defines_var(INT____00037) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00039, step_prod_mach[7]], 1) :: defines_var(INT____00039) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00041, step_prod_mach[7]], 0) :: defines_var(INT____00041) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00043, step_prod_mach[8]], 2) :: defines_var(INT____00043) :: domain;\n")+
"constraint int_lin_eq([-1, 3], [INT____00045, step_prod_mach[8]], 1) :: defines_var(INT____00045) :: domain;\n\
constraint int_lin_eq([-1, 3], [INT____00047, step_prod_mach[8]], 0) :: defines_var(INT____00047) :: domain;\n\
constraint int_lin_eq([-1, 5, 1], [INT____00049, step_prod_mach[1], step_prod_mach[3]], 5) :: defines_var(INT____00049) :: domain;\n\
constraint int_lin_eq([-1, 5, 1], [INT____00051, step_prod_mach[2], step_prod_mach[4]], 5) :: defines_var(INT____00051) :: domain;\n\
constraint int_lin_eq([-1, 5, 1], [INT____00053, step_prod_mach[3], step_prod_mach[5]], 5) :: defines_var(INT____00053) :: domain;\n\
constraint int_lin_eq([-1, 5, 1], [INT____00055, step_prod_mach[4], step_prod_mach[6]], 5) :: defines_var(INT____00055) :: domain;\n\
constraint int_lin_eq([-1, 5, 1], [INT____00057, step_prod_mach[5], step_prod_mach[7]], 5) :: defines_var(INT____00057) :: domain;\n\
constraint int_lin_eq([-1, 5, 1], [INT____00059, step_prod_mach[6], step_prod_mach[8]], 5) :: defines_var(INT____00059) :: domain;\n\
constraint int_lin_eq([-1, 5, 1], [INT____00061, step_prod_mach[7], step_prod_mach[9]], 5) :: defines_var(INT____00061) :: domain;\n\
constraint int_lin_eq([-1, 5, 1], [INT____00063, step_prod_mach[8], step_prod_mach[10]], 5) :: defines_var(INT____00063) :: domain;\n\
constraint int_lin_le([-1, -1, 1], [INT____00002, step_prod_attr[1], step_prod_attr[7]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00004, step_prod_attr[2], step_prod_attr[8]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00006, step_prod_attr[3], step_prod_attr[9]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00008, step_prod_attr[4], step_prod_attr[10]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00010, step_prod_attr[5], step_prod_attr[11]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00012, step_prod_attr[6], step_prod_attr[12]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00014, step_prod_attr[7], step_prod_attr[13]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00016, step_prod_attr[8], step_prod_attr[14]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00018, step_prod_attr[9], step_prod_attr[15]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00020, step_prod_attr[10], step_prod_attr[16]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00022, step_prod_attr[11], step_prod_attr[17]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00024, step_prod_attr[12], step_prod_attr[18]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00026, step_prod_attr[13], step_prod_attr[19]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00028, step_prod_attr[14], step_prod_attr[20]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00030, step_prod_attr[15], step_prod_attr[21]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00032, step_prod_attr[16], step_prod_attr[22]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00034, step_prod_attr[17], step_prod_attr[23]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00036, step_prod_attr[18], step_prod_attr[24]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00038, step_prod_attr[19], step_prod_attr[25]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00040, step_prod_attr[20], step_prod_attr[26]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00042, step_prod_attr[21], step_prod_attr[27]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00044, step_prod_attr[22], step_prod_attr[28]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00046, step_prod_attr[23], step_prod_attr[29]], 0);\n\
constraint int_lin_le([-1, -1, 1], [INT____00048, step_prod_attr[24], step_prod_attr[30]], 0);\n\
constraint int_ne_reif(step_prod_mach[2], 2, BOOL____00066) :: defines_var(BOOL____00066);\n\
constraint int_ne_reif(step_prod_mach[2], 3, BOOL____00068) :: defines_var(BOOL____00068);\n\
constraint int_ne_reif(step_prod_mach[2], 4, BOOL____00070) :: defines_var(BOOL____00070);\n\
constraint int_ne_reif(step_prod_mach[4], 2, BOOL____00072) :: defines_var(BOOL____00072);\n\
constraint int_ne_reif(step_prod_mach[4], 3, BOOL____00074) :: defines_var(BOOL____00074);\n\
constraint int_ne_reif(step_prod_mach[4], 4, BOOL____00076) :: defines_var(BOOL____00076);\n\
constraint int_ne_reif(step_prod_mach[6], 2, BOOL____00078) :: defines_var(BOOL____00078);\n\
constraint int_ne_reif(step_prod_mach[6], 3, BOOL____00080) :: defines_var(BOOL____00080);\n\
constraint int_ne_reif(step_prod_mach[6], 4, BOOL____00082) :: defines_var(BOOL____00082);\n\
constraint int_ne_reif(step_prod_mach[8], 2, BOOL____00084) :: defines_var(BOOL____00084);\n\
constraint int_ne_reif(step_prod_mach[8], 3, BOOL____00086) :: defines_var(BOOL____00086);\n\
constraint int_ne_reif(step_prod_mach[8], 4, BOOL____00088) :: defines_var(BOOL____00088);\n\
constraint int_ne_reif(step_prod_mach[10], 2, BOOL____00090) :: defines_var(BOOL____00090);\n\
constraint int_ne_reif(step_prod_mach[10], 3, BOOL____00092) :: defines_var(BOOL____00092);\n\
constraint int_ne_reif(step_prod_mach[10], 4, BOOL____00094) :: defines_var(BOOL____00094);\n\
solve satisfy;\n\
", "step_prod_attr = array3d(1..5, 1..2, 1..3, [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1]);\n\
step_prod_mach = array2d(1..5, 1..2, [1, 1, 1, 4, 2, 1, 3, 2, 5, 5]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
