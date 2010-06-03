/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2008
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include "test/branch.hh"

#ifdef GECODE_HAS_SET_VARS

namespace Test { namespace Branch {

  /// %Test brancher
  class Set : public SetTest {
  public:
    /// Create and register test
    Set(const std::string& s, const Gecode::IntSet& d, int n)
      : SetTest(s,n,d) {}
    /// Post propagators on variables \a x
    virtual void post(Gecode::Space& home, Gecode::SetVarArray& x) {
      Gecode::SetVarArgs xx(x.size()-1);
      for (int i=x.size()-1; i--;)
        xx[i] = x[i];
      Gecode::rel(home, Gecode::SOT_DUNION, xx, x[x.size()-1]);
    }
  };

  namespace {
    Gecode::IntSet d_dense(-2,3);
    const int v_sparse[6] = {-100,-10,0,10,100,1000};
    Gecode::IntSet d_sparse(v_sparse,6);

    Set d_3("Dense::3",d_dense,3);
    Set s_3("Sparse::3",d_sparse,3);
  }

}}

#endif

// STATISTICS: test-branch
