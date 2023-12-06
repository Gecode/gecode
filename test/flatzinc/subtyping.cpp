/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Jason Nguyen <jason.nguyen@monash.edu>
 *
 *  Copyright:
 *     Jason nguyen, 2023
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
        (void) new FlatZincTest("subtyping::1",
R"FZN(
bool: pb = true;
int: pi = 1;
float: pf = 1.5;
set of int: ps = {1, 3};
var bool: vb :: output_var;
var 1..1: vi :: output_var;
var 1.5..1.5: vf :: output_var;
var set of 1..1: vs :: output_var;
array [1..3] of var bool: vba = [vb, pb, true];
array [1..3] of var int: via = [vi, pi, 3];
array [1..3] of var float: vfa = [vf, pf, 3.5];
array [1..3] of var set of int: vsa = [vs, ps, {1}];
constraint bool_eq(vb, pb);
constraint set_card(vs, pi);
solve satisfy;
)FZN",
R"OUT(vb = true;
vf = 1.5;
vi = 1;
vs = 1..1;
----------
)OUT");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
