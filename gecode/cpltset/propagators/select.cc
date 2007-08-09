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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
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
