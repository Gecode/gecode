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
        (void) new FlatZincTest("fzn_logic",
"var bool: b1 :: output_var;\n\
var bool: b2 :: output_var;\n\
var bool: b3 :: output_var;\n\
var bool: b4 :: output_var;\n\
var bool: b5 :: output_var;\n\
var bool: b6 :: output_var;\n\
array [1..3] of var bool: ab1 :: output_array([1..3]);\n\
array [1..3] of var bool: ab2 :: output_array([1..3]);\n\
\n\
constraint bool_eq(true, true);\n\
constraint bool_eq(false, false);\n\
\n\
constraint bool_not(true, false);\n\
constraint bool_not(false, true);\n\
\n\
constraint bool_and(false, true,  false);\n\
constraint bool_and(false, false, false);\n\
constraint bool_and(true,  false, false);\n\
constraint bool_and(true,  true,  true);\n\
constraint bool_and(b1,    true, true);\n\
\n\
constraint bool_or(false, true,  true);\n\
constraint bool_or(false, false, false);\n\
constraint bool_or(true,  false, true);\n\
constraint bool_or(true,  true,  true);\n\
constraint bool_or(b2,    false, true);\n\
\n\
constraint bool_xor(false, true,  true);\n\
constraint bool_xor(false, false, false);\n\
constraint bool_xor(true,  false, true);\n\
constraint bool_xor(true,  true,  false);\n\
constraint bool_xor(b3,    false, true);\n\
\n\
constraint bool_le_reif(false, true,  true);\n\
constraint bool_le_reif(false, false, true);\n\
constraint bool_le_reif(true,  false, false);\n\
constraint bool_le_reif(true,  true,  true);\n\
constraint bool_le_reif(true,  b4,    true);\n\
\n\
constraint bool_le_reif(false, true, true);\n\
constraint bool_le_reif(false, false, true);\n\
constraint bool_le_reif(true,  false,  false);\n\
constraint bool_le_reif(true,  true,  true);\n\
constraint bool_le_reif(true,  b5,  true);\n\
\n\
constraint bool_not(false, true);\n\
constraint bool_not(true, false);\n\
constraint bool_not(b6, false);\n\
\n\
constraint array_bool_and([false], false);\n\
constraint array_bool_and([true], true);\n\
constraint array_bool_and([false, true],  false);\n\
constraint array_bool_and([false, false], false);\n\
constraint array_bool_and([true,  false], false);\n\
constraint array_bool_and([true,  true],  true);\n\
constraint array_bool_and(ab1, true);\n\
\n\
constraint array_bool_or([false], false);\n\
constraint array_bool_or([true], true);\n\
constraint array_bool_or([false, true],  true);\n\
constraint array_bool_or([false, false], false);\n\
constraint array_bool_or([true,  false], true);\n\
constraint array_bool_or([true,  true],  true);\n\
constraint array_bool_or(ab2, true);\n\
\n\
solve satisfy;\n\
",
"ab1 = array1d(1..3, [true, true, true]);\n\
ab2 = array1d(1..3, [true, false, false]);\n\
b1 = true;\n\
b2 = true;\n\
b3 = true;\n\
b4 = true;\n\
b5 = true;\n\
b6 = true;\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
