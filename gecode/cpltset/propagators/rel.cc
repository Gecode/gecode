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

#include "gecode/cpltset/propagators.hh"

// for set bounds propagators with bdds
// #include "gecode/set/rel.hh"
// #include "gecode/set/rel-op.hh"
// #include "gecode/set/int.hh"
// #include "gecode/set/convex.hh"
// #include "gecode/set/sequence.hh"
// #include "gecode/set/distinct.hh"
// #include "gecode/set/select.hh"

using namespace Gecode::CpltSet;

namespace Gecode {

  void rel(Space* home, CpltSetVar x, SetRelType r, CpltSetVar y, SetConLevel scl) {
//     switch (scl) {
//     case SCL_BND_SBR:
//       {
//         std::cout << "rel set\n";
//         CpltSetView bx(x);
//         CpltSetView by(y);
//         CrdCpltSetView cx(bx);
//         CrdCpltSetView cy(by);
//         SetCpltSetView sx(cx);
//         SetCpltSetView sy(cy);
//         rel_post<SetCpltSetView, SetCpltSetView>(home, sx, r, sy, scl); 
//         break;
//       }
//     default:
      rel_con(home, x, r, y, scl);      
//     }
  }

  void rel(Space* home, IntVar x, SetRelType r, CpltSetVar y, SetConLevel scl) {
//     switch (scl) {
//     case SCL_BND_SBR:
//       {
//         switch(r) {
//         case SRT_SUB:
//           rel(home, y, SRT_SUP, x, scl);
//           break;
//         case SRT_SUP:
//           rel(home, y, SRT_SUB, x, scl);
//           break;
//         default:
//           rel(home, y, r, x, scl);
//         }
//         break;
//       }
//     default:
      rel_con(home, x, r, y, scl);
//     }
  }

  void rel(Space* home, CpltSetVar x, CpltSetRelType r, CpltSetVar y, SetConLevel scl) {
    rel_con(home, x, r, y, scl);
  }

  void rel(Space* home, IntVar x, CpltSetRelType r, CpltSetVar y, SetConLevel scl) {
    rel_con(home, x, r, y, scl);
  }

  void rel(Space* home, CpltSetVar x, CpltSetOpType o, CpltSetVar y, CpltSetRelType r, 
           CpltSetVar z, SetConLevel scl) {
    rel_con_bdd(home, x, o, y, r, z, scl);
  }

  void rel(Space* home, CpltSetVar x, CpltSetOpType o, CpltSetVar y, SetRelType r, 
           CpltSetVar z, SetConLevel scl) {
    rel_con_bdd(home, x, o, y, r, z, scl);
  }

  void rel(Space* home, CpltSetVar x, SetOpType o, CpltSetVar y, CpltSetRelType r, 
           CpltSetVar z, SetConLevel scl) {
    rel_con_bdd(home, x, o, y, r, z, scl);
  }

  void rel(Space* home, CpltSetVar x, SetOpType o, CpltSetVar y, SetRelType r, 
           CpltSetVar z, SetConLevel scl) {
    if (scl == SCL_BND_SBR) {
      rel_con_sbr(home, x, o, y, r, z, scl);
    } else {
      rel_con_bdd(home, x, o, y, r, z, scl);
    }
    
  }

}

// STATISTICS: bdd-post
