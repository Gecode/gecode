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
        (void) new FlatZincTest("cumulatives_full::7",
R"FZN(
var 1..10: s1 :: output_var;
var 1..10: s2 :: output_var;
var 0..1: m1 :: output_var;
var 0..1: m2 :: output_var;
var 4..14: e1;
var 2..15: e2;
var 4..15: ms :: output_var;
constraint gecode_cumulatives([s1, s2], [3, 5], [3, 2], [m1, m2], [4, 6], true);
constraint int_lin_eq([1, -1], [s1, e1], -3);
constraint int_lin_eq([1, -1], [s2, e2], -5);
constraint array_int_maximum(ms, [e1, e2]);
solve minimize ms;
)FZN",
R"OUT(m1 = 1;
m2 = 0;
ms = 6;
s1 = 1;
s2 = 1;
----------
==========
)OUT");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
