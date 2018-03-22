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
        (void) new FlatZincTest("fzn_arrays",
"var bool: b1;\n\
var 1.0..10.0: f1;\n\
var 1..10: i1;\n\
var set of 1..10: s1;\n\
\n\
array [1..3] of var bool: ab1;\n\
array [1..3] of var 1.0..10.0: af1;\n\
array [1..3] of var 1..10: ai1;\n\
array [1..3] of var set of 1..10: as1;\n\
\n\
constraint array_bool_element(1, [true, false], b1);\n\
constraint array_var_bool_element(2, ab1, b1);\n\
\n\
constraint array_int_element(2, [1, 2, 3], i1);\n\
constraint array_var_int_element(2, ai1, i1);\n\
\n\
constraint array_set_element(1, [{1}, {2}, {3}], s1);\n\
constraint array_var_set_element(2, as1, s1);\n\
\n\
solve satisfy;\n\
", "----------\n");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
