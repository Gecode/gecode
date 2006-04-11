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
  bool_not(Space* home, BoolVar b0, BoolVar b1, IntConLevel) {
    if (home->failed()) return;
    NegBoolView n1(b1);
    if (Bool::Eq<BoolView,NegBoolView>::post(home,b0,n1) == ES_FAILED)
      home->fail();
  }
  void
  bool_eq(Space* home, BoolVar b0, BoolVar b1, IntConLevel) {
    if (home->failed()) return;
    if (Bool::Eq<BoolView,BoolView>::post(home,b0,b1) == ES_FAILED)
      home->fail();
  }
  void
  bool_and(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    if (Bool::And<BoolView,BoolView,BoolView>::post(home,b0,b1,b2)
	== ES_FAILED)
      home->fail();
  }
  void
  bool_or(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    NegBoolView n0(b0); NegBoolView n1(b1); NegBoolView n2(b2);
    if (Bool::And<NegBoolView,NegBoolView,NegBoolView>::post(home,n0,n1,n2)
	== ES_FAILED)
      home->fail();
  }
  void
  bool_imp(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    NegBoolView n1(b1); NegBoolView n2(b2);
    if (Bool::And<BoolView,NegBoolView,NegBoolView>::post(home,b0,n1,n2)
	== ES_FAILED)
      home->fail();
  }
  void
  bool_eqv(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    if (Bool::Eqv<BoolView,BoolView,BoolView>::post(home,b0,b1,b2)
	== ES_FAILED)
      home->fail();
  }
  void
  bool_xor(Space* home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel) {
    if (home->failed()) return;
    NegBoolView n2(b2);
    if (Bool::Eqv<BoolView,BoolView,NegBoolView>::post(home,b0,b1,n2)
	== ES_FAILED)
      home->fail();
  }
  void
  bool_and(Space* home, const BoolVarArgs& b, BoolVar c, IntConLevel) {
    if (home->failed()) return;
    ViewArray<BoolView> x(home,b);
    GECODE_ES_FAIL(home,Bool::NaryAnd<BoolView>::post(home,x,c));
  }
  void
  bool_or(Space* home, const BoolVarArgs& b, BoolVar c, IntConLevel) {
    if (home->failed()) return;
    ViewArray<NegBoolView> x(home,b.size());
    for (int i=b.size(); i--; ) {
      NegBoolView nb(b[i]); x[i]=nb;
    }
    NegBoolView nc(c);
    GECODE_ES_FAIL(home,Bool::NaryAnd<NegBoolView>::post(home,x,nc));
  }

}


// STATISTICS: int-post

