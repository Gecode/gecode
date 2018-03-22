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
        (void) new FlatZincTest("alpha",
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
var 1..26: a :: output_var;\n\
var 1..26: b :: output_var;\n\
var 1..26: c :: output_var;\n\
var 1..26: d :: output_var;\n\
var 1..26: e :: output_var;\n\
var 1..26: f :: output_var;\n\
var 1..26: g :: output_var;\n\
var 1..26: h :: output_var;\n\
var 1..26: i :: output_var;\n\
var 1..26: j :: output_var;\n\
var 1..26: k :: output_var;\n\
var 1..26: l :: output_var;\n\
var 1..26: m :: output_var;\n\
var 1..26: n :: output_var;\n\
var 1..26: o :: output_var;\n\
var 1..26: p :: output_var;\n\
var 1..26: q :: output_var;\n\
var 1..26: r :: output_var;\n\
var 1..26: s :: output_var;\n\
var 1..26: t :: output_var;\n\
var 1..26: u :: output_var;\n\
var 1..26: v :: output_var;\n\
var 1..26: w :: output_var;\n\
var 1..26: x :: output_var;\n\
var 1..26: y :: output_var;\n\
var 1..26: z :: output_var;\n\
constraint all_different_int([a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z]);\n\
constraint int_lin_eq([1, 1, 2], [a, j, z], 58);\n\
constraint int_lin_eq([1, 1, 2], [b, e, o], 53);\n\
constraint int_lin_eq([1, 2, 1], [l, o, s], 37);\n\
constraint int_lin_eq([2, 1, 1], [e, g, l], 66);\n\
constraint int_lin_eq([1, 1, 1, 1], [e, l, r, y], 47);\n\
constraint int_lin_eq([1, 1, 1, 1], [g, n, o, s], 61);\n\
constraint int_lin_eq([1, 1, 1, 2], [e, f, g, u], 50);\n\
constraint int_lin_eq([1, 1, 2, 1], [c, e, l, o], 43);\n\
constraint int_lin_eq([2, 1, 1, 1], [e, h, m, t], 72);\n\
constraint int_lin_eq([1, 1, 1, 1, 1], [a, c, e, l, s], 51);\n\
constraint int_lin_eq([1, 1, 1, 1, 1], [a, e, o, p, r], 65);\n\
constraint int_lin_eq([1, 1, 1, 1, 1], [a, k, l, o, p], 59);\n\
constraint int_lin_eq([1, 1, 1, 1, 1], [a, l, t, w, z], 34);\n\
constraint int_lin_eq([1, 1, 1, 1, 1], [e, f, l, t, u], 30);\n\
constraint int_lin_eq([1, 1, 1, 2, 1], [a, b, e, l, t], 45);\n\
constraint int_lin_eq([2, 1, 1, 1, 1], [i, l, n, o, v], 100);\n\
constraint int_lin_eq([1, 1, 1, 1, 2, 1], [a, e, q, r, t, u], 50);\n\
constraint int_lin_eq([1, 1, 2, 1, 1, 1], [a, n, o, p, r, s], 82);\n\
constraint int_lin_eq([2, 1, 1, 1, 1, 1], [c, e, n, o, r, t], 74);\n\
constraint int_lin_eq([1, 1, 1, 1, 2, 1, 1, 1], [a, e, h, n, o, p, s, x], 134);\n\
solve satisfy;\n\
", "a = 5;\n\
b = 13;\n\
c = 9;\n\
d = 16;\n\
e = 20;\n\
f = 4;\n\
g = 24;\n\
h = 21;\n\
i = 25;\n\
j = 17;\n\
k = 23;\n\
l = 2;\n\
m = 8;\n\
n = 12;\n\
o = 10;\n\
p = 19;\n\
q = 7;\n\
r = 11;\n\
s = 15;\n\
t = 3;\n\
u = 1;\n\
v = 26;\n\
w = 6;\n\
x = 22;\n\
y = 14;\n\
z = 18;\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
