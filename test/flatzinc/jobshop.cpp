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
        (void) new FlatZincTest("jobshop",
        "array [1..4] of var 0..14: s :: output_array([1..4]);\
var 0..14: end :: output_var;\
var bool: b1 :: output_var;\
var bool: b2 :: output_var;\
var bool: b3 :: output_var;\
var bool: b4 :: output_var;\
constraint  int_lin_le     ([1,-1], [s[1], s[2]], -2);\
constraint  int_lin_le     ([1,-1], [s[2], end ], -5);\
constraint  int_lin_le     ([1,-1], [s[3], s[4]], -3);\
constraint  int_lin_le     ([1,-1], [s[4], end ], -4);\
constraint  int_lin_le_reif([1,-1], [s[1], s[3]], -2, b1);\
constraint  int_lin_le_reif([1,-1], [s[3], s[1]], -3, b2);\
constraint  bool_or(b1, b2, true);\
constraint  int_lin_le_reif([1,-1], [s[2], s[4]], -5, b3);\
constraint  int_lin_le_reif([1,-1], [s[4], s[2]], -4, b4);\
constraint  bool_or(b3, b4, true);\
solve minimize end;\
", "b1 = true;\n\
b2 = false;\n\
b3 = true;\n\
b4 = false;\n\
end = 11;\n\
s = array1d(1..4, [0, 2, 2, 7]);\n\
----------\n\
==========\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
