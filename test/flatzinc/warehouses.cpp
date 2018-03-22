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
        (void) new FlatZincTest("warehouses::large",
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
array [1..5] of int: capacity = [1, 4, 2, 1, 3];\n\
array [1..50] of int: cost_matrix = [20, 24, 11, 25, 30, 28, 27, 82, 83, 74, 74, 97, 71, 96, 70, 2, 55, 73, 69, 61, 46, 96, 59, 83, 4, 42, 22, 29, 67, 59, 1, 5, 73, 59, 56, 10, 73, 13, 43, 96, 93, 35, 63, 85, 46, 47, 65, 55, 71, 95];\n\
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
var 6..10: INT____00057 :: is_defined_var :: var_is_introduced;\n\
var 11..15: INT____00059 :: is_defined_var :: var_is_introduced;\n\
var 16..20: INT____00061 :: is_defined_var :: var_is_introduced;\n\
var 21..25: INT____00063 :: is_defined_var :: var_is_introduced;\n\
var 26..30: INT____00065 :: is_defined_var :: var_is_introduced;\n\
var 31..35: INT____00067 :: is_defined_var :: var_is_introduced;\n\
var 36..40: INT____00069 :: is_defined_var :: var_is_introduced;\n\
var 41..45: INT____00071 :: is_defined_var :: var_is_introduced;\n\
var 46..50: INT____00073 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00075 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00076 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00077 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00078 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00079 :: is_defined_var :: var_is_introduced;\n\
var 10..1120: INT____00080 :: is_defined_var :: var_is_introduced;\n\
var 10..1120: Total :: output_var = INT____00080;\n\
array [1..10] of var 1..97: cost :: output_array([1..10]);\n\
array [1..5] of var bool: open :: output_array([1..5]);\n\
array [1..10] of var 1..5: supplier :: output_array([1..10]);\n\
var int: use____00081;\n\
var int: use____00082;\n\
var int: use____00083;\n\
var int: use____00084;\n\
var int: use____00085;\n\
constraint array_bool_or([BOOL____00010, BOOL____00009, BOOL____00008, BOOL____00007, BOOL____00006, BOOL____00005, BOOL____00004, BOOL____00003, BOOL____00002, BOOL____00001], open[1]);\n\
constraint array_bool_or([BOOL____00021, BOOL____00020, BOOL____00019, BOOL____00018, BOOL____00017, BOOL____00016, BOOL____00015, BOOL____00014, BOOL____00013, BOOL____00012], open[2]);\n\
constraint array_bool_or([BOOL____00032, BOOL____00031, BOOL____00030, BOOL____00029, BOOL____00028, BOOL____00027, BOOL____00026, BOOL____00025, BOOL____00024, BOOL____00023], open[3]);\n\
constraint array_bool_or([BOOL____00043, BOOL____00042, BOOL____00041, BOOL____00040, BOOL____00039, BOOL____00038, BOOL____00037, BOOL____00036, BOOL____00035, BOOL____00034], open[4]);\n\
constraint array_bool_or([BOOL____00054, BOOL____00053, BOOL____00052, BOOL____00051, BOOL____00050, BOOL____00049, BOOL____00048, BOOL____00047, BOOL____00046, BOOL____00045], open[5]);\n\
constraint array_int_element(INT____00057, cost_matrix, cost[2]);\n\
constraint array_int_element(INT____00059, cost_matrix, cost[3]);\n\
constraint array_int_element(INT____00061, cost_matrix, cost[4]);\n\
constraint array_int_element(INT____00063, cost_matrix, cost[5]);\n\
constraint array_int_element(INT____00065, cost_matrix, cost[6]);\n\
constraint array_int_element(INT____00067, cost_matrix, cost[7]);\n\
constraint array_int_element(INT____00069, cost_matrix, cost[8]);\n\
constraint array_int_element(INT____00071, cost_matrix, cost[9]);\n\
constraint array_int_element(INT____00073, cost_matrix, cost[10]);\n\
constraint array_int_element(supplier[1], cost_matrix, cost[1]);\n\
constraint bool2int(open[1], INT____00075) :: defines_var(INT____00075);\n\
constraint bool2int(open[2], INT____00076) :: defines_var(INT____00076);\n\
constraint bool2int(open[3], INT____00077) :: defines_var(INT____00077);\n\
constraint bool2int(open[4], INT____00078) :: defines_var(INT____00078);\n\
constraint bool2int(open[5], INT____00079) :: defines_var(INT____00079);\n\
constraint count(supplier, 1, use____00081);\n\
constraint count(supplier, 2, use____00082);\n\
constraint count(supplier, 3, use____00083);\n\
constraint count(supplier, 4, use____00084);\n\
constraint count(supplier, 5, use____00085);\n\
constraint int_eq_reif(1, supplier[1], BOOL____00001) :: defines_var(BOOL____00001);\n\
constraint int_eq_reif(1, supplier[2], BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_eq_reif(1, supplier[3], BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_eq_reif(1, supplier[4], BOOL____00004) :: defines_var(BOOL____00004);\n\
constraint int_eq_reif(1, supplier[5], BOOL____00005) :: defines_var(BOOL____00005);\n\
constraint int_eq_reif(1, supplier[6], BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_eq_reif(1, supplier[7], BOOL____00007) :: defines_var(BOOL____00007);\n\
constraint int_eq_reif(1, supplier[8], BOOL____00008) :: defines_var(BOOL____00008);\n\
constraint int_eq_reif(1, supplier[9], BOOL____00009) :: defines_var(BOOL____00009);\n\
constraint int_eq_reif(1, supplier[10], BOOL____00010) :: defines_var(BOOL____00010);\n\
constraint int_eq_reif(2, supplier[1], BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint int_eq_reif(2, supplier[2], BOOL____00013) :: defines_var(BOOL____00013);\n\
constraint int_eq_reif(2, supplier[3], BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint int_eq_reif(2, supplier[4], BOOL____00015) :: defines_var(BOOL____00015);\n\
constraint int_eq_reif(2, supplier[5], BOOL____00016) :: defines_var(BOOL____00016);\n\
constraint int_eq_reif(2, supplier[6], BOOL____00017) :: defines_var(BOOL____00017);\n\
constraint int_eq_reif(2, supplier[7], BOOL____00018) :: defines_var(BOOL____00018);\n\
constraint int_eq_reif(2, supplier[8], BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint int_eq_reif(2, supplier[9], BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_eq_reif(2, supplier[10], BOOL____00021) :: defines_var(BOOL____00021);\n\
constraint int_eq_reif(3, supplier[1], BOOL____00023) :: defines_var(BOOL____00023);\n\
constraint int_eq_reif(3, supplier[2], BOOL____00024) :: defines_var(BOOL____00024);\n\
constraint int_eq_reif(3, supplier[3], BOOL____00025) :: defines_var(BOOL____00025);\n\
constraint int_eq_reif(3, supplier[4], BOOL____00026) :: defines_var(BOOL____00026);\n\
constraint int_eq_reif(3, supplier[5], BOOL____00027) :: defines_var(BOOL____00027);\n\
constraint int_eq_reif(3, supplier[6], BOOL____00028) :: defines_var(BOOL____00028);\n\
constraint int_eq_reif(3, supplier[7], BOOL____00029) :: defines_var(BOOL____00029);\n\
constraint int_eq_reif(3, supplier[8], BOOL____00030) :: defines_var(BOOL____00030);\n\
constraint int_eq_reif(3, supplier[9], BOOL____00031) :: defines_var(BOOL____00031);\n\
constraint int_eq_reif(3, supplier[10], BOOL____00032) :: defines_var(BOOL____00032);\n\
constraint int_eq_reif(4, supplier[1], BOOL____00034) :: defines_var(BOOL____00034);\n\
constraint int_eq_reif(4, supplier[2], BOOL____00035) :: defines_var(BOOL____00035);\n\
constraint int_eq_reif(4, supplier[3], BOOL____00036) :: defines_var(BOOL____00036);\n\
constraint int_eq_reif(4, supplier[4], BOOL____00037) :: defines_var(BOOL____00037);\n\
constraint int_eq_reif(4, supplier[5], BOOL____00038) :: defines_var(BOOL____00038);\n\
constraint int_eq_reif(4, supplier[6], BOOL____00039) :: defines_var(BOOL____00039);\n\
constraint int_eq_reif(4, supplier[7], BOOL____00040) :: defines_var(BOOL____00040);\n\
constraint int_eq_reif(4, supplier[8], BOOL____00041) :: defines_var(BOOL____00041);\n\
constraint int_eq_reif(4, supplier[9], BOOL____00042) :: defines_var(BOOL____00042);\n\
constraint int_eq_reif(4, supplier[10], BOOL____00043) :: defines_var(BOOL____00043);\n\
constraint int_eq_reif(5, supplier[1], BOOL____00045) :: defines_var(BOOL____00045);\n\
constraint int_eq_reif(5, supplier[2], BOOL____00046) :: defines_var(BOOL____00046);\n\
constraint int_eq_reif(5, supplier[3], BOOL____00047) :: defines_var(BOOL____00047);\n\
constraint int_eq_reif(5, supplier[4], BOOL____00048) :: defines_var(BOOL____00048);\n\
constraint int_eq_reif(5, supplier[5], BOOL____00049) :: defines_var(BOOL____00049);\n\
constraint int_eq_reif(5, supplier[6], BOOL____00050) :: defines_var(BOOL____00050);\n\
constraint int_eq_reif(5, supplier[7], BOOL____00051) :: defines_var(BOOL____00051);\n\
constraint int_eq_reif(5, supplier[8], BOOL____00052) :: defines_var(BOOL____00052);\n\
constraint int_eq_reif(5, supplier[9], BOOL____00053) :: defines_var(BOOL____00053);\n\
constraint int_eq_reif(5, supplier[10], BOOL____00054) :: defines_var(BOOL____00054);\n")+"\
constraint int_le(use____00081, 1);\n\
constraint int_le(use____00082, 4);\n\
constraint int_le(use____00083, 2);\n\
constraint int_le(use____00084, 1);\n\
constraint int_le(use____00085, 3);\n\
constraint int_lin_eq([-1, 1], [INT____00057, supplier[2]], -5) :: defines_var(INT____00057) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00059, supplier[3]], -10) :: defines_var(INT____00059) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00061, supplier[4]], -15) :: defines_var(INT____00061) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00063, supplier[5]], -20) :: defines_var(INT____00063) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00065, supplier[6]], -25) :: defines_var(INT____00065) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00067, supplier[7]], -30) :: defines_var(INT____00067) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00069, supplier[8]], -35) :: defines_var(INT____00069) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00071, supplier[9]], -40) :: defines_var(INT____00071) :: domain;\n\
constraint int_lin_eq([-1, 1], [INT____00073, supplier[10]], -45) :: defines_var(INT____00073) :: domain;\n\
constraint int_lin_eq([-1, 30, 30, 30, 30, 30, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [INT____00080, INT____00075, INT____00076, INT____00077, INT____00078, INT____00079, cost[1], cost[2], cost[3], cost[4], cost[5], cost[6], cost[7], cost[8], cost[9], cost[10]], 0) :: defines_var(INT____00080);\n\
solve  :: int_search([supplier[1], supplier[2], supplier[3], supplier[4], supplier[5], supplier[6], supplier[7], supplier[8], supplier[9], supplier[10], cost[1], cost[2], cost[3], cost[4], cost[5], cost[6], cost[7], cost[8], cost[9], cost[10], INT____00075, INT____00076, INT____00077, INT____00078, INT____00079], first_fail, indomain_split, complete) minimize INT____00080;\n\
", "Total = 383;\n\
cost = array1d(1..10, [30, 27, 70, 2, 4, 22, 5, 13, 35, 55]);\n\
open = array1d(1..5, [true, true, true, false, true]);\n\
supplier = array1d(1..10, [5, 2, 5, 1, 5, 2, 2, 3, 2, 3]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
