/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include "gecode/int/element.hh"

namespace Gecode {

  using namespace Int;

  void
  element(Space* home, const IntArgs& c, IntVar x0, IntVar x1,
          int offset, IntConLevel, PropKind) {
    if (home->failed()) return;
    Element::IntSharedArray cs(c.size());
    for (int i = c.size(); i--; )
      if ((c[i] < Limits::Int::int_min) || (c[i] > Limits::Int::int_max))
        throw NumericalOverflow("Int::element");
      else
        cs[i] = c[i];
    if (offset==0) {
      GECODE_ES_FAIL(home,(Element::Int<IntView,IntView>::post(home,cs,x0,x1)));
    } else {
      OffsetView ov(IntView(x0), offset);
      GECODE_ES_FAIL(home,(Element::Int<OffsetView,IntView>::post(home,cs,ov,x1)));
    }
  }

  void
  element(Space* home, const IntArgs& c, IntVar x0, BoolVar x1,
          int offset, IntConLevel, PropKind) {
    if (home->failed()) return;
    Element::IntSharedArray cs(c.size());
    for (int i = c.size(); i--; )
      if ((c[i] < Limits::Int::int_min) || (c[i] > Limits::Int::int_max))
        throw NumericalOverflow("Int::element");
      else
        cs[i] = c[i];
    if (offset==0) {
      GECODE_ES_FAIL(home,(Element::Int<IntView,BoolView>::post(home,cs,x0,x1)));
    } else {
      OffsetView ov(IntView(x0), offset);
      GECODE_ES_FAIL(home,(Element::Int<IntView,BoolView>::post(home,cs,x0,x1)));
    }
  }

  void
  element(Space* home, const IntArgs& c, IntVar x0, int x1,
          int offset, IntConLevel, PropKind) {
    if (home->failed()) return;
    Element::IntSharedArray cs(c.size());
    for (int i = c.size(); i--; )
      if ((c[i] < Limits::Int::int_min) || (c[i] > Limits::Int::int_max))
        throw NumericalOverflow("Int::element");
      else
        cs[i] = c[i];
    ConstIntView cx1(x1);
    if (offset==0) {
      GECODE_ES_FAIL(home,(Element::Int<IntView,ConstIntView>
                           ::post(home,cs,x0,cx1)));
    } else {
      OffsetView ov(IntView(x0), offset);
      GECODE_ES_FAIL(home,(Element::Int<OffsetView,ConstIntView>
                           ::post(home,cs,ov,cx1)));
    }
  }

  void
  element(Space* home, const IntVarArgs& c, IntVar x0, IntVar x1,
          int offset, IntConLevel icl, PropKind) {
    if (home->failed()) return;
    Element::IdxView<IntView>* iv = 
      Element::IdxView<IntView>::init(home,c);
    if ((icl == ICL_DOM) || (icl == ICL_DEF)) {
      if (offset==0) {
        GECODE_ES_FAIL(home,(Element::ViewDom<IntView,IntView,IntView>
                             ::post(home,iv,c.size(),x0,x1)));
      } else {
        OffsetView ov(IntView(x0), offset);
        GECODE_ES_FAIL(home,(Element::ViewDom<IntView,OffsetView,IntView>
                             ::post(home,iv,c.size(),ov,x1)));        
      }
    } else {
      if (offset==0) {
        GECODE_ES_FAIL(home,(Element::ViewBnd<IntView,IntView,IntView>
                             ::post(home,iv,c.size(),x0,x1)));
      } else {
        OffsetView ov(IntView(x0), offset);
        GECODE_ES_FAIL(home,(Element::ViewBnd<IntView,OffsetView,IntView>
                             ::post(home,iv,c.size(),ov,x1)));        
      }
    }
  }

  void
  element(Space* home, const IntVarArgs& c, IntVar x0, int x1,
          int offset, IntConLevel icl, PropKind) {
    if (home->failed()) return;
    Element::IdxView<IntView>* iv = 
      Element::IdxView<IntView>::init(home,c);
    ConstIntView v1(x1);
    if ((icl == ICL_DOM) || (icl == ICL_DEF)) {
      if (offset==0) {
        GECODE_ES_FAIL(home,(Element::ViewDom<IntView,IntView,ConstIntView>
                             ::post(home,iv,c.size(),x0,v1)));
      } else {
        OffsetView ov(IntView(x0), offset);
        GECODE_ES_FAIL(home,(Element::ViewDom<IntView,OffsetView,ConstIntView>
                             ::post(home,iv,c.size(),ov,v1)));        
      }
    } else {
      if (offset==0) {
        GECODE_ES_FAIL(home,(Element::ViewBnd<IntView,IntView,ConstIntView>
                             ::post(home,iv,c.size(),x0,v1)));
      } else {
        OffsetView ov(IntView(x0), offset);
        GECODE_ES_FAIL(home,(Element::ViewBnd<IntView,OffsetView,ConstIntView>
                             ::post(home,iv,c.size(),ov,v1)));        
      }
    }
  }

  void
  element(Space* home, const BoolVarArgs& c, IntVar x0, BoolVar x1,
          int offset, IntConLevel, PropKind) {
    if (home->failed()) return;
    Element::IdxView<BoolView>* iv = 
      Element::IdxView<BoolView>::init(home,c);
    if (offset==0) {
      GECODE_ES_FAIL(home,(Element::ViewBnd<BoolView,IntView,BoolView>
                           ::post(home,iv,c.size(),x0,x1)));
    } else {
      OffsetView ov(IntView(x0), offset);
      GECODE_ES_FAIL(home,(Element::ViewBnd<BoolView,OffsetView,BoolView>
                           ::post(home,iv,c.size(),ov,x1)));      
    }
  }

  void
  element(Space* home, const BoolVarArgs& c, IntVar x0, int x1,
          int offset, IntConLevel, PropKind) {
    if (home->failed()) return;
    Element::IdxView<BoolView>* iv = 
      Element::IdxView<BoolView>::init(home,c);
    ConstIntView v1(x1);
    if (offset==0) {
      GECODE_ES_FAIL(home,(Element::ViewBnd<BoolView,IntView,ConstIntView>
                           ::post(home,iv,c.size(),x0,v1)));
    } else {
      OffsetView ov(IntView(x0), offset);
      GECODE_ES_FAIL(home,(Element::ViewBnd<BoolView,OffsetView,ConstIntView>
                           ::post(home,iv,c.size(),ov,v1)));
    }
  }

  namespace {
    using namespace Int;
    GECODE_REGISTER2(Element::Int<OffsetView,ConstIntView>);
    GECODE_REGISTER2(Element::Int<OffsetView,IntView>);
    GECODE_REGISTER2(Element::Int<IntView,ConstIntView>);
    GECODE_REGISTER2(Element::Int<IntView,BoolView>);
    GECODE_REGISTER2(Element::Int<IntView,IntView>);

    GECODE_REGISTER3(Element::ViewBnd<IntView,OffsetView,ConstIntView>);
    GECODE_REGISTER3(Element::ViewBnd<IntView,OffsetView,IntView>);
    GECODE_REGISTER3(Element::ViewBnd<IntView,IntView,ConstIntView>);
    GECODE_REGISTER3(Element::ViewBnd<IntView,IntView,IntView>);
    GECODE_REGISTER3(Element::ViewBnd<BoolView,OffsetView,ConstIntView>);
    GECODE_REGISTER3(Element::ViewBnd<BoolView,OffsetView,BoolView>);
    GECODE_REGISTER3(Element::ViewBnd<BoolView,IntView,ConstIntView>);
    GECODE_REGISTER3(Element::ViewBnd<BoolView,IntView,BoolView>);

    GECODE_REGISTER3(Element::ViewDom<IntView,OffsetView,ConstIntView>);
    GECODE_REGISTER3(Element::ViewDom<IntView,OffsetView,IntView>);
    GECODE_REGISTER3(Element::ViewDom<IntView,IntView,ConstIntView>);
    GECODE_REGISTER3(Element::ViewDom<IntView,IntView,IntView>);
  }

}

// STATISTICS: int-post

