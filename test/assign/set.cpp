/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#ifdef GECODE_HAS_SET_VARS

namespace Test { namespace Assign {

  /// %Test assignment with disjoint union propagator
  class Set : public SetTest {
  public:
    /// Create and register test
    Set(const std::string& s, const Gecode::IntSet& d, int n)
      : SetTest(s,n,d) {}
    /// Post propagators on variables \a x
    virtual void post(Gecode::Space& home, Gecode::SetVarArray& x) {
      Gecode::SetVarArgs y(x.size()-1);
      for (int i=0; i<x.size()-1; i++)
        y[i]=x[i];
      Gecode::rel(home, Gecode::SOT_UNION, y, x[x.size()-1]);
    }
  };
    
  namespace {
    Gecode::IntArgs s(5, -100,-10,0,10,100);
    Set d_3("Dense::3",Gecode::IntSet(-2,2),3);
    Set d_4("Dense::4",Gecode::IntSet(-2,2),4);
    Set s_3("Sparse::3",Gecode::IntSet(s),3);
    Set s_4("Sparse::4",Gecode::IntSet(s),4);
  }

}}

#endif

// STATISTICS: test-branch

