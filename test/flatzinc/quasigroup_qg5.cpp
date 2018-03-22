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
        (void) new FlatZincTest("quasigroup",
std::string("predicate all_different_int(array [int] of var int: x);\n\
predicate all_equal_int(array [int] of var int: x);\n\
predicate among(var int: n, array [int] of var int: x, set of int: v);\n\
predicate array_bool_lq(array [int] of var bool: x, array [int] of var bool: y);\n\
predicate array_bool_lt(array [int] of var bool: x, array [int] of var bool: y);\n\
predicate array_int_lq(array [int] of var int: x, array [int] of var int: y);\n\
predicate array_int_lt(array [int] of var int: x, array [int] of var int: y);\n\
predicate array_set_partition(array [int] of var set of int: S, set of int: universe);\n\
predicate at_least_int(int: n, array [int] of var int: x, int: v);\n\
predicate at_most_int(int: n, array [int] of var int: x, int: v);\n\
predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate count(array [int] of var int: x, var int: y, var int: c);\n\
predicate count_reif(array [int] of var int: x, var int: y, var int: c, var bool: b);\n\
predicate cumulatives(array [int] of var int: s, array [int] of var int: d, array [int] of var int: r, var int: b);\n\
predicate decreasing_bool(array [int] of var bool: x);\n\
predicate decreasing_int(array [int] of var int: x);\n\
predicate disjoint(var set of int: s1, var set of int: s2);\n\
predicate gecode_array_set_element_union(var set of int: x, array [int] of var set of int: y, var set of int: z);\n\
predicate gecode_bin_packing_load(array [int] of var int: l, array [int] of var int: bin, array [int] of int: w, int: minIndex);\n\
predicate gecode_circuit(int: offset, array [int] of var int: x);\n\
predicate gecode_int_set_channel(array [int] of var int: x, int: xoff, array [int] of var set of int: y, int: yoff);\n\
predicate gecode_inverse_set(array [int] of var set of int: f, array [int] of var set of int: invf, int: xoff, int: yoff);\n\
predicate gecode_link_set_to_booleans(var set of int: s, array [int] of var bool: b, int: idx);\n\
predicate gecode_member_bool_reif(array [int] of var bool: x, var bool: y, var bool: b);\n\
predicate gecode_member_int_reif(array [int] of var int: x, var int: y, var bool: b);\n\
predicate gecode_nooverlap(array [int] of var int: x, array [int] of var int: w, array [int] of var int: y, array [int] of var int: h);\n\
predicate gecode_precede(array [int] of var int: x, int: s, int: t);\n\
predicate gecode_precede_set(array [int] of var set of int: x, int: s, int: t);\n\
predicate gecode_range(array [int] of var int: x, int: xoff, var set of int: s, var set of int: t);\n\
predicate gecode_set_weights(array [int] of int: csi, array [int] of int: cs, var set of int: x, var int: y);\n\
predicate global_cardinality(array [int] of var int: x, array [int] of int: cover, array [int] of var int: counts);\n\
predicate global_cardinality_closed(array [int] of var int: x, array [int] of int: cover, array [int] of var int: counts);\n\
predicate global_cardinality_low_up(array [int] of var int: x, array [int] of int: cover, array [int] of int: lbound, array [int] of int: ubound);\n\
predicate global_cardinality_low_up_closed(array [int] of var int: x, array [int] of int: cover, array [int] of int: lbound, array [int] of int: ubound);\n\
predicate increasing_bool(array [int] of var bool: x);\n\
predicate increasing_int(array [int] of var int: x);\n\
predicate inverse_offsets(array [int] of var int: f, int: foff, array [int] of var int: invf, int: invfoff);\n\
predicate maximum_int(var int: m, array [int] of var int: x);\n\
predicate member_bool(array [int] of var bool: x, var bool: y);\n\
predicate member_int(array [int] of var int: x, var int: y);\n\
predicate minimum_int(var int: m, array [int] of var int: x);\n\
predicate nvalue(var int: n, array [int] of var int: x);\n\
predicate regular(array [int] of var int: x, int: Q, int: S, array [int, int] of int: d, int: q0, set of int: F);\n\
predicate sort(array [int] of var int: x, array [int] of var int: y);\n\
predicate table_bool(array [int] of var bool: x, array [int, int] of bool: t);\n\
predicate table_int(array [int] of var int: x, array [int, int] of int: t);\n\
var 1..21: INT____00001 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00002 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00003 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00005 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00006 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00007 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00009 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00010 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00011 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00013 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00014 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00015 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00017 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00018 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00019 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00021 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00022 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00023 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00025 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00026 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00027 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00029 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00030 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00031 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00033 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00034 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00035 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00037 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00038 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00039 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00041 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00042 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00043 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00045 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00046 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00047 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00049 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00050 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00051 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00053 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00054 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00055 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00057 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00058 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00059 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00061 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00062 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00063 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00065 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00066 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00067 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00069 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00070 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00071 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00073 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00074 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00075 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00077 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00078 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00079 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00081 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00082 :: is_defined_var :: var_is_introduced;\n\
var 1..21: INT____00083 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00085 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00086 :: is_defined_var :: var_is_introduced;\n\
var 2..22: INT____00087 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00089 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00090 :: is_defined_var :: var_is_introduced;\n\
var 3..23: INT____00091 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00093 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00094 :: is_defined_var :: var_is_introduced;\n\
var 4..24: INT____00095 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00097 :: is_defined_var :: var_is_introduced;\n\
var 1..5: INT____00098 :: is_defined_var :: var_is_introduced;\n\
var 5..25: INT____00099 :: is_defined_var :: var_is_introduced;\n\
array [1..25] of var 1..5: q :: output_array([1..5, 1..5]);\n\
constraint all_different_int([q[1], q[2], q[3], q[4], q[5]]);\n\
constraint all_different_int([q[1], q[6], q[11], q[16], q[21]]);\n\
constraint all_different_int([q[2], q[7], q[12], q[17], q[22]]);\n\
constraint all_different_int([q[3], q[8], q[13], q[18], q[23]]);\n\
constraint all_different_int([q[4], q[9], q[14], q[19], q[24]]);\n\
constraint all_different_int([q[5], q[10], q[15], q[20], q[25]]);\n\
constraint all_different_int([q[6], q[7], q[8], q[9], q[10]]);\n\
constraint all_different_int([q[11], q[12], q[13], q[14], q[15]]);\n\
constraint all_different_int([q[16], q[17], q[18], q[19], q[20]]);\n\
constraint all_different_int([q[21], q[22], q[23], q[24], q[25]]);\n\
constraint array_var_int_element(INT____00001, q, INT____00002) :: defines_var(INT____00002);\n\
constraint array_var_int_element(INT____00003, q, 1);\n\
constraint array_var_int_element(INT____00005, q, INT____00006) :: defines_var(INT____00006);\n\
constraint array_var_int_element(INT____00007, q, 1);\n\
constraint array_var_int_element(INT____00009, q, INT____00010) :: defines_var(INT____00010);\n\
constraint array_var_int_element(INT____00011, q, 1);\n\
constraint array_var_int_element(INT____00013, q, INT____00014) :: defines_var(INT____00014);\n\
constraint array_var_int_element(INT____00015, q, 1);\n\
constraint array_var_int_element(INT____00017, q, INT____00018) :: defines_var(INT____00018);\n\
constraint array_var_int_element(INT____00019, q, 1);\n\
constraint array_var_int_element(INT____00021, q, INT____00022) :: defines_var(INT____00022);\n\
constraint array_var_int_element(INT____00023, q, 2);\n\
constraint array_var_int_element(INT____00025, q, INT____00026) :: defines_var(INT____00026);\n\
constraint array_var_int_element(INT____00027, q, 2);\n\
constraint array_var_int_element(INT____00029, q, INT____00030) :: defines_var(INT____00030);\n\
constraint array_var_int_element(INT____00031, q, 2);\n\
constraint array_var_int_element(INT____00033, q, INT____00034) :: defines_var(INT____00034);\n\
constraint array_var_int_element(INT____00035, q, 2);\n\
constraint array_var_int_element(INT____00037, q, INT____00038) :: defines_var(INT____00038);\n\
constraint array_var_int_element(INT____00039, q, 2);\n\
constraint array_var_int_element(INT____00041, q, INT____00042) :: defines_var(INT____00042);\n\
constraint array_var_int_element(INT____00043, q, 3);\n\
constraint array_var_int_element(INT____00045, q, INT____00046) :: defines_var(INT____00046);\n\
constraint array_var_int_element(INT____00047, q, 3);\n\
constraint array_var_int_element(INT____00049, q, INT____00050) :: defines_var(INT____00050);\n\
constraint array_var_int_element(INT____00051, q, 3);\n\
constraint array_var_int_element(INT____00053, q, INT____00054) :: defines_var(INT____00054);\n\
constraint array_var_int_element(INT____00055, q, 3);\n\
constraint array_var_int_element(INT____00057, q, INT____00058) :: defines_var(INT____00058);\n\
constraint array_var_int_element(INT____00059, q, 3);\n\
constraint array_var_int_element(INT____00061, q, INT____00062) :: defines_var(INT____00062);\n\
constraint array_var_int_element(INT____00063, q, 4);\n\
constraint array_var_int_element(INT____00065, q, INT____00066) :: defines_var(INT____00066);\n\
constraint array_var_int_element(INT____00067, q, 4);\n\
constraint array_var_int_element(INT____00069, q, INT____00070) :: defines_var(INT____00070);\n\
constraint array_var_int_element(INT____00071, q, 4);\n\
constraint array_var_int_element(INT____00073, q, INT____00074) :: defines_var(INT____00074);\n\
constraint array_var_int_element(INT____00075, q, 4);\n\
constraint array_var_int_element(INT____00077, q, INT____00078) :: defines_var(INT____00078);\n\
constraint array_var_int_element(INT____00079, q, 4);\n\
constraint array_var_int_element(INT____00081, q, INT____00082) :: defines_var(INT____00082);\n\
constraint array_var_int_element(INT____00083, q, 5);\n\
constraint array_var_int_element(INT____00085, q, INT____00086) :: defines_var(INT____00086);\n\
constraint array_var_int_element(INT____00087, q, 5);\n\
constraint array_var_int_element(INT____00089, q, INT____00090) :: defines_var(INT____00090);\n\
constraint array_var_int_element(INT____00091, q, 5);\n\
constraint array_var_int_element(INT____00093, q, INT____00094) :: defines_var(INT____00094);\n\
constraint array_var_int_element(INT____00095, q, 5);\n\
constraint array_var_int_element(INT____00097, q, INT____00098) :: defines_var(INT____00098);\n\
constraint array_var_int_element(INT____00099, q, 5);\n\
constraint int_eq(q[1], 1);\n\
constraint int_eq(q[7], 2);\n\
constraint int_eq(q[13], 3);\n\
constraint int_eq(q[19], 4);\n\
constraint int_eq(q[25], 5);\n\
constraint int_lin_eq([-1, 5], [INT____00001, q[1]], 4) :: defines_var(INT____00001) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00003, INT____00002], 4) :: defines_var(INT____00003) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00005, q[6]], 3) :: defines_var(INT____00005) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00007, INT____00006], 3) :: defines_var(INT____00007) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00009, q[11]], 2) :: defines_var(INT____00009) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00011, INT____00010], 2) :: defines_var(INT____00011) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00013, q[16]], 1) :: defines_var(INT____00013) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00015, INT____00014], 1) :: defines_var(INT____00015) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00017, q[21]], 0) :: defines_var(INT____00017) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00019, INT____00018], 0) :: defines_var(INT____00019) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00021, q[2]], 4) :: defines_var(INT____00021) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00023, INT____00022], 4) :: defines_var(INT____00023) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00025, q[7]], 3) :: defines_var(INT____00025) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00027, INT____00026], 3) :: defines_var(INT____00027) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00029, q[12]], 2) :: defines_var(INT____00029) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00031, INT____00030], 2) :: defines_var(INT____00031) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00033, q[17]], 1) :: defines_var(INT____00033) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00035, INT____00034], 1) :: defines_var(INT____00035) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00037, q[22]], 0) :: defines_var(INT____00037) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00039, INT____00038], 0) :: defines_var(INT____00039) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00041, q[3]], 4) :: defines_var(INT____00041) :: domain;\n")+"\
constraint int_lin_eq([-1, 5], [INT____00043, INT____00042], 4) :: defines_var(INT____00043) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00045, q[8]], 3) :: defines_var(INT____00045) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00047, INT____00046], 3) :: defines_var(INT____00047) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00049, q[13]], 2) :: defines_var(INT____00049) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00051, INT____00050], 2) :: defines_var(INT____00051) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00053, q[18]], 1) :: defines_var(INT____00053) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00055, INT____00054], 1) :: defines_var(INT____00055) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00057, q[23]], 0) :: defines_var(INT____00057) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00059, INT____00058], 0) :: defines_var(INT____00059) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00061, q[4]], 4) :: defines_var(INT____00061) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00063, INT____00062], 4) :: defines_var(INT____00063) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00065, q[9]], 3) :: defines_var(INT____00065) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00067, INT____00066], 3) :: defines_var(INT____00067) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00069, q[14]], 2) :: defines_var(INT____00069) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00071, INT____00070], 2) :: defines_var(INT____00071) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00073, q[19]], 1) :: defines_var(INT____00073) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00075, INT____00074], 1) :: defines_var(INT____00075) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00077, q[24]], 0) :: defines_var(INT____00077) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00079, INT____00078], 0) :: defines_var(INT____00079) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00081, q[5]], 4) :: defines_var(INT____00081) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00083, INT____00082], 4) :: defines_var(INT____00083) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00085, q[10]], 3) :: defines_var(INT____00085) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00087, INT____00086], 3) :: defines_var(INT____00087) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00089, q[15]], 2) :: defines_var(INT____00089) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00091, INT____00090], 2) :: defines_var(INT____00091) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00093, q[20]], 1) :: defines_var(INT____00093) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00095, INT____00094], 1) :: defines_var(INT____00095) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00097, q[25]], 0) :: defines_var(INT____00097) :: domain;\n\
constraint int_lin_eq([-1, 5], [INT____00099, INT____00098], 0) :: defines_var(INT____00099) :: domain;\n\
constraint int_lt(q[1], 3);\n\
constraint int_lt(q[6], 4);\n\
constraint int_lt(q[11], 5);\n\
solve  :: int_search(q, input_order, indomain, complete) satisfy;\n\
", "q = array2d(1..5, 1..5, [1, 4, 5, 2, 3, 3, 2, 1, 5, 4, 4, 5, 3, 1, 2, 5, 3, 2, 4, 1, 2, 1, 4, 3, 5]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
