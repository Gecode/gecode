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

#include "gecode/int/count.hh"

namespace Gecode {

  void
  count(Space* home, const IntVarArgs& x, int n,
        IntRelType r, int m, IntConLevel, PropKind) {
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
        IntRelType r, int m, IntConLevel, PropKind) {
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
        IntRelType r, int m, IntConLevel, PropKind) {
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
        IntRelType r, IntVar z, IntConLevel, PropKind) {
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
        IntRelType r, IntVar z, IntConLevel, PropKind) {
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
        IntRelType r, IntVar z, IntConLevel, PropKind) {
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

  namespace {
    using namespace Int;
    template <template<class,class> class P>
    class I {
      GECODE_REGISTER2(P<IntView,ConstIntView>);      
      GECODE_REGISTER2(P<IntView,IntView>);      
      GECODE_REGISTER2(P<OffsetView,ZeroIntView>);
    };
    I<Count::EqInt> ieq;
    I<Count::LqInt> ilq;
    I<Count::GqInt> igq;
    I<Count::NqInt> inq;
    
    template <template <class,class,class,bool> class P,bool b>
    class VB {
      GECODE_REGISTER4(P<OffsetView,ZeroIntView,IntView,b>);
      GECODE_REGISTER4(P<IntView,ConstIntView,IntView,b>);
      GECODE_REGISTER4(P<IntView,IntView,IntView,b>);
    };
    template <template <class,class,class,bool> class P>
    class V {
      VB<P,false> vf;
      VB<P,true> vt;
    };
    V<Count::EqView> v1;
    V<Count::LqView> v2;
    V<Count::GqView> v3;
    VB<Count::NqView,true> v4;
  }
}

// STATISTICS: int-post
