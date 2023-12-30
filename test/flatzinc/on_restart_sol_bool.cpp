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
        (void) new FlatZincTest("on_restart::sol_bool",
R"FZN(
predicate gecode_on_restart_sol_bool(var bool: input,var bool: out);
predicate gecode_on_restart_status(var int: s);
predicate int_eq_imp(var int: a,var int: b,var bool: r);
var -1..3: x:: output_var;
var bool: is_even:: is_defined_var:: output_var;
var bool: y:: output_var;
var 0..3: X_INTRODUCED_1_ ::var_is_introduced :: is_defined_var;
var -1..1: X_INTRODUCED_2_ ::var_is_introduced ;
var bool: X_INTRODUCED_4_ ::var_is_introduced ;
var bool: X_INTRODUCED_5_ ::var_is_introduced :: is_defined_var;
var 1..5: X_INTRODUCED_6_ ::var_is_introduced ;
var bool: X_INTRODUCED_7_ ::var_is_introduced :: is_defined_var;
array [1..1] of var int: X_INTRODUCED_3_ ::var_is_introduced  = [x];
constraint int_abs(x,X_INTRODUCED_1_):: ctx_pos:: defines_var(X_INTRODUCED_1_);
constraint int_mod(X_INTRODUCED_1_,2,X_INTRODUCED_2_);
constraint gecode_on_restart_sol_bool(is_even,X_INTRODUCED_4_);
constraint gecode_on_restart_status(X_INTRODUCED_6_);
constraint bool_clause([X_INTRODUCED_5_,X_INTRODUCED_7_],[]);
constraint int_eq_reif(X_INTRODUCED_2_,0,is_even):: defines_var(is_even);
constraint bool_eq_reif(y,X_INTRODUCED_4_,X_INTRODUCED_5_):: defines_var(X_INTRODUCED_5_);
constraint int_eq_imp(X_INTRODUCED_6_,1,X_INTRODUCED_7_):: defines_var(X_INTRODUCED_7_);
solve :: int_search(X_INTRODUCED_3_,input_order,indomain_min,complete) maximize X_INTRODUCED_1_;
)FZN",
R"OUT(is_even = false;
x = -1;
y = false;
----------
is_even = true;
x = 2;
y = false;
----------
is_even = false;
x = 3;
y = true;
----------
==========
)OUT", true, {"--restart", "constant", "--restart-base", "100"});
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
