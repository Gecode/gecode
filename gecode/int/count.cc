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
	IntRelType r, int z, IntConLevel icl) {
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
	IntRelType r, int z, IntConLevel icl) {
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

#define GECODE_INT_CREATE(VY,VZ,C) \
  GECODE_ES_FAIL(home,(C<IntView,VY,VZ,true>::post(home,x,y,z,c)));

  void
  count(Space* home, const IntVarArgs& xa, int yn,
	IntRelType r, IntVar z, IntConLevel icl) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> x(home,xa);
    ConstIntView y(yn);
    int c = 0;
    switch (r) {
    case IRT_EQ:
      GECODE_INT_CREATE(ConstIntView,IntView,Count::EqView); break;
    case IRT_NQ:
      GECODE_INT_CREATE(ConstIntView,IntView,Count::NqView); break;
    case IRT_LE:
      c = -1; // Fall through
    case IRT_LQ:
      GECODE_INT_CREATE(ConstIntView,IntView,Count::LqView); break;
    case IRT_GR:
      c = 1; // Fall through
    case IRT_GQ:
      GECODE_INT_CREATE(ConstIntView,IntView,Count::GqView); break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& xa, IntVar y,
	IntRelType r, IntVar z, IntConLevel icl) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> x(home,xa);
    int c = 0;
    switch (r) {
    case IRT_EQ:
      GECODE_INT_CREATE(IntView,IntView,Count::EqView); break;
    case IRT_NQ:
      GECODE_INT_CREATE(IntView,IntView,Count::NqView); break;
    case IRT_LE:
      c = -1; // Fall through
    case IRT_LQ:
      GECODE_INT_CREATE(IntView,IntView,Count::LqView); break;
    case IRT_GR:
      c = 1; // Fall through
    case IRT_GQ:
      GECODE_INT_CREATE(IntView,IntView,Count::GqView); break;
    default:
      throw UnknownRelation("Int::count");
    }
  }

}

#undef GECODE_INT_CREATE

// STATISTICS: int-post

