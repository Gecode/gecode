/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

#include "gecode/int/channel.hh"

namespace Gecode {

  void
  channel(Space* home, const IntVarArgs& x, const IntVarArgs& y,
          IntConLevel icl) {
    using namespace Int;
    using namespace Channel;
    int n = x.size();
    if (n != y.size())
      throw ArgumentSizeMismatch("Int::channel");
    if (x.same() || y.same())
      throw ArgumentSame("Int::channel");
    if (home->failed()) return;
    if (n == 0)
      return;

    if (icl == ICL_DOM) {
      DomInfo<IntView>* di
        = DomInfo<IntView>::allocate(home,2*n);
      for (int i=n; i--; ) {
        di[i  ].init(x[i],n);
        di[i+n].init(y[i],n);
      }
      if (x.same(y)) {
        GECODE_ES_FAIL(home,(Dom<IntView,true>::post(home,n,di)));
      } else {
        GECODE_ES_FAIL(home,(Dom<IntView,false>::post(home,n,di)));
      }
    } else {
      ValInfo<IntView>* vi
        = ValInfo<IntView>::allocate(home,2*n);
      for (int i=n; i--; ) {
        vi[i  ].init(x[i],n);
        vi[i+n].init(y[i],n);
      }
      if (x.same(y)) {
        GECODE_ES_FAIL(home,(Val<IntView,true>::post(home,n,vi)));
      } else {
        GECODE_ES_FAIL(home,(Val<IntView,false>::post(home,n,vi)));
      }
    }
  }

  void
  channel(Space* home, BoolVar x0, IntVar x1, IntConLevel) {
    using namespace Int;
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Channel::LinkSingle::post(home,x0,x1));
  }

  void
  channel(Space* home, IntVar x0, BoolVar x1, IntConLevel) {
    using namespace Int;
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Channel::LinkSingle::post(home,x1,x0));
  }

  void
  channel(Space* home, const BoolVarArgs& x, IntVar y, int o, IntConLevel) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    GECODE_ES_FAIL(home,Channel::LinkMulti::post(home,xv,y,o));
  }

}

// STATISTICS: int-post
