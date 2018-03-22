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
        (void) new FlatZincTest("test_approx_bnb",
"var 1..10: x :: output_var;\n\
\n\
solve\n\
	:: int_search([x], input_order, indomain_max, complete)\n\
	minimize x;\n\
",
"x = 10;\n\
----------\n\
x = 9;\n\
----------\n\
x = 8;\n\
----------\n\
x = 7;\n\
----------\n\
x = 6;\n\
----------\n\
x = 5;\n\
----------\n\
x = 4;\n\
----------\n\
x = 3;\n\
----------\n\
x = 2;\n\
----------\n\
x = 1;\n\
----------\n\
==========\n\
", true);
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
