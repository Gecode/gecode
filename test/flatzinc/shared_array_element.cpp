/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2018
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
        (void) new FlatZincTest("SharedArrayElement",
"predicate count(array [int] of var int: x,var int: y,var int: c);\n\
array [1..6] of int: X_INTRODUCED_25_ = [0,0,1,1,0,1];\n\
array [1..6] of int: X_INTRODUCED_27_ = [1,0,0,0,1,0];\n\
array [1..6] of int: X_INTRODUCED_29_ = [1,1,0,1,0,0];\n\
array [1..3] of int: X_INTRODUCED_43_ = [1,1,1];\n\
array [1..5] of int: X_INTRODUCED_49_ = [1,1,1,1,1];\n\
var 1..3: X_INTRODUCED_0_;\n\
var 1..3: X_INTRODUCED_1_;\n\
var 1..3: X_INTRODUCED_2_;\n\
var 1..3: X_INTRODUCED_3_;\n\
var 1..3: X_INTRODUCED_4_;\n\
var 1..3: X_INTRODUCED_5_;\n\
var 1..3: X_INTRODUCED_6_;\n\
var 1..3: X_INTRODUCED_7_;\n\
var 1..3: X_INTRODUCED_8_;\n\
var 1..3: X_INTRODUCED_9_;\n\
var 0..1: X_INTRODUCED_26_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_28_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_30_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_31_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_32_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_33_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_34_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_35_ ::var_is_introduced :: is_defined_var;\n\
var 0..0: X_INTRODUCED_36_ ::var_is_introduced :: is_defined_var;\n\
var 0..0: X_INTRODUCED_37_ ::var_is_introduced :: is_defined_var;\n\
var 0..0: X_INTRODUCED_38_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_39_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_40_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_41_ ::var_is_introduced :: is_defined_var;\n\
var 0..1: X_INTRODUCED_42_ ::var_is_introduced :: is_defined_var;\n\
array [1..10] of var int: X:: output_array([1..10]) = [X_INTRODUCED_0_,X_INTRODUCED_1_,X_INTRODUCED_2_,X_INTRODUCED_3_,X_INTRODUCED_4_,X_INTRODUCED_5_,X_INTRODUCED_6_,X_INTRODUCED_7_,X_INTRODUCED_8_,X_INTRODUCED_9_];\n\
array [1..15] of var int: Y:: output_array([1..15]) = [X_INTRODUCED_26_,X_INTRODUCED_28_,X_INTRODUCED_30_,X_INTRODUCED_31_,X_INTRODUCED_32_,X_INTRODUCED_33_,X_INTRODUCED_34_,X_INTRODUCED_35_,X_INTRODUCED_36_,X_INTRODUCED_37_,X_INTRODUCED_38_,X_INTRODUCED_39_,X_INTRODUCED_40_,X_INTRODUCED_41_,X_INTRODUCED_42_];\n\
constraint array_int_element(X_INTRODUCED_0_,X_INTRODUCED_25_,X_INTRODUCED_26_):: defines_var(X_INTRODUCED_26_);\n\
constraint array_int_element(X_INTRODUCED_0_,X_INTRODUCED_27_,X_INTRODUCED_28_):: defines_var(X_INTRODUCED_28_);\n\
constraint array_int_element(X_INTRODUCED_0_,X_INTRODUCED_29_,X_INTRODUCED_30_):: defines_var(X_INTRODUCED_30_);\n\
constraint array_int_element(X_INTRODUCED_1_,X_INTRODUCED_27_,X_INTRODUCED_31_):: defines_var(X_INTRODUCED_31_);\n\
constraint array_int_element(X_INTRODUCED_1_,X_INTRODUCED_29_,X_INTRODUCED_32_):: defines_var(X_INTRODUCED_32_);\n\
constraint array_int_element(X_INTRODUCED_2_,X_INTRODUCED_27_,X_INTRODUCED_33_):: defines_var(X_INTRODUCED_33_);\n\
constraint array_int_element(X_INTRODUCED_2_,X_INTRODUCED_29_,X_INTRODUCED_34_):: defines_var(X_INTRODUCED_34_);\n\
constraint array_int_element(X_INTRODUCED_3_,X_INTRODUCED_27_,X_INTRODUCED_35_):: defines_var(X_INTRODUCED_35_);\n\
constraint array_int_element(X_INTRODUCED_3_,X_INTRODUCED_29_,X_INTRODUCED_36_):: defines_var(X_INTRODUCED_36_);\n\
constraint array_int_element(X_INTRODUCED_4_,X_INTRODUCED_27_,X_INTRODUCED_37_):: defines_var(X_INTRODUCED_37_);\n\
constraint array_int_element(X_INTRODUCED_4_,X_INTRODUCED_29_,X_INTRODUCED_38_):: defines_var(X_INTRODUCED_38_);\n\
constraint array_int_element(X_INTRODUCED_5_,X_INTRODUCED_27_,X_INTRODUCED_39_):: defines_var(X_INTRODUCED_39_);\n\
constraint array_int_element(X_INTRODUCED_5_,X_INTRODUCED_29_,X_INTRODUCED_40_):: defines_var(X_INTRODUCED_40_);\n\
constraint array_int_element(X_INTRODUCED_6_,X_INTRODUCED_27_,X_INTRODUCED_41_):: defines_var(X_INTRODUCED_41_);\n\
constraint array_int_element(X_INTRODUCED_6_,X_INTRODUCED_29_,X_INTRODUCED_42_):: defines_var(X_INTRODUCED_42_);\n\
constraint int_lin_eq(X_INTRODUCED_43_,[X_INTRODUCED_31_,X_INTRODUCED_28_,X_INTRODUCED_33_],1);\n\
constraint int_lin_eq(X_INTRODUCED_43_,[X_INTRODUCED_33_,X_INTRODUCED_31_,X_INTRODUCED_35_],1);\n\
constraint int_lin_eq(X_INTRODUCED_43_,[X_INTRODUCED_35_,X_INTRODUCED_33_,X_INTRODUCED_37_],1);\n\
constraint int_lin_eq(X_INTRODUCED_43_,[X_INTRODUCED_37_,X_INTRODUCED_35_,X_INTRODUCED_39_],1);\n\
constraint int_lin_eq(X_INTRODUCED_43_,[X_INTRODUCED_39_,X_INTRODUCED_37_,X_INTRODUCED_41_],1);\n\
constraint int_lin_eq(X_INTRODUCED_49_,[X_INTRODUCED_36_,X_INTRODUCED_34_,X_INTRODUCED_32_,X_INTRODUCED_30_,X_INTRODUCED_38_],2);\n\
constraint int_lin_eq(X_INTRODUCED_49_,[X_INTRODUCED_38_,X_INTRODUCED_36_,X_INTRODUCED_34_,X_INTRODUCED_32_,X_INTRODUCED_40_],2);\n\
constraint int_lin_eq(X_INTRODUCED_49_,[X_INTRODUCED_40_,X_INTRODUCED_38_,X_INTRODUCED_36_,X_INTRODUCED_34_,X_INTRODUCED_42_],2);\n\
constraint count(X,2,1);\n\
solve  satisfy;\n\
",
"X = array1d(1..10, [2, 3, 1, 3, 3, 1, 3, 1, 1, 1]);\n\
Y = array1d(1..15, [0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0]);\n\
----------\n\
"
);
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
