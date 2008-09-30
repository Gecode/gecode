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

#include <gecode/scheduling.hh>

namespace Gecode {

  void
  unary(Space& home, const IntVarArgs& st, const IntArgs& dur) {
    using namespace Scheduling;
    if (st.same(home))
      throw ArgumentSame("Scheduling::unary");
    if (st.size() != dur.size())
      throw ArgumentSizeMismatch("Scheduling::unary");
    IntVarArgs ij(2);
    IntArgs c1(2,  1,-1);
    IntArgs c2(2, -1, 1);
    for (int i=0; i<st.size(); i++) {
      ij[0]=st[i];
      for (int j=i+1; j<st.size(); j++) {
        ij[1]=st[j];
        BoolVar b1(home,0,1), b2(home,0,1);
        linear(home, c1, ij, IRT_LQ, -dur[i], b1);
        linear(home, c2, ij, IRT_LQ, -dur[j], b2);
        rel(home, b1, BOT_XOR, b2, 1);
      }
    }
  }

}

// STATISTICS: scheduling-post
