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
        (void) new FlatZincTest("on_restart::sol_set",
R"FZN(predicate int_eq_imp(var int: a,var int: b,var bool: r);
predicate gecode_on_restart_status(var int: s);
predicate gecode_on_restart_sol_set(var set of int: input,var set of int: out);
var set of 1..3: x:: output_var;
var set of 1..100: y:: output_var;
var 0..6: X_INTRODUCED_0_:: is_defined_var;
var bool: X_INTRODUCED_1_ ::var_is_introduced :: is_defined_var;
var 0..1: X_INTRODUCED_2_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_10_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_11_ ::var_is_introduced :: is_defined_var;
var 0..2: X_INTRODUCED_12_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_20_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_21_ ::var_is_introduced :: is_defined_var;
var 0..3: X_INTRODUCED_22_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_30_ ::var_is_introduced :: is_defined_var;
var 1..5: X_INTRODUCED_32_ ::var_is_introduced ;
var bool: X_INTRODUCED_33_ ::var_is_introduced :: is_defined_var;
var set of 1..3: X_INTRODUCED_34_ ::var_is_introduced ;
var bool: X_INTRODUCED_41_ ::var_is_introduced :: is_defined_var;
var bool: X_INTRODUCED_42_ ::var_is_introduced :: is_defined_var;
array [1..1] of var set of int: X_INTRODUCED_31_ ::var_is_introduced  = [x];
constraint bool_clause([X_INTRODUCED_1_,X_INTRODUCED_10_],[]);
constraint bool_clause([X_INTRODUCED_11_,X_INTRODUCED_20_],[]);
constraint bool_clause([X_INTRODUCED_21_,X_INTRODUCED_30_],[]);
constraint gecode_on_restart_status(X_INTRODUCED_32_);
constraint gecode_on_restart_sol_set(x,X_INTRODUCED_34_);
constraint bool_clause([X_INTRODUCED_41_],[X_INTRODUCED_33_]);
constraint bool_clause([X_INTRODUCED_33_,X_INTRODUCED_42_],[]);
constraint int_eq_imp(X_INTRODUCED_2_,1,X_INTRODUCED_1_);
constraint int_eq_imp(X_INTRODUCED_12_,2,X_INTRODUCED_11_);
constraint int_eq_imp(X_INTRODUCED_22_,3,X_INTRODUCED_21_);
constraint int_lin_eq([1,1,1,-1],[X_INTRODUCED_2_,X_INTRODUCED_12_,X_INTRODUCED_22_,X_INTRODUCED_0_],0):: ctx_neg:: defines_var(X_INTRODUCED_0_);
constraint set_in_reif(1,x,X_INTRODUCED_1_):: defines_var(X_INTRODUCED_1_);
constraint int_eq_imp(X_INTRODUCED_2_,0,X_INTRODUCED_10_):: defines_var(X_INTRODUCED_10_);
constraint set_in_reif(2,x,X_INTRODUCED_11_):: defines_var(X_INTRODUCED_11_);
constraint int_eq_imp(X_INTRODUCED_12_,0,X_INTRODUCED_20_):: defines_var(X_INTRODUCED_20_);
constraint set_in_reif(3,x,X_INTRODUCED_21_):: defines_var(X_INTRODUCED_21_);
constraint int_eq_imp(X_INTRODUCED_22_,0,X_INTRODUCED_30_):: defines_var(X_INTRODUCED_30_);
constraint int_eq_reif(X_INTRODUCED_32_,1,X_INTRODUCED_33_):: defines_var(X_INTRODUCED_33_);
constraint set_eq_reif(y,1..0,X_INTRODUCED_41_):: defines_var(X_INTRODUCED_41_);
constraint set_eq_reif(y,X_INTRODUCED_34_,X_INTRODUCED_42_):: defines_var(X_INTRODUCED_42_);
solve :: set_search(X_INTRODUCED_31_,input_order,indomain_min,complete) minimize X_INTRODUCED_0_;
)FZN",
R"OUT(x = 1..3;
y = {};
----------
x = 1..2;
y = 1..3;
----------
x = 1..1;
y = 1..2;
----------
x = {};
y = 1..1;
----------
==========
)OUT", true, {"--restart", "constant", "--restart-base", "100"});
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
