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
        (void) new FlatZincTest("photo",
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
array [1..34] of int: prefs = [0, 2, 0, 4, 0, 7, 1, 4, 1, 8, 2, 3, 2, 4, 3, 0, 3, 4, 4, 5, 4, 0, 5, 0, 5, 8, 6, 2, 6, 7, 7, 8, 7, 6];\n\
var bool: BOOL____00019 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00020 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00022 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00023 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00025 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00026 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00028 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00029 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00031 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00032 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00034 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00035 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00037 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00038 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00040 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00041 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00043 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00044 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00046 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00047 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00050 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00051 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00053 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00054 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00056 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00057 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00059 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00060 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00062 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00063 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00001 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00002 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00003 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00004 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00005 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00006 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00007 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00008 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00009 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00010 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00011 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00012 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00013 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00014 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00015 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00016 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00017 :: is_defined_var :: var_is_introduced;\n\
var 0..8: INT____00018 :: is_defined_var :: var_is_introduced;\n\
array [1..17] of var bool: ful;\n\
array [1..9] of var 0..8: pos :: output_array([0..8]);\n\
var 0..8: satisfies :: output_var = INT____00018;\n\
constraint all_different_int(pos);\n\
constraint bool2int(ful[1], INT____00001) :: defines_var(INT____00001);\n\
constraint bool2int(ful[2], INT____00002) :: defines_var(INT____00002);\n\
constraint bool2int(ful[3], INT____00003) :: defines_var(INT____00003);\n\
constraint bool2int(ful[4], INT____00004) :: defines_var(INT____00004);\n\
constraint bool2int(ful[5], INT____00005) :: defines_var(INT____00005);\n\
constraint bool2int(ful[6], INT____00006) :: defines_var(INT____00006);\n\
constraint bool2int(ful[7], INT____00007) :: defines_var(INT____00007);\n\
constraint bool2int(ful[8], INT____00008) :: defines_var(INT____00008);\n\
constraint bool2int(ful[9], INT____00009) :: defines_var(INT____00009);\n\
constraint bool2int(ful[10], INT____00010) :: defines_var(INT____00010);\n\
constraint bool2int(ful[11], INT____00011) :: defines_var(INT____00011);\n\
constraint bool2int(ful[12], INT____00012) :: defines_var(INT____00012);\n\
constraint bool2int(ful[13], INT____00013) :: defines_var(INT____00013);\n\
constraint bool2int(ful[14], INT____00014) :: defines_var(INT____00014);\n\
constraint bool2int(ful[15], INT____00015) :: defines_var(INT____00015);\n\
constraint bool2int(ful[16], INT____00016) :: defines_var(INT____00016);\n\
constraint bool2int(ful[17], INT____00017) :: defines_var(INT____00017);\n\
constraint bool_xor(BOOL____00019, BOOL____00020, ful[1]);\n\
constraint bool_xor(BOOL____00022, BOOL____00023, ful[2]);\n\
constraint bool_xor(BOOL____00023, BOOL____00022, ful[11]);\n\
constraint bool_xor(BOOL____00025, BOOL____00026, ful[3]);\n\
constraint bool_xor(BOOL____00028, BOOL____00029, ful[4]);\n\
constraint bool_xor(BOOL____00031, BOOL____00032, ful[5]);\n\
constraint bool_xor(BOOL____00034, BOOL____00035, ful[6]);\n\
constraint bool_xor(BOOL____00037, BOOL____00038, ful[7]);\n\
constraint bool_xor(BOOL____00040, BOOL____00041, ful[8]);\n\
constraint bool_xor(BOOL____00043, BOOL____00044, ful[9]);\n\
constraint bool_xor(BOOL____00046, BOOL____00047, ful[10]);\n\
constraint bool_xor(BOOL____00050, BOOL____00051, ful[12]);\n\
constraint bool_xor(BOOL____00053, BOOL____00054, ful[13]);\n\
constraint bool_xor(BOOL____00056, BOOL____00057, ful[14]);\n\
constraint bool_xor(BOOL____00059, BOOL____00060, ful[15]);\n\
constraint bool_xor(BOOL____00060, BOOL____00059, ful[17]);\n\
constraint bool_xor(BOOL____00062, BOOL____00063, ful[16]);\n\
constraint int_lin_eq([-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [INT____00018, INT____00001, INT____00002, INT____00003, INT____00004, INT____00005, INT____00006, INT____00007, INT____00008, INT____00009, INT____00010, INT____00011, INT____00012, INT____00013, INT____00014, INT____00015, INT____00016, INT____00017], 0) :: defines_var(INT____00018);\n\
constraint int_lin_eq_reif([-1, 1], [pos[1], pos[3]], -1, BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_lin_eq_reif([-1, 1], [pos[1], pos[4]], -1, BOOL____00040) :: defines_var(BOOL____00040);\n\
constraint int_lin_eq_reif([-1, 1], [pos[1], pos[5]], -1, BOOL____00023) :: defines_var(BOOL____00023);\n\
constraint int_lin_eq_reif([-1, 1], [pos[1], pos[6]], -1, BOOL____00050) :: defines_var(BOOL____00050);\n\
constraint int_lin_eq_reif([-1, 1], [pos[1], pos[8]], -1, BOOL____00026) :: defines_var(BOOL____00026);\n\
constraint int_lin_eq_reif([-1, 1], [pos[2], pos[5]], -1, BOOL____00029) :: defines_var(BOOL____00029);\n\
constraint int_lin_eq_reif([-1, 1], [pos[2], pos[9]], -1, BOOL____00032) :: defines_var(BOOL____00032);\n\
constraint int_lin_eq_reif([-1, 1], [pos[3], pos[4]], -1, BOOL____00035) :: defines_var(BOOL____00035);\n\
constraint int_lin_eq_reif([-1, 1], [pos[3], pos[5]], -1, BOOL____00038) :: defines_var(BOOL____00038);\n\
constraint int_lin_eq_reif([-1, 1], [pos[3], pos[7]], -1, BOOL____00056) :: defines_var(BOOL____00056);\n\
constraint int_lin_eq_reif([-1, 1], [pos[4], pos[5]], -1, BOOL____00044) :: defines_var(BOOL____00044);\n\
constraint int_lin_eq_reif([-1, 1], [pos[5], pos[6]], -1, BOOL____00047) :: defines_var(BOOL____00047);\n\
constraint int_lin_eq_reif([-1, 1], [pos[6], pos[9]], -1, BOOL____00054) :: defines_var(BOOL____00054);\n\
constraint int_lin_eq_reif([-1, 1], [pos[7], pos[8]], -1, BOOL____00060) :: defines_var(BOOL____00060);\n\
constraint int_lin_eq_reif([-1, 1], [pos[8], pos[9]], -1, BOOL____00063) :: defines_var(BOOL____00063);\n\
constraint int_lin_eq_reif([1, -1], [pos[1], pos[3]], -1, BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint int_lin_eq_reif([1, -1], [pos[1], pos[4]], -1, BOOL____00041) :: defines_var(BOOL____00041);\n\
constraint int_lin_eq_reif([1, -1], [pos[1], pos[5]], -1, BOOL____00022) :: defines_var(BOOL____00022);\n\
constraint int_lin_eq_reif([1, -1], [pos[1], pos[6]], -1, BOOL____00051) :: defines_var(BOOL____00051);\n\
constraint int_lin_eq_reif([1, -1], [pos[1], pos[8]], -1, BOOL____00025) :: defines_var(BOOL____00025);\n\
constraint int_lin_eq_reif([1, -1], [pos[2], pos[5]], -1, BOOL____00028) :: defines_var(BOOL____00028);\n\
constraint int_lin_eq_reif([1, -1], [pos[2], pos[9]], -1, BOOL____00031) :: defines_var(BOOL____00031);\n\
constraint int_lin_eq_reif([1, -1], [pos[3], pos[4]], -1, BOOL____00034) :: defines_var(BOOL____00034);\n\
constraint int_lin_eq_reif([1, -1], [pos[3], pos[5]], -1, BOOL____00037) :: defines_var(BOOL____00037);\n\
constraint int_lin_eq_reif([1, -1], [pos[3], pos[7]], -1, BOOL____00057) :: defines_var(BOOL____00057);\n\
constraint int_lin_eq_reif([1, -1], [pos[4], pos[5]], -1, BOOL____00043) :: defines_var(BOOL____00043);\n\
constraint int_lin_eq_reif([1, -1], [pos[5], pos[6]], -1, BOOL____00046) :: defines_var(BOOL____00046);\n\
constraint int_lin_eq_reif([1, -1], [pos[6], pos[9]], -1, BOOL____00053) :: defines_var(BOOL____00053);\n\
constraint int_lin_eq_reif([1, -1], [pos[7], pos[8]], -1, BOOL____00059) :: defines_var(BOOL____00059);\n\
constraint int_lin_eq_reif([1, -1], [pos[8], pos[9]], -1, BOOL____00062) :: defines_var(BOOL____00062);\n\
constraint int_lt(pos[1], pos[2]);\n\
solve  :: int_search(pos, first_fail, indomain, complete) maximize INT____00018;\n\
", "pos = array1d(0..8, [0, 1, 4, 3, 2, 8, 5, 6, 7]);\n\
satisfies = 8;\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
