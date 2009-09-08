/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004/2005
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

  // for posting
  template<class Card, bool isView, bool shared>
  inline
  BndImp<Card, isView, shared>::
  BndImp(Space& home, ViewArray<IntView>& x0, ViewArray<Card>& k0,
         bool cf,  bool nolbc) :
    Propagator(home), x(x0), k(k0),
    card_fixed(cf), skip_lbc(nolbc) {
    home.notice(*this,AP_DISPOSE);
    x.subscribe(home, *this, PC_INT_BND);
    k.subscribe(home, *this, PC_INT_BND);
  }

  // for cloning
  template<class Card, bool isView, bool shared>
  forceinline
  BndImp<Card, isView, shared>::
  BndImp(Space& home, bool share, BndImp<Card, isView, shared>& p)
    : Propagator(home, share, p),
      card_fixed(p.card_fixed), skip_lbc(p.skip_lbc) {
    x.update(home, share, p.x);
    k.update(home, share, p.k);
  }

  template<class Card, bool isView, bool shared>
  size_t
  BndImp<Card, isView, shared>::dispose(Space& home){
    home.ignore(*this,AP_DISPOSE);
    if (!home.failed()) {
      x.cancel(home,*this, PC_INT_BND);
      k.cancel(home,*this, PC_INT_BND);
    }
    lps.dispose();
    ups.dispose();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class Card, bool isView, bool shared>
  size_t
  BndImp<Card, isView, shared>::allocated(void) const {
    return lps.allocated() + ups.allocated();
  }

  template<class Card, bool isView, bool shared>
  Actor*
  BndImp<Card, isView, shared>::copy(Space& home, bool share){
    return new (home) BndImp<Card, isView, shared>
      (home, share, *this);
  }

  template<class Card, bool isView, bool shared>
  PropCost
  BndImp<Card, isView, shared>::cost(const Space&, const ModEventDelta&) const {
    /*
     * The bounds consistent version of the Global Cardinality constraint
     * has a theoretical complexity of
     *   O(t + n\cdot log(n)) with
     *   n = number of variables
     *   t = time needed to sort the domain bounds of the variables
     */
    return PropCost::linear(PropCost::HI,x.size());
  }

  template<class Card, bool isView, bool shared>
  ExecStatus
  BndImp<Card, isView, shared>::pruneCards(Space& home) {
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

  template<class Card, bool isView, bool shared>
  ExecStatus
  BndImp<Card, isView, shared>::propagate(Space& home, const ModEventDelta&) {
    if (isView)
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
        // We only need the counts in the isView case or when all
        // x are assigned
        if (!isView)
          break;
      }
    }

    if (isView) {
      // before propagation performs inferences on cardinality variables:
      if (noa > 0) {
        int n  = x.size();
        int ks = k.size();

        for (int i = ks; i--; ) {
          if (!k[i].assigned()) {
            int ub = n - (noa - count[i]);
            int lb = count[i];
            GECODE_ME_CHECK(k[i].lq(home, ub));
            GECODE_ME_CHECK(k[i].gq(home, lb));
          }
        }
      }

      if (!card_consistent<Card>(x, k))
        return ES_FAILED;

      {
        bool mod;
        GECODE_ES_CHECK((prop_card<Card, shared>(home, x, k, mod)));
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
      for (int i = k.size(); i--; ) {
        GECODE_ME_CHECK(k[i].eq(home, count[i]));
        all_assigned &= (!isView) || k[i].assigned();
      }
      if ( (!isView) || all_assigned)
        return ES_SUBSUMED(*this,home);
    }

    if (isView)
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
      lps.init(k, false);
      ups.init(k, true);
    } else if (isView) {
      // if there has been a change to the cardinality variables
      // reconstruction of the partial sum structure is necessary
      if (lps.check_update_min(k)) {
        lps.dispose();
        lps.init(k, false);
      }

      if (ups.check_update_max(k)) {
        ups.dispose();
        ups.init(k, true);
      }
    }

    // assert that the minimal value of the partial sum structure for
    // LBC is consistent with the smallest value a variable can take
    assert(lps.minValue() <= x[nu[0]].min());
    // assert that the maximal value of the partial sum structure for
    // UBC is consistent with the largest value a variable can take
    //std::cerr << x[mu[x.size() - 1]].max() <<"<="<< ups->maxValue() << "\n";
    //assert(x[mu[x.size() - 1]].max() <= ups->maxValue());

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
          if (++i < n) {
            min = x[nu[i]].min();
          }
        } else {
          if (max != last) {
            last = max;
            hall[++nb].bounds = last;
          }
          rank[mu[j]].max = nb;
          if (++j == n) {
            break;
          }
          max = x[mu[j]].max() + 1;
        }
      }
    }

    int rightmost = nb + 1; // rightmost accesible value in bounds
    hall[rightmost].bounds = ups.lastValue + 1 ;

    if (isView) {
      skip_lbc = true;
      for (int i = k.size(); i--; )
        if (k[i].min() != 0) {
          skip_lbc = false;
          break;
        }
    }

    if (!card_fixed && !skip_lbc) {
      GECODE_ES_CHECK((lbc(home, nb, hall, rank, mu, nu)));
    }

    GECODE_ES_CHECK((ubc(home, nb, hall, rank, mu, nu)));

    if (isView) {
      bool mod;
      GECODE_ES_CHECK((prop_card<Card, shared>(home, x, k, mod)));
    }

    for (int i = k.size(); i--; ) {
      count[i] = 0;
    }
    for (int i = x.size(); i--; ) {
      if (x[i].assigned()) {
        int idx = lookupValue(k,x[i].val());
        count[idx]++;
      } else {
        // We won't need the remaining counts, they're only used when
        // all x are assigned
        return ES_NOFIX;
      }
    }

    all_assigned = true;
    for (int i = k.size(); i--; ) {
      GECODE_ME_CHECK(k[i].eq(home, count[i]));
      all_assigned &= (!isView) || k[i].assigned();
    }
    return all_assigned ? ES_SUBSUMED(*this,home) : ES_NOFIX;
  }

  /**
   * \brief Sharing test for the bounds consistent global cardinality
   * propagator.
   *
   * Due to parametrization on the problem views, \a View1 and the
   * cardinality views, \a View2 the test can be applied for both cases
   * of fixed cardinalities and cardinality variables.
   */
  template<class View1, class View2>
  class SharingTest {
  public:
    /**
     * \brief Test whether the problem views in \a x0 or the
     *  cardinality views in \a k0 contain shared views.
     */
    static bool shared(Space& home, ViewArray<View1>& x0,
                       ViewArray<View2>& k0) {
      ViewArray<View1> xc(home, x0.size()+k0.size());
      for (int i = 0; i < x0.size(); i++) {
        xc[i] = x0[i];
      }
      for (int i = x0.size(); i < x0.size()+k0.size(); i++) {
        xc[i] = k0[i - x0.size()].intview();
      }
      return xc.shared(home);
    }
  };

  /**
   * \brief Specialization of class SharingTest for the case of fixed
   * cardinalities using IntView as \a View1 and OccurBndsView as \a View2
   */
  template<>
  class SharingTest<IntView,OccurBndsView> {
  public:
    /// Test whether the problem views in \a x0 contain shared views.
    static bool shared(Space& home,
                       ViewArray<IntView>& xs, ViewArray<OccurBndsView>&) {
      return xs.shared(home);
    }
  };

  template<class Card, bool isView>
  ExecStatus
  Bnd<Card, isView>::post(Space& home,
                          ViewArray<IntView>& x0,
                          ViewArray<Card>& k0) {
    bool cardfix = true;
    bool nolbc = true;
    for (int i = k0.size(); i--; ) {
      cardfix &= k0[i].assigned();
      nolbc &= (k0[i].min() == 0);
    }
    if (SharingTest<IntView,Card>::shared(home,x0,k0)) {
      new (home) BndImp<Card, isView, true>
        (home, x0, k0, cardfix, nolbc);
    } else {
      new (home) BndImp<Card, isView, false>
        (home, x0, k0, cardfix, nolbc);
    }
    return ES_OK;
  }

}}}

// STATISTICS: int-prop

