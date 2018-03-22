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
        (void) new FlatZincTest("sat_cmp_reif",
"var bool: lt_a;\n\
var bool: lt_b;\n\
var bool: gt_a;\n\
var bool: gt_b;\n\
var bool: le_a;\n\
var bool: le_b;\n\
var bool: le_c;\n\
var bool: ge_a;\n\
var bool: ge_b;\n\
array [1..9] of var bool: result :: output_array([1..9]) =\n\
	[lt_a, lt_b, gt_a, gt_b, le_a, le_b, le_c, ge_a, ge_b];\n\
constraint bool_lt_reif(lt_a,  lt_b, true);\n\
constraint bool_lt_reif(gt_b,  gt_a, true);\n\
constraint bool_le_reif(true,  le_a, true);\n\
constraint bool_le_reif(le_b,  false, true);\n\
constraint bool_le_reif(false, true, true);\n\
constraint bool_le_reif(ge_a, false, true);\n\
constraint bool_le_reif(true, ge_b, true);\n\
\n\
solve satisfy;\n\
","result = array1d(1..9, [false, true, true, false, true, false, false, false, true]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
