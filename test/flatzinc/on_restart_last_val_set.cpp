/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Copyright:
 *     Jip J. Dekker, 2023
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
        (void) new FlatZincTest("on_restart::last_val_set",
R"FZN(
predicate gecode_on_restart_status(var int: s);
predicate gecode_on_restart_last_val_set(var set of int: input,var set of int: out);
predicate array_int_maximum(var int: m,array [int] of var int: x);
var -1..4: x:: output_var;
var set of 1..10: y:: output_var;
var 0..4: X_INTRODUCED_1_ ::var_is_introduced :: is_defined_var;
var 1..5: X_INTRODUCED_3_ ::var_is_introduced ;
var bool: X_INTRODUCED_4_ ::var_is_introduced :: is_defined_var;
var set of 1..10: X_INTRODUCED_5_ ::var_is_introduced ;
var bool: X_INTRODUCED_6_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_7_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_9_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_10_ ::var_is_introduced :: is_defined_var;
var 0..2: X_INTRODUCED_11_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_13_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_14_ ::var_is_introduced :: is_defined_var;
var 0..3: X_INTRODUCED_15_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_17_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_18_ ::var_is_introduced :: is_defined_var;
var 0..4: X_INTRODUCED_19_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_21_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_22_ ::var_is_introduced :: is_defined_var;
var 0..5: X_INTRODUCED_23_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_25_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_26_ ::var_is_introduced :: is_defined_var;
var 0..6: X_INTRODUCED_27_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_29_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_30_ ::var_is_introduced :: is_defined_var;
var 0..7: X_INTRODUCED_31_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_33_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_34_ ::var_is_introduced :: is_defined_var;
var 0..8: X_INTRODUCED_35_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_37_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_38_ ::var_is_introduced :: is_defined_var;
var 0..9: X_INTRODUCED_39_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_41_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_42_ ::var_is_introduced :: is_defined_var;
var 0..10: X_INTRODUCED_43_ ::var_is_introduced :: is_defined_var;
var 0..10: X_INTRODUCED_45_ ::var_is_introduced :: is_defined_var;
var 0..20: X_INTRODUCED_47_ ::var_is_introduced :: is_defined_var;
var set of 0..20: X_INTRODUCED_50_ ::var_is_introduced ;
var set of 0..20: X_INTRODUCED_53_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_60_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_61_ ::var_is_introduced :: is_defined_var;
array [1..1] of var int: X_INTRODUCED_2_ ::var_is_introduced  = [x];
array [1..10] of var int: X_INTRODUCED_44_ ::var_is_introduced  = [X_INTRODUCED_7_,X_INTRODUCED_11_,X_INTRODUCED_15_,X_INTRODUCED_19_,X_INTRODUCED_23_,X_INTRODUCED_27_,X_INTRODUCED_31_,X_INTRODUCED_35_,X_INTRODUCED_39_,X_INTRODUCED_43_];
constraint int_abs(x,X_INTRODUCED_1_):: ctx_pos:: defines_var(X_INTRODUCED_1_);
constraint gecode_on_restart_status(X_INTRODUCED_3_);
constraint gecode_on_restart_last_val_set(y,X_INTRODUCED_5_);
constraint array_int_maximum(X_INTRODUCED_45_,X_INTRODUCED_44_):: defines_var(X_INTRODUCED_45_);
constraint set_in(X_INTRODUCED_47_,X_INTRODUCED_50_);
constraint set_card(X_INTRODUCED_50_,1);
constraint bool_clause([X_INTRODUCED_60_],[X_INTRODUCED_4_]);
constraint bool_clause([X_INTRODUCED_4_,X_INTRODUCED_61_],[]);
constraint int_eq_reif(X_INTRODUCED_3_,1,X_INTRODUCED_4_):: defines_var(X_INTRODUCED_4_);
constraint set_in_reif(1,X_INTRODUCED_5_,X_INTRODUCED_6_):: defines_var(X_INTRODUCED_6_);
constraint bool2int(X_INTRODUCED_6_,X_INTRODUCED_7_):: defines_var(X_INTRODUCED_7_);
constraint set_in_reif(2,X_INTRODUCED_5_,X_INTRODUCED_9_):: defines_var(X_INTRODUCED_9_);
constraint bool2int(X_INTRODUCED_9_,X_INTRODUCED_10_):: defines_var(X_INTRODUCED_10_);
constraint int_lin_eq([2,-1],[X_INTRODUCED_10_,X_INTRODUCED_11_],0):: defines_var(X_INTRODUCED_11_);
constraint set_in_reif(3,X_INTRODUCED_5_,X_INTRODUCED_13_):: defines_var(X_INTRODUCED_13_);
constraint bool2int(X_INTRODUCED_13_,X_INTRODUCED_14_):: defines_var(X_INTRODUCED_14_);
constraint int_lin_eq([3,-1],[X_INTRODUCED_14_,X_INTRODUCED_15_],0):: defines_var(X_INTRODUCED_15_);
constraint set_in_reif(4,X_INTRODUCED_5_,X_INTRODUCED_17_):: defines_var(X_INTRODUCED_17_);
constraint bool2int(X_INTRODUCED_17_,X_INTRODUCED_18_):: defines_var(X_INTRODUCED_18_);
constraint int_lin_eq([4,-1],[X_INTRODUCED_18_,X_INTRODUCED_19_],0):: defines_var(X_INTRODUCED_19_);
constraint set_in_reif(5,X_INTRODUCED_5_,X_INTRODUCED_21_):: defines_var(X_INTRODUCED_21_);
constraint bool2int(X_INTRODUCED_21_,X_INTRODUCED_22_):: defines_var(X_INTRODUCED_22_);
constraint int_lin_eq([5,-1],[X_INTRODUCED_22_,X_INTRODUCED_23_],0):: defines_var(X_INTRODUCED_23_);
constraint set_in_reif(6,X_INTRODUCED_5_,X_INTRODUCED_25_):: defines_var(X_INTRODUCED_25_);
constraint bool2int(X_INTRODUCED_25_,X_INTRODUCED_26_):: defines_var(X_INTRODUCED_26_);
constraint int_lin_eq([6,-1],[X_INTRODUCED_26_,X_INTRODUCED_27_],0):: defines_var(X_INTRODUCED_27_);
constraint set_in_reif(7,X_INTRODUCED_5_,X_INTRODUCED_29_):: defines_var(X_INTRODUCED_29_);
constraint bool2int(X_INTRODUCED_29_,X_INTRODUCED_30_):: defines_var(X_INTRODUCED_30_);
constraint int_lin_eq([7,-1],[X_INTRODUCED_30_,X_INTRODUCED_31_],0):: defines_var(X_INTRODUCED_31_);
constraint set_in_reif(8,X_INTRODUCED_5_,X_INTRODUCED_33_):: defines_var(X_INTRODUCED_33_);
constraint bool2int(X_INTRODUCED_33_,X_INTRODUCED_34_):: defines_var(X_INTRODUCED_34_);
constraint int_lin_eq([8,-1],[X_INTRODUCED_34_,X_INTRODUCED_35_],0):: defines_var(X_INTRODUCED_35_);
constraint set_in_reif(9,X_INTRODUCED_5_,X_INTRODUCED_37_):: defines_var(X_INTRODUCED_37_);
constraint bool2int(X_INTRODUCED_37_,X_INTRODUCED_38_):: defines_var(X_INTRODUCED_38_);
constraint int_lin_eq([9,-1],[X_INTRODUCED_38_,X_INTRODUCED_39_],0):: defines_var(X_INTRODUCED_39_);
constraint set_in_reif(10,X_INTRODUCED_5_,X_INTRODUCED_41_):: defines_var(X_INTRODUCED_41_);
constraint bool2int(X_INTRODUCED_41_,X_INTRODUCED_42_):: defines_var(X_INTRODUCED_42_);
constraint int_lin_eq([10,-1],[X_INTRODUCED_42_,X_INTRODUCED_43_],0):: defines_var(X_INTRODUCED_43_);
constraint int_lin_eq([2,-1],[X_INTRODUCED_45_,X_INTRODUCED_47_],0):: defines_var(X_INTRODUCED_47_);
constraint set_union(X_INTRODUCED_5_,X_INTRODUCED_50_,X_INTRODUCED_53_):: defines_var(X_INTRODUCED_53_);
constraint set_eq_reif(y,1..1,X_INTRODUCED_60_):: defines_var(X_INTRODUCED_60_);
constraint set_eq_reif(y,X_INTRODUCED_53_,X_INTRODUCED_61_):: defines_var(X_INTRODUCED_61_);
solve :: int_search(X_INTRODUCED_2_,input_order,indomain_min,complete) maximize X_INTRODUCED_1_;
)FZN",
R"OUT(x = -1;
y = 1..1;
----------
x = 2;
y = 1..2;
----------
x = 3;
y = {1, 2, 4};
----------
x = 4;
y = {1, 2, 4, 8};
----------
==========
)OUT", true, {"--restart", "constant", "--restart-base", "100"});
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
