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
        (void) new FlatZincTest("knights",
"predicate all_different_int(array [int] of var int: x);\n\
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
array [1..36] of set of int: neighbours = [{9, 14}, {10, 13, 15}, {7, 11, 14, 16}, {8, 12, 15, 17}, {9, 16, 18}, {10, 17}, {3, 15, 20}, {4, 16, 19, 21}, {1, 5, 13, 17, 20, 22}, {2, 6, 14, 18, 21, 23}, {3, 15, 22, 24}, {4, 16, 23}, {2, 9, 21, 26}, {1, 3, 10, 22, 25, 27}, {2, 4, 7, 11, 19, 23, 26, 28}, {3, 5, 8, 12, 20, 24, 27, 29}, {4, 6, 9, 21, 28, 30}, {5, 10, 22, 29}, {8, 15, 27, 32}, {7, 9, 16, 28, 31, 33}, {8, 10, 13, 17, 25, 29, 32, 34}, {9, 11, 14, 18, 26, 30, 33, 35}, {10, 12, 15, 27, 34, 36}, {11, 16, 28, 35}, {14, 21, 33}, {13, 15, 22, 34}, {14, 16, 19, 23, 31, 35}, {15, 17, 20, 24, 32, 36}, {16, 18, 21, 33}, {17, 22, 34}, {20, 27}, {19, 21, 28}, {20, 22, 25, 29}, {21, 23, 26, 30}, {22, 24, 27}, {23, 28}];\n\
var set of 1..36: SET____00001 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00002 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00003 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00004 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00005 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00006 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00007 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00008 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00009 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00010 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00011 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00012 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00013 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00014 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00015 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00016 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00017 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00018 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00019 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00020 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00021 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00022 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00023 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00024 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00025 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00026 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00027 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00028 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00029 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00030 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00031 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00032 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00033 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00034 :: is_defined_var :: var_is_introduced;\n\
var set of 1..36: SET____00035 :: is_defined_var :: var_is_introduced;\n\
array [1..36] of var 1..36: p :: output_array([1..36]);\n\
constraint all_different_int(p);\n\
constraint array_set_element(p[1], neighbours, SET____00001) :: defines_var(SET____00001);\n\
constraint array_set_element(p[2], neighbours, SET____00002) :: defines_var(SET____00002);\n\
constraint array_set_element(p[3], neighbours, SET____00003) :: defines_var(SET____00003);\n\
constraint array_set_element(p[4], neighbours, SET____00004) :: defines_var(SET____00004);\n\
constraint array_set_element(p[5], neighbours, SET____00005) :: defines_var(SET____00005);\n\
constraint array_set_element(p[6], neighbours, SET____00006) :: defines_var(SET____00006);\n\
constraint array_set_element(p[7], neighbours, SET____00007) :: defines_var(SET____00007);\n\
constraint array_set_element(p[8], neighbours, SET____00008) :: defines_var(SET____00008);\n\
constraint array_set_element(p[9], neighbours, SET____00009) :: defines_var(SET____00009);\n\
constraint array_set_element(p[10], neighbours, SET____00010) :: defines_var(SET____00010);\n\
constraint array_set_element(p[11], neighbours, SET____00011) :: defines_var(SET____00011);\n\
constraint array_set_element(p[12], neighbours, SET____00012) :: defines_var(SET____00012);\n\
constraint array_set_element(p[13], neighbours, SET____00013) :: defines_var(SET____00013);\n\
constraint array_set_element(p[14], neighbours, SET____00014) :: defines_var(SET____00014);\n\
constraint array_set_element(p[15], neighbours, SET____00015) :: defines_var(SET____00015);\n\
constraint array_set_element(p[16], neighbours, SET____00016) :: defines_var(SET____00016);\n\
constraint array_set_element(p[17], neighbours, SET____00017) :: defines_var(SET____00017);\n\
constraint array_set_element(p[18], neighbours, SET____00018) :: defines_var(SET____00018);\n\
constraint array_set_element(p[19], neighbours, SET____00019) :: defines_var(SET____00019);\n\
constraint array_set_element(p[20], neighbours, SET____00020) :: defines_var(SET____00020);\n\
constraint array_set_element(p[21], neighbours, SET____00021) :: defines_var(SET____00021);\n\
constraint array_set_element(p[22], neighbours, SET____00022) :: defines_var(SET____00022);\n\
constraint array_set_element(p[23], neighbours, SET____00023) :: defines_var(SET____00023);\n\
constraint array_set_element(p[24], neighbours, SET____00024) :: defines_var(SET____00024);\n\
constraint array_set_element(p[25], neighbours, SET____00025) :: defines_var(SET____00025);\n\
constraint array_set_element(p[26], neighbours, SET____00026) :: defines_var(SET____00026);\n\
constraint array_set_element(p[27], neighbours, SET____00027) :: defines_var(SET____00027);\n\
constraint array_set_element(p[28], neighbours, SET____00028) :: defines_var(SET____00028);\n\
constraint array_set_element(p[29], neighbours, SET____00029) :: defines_var(SET____00029);\n\
constraint array_set_element(p[30], neighbours, SET____00030) :: defines_var(SET____00030);\n\
constraint array_set_element(p[31], neighbours, SET____00031) :: defines_var(SET____00031);\n\
constraint array_set_element(p[32], neighbours, SET____00032) :: defines_var(SET____00032);\n\
constraint array_set_element(p[33], neighbours, SET____00033) :: defines_var(SET____00033);\n\
constraint array_set_element(p[34], neighbours, SET____00034) :: defines_var(SET____00034);\n\
constraint array_set_element(p[35], neighbours, SET____00035) :: defines_var(SET____00035);\n\
constraint int_eq(p[1], 1);\n\
constraint int_eq(p[2], 9);\n\
constraint int_eq(p[36], 14);\n\
constraint set_in(p[2], SET____00001);\n\
constraint set_in(p[3], SET____00002);\n\
constraint set_in(p[4], SET____00003);\n\
constraint set_in(p[5], SET____00004);\n\
constraint set_in(p[6], SET____00005);\n\
constraint set_in(p[7], SET____00006);\n\
constraint set_in(p[8], SET____00007);\n\
constraint set_in(p[9], SET____00008);\n\
constraint set_in(p[10], SET____00009);\n\
constraint set_in(p[11], SET____00010);\n\
constraint set_in(p[12], SET____00011);\n\
constraint set_in(p[13], SET____00012);\n\
constraint set_in(p[14], SET____00013);\n\
constraint set_in(p[15], SET____00014);\n\
constraint set_in(p[16], SET____00015);\n\
constraint set_in(p[17], SET____00016);\n\
constraint set_in(p[18], SET____00017);\n\
constraint set_in(p[19], SET____00018);\n\
constraint set_in(p[20], SET____00019);\n\
constraint set_in(p[21], SET____00020);\n\
constraint set_in(p[22], SET____00021);\n\
constraint set_in(p[23], SET____00022);\n\
constraint set_in(p[24], SET____00023);\n\
constraint set_in(p[25], SET____00024);\n\
constraint set_in(p[26], SET____00025);\n\
constraint set_in(p[27], SET____00026);\n\
constraint set_in(p[28], SET____00027);\n\
constraint set_in(p[29], SET____00028);\n\
constraint set_in(p[30], SET____00029);\n\
constraint set_in(p[31], SET____00030);\n\
constraint set_in(p[32], SET____00031);\n\
constraint set_in(p[33], SET____00032);\n\
constraint set_in(p[34], SET____00033);\n\
constraint set_in(p[35], SET____00034);\n\
constraint set_in(p[36], SET____00035);\n\
solve  :: int_search(p, input_order, indomain_min, complete) satisfy;\n\
", "p = array1d(1..36, [1, 9, 5, 16, 3, 7, 15, 2, 10, 6, 17, 30, 34, 26, 13, 21, 32, 19, 8, 4, 12, 23, 36, 28, 20, 31, 27, 35, 24, 11, 22, 18, 29, 33, 25, 14]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
