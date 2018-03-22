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
        (void) new FlatZincTest("template_design",
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
array [1..7] of int: d = [250, 255, 260, 500, 500, 800, 1100];\n\
var 204..408: INT____00002 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00003 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00004 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00005 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00006 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00007 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00008 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00009 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00010 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00011 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00012 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00013 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00014 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00015 :: is_defined_var :: var_is_introduced;\n\
var 0..7344: INT____00016 :: is_defined_var :: var_is_introduced;\n\
var 408..816: INT____00017 :: is_defined_var :: var_is_introduced;\n\
var 408..816: Production :: output_var = INT____00017;\n\
array [1..2] of var 1..816: R :: output_array([1..2]);\n\
var 7..408: Surplus;\n\
array [1..14] of var 0..9: p :: output_array([1..7, 1..2]);\n\
constraint array_int_lq([p[7], p[8]], [p[9], p[10]]);\n\
constraint int_div(Production, 2, INT____00002) :: defines_var(INT____00002);\n\
constraint int_le(INT____00002, R[2]);\n\
constraint int_le(R[1], INT____00002);\n\
constraint int_lin_eq([-1, 9], [Surplus, Production], 3665);\n\
constraint int_lin_eq([1, 1, 1, 1, 1, 1, 1], [p[1], p[3], p[5], p[7], p[9], p[11], p[13]], 9);\n\
constraint int_lin_eq([1, 1, 1, 1, 1, 1, 1], [p[2], p[4], p[6], p[8], p[10], p[12], p[14]], 9);\n\
constraint int_lin_le([-1, -1], [INT____00003, INT____00005], -250);\n\
constraint int_lin_le([-1, -1], [INT____00004, INT____00006], -255);\n\
constraint int_lin_le([-1, -1], [INT____00007, INT____00008], -260);\n\
constraint int_lin_le([-1, -1], [INT____00009, INT____00010], -500);\n\
constraint int_lin_le([-1, -1], [INT____00011, INT____00012], -500);\n\
constraint int_lin_le([-1, -1], [INT____00013, INT____00014], -800);\n\
constraint int_lin_le([-1, -1], [INT____00015, INT____00016], -1100);\n\
constraint int_lin_le([-1, 1, 1], [Surplus, INT____00003, INT____00005], 500);\n\
constraint int_lin_le([-1, 1, 1], [Surplus, INT____00004, INT____00006], 510);\n\
constraint int_lin_le([-1, 1, 1], [Surplus, INT____00007, INT____00008], 520);\n\
constraint int_lin_le([-1, 1, 1], [Surplus, INT____00009, INT____00010], 1000);\n\
constraint int_lin_le([-1, 1, 1], [Surplus, INT____00011, INT____00012], 1000);\n\
constraint int_lin_le([-1, 1, 1], [Surplus, INT____00013, INT____00014], 1600);\n\
constraint int_lin_le([-1, 1, 1], [Surplus, INT____00015, INT____00016], 2200);\n\
constraint int_lin_le([1, -1, 1, -1], [INT____00003, INT____00004, INT____00005, INT____00006], -1);\n\
constraint int_lin_le([1, 1, -1, -1], [INT____00004, INT____00006, INT____00007, INT____00008], -1);\n\
constraint int_lin_le([1, 1, -1, -1], [INT____00007, INT____00008, INT____00009, INT____00010], -1);\n\
constraint int_lin_le([1, 1, -1, -1], [INT____00011, INT____00012, INT____00013, INT____00014], -1);\n\
constraint int_lin_le([1, 1, -1, -1], [INT____00013, INT____00014, INT____00015, INT____00016], -1);\n\
constraint int_lin_le([-1, 1, 1, 1, 1], [Surplus, INT____00003, INT____00004, INT____00005, INT____00006], 1010);\n\
constraint int_lin_le([-1, 1, 1, 1, 1, 1, 1], [Surplus, INT____00003, INT____00004, INT____00005, INT____00006, INT____00007, INT____00008], 1530);\n\
constraint int_lin_le([-1, 1, 1, 1, 1, 1, 1, 1, 1], [Surplus, INT____00003, INT____00004, INT____00005, INT____00006, INT____00007, INT____00008, INT____00009, INT____00010], 2530);\n\
constraint int_lin_le([-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [Surplus, INT____00003, INT____00004, INT____00005, INT____00006, INT____00007, INT____00008, INT____00009, INT____00010, INT____00011, INT____00012], 3530);\n\
constraint int_lin_le([-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [Surplus, INT____00003, INT____00004, INT____00005, INT____00006, INT____00007, INT____00008, INT____00009, INT____00010, INT____00011, INT____00012, INT____00013, INT____00014], 5130);\n\
constraint int_plus(R[1], R[2], INT____00017) :: defines_var(INT____00017);\n\
constraint int_times(p[1], R[1], INT____00003) :: defines_var(INT____00003);\n\
constraint int_times(p[2], R[2], INT____00005) :: defines_var(INT____00005);\n\
constraint int_times(p[3], R[1], INT____00004) :: defines_var(INT____00004);\n\
constraint int_times(p[4], R[2], INT____00006) :: defines_var(INT____00006);\n\
constraint int_times(p[5], R[1], INT____00007) :: defines_var(INT____00007);\n\
constraint int_times(p[6], R[2], INT____00008) :: defines_var(INT____00008);\n\
constraint int_times(p[7], R[1], INT____00009) :: defines_var(INT____00009);\n\
constraint int_times(p[8], R[2], INT____00010) :: defines_var(INT____00010);\n\
constraint int_times(p[9], R[1], INT____00011) :: defines_var(INT____00011);\n\
constraint int_times(p[10], R[2], INT____00012) :: defines_var(INT____00012);\n\
constraint int_times(p[11], R[1], INT____00013) :: defines_var(INT____00013);\n\
constraint int_times(p[12], R[2], INT____00014) :: defines_var(INT____00014);\n\
constraint int_times(p[13], R[1], INT____00015) :: defines_var(INT____00015);\n\
constraint int_times(p[14], R[2], INT____00016) :: defines_var(INT____00016);\n\
solve  :: int_search([p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], R[1], R[2]], input_order, indomain_min, complete) minimize INT____00017;\n\
", "Production = 442;\n\
R = array1d(1..2, [167, 275]);\n\
p = array2d(1..7, 1..2, [0, 1, 2, 0, 1, 1, 3, 0, 3, 0, 0, 3, 0, 4]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
