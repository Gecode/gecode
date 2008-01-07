/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2008
 *
 *  Last modified:
 *     $Date: 2007-11-30 13:58:34 +0100 (Fr, 30 Nov 2007) $ by $Author: tack $
 *     $Revision: 5524 $
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

// The n-queens model, as a JavaScript function
function queens(n) {
  var q = new Array;
  var qm = new Array;
  var qp = new Array;
  for (var i=0; i<n; i++) {
    q[i] = variable("Int", "q"+i, [0,n-1]);
    qm[i] = pair(-i, q[i]);
    qp[i] = pair(i, q[i]);
  }
  constraint("Int::Distinct::Val<Int::IntView>", q);
  constraint("Int::Distinct::Val<Int::OffsetView>", qm);
  constraint("Int::Distinct::Val<Int::OffsetView>", qp);
  constraint("ViewValBranching<Int::IntView,"+
                              "int,Int::Branch::BySizeMin,"+
                              "Int::Branch::ValMin>", q);
  return q;
}

// Execute the script
queens(10)
