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

#include <gecode/int/rel.hh>
#include <gecode/int/bool.hh>

#include <algorithm>

namespace Gecode {

  using namespace Int;

  void
  rel(Space& home, IntVar x0, IntRelType r, int n, 
      IntConLevel, PropKind) {
    Limits::check(n,"Int::rel");
    if (home.failed()) return;
    IntView x(x0);
    switch (r) {
    case IRT_EQ: GECODE_ME_FAIL(home,x.eq(home,n)); break;
    case IRT_NQ: GECODE_ME_FAIL(home,x.nq(home,n)); break;
    case IRT_LQ: GECODE_ME_FAIL(home,x.lq(home,n)); break;
    case IRT_LE: GECODE_ME_FAIL(home,x.le(home,n)); break;
    case IRT_GQ: GECODE_ME_FAIL(home,x.gq(home,n)); break;
    case IRT_GR: GECODE_ME_FAIL(home,x.gr(home,n)); break;
    default: throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space& home, const IntVarArgs& x, IntRelType r, int n, 
      IntConLevel, PropKind) {
    Limits::check(n,"Int::rel");
    if (home.failed()) return;
    switch (r) {
    case IRT_EQ: 
      for (int i=x.size(); i--; ) {
        IntView xi(x[i]); GECODE_ME_FAIL(home,xi.eq(home,n));
      }
      break;
    case IRT_NQ:
      for (int i=x.size(); i--; ) {
        IntView xi(x[i]); GECODE_ME_FAIL(home,xi.nq(home,n));
      }
      break;
    case IRT_LQ:
      for (int i=x.size(); i--; ) {
        IntView xi(x[i]); GECODE_ME_FAIL(home,xi.lq(home,n));
      }
      break;
    case IRT_LE:
      for (int i=x.size(); i--; ) {
        IntView xi(x[i]); GECODE_ME_FAIL(home,xi.le(home,n));
      }
      break;
    case IRT_GQ:
      for (int i=x.size(); i--; ) {
        IntView xi(x[i]); GECODE_ME_FAIL(home,xi.gq(home,n));
      }
      break;
    case IRT_GR:
      for (int i=x.size(); i--; ) {
        IntView xi(x[i]); GECODE_ME_FAIL(home,xi.gr(home,n));
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space& home, IntVar x0, IntRelType r, IntVar x1, 
      IntConLevel icl, PropKind) {
    if (home.failed()) return;
    switch (r) {
    case IRT_EQ:
      if ((icl == ICL_DOM) || (icl == ICL_DEF)) {
        GECODE_ES_FAIL(home,(Rel::EqDom<IntView,IntView>::post(home,x0,x1)));
      } else {
        GECODE_ES_FAIL(home,(Rel::EqBnd<IntView,IntView>::post(home,x0,x1)));
      }
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,Rel::Nq<IntView>::post(home,x0,x1)); break;
    case IRT_GQ:
      std::swap(x0,x1); // Fall through
    case IRT_LQ:
      GECODE_ES_FAIL(home,Rel::Lq<IntView>::post(home,x0,x1)); break;
    case IRT_GR:
      std::swap(x0,x1); // Fall through
    case IRT_LE:
      GECODE_ES_FAIL(home,Rel::Le<IntView>::post(home,x0,x1)); break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space& home, const IntVarArgs& x, IntRelType r, IntVar y, 
      IntConLevel icl, PropKind) {
    if (home.failed()) return;
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<IntView> xv(home,x.size()+1);
        xv[x.size()]=y;
        for (int i=x.size(); i--; )
          xv[i]=x[i];
        if ((icl == ICL_DOM) || (icl == ICL_DEF)) {
          GECODE_ES_FAIL(home,Rel::NaryEqDom<IntView>::post(home,xv));
        } else {
          GECODE_ES_FAIL(home,Rel::NaryEqBnd<IntView>::post(home,xv));
        }
      }
      break;
    case IRT_NQ:
      for (int i=x.size(); i--; ) {
        GECODE_ES_FAIL(home,Rel::Nq<IntView>::post(home,x[i],y)); 
      }
      break;
    case IRT_GQ:
      for (int i=x.size(); i--; ) {
        GECODE_ES_FAIL(home,Rel::Lq<IntView>::post(home,y,x[i])); 
      }
      break;
    case IRT_LQ:
      for (int i=x.size(); i--; ) {
        GECODE_ES_FAIL(home,Rel::Lq<IntView>::post(home,x[i],y)); 
      }
      break;
    case IRT_GR:
      for (int i=x.size(); i--; ) {
        GECODE_ES_FAIL(home,Rel::Le<IntView>::post(home,y,x[i])); 
      }
      break;
    case IRT_LE:
      for (int i=x.size(); i--; ) {
        GECODE_ES_FAIL(home,Rel::Le<IntView>::post(home,x[i],y)); 
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }


  void
  rel(Space& home, IntVar x0, IntRelType r, IntVar x1, BoolVar b,
      IntConLevel icl, PropKind) {
    if (home.failed()) return;
    switch (r) {
    case IRT_EQ:
      if ((icl == ICL_DOM) || (icl == ICL_DEF)) {
        GECODE_ES_FAIL(home,(Rel::ReEqDom<IntView,BoolView>
                             ::post(home,x0,x1,b)));
      } else {
        GECODE_ES_FAIL(home,(Rel::ReEqBnd<IntView,BoolView>
                             ::post(home,x0,x1,b)));
      }
      break;
    case IRT_NQ:
      {
        NegBoolView n(b);
        if (icl == ICL_BND) {
          GECODE_ES_FAIL(home,(Rel::ReEqBnd<IntView,NegBoolView>
                               ::post(home,x0,x1,n)));
        } else {
          GECODE_ES_FAIL(home,(Rel::ReEqDom<IntView,NegBoolView>
                               ::post(home,x0,x1,n)));
        }
      }
      break;
    case IRT_GQ:
      std::swap(x0,x1); // Fall through
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Rel::ReLq<IntView,BoolView>::post(home,x0,x1,b)));
      break;
    case IRT_LE:
      std::swap(x0,x1); // Fall through
    case IRT_GR:
      {
        NegBoolView n(b);
        GECODE_ES_FAIL(home,(Rel::ReLq<IntView,NegBoolView>::post(home,x0,x1,n)));
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space& home, IntVar x, IntRelType r, int n, BoolVar b,
      IntConLevel icl, PropKind) {
    Limits::check(n,"Int::rel");
    if (home.failed()) return;
    switch (r) {
    case IRT_EQ:
      if ((icl == ICL_DOM) || (icl == ICL_DEF)) {
        GECODE_ES_FAIL(home,(Rel::ReEqDomInt<IntView,BoolView>
                             ::post(home,x,n,b)));
      } else {
        GECODE_ES_FAIL(home,(Rel::ReEqBndInt<IntView,BoolView>
                             ::post(home,x,n,b)));
      }
      break;
    case IRT_NQ:
      {
        NegBoolView nb(b);
        if (icl == ICL_BND) {
          GECODE_ES_FAIL(home,(Rel::ReEqBndInt<IntView,NegBoolView>
                               ::post(home,x,n,nb)));
        } else {
          GECODE_ES_FAIL(home,(Rel::ReEqDomInt<IntView,NegBoolView>
                               ::post(home,x,n,nb)));
        }
      }
      break;
    case IRT_LE:
      n--; // Fall through
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Rel::ReLqInt<IntView,BoolView>
                           ::post(home,x,n,b)));
      break;
    case IRT_GQ:
      n--; // Fall through
    case IRT_GR:
      {
        NegBoolView nb(b);
        GECODE_ES_FAIL(home,(Rel::ReLqInt<IntView,NegBoolView>
                             ::post(home,x,n,nb)));
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space& home, const IntVarArgs& x, IntRelType r, 
      IntConLevel icl, PropKind) {
    if (home.failed() || (x.size() < 2)) return;
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<IntView> xv(home,x);
        if ((icl == ICL_DOM) || (icl == ICL_DEF)) {
          GECODE_ES_FAIL(home,Rel::NaryEqDom<IntView>::post(home,xv));
        } else {
          GECODE_ES_FAIL(home,Rel::NaryEqBnd<IntView>::post(home,xv));
        }
      }
      break;
    case IRT_NQ:
      distinct(home,x,icl);
      break;
    case IRT_LE:
      for (int i=x.size()-1; i--; )
        GECODE_ES_FAIL(home,Rel::Le<IntView>::post(home,x[i],x[i+1]));
      break;
    case IRT_LQ:
      for (int i=x.size()-1; i--; )
        GECODE_ES_FAIL(home,Rel::Lq<IntView>::post(home,x[i],x[i+1]));
      break;
    case IRT_GR:
      for (int i=x.size()-1; i--; )
        GECODE_ES_FAIL(home,Rel::Le<IntView>::post(home,x[i+1],x[i]));
      break;
    case IRT_GQ:
      for (int i=x.size()-1; i--; )
        GECODE_ES_FAIL(home,Rel::Lq<IntView>::post(home,x[i+1],x[i]));
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space& home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntConLevel icl, PropKind) {
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::rel");
    if (home.failed()) return;

    switch (r) {
    case IRT_GR: 
      {
        ViewArray<IntView> xv(home,x), yv(home,y);
        GECODE_ES_FAIL(home,Rel::Lex<IntView>::post(home,yv,xv,true));
      }
      break;
    case IRT_LE: 
      {
        ViewArray<IntView> xv(home,x), yv(home,y);
        GECODE_ES_FAIL(home,Rel::Lex<IntView>::post(home,xv,yv,true));
      }
      break;
    case IRT_GQ: 
      {
        ViewArray<IntView> xv(home,x), yv(home,y);
        GECODE_ES_FAIL(home,Rel::Lex<IntView>::post(home,yv,xv,false));
      }
      break;
    case IRT_LQ: 
      {
        ViewArray<IntView> xv(home,x), yv(home,y);
        GECODE_ES_FAIL(home,Rel::Lex<IntView>::post(home,xv,yv,false));
      }
      break;
    case IRT_EQ:
      if ((icl == ICL_DOM) || (icl == ICL_DEF))
        for (int i=x.size(); i--; ) {
          GECODE_ES_FAIL(home,(Rel::EqDom<IntView,IntView>
                               ::post(home,x[i],y[i])));
        }
      else
        for (int i=x.size(); i--; ) {
          GECODE_ES_FAIL(home,(Rel::EqBnd<IntView,IntView>
                               ::post(home,x[i],y[i])));
        }
      break;
    case IRT_NQ: 
      {
        ViewArray<BoolView> b(home,x.size());
        for (int i=x.size(); i--; ) {
          BoolVar bi(home,0,1); b[i]=bi;
          NegBoolView n(b[i]);
          GECODE_ES_FAIL(home,(Rel::ReEqDom<IntView,NegBoolView>
                               ::post(home,x[i],y[i],n)));
        }
        GECODE_ES_FAIL(home,Bool::NaryOrTrue<BoolView>::post(home,b));
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  namespace {
    using namespace Int;
    GECODE_REGISTER2(Rel::EqBnd<IntView,IntView>);
    GECODE_REGISTER2(Rel::EqBnd<IntView,ConstIntView>);
    GECODE_REGISTER2(Rel::EqBnd<BoolView,ConstIntView>);
    GECODE_REGISTER2(Rel::EqBnd<BoolView,BoolView>);
    GECODE_REGISTER2(Rel::EqBnd<MinusView,IntView>);
    GECODE_REGISTER2(Rel::EqBnd<MinusView,MinusView>);

    GECODE_REGISTER2(Rel::EqDom<IntView,IntView>);
    GECODE_REGISTER2(Rel::EqDom<IntView,ConstIntView>);
    GECODE_REGISTER2(Rel::EqDom<MinusView,IntView>);

    GECODE_REGISTER1(Rel::NaryEqBnd<IntView>);
    GECODE_REGISTER1(Rel::NaryEqDom<IntView>);

    GECODE_REGISTER2(Rel::ReEqDomInt<IntView,NegBoolView>);
    GECODE_REGISTER2(Rel::ReEqDomInt<IntView,BoolView>);
    GECODE_REGISTER2(Rel::ReEqDom<IntView,NegBoolView>);
    GECODE_REGISTER2(Rel::ReEqDom<IntView,BoolView>);

    GECODE_REGISTER2(Rel::ReEqBndInt<IntView,NegBoolView>);
    GECODE_REGISTER2(Rel::ReEqBndInt<IntView,BoolView>);
    GECODE_REGISTER2(Rel::ReEqBnd<IntView,NegBoolView>);
    GECODE_REGISTER2(Rel::ReEqBnd<IntView,BoolView>);

    GECODE_REGISTER1(Rel::Nq<BoolView>);
    GECODE_REGISTER1(Rel::Nq<IntView>);
    GECODE_REGISTER1(Rel::Nq<OffsetView>);

    GECODE_REGISTER1(Rel::Lq<BoolView>);
    GECODE_REGISTER1(Rel::Lq<IntView>);
    GECODE_REGISTER1(Rel::Lq<MinusView>);
    GECODE_REGISTER1(Rel::Le<BoolView>);
    GECODE_REGISTER1(Rel::Le<IntView>);
    GECODE_REGISTER2(Rel::ReLq<IntView, NegBoolView>);
    GECODE_REGISTER2(Rel::ReLq<IntView, BoolView>);
    GECODE_REGISTER2(Rel::ReLqInt<IntView, NegBoolView>);
    GECODE_REGISTER2(Rel::ReLqInt<IntView, BoolView>);

    GECODE_REGISTER1(Rel::Lex<BoolView>);
    GECODE_REGISTER1(Rel::Lex<IntView>);

  }

}

// STATISTICS: int-post

