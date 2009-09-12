/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2005
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
 */

namespace Gecode { namespace Int { namespace GCC {

  template<class Card, bool shared>
  forceinline
  ValImp<Card,shared>::ValImp(Space& home, 
                              ViewArray<IntView>& x0, ViewArray<Card>& k0)
    : Propagator(home), x(x0), k(k0){
    x.subscribe(home, *this, PC_INT_VAL);
    k.subscribe(home, *this, PC_INT_VAL);
  }

  template<class Card, bool shared>
  forceinline
  ValImp<Card,shared>::ValImp(Space& home, bool share, ValImp<Card,shared>& p)
    : Propagator(home,share,p) {
    x.update(home,share, p.x);
    k.update(home,share, p.k);
  }

  template<class Card, bool shared>
  size_t
  ValImp<Card,shared>::dispose(Space& home) {
    x.cancel(home,*this, PC_INT_VAL);
    k.cancel(home,*this, PC_INT_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class Card, bool shared>
  Actor*
  ValImp<Card,shared>::copy(Space& home, bool share) {
    return new (home) ValImp<Card,shared>(home,share,*this);
  }

  template<class Card, bool shared>
  PropCost
  ValImp<Card,shared>::cost(const Space&, const ModEventDelta&) const {
    /*
     * Complexity depends on the time needed for value lookup in \a k
     * which is O(n log n).
     */
    return PropCost::linear(PropCost::HI,x.size());
  }

  template<class Card, bool shared>
  ExecStatus
  prop_val(Space& home, Propagator& p, 
           ViewArray<IntView>& x, ViewArray<Card>& k) {
    assert(x.size() > 0);

    int n = x.size();
    int m = k.size();

    Region r(home);
    // count[i] denotes how often value k[i].card() occurs in x
    int* count = r.alloc<int>(m);
    // stack of values having reached their maximum occurence
    int* rem = r.alloc<int>(m);
    // keep track whether a value is already on the stack
    bool* onrem = r.alloc<bool>(m);
    // stacksize
    int rs = 0;

    // initialization
    int sum_min = 0;
    int removed = 0;
    for (int i = m; i--; ) {
      removed += k[i].counter();
      sum_min += k[i].min();

      count[i] = 0;
      onrem[i] = false;
    }

    for (int i = m; i--; ) {
      // less than or equal than the total number of free variables
      // to satisfy the required occurences
      if (!k[i].assigned()) {
        int mub = n + removed - (sum_min - k[i].min());
        GECODE_ME_CHECK(k[i].lq(home, mub));
      }
    }

    // Due to lookup operation counting requires O(n \cdot log(n)) time
    bool all_assigned = true;
    // number of assigned views with respect to the current problem size
    int  noa   = 0;
    // total number of assigned views wrt. the original probem size
    int  t_noa = 0;
    for (int i = n; i--; )
      if (x[i].assigned()) {
        int idx = lookupValue(k,x[i].val());
        if (idx == -1)
          return ES_FAILED;
        assert(idx >= 0 && idx < m);
        count[idx]++;
        noa++;
      } else {
        all_assigned = false;
      }
    // number of unassigned views
    int  non = x.size() - noa;

    // check for subsumption
    if (all_assigned) {
      for (int i = m; i--; )
        GECODE_ME_CHECK((k[i].eq(home, count[i] + k[i].counter())));
      return ES_SUBSUMED(p,home);
    }

    // total number of unsatisfied miminum occurences
    int req = 0;
    // number of values whose min requirements are not yet met
    int n_r = 0;
    // if only one value is unsatisified single holds the index of that value
    int single = 0;

    for (int i = m; i--; ) {
      int ci = count[i] + k[i].counter();
      t_noa += ci;
      if (ci == 0) { // this works
        req += k[i].min();
        n_r++;
        single = i;
      }

      // number of unassigned views cannot satisfy
      // the required minimum occurence
      if (req > non)
        return ES_FAILED;
    }

    // if only one unsatisfied occurences is left
    if (req == non && n_r == 1) {
      for (int i = n; i--; ) {
        // try to assign it
        if (!x[i].assigned()) {
          GECODE_ME_CHECK(x[i].eq(home, k[single].card()));
          assert((single >= 0) && (single < m));
          count[single]++;
        }
      }
      assert(single >= 0 && single < m);
      // this might happen in case of sharing
      if (shared && count[single] < k[single].min())
        count[single] = k[single].min();

      for (int i = m; i--; )
        GECODE_ME_CHECK(k[i].eq(home, count[i] + k[i].counter()));
      return ES_SUBSUMED(p,home);
    }

    for (int i = m; i--; ) {
      int ci = count[i] + k[i].counter();
      if ((ci == k[i].max()) && !onrem[i]) {
        assert(rs >= 0 && rs < m);
        rem[rs] = k[i].card();
        k[i].counter(ci);
        rs++;
        onrem[i] = true;
        if (Card::propagate && !k[i].assigned()) {
          // the solution contains ci occurences of value k[i].card();
          GECODE_ME_CHECK(k[i].eq(home, ci));
        }
      } else {
        if (ci > k[i].max())
          return ES_FAILED;
        
        // in case of variable cardinalities
        if (Card::propagate && !k[i].assigned()) {
          if (ci > k[i].min())
            GECODE_ME_CHECK(k[i].gq(home, ci));
          int occupied = t_noa - ci;
          int mub = x.size() + removed - occupied;

          GECODE_ME_CHECK(k[i].lq(home, mub));
        }
      }
      // reset counter
      count[i] = 0;
    }

    // reduce the problem size
    for (int i = n; i--; ) {
      if (x[i].assigned()) {
        int idx = lookupValue(k,x[i].val());
        if (idx == -1) {
          return ES_FAILED;
        }
        assert(idx >= 0 && idx < m);
        if (onrem[idx]) {
          x[i] = x[--n];
        }
      }
    }
    x.size(n);

    // remove alredy satisfied values
    if (rs > 0) {
      IntSet remset(&rem[0], rs);
      for (int i = x.size(); i--;) {
        IntSetRanges rr(remset);
        if (!x[i].assigned())
          GECODE_ME_CHECK(x[i].minus_r(home, rr));
      }
    }

    all_assigned = true;

    for (int i = x.size(); i--; ) {
      if (x[i].assigned()) {
        int idx = lookupValue(k,x[i].val());
        if (idx == -1)
          return ES_FAILED;
        assert(idx >= 0 && idx < m);
        count[idx]++;
      } else {
        all_assigned = false;
      }
    }

    if (all_assigned) {
      for (int i = k.size(); i--; )
        GECODE_ME_CHECK((k[i].eq(home, count[i] + k[i].counter())));
      return ES_SUBSUMED(p,home);
    }

    if (Card::propagate) {
      // check again consistnecy of cardinalities
      int reqmin = 0;
      int allmax = 0;
      m    = k.size();
      n    = x.size();
      for (int i = m; i--; ) {
        int ci = k[i].counter();
        if (ci > k[i].max()) {
          return ES_FAILED;
        } else {
          allmax += (k[i].max() - ci);
          if (ci < k[i].min()) {
            reqmin += (k[i].min() - ci);
          }
        }
        if (k[i].min() > n)
          return ES_FAILED;
        if (!k[i].assigned())
          GECODE_ME_CHECK((k[i].lq(home, n)));
      }

      if ((n < reqmin) || (allmax < n))
        return ES_FAILED;
    }

    return ES_NOFIX;
  }

  template<class Card, bool shared>
  ExecStatus
  ValImp<Card,shared>::propagate(Space& home, const ModEventDelta&) {
    return prop_val<Card,shared>(home, *this, x, k);
  }

  template<class Card>
  ExecStatus
  Val<Card>::post(Space& home,
                  ViewArray<IntView>& x, ViewArray<Card>& k) {
    GECODE_ES_CHECK((postSideConstraints<Card>(home,x,k)));

    bool shared = x.shared(home);

    if (isDistinct<Card>(home,x,k) && !shared)
      return Distinct::Val<IntView>::post(home,x);
   
    if (shared)
      (void) new (home) ValImp<Card,true>(home,x,k);
    else
      (void) new (home) ValImp<Card,false>(home,x,k);
    return ES_OK;
  }


}}}

// STATISTICS: int-prop

