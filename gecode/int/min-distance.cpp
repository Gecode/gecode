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


#include <gecode/int.hh>

#include <algorithm>
#include <utility>

namespace Gecode {

  IntDistance::Data::Data(int n0, const IntArgs& m)
    : n(n0), matrix(m) {}

  IntDistance::Data::Data(int n0, std::function<int(int,int)> f)
    : n(n0), function(std::move(f)) {}

  IntDistance::Data::~Data(void) {}

  IntDistance::IntDistance(int n, const IntArgs& matrix) {
    Int::Limits::positive(n,"IntDistance");
    if (static_cast<long long int>(n)*n != matrix.size())
      throw Int::ArgumentSizeMismatch("IntDistance");
    for (int a=0; a<n; a++)
      for (int b=0; b<n; b++) {
        Int::Limits::nonnegative(matrix[a*n+b],"IntDistance");
        if (((a == b) && (matrix[a*n+b] != 0)) ||
            (matrix[a*n+b] != matrix[b*n+a]))
          throw Int::IllegalOperation("IntDistance");
      }
    object(new Data(n,matrix));
  }

  IntDistance::IntDistance(int n, std::function<int(int,int)> f) {
    Int::Limits::positive(n,"IntDistance");
    if (!f)
      throw InvalidFunction("IntDistance");
    object(new Data(n,std::move(f)));
  }

}

/**
 * \namespace Gecode::Int::MinDistance
 *
 * The forward-bound and greedy conflict-matching algorithms follow:
 *   M. Z. Lagerkvist, Propagation Algorithms for the Minimum-Distance
 *   Constraint over Selected Points, ModRef 2026.
 *   https://github.com/zayenz/modref2026-minimum-distance-propagators
 *
 * \brief Minimum-distance propagators over indexed sites
 */
namespace Gecode { namespace Int { namespace MinDistance {

  /// Inclusive requirement, with an empty matrix denoting all zeroes
  forceinline int
  requirement(const IntSharedArray& r, int n, int i, int j) {
    return !r ? 0 : r[i*n+j];
  }

  /// Prune the partner of an assigned endpoint
  ExecStatus
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

  /// Pair maximum and its witness
  struct PairCache {
    int a, b, upper;
  };

  /// Recompute the maximum distance between current domains
  void
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
  ExecStatus
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
  ExecStatus
  zero(Space& home, const ViewArray<IntView>& x, const IntDistance& d,
       const IntSharedArray& r, bool& changed, bool& realized) {
    int count = 0, first = -1, second = -1;
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

  /// Forward-bound actor for one selected-position pair
  class Pair : public Propagator {
  protected:
    IntView x, y, z;
    IntDistance d;
    int r;
    Pair(Home home, IntView x0, IntView y0, IntView z0,
         const IntDistance& d0, int r0)
      : Propagator(home), x(x0), y(y0), z(z0), d(d0), r(r0) {
      home.notice(*this,AP_DISPOSE);
      x.subscribe(home,*this,PC_INT_DOM);
      y.subscribe(home,*this,PC_INT_DOM);
      z.subscribe(home,*this,PC_INT_BND);
    }
    Pair(Space& home, Pair& p)
      : Propagator(home,p), d(p.d), r(p.r) {
      x.update(home,p.x); y.update(home,p.y); z.update(home,p.z);
    }
  public:
    static ExecStatus post(Home home, IntView x, IntView y, IntView z,
                           const IntDistance& d, int r) {
      if (x.assigned() && y.assigned()) {
        int v = d(x.val(),y.val());
        if (v < r)
          return ES_FAILED;
        GECODE_ME_CHECK(z.lq(home,v));
      } else {
        (void) new (home) Pair(home,x,y,z,d,r);
      }
      return ES_OK;
    }
    virtual Actor* copy(Space& home) {
      return new (home) Pair(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::quadratic(PropCost::HI,d.size());
    }
    virtual void reschedule(Space& home) {
      x.reschedule(home,*this,PC_INT_DOM);
      y.reschedule(home,*this,PC_INT_DOM);
      z.reschedule(home,*this,PC_INT_BND);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      int threshold = std::max(r,z.min());
      if (x.assigned() && y.assigned()) {
        int v = d(x.val(),y.val());
        if (v < threshold)
          return ES_FAILED;
        GECODE_ME_CHECK(z.lq(home,v));
        return home.ES_SUBSUMED(*this);
      }
      bool changed = false;
      if (x.assigned()) {
        GECODE_ES_CHECK(forward(home,x,y,threshold,d,changed));
      } else if (y.assigned()) {
        GECODE_ES_CHECK(forward(home,y,x,threshold,d,changed));
      }
      PairCache c;
      pair_max(x,y,d,c);
      if (c.upper < threshold)
        return ES_FAILED;
      ModEvent me = z.lq(home,c.upper);
      GECODE_ME_CHECK(me);
      return (changed || me_modified(me) || (z.min() > threshold)) ? ES_NOFIX : ES_FIX;
    }
    virtual size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      x.cancel(home,*this,PC_INT_DOM);
      y.cancel(home,*this,PC_INT_DOM);
      z.cancel(home,*this,PC_INT_BND);
      d.~IntDistance();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
  };

  /// Equality finalizer and zero witness for the pairwise organization
  class Finalize : public MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND> {
  protected:
    IntDistance d;
    IntSharedArray r;
    Finalize(Home home, ViewArray<IntView>& x, IntView z,
             const IntDistance& d0, const IntSharedArray& r0)
      : MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>(home,x,z), d(d0), r(r0) {
      home.notice(*this,AP_DISPOSE);
    }
    Finalize(Space& home, Finalize& p)
      : MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>(home,p), d(p.d), r(p.r) {}
  public:
    static ExecStatus post(Home home, ViewArray<IntView>& x, IntView z,
                           const IntDistance& d, const IntSharedArray& r) {
      (void) new (home) Finalize(home,x,z,d,r);
      return ES_OK;
    }
    virtual Actor* copy(Space& home) {
      return new (home) Finalize(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::quadratic(PropCost::HI,x.size());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      if (x.assigned()) {
        GECODE_ES_CHECK(assigned(home,x,y,d,r));
        return home.ES_SUBSUMED(*this);
      }
      if (y.max() != 0)
        return ES_FIX;
      bool changed = false, realized = false;
      GECODE_ES_CHECK(zero(home,x,d,r,changed,realized));
      if (realized)
        return home.ES_SUBSUMED(*this);
      return changed ? ES_NOFIX : ES_FIX;
    }
    virtual size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      d.~IntDistance(); r.~IntSharedArray();
      (void) MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>::dispose(home);
      return sizeof(*this);
    }
  };

  /// Single forward-bound actor with domain advisors and pair witnesses
  class AdvisorForward : public Propagator {
  protected:
    class XAdvisor : public Advisor {
    public:
      int index;
      XAdvisor(Space& home, Propagator& p, Council<XAdvisor>& c, int i)
        : Advisor(home,p,c), index(i) {}
      XAdvisor(Space& home, XAdvisor& a)
        : Advisor(home,a), index(a.index) {}
    };
    ViewArray<IntView> x;
    IntView z;
    IntDistance d;
    IntSharedArray r;
    Council<XAdvisor> council;
    PairCache* cache;
    bool* dirty;
    int last_min;
    bool z_shared;
    AdvisorForward(Home home, ViewArray<IntView>& x0, IntView z0,
                   const IntDistance& d0, const IntSharedArray& r0)
      : Propagator(home), x(x0), z(z0), d(d0), r(r0), council(home),
        last_min(-1), z_shared(false) {
      home.notice(*this,AP_DISPOSE);
      Space& s = home;
      int np = static_cast<int>(static_cast<long long int>(x.size())*(x.size()-1)/2);
      cache = s.alloc<PairCache>(np);
      for (int i=0; i<np; i++)
        cache[i] = {-1,-1,-1};
      dirty = s.alloc<bool>(x.size());
      for (int i=0; i<x.size(); i++) {
        dirty[i] = true;
        z_shared |= shared(x[i],z);
        x[i].subscribe(home,*new (home) XAdvisor(home,*this,council,i));
      }
      z.subscribe(home,*this,PC_INT_BND);
    }
    AdvisorForward(Space& home, AdvisorForward& p)
      : Propagator(home,p), d(p.d), r(p.r), council(home),
        last_min(p.last_min), z_shared(p.z_shared) {
      x.update(home,p.x); z.update(home,p.z); council.update(home,p.council);
      int np = static_cast<int>(static_cast<long long int>(x.size())*(x.size()-1)/2);
      cache = home.alloc<PairCache>(np);
      std::copy(p.cache,p.cache+np,cache);
      dirty = home.alloc<bool>(x.size());
      std::copy(p.dirty,p.dirty+x.size(),dirty);
    }
  public:
    static ExecStatus post(Home home, ViewArray<IntView>& x, IntView z,
                           const IntDistance& d, const IntSharedArray& r) {
      (void) new (home) AdvisorForward(home,x,z,d,r);
      return ES_OK;
    }
    virtual Actor* copy(Space& home) {
      return new (home) AdvisorForward(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::quadratic(PropCost::HI,
                                static_cast<unsigned int>(x.size())+d.size());
    }
    virtual void reschedule(Space& home) {
      z.reschedule(home,*this,PC_INT_BND);
      // Advisors have no reschedule method; ensure a manual reschedule also
      // revisits every pair, even if no new domain event has been delivered.
      last_min = -1;
    }
    virtual ExecStatus advise(Space&, Advisor& a, const Delta&) {
      dirty[static_cast<XAdvisor&>(a).index] = true;
      return ES_NOFIX;
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
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
            if (!(all_dirty || dirty[i] || dirty[j]))
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
            if (all_dirty || changed || dirty[i] || dirty[j]) {
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
        for (int i=0; i<x.size(); i++)
          dirty[i] = false;
        last_min = z.min();
        return ES_FIX;
      }
    }
    virtual size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      for (Advisors<XAdvisor> a(council); a(); ++a)
        x[a.advisor().index].cancel(home,a.advisor());
      council.dispose(home);
      z.cancel(home,*this,PC_INT_BND);
      home.free<PairCache>(cache,static_cast<int>(static_cast<long long int>(x.size())*(x.size()-1)/2));
      home.free<bool>(dirty,x.size());
      d.~IntDistance(); r.~IntSharedArray();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
  };

  /// Independent matching-based objective upper bound
  class Matching : public MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND> {
  protected:
    IntDistance d;
    Matching(Home home, ViewArray<IntView>& x, IntView z,
             const IntDistance& d0)
      : MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>(home,x,z), d(d0) {
      home.notice(*this,AP_DISPOSE);
    }
    Matching(Space& home, Matching& p)
      : MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>(home,p), d(p.d) {}
    bool refuted(int* sites, int n, bool* matched, int threshold) const {
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
  public:
    static ExecStatus post(Home home, ViewArray<IntView>& x, IntView z,
                           const IntDistance& d) {
      if (!z.assigned())
        (void) new (home) Matching(home,x,z,d);
      return ES_OK;
    }
    virtual Actor* copy(Space& home) {
      return new (home) Matching(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::cubic(PropCost::HI,d.size());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
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
      int lo = y.min(), hi = y.max();
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
    virtual size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      d.~IntDistance();
      (void) MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND>::dispose(home);
      return sizeof(*this);
    }
  };

  /// Validate the organization and number of positions
  void check(const IntVarArgs& x, MinDistancePropKind kind) {
    if (x.size() < 2)
      throw TooFewArguments("Int::minimum_distance");
    if ((kind != MDP_DECOMPOSED) && (kind != MDP_SINGLE))
      throw IllegalOperation("Int::minimum_distance");
    Limits::nonnegative(static_cast<long long int>(x.size())*(x.size()-1)/2,
                        "Int::minimum_distance");
  }

  /// Shared posting for the two public overloads
  ExecStatus post(Home home, const IntVarArgs& x, IntView z,
                  const IntDistance& d, const IntSharedArray& r,
                  MinDistancePropKind kind, IntPropLevel ipl) {
    ViewArray<IntView> views(home,x);
    for (int i=0; i<views.size(); i++) {
      GECODE_ME_CHECK(views[i].gq(home,0));
      GECODE_ME_CHECK(views[i].le(home,d.size()));
    }
    GECODE_ME_CHECK(z.gq(home,0));
    if (views.assigned())
      return assigned(home,views,z,d,r);
    if (kind == MDP_DECOMPOSED) {
      for (int i=0; i<views.size(); i++)
        for (int j=i+1; j<views.size(); j++)
          GECODE_ES_CHECK(Pair::post(home,views[i],views[j],z,d,
                                    requirement(r,views.size(),i,j)));
      GECODE_ES_CHECK(Finalize::post(home,views,z,d,r));
    } else {
      GECODE_ES_CHECK(AdvisorForward::post(home,views,z,d,r));
    }
    if (ba(ipl) != IPL_BASIC)
      GECODE_ES_CHECK(Matching::post(home,views,z,d));
    return ES_OK;
  }

}}}

namespace Gecode {

  void minimum_distance(Home home, const IntVarArgs& x, IntVar z,
                        const IntDistance& d, IntPropLevel ipl,
                        MinDistancePropKind kind) {
    Int::MinDistance::check(x,kind);
    GECODE_POST;
    IntSharedArray r;
    GECODE_ES_FAIL(Int::MinDistance::post(home,x,z,d,r,kind,ipl));
  }

  void minimum_distance(Home home, const IntVarArgs& x, IntVar z,
                        const IntDistance& d, const IntArgs& r,
                        IntPropLevel ipl, MinDistancePropKind kind) {
    Int::MinDistance::check(x,kind);
    if (static_cast<long long int>(x.size())*x.size() != r.size())
      throw Int::ArgumentSizeMismatch("Int::minimum_distance");
    for (int i=0; i<x.size(); i++)
      for (int j=0; j<x.size(); j++) {
        Int::Limits::nonnegative(r[i*x.size()+j],"Int::minimum_distance");
        if (((i == j) && (r[i*x.size()+j] != 0)) ||
            (r[i*x.size()+j] != r[j*x.size()+i]))
          throw Int::IllegalOperation("Int::minimum_distance");
      }
    GECODE_POST;
    IntSharedArray requirements(r);
    GECODE_ES_FAIL(Int::MinDistance::post(home,x,z,d,requirements,kind,ipl));
  }

}

// STATISTICS: int-prop
