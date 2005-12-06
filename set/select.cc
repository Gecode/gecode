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

#include "set/select.hh"

using namespace Gecode::Set;

namespace Gecode {

  void
  selectUnion(Space* home, const SetVarArgs& sn, SetVar s2, SetVar s1) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<ComplementView<SetView> > iv(home, sn);
    IntSet universe(Limits::Set::int_min,
                     Limits::Set::int_max);
    SetView s1v(s1);
    ComplementView<SetView> cs1(s1v);
    GECODE_ES_FAIL(home,(Select::SelectIntersection<ComplementView<SetView>,
			 SetView>::post(home,cs1,iv,s2,universe)));
  }

  void
  selectInter(Space* home, const SetVarArgs& sn, SetVar s2, SetVar s1) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, sn);
    IntSet universe(Limits::Set::int_min,
                     Limits::Set::int_max);
    GECODE_ES_FAIL(home,
                   (Select::SelectIntersection<SetView,SetView>::
		    post(home,s1,iv,s2,universe)));
  }

  void
  selectInterIn(Space* home, const SetVarArgs& sn, SetVar s2, SetVar s1,
                const IntSet& universe) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, sn);
    GECODE_ES_FAIL(home,
                   (Select::SelectIntersection<SetView,SetView>::
		    post(home,s1,iv,s2,universe)));
  }

  void
  selectSets(Space* home, const SetVarArgs& sn, IntVar i, SetVar s) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<ComplementView<SetView> > iv(home, sn);
    Int::IntView ii(i);
    SingletonView single(ii);
    SetView sv(s);
    ComplementView<SetView> cs(sv);
    IntSet universe(Limits::Set::int_min,
                     Limits::Set::int_max);
    GECODE_ES_FAIL(home,(Select::SelectIntersection<ComplementView<SetView>,
			 SingletonView>::post(home, cs, iv, single, universe)));
  }

  void
  selectDisjoint(Space* home, const SetVarArgs& sn, SetVar s) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, sn);
    GECODE_ES_FAIL(home,Select::SelectDisjoint::post(home,iv,s));
  }
  
}

// STATISTICS: set-post

