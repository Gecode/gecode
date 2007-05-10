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
  count(Space* home, const IntVarArgs& x, int n,
        IntRelType r, int m, IntConLevel, PropVar) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    ConstIntView y(n);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqInt<IntView,ConstIntView>
                           ::post(home,xv,y,m)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqInt<IntView,ConstIntView>
                           ::post(home,xv,y,m)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqInt<IntView,ConstIntView>
                           ::post(home,xv,y,m-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqInt<IntView,ConstIntView>
                           ::post(home,xv,y,m)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqInt<IntView,ConstIntView>
                           ::post(home,xv,y,m+1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqInt<IntView,ConstIntView>
                           ::post(home,xv,y,m)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& x, IntVar y,
        IntRelType r, int m, IntConLevel, PropVar) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqInt<IntView,IntView>
                           ::post(home,xv,y,m)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqInt<IntView,IntView>
                           ::post(home,xv,y,m)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqInt<IntView,IntView>
                           ::post(home,xv,y,m-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqInt<IntView,IntView>
                           ::post(home,xv,y,m)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqInt<IntView,IntView>
                           ::post(home,xv,y,m+1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqInt<IntView,IntView>
                           ::post(home,xv,y,m)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& x, const IntArgs& y,
        IntRelType r, int m, IntConLevel, PropVar) {
    using namespace Int;
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::count");
    if (home->failed()) return;

    ViewArray<OffsetView> xy(home,x.size());
    for (int i=x.size(); i--; )
      xy[i].init(x[i],-y[i]);

    ZeroIntView z;
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqInt<OffsetView,ZeroIntView>
                           ::post(home,xy,z,m)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqInt<OffsetView,ZeroIntView>
                           ::post(home,xy,z,m)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqInt<OffsetView,ZeroIntView>
                           ::post(home,xy,z,m-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqInt<OffsetView,ZeroIntView>
                           ::post(home,xy,z,m)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqInt<OffsetView,ZeroIntView>
                           ::post(home,xy,z,m+1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqInt<OffsetView,ZeroIntView>
                           ::post(home,xy,z,m)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& x, int n,
        IntRelType r, IntVar z, IntConLevel, PropVar) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    ConstIntView yv(n);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqView<IntView,ConstIntView,IntView,true>
                           ::post(home,xv,yv,z,0)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqView<IntView,ConstIntView,IntView,true>
                           ::post(home,xv,yv,z,0)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqView<IntView,ConstIntView,IntView,true>
                           ::post(home,xv,yv,z,-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqView<IntView,ConstIntView,IntView,true>
                           ::post(home,xv,yv,z,0)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqView<IntView,ConstIntView,IntView,true>
                           ::post(home,xv,yv,z,1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqView<IntView,ConstIntView,IntView,true>
                           ::post(home,xv,yv,z,0)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& x, IntVar y,
        IntRelType r, IntVar z, IntConLevel, PropVar) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqView<IntView,IntView,IntView,true>
                           ::post(home,xv,y,z,0)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqView<IntView,IntView,IntView,true>
                           ::post(home,xv,y,z,0)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqView<IntView,IntView,IntView,true>
                           ::post(home,xv,y,z,-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqView<IntView,IntView,IntView,true>
                           ::post(home,xv,y,z,0)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqView<IntView,IntView,IntView,true>
                           ::post(home,xv,y,z,1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqView<IntView,IntView,IntView,true>
                           ::post(home,xv,y,z,0)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& x, const IntArgs& y,
        IntRelType r, IntVar z, IntConLevel, PropVar) {
    using namespace Int;
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::count");
    if (home->failed()) return;

    ViewArray<OffsetView> xy(home,x.size());
    for (int i=x.size(); i--; )
      xy[i].init(x[i],-y[i]);

    ZeroIntView u;
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Count::EqView<OffsetView,ZeroIntView,IntView,true>
                           ::post(home,xy,u,z,0)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Count::NqView<OffsetView,ZeroIntView,IntView,true>
                           ::post(home,xy,u,z,0)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Count::LqView<OffsetView,ZeroIntView,IntView,true>
                           ::post(home,xy,u,z,-1)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Count::LqView<OffsetView,ZeroIntView,IntView,true>
                           ::post(home,xy,u,z,0)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Count::GqView<OffsetView,ZeroIntView,IntView,true>
                           ::post(home,xy,u,z,1)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Count::GqView<OffsetView,ZeroIntView,IntView,true>
                           ::post(home,xy,u,z,0)));
      break;
    default:
      throw UnknownRelation("Int::count");
    }
  }


}

// STATISTICS: int-post
