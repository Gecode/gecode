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
        (void) new FlatZincTest("blocksworld::2",
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
predicate table_int(array [int] of var int: x, array [int, int] of int: t);\n")+
"array [1..5] of int: final_loc = [2, 3, 4, 5, 0];\n\
array [1..5] of int: initial_loc = [0, 1, 2, 3, 4];\n\
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
var bool: BOOL____00090 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00091 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00092 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00093 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00094 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00095 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00096 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00097 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00098 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00099 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00100 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00101 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00102 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00103 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00104 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00105 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00106 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00107 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00108 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00109 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00110 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00111 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00112 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00113 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00114 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00115 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00116 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00117 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00118 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00119 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00120 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00121 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00122 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00123 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00124 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00125 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00126 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00127 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00128 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00129 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00130 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00131 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00132 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00133 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00134 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00135 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00136 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00137 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00138 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00139 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00140 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00141 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00142 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00143 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00144 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00145 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00146 :: is_defined_var :: var_is_introduced;\n"+
"var bool: BOOL____00147 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00148 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00149 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00150 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00151 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00152 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00153 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00154 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00155 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00156 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00157 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00158 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00159 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00160 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00161 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00162 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00163 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00164 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00165 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00166 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00167 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00168 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00169 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00170 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00171 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00172 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00173 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00174 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00175 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00176 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00177 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00178 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00179 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00180 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00181 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00182 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00183 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00184 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00185 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00186 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00187 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00188 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00189 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00190 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00191 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00192 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00193 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00194 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00195 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00196 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00197 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00198 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00199 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00200 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00201 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00202 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00203 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00204 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00205 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00206 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00207 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00208 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00209 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00210 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00211 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00212 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00213 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00214 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00215 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00216 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00217 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00218 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00219 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00220 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00221 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00222 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00223 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00224 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00225 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00226 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00227 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00228 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00229 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00230 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00231 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00232 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00233 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00234 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00235 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00236 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00237 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00238 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00239 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00240 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00241 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00242 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00243 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00244 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00245 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00246 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00247 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00248 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00249 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00250 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00251 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00252 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00253 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00254 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00255 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00256 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00257 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00258 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00259 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00260 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00261 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00262 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00263 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00264 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00265 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00266 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00267 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00268 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00269 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00270 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00271 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00272 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00273 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00274 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00275 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00276 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00277 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00278 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00279 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00280 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00281 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00282 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00283 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00284 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00285 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00286 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00287 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00288 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00289 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00290 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00291 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00292 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00293 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00294 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00295 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00296 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00297 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00298 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00299 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00300 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00301 :: is_defined_var :: var_is_introduced;\n"+
"var bool: BOOL____00302 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00303 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00304 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00305 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00306 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00307 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00308 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00309 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00310 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00311 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00312 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00313 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00314 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00315 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00316 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00317 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00318 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00319 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00320 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00321 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00322 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00323 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00324 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00325 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00326 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00327 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00328 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00329 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00330 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00331 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00332 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00333 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00334 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00335 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00336 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00337 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00338 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00339 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00340 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00341 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00342 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00343 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00344 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00345 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00346 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00347 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00348 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00349 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00350 :: is_defined_var :: var_is_introduced;\n\
array [1..30] of var -5..5: on :: output_array([1..6, 1..5]);\n\
constraint all_different_int([on[1], on[2], on[3], on[4], on[5]]);\n\
constraint all_different_int([on[6], on[7], on[8], on[9], on[10]]);\n\
constraint all_different_int([on[11], on[12], on[13], on[14], on[15]]);\n\
constraint all_different_int([on[16], on[17], on[18], on[19], on[20]]);\n\
constraint all_different_int([on[21], on[22], on[23], on[24], on[25]]);\n\
constraint all_different_int([on[26], on[27], on[28], on[29], on[30]]);\n\
constraint array_bool_and([BOOL____00011, BOOL____00010, BOOL____00009, BOOL____00008, BOOL____00001], BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint array_bool_and([BOOL____00025, BOOL____00024, BOOL____00023, BOOL____00015, BOOL____00022], BOOL____00026) :: defines_var(BOOL____00026);\n\
constraint array_bool_and([BOOL____00039, BOOL____00038, BOOL____00029, BOOL____00037, BOOL____00036], BOOL____00040) :: defines_var(BOOL____00040);\n\
constraint array_bool_and([BOOL____00053, BOOL____00043, BOOL____00052, BOOL____00051, BOOL____00050], BOOL____00054) :: defines_var(BOOL____00054);\n\
constraint array_bool_and([BOOL____00057, BOOL____00067, BOOL____00066, BOOL____00065, BOOL____00064], BOOL____00068) :: defines_var(BOOL____00068);\n\
constraint array_bool_and([BOOL____00081, BOOL____00080, BOOL____00079, BOOL____00078, BOOL____00071], BOOL____00082) :: defines_var(BOOL____00082);\n\
constraint array_bool_and([BOOL____00095, BOOL____00094, BOOL____00093, BOOL____00085, BOOL____00092], BOOL____00096) :: defines_var(BOOL____00096);\n\
constraint array_bool_and([BOOL____00109, BOOL____00108, BOOL____00099, BOOL____00107, BOOL____00106], BOOL____00110) :: defines_var(BOOL____00110);\n\
constraint array_bool_and([BOOL____00123, BOOL____00113, BOOL____00122, BOOL____00121, BOOL____00120], BOOL____00124) :: defines_var(BOOL____00124);\n\
constraint array_bool_and([BOOL____00127, BOOL____00137, BOOL____00136, BOOL____00135, BOOL____00134], BOOL____00138) :: defines_var(BOOL____00138);\n\
constraint array_bool_and([BOOL____00151, BOOL____00150, BOOL____00149, BOOL____00148, BOOL____00141], BOOL____00152) :: defines_var(BOOL____00152);\n\
constraint array_bool_and([BOOL____00165, BOOL____00164, BOOL____00163, BOOL____00155, BOOL____00162], BOOL____00166) :: defines_var(BOOL____00166);\n\
constraint array_bool_and([BOOL____00179, BOOL____00178, BOOL____00169, BOOL____00177, BOOL____00176], BOOL____00180) :: defines_var(BOOL____00180);\n\
constraint array_bool_and([BOOL____00193, BOOL____00183, BOOL____00192, BOOL____00191, BOOL____00190], BOOL____00194) :: defines_var(BOOL____00194);\n\
constraint array_bool_and([BOOL____00197, BOOL____00207, BOOL____00206, BOOL____00205, BOOL____00204], BOOL____00208) :: defines_var(BOOL____00208);\n\
constraint array_bool_and([BOOL____00221, BOOL____00220, BOOL____00219, BOOL____00218, BOOL____00211], BOOL____00222) :: defines_var(BOOL____00222);\n\
constraint array_bool_and([BOOL____00235, BOOL____00234, BOOL____00233, BOOL____00225, BOOL____00232], BOOL____00236) :: defines_var(BOOL____00236);\n\
constraint array_bool_and([BOOL____00249, BOOL____00248, BOOL____00239, BOOL____00247, BOOL____00246], BOOL____00250) :: defines_var(BOOL____00250);\n\
constraint array_bool_and([BOOL____00263, BOOL____00253, BOOL____00262, BOOL____00261, BOOL____00260], BOOL____00264) :: defines_var(BOOL____00264);\n\
constraint array_bool_and([BOOL____00267, BOOL____00277, BOOL____00276, BOOL____00275, BOOL____00274], BOOL____00278) :: defines_var(BOOL____00278);\n\
constraint array_bool_and([BOOL____00291, BOOL____00290, BOOL____00289, BOOL____00288, BOOL____00281], BOOL____00292) :: defines_var(BOOL____00292);\n\
constraint array_bool_and([BOOL____00305, BOOL____00304, BOOL____00303, BOOL____00295, BOOL____00302], BOOL____00306) :: defines_var(BOOL____00306);\n\
constraint array_bool_and([BOOL____00319, BOOL____00318, BOOL____00309, BOOL____00317, BOOL____00316], BOOL____00320) :: defines_var(BOOL____00320);\n\
constraint array_bool_and([BOOL____00333, BOOL____00323, BOOL____00332, BOOL____00331, BOOL____00330], BOOL____00334) :: defines_var(BOOL____00334);\n\
constraint array_bool_and([BOOL____00337, BOOL____00347, BOOL____00346, BOOL____00345, BOOL____00344], BOOL____00348) :: defines_var(BOOL____00348);\n\
constraint array_bool_and([BOOL____00013, BOOL____00006, BOOL____00005, BOOL____00004, BOOL____00003, BOOL____00002], BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint array_bool_and([BOOL____00027, BOOL____00020, BOOL____00019, BOOL____00018, BOOL____00017, BOOL____00016], BOOL____00028) :: defines_var(BOOL____00028);\n\
constraint array_bool_and([BOOL____00041, BOOL____00034, BOOL____00033, BOOL____00032, BOOL____00031, BOOL____00030], BOOL____00042) :: defines_var(BOOL____00042);\n\
constraint array_bool_and([BOOL____00055, BOOL____00048, BOOL____00047, BOOL____00046, BOOL____00045, BOOL____00044], BOOL____00056) :: defines_var(BOOL____00056);\n\
constraint array_bool_and([BOOL____00069, BOOL____00062, BOOL____00061, BOOL____00060, BOOL____00059, BOOL____00058], BOOL____00070) :: defines_var(BOOL____00070);\n\
constraint array_bool_and([BOOL____00083, BOOL____00076, BOOL____00075, BOOL____00074, BOOL____00073, BOOL____00072], BOOL____00084) :: defines_var(BOOL____00084);\n\
constraint array_bool_and([BOOL____00097, BOOL____00090, BOOL____00089, BOOL____00088, BOOL____00087, BOOL____00086], BOOL____00098) :: defines_var(BOOL____00098);\n\
constraint array_bool_and([BOOL____00111, BOOL____00104, BOOL____00103, BOOL____00102, BOOL____00101, BOOL____00100], BOOL____00112) :: defines_var(BOOL____00112);\n\
constraint array_bool_and([BOOL____00125, BOOL____00118, BOOL____00117, BOOL____00116, BOOL____00115, BOOL____00114], BOOL____00126) :: defines_var(BOOL____00126);\n\
constraint array_bool_and([BOOL____00139, BOOL____00132, BOOL____00131, BOOL____00130, BOOL____00129, BOOL____00128], BOOL____00140) :: defines_var(BOOL____00140);\n\
constraint array_bool_and([BOOL____00153, BOOL____00146, BOOL____00145, BOOL____00144, BOOL____00143, BOOL____00142], BOOL____00154) :: defines_var(BOOL____00154);\n\
constraint array_bool_and([BOOL____00167, BOOL____00160, BOOL____00159, BOOL____00158, BOOL____00157, BOOL____00156], BOOL____00168) :: defines_var(BOOL____00168);\n\
constraint array_bool_and([BOOL____00181, BOOL____00174, BOOL____00173, BOOL____00172, BOOL____00171, BOOL____00170], BOOL____00182) :: defines_var(BOOL____00182);\n\
constraint array_bool_and([BOOL____00195, BOOL____00188, BOOL____00187, BOOL____00186, BOOL____00185, BOOL____00184], BOOL____00196) :: defines_var(BOOL____00196);\n\
constraint array_bool_and([BOOL____00209, BOOL____00202, BOOL____00201, BOOL____00200, BOOL____00199, BOOL____00198], BOOL____00210) :: defines_var(BOOL____00210);\n\
constraint array_bool_and([BOOL____00223, BOOL____00216, BOOL____00215, BOOL____00214, BOOL____00213, BOOL____00212], BOOL____00224) :: defines_var(BOOL____00224);\n\
constraint array_bool_and([BOOL____00237, BOOL____00230, BOOL____00229, BOOL____00228, BOOL____00227, BOOL____00226], BOOL____00238) :: defines_var(BOOL____00238);\n\
constraint array_bool_and([BOOL____00251, BOOL____00244, BOOL____00243, BOOL____00242, BOOL____00241, BOOL____00240], BOOL____00252) :: defines_var(BOOL____00252);\n\
constraint array_bool_and([BOOL____00265, BOOL____00258, BOOL____00257, BOOL____00256, BOOL____00255, BOOL____00254], BOOL____00266) :: defines_var(BOOL____00266);\n\
constraint array_bool_and([BOOL____00279, BOOL____00272, BOOL____00271, BOOL____00270, BOOL____00269, BOOL____00268], BOOL____00280) :: defines_var(BOOL____00280);\n\
constraint array_bool_and([BOOL____00293, BOOL____00286, BOOL____00285, BOOL____00284, BOOL____00283, BOOL____00282], BOOL____00294) :: defines_var(BOOL____00294);\n\
constraint array_bool_and([BOOL____00307, BOOL____00300, BOOL____00299, BOOL____00298, BOOL____00297, BOOL____00296], BOOL____00308) :: defines_var(BOOL____00308);\n\
constraint array_bool_and([BOOL____00321, BOOL____00314, BOOL____00313, BOOL____00312, BOOL____00311, BOOL____00310], BOOL____00322) :: defines_var(BOOL____00322);\n\
constraint array_bool_and([BOOL____00335, BOOL____00328, BOOL____00327, BOOL____00326, BOOL____00325, BOOL____00324], BOOL____00336) :: defines_var(BOOL____00336);\n\
constraint array_bool_and([BOOL____00349, BOOL____00342, BOOL____00341, BOOL____00340, BOOL____00339, BOOL____00338], BOOL____00350) :: defines_var(BOOL____00350);\n\
constraint array_bool_or([BOOL____00012, BOOL____00007], BOOL____00013) :: defines_var(BOOL____00013);\n\
constraint array_bool_or([BOOL____00026, BOOL____00021], BOOL____00027) :: defines_var(BOOL____00027);\n\
constraint array_bool_or([BOOL____00040, BOOL____00035], BOOL____00041) :: defines_var(BOOL____00041);\n\
constraint array_bool_or([BOOL____00054, BOOL____00049], BOOL____00055) :: defines_var(BOOL____00055);\n\
constraint array_bool_or([BOOL____00068, BOOL____00063], BOOL____00069) :: defines_var(BOOL____00069);\n\
constraint array_bool_or([BOOL____00082, BOOL____00077], BOOL____00083) :: defines_var(BOOL____00083);\n\
constraint array_bool_or([BOOL____00096, BOOL____00091], BOOL____00097) :: defines_var(BOOL____00097);\n\
constraint array_bool_or([BOOL____00110, BOOL____00105], BOOL____00111) :: defines_var(BOOL____00111);\n\
constraint array_bool_or([BOOL____00124, BOOL____00119], BOOL____00125) :: defines_var(BOOL____00125);\n\
constraint array_bool_or([BOOL____00138, BOOL____00133], BOOL____00139) :: defines_var(BOOL____00139);\n\
constraint array_bool_or([BOOL____00152, BOOL____00147], BOOL____00153) :: defines_var(BOOL____00153);\n\
constraint array_bool_or([BOOL____00166, BOOL____00161], BOOL____00167) :: defines_var(BOOL____00167);\n\
constraint array_bool_or([BOOL____00180, BOOL____00175], BOOL____00181) :: defines_var(BOOL____00181);\n\
constraint array_bool_or([BOOL____00194, BOOL____00189], BOOL____00195) :: defines_var(BOOL____00195);\n\
constraint array_bool_or([BOOL____00208, BOOL____00203], BOOL____00209) :: defines_var(BOOL____00209);\n\
constraint array_bool_or([BOOL____00222, BOOL____00217], BOOL____00223) :: defines_var(BOOL____00223);\n\
constraint array_bool_or([BOOL____00236, BOOL____00231], BOOL____00237) :: defines_var(BOOL____00237);\n\
constraint array_bool_or([BOOL____00250, BOOL____00245], BOOL____00251) :: defines_var(BOOL____00251);\n\
constraint array_bool_or([BOOL____00264, BOOL____00259], BOOL____00265) :: defines_var(BOOL____00265);\n\
constraint array_bool_or([BOOL____00278, BOOL____00273], BOOL____00279) :: defines_var(BOOL____00279);\n\
constraint array_bool_or([BOOL____00292, BOOL____00287], BOOL____00293) :: defines_var(BOOL____00293);\n\
constraint array_bool_or([BOOL____00306, BOOL____00301], BOOL____00307) :: defines_var(BOOL____00307);\n\
constraint array_bool_or([BOOL____00320, BOOL____00315], BOOL____00321) :: defines_var(BOOL____00321);\n\
constraint array_bool_or([BOOL____00334, BOOL____00329], BOOL____00335) :: defines_var(BOOL____00335);\n\
constraint array_bool_or([BOOL____00348, BOOL____00343], BOOL____00349) :: defines_var(BOOL____00349);\n"+
"constraint bool_le(BOOL____00001, BOOL____00014);\n\
constraint bool_le(BOOL____00015, BOOL____00028);\n\
constraint bool_le(BOOL____00029, BOOL____00042);\n\
constraint bool_le(BOOL____00043, BOOL____00056);\n\
constraint bool_le(BOOL____00057, BOOL____00070);\n\
constraint bool_le(BOOL____00071, BOOL____00084);\n\
constraint bool_le(BOOL____00085, BOOL____00098);\n\
constraint bool_le(BOOL____00099, BOOL____00112);\n\
constraint bool_le(BOOL____00113, BOOL____00126);\n\
constraint bool_le(BOOL____00127, BOOL____00140);\n\
constraint bool_le(BOOL____00141, BOOL____00154);\n\
constraint bool_le(BOOL____00155, BOOL____00168);\n\
constraint bool_le(BOOL____00169, BOOL____00182);\n\
constraint bool_le(BOOL____00183, BOOL____00196);\n\
constraint bool_le(BOOL____00197, BOOL____00210);\n\
constraint bool_le(BOOL____00211, BOOL____00224);\n\
constraint bool_le(BOOL____00225, BOOL____00238);\n\
constraint bool_le(BOOL____00239, BOOL____00252);\n\
constraint bool_le(BOOL____00253, BOOL____00266);\n\
constraint bool_le(BOOL____00267, BOOL____00280);\n\
constraint bool_le(BOOL____00281, BOOL____00294);\n\
constraint bool_le(BOOL____00295, BOOL____00308);\n\
constraint bool_le(BOOL____00309, BOOL____00322);\n\
constraint bool_le(BOOL____00323, BOOL____00336);\n\
constraint bool_le(BOOL____00337, BOOL____00350);\n\
constraint int_eq(on[1], -1);\n\
constraint int_eq(on[2], 1);\n\
constraint int_eq(on[3], 2);\n\
constraint int_eq(on[4], 3);\n\
constraint int_eq(on[5], 4);\n\
constraint int_eq(on[26], 2);\n\
constraint int_eq(on[27], 3);\n\
constraint int_eq(on[28], 4);\n\
constraint int_eq(on[29], 5);\n\
constraint int_eq(on[30], -5);\n\
constraint int_lt_reif(on[6], 0, BOOL____00007) :: defines_var(BOOL____00007);\n\
constraint int_lt_reif(on[7], 0, BOOL____00021) :: defines_var(BOOL____00021);\n\
constraint int_lt_reif(on[8], 0, BOOL____00035) :: defines_var(BOOL____00035);\n\
constraint int_lt_reif(on[9], 0, BOOL____00049) :: defines_var(BOOL____00049);\n\
constraint int_lt_reif(on[10], 0, BOOL____00063) :: defines_var(BOOL____00063);\n\
constraint int_lt_reif(on[11], 0, BOOL____00077) :: defines_var(BOOL____00077);\n\
constraint int_lt_reif(on[12], 0, BOOL____00091) :: defines_var(BOOL____00091);\n\
constraint int_lt_reif(on[13], 0, BOOL____00105) :: defines_var(BOOL____00105);\n\
constraint int_lt_reif(on[14], 0, BOOL____00119) :: defines_var(BOOL____00119);\n\
constraint int_lt_reif(on[15], 0, BOOL____00133) :: defines_var(BOOL____00133);\n\
constraint int_lt_reif(on[16], 0, BOOL____00147) :: defines_var(BOOL____00147);\n\
constraint int_lt_reif(on[17], 0, BOOL____00161) :: defines_var(BOOL____00161);\n\
constraint int_lt_reif(on[18], 0, BOOL____00175) :: defines_var(BOOL____00175);\n\
constraint int_lt_reif(on[19], 0, BOOL____00189) :: defines_var(BOOL____00189);\n\
constraint int_lt_reif(on[20], 0, BOOL____00203) :: defines_var(BOOL____00203);\n\
constraint int_lt_reif(on[21], 0, BOOL____00217) :: defines_var(BOOL____00217);\n\
constraint int_lt_reif(on[22], 0, BOOL____00231) :: defines_var(BOOL____00231);\n\
constraint int_lt_reif(on[23], 0, BOOL____00245) :: defines_var(BOOL____00245);\n\
constraint int_lt_reif(on[24], 0, BOOL____00259) :: defines_var(BOOL____00259);\n\
constraint int_lt_reif(on[25], 0, BOOL____00273) :: defines_var(BOOL____00273);\n\
constraint int_lt_reif(on[26], 0, BOOL____00287) :: defines_var(BOOL____00287);\n\
constraint int_lt_reif(on[27], 0, BOOL____00301) :: defines_var(BOOL____00301);\n\
constraint int_lt_reif(on[28], 0, BOOL____00315) :: defines_var(BOOL____00315);\n\
constraint int_lt_reif(on[29], 0, BOOL____00329) :: defines_var(BOOL____00329);\n\
constraint int_lt_reif(on[30], 0, BOOL____00343) :: defines_var(BOOL____00343);\n\
constraint int_ne_reif(on[1], 1, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_ne_reif(on[1], 2, BOOL____00016) :: defines_var(BOOL____00016);\n\
constraint int_ne_reif(on[1], 3, BOOL____00030) :: defines_var(BOOL____00030);\n\
constraint int_ne_reif(on[1], 4, BOOL____00044) :: defines_var(BOOL____00044);\n\
constraint int_ne_reif(on[1], 5, BOOL____00058) :: defines_var(BOOL____00058);\n\
constraint int_ne_reif(on[1], on[6], BOOL____00001) :: defines_var(BOOL____00001);\n\
constraint int_ne_reif(on[1], on[7], BOOL____00022) :: defines_var(BOOL____00022);\n\
constraint int_ne_reif(on[1], on[8], BOOL____00036) :: defines_var(BOOL____00036);\n\
constraint int_ne_reif(on[1], on[9], BOOL____00050) :: defines_var(BOOL____00050);\n\
constraint int_ne_reif(on[1], on[10], BOOL____00064) :: defines_var(BOOL____00064);\n\
constraint int_ne_reif(on[2], 1, BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_ne_reif(on[2], 2, BOOL____00017) :: defines_var(BOOL____00017);\n\
constraint int_ne_reif(on[2], 3, BOOL____00031) :: defines_var(BOOL____00031);\n\
constraint int_ne_reif(on[2], 4, BOOL____00045) :: defines_var(BOOL____00045);\n\
constraint int_ne_reif(on[2], 5, BOOL____00059) :: defines_var(BOOL____00059);\n\
constraint int_ne_reif(on[2], on[6], BOOL____00008) :: defines_var(BOOL____00008);\n\
constraint int_ne_reif(on[2], on[7], BOOL____00015) :: defines_var(BOOL____00015);\n\
constraint int_ne_reif(on[2], on[8], BOOL____00037) :: defines_var(BOOL____00037);\n\
constraint int_ne_reif(on[2], on[9], BOOL____00051) :: defines_var(BOOL____00051);\n\
constraint int_ne_reif(on[2], on[10], BOOL____00065) :: defines_var(BOOL____00065);\n\
constraint int_ne_reif(on[3], 1, BOOL____00004) :: defines_var(BOOL____00004);\n\
constraint int_ne_reif(on[3], 2, BOOL____00018) :: defines_var(BOOL____00018);\n\
constraint int_ne_reif(on[3], 3, BOOL____00032) :: defines_var(BOOL____00032);\n\
constraint int_ne_reif(on[3], 4, BOOL____00046) :: defines_var(BOOL____00046);\n\
constraint int_ne_reif(on[3], 5, BOOL____00060) :: defines_var(BOOL____00060);\n\
constraint int_ne_reif(on[3], on[6], BOOL____00009) :: defines_var(BOOL____00009);\n\
constraint int_ne_reif(on[3], on[7], BOOL____00023) :: defines_var(BOOL____00023);\n\
constraint int_ne_reif(on[3], on[8], BOOL____00029) :: defines_var(BOOL____00029);\n\
constraint int_ne_reif(on[3], on[9], BOOL____00052) :: defines_var(BOOL____00052);\n\
constraint int_ne_reif(on[3], on[10], BOOL____00066) :: defines_var(BOOL____00066);\n\
constraint int_ne_reif(on[4], 1, BOOL____00005) :: defines_var(BOOL____00005);\n\
constraint int_ne_reif(on[4], 2, BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint int_ne_reif(on[4], 3, BOOL____00033) :: defines_var(BOOL____00033);\n\
constraint int_ne_reif(on[4], 4, BOOL____00047) :: defines_var(BOOL____00047);\n\
constraint int_ne_reif(on[4], 5, BOOL____00061) :: defines_var(BOOL____00061);\n\
constraint int_ne_reif(on[4], on[6], BOOL____00010) :: defines_var(BOOL____00010);\n\
constraint int_ne_reif(on[4], on[7], BOOL____00024) :: defines_var(BOOL____00024);\n\
constraint int_ne_reif(on[4], on[8], BOOL____00038) :: defines_var(BOOL____00038);\n\
constraint int_ne_reif(on[4], on[9], BOOL____00043) :: defines_var(BOOL____00043);\n\
constraint int_ne_reif(on[4], on[10], BOOL____00067) :: defines_var(BOOL____00067);\n\
constraint int_ne_reif(on[5], 1, BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_ne_reif(on[5], 2, BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_ne_reif(on[5], 3, BOOL____00034) :: defines_var(BOOL____00034);\n\
constraint int_ne_reif(on[5], 4, BOOL____00048) :: defines_var(BOOL____00048);\n\
constraint int_ne_reif(on[5], 5, BOOL____00062) :: defines_var(BOOL____00062);\n\
constraint int_ne_reif(on[5], on[6], BOOL____00011) :: defines_var(BOOL____00011);\n\
constraint int_ne_reif(on[5], on[7], BOOL____00025) :: defines_var(BOOL____00025);\n\
constraint int_ne_reif(on[5], on[8], BOOL____00039) :: defines_var(BOOL____00039);\n\
constraint int_ne_reif(on[5], on[9], BOOL____00053) :: defines_var(BOOL____00053);\n\
constraint int_ne_reif(on[5], on[10], BOOL____00057) :: defines_var(BOOL____00057);\n\
constraint int_ne_reif(on[6], 1, BOOL____00072) :: defines_var(BOOL____00072);\n\
constraint int_ne_reif(on[6], 2, BOOL____00086) :: defines_var(BOOL____00086);\n\
constraint int_ne_reif(on[6], 3, BOOL____00100) :: defines_var(BOOL____00100);\n\
constraint int_ne_reif(on[6], 4, BOOL____00114) :: defines_var(BOOL____00114);\n\
constraint int_ne_reif(on[6], 5, BOOL____00128) :: defines_var(BOOL____00128);\n\
constraint int_ne_reif(on[6], on[11], BOOL____00071) :: defines_var(BOOL____00071);\n\
constraint int_ne_reif(on[6], on[12], BOOL____00092) :: defines_var(BOOL____00092);\n\
constraint int_ne_reif(on[6], on[13], BOOL____00106) :: defines_var(BOOL____00106);\n\
constraint int_ne_reif(on[6], on[14], BOOL____00120) :: defines_var(BOOL____00120);\n\
constraint int_ne_reif(on[6], on[15], BOOL____00134) :: defines_var(BOOL____00134);\n\
constraint int_ne_reif(on[7], 1, BOOL____00073) :: defines_var(BOOL____00073);\n\
constraint int_ne_reif(on[7], 2, BOOL____00087) :: defines_var(BOOL____00087);\n\
constraint int_ne_reif(on[7], 3, BOOL____00101) :: defines_var(BOOL____00101);\n\
constraint int_ne_reif(on[7], 4, BOOL____00115) :: defines_var(BOOL____00115);\n\
constraint int_ne_reif(on[7], 5, BOOL____00129) :: defines_var(BOOL____00129);\n\
constraint int_ne_reif(on[7], on[11], BOOL____00078) :: defines_var(BOOL____00078);\n\
constraint int_ne_reif(on[7], on[12], BOOL____00085) :: defines_var(BOOL____00085);\n\
constraint int_ne_reif(on[7], on[13], BOOL____00107) :: defines_var(BOOL____00107);\n\
constraint int_ne_reif(on[7], on[14], BOOL____00121) :: defines_var(BOOL____00121);\n\
constraint int_ne_reif(on[7], on[15], BOOL____00135) :: defines_var(BOOL____00135);\n\
constraint int_ne_reif(on[8], 1, BOOL____00074) :: defines_var(BOOL____00074);\n\
constraint int_ne_reif(on[8], 2, BOOL____00088) :: defines_var(BOOL____00088);\n\
constraint int_ne_reif(on[8], 3, BOOL____00102) :: defines_var(BOOL____00102);\n\
constraint int_ne_reif(on[8], 4, BOOL____00116) :: defines_var(BOOL____00116);\n\
constraint int_ne_reif(on[8], 5, BOOL____00130) :: defines_var(BOOL____00130);\n\
constraint int_ne_reif(on[8], on[11], BOOL____00079) :: defines_var(BOOL____00079);\n\
constraint int_ne_reif(on[8], on[12], BOOL____00093) :: defines_var(BOOL____00093);\n\
constraint int_ne_reif(on[8], on[13], BOOL____00099) :: defines_var(BOOL____00099);\n\
constraint int_ne_reif(on[8], on[14], BOOL____00122) :: defines_var(BOOL____00122);\n\
constraint int_ne_reif(on[8], on[15], BOOL____00136) :: defines_var(BOOL____00136);\n\
constraint int_ne_reif(on[9], 1, BOOL____00075) :: defines_var(BOOL____00075);\n\
constraint int_ne_reif(on[9], 2, BOOL____00089) :: defines_var(BOOL____00089);\n\
constraint int_ne_reif(on[9], 3, BOOL____00103) :: defines_var(BOOL____00103);\n\
constraint int_ne_reif(on[9], 4, BOOL____00117) :: defines_var(BOOL____00117);\n\
constraint int_ne_reif(on[9], 5, BOOL____00131) :: defines_var(BOOL____00131);\n\
constraint int_ne_reif(on[9], on[11], BOOL____00080) :: defines_var(BOOL____00080);\n\
constraint int_ne_reif(on[9], on[12], BOOL____00094) :: defines_var(BOOL____00094);\n\
constraint int_ne_reif(on[9], on[13], BOOL____00108) :: defines_var(BOOL____00108);\n\
constraint int_ne_reif(on[9], on[14], BOOL____00113) :: defines_var(BOOL____00113);\n\
constraint int_ne_reif(on[9], on[15], BOOL____00137) :: defines_var(BOOL____00137);\n\
constraint int_ne_reif(on[10], 1, BOOL____00076) :: defines_var(BOOL____00076);\n\
constraint int_ne_reif(on[10], 2, BOOL____00090) :: defines_var(BOOL____00090);\n\
constraint int_ne_reif(on[10], 3, BOOL____00104) :: defines_var(BOOL____00104);\n\
constraint int_ne_reif(on[10], 4, BOOL____00118) :: defines_var(BOOL____00118);\n\
constraint int_ne_reif(on[10], 5, BOOL____00132) :: defines_var(BOOL____00132);\n\
constraint int_ne_reif(on[10], on[11], BOOL____00081) :: defines_var(BOOL____00081);\n\
constraint int_ne_reif(on[10], on[12], BOOL____00095) :: defines_var(BOOL____00095);\n\
constraint int_ne_reif(on[10], on[13], BOOL____00109) :: defines_var(BOOL____00109);\n\
constraint int_ne_reif(on[10], on[14], BOOL____00123) :: defines_var(BOOL____00123);\n\
constraint int_ne_reif(on[10], on[15], BOOL____00127) :: defines_var(BOOL____00127);\n\
constraint int_ne_reif(on[11], 1, BOOL____00142) :: defines_var(BOOL____00142);\n\
constraint int_ne_reif(on[11], 2, BOOL____00156) :: defines_var(BOOL____00156);\n\
constraint int_ne_reif(on[11], 3, BOOL____00170) :: defines_var(BOOL____00170);\n\
constraint int_ne_reif(on[11], 4, BOOL____00184) :: defines_var(BOOL____00184);\n\
constraint int_ne_reif(on[11], 5, BOOL____00198) :: defines_var(BOOL____00198);\n\
constraint int_ne_reif(on[11], on[16], BOOL____00141) :: defines_var(BOOL____00141);\n\
constraint int_ne_reif(on[11], on[17], BOOL____00162) :: defines_var(BOOL____00162);\n\
constraint int_ne_reif(on[11], on[18], BOOL____00176) :: defines_var(BOOL____00176);\n\
constraint int_ne_reif(on[11], on[19], BOOL____00190) :: defines_var(BOOL____00190);\n\
constraint int_ne_reif(on[11], on[20], BOOL____00204) :: defines_var(BOOL____00204);\n\
constraint int_ne_reif(on[12], 1, BOOL____00143) :: defines_var(BOOL____00143);\n\
constraint int_ne_reif(on[12], 2, BOOL____00157) :: defines_var(BOOL____00157);\n\
constraint int_ne_reif(on[12], 3, BOOL____00171) :: defines_var(BOOL____00171);\n\
constraint int_ne_reif(on[12], 4, BOOL____00185) :: defines_var(BOOL____00185);\n\
constraint int_ne_reif(on[12], 5, BOOL____00199) :: defines_var(BOOL____00199);\n\
constraint int_ne_reif(on[12], on[16], BOOL____00148) :: defines_var(BOOL____00148);\n\
constraint int_ne_reif(on[12], on[17], BOOL____00155) :: defines_var(BOOL____00155);\n\
constraint int_ne_reif(on[12], on[18], BOOL____00177) :: defines_var(BOOL____00177);\n\
constraint int_ne_reif(on[12], on[19], BOOL____00191) :: defines_var(BOOL____00191);\n\
constraint int_ne_reif(on[12], on[20], BOOL____00205) :: defines_var(BOOL____00205);\n\
constraint int_ne_reif(on[13], 1, BOOL____00144) :: defines_var(BOOL____00144);\n\
constraint int_ne_reif(on[13], 2, BOOL____00158) :: defines_var(BOOL____00158);\n\
constraint int_ne_reif(on[13], 3, BOOL____00172) :: defines_var(BOOL____00172);\n\
constraint int_ne_reif(on[13], 4, BOOL____00186) :: defines_var(BOOL____00186);\n\
constraint int_ne_reif(on[13], 5, BOOL____00200) :: defines_var(BOOL____00200);\n\
constraint int_ne_reif(on[13], on[16], BOOL____00149) :: defines_var(BOOL____00149);\n\
constraint int_ne_reif(on[13], on[17], BOOL____00163) :: defines_var(BOOL____00163);\n\
constraint int_ne_reif(on[13], on[18], BOOL____00169) :: defines_var(BOOL____00169);\n\
constraint int_ne_reif(on[13], on[19], BOOL____00192) :: defines_var(BOOL____00192);\n\
constraint int_ne_reif(on[13], on[20], BOOL____00206) :: defines_var(BOOL____00206);\n\
constraint int_ne_reif(on[14], 1, BOOL____00145) :: defines_var(BOOL____00145);\n\
constraint int_ne_reif(on[14], 2, BOOL____00159) :: defines_var(BOOL____00159);\n\
constraint int_ne_reif(on[14], 3, BOOL____00173) :: defines_var(BOOL____00173);\n\
constraint int_ne_reif(on[14], 4, BOOL____00187) :: defines_var(BOOL____00187);\n\
constraint int_ne_reif(on[14], 5, BOOL____00201) :: defines_var(BOOL____00201);\n\
constraint int_ne_reif(on[14], on[16], BOOL____00150) :: defines_var(BOOL____00150);\n\
constraint int_ne_reif(on[14], on[17], BOOL____00164) :: defines_var(BOOL____00164);\n\
constraint int_ne_reif(on[14], on[18], BOOL____00178) :: defines_var(BOOL____00178);\n\
constraint int_ne_reif(on[14], on[19], BOOL____00183) :: defines_var(BOOL____00183);\n\
constraint int_ne_reif(on[14], on[20], BOOL____00207) :: defines_var(BOOL____00207);\n\
constraint int_ne_reif(on[15], 1, BOOL____00146) :: defines_var(BOOL____00146);\n\
constraint int_ne_reif(on[15], 2, BOOL____00160) :: defines_var(BOOL____00160);\n\
constraint int_ne_reif(on[15], 3, BOOL____00174) :: defines_var(BOOL____00174);\n\
constraint int_ne_reif(on[15], 4, BOOL____00188) :: defines_var(BOOL____00188);\n\
constraint int_ne_reif(on[15], 5, BOOL____00202) :: defines_var(BOOL____00202);\n\
constraint int_ne_reif(on[15], on[16], BOOL____00151) :: defines_var(BOOL____00151);\n\
constraint int_ne_reif(on[15], on[17], BOOL____00165) :: defines_var(BOOL____00165);\n\
constraint int_ne_reif(on[15], on[18], BOOL____00179) :: defines_var(BOOL____00179);\n\
constraint int_ne_reif(on[15], on[19], BOOL____00193) :: defines_var(BOOL____00193);\n\
constraint int_ne_reif(on[15], on[20], BOOL____00197) :: defines_var(BOOL____00197);\n\
constraint int_ne_reif(on[16], 1, BOOL____00212) :: defines_var(BOOL____00212);\n\
constraint int_ne_reif(on[16], 2, BOOL____00226) :: defines_var(BOOL____00226);\n\
constraint int_ne_reif(on[16], 3, BOOL____00240) :: defines_var(BOOL____00240);\n\
constraint int_ne_reif(on[16], 4, BOOL____00254) :: defines_var(BOOL____00254);\n\
constraint int_ne_reif(on[16], 5, BOOL____00268) :: defines_var(BOOL____00268);\n\
constraint int_ne_reif(on[16], on[21], BOOL____00211) :: defines_var(BOOL____00211);\n\
constraint int_ne_reif(on[16], on[22], BOOL____00232) :: defines_var(BOOL____00232);\n"+
"constraint int_ne_reif(on[16], on[23], BOOL____00246) :: defines_var(BOOL____00246);\n\
constraint int_ne_reif(on[16], on[24], BOOL____00260) :: defines_var(BOOL____00260);\n\
constraint int_ne_reif(on[16], on[25], BOOL____00274) :: defines_var(BOOL____00274);\n\
constraint int_ne_reif(on[17], 1, BOOL____00213) :: defines_var(BOOL____00213);\n\
constraint int_ne_reif(on[17], 2, BOOL____00227) :: defines_var(BOOL____00227);\n\
constraint int_ne_reif(on[17], 3, BOOL____00241) :: defines_var(BOOL____00241);\n\
constraint int_ne_reif(on[17], 4, BOOL____00255) :: defines_var(BOOL____00255);\n\
constraint int_ne_reif(on[17], 5, BOOL____00269) :: defines_var(BOOL____00269);\n\
constraint int_ne_reif(on[17], on[21], BOOL____00218) :: defines_var(BOOL____00218);\n\
constraint int_ne_reif(on[17], on[22], BOOL____00225) :: defines_var(BOOL____00225);\n\
constraint int_ne_reif(on[17], on[23], BOOL____00247) :: defines_var(BOOL____00247);\n\
constraint int_ne_reif(on[17], on[24], BOOL____00261) :: defines_var(BOOL____00261);\n\
constraint int_ne_reif(on[17], on[25], BOOL____00275) :: defines_var(BOOL____00275);\n\
constraint int_ne_reif(on[18], 1, BOOL____00214) :: defines_var(BOOL____00214);\n\
constraint int_ne_reif(on[18], 2, BOOL____00228) :: defines_var(BOOL____00228);\n\
constraint int_ne_reif(on[18], 3, BOOL____00242) :: defines_var(BOOL____00242);\n\
constraint int_ne_reif(on[18], 4, BOOL____00256) :: defines_var(BOOL____00256);\n\
constraint int_ne_reif(on[18], 5, BOOL____00270) :: defines_var(BOOL____00270);\n\
constraint int_ne_reif(on[18], on[21], BOOL____00219) :: defines_var(BOOL____00219);\n\
constraint int_ne_reif(on[18], on[22], BOOL____00233) :: defines_var(BOOL____00233);\n\
constraint int_ne_reif(on[18], on[23], BOOL____00239) :: defines_var(BOOL____00239);\n\
constraint int_ne_reif(on[18], on[24], BOOL____00262) :: defines_var(BOOL____00262);\n\
constraint int_ne_reif(on[18], on[25], BOOL____00276) :: defines_var(BOOL____00276);\n\
constraint int_ne_reif(on[19], 1, BOOL____00215) :: defines_var(BOOL____00215);\n\
constraint int_ne_reif(on[19], 2, BOOL____00229) :: defines_var(BOOL____00229);\n\
constraint int_ne_reif(on[19], 3, BOOL____00243) :: defines_var(BOOL____00243);\n\
constraint int_ne_reif(on[19], 4, BOOL____00257) :: defines_var(BOOL____00257);\n\
constraint int_ne_reif(on[19], 5, BOOL____00271) :: defines_var(BOOL____00271);\n\
constraint int_ne_reif(on[19], on[21], BOOL____00220) :: defines_var(BOOL____00220);\n\
constraint int_ne_reif(on[19], on[22], BOOL____00234) :: defines_var(BOOL____00234);\n\
constraint int_ne_reif(on[19], on[23], BOOL____00248) :: defines_var(BOOL____00248);\n\
constraint int_ne_reif(on[19], on[24], BOOL____00253) :: defines_var(BOOL____00253);\n\
constraint int_ne_reif(on[19], on[25], BOOL____00277) :: defines_var(BOOL____00277);\n\
constraint int_ne_reif(on[20], 1, BOOL____00216) :: defines_var(BOOL____00216);\n\
constraint int_ne_reif(on[20], 2, BOOL____00230) :: defines_var(BOOL____00230);\n\
constraint int_ne_reif(on[20], 3, BOOL____00244) :: defines_var(BOOL____00244);\n\
constraint int_ne_reif(on[20], 4, BOOL____00258) :: defines_var(BOOL____00258);\n\
constraint int_ne_reif(on[20], 5, BOOL____00272) :: defines_var(BOOL____00272);\n\
constraint int_ne_reif(on[20], on[21], BOOL____00221) :: defines_var(BOOL____00221);\n\
constraint int_ne_reif(on[20], on[22], BOOL____00235) :: defines_var(BOOL____00235);\n\
constraint int_ne_reif(on[20], on[23], BOOL____00249) :: defines_var(BOOL____00249);\n\
constraint int_ne_reif(on[20], on[24], BOOL____00263) :: defines_var(BOOL____00263);\n\
constraint int_ne_reif(on[20], on[25], BOOL____00267) :: defines_var(BOOL____00267);\n\
constraint int_ne_reif(on[21], 1, BOOL____00282) :: defines_var(BOOL____00282);\n\
constraint int_ne_reif(on[21], 2, BOOL____00296) :: defines_var(BOOL____00296);\n\
constraint int_ne_reif(on[21], 3, BOOL____00310) :: defines_var(BOOL____00310);\n\
constraint int_ne_reif(on[21], 4, BOOL____00324) :: defines_var(BOOL____00324);\n\
constraint int_ne_reif(on[21], 5, BOOL____00338) :: defines_var(BOOL____00338);\n\
constraint int_ne_reif(on[21], on[26], BOOL____00281) :: defines_var(BOOL____00281);\n\
constraint int_ne_reif(on[21], on[27], BOOL____00302) :: defines_var(BOOL____00302);\n\
constraint int_ne_reif(on[21], on[28], BOOL____00316) :: defines_var(BOOL____00316);\n\
constraint int_ne_reif(on[21], on[29], BOOL____00330) :: defines_var(BOOL____00330);\n\
constraint int_ne_reif(on[21], on[30], BOOL____00344) :: defines_var(BOOL____00344);\n\
constraint int_ne_reif(on[22], 1, BOOL____00283) :: defines_var(BOOL____00283);\n\
constraint int_ne_reif(on[22], 2, BOOL____00297) :: defines_var(BOOL____00297);\n\
constraint int_ne_reif(on[22], 3, BOOL____00311) :: defines_var(BOOL____00311);\n\
constraint int_ne_reif(on[22], 4, BOOL____00325) :: defines_var(BOOL____00325);\n\
constraint int_ne_reif(on[22], 5, BOOL____00339) :: defines_var(BOOL____00339);\n\
constraint int_ne_reif(on[22], on[26], BOOL____00288) :: defines_var(BOOL____00288);\n\
constraint int_ne_reif(on[22], on[27], BOOL____00295) :: defines_var(BOOL____00295);\n\
constraint int_ne_reif(on[22], on[28], BOOL____00317) :: defines_var(BOOL____00317);\n\
constraint int_ne_reif(on[22], on[29], BOOL____00331) :: defines_var(BOOL____00331);\n\
constraint int_ne_reif(on[22], on[30], BOOL____00345) :: defines_var(BOOL____00345);\n\
constraint int_ne_reif(on[23], 1, BOOL____00284) :: defines_var(BOOL____00284);\n\
constraint int_ne_reif(on[23], 2, BOOL____00298) :: defines_var(BOOL____00298);\n\
constraint int_ne_reif(on[23], 3, BOOL____00312) :: defines_var(BOOL____00312);\n\
constraint int_ne_reif(on[23], 4, BOOL____00326) :: defines_var(BOOL____00326);\n\
constraint int_ne_reif(on[23], 5, BOOL____00340) :: defines_var(BOOL____00340);\n\
constraint int_ne_reif(on[23], on[26], BOOL____00289) :: defines_var(BOOL____00289);\n\
constraint int_ne_reif(on[23], on[27], BOOL____00303) :: defines_var(BOOL____00303);\n\
constraint int_ne_reif(on[23], on[28], BOOL____00309) :: defines_var(BOOL____00309);\n\
constraint int_ne_reif(on[23], on[29], BOOL____00332) :: defines_var(BOOL____00332);\n\
constraint int_ne_reif(on[23], on[30], BOOL____00346) :: defines_var(BOOL____00346);\n\
constraint int_ne_reif(on[24], 1, BOOL____00285) :: defines_var(BOOL____00285);\n\
constraint int_ne_reif(on[24], 2, BOOL____00299) :: defines_var(BOOL____00299);\n\
constraint int_ne_reif(on[24], 3, BOOL____00313) :: defines_var(BOOL____00313);\n\
constraint int_ne_reif(on[24], 4, BOOL____00327) :: defines_var(BOOL____00327);\n\
constraint int_ne_reif(on[24], 5, BOOL____00341) :: defines_var(BOOL____00341);\n\
constraint int_ne_reif(on[24], on[26], BOOL____00290) :: defines_var(BOOL____00290);\n\
constraint int_ne_reif(on[24], on[27], BOOL____00304) :: defines_var(BOOL____00304);\n\
constraint int_ne_reif(on[24], on[28], BOOL____00318) :: defines_var(BOOL____00318);\n\
constraint int_ne_reif(on[24], on[29], BOOL____00323) :: defines_var(BOOL____00323);\n\
constraint int_ne_reif(on[24], on[30], BOOL____00347) :: defines_var(BOOL____00347);\n\
constraint int_ne_reif(on[25], 1, BOOL____00286) :: defines_var(BOOL____00286);\n\
constraint int_ne_reif(on[25], 2, BOOL____00300) :: defines_var(BOOL____00300);\n\
constraint int_ne_reif(on[25], 3, BOOL____00314) :: defines_var(BOOL____00314);\n\
constraint int_ne_reif(on[25], 4, BOOL____00328) :: defines_var(BOOL____00328);\n\
constraint int_ne_reif(on[25], 5, BOOL____00342) :: defines_var(BOOL____00342);\n\
constraint int_ne_reif(on[25], on[26], BOOL____00291) :: defines_var(BOOL____00291);\n\
constraint int_ne_reif(on[25], on[27], BOOL____00305) :: defines_var(BOOL____00305);\n\
constraint int_ne_reif(on[25], on[28], BOOL____00319) :: defines_var(BOOL____00319);\n\
constraint int_ne_reif(on[25], on[29], BOOL____00333) :: defines_var(BOOL____00333);\n\
constraint int_ne_reif(on[25], on[30], BOOL____00337) :: defines_var(BOOL____00337);\n\
constraint set_in(on[1], {-1, 2, 3, 4, 5});\n\
constraint set_in(on[2], {-2, 1, 3, 4, 5});\n\
constraint set_in(on[3], {-3, 1, 2, 4, 5});\n\
constraint set_in(on[4], {-4, 1, 2, 3, 5});\n\
constraint set_in(on[5], {-5, 1, 2, 3, 4});\n\
constraint set_in(on[6], {-1, 2, 3, 4, 5});\n\
constraint set_in(on[7], {-2, 1, 3, 4, 5});\n\
constraint set_in(on[8], {-3, 1, 2, 4, 5});\n\
constraint set_in(on[9], {-4, 1, 2, 3, 5});\n\
constraint set_in(on[10], {-5, 1, 2, 3, 4});\n\
constraint set_in(on[11], {-1, 2, 3, 4, 5});\n\
constraint set_in(on[12], {-2, 1, 3, 4, 5});\n\
constraint set_in(on[13], {-3, 1, 2, 4, 5});\n\
constraint set_in(on[14], {-4, 1, 2, 3, 5});\n\
constraint set_in(on[15], {-5, 1, 2, 3, 4});\n\
constraint set_in(on[16], {-1, 2, 3, 4, 5});\n\
constraint set_in(on[17], {-2, 1, 3, 4, 5});\n\
constraint set_in(on[18], {-3, 1, 2, 4, 5});\n\
constraint set_in(on[19], {-4, 1, 2, 3, 5});\n\
constraint set_in(on[20], {-5, 1, 2, 3, 4});\n\
constraint set_in(on[21], {-1, 2, 3, 4, 5});\n\
constraint set_in(on[22], {-2, 1, 3, 4, 5});\n\
constraint set_in(on[23], {-3, 1, 2, 4, 5});\n\
constraint set_in(on[24], {-4, 1, 2, 3, 5});\n\
constraint set_in(on[25], {-5, 1, 2, 3, 4});\n\
constraint set_in(on[26], {-1, 2, 3, 4, 5});\n\
constraint set_in(on[27], {-2, 1, 3, 4, 5});\n\
constraint set_in(on[28], {-3, 1, 2, 4, 5});\n\
constraint set_in(on[29], {-4, 1, 2, 3, 5});\n\
constraint set_in(on[30], {-5, 1, 2, 3, 4});\n\
solve  :: int_search([on[1], on[2], on[3], on[4], on[5], on[6], on[7], on[8], on[9], on[10], on[11], on[12], on[13], on[14], on[15], on[16], on[17], on[18], on[19], on[20], on[21], on[22], on[23], on[24], on[25], on[26], on[27], on[28], on[29], on[30]], first_fail, indomain_split, complete) satisfy;\n\
", "on = array2d(1..6, 1..5, [-1, 1, 2, 3, 4, -1, 1, 2, 3, -5, -1, 1, 2, -4, -5, -1, 1, -3, -4, -5, -1, -2, -3, -4, -5, 2, 3, 4, 5, -5]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
