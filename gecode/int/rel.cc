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

#include "gecode/int/rel.hh"
#include "gecode/int/bool.hh"

#include <algorithm>

namespace Gecode {

  using namespace Int;

  void
  rel(Space* home, IntVar x0, IntRelType r, int n, IntConLevel) {
    if (home->failed()) return;
    IntView x(x0);
    switch (r) {
    case IRT_EQ:
      GECODE_ME_FAIL(home,x.eq(home,n)); break;
    case IRT_NQ:
      GECODE_ME_FAIL(home,x.nq(home,n)); break;
    case IRT_LQ:
      GECODE_ME_FAIL(home,x.lq(home,n)); break;
    case IRT_LE:
      GECODE_ME_FAIL(home,x.le(home,n)); break;
    case IRT_GQ:
      GECODE_ME_FAIL(home,x.gq(home,n)); break;
    case IRT_GR:
      GECODE_ME_FAIL(home,x.gr(home,n)); break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space* home, BoolVar x0, IntRelType r, int n, IntConLevel) {
    if (home->failed()) return;
    BoolView x(x0);
    if (n == 0) {
      switch (r) {
      case IRT_LQ:
      case IRT_EQ:
        GECODE_ME_FAIL(home,x.zero(home)); break;
      case IRT_NQ:
      case IRT_GR:
        GECODE_ME_FAIL(home,x.one(home)); break;
      case IRT_LE:
        home->fail(); break;
      case IRT_GQ:
        break;
      default:
        throw UnknownRelation("Int::rel");
      }
    } else if (n == 1) {
      switch (r) {
      case IRT_GQ:
      case IRT_EQ:
        GECODE_ME_FAIL(home,x.one(home)); break;
      case IRT_NQ:
      case IRT_LE:
        GECODE_ME_FAIL(home,x.zero(home)); break;
      case IRT_GR:
        home->fail(); break;
      case IRT_LQ:
        break;
      default:
        throw UnknownRelation("Int::rel");
      }
    } else {
      home->fail();
    }
  }

  void
  rel(Space* home, IntVar x0, IntRelType r, IntVar x1, IntConLevel icl) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      if (icl == ICL_BND) {
        GECODE_ES_FAIL(home,(Rel::EqBnd<IntView,IntView>::post(home,x0,x1)));
      } else {
        GECODE_ES_FAIL(home,(Rel::EqDom<IntView,IntView>::post(home,x0,x1)));
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
  rel(Space* home, BoolVar x0, IntRelType r, BoolVar x1, IntConLevel) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>::post(home,x0,x1)));
      break;
    case IRT_NQ: 
      {
        NegBoolView n1(x1);
        GECODE_ES_FAIL(home,(Bool::Eq<BoolView,NegBoolView>::post(home,x0,n1)));
      }
      break;
    case IRT_GQ:
      std::swap(x0,x1); // Fall through
    case IRT_LQ:
      GECODE_ES_FAIL(home,Rel::Lq<BoolView>::post(home,x0,x1)); break;
    case IRT_GR:
      {
        BoolView b0(x0); BoolView b1(x1);
        GECODE_ME_FAIL(home,b0.one(home));
        GECODE_ME_FAIL(home,b1.zero(home));
      }
      break;
    case IRT_LE:
      {
        BoolView b0(x0); BoolView b1(x1);
        GECODE_ME_FAIL(home,b0.zero(home));
        GECODE_ME_FAIL(home,b1.one(home));
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }


  void
  rel(Space* home, IntVar x0, IntRelType r, IntVar x1, BoolVar b,
      IntConLevel icl) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      if (icl == ICL_BND) {
        GECODE_ES_FAIL(home,(Rel::ReEqBnd<IntView,BoolView>
                             ::post(home,x0,x1,b)));
      } else {
        GECODE_ES_FAIL(home,(Rel::ReEqDom<IntView,BoolView>
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
  rel(Space* home, IntVar x, IntRelType r, int n, BoolVar b,
      IntConLevel icl) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      if (icl == ICL_BND) {
        GECODE_ES_FAIL(home,(Rel::ReEqBndInt<IntView,BoolView>
                             ::post(home,x,n,b)));
      } else {
        GECODE_ES_FAIL(home,(Rel::ReEqDomInt<IntView,BoolView>
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
  rel(Space* home, const IntVarArgs& x, IntRelType r, IntConLevel icl) {
    if (home->failed() || (x.size() < 2)) return;
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<IntView> xv(home,x);
        if (icl == ICL_BND) {
          GECODE_ES_FAIL(home,Rel::NaryEqBnd<IntView>::post(home,xv));
        } else {
          GECODE_ES_FAIL(home,Rel::NaryEqDom<IntView>::post(home,xv));
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
  rel(Space* home, const BoolVarArgs& x, IntRelType r, IntConLevel icl) {
    if (home->failed() || (x.size() < 2)) return;
    switch (r) {
    case IRT_EQ:
      for (int i=x.size()-1; i--; )
        GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>::post(home,x[i],x[i+1])));
      break;
    case IRT_NQ:
      if (x.size() == 2) {
        NegBoolView n(x[1]);
        GECODE_ES_FAIL(home,(Bool::Eq<BoolView,NegBoolView>::post(home,x[0],n)));
      } else {
        home->fail();
      }
      break;
    case IRT_LE:
      if (x.size() == 2) {
        BoolView b0(x[0]); BoolView b1(x[1]);
        GECODE_ME_FAIL(home,b0.zero(home));
        GECODE_ME_FAIL(home,b1.one(home));
      } else {
        home->fail();
      }
      break;
    case IRT_LQ:
      for (int i=x.size()-1; i--; )
        GECODE_ES_FAIL(home,Rel::Lq<BoolView>::post(home,x[i],x[i+1]));
      break;
    case IRT_GR:
      if (x.size() == 2) {
        BoolView b0(x[0]); BoolView b1(x[1]);
        GECODE_ME_FAIL(home,b0.one(home));
        GECODE_ME_FAIL(home,b1.zero(home));
      } else {
        home->fail();
      }
      break;
    case IRT_GQ:
      for (int i=x.size()-1; i--; )
        GECODE_ES_FAIL(home,Rel::Lq<BoolView>::post(home,x[i+1],x[i]));
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space* home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntConLevel icl) {
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::rel");
    if (home->failed()) return;

    switch (r) {
    case IRT_GR: 
      {
        ViewArray<ViewTuple<IntView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=y[i]; xy[i][1]=x[i];
        }
        GECODE_ES_FAIL(home,Rel::Lex<IntView>::post(home,xy,true));
      }
      break;
    case IRT_LE: 
      {
        ViewArray<ViewTuple<IntView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=x[i]; xy[i][1]=y[i];
        }
        GECODE_ES_FAIL(home,Rel::Lex<IntView>::post(home,xy,true));
      }
      break;
    case IRT_GQ: 
      {
        ViewArray<ViewTuple<IntView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=y[i]; xy[i][1]=x[i];
        }
        GECODE_ES_FAIL(home,Rel::Lex<IntView>::post(home,xy,false));
      }
      break;
    case IRT_LQ: 
      {
        ViewArray<ViewTuple<IntView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=x[i]; xy[i][1]=y[i];
        }
        GECODE_ES_FAIL(home,Rel::Lex<IntView>::post(home,xy,false));
      }
      break;
    case IRT_EQ:
      if (icl == ICL_BND)
        for (int i=x.size(); i--; ) {
          GECODE_ES_FAIL(home,(Rel::EqBnd<IntView,IntView>
                               ::post(home,x[i],y[i])));
        }
      else
        for (int i=x.size(); i--; ) {
          GECODE_ES_FAIL(home,(Rel::EqDom<IntView,IntView>
                               ::post(home,x[i],y[i])));
        }
      break;
    case IRT_NQ: 
      {
        ViewArray<ViewTuple<IntView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=x[i]; xy[i][1]=y[i];
        }
        GECODE_ES_FAIL(home,Rel::NaryNq<IntView>::post(home,xy));
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space* home, const BoolVarArgs& x, IntRelType r, const BoolVarArgs& y,
      IntConLevel) {
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::rel");
    if (home->failed()) return;

    switch (r) {
    case IRT_GR: 
      {
        ViewArray<ViewTuple<BoolView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=y[i]; xy[i][1]=x[i];
        }
        GECODE_ES_FAIL(home,Rel::Lex<BoolView>::post(home,xy,true));
      }
      break;
    case IRT_LE: 
      {
        ViewArray<ViewTuple<BoolView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=x[i]; xy[i][1]=y[i];
        }
        GECODE_ES_FAIL(home,Rel::Lex<BoolView>::post(home,xy,true));
      }
      break;
    case IRT_GQ: 
      {
        ViewArray<ViewTuple<BoolView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=y[i]; xy[i][1]=x[i];
        }
        GECODE_ES_FAIL(home,Rel::Lex<BoolView>::post(home,xy,false));
      }
      break;
    case IRT_LQ: 
      {
        ViewArray<ViewTuple<BoolView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=x[i]; xy[i][1]=y[i];
        }
        GECODE_ES_FAIL(home,Rel::Lex<BoolView>::post(home,xy,false));
      }
      break;
    case IRT_EQ:
      for (int i=x.size(); i--; ) {
        GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>
                             ::post(home,x[i],y[i])));
      }
      break;
    case IRT_NQ: 
      {
        ViewArray<ViewTuple<BoolView,2> > xy(home,x.size());
        for (int i = x.size(); i--; ) {
          xy[i][0]=x[i]; xy[i][1]=y[i];
        }
        GECODE_ES_FAIL(home,Rel::NaryNq<BoolView>::post(home,xy));
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

}

// STATISTICS: int-post

