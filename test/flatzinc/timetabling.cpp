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
        (void) new FlatZincTest("timetabling",
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
array [1..80] of int: Cs = [2, 5, 4, 3, 4, 6, 3, 1, 3, 1, 6, 2, 2, 3, 6, 1, 6, 4, 3, 1, 6, 4, 2, 5, 6, 2, 4, 1, 6, 1, 4, 3, 6, 4, 5, 3, 5, 4, 6, 2, 4, 2, 1, 5, 6, 2, 3, 1, 4, 3, 5, 6, 6, 3, 2, 4, 4, 6, 5, 1, 3, 2, 6, 4, 3, 6, 5, 1, 3, 5, 2, 6, 4, 3, 5, 2, 5, 4, 6, 2];\n\
var 4..6: INT____00001 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00002 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00003 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00004 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00005 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00006 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00007 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00008 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00009 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00010 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00011 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00012 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00013 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00014 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00015 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00016 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00017 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00018 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00019 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00020 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00021 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00022 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00023 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00024 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00025 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00026 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00027 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00028 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00029 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00030 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00031 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00032 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00033 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00034 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00035 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00036 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00037 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00038 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00039 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00040 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00041 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00042 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00043 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00044 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00045 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00046 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00047 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00048 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00049 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00050 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00051 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00052 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00053 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00054 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00055 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00056 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00057 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00058 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00059 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00060 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00061 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00062 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00063 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00064 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00065 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00066 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00067 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00068 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00069 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00070 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00071 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00072 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00073 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00074 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00075 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00076 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00077 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00078 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00079 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00080 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00081 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00082 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00083 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00084 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00085 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00086 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00087 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00088 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00089 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00090 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00091 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00092 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00093 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00094 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00095 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00096 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00097 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00098 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00099 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00100 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00101 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00102 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00103 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00104 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00105 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00106 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00107 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00108 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00109 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00110 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00111 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00112 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00113 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00114 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00115 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00116 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00117 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00118 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00119 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00120 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00121 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00122 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00123 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00124 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00125 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00126 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00127 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00128 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00129 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00130 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00131 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00132 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00133 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00134 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00135 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00136 :: is_defined_var :: var_is_introduced;\n\
var 7..9: INT____00137 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00138 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00139 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00140 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00141 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00142 :: is_defined_var :: var_is_introduced;\n\
var 13..15: INT____00143 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00144 :: is_defined_var :: var_is_introduced;\n\
var 10..12: INT____00145 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00146 :: is_defined_var :: var_is_introduced;\n\
var 16..18: INT____00147 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00148 :: is_defined_var :: var_is_introduced;\n\
var 4..6: INT____00149 :: is_defined_var :: var_is_introduced;\n\
var 1..6: INT____00150 :: is_defined_var :: var_is_introduced;\n\
array [1..80] of var 1..3: x :: output_array([1..20, 1..4]);\n")+"\
array [1..18] of var 1..6: z :: output_array([1..6, 1..3]);\n\
constraint all_different_int([INT____00002, INT____00004, INT____00006, INT____00008]);\n\
constraint all_different_int([INT____00010, INT____00012, INT____00014, INT____00015]);\n\
constraint all_different_int([INT____00017, INT____00018, INT____00020, INT____00022]);\n\
constraint all_different_int([INT____00024, INT____00026, INT____00028, INT____00029]);\n\
constraint all_different_int([INT____00031, INT____00033, INT____00035, INT____00036]);\n\
constraint all_different_int([INT____00038, INT____00040, INT____00042, INT____00044]);\n\
constraint all_different_int([INT____00046, INT____00048, INT____00050, INT____00051]);\n\
constraint all_different_int([INT____00053, INT____00054, INT____00056, INT____00058]);\n\
constraint all_different_int([INT____00060, INT____00062, INT____00064, INT____00066]);\n\
constraint all_different_int([INT____00068, INT____00070, INT____00072, INT____00074]);\n\
constraint all_different_int([INT____00076, INT____00078, INT____00079, INT____00081]);\n\
constraint all_different_int([INT____00083, INT____00085, INT____00087, INT____00088]);\n\
constraint all_different_int([INT____00090, INT____00092, INT____00094, INT____00096]);\n\
constraint all_different_int([INT____00098, INT____00100, INT____00102, INT____00104]);\n\
constraint all_different_int([INT____00106, INT____00108, INT____00110, INT____00111]);\n\
constraint all_different_int([INT____00113, INT____00115, INT____00117, INT____00119]);\n\
constraint all_different_int([INT____00121, INT____00123, INT____00125, INT____00126]);\n\
constraint all_different_int([INT____00128, INT____00130, INT____00132, INT____00134]);\n\
constraint all_different_int([INT____00136, INT____00138, INT____00140, INT____00142]);\n\
constraint all_different_int([INT____00144, INT____00146, INT____00148, INT____00150]);\n\
constraint array_var_int_element(INT____00001, z, INT____00002) :: defines_var(INT____00002);\n\
constraint array_var_int_element(INT____00003, z, INT____00004) :: defines_var(INT____00004);\n\
constraint array_var_int_element(INT____00005, z, INT____00006) :: defines_var(INT____00006);\n\
constraint array_var_int_element(INT____00007, z, INT____00008) :: defines_var(INT____00008);\n\
constraint array_var_int_element(INT____00009, z, INT____00010) :: defines_var(INT____00010);\n\
constraint array_var_int_element(INT____00011, z, INT____00012) :: defines_var(INT____00012);\n\
constraint array_var_int_element(INT____00013, z, INT____00014) :: defines_var(INT____00014);\n\
constraint array_var_int_element(INT____00016, z, INT____00017) :: defines_var(INT____00017);\n\
constraint array_var_int_element(INT____00019, z, INT____00020) :: defines_var(INT____00020);\n\
constraint array_var_int_element(INT____00021, z, INT____00022) :: defines_var(INT____00022);\n\
constraint array_var_int_element(INT____00023, z, INT____00024) :: defines_var(INT____00024);\n\
constraint array_var_int_element(INT____00025, z, INT____00026) :: defines_var(INT____00026);\n\
constraint array_var_int_element(INT____00027, z, INT____00028) :: defines_var(INT____00028);\n\
constraint array_var_int_element(INT____00030, z, INT____00031) :: defines_var(INT____00031);\n\
constraint array_var_int_element(INT____00032, z, INT____00033) :: defines_var(INT____00033);\n\
constraint array_var_int_element(INT____00034, z, INT____00035) :: defines_var(INT____00035);\n\
constraint array_var_int_element(INT____00037, z, INT____00038) :: defines_var(INT____00038);\n\
constraint array_var_int_element(INT____00039, z, INT____00040) :: defines_var(INT____00040);\n\
constraint array_var_int_element(INT____00041, z, INT____00042) :: defines_var(INT____00042);\n\
constraint array_var_int_element(INT____00043, z, INT____00044) :: defines_var(INT____00044);\n\
constraint array_var_int_element(INT____00045, z, INT____00046) :: defines_var(INT____00046);\n\
constraint array_var_int_element(INT____00047, z, INT____00048) :: defines_var(INT____00048);\n\
constraint array_var_int_element(INT____00049, z, INT____00050) :: defines_var(INT____00050);\n\
constraint array_var_int_element(INT____00052, z, INT____00053) :: defines_var(INT____00053);\n\
constraint array_var_int_element(INT____00055, z, INT____00056) :: defines_var(INT____00056);\n\
constraint array_var_int_element(INT____00057, z, INT____00058) :: defines_var(INT____00058);\n\
constraint array_var_int_element(INT____00059, z, INT____00060) :: defines_var(INT____00060);\n\
constraint array_var_int_element(INT____00061, z, INT____00062) :: defines_var(INT____00062);\n\
constraint array_var_int_element(INT____00063, z, INT____00064) :: defines_var(INT____00064);\n\
constraint array_var_int_element(INT____00065, z, INT____00066) :: defines_var(INT____00066);\n\
constraint array_var_int_element(INT____00067, z, INT____00068) :: defines_var(INT____00068);\n\
constraint array_var_int_element(INT____00069, z, INT____00070) :: defines_var(INT____00070);\n\
constraint array_var_int_element(INT____00071, z, INT____00072) :: defines_var(INT____00072);\n\
constraint array_var_int_element(INT____00073, z, INT____00074) :: defines_var(INT____00074);\n\
constraint array_var_int_element(INT____00075, z, INT____00076) :: defines_var(INT____00076);\n\
constraint array_var_int_element(INT____00077, z, INT____00078) :: defines_var(INT____00078);\n\
constraint array_var_int_element(INT____00080, z, INT____00081) :: defines_var(INT____00081);\n\
constraint array_var_int_element(INT____00082, z, INT____00083) :: defines_var(INT____00083);\n\
constraint array_var_int_element(INT____00084, z, INT____00085) :: defines_var(INT____00085);\n\
constraint array_var_int_element(INT____00086, z, INT____00087) :: defines_var(INT____00087);\n\
constraint array_var_int_element(INT____00089, z, INT____00090) :: defines_var(INT____00090);\n\
constraint array_var_int_element(INT____00091, z, INT____00092) :: defines_var(INT____00092);\n\
constraint array_var_int_element(INT____00093, z, INT____00094) :: defines_var(INT____00094);\n\
constraint array_var_int_element(INT____00095, z, INT____00096) :: defines_var(INT____00096);\n\
constraint array_var_int_element(INT____00097, z, INT____00098) :: defines_var(INT____00098);\n\
constraint array_var_int_element(INT____00099, z, INT____00100) :: defines_var(INT____00100);\n\
constraint array_var_int_element(INT____00101, z, INT____00102) :: defines_var(INT____00102);\n\
constraint array_var_int_element(INT____00103, z, INT____00104) :: defines_var(INT____00104);\n\
constraint array_var_int_element(INT____00105, z, INT____00106) :: defines_var(INT____00106);\n\
constraint array_var_int_element(INT____00107, z, INT____00108) :: defines_var(INT____00108);\n\
constraint array_var_int_element(INT____00109, z, INT____00110) :: defines_var(INT____00110);\n\
constraint array_var_int_element(INT____00112, z, INT____00113) :: defines_var(INT____00113);\n\
constraint array_var_int_element(INT____00114, z, INT____00115) :: defines_var(INT____00115);\n\
constraint array_var_int_element(INT____00116, z, INT____00117) :: defines_var(INT____00117);\n\
constraint array_var_int_element(INT____00118, z, INT____00119) :: defines_var(INT____00119);\n\
constraint array_var_int_element(INT____00120, z, INT____00121) :: defines_var(INT____00121);\n\
constraint array_var_int_element(INT____00122, z, INT____00123) :: defines_var(INT____00123);\n\
constraint array_var_int_element(INT____00124, z, INT____00125) :: defines_var(INT____00125);\n\
constraint array_var_int_element(INT____00127, z, INT____00128) :: defines_var(INT____00128);\n\
constraint array_var_int_element(INT____00129, z, INT____00130) :: defines_var(INT____00130);\n\
constraint array_var_int_element(INT____00131, z, INT____00132) :: defines_var(INT____00132);\n\
constraint array_var_int_element(INT____00133, z, INT____00134) :: defines_var(INT____00134);\n\
constraint array_var_int_element(INT____00135, z, INT____00136) :: defines_var(INT____00136);\n\
constraint array_var_int_element(INT____00137, z, INT____00138) :: defines_var(INT____00138);\n\
constraint array_var_int_element(INT____00139, z, INT____00140) :: defines_var(INT____00140);\n\
constraint array_var_int_element(INT____00141, z, INT____00142) :: defines_var(INT____00142);\n\
constraint array_var_int_element(INT____00143, z, INT____00144) :: defines_var(INT____00144);\n\
constraint array_var_int_element(INT____00145, z, INT____00146) :: defines_var(INT____00146);\n\
constraint array_var_int_element(INT____00147, z, INT____00148) :: defines_var(INT____00148);\n\
constraint array_var_int_element(INT____00149, z, INT____00150) :: defines_var(INT____00150);\n\
constraint array_var_int_element(x[8], z, INT____00015) :: defines_var(INT____00015);\n\
constraint array_var_int_element(x[10], z, INT____00018) :: defines_var(INT____00018);\n\
constraint array_var_int_element(x[16], z, INT____00029) :: defines_var(INT____00029);\n\
constraint array_var_int_element(x[20], z, INT____00036) :: defines_var(INT____00036);\n\
constraint array_var_int_element(x[28], z, INT____00051) :: defines_var(INT____00051);\n\
constraint array_var_int_element(x[30], z, INT____00054) :: defines_var(INT____00054);\n\
constraint array_var_int_element(x[43], z, INT____00079) :: defines_var(INT____00079);\n\
constraint array_var_int_element(x[48], z, INT____00088) :: defines_var(INT____00088);\n\
constraint array_var_int_element(x[60], z, INT____00111) :: defines_var(INT____00111);\n\
constraint array_var_int_element(x[68], z, INT____00126) :: defines_var(INT____00126);\n\
constraint int_lin_eq([-1, 1], [INT____00001, x[1]], -3) :: defines_var(INT____00001) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00003, x[2]], -12) :: defines_var(INT____00003) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00005, x[3]], -9) :: defines_var(INT____00005) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00007, x[4]], -6) :: defines_var(INT____00007) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00009, x[5]], -9) :: defines_var(INT____00009) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00011, x[6]], -15) :: defines_var(INT____00011) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00013, x[7]], -6) :: defines_var(INT____00013) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00016, x[9]], -6) :: defines_var(INT____00016) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00019, x[11]], -15) :: defines_var(INT____00019) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00021, x[12]], -3) :: defines_var(INT____00021) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00023, x[13]], -3) :: defines_var(INT____00023) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00025, x[14]], -6) :: defines_var(INT____00025) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00027, x[15]], -15) :: defines_var(INT____00027) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00030, x[17]], -15) :: defines_var(INT____00030) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00032, x[18]], -9) :: defines_var(INT____00032) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00034, x[19]], -6) :: defines_var(INT____00034) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00037, x[21]], -15) :: defines_var(INT____00037) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00039, x[22]], -9) :: defines_var(INT____00039) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00041, x[23]], -3) :: defines_var(INT____00041) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00043, x[24]], -12) :: defines_var(INT____00043) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00045, x[25]], -15) :: defines_var(INT____00045) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00047, x[26]], -3) :: defines_var(INT____00047) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00049, x[27]], -9) :: defines_var(INT____00049) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00052, x[29]], -15) :: defines_var(INT____00052) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00055, x[31]], -9) :: defines_var(INT____00055) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00057, x[32]], -6) :: defines_var(INT____00057) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00059, x[33]], -15) :: defines_var(INT____00059) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00061, x[34]], -9) :: defines_var(INT____00061) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00063, x[35]], -12) :: defines_var(INT____00063) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00065, x[36]], -6) :: defines_var(INT____00065) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00067, x[37]], -12) :: defines_var(INT____00067) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00069, x[38]], -9) :: defines_var(INT____00069) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00071, x[39]], -15) :: defines_var(INT____00071) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00073, x[40]], -3) :: defines_var(INT____00073) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00075, x[41]], -9) :: defines_var(INT____00075) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00077, x[42]], -3) :: defines_var(INT____00077) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00080, x[44]], -12) :: defines_var(INT____00080) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00082, x[45]], -15) :: defines_var(INT____00082) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00084, x[46]], -3) :: defines_var(INT____00084) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00086, x[47]], -6) :: defines_var(INT____00086) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00089, x[49]], -9) :: defines_var(INT____00089) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00091, x[50]], -6) :: defines_var(INT____00091) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00093, x[51]], -12) :: defines_var(INT____00093) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00095, x[52]], -15) :: defines_var(INT____00095) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00097, x[53]], -15) :: defines_var(INT____00097) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00099, x[54]], -6) :: defines_var(INT____00099) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00101, x[55]], -3) :: defines_var(INT____00101) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00103, x[56]], -9) :: defines_var(INT____00103) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00105, x[57]], -9) :: defines_var(INT____00105) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00107, x[58]], -15) :: defines_var(INT____00107) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00109, x[59]], -12) :: defines_var(INT____00109) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00112, x[61]], -6) :: defines_var(INT____00112) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00114, x[62]], -3) :: defines_var(INT____00114) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00116, x[63]], -15) :: defines_var(INT____00116) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00118, x[64]], -9) :: defines_var(INT____00118) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00120, x[65]], -6) :: defines_var(INT____00120) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00122, x[66]], -15) :: defines_var(INT____00122) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00124, x[67]], -12) :: defines_var(INT____00124) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00127, x[69]], -6) :: defines_var(INT____00127) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00129, x[70]], -12) :: defines_var(INT____00129) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00131, x[71]], -3) :: defines_var(INT____00131) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00133, x[72]], -15) :: defines_var(INT____00133) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00135, x[73]], -9) :: defines_var(INT____00135) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00137, x[74]], -6) :: defines_var(INT____00137) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00139, x[75]], -12) :: defines_var(INT____00139) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00141, x[76]], -3) :: defines_var(INT____00141) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00143, x[77]], -12) :: defines_var(INT____00143) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00145, x[78]], -9) :: defines_var(INT____00145) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00147, x[79]], -15) :: defines_var(INT____00147) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00149, x[80]], -3) :: defines_var(INT____00149) :: domain;\n\
solve satisfy;\n\
", "x = array2d(1..20, 1..4, [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]);\n\
z = array2d(1..6, 1..3, [6, 1, 1, 4, 1, 1, 3, 1, 1, 2, 1, 1, 5, 1, 1, 1, 1, 1]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
