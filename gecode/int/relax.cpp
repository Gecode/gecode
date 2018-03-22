/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

#include <gecode/int.hh>
#include <gecode/search/relax.hh>

namespace Gecode { namespace Int {

  /// Class for posting equalities for non-relaxed variables
  class Relax {
  public:
    /// Post the actual equality for non-relaxed variables
    ExecStatus operator ()(Home home, IntVar x, IntVar sx);
    /// Post the actual equality for non-relaxed variables
    ExecStatus operator ()(Home home, BoolVar x, BoolVar sx);
  };

  forceinline ExecStatus
  Relax::operator ()(Home home, IntVar x, IntVar sx) {
    IntView xv(x);
    return me_failed(xv.eq(home,sx.val())) ? ES_FAILED : ES_OK;
  }

  forceinline ExecStatus
  Relax::operator ()(Home home, BoolVar x, BoolVar sx) {
    BoolView xv(x);
    return me_failed(xv.eq(home,sx.val())) ? ES_FAILED : ES_OK;
  }

}}

namespace Gecode {

  void
  relax(Home home, const IntVarArgs& x, const IntVarArgs& sx,
        Rnd r, double p) {
    if (x.size() != sx.size())
      throw Int::ArgumentSizeMismatch("Int::relax");
    if ((p < 0.0) || (p > 1.0))
      throw Int::OutOfLimits("Int::relax");
    Int::Relax ir;
    Search::relax<IntVarArgs,Int::Relax>(home,x,sx,r,p,ir);
  }

  void
  relax(Home home, const BoolVarArgs& x, const BoolVarArgs& sx,
        Rnd r, double p) {
    if (x.size() != sx.size())
      throw Int::ArgumentSizeMismatch("Int::relax");
    if ((p < 0.0) || (p > 1.0))
      throw Int::OutOfLimits("Int::relax");
    Int::Relax ir;
    Search::relax<BoolVarArgs,Int::Relax>(home,x,sx,r,p,ir);
  }

}

// STATISTICS: int-other

