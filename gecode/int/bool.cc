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

#include "gecode/int/bool.hh"

namespace Gecode {

  using namespace Int;

  void
  rel(Space* home, BoolVar x0, IntRelType r, BoolVar x1, IntConLevel) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>
                           ::post(home,x0,x1)));
      break;
    case IRT_NQ: 
      {
        NegBoolView n1(x1);
        GECODE_ES_FAIL(home,(Bool::Eq<BoolView,NegBoolView>
                             ::post(home,x0,n1)));
      }
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,Bool::Lq<BoolView>::post(home,x1,x0)); 
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,Bool::Lq<BoolView>::post(home,x0,x1)); 
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,Bool::Le<BoolView>::post(home,x1,x0)); 
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,Bool::Le<BoolView>::post(home,x0,x1)); 
      break;
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
      throw NotZeroOne("Int::rel");
    }
  }

  void
  rel(Space* home, BoolVar x0, BoolOpType o, BoolVar x1, BoolVar x2, 
      IntConLevel) {
    if (home->failed()) return;
    switch (o) {
    case BOT_AND:
      {
        NegBoolView n0(x0); NegBoolView n1(x1); NegBoolView n2(x2);
        GECODE_ES_FAIL(home,(Bool::Or<NegBoolView,NegBoolView,NegBoolView>
                             ::post(home,n0,n1,n2)));
      }
      break;
    case BOT_OR:
      GECODE_ES_FAIL(home,(Bool::Or<BoolView,BoolView,BoolView>
                           ::post(home,x0,x1,x2)));
      break;
    case BOT_IMP:
      {
        NegBoolView n0(x0);
        GECODE_ME_FAIL(home,(Bool::Or<NegBoolView,BoolView,BoolView>
                             ::post(home,n0,x1,x2)));
      }
      break;
    case BOT_EQV:
      GECODE_ES_FAIL(home,(Bool::Eqv<BoolView,BoolView,BoolView>
                           ::post(home,x0,x1,x2)));
      break;
    case BOT_XOR:
      {
        NegBoolView n2(x2);
        GECODE_ES_FAIL(home,(Bool::Eqv<BoolView,BoolView,NegBoolView>
                             ::post(home,x0,x1,n2)));
      }
      break;
    default:
      throw UnknownBoolOp("Int::rel");
    }
  }

  void
  rel(Space* home, BoolVar x0, BoolOpType o, BoolVar x1, int n, 
      IntConLevel) {
    if (home->failed()) return;
    if (n == 0) {
      switch (o) {
      case BOT_AND:
        {
          NegBoolView n0(x0); NegBoolView n1(x1);
          GECODE_ES_FAIL(home,(Bool::BinOrTrue<NegBoolView,NegBoolView>
                               ::post(home,n0,n1)));
        }
        break;
      case BOT_OR:
        {
          BoolView b0(x0); BoolView b1(x1);
          GECODE_ME_FAIL(home,b0.zero(home));
          GECODE_ME_FAIL(home,b1.zero(home));
        }
        break;
      case BOT_IMP:
        {
          BoolView b0(x0); BoolView b1(x1);
          GECODE_ME_FAIL(home,b0.one(home));
          GECODE_ME_FAIL(home,b1.zero(home));
        }
        break;
      case BOT_EQV:
        {
          NegBoolView n0(x0);
          GECODE_ES_FAIL(home,(Bool::Eq<NegBoolView,BoolView>
                               ::post(home,n0,x1)));
        }
        break;
      case BOT_XOR:
        GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>
                             ::post(home,x0,x1)));
        break;
      default:
        throw UnknownBoolOp("Int::rel");
      }
    } else if (n == 1) {
      switch (o) {
      case BOT_AND:
        {
          BoolView b0(x0); BoolView b1(x1);
          GECODE_ME_FAIL(home,b0.one(home));
          GECODE_ME_FAIL(home,b1.one(home));
        }
        break;
      case BOT_OR:
        GECODE_ES_FAIL(home,(Bool::BinOrTrue<BoolView,BoolView>
                             ::post(home,x0,x1)));
        break;
      case BOT_IMP:
        {
          NegBoolView n0(x0);
          GECODE_ES_FAIL(home,(Bool::BinOrTrue<NegBoolView,BoolView>
                               ::post(home,n0,x1)));
        }
        break;
      case BOT_EQV:
        GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>
                             ::post(home,x0,x1)));
        break;
      case BOT_XOR:
        {
          NegBoolView n0(x0);
          GECODE_ES_FAIL(home,(Bool::Eq<NegBoolView,BoolView>
                               ::post(home,n0,x1)));
        }
        break;
      default:
        throw UnknownBoolOp("Int::rel");
      }
    } else {
      throw NotZeroOne("Int::rel");
    }
  }


  void
  bool_and(Space* home, const BoolVarArgs& b, BoolVar c, IntConLevel) {
    if (home->failed()) return;
    ViewArray<NegBoolView> x(home,b.size());
    for (int i=b.size(); i--; ) {
      NegBoolView nb(b[i]); x[i]=nb;
    }
    NegBoolView nc(c);
    GECODE_ES_FAIL(home,Bool::NaryOr<NegBoolView>::post(home,x,nc));
  }
  void
  bool_and(Space* home, const BoolVarArgs& b, bool c, IntConLevel) {
    if (home->failed()) return;
    if (c) {
      for (int i=b.size(); i--; ) {
        BoolView bvi(b[i]); GECODE_ME_FAIL(home,bvi.one(home));
      }
    } else {
      ViewArray<NegBoolView> x(home,b.size());
      for (int i=b.size(); i--; ) {
        NegBoolView nb(b[i]); x[i]=nb;
      }
      GECODE_ES_FAIL(home,Bool::NaryOrTrue<NegBoolView>::post(home,x));
    }
  }

  void
  bool_or(Space* home, const BoolVarArgs& b, BoolVar c, IntConLevel) {
    if (home->failed()) return;
    ViewArray<BoolView> x(home,b);
    GECODE_ES_FAIL(home,Bool::NaryOr<BoolView>::post(home,x,c));
  }
  void
  bool_or(Space* home, const BoolVarArgs& b, bool c, IntConLevel) {
    if (home->failed()) return;
    if (c) {
      ViewArray<BoolView> x(home,b);
      GECODE_ES_FAIL(home,Bool::NaryOrTrue<BoolView>::post(home,x));
    } else {
      for (int i=b.size(); i--; ) {
        BoolView bvi(b[i]);
        GECODE_ME_FAIL(home,bvi.zero(home));
      }
    }
  }

}


// STATISTICS: int-post

