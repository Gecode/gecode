/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date: 2006-11-25 19:48:19 +0100 (Sat, 25 Nov 2006) $ by $Author: schulte $
 *     $Revision: 3996 $
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

#include "gecode/int/channel.hh"

namespace Gecode { namespace Int { namespace Channel {

  /*
   * Iterator over Boolean array
   *
   */
  class BoolIter {
  private:
    const ViewArray<BoolView>& x;
    const int o;
    int i;
  public:
    BoolIter(const ViewArray<BoolView>& x0, int o0) :
      x(x0), o(o0), i(0) {
      while (x[i].zero()) i++;
    }
    bool operator()(void) const {
      return i<x.size();
    }
    int val(void) const {
      return i+o;
    }
    void operator++(void) {
      do {
        i++;
      } while ((i<x.size()) && x[i].zero());
    }
  };


  forceinline
  LinkMulti::LinkMulti(Space* home, ViewArray<BoolView>& x, IntView y, int o0)
    : MixNaryOnePropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_DOM>
  (home,x,y), o(o0) {}

  ExecStatus
  LinkMulti::post(Space* home, ViewArray<BoolView>& x, IntView y, int o) {
    int n=x.size();
    GECODE_ME_CHECK(y.gq(home,o));
    GECODE_ME_CHECK(y.le(home,o+n));
    (void) new (home) LinkMulti(home,x,y,o);
    return ES_OK;
  }


  forceinline
  LinkMulti::LinkMulti(Space* home, bool share, LinkMulti& p)
    : MixNaryOnePropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_DOM>
  (home,share,p), o(p.o) {}

  Actor*
  LinkMulti::copy(Space* home, bool share) {
    return new (home) LinkMulti(home,share,*this);
  }

  PropCost
  LinkMulti::cost(void) const {
    return PC_UNARY_LO;
  }

  ExecStatus
  LinkMulti::propagate(Space* home) {
    assert((y.min()-o >= 0) && (y.max()-o < n));
    if (IntView::pme(this) == ME_INT_VAL) {
      assert(y.assigned());
      int j=y.val()-o;
      for (int i=0; i<j; i++)
        GECODE_ME_CHECK(x[i].zero(home));
      GECODE_ME_CHECK(x[j].one(home));
      for (int i=j+1; i<x.size(); i++)
        GECODE_ME_CHECK(x[i].zero(home));
      return ES_SUBSUMED(this,sizeof(*this));
    }
    if (IntView::pme(this) == ME_INT_BND) {
      // Assign all Boolean views zero that are outside bounds
      int min=y.min()-o;
      for (int i=0; i<min; i++)
        GECODE_ME_CHECK(x[i].zero(home));
      int max=y.max()-o;
      for (int i=max+1; i<x.size(); i++)
        GECODE_ME_CHECK(x[i].zero(home));
      x.drop_fst(min); 
      x.drop_lst(max-min); 
      o += min;
    }
    if (BoolView::pme(this) == ME_BOOL_VAL) {
      {
        // Remove zeros on the left
        int i=0;
        while ((i < x.size()) && x[i].zero()) i++;
        if (i >= x.size())
          return ES_FAILED;
        x.drop_fst(i); o += i;
      }
      {
        // Remove zeros on the right
        int i=x.size()-1;
        while ((i >= 0) && x[i].zero()) i--;
        assert(i >= 0);
        x.drop_lst(i);
      }
      assert(x.size() >= 1);
      // Is there only one left?
      if (x.size() == 1) {
        GECODE_ME_CHECK(x[0].one(home));
        GECODE_ME_CHECK(y.eq(home,o));
        return ES_SUBSUMED(this,sizeof(*this));
      }
      // Check whether there is a one somewhere else
      for (int i=x.size(); i--; )
        if (x[i].one()) {
          for (int j=0; j<i; j++)
            GECODE_ME_CHECK(x[j].zero(home));
          for (int j=i+1; j<x.size(); j++)
            GECODE_ME_CHECK(x[j].zero(home));
          GECODE_ME_CHECK(y.eq(home,i+o));
          return ES_SUBSUMED(this,sizeof(*this));
        }
      // Update bounds
      GECODE_ME_CHECK(y.gq(home,o));
      GECODE_ME_CHECK(y.le(home,o+x.size()));
    }
    // Propagate from y to Boolean views
    if ((IntView::pme(this) == ME_INT_BND) ||
        (IntView::pme(this) == ME_INT_DOM)) {
      ViewValues<IntView> v(y);
      int i=0; 
      do {
        if (i < v.val()-o) {
          ModEvent me = x[i].zero(home);
          assert(!me_failed(me));
          ++i;
        } else if (i > v.val()-o) {
          ++v;
        } else {
          assert(i == v.val()-o);
          ++i; ++v;
        }
      } while (v() && (i < x.size()));
    }
    // Propagate from Boolean views to y
    if (BoolView::pme(this) == ME_BOOL_VAL) {
      BoolIter bv(x,o);
      Iter::Values::ToRanges<BoolIter> br(bv);
      ModEvent me = y.narrow(home,br);
      if (me_failed(me))
        return ES_FAILED;
      if (me == ME_INT_VAL) {
        GECODE_ME_CHECK(x[y.val()-o].one(home));
        return ES_SUBSUMED(this,sizeof(*this));
      }
    }
    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop

