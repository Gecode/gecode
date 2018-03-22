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
        (void) new FlatZincTest("fzn_arith",
"var 1.0..10.0: f1 :: output_var;\n\
var 1.0..10.0: f2 :: output_var;\n\
var 1.0..10.0: f3 :: output_var;\n\
var 1.0..10.0: f4 :: output_var;\n\
var 1.0..10.0: f5 :: output_var;\n\
var 1.0..10.0: f6 :: output_var;\n\
var 1.0..10.0: f7 :: output_var;\n\
var 1.0..10.0: f8 :: output_var;\n\
var 1.0..10.0: f9 :: output_var;\n\
\n\
var 1..10: i1 :: output_var;\n\
var 1..10: i2 :: output_var;\n\
var 1..10: i3 :: output_var;\n\
var 1..10: i4 :: output_var;\n\
var 1..10: i5 :: output_var;\n\
var 1..10: i6 :: output_var;\n\
var 1..10: i7 :: output_var;\n\
var 1..10: i8 :: output_var;\n\
var 1..10: i9 :: output_var;\n\
\n\
constraint float_plus(1.0, 1.0, 2.0);\n\
constraint float_plus(f1, 1.0, 3.0);\n\
\n\
constraint float_plus(0.0, 1.0, 1.0);\n\
constraint float_plus(2.0, 1.0, f2);\n\
constraint float_plus(-1.0, 1.0, 0.0);\n\
constraint float_plus(-5.0, f4, 0.0);\n\
constraint int_plus(1, 1, 2);\n\
constraint int_plus(i1, 1, 3);\n\
\n\
constraint int_plus(0, 1, 1);\n\
constraint int_plus(2, 1, i2);\n\
\n\
constraint int_times(1, 1, 1);\n\
constraint int_times(i3, 2, 8);\n\
\n\
constraint int_plus(-1, 1, 0);\n\
constraint int_plus(-5, i4, 0);\n\
\n\
constraint int_div(1, 1, 1);\n\
constraint int_div(i5, 3, 2);\n\
\n\
constraint int_mod(1, 2, 1);\n\
constraint int_mod(i6, 6, 0);\n\
\n\
constraint int_min(1, 2, 1);\n\
constraint int_min(i7, 9, 8);\n\
\n\
constraint int_max(1, 2, 2);\n\
constraint int_max(i8, 7, 9);\n\
\n\
constraint int_abs(-1, 1);\n\
constraint int_abs(-10, i9);\n\
\n\
solve satisfy;\n\
",
"f1 = 2.0;\n\
f2 = 3.0;\n\
f3 = 1.0;\n\
f4 = 5.0;\n\
f5 = 1.0;\n\
f6 = 1.0;\n\
f7 = 1.0;\n\
f8 = 1.0;\n\
f9 = 1.0;\n\
i1 = 2;\n\
i2 = 3;\n\
i3 = 4;\n\
i4 = 5;\n\
i5 = 6;\n\
i6 = 6;\n\
i7 = 8;\n\
i8 = 9;\n\
i9 = 10;\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
