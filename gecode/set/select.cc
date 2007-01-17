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
  selectUnion(Space* home, const SetVarArgs& x, SetVar y, SetVar z, int offset) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    if (offset == 0) {
      GECODE_ES_FAIL(home,
                     (Select::SelectUnion<SetView,SetView>::
                      post(home,z,iv,y)));
    } else {
      SetView vy(y);
      OffsetSetView<SetView> oy(vy, offset);
      GECODE_ES_FAIL(home,
                     (Select::SelectUnion<SetView,OffsetSetView<SetView> >::
                      post(home,z,iv,oy)));
    }
  }

  void
  selectUnion(Space* home, const IntSetArgs& s, SetVar y, SetVar z, int offset) {
    Support::SharedArray<IntSet,true> x(s.size());
    for (int i=s.size(); i--;)
      new (&x[i]) IntSet(s[i]);
    if (home->failed()) return;
    if (offset == 0) {
      GECODE_ES_FAIL(home,
                     (Select::SelectUnionConst<SetView,SetView>::
                      post(home,z,x,y)));
    } else {
      SetView vy(y);
      OffsetSetView<SetView> oy(vy, offset);
      GECODE_ES_FAIL(home,
                     (Select::SelectUnionConst<SetView,OffsetSetView<SetView> >::
                      post(home,z,x,oy)));
    }
  }

  void
  selectInter(Space* home, const SetVarArgs& x, SetVar y, SetVar z, int offset) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    IntSet universe(Limits::Set::int_min,
                    Limits::Set::int_max);
    if (offset == 0) {
      GECODE_ES_FAIL(home,
                     (Select::SelectIntersection<SetView,SetView>::
                      post(home,z,iv,y,universe)));
    } else {
      SetView vy(y);
      OffsetSetView<SetView> oy(vy, offset);
      GECODE_ES_FAIL(home,
                     (Select::SelectIntersection<SetView,OffsetSetView<SetView> >::
                      post(home,z,iv,oy,universe)));
    }
  }

  void
  selectInterIn(Space* home, const SetVarArgs& x, SetVar y, SetVar z,
                const IntSet& universe, int offset) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    if (offset == 0) {
      GECODE_ES_FAIL(home,
                     (Select::SelectIntersection<SetView,SetView>::
                      post(home,z,iv,y,universe)));
    } else {
      SetView vy(y);
      OffsetSetView<SetView> oy(vy, offset);
      GECODE_ES_FAIL(home,
                     (Select::SelectIntersection<SetView,OffsetSetView<SetView> >::
                      post(home,z,iv,oy,universe)));
    }
  }

  void
  selectSet(Space* home, const SetVarArgs& x, IntVar y, SetVar z, int offset) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView > iv(home, x);
    SetView zv(z);

    if (offset == 0) {
      Int::IntView yv(y);
      SingletonView single(yv);
      GECODE_ES_FAIL(home,(Select::SelectUnion<SetView,
                           SingletonView>::post(home, z, iv, single)));
    } else {
      Int::IntView yv(y);
      SingletonView single(yv);
      OffsetSetView<SingletonView> osingle(single, offset);
      GECODE_ES_FAIL(home,(Select::SelectUnion<SetView,
                           OffsetSetView<SingletonView> >::post(home, z, iv,
                                                                osingle)));
      
    }
  }

  void
  selectSet(Space* home, const IntSetArgs& s, IntVar y, SetVar z, int offset) {
    if (home->failed()) return;
    SetView zv(z);
    Support::SharedArray<IntSet,true> x(s.size());
    for (int i=s.size(); i--;)
      new (&x[i]) IntSet(s[i]);

    if (offset == 0) {
      Int::IntView yv(y);
      SingletonView single(yv);
      GECODE_ES_FAIL(home,(Select::SelectUnionConst<SetView,
                           SingletonView>::post(home, z, x, single)));
    } else {
      Int::IntView yv(y);
      SingletonView single(yv);
      OffsetSetView<SingletonView> osingle(single, offset);
      GECODE_ES_FAIL(home,(Select::SelectUnionConst<SetView,
                           OffsetSetView<SingletonView> >::post(home, z, x,
                                                                osingle)));
      
    }
  }

  void
  selectDisjoint(Space* home, const SetVarArgs& x, SetVar y) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    GECODE_ES_FAIL(home,Select::SelectDisjoint::post(home,iv,y));
  }
  
}

// STATISTICS: set-post

