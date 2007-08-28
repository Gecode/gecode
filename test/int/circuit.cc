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
   * \defgroup TaskTestIntCircuit Circuit constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// Simple test for circuit constraint
  class Circuit : public IntTest {
  public:
    /// Create and register test
    Circuit(int n, int min, int max, Gecode::IntConLevel icl)
      : IntTest("Circuit::" + str(icl) + "::" + str(n),
                n,min,max,false,icl) {
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
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      circuit(home, x, icl);
    }
  };

  Circuit c1v(1,0,0,Gecode::ICL_VAL);
  Circuit c2v(2,0,1,Gecode::ICL_VAL);
  Circuit c3v(3,0,2,Gecode::ICL_VAL);
  Circuit c4v(4,0,3,Gecode::ICL_VAL);
  Circuit c5v(5,0,4,Gecode::ICL_VAL);
  Circuit c6v(6,0,5,Gecode::ICL_VAL);

  Circuit c1d(1,0,0,Gecode::ICL_DOM);
  Circuit c2d(2,0,1,Gecode::ICL_DOM);
  Circuit c3d(3,0,2,Gecode::ICL_DOM);
  Circuit c4d(4,0,3,Gecode::ICL_DOM);
  Circuit c5d(5,0,4,Gecode::ICL_DOM);
  Circuit c6d(6,0,5,Gecode::ICL_DOM);
  //@}

}}}

// STATISTICS: test-int
