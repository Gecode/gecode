/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#include "gecode/cpltset.hh"
#include "gecode/cpltset/propagators.hh"

using namespace Gecode::CpltSet;

namespace Gecode {

  void selectUnion(Space* home, const CpltSetVarArgs& x, CpltSetVar s, CpltSetVar t, 
                   SetConLevel scl) {
    selectUnion_con(home, x, s, t, scl);
  }

//   void range(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, 
//              SetConLevel scl) {
//     ViewArray<IntView> iv(home, x.size());
//     ViewArray<SingletonCpltSetView> sbv(home, iv.size());
//     for (int i = sbv.size(); i--; ) {
//       sbv[i].init(iv[i]);
//       GECODE_ME_FAIL(home, sbv[i].cardinality(home, 1));
//     }
//     CpltSetView selview(s);
//     CpltSetView unionview(t);
    
//     selectUnion_post(home, sbv, selview, unionview, scl);
//   }

  void findNonEmptySub(Space* home, const CpltSetVarArgs& x, CpltSetVar s, 
                       CpltSetVar t, SetConLevel scl) {
    findNonEmptySub_con(home, x, s, t, scl);
  }

}

// STATISTICS: bdd-post
