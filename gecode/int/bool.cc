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
  bool_not(Space* home, BoolVar b0, BoolVar b1, IntConLevel) {
    if (home->failed()) return;
    NegBoolView n0(b0);
    GECODE_ES_FAIL(home,(Bool::Eq<NegBoolView,BoolView>::post(home,n0,b1)));
  }

  void
  bool_eq(Space* home, BoolVar b0, BoolVar b1, IntConLevel) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>::post(home,b0,b1)));
  }

  void
  bool_and(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    NegBoolView n0(b0); NegBoolView n1(b1); NegBoolView n2(b2);
    GECODE_ES_FAIL(home,(Bool::Or<NegBoolView,NegBoolView,NegBoolView>
                         ::post(home,n0,n1,n2)));
  }
  void
  bool_and(Space* home, BoolVar b0, BoolVar b1, bool b2, IntConLevel) {
    if (home->failed()) return;
    if (b2) {
      BoolView bv0(b0); BoolView bv1(b1);
      GECODE_ME_FAIL(home,bv0.one(home));
      GECODE_ME_FAIL(home,bv1.one(home));
    } else {
      NegBoolView n0(b0); NegBoolView n1(b1);
      GECODE_ES_FAIL(home,(Bool::BinOrTrue<NegBoolView,NegBoolView>
                           ::post(home,n0,n1)));
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
  bool_or(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,
                   (Bool::Or<BoolView,BoolView,BoolView>
                    ::post(home,b0,b1,b2)));
  }
  void
  bool_or(Space* home, BoolVar b0, BoolVar b1, bool b2, IntConLevel) {
    if (home->failed()) return;
    if (b2) {
      GECODE_ES_FAIL(home,
                     (Bool::BinOrTrue<BoolView,BoolView>::post(home,b0,b1)));
    } else {
      BoolView bv0(b0);
      BoolView bv1(b1);
      GECODE_ME_FAIL(home,bv0.zero(home));
      GECODE_ME_FAIL(home,bv1.zero(home));
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

  void
  bool_imp(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    NegBoolView n0(b0);
    GECODE_ME_FAIL(home,(Bool::Or<NegBoolView,BoolView,BoolView>
                         ::post(home,n0,b1,b2)));
  }
  void
  bool_imp(Space* home, BoolVar b0, BoolVar b1, bool b2, IntConLevel) {
    if (home->failed()) return;
    if (b2) {
      NegBoolView n0(b0);
      GECODE_ME_FAIL(home,(Bool::BinOrTrue<NegBoolView,BoolView>
                           ::post(home,n0,b1)));
    } else {
      BoolView bv0(b0); BoolView bv1(b1);
      GECODE_ME_FAIL(home,bv0.one(home));
      GECODE_ME_FAIL(home,bv1.zero(home));
    }
  }

  void
  bool_eqv(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,(Bool::Eqv<BoolView,BoolView,BoolView>
                         ::post(home,b0,b1,b2)));
  }
  void
  bool_eqv(Space* home, BoolVar b0, BoolVar b1, bool b2, IntConLevel) {
    if (home->failed()) return;
    if (b2) {
      GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>::post(home,b0,b1)));
    } else {
      NegBoolView n0(b0);
      GECODE_ES_FAIL(home,(Bool::Eq<NegBoolView,BoolView>::post(home,n0,b1)));
    }
  }

  void
  bool_xor(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    NegBoolView n2(b2);
    GECODE_ES_FAIL(home,(Bool::Eqv<BoolView,BoolView,NegBoolView>
                         ::post(home,b0,b1,n2)));
  }
  void
  bool_xor(Space* home, BoolVar b0, BoolVar b1, bool b2, IntConLevel) {
    if (home->failed()) return;
    if (b2) {
      NegBoolView n0(b0);
      GECODE_ES_FAIL(home,(Bool::Eq<NegBoolView,BoolView>::post(home,n0,b1)));
    } else {
      GECODE_ES_FAIL(home,(Bool::Eq<BoolView,BoolView>::post(home,b0,b1)));
    }
  }

}


// STATISTICS: int-post

