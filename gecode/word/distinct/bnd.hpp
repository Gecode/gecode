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
 */

namespace Gecode { namespace Word { namespace Distinct {

  namespace BndSupport {
    /// Ranked endpoint in [-2, 2^64+2], including width-64 sentinels
    class Endpoint {
    public:
      int zone;
      WordValue value;
      forceinline static Endpoint rank(WordValue v) {
        Endpoint e={0,v}; return e;
      }
      forceinline static Endpoint before(WordValue v) {
        if (v >= 2U) return rank(v-2U);
        Endpoint e={-1,2U-v}; return e;
      }
      forceinline static Endpoint after(WordValue v) {
        if (v != ~WordValue(0)) return rank(v+1U);
        Endpoint e={1,0}; return e;
      }
      forceinline Endpoint plus_two(void) const {
        if (zone < 0) {
          if (value > 2U) { Endpoint e={-1,value-2U}; return e; }
          return rank(2U-value);
        }
        if (zone > 0) { Endpoint e={1,value+2U}; return e; }
        if (value <= ~WordValue(0)-2U) return rank(value+2U);
        Endpoint e={1,value-(~WordValue(0)-1U)}; return e;
      }
      forceinline WordValue as_rank(void) const {
        assert(zone == 0); return value;
      }
    };

    forceinline bool operator <(const Endpoint& a, const Endpoint& b) {
      return (a.zone < b.zone) ||
        ((a.zone == b.zone) &&
         ((a.zone < 0) ? (a.value > b.value) : (a.value < b.value)));
    }
    forceinline bool operator ==(const Endpoint& a, const Endpoint& b) {
      return (a.zone == b.zone) && (a.value == b.value);
    }
    forceinline unsigned int
    distance(const Endpoint& a, const Endpoint& b, unsigned int limit) {
      assert(!(a < b));
      WordValue d;
      if ((a.zone == 1) && (b.zone == 0)) {
        WordValue tail=~WordValue(0)-b.value;
        if ((tail >= limit) || (a.value >= limit-tail-1U)) return limit;
        d=tail+1U+a.value;
      } else if ((a.zone == 0) && (b.zone == -1)) {
        if ((a.value >= limit) || (b.value >= limit-a.value)) return limit;
        d=a.value+b.value;
      } else if (a.zone != b.zone) {
        return limit;
      } else {
        d=(a.zone < 0) ? b.value-a.value : a.value-b.value;
      }
      return (d >= static_cast<WordValue>(limit)) ?
        limit : static_cast<unsigned int>(d);
    }
    forceinline bool shorter(unsigned int d, const Endpoint& a,
                             const Endpoint& b, unsigned int limit) {
      return !(a < b) && (d < distance(a,b,limit));
    }
    forceinline bool same(unsigned int d, const Endpoint& a,
                          const Endpoint& b, unsigned int limit) {
      return !(a < b) && (d == distance(a,b,limit));
    }

    class Rank { public: int min, max; };
    class HallInfo {
    public:
      Endpoint bounds;
      int t, h;
      unsigned int d;
    };

    template<class View> class MinIncIdx {
    protected:
      ViewArray<View> x;
    public:
      forceinline MinIncIdx(const ViewArray<View>& x0) : x(x0) {}
      forceinline bool operator ()(int i, int j) {
        return x[i].rank_minimum() < x[j].rank_minimum();
      }
    };
    template<class View> class MaxIncIdx {
    protected:
      ViewArray<View> x;
    public:
      forceinline MaxIncIdx(const ViewArray<View>& x0) : x(x0) {}
      forceinline bool operator ()(int i, int j) {
        return x[i].rank_maximum() < x[j].rank_maximum();
      }
    };

    forceinline void pathset_t(HallInfo h[], int s, int e, int to) {
      int k,l; for (l=s; (k=l) != e; h[k].t=to) l=h[k].t;
    }
    forceinline void pathset_h(HallInfo h[], int s, int e, int to) {
      int k,l; for (l=s; (k=l) != e; h[k].h=to) l=h[k].h;
    }
    forceinline int pathmin_h(const HallInfo h[], int i) {
      while (h[i].h < i) i=h[i].h; return i;
    }
    forceinline int pathmin_t(const HallInfo h[], int i) {
      while (h[i].t < i) i=h[i].t; return i;
    }
    forceinline int pathmax_h(const HallInfo h[], int i) {
      while (h[i].h > i) i=h[i].h; return i;
    }
    forceinline int pathmax_t(const HallInfo h[], int i) {
      while (h[i].t > i) i=h[i].t; return i;
    }
  }

  template<class View>
  forceinline Bnd<View>::Bnd(Home home, ViewArray<View>& x0)
    : NaryPropagator<View,PC_WORD_BND>(home,x0) {}
  template<class View>
  forceinline Bnd<View>::Bnd(Space& home, Bnd& p)
    : NaryPropagator<View,PC_WORD_BND>(home,p) {}

  template<class View>
  ExecStatus Bnd<View>::narrow(Home home, ViewArray<View>& x) {
    using namespace BndSupport;
    const int n=x.size();
    Region r;
    int* mins=r.alloc<int>(n); int* maxs=r.alloc<int>(n);
    for (int i=0; i<n; i++) mins[i]=maxs[i]=i;
    MinIncIdx<View> min_inc(x); Support::quicksort<int,MinIncIdx<View> >(mins,n,min_inc);
    MaxIncIdx<View> max_inc(x); Support::quicksort<int,MaxIncIdx<View> >(maxs,n,max_inc);

    HallInfo* hall=r.alloc<HallInfo>(2*n+2); Rank* rank=r.alloc<Rank>(n);
    int nb=0;
    Endpoint minimum=Endpoint::rank(x[mins[0]].rank_minimum());
    Endpoint maximum=Endpoint::after(x[maxs[0]].rank_maximum());
    Endpoint last=Endpoint::before(x[mins[0]].rank_minimum());
    hall[0].bounds=last;
    int i=0,j=0;
    while (true) {
      if ((i < n) && (minimum < maximum)) {
        if (!(minimum == last)) hall[++nb].bounds=last=minimum;
        rank[mins[i]].min=nb;
        if (++i < n) minimum=Endpoint::rank(x[mins[i]].rank_minimum());
      } else {
        if (!(maximum == last)) hall[++nb].bounds=last=maximum;
        rank[maxs[j]].max=nb;
        if (++j == n) break;
        maximum=Endpoint::after(x[maxs[j]].rank_maximum());
      }
    }
    hall[nb+1].bounds=hall[nb].bounds.plus_two();

    ExecStatus es=ES_FIX;
    const unsigned int cap=static_cast<unsigned int>(n)+1U;
    for (i=nb+2; --i;) {
      hall[i].t=hall[i].h=i-1;
      hall[i].d=distance(hall[i].bounds,hall[i-1].bounds,cap);
    }
    for (i=0; i<n; i++) {
      int x0=rank[maxs[i]].min, z=pathmax_t(hall,x0+1), jt=hall[z].t;
      if (--hall[z].d == 0) hall[z=pathmax_t(hall,hall[z].t=z+1)].t=jt;
      pathset_t(hall,x0+1,z,z);
      int y=rank[maxs[i]].max;
      if (shorter(hall[z].d,hall[z].bounds,hall[y].bounds,cap)) {
        return ES_FAILED;
      }
      if (hall[x0].h > x0) {
        int w=pathmax_h(hall,hall[x0].h);
        ModEvent me=x[maxs[i]].narrow_range(home,hall[w].bounds.as_rank(),
                                             x[maxs[i]].rank_maximum());
        if (me_failed(me)) return ES_FAILED;
        if ((me == ME_WORD_DOM) || (me == ME_WORD_VAL)) es=ES_NOFIX;
        pathset_h(hall,x0,w,w);
      }
      if (same(hall[z].d,hall[z].bounds,hall[y].bounds,cap)) {
        pathset_h(hall,hall[y].h,jt-1,y); hall[y].h=jt-1;
      }
    }
    for (i=nb+1; i--;) {
      hall[i].t=hall[i].h=i+1;
      hall[i].d=distance(hall[i+1].bounds,hall[i].bounds,cap);
    }
    for (i=n; --i>=0;) {
      int x0=rank[mins[i]].max, z=pathmin_t(hall,x0-1), jt=hall[z].t;
      if (--hall[z].d == 0) hall[z=pathmin_t(hall,hall[z].t=z-1)].t=jt;
      pathset_t(hall,x0-1,z,z);
      int y=rank[mins[i]].min;
      if (shorter(hall[z].d,hall[y].bounds,hall[z].bounds,cap)) {
        return ES_FAILED;
      }
      if (hall[x0].h < x0) {
        int w=pathmin_h(hall,hall[x0].h);
        ModEvent me=x[mins[i]].narrow_range(home,x[mins[i]].rank_minimum(),
                                             hall[w].bounds.as_rank()-1U);
        if (me_failed(me)) return ES_FAILED;
        if ((me == ME_WORD_DOM) || (me == ME_WORD_VAL)) es=ES_NOFIX;
        pathset_h(hall,x0,w,w);
      }
      if (same(hall[z].d,hall[y].bounds,hall[z].bounds,cap)) {
        pathset_h(hall,hall[y].h,jt+1,y); hall[y].h=jt+1;
      }
    }
    return es;
  }

  template<class View>
  ExecStatus Bnd<View>::post(Home home, ViewArray<View>& x) {
    if (x.size() < 2) return ES_OK;
    if (x.size() == 2)
      return Rel::Nq<View,View>::post(home,x[0],x[1]);
    ExecStatus es;
    do { es=narrow(home,x); GECODE_ES_CHECK(es); } while (es == ES_NOFIX);
    bool assigned=true;
    for (int i=0; assigned && (i<x.size()); i++) assigned=x[i].assigned();
    if (!assigned) (void) new (home) Bnd(home,x);
    return ES_OK;
  }
  template<class View> Actor* Bnd<View>::copy(Space& home) {
    return new (home) Bnd(home,*this);
  }
  template<class View>
  PropCost Bnd<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::LO,x.size());
  }
  template<class View>
  ExecStatus Bnd<View>::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,x); GECODE_ES_CHECK(es);
    bool assigned=true;
    for (int i=0; assigned && (i<x.size()); i++) assigned=x[i].assigned();
    return assigned ? home.ES_SUBSUMED(*this) : es;
  }

}}}

// STATISTICS: word-prop
