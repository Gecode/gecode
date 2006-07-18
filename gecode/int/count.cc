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

#define GECODE_INT_CREATE(VY,VZ,C) \
  GECODE_ES_FAIL(home,(C<IntView,VY,VZ,true>::post(home,x,y,z,c)));

  void
  count(Space* home, const IntVarArgs& xa, int yn,
	IntRelType r, int zn, IntConLevel icl) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> x(home,xa);
    ConstIntView y(yn);
    ConstIntView z(zn);
    int c = 0;
    switch (r) {
    case IRT_EQ: 
      GECODE_INT_CREATE(ConstIntView,ConstIntView,Count::Eq); break;
    case IRT_NQ: 
      GECODE_INT_CREATE(ConstIntView,ConstIntView,Count::Nq); break;
    case IRT_LE: 
      c = 1; // Fall through
    case IRT_LQ: 
      GECODE_INT_CREATE(ConstIntView,ConstIntView,Count::Lq); break;
    case IRT_GR: 
      c = -1; // Fall through
    case IRT_GQ: 
      GECODE_INT_CREATE(ConstIntView,ConstIntView,Count::Gq); break;
    default: 
      throw UnknownRelation("Int::count");
    }
  }

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
      GECODE_INT_CREATE(ConstIntView,IntView,Count::Eq); break;
    case IRT_NQ: 
      GECODE_INT_CREATE(ConstIntView,IntView,Count::Nq); break;
    case IRT_LE: 
      c = 1; // Fall through
    case IRT_LQ: 
      GECODE_INT_CREATE(ConstIntView,IntView,Count::Lq); break;
    case IRT_GR: 
      c = -1; // Fall through
    case IRT_GQ: 
      GECODE_INT_CREATE(ConstIntView,IntView,Count::Gq); break;
    default: 
      throw UnknownRelation("Int::count");
    }
  }

  void
  count(Space* home, const IntVarArgs& xa, IntVar y,
	IntRelType r, int zn, IntConLevel icl) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> x(home,xa);
    ConstIntView z(zn);
    int c = 0;
    switch (r) {
    case IRT_EQ: 
      GECODE_INT_CREATE(IntView,ConstIntView,Count::Eq); break;
    case IRT_NQ: 
      GECODE_INT_CREATE(IntView,ConstIntView,Count::Nq); break;
    case IRT_LE: 
      c = 1; // Fall through
    case IRT_LQ: 
      GECODE_INT_CREATE(IntView,ConstIntView,Count::Lq); break;
    case IRT_GR: 
      c = -1; // Fall through
    case IRT_GQ: 
      GECODE_INT_CREATE(IntView,ConstIntView,Count::Gq); break;
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
      GECODE_INT_CREATE(IntView,IntView,Count::Eq); break;
    case IRT_NQ: 
      GECODE_INT_CREATE(IntView,IntView,Count::Nq); break;
    case IRT_LE: 
      c = 1; // Fall through
    case IRT_LQ: 
      GECODE_INT_CREATE(IntView,IntView,Count::Lq); break;
    case IRT_GR: 
      c = -1; // Fall through
    case IRT_GQ: 
      GECODE_INT_CREATE(IntView,IntView,Count::Gq); break;
    default: 
      throw UnknownRelation("Int::count");
    }
  }

}

#undef GECODE_INT_CREATE

// STATISTICS: int-post

