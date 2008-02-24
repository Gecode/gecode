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

#include "gecode/int/channel.hh"

namespace Gecode {

  void
  channel(Space* home, const IntVarArgs& x, const IntVarArgs& y,
          IntConLevel icl, PropKind) {
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
  channel(Space* home, BoolVar x0, IntVar x1, IntConLevel, PropKind) {
    using namespace Int;
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Channel::LinkSingle::post(home,x0,x1));
  }

  void
  channel(Space* home, const BoolVarArgs& x, IntVar y, int o, 
          IntConLevel, PropKind) {
    using namespace Int;
    if (x.same())
      throw ArgumentSame("Int::channel");
    Limits::check(o,"Int::channel");
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    GECODE_ES_FAIL(home,Channel::LinkMulti::post(home,xv,y,o));
  }

  namespace {
    using namespace Int;
    GECODE_REGISTER2(Channel::Dom<IntView, false>);
    GECODE_REGISTER2(Channel::Dom<IntView, true>);
    GECODE_REGISTER2(Channel::Val<IntView, false>);
    GECODE_REGISTER2(Channel::Val<IntView, true>);
    GECODE_REGISTER1(Channel::LinkSingle);
    GECODE_REGISTER1(Channel::LinkMulti);
  }

}

// STATISTICS: int-post
