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
        (void) new FlatZincTest("fzn_comparison",
"var bool: b1;\n\
var bool: b2;\n\
var bool: b3;\n\
var bool: b4;\n\
var bool: b5;\n\
var bool: b6;\n\
\n\
var bool: rb1;\n\
var bool: rb2;\n\
var bool: rb3;\n\
var bool: rb4;\n\
var bool: rb5;\n\
var bool: rb6;\n\
\n\
var 1.0..10.0: f1;\n\
var 1.0..10.0: f2;\n\
var 1.0..10.0: f3;\n\
var 1.0..10.0: f4;\n\
var 1.0..10.0: f5;\n\
var 1.0..10.0: f6;\n\
\n\
var bool: rf1;\n\
var bool: rf2;\n\
var bool: rf3;\n\
var bool: rf4;\n\
var bool: rf5;\n\
var bool: rf6;\n\
\n\
var 1..10: i1;\n\
var 1..10: i2;\n\
var 1..10: i3;\n\
var 1..10: i4;\n\
var 1..10: i5;\n\
var 1..10: i6;\n\
\n\
var bool: ri1;\n\
var bool: ri2;\n\
var bool: ri3;\n\
var bool: ri4;\n\
var bool: ri5;\n\
var bool: ri6;\n\
\n\
array [1..3] of var 1.0..10.0: fa1;\n\
array [1..3] of var 1.0..10.0: fa2;\n\
array [1..3] of var 1.0..10.0: fa3;\n\
array [1..3] of var 1.0..10.0: fa4;\n\
array [1..3] of var 1.0..10.0: fa5;\n\
\n\
var bool: rfa1;\n\
var bool: rfa2;\n\
var bool: rfa3;\n\
var bool: rfa4;\n\
var bool: rfa5;\n\
\n\
array [1..3] of var 1..10: ia1;\n\
array [1..3] of var 1..10: ia2;\n\
array [1..3] of var 1..10: ia3;\n\
array [1..3] of var 1..10: ia4;\n\
array [1..3] of var 1..10: ia5;\n\
\n\
var bool: ria1;\n\
var bool: ria2;\n\
var bool: ria3;\n\
var bool: ria4;\n\
var bool: ria5;\n\
\n\
var set of 1..3: s1;\n\
var set of 1..3: s2;\n\
var set of 1..3: s3;\n\
var set of 1..3: s4;\n\
var set of 1..3: s5;\n\
var set of 1..3: s6;\n\
\n\
var bool: rs1;\n\
var bool: rs2;\n\
var bool: rs3;\n\
var bool: rs4;\n\
var bool: rs5;\n\
var bool: rs6;\n\
\n\
% int_{lt,le,eq,ne}\n\
\n\
constraint int_lt(2, 3);\n\
constraint int_lt(2, i1);\n\
constraint int_lt(i1, 9);\n\
\n\
constraint int_le(2, 3);\n\
constraint int_le(2, i2);\n\
constraint int_le(i2, 9);\n\
\n\
constraint int_le(2, i3);\n\
constraint int_le(i3, 9);\n\
\n\
constraint int_lt(2, i4);\n\
constraint int_lt(i4, 9);\n\
\n\
constraint int_eq(2, 2);\n\
constraint int_eq(2, i5);\n\
\n\
constraint int_ne(2, 3);\n\
constraint int_ne(1, i6);\n\
\n\
% int_{lt,le,eq,ne}_reif\n\
\n\
constraint int_lt_reif(2, 3, ri1);\n\
constraint int_lt_reif(2, i1, ri1);\n\
constraint int_lt_reif(i1, 9, ri1);\n\
\n\
constraint int_le_reif(2, 3, ri2);\n\
constraint int_le_reif(2, i2, ri2);\n\
constraint int_le_reif(i2, 9, ri2);\n\
\n\
constraint int_le_reif(2, 3, ri3);\n\
constraint int_le_reif(2, i3, ri3);\n\
constraint int_le_reif(i3, 9, ri3);\n\
\n\
constraint int_lt_reif(2, 3, ri4);\n\
constraint int_lt_reif(2, i4, ri4);\n\
constraint int_lt_reif(i4, 9, ri4);\n\
\n\
constraint int_eq_reif(2, 2, ri5);\n\
constraint int_eq_reif(2, i5, ri5);\n\
\n\
constraint int_ne_reif(2, 3, ri6);\n\
constraint int_ne_reif(1, i6, ri6);\n\
\n\
% float_{lt,le,ge,gt,eq,ne}\n\
\n\
% constraint float_lt(2.0, 3.0);\n\
% constraint float_lt(2.0, f1);\n\
% constraint float_lt(f1, 9.0);\n\
\n\
constraint float_le(2.0, 3.0);\n\
constraint float_le(2.0, f2);\n\
constraint float_le(f2, 9.0);\n\
\n\
constraint float_le(2.0, f3);\n\
constraint float_le(f3, 9.0);\n\
\n\
constraint float_eq(2.0, 2.0);\n\
constraint float_eq(2.0, f5);\n\
\n\
% constraint float_ne(2.0, 3.0);\n\
% constraint float_ne(2.0, f6);\n\
\n\
% float_{lt,le,eq,ne}_reif\n\
\n\
% constraint float_lt_reif(2.0, 3.0, rf1);\n\
% constraint float_lt_reif(2.0, f1, rf1);\n\
% constraint float_lt_reif(f1, 9.0, rf1);\n\
\n\
constraint float_le_reif(2.0, 3.0, rf2);\n\
constraint float_le_reif(2.0, f2, rf2);\n\
constraint float_le_reif(f2, 9.0, rf2);\n\
\n\
constraint float_le_reif(2.0, 3.0, rf3);\n\
constraint float_le_reif(2.0, f3, rf3);\n\
constraint float_le_reif(f3, 9.0, rf3);\n\
\n\
constraint float_eq_reif(2.0, 2.0, rf5);\n\
constraint float_eq_reif(2.0, f5, rf5);\n\
\n\
% constraint float_ne_reif(2.0, 3.0, rf6);\n\
% constraint float_ne_reif(2.0, f6, rf6);\n\
\n\
% set_{lt,le,gt,ge,eq,ne}\n\
\n\
constraint set_lt({}, {1, 2, 3});\n\
constraint set_lt({}, s1);\n\
constraint set_lt(s1, {1, 2, 3});\n\
\n\
constraint set_le({}, {1, 2, 3});\n\
constraint set_le({1}, s2);\n\
constraint set_le(s2, {1, 2, 3});\n\
\n\
constraint set_lt({}, {1, 2, 3});\n\
constraint set_lt({}, s3);\n\
constraint set_lt(s3, {1, 2, 3});\n\
\n\
constraint set_le({}, {1, 2, 3});\n\
constraint set_le({1}, s4);\n\
constraint set_le(s4, {1, 2, 3});\n\
\n\
constraint set_eq({1, 2, 3}, {1, 2, 3});\n\
constraint set_eq(s5, {1, 2, 3});\n\
\n\
constraint set_ne({}, {1, 2, 3});\n\
constraint set_ne(s6, {1, 2, 3});\n\
\n\
% int_lin_{lt,le,gt,ge,eq}\n\
\n\
% constraint int_lin_lt([1, 2, 3], [1, 2, 3], 100);\n\
% constraint int_lin_lt([1, 2, 3], ia1, 10);\n\
\n\
constraint int_lin_le([1, 2, 3], [1, 2, 3], 100);\n\
constraint int_lin_le([1, 2, 3], ia2, 10);\n\
\n\
% constraint int_lin_gt([1, 2, 3], [1, 2, 3], 10);\n\
% constraint int_lin_gt([1, 2, 3], ia3, 10);\n\
\n\
constraint int_lin_le([-1, -2, -3], [1, 2, 3], -10);\n\
constraint int_lin_le([-1, -2, -3], ia4, -10);\n\
\n\
constraint int_lin_eq([1, 2, 3], [1, 2, 3], 14);\n\
constraint int_lin_eq([1, 2, 3], ia5, 14);\n\
\n\
% int_lin_{lt,le,gt,ge,eq}_reif\n\
\n\
% constraint int_lin_lt_reif([1, 2, 3], [1, 2, 3], 100, ria1);\n\
% constraint int_lin_lt_reif([1, 2, 3], ia1, 10, ria1);\n\
\n\
constraint int_lin_le_reif([1, 2, 3], [1, 2, 3], 100, ria2);\n\
constraint int_lin_le_reif([1, 2, 3], ia2, 10, ria2);\n\
\n\
% constraint int_lin_gt_reif([1, 2, 3], [1, 2, 3], 10, ria3);\n\
% constraint int_lin_gt_reif([1, 2, 3], ia3, 10, ria3);\n\
\n\
constraint int_lin_le_reif([-1, -2, -3], [1, 2, 3], -10, ria4);\n\
constraint int_lin_le_reif([-1, -2, -3], ia4, -10, ria4);\n\
\n\
constraint int_lin_eq_reif([1, 2, 3], [1, 2, 3], 14, ria5);\n\
constraint int_lin_eq_reif([1, 2, 3], ia5, 14, ria5);\n\
\n\
solve satisfy;\n\
",
"----------\n");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
