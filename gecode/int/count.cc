/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include "gecode/int/count.hh"

namespace Gecode {

  void
  count(Space* home, const IntVarArgs& xa, int y,
        IntRelType r, int z, IntConLevel) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> x(home,xa);
    ConstIntView yv(y);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqInt<IntView,ConstIntView>
                                 ::post(home,x,yv,z)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqInt<IntView,ConstIntView>
                                 ::post(home,x,yv,z)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqInt<IntView,ConstIntView>
                                 ::post(home,x,yv,z-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqInt<IntView,ConstIntView>
                                 ::post(home,x,yv,z)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqInt<IntView,ConstIntView>
                                 ::post(home,x,yv,z+1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqInt<IntView,ConstIntView>
                                 ::post(home,x,yv,z)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& xa, IntVar y,
        IntRelType r, int z, IntConLevel) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> x(home,xa);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqInt<IntView,IntView>
                                 ::post(home,x,y,z)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqInt<IntView,IntView>
                                 ::post(home,x,y,z)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqInt<IntView,IntView>
                                 ::post(home,x,y,z-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqInt<IntView,IntView>
                                 ::post(home,x,y,z)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqInt<IntView,IntView>
                                 ::post(home,x,y,z+1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqInt<IntView,IntView>
                                 ::post(home,x,y,z)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& xa, int y,
        IntRelType r, IntVar z, IntConLevel) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> x(home,xa);
    ConstIntView yv(y);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqView<IntView,ConstIntView,IntView,true>
                           ::post(home,x,yv,z,0)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqView<IntView,ConstIntView,IntView,true>
                           ::post(home,x,yv,z,0)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqView<IntView,ConstIntView,IntView,true>
                           ::post(home,x,yv,z,-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqView<IntView,ConstIntView,IntView,true>
                           ::post(home,x,yv,z,0)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqView<IntView,ConstIntView,IntView,true>
                           ::post(home,x,yv,z,1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqView<IntView,ConstIntView,IntView,true>
                           ::post(home,x,yv,z,0)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& xa, IntVar y,
        IntRelType r, IntVar z, IntConLevel) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> x(home,xa);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqView<IntView,IntView,IntView,true>
                           ::post(home,x,y,z,0)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqView<IntView,IntView,IntView,true>
                           ::post(home,x,y,z,0)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqView<IntView,IntView,IntView,true>
                           ::post(home,x,y,z,-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqView<IntView,IntView,IntView,true>
                           ::post(home,x,y,z,0)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqView<IntView,IntView,IntView,true>
                           ::post(home,x,y,z,1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqView<IntView,IntView,IntView,true>
                           ::post(home,x,y,z,0)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

}

// STATISTICS: int-post
