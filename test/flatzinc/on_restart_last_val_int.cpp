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
        (void) new FlatZincTest("on_restart::last_val_int",
R"FZN(
predicate gecode_on_restart_status(var int: s);
predicate gecode_on_restart_last_val_int(var int: input,var int: out);
predicate int_lin_eq_imp(array [int] of int: as,array [int] of var int: bs,int: c,var bool: r);
predicate int_eq_imp(var int: a,var int: b,var bool: r);
array [1..2] of int: X_INTRODUCED_15_ = [1,-1];
var -1..4: x:: output_var;
var int: y:: output_var;
var 0..4: X_INTRODUCED_1_ ::var_is_introduced :: is_defined_var;
var 1..5: X_INTRODUCED_3_ ::var_is_introduced ;
var bool: X_INTRODUCED_4_ ::var_is_introduced :: is_defined_var;
var int: X_INTRODUCED_6_ ::var_is_introduced ;
var bool: X_INTRODUCED_16_ ::var_is_introduced :: is_defined_var;
array [1..1] of var int: X_INTRODUCED_2_ ::var_is_introduced  = [x];
constraint int_abs(x,X_INTRODUCED_1_):: ctx_pos:: defines_var(X_INTRODUCED_1_);
constraint gecode_on_restart_status(X_INTRODUCED_3_);
constraint gecode_on_restart_last_val_int(y,X_INTRODUCED_6_);
constraint bool_clause([X_INTRODUCED_4_,X_INTRODUCED_16_],[]);
constraint int_eq_imp(y,0,X_INTRODUCED_4_);
constraint int_eq_reif(X_INTRODUCED_3_,1,X_INTRODUCED_4_):: defines_var(X_INTRODUCED_4_);
constraint int_lin_eq_imp(X_INTRODUCED_15_,[y,X_INTRODUCED_6_],1,X_INTRODUCED_16_):: defines_var(X_INTRODUCED_16_);
solve :: int_search(X_INTRODUCED_2_,input_order,indomain_min,complete) maximize X_INTRODUCED_1_;
)FZN",
R"OUT(x = -1;
y = 0;
----------
x = 2;
y = 1;
----------
x = 3;
y = 2;
----------
x = 4;
y = 3;
----------
==========
)OUT", true, {"--restart", "constant", "--restart-base", "100"});
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
