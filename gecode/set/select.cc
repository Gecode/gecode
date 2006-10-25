/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/set/select.hh"

using namespace Gecode::Set;

namespace Gecode {

  void
  selectUnion(Space* home, const SetVarArgs& x, SetVar y, SetVar z) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<ComplementView<SetView> > iv(home, x);
    IntSet universe(Limits::Set::int_min,
                    Limits::Set::int_max);
    SetView zv(z);
    ComplementView<SetView> cz(zv);
    GECODE_ES_FAIL(home,(Select::SelectIntersection<ComplementView<SetView>,
                         SetView>::post(home,cz,iv,y,universe)));
  }

  void
  selectInter(Space* home, const SetVarArgs& x, SetVar y, SetVar z) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    IntSet universe(Limits::Set::int_min,
                    Limits::Set::int_max);
    GECODE_ES_FAIL(home,
                   (Select::SelectIntersection<SetView,SetView>::
                    post(home,z,iv,y,universe)));
  }

  void
  selectInterIn(Space* home, const SetVarArgs& x, SetVar y, SetVar z,
                const IntSet& universe) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    GECODE_ES_FAIL(home,
                   (Select::SelectIntersection<SetView,SetView>::
                    post(home,z,iv,y,universe)));
  }

  void
  selectSet(Space* home, const SetVarArgs& x, IntVar y, SetVar z) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<ComplementView<SetView> > iv(home, x);
    Int::IntView yv(y);
    SingletonView single(yv);
    SetView zv(z);
    ComplementView<SetView> cz(zv);
    IntSet universe(Limits::Set::int_min,
                    Limits::Set::int_max);
    GECODE_ES_FAIL(home,(Select::SelectIntersection<ComplementView<SetView>,
                         SingletonView>::post(home, cz, iv,
                                              single, universe)));
  }

  void
  selectDisjoint(Space* home, const SetVarArgs& x, SetVar y) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    GECODE_ES_FAIL(home,Select::SelectDisjoint::post(home,iv,y));
  }
  
}

// STATISTICS: set-post

