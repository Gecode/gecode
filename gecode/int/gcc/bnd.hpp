/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004/2005
 *     Christian Schulte, 2009
 *     Guido Tack, 2009
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
 *
 */

namespace Gecode { namespace Int { namespace GCC {

  template<class Card, bool shared>
  forceinline
  BndImp<Card,shared>::
  BndImp(Space& home, ViewArray<IntView>& x0, ViewArray<Card>& k0,
         bool cf,  bool nolbc) :
    Propagator(home), x(x0), y(home, x0), k(k0),
    card_fixed(cf), skip_lbc(nolbc) {
    y.subscribe(home, *this, PC_INT_BND);
    k.subscribe(home, *this, PC_INT_BND);
  }

  template<class Card, bool shared>
  forceinline
  BndImp<Card,shared>::
  BndImp(Space& home, bool share, BndImp<Card,shared>& p)
    : Propagator(home, share, p),
      card_fixed(p.card_fixed), skip_lbc(p.skip_lbc) {
    x.update(home, share, p.x);
    y.update(home, share, p.y);
    k.update(home, share, p.k);
  }

  template<class Card, bool shared>
  size_t
  BndImp<Card,shared>::dispose(Space& home){
    y.cancel(home,*this, PC_INT_BND);
    k.cancel(home,*this, PC_INT_BND);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class Card, bool shared>
  size_t
  BndImp<Card,shared>::allocated(void) const {
    return lps.allocated() + ups.allocated();
  }

  template<class Card, bool shared>
  Actor*
  BndImp<Card,shared>::copy(Space& home, bool share){
    return new (home) BndImp<Card,shared>
      (home, share, *this);
  }

  template<class Card, bool shared>
  PropCost
  BndImp<Card,shared>::cost(const Space&,
                            const ModEventDelta& med) const {
    int ksize = Card::propagate ? k.size() : 0;
    if (IntView::me(med) == ME_INT_VAL)
      return PropCost::linear(PropCost::LO, y.size() + ksize);
    else
      return PropCost::quadratic(PropCost::LO, x.size() + ksize);
  }

  template<class Card, bool shared>
  ExecStatus
  BndImp<Card,shared>::pruneCards(Space& home) {
    // Remove all values with 0 max occurrence
    // and remove corresponding occurrence variables from k
    
    int noOfZeroes = 0;
    for (int i=k.size(); i--;)
      if (k[i].max() == 0)
        noOfZeroes++;

    if (noOfZeroes > 0) {
      IntArgs zeroIdx(noOfZeroes);
      noOfZeroes = 0;
      int j = 0;
      for (int i=0; i<k.size(); i++) {
        if (k[i].max() == 0) {
          zeroIdx[noOfZeroes++] = k[i].card();            
        } else {
          k[j++] = k[i];
        }
      }
      k.size(j);
      for (int i=x.size(); i--;) {
        IntSet zeroesI(&zeroIdx[0], noOfZeroes);
        IntSetRanges zeroesR(zeroesI);
        GECODE_ME_CHECK(x[i].minus_r(home, zeroesR));
      }
      lps.dispose();
      ups.dispose();
    }
    return ES_FIX;
  }

  template<class Card, bool shared>
  ExecStatus
  BndImp<Card,shared>::propagate(Space& home,
                                 const ModEventDelta& med) {
    if (IntView::me(med) == ME_INT_VAL) {
      GECODE_ES_CHECK((prop_val<Card,shared>(home,*this,y,k)));
      return ES_NOFIX_PARTIAL(*this,IntView::med(ME_INT_BND));
    }

    if (Card::propagate)
      GECODE_ES_CHECK(pruneCards(home));

    Region r(home);
    int* count = r.alloc<int>(k.size());

    for (int i = k.size(); i--; )
      count[i] = 0;
    bool all_assigned = true;
    int noa = 0;
    for (int i = x.size(); i--; ) {
      if (x[i].assigned()) {
        noa++;
        int idx = lookupValue(k,x[i].val());
        // reduction is essential for order on value nodes in dom
        // hence introduce test for failed lookup
        if (idx == -1)
          return ES_FAILED;
        count[idx]++;
      } else {
        all_assigned = false;
        // We only need the counts in the view case or when all
        // x are assigned
        if (!Card::propagate)
          break;
      }
    }

    if (Card::propagate) {
      // before propagation performs inferences on cardinality variables:
      if (noa > 0)
        for (int i = k.size(); i--; )
          if (!k[i].assigned()) {
            GECODE_ME_CHECK(k[i].lq(home, x.size() - (noa - count[i])));
            GECODE_ME_CHECK(k[i].gq(home, count[i]));
          }

      if (!card_consistent<Card>(x, k))
        return ES_FAILED;

      {
        bool mod;
        GECODE_ES_CHECK((prop_card<Card,shared>(home, x, k, mod)));
      }

      // Cardinalities may have been modified, so recompute
      // count and all_assigned
      for (int i = k.size(); i--; )
        count[i] = 0;
      all_assigned = true;
      for (int i = x.size(); i--; ) {
        if (x[i].assigned()) {
          int idx = lookupValue(k,x[i].val());
          // reduction is essential for order on value nodes in dom
          // hence introduce test for failed lookup
          if (idx == -1)
            return ES_FAILED;
          count[idx]++;
        } else {
          // We won't need the remaining counts, they're only used when
          // all x are assigned
          all_assigned = false;
          break;
        }
      }
    }

    if (all_assigned) {
      for (int i = k.size(); i--; )
        GECODE_ME_CHECK(k[i].eq(home, count[i]));
      return ES_SUBSUMED(*this,home);
    }

    if (Card::propagate)
      GECODE_ES_CHECK(pruneCards(home));

    int n = x.size();

    int* mu = r.alloc<int>(n);
    int* nu = r.alloc<int>(n);

    for (int i = n; i--; )
      nu[i] = mu[i] = i;

    // Create sorting permutation mu according to the variables upper bounds
    MaxInc<IntView> max_inc(x);
    Support::quicksort<int, MaxInc<IntView> >(mu, n, max_inc);

    // Create sorting permutation nu according to the variables lower bounds
    MinInc<IntView> min_inc(x);
    Support::quicksort<int, MinInc<IntView> >(nu, n, min_inc);

    // Sort the cardinality bounds by index
    MinIdx<Card> min_idx;
    Support::quicksort<Card, MinIdx<Card> >(&k[0], k.size(), min_idx);

    if (!lps.initialized()) {
      assert (!ups.initialized());
      lps.init(home, k, false);
      ups.init(home, k, true);
    } else if (Card::propagate) {
      // if there has been a change to the cardinality variables
      // reconstruction of the partial sum structure is necessary
      if (lps.check_update_min(k))
        lps.init(home, k, false);
      if (ups.check_update_max(k))
        ups.init(home, k, true);
    }

    // assert that the minimal value of the partial sum structure for
    // LBC is consistent with the smallest value a variable can take
    assert(lps.minValue() <= x[nu[0]].min());
    // assert that the maximal value of the partial sum structure for
    // UBC is consistent with the largest value a variable can take

    /*
     *  Setup rank and bounds info
     *  Since this implementation is based on the theory of Hall Intervals
     *  additional datastructures are needed in order to represent these
     *  intervals and the "partial-sum" data structure (cf."gcc/gccbndsup.hpp")
     *
     */

    HallInfo* hall = r.alloc<HallInfo>(2 * n + 2);
    Rank* rank = r.alloc<Rank>(n);

    int nb = 0;
    // setup bounds and rank
    int min        = x[nu[0]].min();
    int max        = x[mu[0]].max() + 1;
    int last       = lps.firstValue + 1; //equivalent to last = min -2
    hall[0].bounds = last;

    /*
     * First the algorithm merges the arrays minsorted and maxsorted
     * into bounds i.e. hall[].bounds contains the ordered union
     * of the lower and upper domain bounds including two sentinels
     * at the beginning and at the end of the set
     * ( the upper variable bounds in this union are increased by 1 )
     */

    {
      int i = 0;
      int j = 0;
      while (true) {
        if (i < n && min < max) {
          if (min != last) {
            last = min;
            hall[++nb].bounds = last;
          }
          rank[nu[i]].min = nb;
          if (++i < n)
            min = x[nu[i]].min();
        } else {
          if (max != last) {
            last = max;
            hall[++nb].bounds = last;
          }
          rank[mu[j]].max = nb;
          if (++j == n)
            break;
          max = x[mu[j]].max() + 1;
        }
      }
    }

    int rightmost = nb + 1; // rightmost accesible value in bounds
    hall[rightmost].bounds = ups.lastValue + 1 ;

    if (Card::propagate) {
      skip_lbc = true;
      for (int i = k.size(); i--; )
        if (k[i].min() != 0) {
          skip_lbc = false;
          break;
        }
    }

    if (!card_fixed && !skip_lbc)
      GECODE_ES_CHECK((lbc(home, nb, hall, rank, mu, nu)));

    GECODE_ES_CHECK((ubc(home, nb, hall, rank, mu, nu)));

    if (Card::propagate) {
      bool mod;
      GECODE_ES_CHECK((prop_card<Card, shared>(home, x, k, mod)));
    }

    for (int i = k.size(); i--; )
      count[i] = 0;
    for (int i = x.size(); i--; )
      if (x[i].assigned()) {
        count[lookupValue(k,x[i].val())]++;
      } else {
        // We won't need the remaining counts, they're only used when
        // all x are assigned
        return ES_NOFIX;
      }

    for (int i = k.size(); i--; )
      GECODE_ME_CHECK(k[i].eq(home, count[i]));

    return ES_SUBSUMED(*this,home);
  }


  template<class Card>
  ExecStatus
  Bnd<Card>::post(Space& home,
                  ViewArray<IntView>& x, ViewArray<Card>& k) {
    bool cardfix = true;
    for (int i=k.size(); i--; )
      if (!k[i].assigned()) {
        cardfix = false; break;
      }
    bool nolbc = true;
    for (int i=k.size(); i--; )
      if (k[i].min() != 0) {
        nolbc = false; break;
      }

    GECODE_ES_CHECK((postSideConstraints<Card>(home, x, k)));

    bool s = shared(home,x,k);
    if (!s && isDistinct<Card>(home,x,k))
      return Distinct::Bnd<IntView>::post(home,x);

    if (s) {
      (void) new (home) BndImp<Card,true>(home,x,k,cardfix,nolbc);
    } else {
      (void) new (home) BndImp<Card,false>(home,x,k,cardfix,nolbc);
    }
      return ES_OK;
  }

}}}

// STATISTICS: int-prop
