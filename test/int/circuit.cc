/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

#include "test/int.hh"

namespace Test { namespace Int { namespace Circuit {

  /**
   * \defgroup TaskTestIntGraph Graph constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// Simple test for circuit constraint
  class Circuit : public IntTest {
  public:
    /// Create and register test
    Circuit(const char* t, int n, int min, int max, IntConLevel icl)
      : IntTest(t,n,min,max,false,icl) {
      testdomcon = false;
    }
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=x.size(); i--; )
        if ((x[i] < 0) || (x[i] > x.size()-1))
          return false;
      int reachable = 0;
      {
        int j=0;
        for (int i=x.size(); i--; ) {
          j=x[j]; reachable |= (1 << j);
        }
      }
      for (int i=x.size(); i--; )
        if (!(reachable & (1 << i)))
          return false;
      return true;
    }
    /// Post circuit constraint on \a x
    virtual void post(Space* home, IntVarArray& x) {
      circuit(home, x, icl);
    }
  };

  Circuit c1v("Circuit::Val::1",1,0,0,ICL_VAL);
  Circuit c2v("Circuit::Val::2",2,0,1,ICL_VAL);
  Circuit c3v("Circuit::Val::3",3,0,2,ICL_VAL);
  Circuit c4v("Circuit::Val::4",4,0,3,ICL_VAL);
  Circuit c5v("Circuit::Val::5",5,0,4,ICL_VAL);
  Circuit c6v("Circuit::Val::6",6,0,5,ICL_VAL);

  Circuit c1d("Circuit::Dom::1",1,0,0,ICL_DOM);
  Circuit c2d("Circuit::Dom::2",2,0,1,ICL_DOM);
  Circuit c3d("Circuit::Dom::3",3,0,2,ICL_DOM);
  Circuit c4d("Circuit::Dom::4",4,0,3,ICL_DOM);
  Circuit c5d("Circuit::Dom::5",5,0,4,ICL_DOM);
  Circuit c6d("Circuit::Dom::6",6,0,5,ICL_DOM);
  //@}

}}}

// STATISTICS: test-int
