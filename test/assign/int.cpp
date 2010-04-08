/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include "test/assign.hh"

namespace Test { namespace Assign {

  /// %Test assignment with distinct propagator
  class Int : public IntTest {
  public:
    /// Create and register test
    Int(const std::string& s, const Gecode::IntSet& d, int n)
      : IntTest(s,n,d) {}
    /// Post propagators on variables \a x
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      Gecode::distinct(home, x);
    }
  };

  Gecode::IntSet d_dense(-2,2);
  const int v_sparse[5] = {-100,-10,0,10,100};
  Gecode::IntSet d_sparse(v_sparse,5);

  Gecode::IntSet d_large(-2,10);

  Int d_3("Dense::3",d_dense,3);
  Int d_5("Dense::5",d_dense,5);
  Int s_3("Sparse::3",d_sparse,3);
  Int s_5("Sparse::5",d_sparse,5);
  Int l_2("Large::2",d_large,2);
  Int l_3("Large::3",d_large,3);

}}

// STATISTICS: test-branch

