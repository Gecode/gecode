/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004, 2005
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

#include "set.hh"
#include "iter.hh"
#include "set/rel.hh"

namespace Gecode {

  void
  dom(Space* home, SetVar s, SetRelType r, int i) {
    IntSet d(i,i);
    dom(home, s, r, d);
  }

  void
  dom(Space* home, SetVar s, SetRelType r, int i, int j) {
    IntSet d(i,j);
    dom(home, s, r, d);
  }

  void
  dom(Space* home, SetVar s, SetRelType r, const IntSet& is) {
    if (home->failed()) return;

    Set::SetView _s(s);

    switch(r) {
    case SRT_EQ:
      {
	if (is.size() == 1) {
	  GECODE_ME_FAIL(home,_s.include(home, is.min(), is.max()));
	  GECODE_ME_FAIL(home,_s.intersect(home, is.min(), is.max()));
	} else {
	  IntSetRanges rd1(is);
	  GECODE_ME_FAIL(home,_s.includeI(home, rd1));
	  IntSetRanges rd2(is);
	  GECODE_ME_FAIL(home,_s.intersectI(home, rd2));
	}
      }
      break;
    case SRT_DISJ:
      {
	if (is.size() == 1) {
	  GECODE_ME_FAIL(home,_s.exclude(home, is.min(), is.max()));
	} else {
	  IntSetRanges rd(is);
	  GECODE_ME_FAIL(home,_s.excludeI(home, rd));
	}
      }
      break;
    case SRT_NQ:
	if (is.size() == 1) {
	  GECODE_ME_FAIL(home,_s.exclude(home, is.min(), is.max()));
	} else {
	  Set::ConstantView cv(home, is);
	  GECODE_ES_FAIL(home,
			 (Set::Rel::Distinct<Set::SetView,
			  Set::ConstantView>::post(home, s, cv)));
	}
      break;
    case SRT_SUB:
      {
 	if (is.size() == 1) {
 	  GECODE_ME_FAIL(home,_s.intersect(home, is.min(), is.max()));
 	} else {
	  IntSetRanges rd(is);
	  GECODE_ME_FAIL(home,_s.intersectI(home, rd));
 	}
      }
      break;
    case SRT_SUP:
      {
	if (is.size() == 1) {
	  GECODE_ME_FAIL(home,_s.include(home, is.min(), is.max()));
	} else {
	  IntSetRanges rd(is);
	  GECODE_ME_FAIL(home,_s.includeI(home, rd));
	}
      }
      break;
    case SRT_CMPL:
      {
	if (is.size() == 1) {
	  GECODE_ME_FAIL(home,_s.exclude(home, is.min(), is.max()));
	  GECODE_ME_FAIL(home,
			 _s.include(home,
				    Limits::Set::int_min,
				    is.min()-1) );
	  GECODE_ME_FAIL(home,
			 _s.include(home, is.max()+1,
				    Limits::Set::int_max) );
	} else {
	  IntSetRanges rd1(is);
	  Set::RangesCompl<IntSetRanges > rdC1(rd1);
	  GECODE_ME_FAIL(home,_s.includeI(home, rdC1));
	  IntSetRanges rd2(is);
	  Set::RangesCompl<IntSetRanges > rdC2(rd2);
	  GECODE_ME_FAIL(home,_s.intersectI(home, rdC2));
	}
      }
      break;
    }
  }

  void
  dom(Space* home, SetVar s, SetRelType r, int i, BoolVar b) {
    IntSet d(i,i);
    dom(home, s, r, d, b);
  }

  void
  dom(Space* home, SetVar s, SetRelType r, int i, int j, BoolVar b) {
    IntSet d(i,j);
    dom(home, s, r, d, b);
  }

  void
  dom(Space* home, SetVar s, SetRelType r, const IntSet& is, BoolVar b) {
    if (home->failed()) return;
    switch(r) {
    case SRT_EQ:
      {
	Set::ConstantView cv(home, is);
	GECODE_ES_FAIL(home,
		       (Set::Rel::ReEq<Set::SetView,
			Set::ConstantView>::post(home, s, cv, b)));
      }
      break;
    case SRT_NQ:
      {
	BoolVar notb(home,0,1);
	bool_not(home, b, notb);
	Set::ConstantView cv(home, is);
	GECODE_ES_FAIL(home,
		       (Set::Rel::ReEq<Set::SetView,
			Set::ConstantView>::post(home, s, cv, notb)));
      }
      break;
    case SRT_SUB:
      {
	Set::ConstantView cv(home, is);
	GECODE_ES_FAIL(home,
		       (Set::Rel::ReSubset<Set::SetView,Set::ConstantView>
			::post(home, s, cv, b)));
      }
      break;
    case SRT_SUP:
      {
	Set::ConstantView cv(home, is);
	GECODE_ES_FAIL(home,
		       (Set::Rel::ReSubset<Set::ConstantView,Set::SetView>
			::post(home, cv, s, b)));
      }
      break;
    case SRT_DISJ:
      {
	// x||y <=> b is equivalent to
	// ( y <= complement(x) and x<=complement(y) ) <=> b

	// set up BoolVars for the conjunction
	BoolVar b1(home, 0, 1);
	BoolVar b2(home, 0, 1);
	bool_and(home, b1, b2, b);	

	// compute complement of is
	IntSetRanges dr1(is);
	Set::RangesCompl<IntSetRanges > dc1(dr1);
	
	// First conjunct: ( s <= complement(is) ) <=> b1
	IntSet dcompl(dc1);
	Set::ConstantView cvcompl(home, dcompl);
	GECODE_ES_FAIL(home,
		       (Set::Rel::ReSubset<Set::SetView,Set::ConstantView>
			::post(home, s, cvcompl, b1)));

	// Second conjunct: ( complement(s) <= d ) <=> b2
	Set::ConstantView cv(home, is);
	Set::SetView sv(s);
	Set::ComplementView<Set::SetView> cs(sv);
	GECODE_ES_FAIL(home,
		       (Set::Rel::ReSubset<Set::ConstantView,Set::ComplementView<Set::SetView> >
			::post(home, cv, cs, b2)));

      }
      break;
    case SRT_CMPL:
      {
	Set::SetView sv(s);
	Set::ComplementView<Set::SetView> cs(sv);
	Set::ConstantView cv(home, is);
	GECODE_ES_FAIL(home,
		       (Set::Rel::ReEq<Set::ComplementView<Set::SetView>,
			Set::ConstantView>
			::post(home, cs, cv, b)));
      }
      break;
    }
  }

}

// STATISTICS: set-post
