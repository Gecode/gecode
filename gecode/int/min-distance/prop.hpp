/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

namespace Gecode { namespace Int { namespace MinDistance {


  /// Inclusive requirement, with an empty matrix denoting all zeroes
  forceinline int
  requirement(const IntSharedArray& r, int n, int i, int j) {
    return !r ? 0 : r[i*n+j];
  }

  /// Prune the partner of an assigned endpoint
  inline ExecStatus
  forward(Space& home, IntView a, IntView b, int threshold,
          const IntDistance& d, bool& changed) {
    Region region;
    int* remove = region.alloc<int>(b.size());
    int nr = 0;
    for (ViewValues<IntView> v(b); v(); ++v)
      if (d(a.val(),v.val()) < threshold)
        remove[nr++] = v.val();
    if (nr != 0) {
      Iter::Values::Array values(remove,static_cast<unsigned int>(nr));
      ModEvent me = b.minus_v(home,values,false);
      GECODE_ME_CHECK(me);
      changed |= me_modified(me);
    }
    return ES_OK;
  }

  /// Recompute the maximum distance between current domains
  inline void
  pair_max(IntView x, IntView y, const IntDistance& d, PairCache& c) {
    c.upper = -1;
    for (ViewValues<IntView> a(x); a(); ++a)
      for (ViewValues<IntView> b(y); b(); ++b) {
        int v = d(a.val(),b.val());
        if (v > c.upper) {
          c.a = a.val(); c.b = b.val(); c.upper = v;
        }
      }
  }

  /// Validate and bind a complete assignment
  inline ExecStatus
  assigned(Space& home, const ViewArray<IntView>& x, IntView z,
           const IntDistance& d, const IntSharedArray& r) {
    int minimum = Limits::max;
    for (int i=0; i<x.size(); i++)
      for (int j=i+1; j<x.size(); j++) {
        int v = d(x[i].val(),x[j].val());
        if (v < requirement(r,x.size(),i,j))
          return ES_FAILED;
        minimum = std::min(minimum,v);
      }
    GECODE_ME_CHECK(z.eq(home,minimum));
    return ES_OK;
  }

  /// Enforce an actual zero-distance witness when the objective is zero
  inline ExecStatus
  zero(Space& home, const ViewArray<IntView>& x, const IntDistance& d,
       const IntSharedArray& r, bool& changed, bool& realized) {
    int count = 0;
    int first = -1;
    int second = -1;
    for (int i=0; i<x.size(); i++)
      for (int j=i+1; j<x.size(); j++) {
        if (requirement(r,x.size(),i,j) != 0)
          continue;
        if (x[i].assigned() && x[j].assigned() &&
            (d(x[i].val(),x[j].val()) == 0)) {
          realized = true;
          return ES_OK;
        }
        bool supported = false;
        for (ViewValues<IntView> a(x[i]); a() && !supported; ++a)
          for (ViewValues<IntView> b(x[j]); b(); ++b)
            if (d(a.val(),b.val()) == 0) {
              supported = true;
              break;
            }
        if (supported) {
          first = i; second = j; count++;
        }
      }
    if (count == 0)
      return ES_FAILED;
    if (count == 1) {
      // Only this pair can attain zero, so both endpoints need zero support.
      Region region;
      for (int pass=0; pass<2; pass++) {
        IntView a = x[pass == 0 ? first : second];
        IntView b = x[pass == 0 ? second : first];
        int* remove = region.alloc<int>(a.size());
        int nr = 0;
        for (ViewValues<IntView> v(a); v(); ++v) {
          bool supported = false;
          for (ViewValues<IntView> w(b); w(); ++w)
            if (d(v.val(),w.val()) == 0) {
              supported = true;
              break;
            }
          if (!supported)
            remove[nr++] = v.val();
        }
        if (nr != 0) {
          Iter::Values::Array values(remove,static_cast<unsigned int>(nr));
          ModEvent me = a.minus_v(home,values,false);
          GECODE_ME_CHECK(me);
          changed |= me_modified(me);
        }
      }
    }
    return ES_OK;
  }

  forceinline
  Pair::Pair(Home home, IntView x0, IntView y0, IntView z0,
             const IntDistance& d0, int r0)
    : MixTernaryPropagator<IntView,PC_INT_DOM,IntView,PC_INT_DOM,
                                IntView,PC_INT_BND>(home,x0,y0,z0),
      d(d0), r(r0) {
    home.notice(*this,AP_DISPOSE);
  }

  forceinline
  Pair::Pair(Space& home, Pair& p)
    : MixTernaryPropagator<IntView,PC_INT_DOM,IntView,PC_INT_DOM,
                                IntView,PC_INT_BND>(home,p),
      d(p.d), r(p.r) {}

  inline ExecStatus
  Pair::post(Home home, IntView x0, IntView x1, IntView x2,
             const IntDistance& d, int r) {
    if (x0.assigned() && x1.assigned()) {
      int v = d(x0.val(),x1.val());
      if (v < r)
        return ES_FAILED;
      GECODE_ME_CHECK(x2.lq(home,v));
    } else {
      (void) new (home) Pair(home,x0,x1,x2,d,r);
    }
    return ES_OK;
  }

  inline Actor*
  Pair::copy(Space& home) {
    return new (home) Pair(home,*this);
  }

  inline PropCost
  Pair::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::HI,d.size());
  }

  inline ExecStatus
  Pair::propagate(Space& home, const ModEventDelta&) {
    int threshold = std::max(r,x2.min());
    if (x0.assigned() && x1.assigned()) {
      int v = d(x0.val(),x1.val());
      if (v < threshold)
        return ES_FAILED;
      GECODE_ME_CHECK(x2.lq(home,v));
      return home.ES_SUBSUMED(*this);
    }
    bool changed = false;
    if (x0.assigned()) {
      GECODE_ES_CHECK(forward(home,x0,x1,threshold,d,changed));
    } else if (x1.assigned()) {
      GECODE_ES_CHECK(forward(home,x1,x0,threshold,d,changed));
    }
    PairCache c;
    pair_max(x0,x1,d,c);
    if (c.upper < threshold)
      return ES_FAILED;
    ModEvent me = x2.lq(home,c.upper);
    GECODE_ME_CHECK(me);
    return (changed || me_modified(me) || (x2.min() > threshold)) ? ES_NOFIX : ES_FIX;
  }

  inline size_t
  Pair::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    d.~IntDistance();
    (void) MixTernaryPropagator<IntView,PC_INT_DOM,IntView,PC_INT_DOM,
                                IntView,PC_INT_BND>::dispose(home);
    return sizeof(*this);
  }

  forceinline
  Finalize::Finalize(Home home, ViewArray<IntView>& x, IntView z,
                     const IntDistance& d0, const IntSharedArray& r0)
    : MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>(home,x,z),
      d(d0), r(r0) {
    home.notice(*this,AP_DISPOSE);
  }

  forceinline
  Finalize::Finalize(Space& home, Finalize& p)
    : MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>(home,p),
      d(p.d), r(p.r) {}

  inline ExecStatus
  Finalize::post(Home home, ViewArray<IntView>& x, IntView z,
                 const IntDistance& d, const IntSharedArray& r) {
    (void) new (home) Finalize(home,x,z,d,r);
    return ES_OK;
  }

  inline Actor*
  Finalize::copy(Space& home) {
    return new (home) Finalize(home,*this);
  }

  inline PropCost
  Finalize::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::HI,x.size());
  }

  inline ExecStatus
  Finalize::propagate(Space& home, const ModEventDelta&) {
    if (x.assigned()) {
      GECODE_ES_CHECK(assigned(home,x,y,d,r));
      return home.ES_SUBSUMED(*this);
    }
    if (y.max() != 0)
      return ES_FIX;
    bool changed = false;
    bool realized = false;
    GECODE_ES_CHECK(zero(home,x,d,r,changed,realized));
    if (realized)
      return home.ES_SUBSUMED(*this);
    return changed ? ES_NOFIX : ES_FIX;
  }

  inline size_t
  Finalize::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    d.~IntDistance(); r.~IntSharedArray();
    (void) MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>::dispose(home);
    return sizeof(*this);
  }

  forceinline
  AdvisorForward::XAdvisor::XAdvisor(Space& home, Propagator& p,
                                     Council<XAdvisor>& c, int i)
    : Advisor(home,p,c), index(i) {}

  forceinline
  AdvisorForward::XAdvisor::XAdvisor(Space& home, XAdvisor& a)
    : Advisor(home,a), index(a.index) {}

  forceinline
  AdvisorForward::AdvisorForward(Home home, ViewArray<IntView>& x0, IntView z0,
                                 const IntDistance& d0, const IntSharedArray& r0)
    : Propagator(home), x(x0), z(z0), d(d0), r(r0), council(home),
      dirty(static_cast<Space&>(home),static_cast<unsigned int>(x.size()),true),
      last_min(-1), z_shared(false) {
    home.notice(*this,AP_DISPOSE);
    Space& s = home;
    int np = static_cast<int>(static_cast<long long int>(x.size())*(x.size()-1)/2);
    cache = s.alloc<PairCache>(np);
    for (int i=0; i<np; i++)
      cache[i] = {-1,-1,-1};
    for (int i=0; i<x.size(); i++) {
      z_shared |= shared(x[i],z);
      x[i].subscribe(home,*new (home) XAdvisor(home,*this,council,i));
    }
    z.subscribe(home,*this,PC_INT_BND);
  }

  forceinline
  AdvisorForward::AdvisorForward(Space& home, AdvisorForward& p)
    : Propagator(home,p), d(p.d), r(p.r), council(home),
      dirty(home,p.dirty), last_min(p.last_min), z_shared(p.z_shared) {
    x.update(home,p.x); z.update(home,p.z); council.update(home,p.council);
    int np = static_cast<int>(static_cast<long long int>(x.size())*(x.size()-1)/2);
    cache = home.alloc<PairCache>(np);
    std::copy(p.cache,p.cache+np,cache);
  }

  inline ExecStatus
  AdvisorForward::post(Home home, ViewArray<IntView>& x, IntView z,
                       const IntDistance& d, const IntSharedArray& r) {
    (void) new (home) AdvisorForward(home,x,z,d,r);
    return ES_OK;
  }

  inline Actor*
  AdvisorForward::copy(Space& home) {
    return new (home) AdvisorForward(home,*this);
  }

  inline PropCost
  AdvisorForward::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::HI,
                              static_cast<unsigned int>(x.size())+d.size());
  }

  forceinline void
  AdvisorForward::reschedule(Space& home) {
    z.reschedule(home,*this,PC_INT_BND);
    // Advisors have no reschedule method; ensure a manual reschedule also
    // revisits every pair, even if no new domain event has been delivered.
    last_min = -1;
  }

  inline ExecStatus
  AdvisorForward::advise(Space&, Advisor& a, const Delta&) {
    dirty.set(static_cast<XAdvisor&>(a).index);
    return ES_NOFIX;
  }

  inline ExecStatus
  AdvisorForward::propagate(Space& home, const ModEventDelta&) {
    for (;;) {
      if (x.assigned()) {
        GECODE_ES_CHECK(assigned(home,x,z,d,r));
        return home.ES_SUBSUMED(*this);
      }
      int zmin = z.min();
      bool all_dirty = zmin != last_min;
      bool changed = false;
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++) {
          if (!(all_dirty || dirty.get(i) || dirty.get(j)))
            continue;
          int t = std::max(zmin,requirement(r,x.size(),i,j));
          if (x[i].assigned() && !x[j].assigned()) {
            GECODE_ES_CHECK(forward(home,x[i],x[j],t,d,changed));
          } else if (x[j].assigned() && !x[i].assigned()) {
            GECODE_ES_CHECK(forward(home,x[j],x[i],t,d,changed));
          }
        }
      if (z.assigned() && (z.val() == 0)) {
        bool realized = false;
        GECODE_ES_CHECK(zero(home,x,d,r,changed,realized));
      }
      int upper = Limits::max;
      int k = 0;
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++,k++) {
          PairCache& c = cache[k];
          if (all_dirty || changed || dirty.get(i) || dirty.get(j)) {
            // Unchanged pairs retain their witnesses. For a changed pair,
            // a surviving witness still attains the maximum; shrinking
            // domains cannot expose a larger distance.
            if ((c.upper < 0) || !x[i].in(c.a) || !x[j].in(c.b))
              pair_max(x[i],x[j],d,c);
            if (c.upper < requirement(r,x.size(),i,j))
              return ES_FAILED;
          }
          upper = std::min(upper,c.upper);
        }
      ModEvent me = z.lq(home,upper);
      GECODE_ME_CHECK(me);
      if (x.assigned()) {
        GECODE_ES_CHECK(assigned(home,x,z,d,r));
        return home.ES_SUBSUMED(*this);
      }
      if (changed || (z.min() != zmin) ||
          (me_modified(me) && (z_shared || (z.max() == 0)))) {
        // Own pruning can affect a pair already visited in this pass.
        // Restart all assigned-endpoint work; valid maxima remain cached.
        last_min = -1;
        continue;
      }
      dirty.clearall();
      last_min = z.min();
      return ES_FIX;
    }
  }

  inline size_t
  AdvisorForward::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    for (Advisors<XAdvisor> a(council); a(); ++a)
      x[a.advisor().index].cancel(home,a.advisor());
    council.dispose(home);
    z.cancel(home,*this,PC_INT_BND);
    int np = static_cast<int>(
      static_cast<long long int>(x.size())*(x.size()-1)/2);
    home.free<PairCache>(cache,np);
    dirty.dispose(home);
    d.~IntDistance(); r.~IntSharedArray();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  forceinline
  Matching::Matching(Home home, ViewArray<IntView>& x, IntView z,
                     const IntDistance& d0)
    : MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>(home,x,z), d(d0) {
    home.notice(*this,AP_DISPOSE);
    home.notice(*this,AP_WEAKLY);
  }

  forceinline
  Matching::Matching(Space& home, Matching& p)
    : MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>(home,p), d(p.d) {}

  inline bool
  Matching::refuted(int* sites, int n, bool* matched, int threshold) const {
    if (threshold == 0)
      return false; // Repeated sites are permitted at distance zero.
    std::fill(matched,matched+n,false);
    int edges = 0;
    for (int a=0; a<n; a++) {
      if (matched[a])
        continue;
      for (int b=a+1; b<n; b++)
        if (!matched[b] && (d(sites[a],sites[b]) < threshold)) {
          matched[a] = matched[b] = true;
          edges++;
          break;
        }
    }
    return n-edges < x.size();
  }

  inline ExecStatus
  Matching::post(Home home, ViewArray<IntView>& x, IntView z,
                 const IntDistance& d) {
    if (!z.assigned())
      (void) new (home) Matching(home,x,z,d);
    return ES_OK;
  }

  inline Actor*
  Matching::copy(Space& home) {
    return new (home) Matching(home,*this);
  }

  inline PropCost
  Matching::cost(const Space&, const ModEventDelta&) const {
    return PropCost::cubic(PropCost::HI,d.size());
  }

  inline ExecStatus
  Matching::propagate(Space& home, const ModEventDelta&) {
    if (y.assigned())
      return home.ES_SUBSUMED(*this);
    Region region;
    bool* present = region.alloc<bool>(d.size());
    std::fill(present,present+d.size(),false);
    for (int i=0; i<x.size(); i++)
      for (ViewValues<IntView> v(x[i]); v(); ++v)
        present[v.val()] = true;
    int* sites = region.alloc<int>(d.size());
    int n = 0;
    for (int i=0; i<d.size(); i++)
      if (present[i])
        sites[n++] = i;
    bool* matched = region.alloc<bool>(n);
    int lo = y.min();
    int hi = y.max();
    if (refuted(sites,n,matched,lo))
      return ES_FAILED;
    // A failed certificate at t excludes every objective >= t. Greedy
    // certificate sizes themselves need not be monotone, so this search
    // can miss tighter bounds but cannot delete a feasible objective.
    while (lo < hi) {
      int mid = lo+(hi-lo+1)/2;
      if (refuted(sites,n,matched,mid))
        hi = mid-1;
      else
        lo = mid;
    }
    ModEvent me = y.lq(home,hi);
    GECODE_ME_CHECK(me);
    if (y.assigned())
      return home.ES_SUBSUMED(*this);
    return me_modified(me) ? ES_NOFIX : ES_FIX;
  }

  inline size_t
  Matching::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    home.ignore(*this,AP_WEAKLY);
    d.~IntDistance();
    (void) MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>::dispose(home);
    return sizeof(*this);
  }

}}}

// STATISTICS: int-prop
