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
        (void) new FlatZincTest("blocksworld::1",
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
array [1..3] of int: final_loc = [2, 3, 0];\n\
array [1..3] of int: initial_loc = [2, 0, 1];\n")+
"var bool: BOOL____00001 :: is_defined_var :: var_is_introduced;\n\
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
var bool: BOOL____00025 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00026 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00027 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00028 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00029 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00030 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00031 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00032 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00033 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00034 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00035 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00036 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00037 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00038 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00039 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00040 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00041 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00042 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00043 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00044 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00045 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00046 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00047 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00048 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00049 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00050 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00051 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00052 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00053 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00054 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00055 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00056 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00057 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00058 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00059 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00060 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00061 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00062 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00063 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00064 :: is_defined_var :: var_is_introduced;\n\
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
var bool: BOOL____00090 :: is_defined_var :: var_is_introduced;\n"+
"array [1..12] of var -3..3: on :: output_array([1..4, 1..3]);\n\
constraint all_different_int([on[1], on[2], on[3]]);\n\
constraint all_different_int([on[4], on[5], on[6]]);\n\
constraint all_different_int([on[7], on[8], on[9]]);\n\
constraint all_different_int([on[10], on[11], on[12]]);\n\
constraint array_bool_and([BOOL____00007, BOOL____00006, BOOL____00001], BOOL____00008) :: defines_var(BOOL____00008);\n\
constraint array_bool_and([BOOL____00017, BOOL____00011, BOOL____00016], BOOL____00018) :: defines_var(BOOL____00018);\n\
constraint array_bool_and([BOOL____00021, BOOL____00027, BOOL____00026], BOOL____00028) :: defines_var(BOOL____00028);\n\
constraint array_bool_and([BOOL____00037, BOOL____00036, BOOL____00031], BOOL____00038) :: defines_var(BOOL____00038);\n\
constraint array_bool_and([BOOL____00047, BOOL____00041, BOOL____00046], BOOL____00048) :: defines_var(BOOL____00048);\n\
constraint array_bool_and([BOOL____00051, BOOL____00057, BOOL____00056], BOOL____00058) :: defines_var(BOOL____00058);\n\
constraint array_bool_and([BOOL____00067, BOOL____00066, BOOL____00061], BOOL____00068) :: defines_var(BOOL____00068);\n\
constraint array_bool_and([BOOL____00077, BOOL____00071, BOOL____00076], BOOL____00078) :: defines_var(BOOL____00078);\n\
constraint array_bool_and([BOOL____00081, BOOL____00087, BOOL____00086], BOOL____00088) :: defines_var(BOOL____00088);\n\
constraint array_bool_and([BOOL____00009, BOOL____00004, BOOL____00003, BOOL____00002], BOOL____00010) :: defines_var(BOOL____00010);\n\
constraint array_bool_and([BOOL____00019, BOOL____00014, BOOL____00013, BOOL____00012], BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint array_bool_and([BOOL____00029, BOOL____00024, BOOL____00023, BOOL____00022], BOOL____00030) :: defines_var(BOOL____00030);\n\
constraint array_bool_and([BOOL____00039, BOOL____00034, BOOL____00033, BOOL____00032], BOOL____00040) :: defines_var(BOOL____00040);\n\
constraint array_bool_and([BOOL____00049, BOOL____00044, BOOL____00043, BOOL____00042], BOOL____00050) :: defines_var(BOOL____00050);\n\
constraint array_bool_and([BOOL____00059, BOOL____00054, BOOL____00053, BOOL____00052], BOOL____00060) :: defines_var(BOOL____00060);\n\
constraint array_bool_and([BOOL____00069, BOOL____00064, BOOL____00063, BOOL____00062], BOOL____00070) :: defines_var(BOOL____00070);\n\
constraint array_bool_and([BOOL____00079, BOOL____00074, BOOL____00073, BOOL____00072], BOOL____00080) :: defines_var(BOOL____00080);\n\
constraint array_bool_and([BOOL____00089, BOOL____00084, BOOL____00083, BOOL____00082], BOOL____00090) :: defines_var(BOOL____00090);\n\
constraint array_bool_or([BOOL____00008, BOOL____00005], BOOL____00009) :: defines_var(BOOL____00009);\n\
constraint array_bool_or([BOOL____00018, BOOL____00015], BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint array_bool_or([BOOL____00028, BOOL____00025], BOOL____00029) :: defines_var(BOOL____00029);\n\
constraint array_bool_or([BOOL____00038, BOOL____00035], BOOL____00039) :: defines_var(BOOL____00039);\n\
constraint array_bool_or([BOOL____00048, BOOL____00045], BOOL____00049) :: defines_var(BOOL____00049);\n\
constraint array_bool_or([BOOL____00058, BOOL____00055], BOOL____00059) :: defines_var(BOOL____00059);\n\
constraint array_bool_or([BOOL____00068, BOOL____00065], BOOL____00069) :: defines_var(BOOL____00069);\n\
constraint array_bool_or([BOOL____00078, BOOL____00075], BOOL____00079) :: defines_var(BOOL____00079);\n\
constraint array_bool_or([BOOL____00088, BOOL____00085], BOOL____00089) :: defines_var(BOOL____00089);\n\
constraint bool_le(BOOL____00001, BOOL____00010);\n\
constraint bool_le(BOOL____00011, BOOL____00020);\n\
constraint bool_le(BOOL____00021, BOOL____00030);\n\
constraint bool_le(BOOL____00031, BOOL____00040);\n\
constraint bool_le(BOOL____00041, BOOL____00050);\n\
constraint bool_le(BOOL____00051, BOOL____00060);\n\
constraint bool_le(BOOL____00061, BOOL____00070);\n\
constraint bool_le(BOOL____00071, BOOL____00080);\n\
constraint bool_le(BOOL____00081, BOOL____00090);\n\
constraint int_eq(on[1], 2);\n\
constraint int_eq(on[2], -2);\n\
constraint int_eq(on[3], 1);\n\
constraint int_eq(on[10], 2);\n\
constraint int_eq(on[11], 3);\n\
constraint int_eq(on[12], -3);\n"+
"constraint int_lt_reif(on[4], 0, BOOL____00005) :: defines_var(BOOL____00005);\n\
constraint int_lt_reif(on[5], 0, BOOL____00015) :: defines_var(BOOL____00015);\n\
constraint int_lt_reif(on[6], 0, BOOL____00025) :: defines_var(BOOL____00025);\n\
constraint int_lt_reif(on[7], 0, BOOL____00035) :: defines_var(BOOL____00035);\n\
constraint int_lt_reif(on[8], 0, BOOL____00045) :: defines_var(BOOL____00045);\n\
constraint int_lt_reif(on[9], 0, BOOL____00055) :: defines_var(BOOL____00055);\n\
constraint int_lt_reif(on[10], 0, BOOL____00065) :: defines_var(BOOL____00065);\n\
constraint int_lt_reif(on[11], 0, BOOL____00075) :: defines_var(BOOL____00075);\n\
constraint int_lt_reif(on[12], 0, BOOL____00085) :: defines_var(BOOL____00085);\n\
constraint int_ne_reif(on[1], 1, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_ne_reif(on[1], 2, BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint int_ne_reif(on[1], 3, BOOL____00022) :: defines_var(BOOL____00022);\n\
constraint int_ne_reif(on[1], on[4], BOOL____00001) :: defines_var(BOOL____00001);\n\
constraint int_ne_reif(on[1], on[5], BOOL____00016) :: defines_var(BOOL____00016);\n\
constraint int_ne_reif(on[1], on[6], BOOL____00026) :: defines_var(BOOL____00026);\n\
constraint int_ne_reif(on[2], 1, BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_ne_reif(on[2], 2, BOOL____00013) :: defines_var(BOOL____00013);\n\
constraint int_ne_reif(on[2], 3, BOOL____00023) :: defines_var(BOOL____00023);\n\
constraint int_ne_reif(on[2], on[4], BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_ne_reif(on[2], on[5], BOOL____00011) :: defines_var(BOOL____00011);\n\
constraint int_ne_reif(on[2], on[6], BOOL____00027) :: defines_var(BOOL____00027);\n\
constraint int_ne_reif(on[3], 1, BOOL____00004) :: defines_var(BOOL____00004);\n\
constraint int_ne_reif(on[3], 2, BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint int_ne_reif(on[3], 3, BOOL____00024) :: defines_var(BOOL____00024);\n\
constraint int_ne_reif(on[3], on[4], BOOL____00007) :: defines_var(BOOL____00007);\n\
constraint int_ne_reif(on[3], on[5], BOOL____00017) :: defines_var(BOOL____00017);\n\
constraint int_ne_reif(on[3], on[6], BOOL____00021) :: defines_var(BOOL____00021);\n\
constraint int_ne_reif(on[4], 1, BOOL____00032) :: defines_var(BOOL____00032);\n\
constraint int_ne_reif(on[4], 2, BOOL____00042) :: defines_var(BOOL____00042);\n\
constraint int_ne_reif(on[4], 3, BOOL____00052) :: defines_var(BOOL____00052);\n\
constraint int_ne_reif(on[4], on[7], BOOL____00031) :: defines_var(BOOL____00031);\n\
constraint int_ne_reif(on[4], on[8], BOOL____00046) :: defines_var(BOOL____00046);\n\
constraint int_ne_reif(on[4], on[9], BOOL____00056) :: defines_var(BOOL____00056);\n\
constraint int_ne_reif(on[5], 1, BOOL____00033) :: defines_var(BOOL____00033);\n\
constraint int_ne_reif(on[5], 2, BOOL____00043) :: defines_var(BOOL____00043);\n\
constraint int_ne_reif(on[5], 3, BOOL____00053) :: defines_var(BOOL____00053);\n\
constraint int_ne_reif(on[5], on[7], BOOL____00036) :: defines_var(BOOL____00036);\n\
constraint int_ne_reif(on[5], on[8], BOOL____00041) :: defines_var(BOOL____00041);\n\
constraint int_ne_reif(on[5], on[9], BOOL____00057) :: defines_var(BOOL____00057);\n\
constraint int_ne_reif(on[6], 1, BOOL____00034) :: defines_var(BOOL____00034);\n\
constraint int_ne_reif(on[6], 2, BOOL____00044) :: defines_var(BOOL____00044);\n\
constraint int_ne_reif(on[6], 3, BOOL____00054) :: defines_var(BOOL____00054);\n\
constraint int_ne_reif(on[6], on[7], BOOL____00037) :: defines_var(BOOL____00037);\n\
constraint int_ne_reif(on[6], on[8], BOOL____00047) :: defines_var(BOOL____00047);\n\
constraint int_ne_reif(on[6], on[9], BOOL____00051) :: defines_var(BOOL____00051);\n\
constraint int_ne_reif(on[7], 1, BOOL____00062) :: defines_var(BOOL____00062);\n\
constraint int_ne_reif(on[7], 2, BOOL____00072) :: defines_var(BOOL____00072);\n\
constraint int_ne_reif(on[7], 3, BOOL____00082) :: defines_var(BOOL____00082);\n\
constraint int_ne_reif(on[7], on[10], BOOL____00061) :: defines_var(BOOL____00061);\n\
constraint int_ne_reif(on[7], on[11], BOOL____00076) :: defines_var(BOOL____00076);\n\
constraint int_ne_reif(on[7], on[12], BOOL____00086) :: defines_var(BOOL____00086);\n\
constraint int_ne_reif(on[8], 1, BOOL____00063) :: defines_var(BOOL____00063);\n\
constraint int_ne_reif(on[8], 2, BOOL____00073) :: defines_var(BOOL____00073);\n\
constraint int_ne_reif(on[8], 3, BOOL____00083) :: defines_var(BOOL____00083);\n\
constraint int_ne_reif(on[8], on[10], BOOL____00066) :: defines_var(BOOL____00066);\n\
constraint int_ne_reif(on[8], on[11], BOOL____00071) :: defines_var(BOOL____00071);\n\
constraint int_ne_reif(on[8], on[12], BOOL____00087) :: defines_var(BOOL____00087);\n\
constraint int_ne_reif(on[9], 1, BOOL____00064) :: defines_var(BOOL____00064);\n\
constraint int_ne_reif(on[9], 2, BOOL____00074) :: defines_var(BOOL____00074);\n\
constraint int_ne_reif(on[9], 3, BOOL____00084) :: defines_var(BOOL____00084);\n\
constraint int_ne_reif(on[9], on[10], BOOL____00067) :: defines_var(BOOL____00067);\n\
constraint int_ne_reif(on[9], on[11], BOOL____00077) :: defines_var(BOOL____00077);\n\
constraint int_ne_reif(on[9], on[12], BOOL____00081) :: defines_var(BOOL____00081);\n\
constraint set_in(on[1], {-1, 2, 3});\n\
constraint set_in(on[2], {-2, 1, 3});\n\
constraint set_in(on[3], {-3, 1, 2});\n\
constraint set_in(on[4], {-1, 2, 3});\n\
constraint set_in(on[5], {-2, 1, 3});\n\
constraint set_in(on[6], {-3, 1, 2});\n\
constraint set_in(on[7], {-1, 2, 3});\n\
constraint set_in(on[8], {-2, 1, 3});\n\
constraint set_in(on[9], {-3, 1, 2});\n\
constraint set_in(on[10], {-1, 2, 3});\n\
constraint set_in(on[11], {-2, 1, 3});\n\
constraint set_in(on[12], {-3, 1, 2});\n\
solve  :: int_search([on[1], on[2], on[3], on[4], on[5], on[6], on[7], on[8], on[9], on[10], on[11], on[12]], first_fail, indomain_split, complete) satisfy;\n\
", "on = array2d(1..4, 1..3, [2, -2, 1, 2, -2, -3, -1, -2, -3, 2, 3, -3]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
