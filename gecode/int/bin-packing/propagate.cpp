/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Fabio Tardivo <ftardivo@nmsu.edu>
 *
 *  Copyright:
 *     Fabio Tardivo, 2024
 *     Christian Schulte, 2010
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/int/bin-packing.hh>

namespace Gecode { namespace Int { namespace BinPacking {

  /*
   * Packing propagator
   *
   */

  PropCost
  Pack::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::HI,bs.size());
  }

  void
  Pack::reschedule(Space& home) {
    l.reschedule(home,*this,PC_INT_BND);
    bs.reschedule(home,*this,PC_INT_DOM);
  }

  Actor*
  Pack::copy(Space& home) {
    return new (home) Pack(home,*this);
  }

  /// Record tell information
  class TellCache {
  protected:
    /// Values (sorted) to be pruned from view
    int* _nq;
    /// Number of values to be pruned
    int _n_nq;
    /// Value to which view should be assigned
    int _eq;
  public:
    /// Initialize cache for at most \a m values
    TellCache(Region& region, int m);
    /// Record that view must be different from \a j
    void nq(int j);
    /// Record that view must be equal to \a j, return false if not possible
    void eq(int j);
    /// Perform tell to view \a x and reset cache
    ExecStatus tell(Space& home, IntView x);
  };

  forceinline
  TellCache::TellCache(Region& region, int m)
    : _nq(region.alloc<int>(m)), _n_nq(0), _eq(-1) {}
  forceinline void
  TellCache::nq(int j) {
    _nq[_n_nq++] = j;
  }
  forceinline void
  TellCache::eq(int j) {
    // For eq: -1 mean not yet assigned, -2 means failure, positive means value
    if (_eq == -1)
      _eq = j;
    else
      _eq = -2;
  }
  ExecStatus
  TellCache::tell(Space& home, IntView x) {
    if (_eq == -2) {
      return ES_FAILED;
    } else if (_eq >= 0) {
      GECODE_ME_CHECK(x.eq(home,_eq));
    }
    Iter::Values::Array nqi(_nq, _n_nq);
    GECODE_ME_CHECK(x.minus_v(home, nqi));
    _n_nq=0; _eq=-1;
    return ES_OK;
  }


  /*
   * Propagation proper
   *
   */
  ExecStatus
  Pack::propagate(Space& home, const ModEventDelta& med) {
    // Number of items
    int n = bs.size();
    // Number of bins
    int m = l.size();

    Region region;
    {

      // Possible sizes for bins
      int* s = region.alloc<int>(m);

      for (int j=0; j<m; j++)
        s[j] = 0;

      // Compute sizes for bins
      if (OffsetView::me(med) == ME_INT_VAL) {
        // Also eliminate assigned items
        int k=0;
        for (int i=0; i<n; i++)
          if (bs[i].assigned()) {
            int j = bs[i].bin().val();
            l[j].offset(l[j].offset() - bs[i].size());
            t -= bs[i].size();
          } else {
            for (ViewValues<IntView> j(bs[i].bin()); j(); ++j)
              s[j.val()] += bs[i].size();
            bs[k++] = bs[i];
          }
        n=k; bs.size(n);
      } else {
        for (int i=0; i<n; i++) {
          assert(!bs[i].assigned());
          for (ViewValues<IntView> j(bs[i].bin()); j(); ++j)
            s[j.val()] += bs[i].size();
        }
      }

      // Propagate bin loads and compute lower and upper bound
      int min = t, max = t;
      for (int j=0; j<m; j++) {
        GECODE_ME_CHECK(l[j].gq(home,0));
        GECODE_ME_CHECK(l[j].lq(home,s[j]));
        min -= l[j].max(); max -= l[j].min();
      }

      // Propagate that load must be equal to total size
      for (bool mod = true; mod; ) {
        mod = false; ModEvent me;
        for (int j=0; j<m; j++) {
          int lj_min = l[j].min();
          me = l[j].gq(home, min + l[j].max());
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            max += lj_min - l[j].min(); mod = true;
          }
          int lj_max = l[j].max();
          me = l[j].lq(home, max + l[j].min());
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            min += lj_max - l[j].max(); mod = true;
          }
        }
      }

      if (n == 0) {
        assert(l.assigned());
        return home.ES_SUBSUMED(*this);
      }


      {
        TellCache tc(region,m);

        int k=0;
        for (int i=0; i<n; i++) {
          for (ViewValues<IntView> j(bs[i].bin()); j(); ++j) {
            if (bs[i].size() > l[j.val()].max())
              tc.nq(j.val());
            if (s[j.val()] - bs[i].size() < l[j.val()].min())
              tc.eq(j.val());
          }
          GECODE_ES_CHECK(tc.tell(home,bs[i].bin()));
          // Eliminate assigned bin
          if (bs[i].assigned()) {
            int j = bs[i].bin().val();
            l[j].offset(l[j].offset() - bs[i].size());
            t -= bs[i].size();
          } else {
            bs[k++] = bs[i];
          }
        }
        n=k; bs.size(n);
      }
      region.free();
    }

    // Only if the propagator is at fixpoint here, continue with the more
    // expensive stage for propagation.
    if (IntView::me(modeventdelta()) != ME_INT_NONE)
      return ES_NOFIX;

    // Now the invariant holds that no more assigned bins exist!
    {

      // Size of items
      SizeSetMinusOne* s = region.alloc<SizeSetMinusOne>(m);

      for (int j=0; j<m; j++)
        s[j] = SizeSetMinusOne(region,n);

      // Set up size information
      for (int i=0; i<n; i++) {
        assert(!bs[i].assigned());
        for (ViewValues<IntView> j(bs[i].bin()); j(); ++j)
          s[j.val()].add(bs[i].size());
      }

      for (int j=0; j<m; j++) {
        // Can items still be packed into bin?
        if (nosum(static_cast<SizeSet&>(s[j]), l[j].min(), l[j].max()))
          return ES_FAILED;
        int ap, bp;
        // Must there be packed more items into bin?
        if (nosum(static_cast<SizeSet&>(s[j]), l[j].min(), l[j].min(),
                  ap, bp))
          GECODE_ME_CHECK(l[j].gq(home,bp));
        // Must there be packed less items into bin?
        if (nosum(static_cast<SizeSet&>(s[j]), l[j].max(), l[j].max(),
                  ap, bp))
          GECODE_ME_CHECK(l[j].lq(home,ap));
      }

      TellCache tc(region,m);

      int k=0;
      for (int i=0; i<n; i++) {
        assert(!bs[i].assigned());
        for (ViewValues<IntView> j(bs[i].bin()); j(); ++j) {
          // Items must be removed in decreasing size!
          s[j.val()].minus(bs[i].size());
          // Can item i still be packed into bin j?
          if (nosum(s[j.val()],
                    l[j.val()].min() - bs[i].size(),
                    l[j.val()].max() - bs[i].size()))
            tc.nq(j.val());
          // Must item i be packed into bin j?
          if (nosum(s[j.val()], l[j.val()].min(), l[j.val()].max()))
            tc.eq(j.val());
        }
        GECODE_ES_CHECK(tc.tell(home,bs[i].bin()));
        if (bs[i].assigned()) {
          int j = bs[i].bin().val();
          l[j].offset(l[j].offset() - bs[i].size());
          t -= bs[i].size();
        } else {
          bs[k++] = bs[i];
        }
      }
      n=k; bs.size(n);
      region.free();
    }

    // Perform lower bound checking
    if (n > 0) {

      {
        // Allocate auxiliary data
        int n_bins = l.size();
        int n_weights_reductions = n_bins + bs.size();
        int* weights_base_reduction =
          region.alloc<int>(n_weights_reductions);
        int* weights_current_reduction =
          region.alloc<int>(n_weights_reductions);
        int* delta_reductions = region.alloc<int>(n_reductions);

        // Initialize reductions
        int capacity_base_reduction = 0;
        calc_reductions(bs, l, weights_base_reduction,
                        capacity_base_reduction, delta_reductions);

        for (int r_idx = 0; r_idx < n_reductions; r_idx += 1) {
          // Calculate reduction parameters
          int& delta = delta_reductions[r_idx];
          long long int capacity_ll =
            static_cast<long long int>(capacity_base_reduction) + delta;

          if ((capacity_ll > 0) &&
              (capacity_ll <= std::numeric_limits<int>::max())) {
            int capacity = static_cast<int>(capacity_ll);
            // Calculate reduction
            int n_not_zero_weights = 0;
            int max_weight = 0;
            int* weights = weights_current_reduction;
            for (int w_idx = 0; w_idx < n_weights_reductions; w_idx += 1) {
              int weight =
                weights_base_reduction[w_idx] + (w_idx < n_bins ? delta : 0);
              n_not_zero_weights += weight != 0;
              max_weight = std::max(max_weight, weight);
              weights[w_idx] = weight;
            }

            // Check lower bounds
            int lower_bound =
              calc_dff_lower_bound<f_ccm1, l_ccm1>
                (weights, n_weights_reductions, capacity,
                 n_not_zero_weights, max_weight);
            if (lower_bound > n_bins)
              return ES_FAILED;

            lower_bound =
              calc_dff_lower_bound<f_mt, l_mt>
                (weights, n_weights_reductions, capacity,
                 n_not_zero_weights, max_weight);
            if (lower_bound > n_bins)
              return ES_FAILED;

            lower_bound =
              calc_dff_lower_bound<f_bj1, l_bj1>
                (weights, n_weights_reductions, capacity,
                 n_not_zero_weights, max_weight);
            if (lower_bound > n_bins)
              return ES_FAILED;

            lower_bound =
              calc_dff_lower_bound<f_vb2, l_vb2>
                (weights, n_weights_reductions, capacity,
                 n_not_zero_weights, max_weight, true);
            if (lower_bound > n_bins)
              return ES_FAILED;

            lower_bound =
              calc_dff_lower_bound<f_fs1, l_fs1>
                (weights, n_weights_reductions, capacity,
                 n_not_zero_weights, max_weight, true);
            if (lower_bound > n_bins)
              return ES_FAILED;

            lower_bound =
              calc_dff_lower_bound<f_rad2, l_rad2>
                (weights, n_weights_reductions, capacity,
                 n_not_zero_weights, max_weight);
            if (lower_bound > n_bins)
              return ES_FAILED;
          }
        }
      }
      region.free();
    }

    return ES_NOFIX;
  }

  ExecStatus
  Pack::post(Home home, ViewArray<OffsetView>& l, ViewArray<Item>& bs) {
    // Sort according to size
    Support::quicksort(&bs[0], bs.size());
    // Total size of items
    int s = 0;
    // Constrain bins
    for (int i=0; i<bs.size(); i++) {
      s += bs[i].size();
      GECODE_ME_CHECK(bs[i].bin().gq(home,0));
      GECODE_ME_CHECK(bs[i].bin().le(home,l.size()));
    }
    // Eliminate zero sized items (which are at the end as the size are sorted)
    {
      int n = bs.size();
      while ((n > 0) && (bs[n-1].size() == 0))
        n--;
      bs.size(n);
    }
    if (bs.size() == 0) {
      // No items to be packed
      for (int i=0; i<l.size(); i++)
        GECODE_ME_CHECK(l[i].eq(home,0));
      return ES_OK;
    } else if (l.size() == 0) {
      // No bins available
      return ES_FAILED;
    } else {
      // Constrain load
      for (int j=0; j<l.size(); j++) {
        GECODE_ME_CHECK(l[j].gq(home,0));
        GECODE_ME_CHECK(l[j].lq(home,s));
      }
      (void) new (home) Pack(home,l,bs);
      return ES_OK;
    }
  }

  void
  Pack::calc_reductions(const ViewArray<Item>& bs,
                        const ViewArray<OffsetView>& l,
                        int* weights_base_reduction,
                        int& capacity_base_reduction,
                        int* delta_reductions) {
    // Reset values
    int n_weights_reductions = l.size() + bs.size();
    for (int w_idx = 0; w_idx < n_weights_reductions; w_idx += 1)
      weights_base_reduction[w_idx] = 0;

    // R0
    int n_bins = l.size();
    int n_items = bs.size();
    capacity_base_reduction = 0;
    for (int b_idx = 0; b_idx < n_bins; b_idx += 1)
      capacity_base_reduction =
        std::max(capacity_base_reduction, l[b_idx].max());
    for (int b_idx = 0; b_idx < n_bins; b_idx += 1)
      weights_base_reduction[b_idx] =
        capacity_base_reduction - l[b_idx].max();
    for (int i_idx = 0; i_idx < n_items; i_idx += 1) {
      bool i_assigned = bs[i_idx].bin().assigned();
      int i_weight = bs[i_idx].size();
      if (i_assigned) {
        int b_idx = bs[i_idx].bin().val();
        weights_base_reduction[b_idx] += i_weight;
      } else {
        weights_base_reduction[n_bins + i_idx] = i_weight;
      }
    }

    // RMin, RMax
    int smallest_virtual_weight = std::numeric_limits<int>::max();
    for (int b_idx = 0; b_idx < n_bins; b_idx += 1)
      smallest_virtual_weight =
        std::min(smallest_virtual_weight, weights_base_reduction[b_idx]);
    delta_reductions[0] = -smallest_virtual_weight;
    delta_reductions[1] = 0;
    delta_reductions[2] = capacity_base_reduction -
      smallest_virtual_weight - smallest_virtual_weight + 1;
  }

}}}

// STATISTICS: int-prop
