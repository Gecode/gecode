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

#include <algorithm>

namespace Gecode { namespace Int { namespace Arithmetic {

  /// A representable, conservative interval for an exact product.
  struct ProductInterval {
    int min;
    int max;
  };

  /// Multiply two representable intervals, clipping out-of-range endpoints.
  forceinline ProductInterval
  product_interval_mul(ProductInterval a, int bmin, int bmax) {
    long long int p[4] = {
      static_cast<long long int>(a.min) * bmin,
      static_cast<long long int>(a.min) * bmax,
      static_cast<long long int>(a.max) * bmin,
      static_cast<long long int>(a.max) * bmax
    };
    long long int l = *std::min_element(p,p+4);
    long long int u = *std::max_element(p,p+4);
    l = std::max(static_cast<long long int>(Limits::min),
                 std::min(l,static_cast<long long int>(Limits::max)));
    u = std::max(static_cast<long long int>(Limits::min),
                 std::min(u,static_cast<long long int>(Limits::max)));
    ProductInterval r = {static_cast<int>(l),static_cast<int>(u)};
    return r;
  }

  /// Multiply two representable intervals.
  forceinline ProductInterval
  product_interval_mul(ProductInterval a, ProductInterval b) {
    return product_interval_mul(a,b.min,b.max);
  }

  /// Compute a nonnegative power, saturating at the integer limit.
  forceinline int
  product_power_abs(int x, int n) {
    const long long int a = x < 0 ? -static_cast<long long int>(x) : x;
    long long int p=1;
    for (int i=0; i<n; i++) {
      if ((a != 0) && (p > static_cast<long long int>(Limits::max)/a))
        return Limits::max;
      p *= a;
    }
    return static_cast<int>(p);
  }

  /// Bounds for a repeated occurrence viewed as an integer power.
  forceinline ProductInterval
  product_power_interval(IntView x, int n) {
    const int l=product_power_abs(x.min(),n);
    const int u=product_power_abs(x.max(),n);
    if ((n & 1) != 0) {
      ProductInterval p = {x.min() < 0 ? -l : l,
                           x.max() < 0 ? -u : u};
      return p;
    }
    ProductInterval p = {
      ((x.min() <= 0) && (x.max() >= 0)) ? 0 : std::min(l,u),
      std::max(l,u)
    };
    return p;
  }

  forceinline bool
  product_power_le(int x, int n, int limit) {
    long long int p=1;
    for (int i=0; i<n; i++) {
      if ((x != 0) && (p > static_cast<long long int>(limit)/x))
        return false;
      p *= x;
    }
    return true;
  }

  /// Floor and ceiling of a nonnegative integer root.
  inline int
  product_floor_root(int x, int n) {
    int l=0, u=x;
    while (l < u) {
      const int m=l+(u-l+1)/2;
      if (product_power_le(m,n,x)) l=m; else u=m-1;
    }
    return l;
  }

  forceinline int
  product_ceil_root(int x, int n) {
    if (x <= 0) return 0;
    return product_floor_root(x-1,n)+1;
  }

  /// Compute the hull of quotients by a zero-free interval.
  forceinline ProductInterval
  product_quotient_interval(int ymin, int ymax,
                            int qmin, int qmax) {
    long long int c[4] = {
      ceil_div_xx(static_cast<long long int>(ymin),
                  static_cast<long long int>(qmin)),
      ceil_div_xx(static_cast<long long int>(ymin),
                  static_cast<long long int>(qmax)),
      ceil_div_xx(static_cast<long long int>(ymax),
                  static_cast<long long int>(qmin)),
      ceil_div_xx(static_cast<long long int>(ymax),
                  static_cast<long long int>(qmax))
    };
    long long int f[4] = {
      floor_div_xx(static_cast<long long int>(ymin),
                   static_cast<long long int>(qmin)),
      floor_div_xx(static_cast<long long int>(ymin),
                   static_cast<long long int>(qmax)),
      floor_div_xx(static_cast<long long int>(ymax),
                   static_cast<long long int>(qmin)),
      floor_div_xx(static_cast<long long int>(ymax),
                   static_cast<long long int>(qmax))
    };
    const long long int l = std::max
      (static_cast<long long int>(Limits::min),std::min
       (*std::min_element(c,c+4),static_cast<long long int>(Limits::max)));
    const long long int u = std::max
      (static_cast<long long int>(Limits::min),std::min
       (*std::max_element(f,f+4),static_cast<long long int>(Limits::max)));
    ProductInterval r = {static_cast<int>(l),static_cast<int>(u)};
    return r;
  }

  /// Compute product bounds, optionally omitting one factor.
  inline ProductInterval
  product_interval(const ViewArray<IntView>& x, int omit=-1) {
    ProductInterval r = {1,1};
    for (int i=0; i<x.size(); i++) {
      if (i == omit) continue;
      bool first=true;
      int n=0;
      for (int j=0; j<x.size(); j++)
        if ((j != omit) && (x[j] == x[i])) {
          if (j < i) first=false;
          n++;
        }
      if (first)
        r = product_interval_mul(r,product_power_interval(x[i],n));
    }
    return r;
  }

  /// Evaluate a tuple without overflowing internal arithmetic.
  inline bool
  product_value(const int* v, int n, int& p) {
    for (int i=0; i<n; i++)
      if (v[i] == 0) {
        p = 0;
        return true;
      }
    long long int q = 1;
    for (int i=0; i<n; i++) {
      if (Limits::overflow_mul(q,static_cast<long long int>(v[i])))
        return false;
      q *= static_cast<long long int>(v[i]);
    }
    if (!Limits::valid(q))
      return false;
    p = static_cast<int>(q);
    return true;
  }

  /// Determine the relation when bounds or assignments prove it.
  inline RelTest
  product_status(const ViewArray<IntView>& x, const IntView& y) {
    ProductInterval p = product_interval(x);
    if ((p.max < y.min()) || (p.min > y.max()))
      return RT_FALSE;
    bool assigned = true;
    for (int i=0; assigned && (i<x.size()); i++)
      assigned = x[i].assigned();
    if (!assigned)
      return RT_MAYBE;
    Region r;
    int* v = r.alloc<int>(x.size());
    for (int i=0; i<x.size(); i++) v[i] = x[i].val();
    int q;
    if (!product_value(v,x.size(),q) || !y.in(q))
      return RT_FALSE;
    return y.assigned() ? RT_TRUE : RT_MAYBE;
  }

  forceinline
  Product::Product(Home home, ViewArray<IntView>& z, IntView w, bool n)
    : NaryOnePropagator<IntView,PC_INT_BND>(home,z,w), neg(n) {
    home.notice(*this,AP_WEAKLY);
  }

  inline ExecStatus
  Product::post(Home home, ViewArray<IntView>& x, IntView y, bool neg) {
    for (int i=x.size(); i--;) {
      if (!x[i].assigned())
        continue;
      if (x[i].val() == 0) {
        GECODE_ME_CHECK(y.eq(home,0));
        return ES_OK;
      }
      if ((x[i].val() == 1) || (x[i].val() == -1)) {
        neg ^= x[i].val() == -1;
        x.move_lst(i);
      }
    }
    if (x.size() == 0) {
      GECODE_ME_CHECK(y.eq(home,neg ? -1 : 1));
      return ES_OK;
    }
    (void) new (home) Product(home,x,y,neg);
    return ES_OK;
  }

  forceinline
  Product::Product(Space& home, Product& p)
    : NaryOnePropagator<IntView,PC_INT_BND>(home,p), neg(p.neg) {}

  forceinline Actor*
  Product::copy(Space& home) {
    return new (home) Product(home,*this);
  }

  forceinline PropCost
  Product::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::LO,x.size()+1);
  }

  forceinline size_t
  Product::dispose(Space& home) {
    home.ignore(*this,AP_WEAKLY);
    (void) NaryOnePropagator<IntView,PC_INT_BND>::dispose(home);
    return sizeof(*this);
  }

  inline ExecStatus
  Product::propagate(Space& home, const ModEventDelta&) {
    // Absorb a fixed zero and rewrite when new units have appeared.
    int units=0;
    bool next_neg=neg;
    for (int i=x.size(); i--;) {
      if (!x[i].assigned())
        continue;
      if (x[i].val() == 0) {
        GECODE_ME_CHECK(y.eq(home,0));
        return home.ES_SUBSUMED(*this);
      }
      if ((x[i].val() == 1) || (x[i].val() == -1)) {
        next_neg ^= x[i].val() == -1;
        units++;
      }
    }
    if (units > 0) {
      ViewArray<IntView> z(home,x.size()-units);
      int j=0;
      for (int i=0; i<x.size(); i++)
        if (!x[i].assigned() || ((x[i].val() != 1) && (x[i].val() != -1)))
          z[j++]=x[i];
      GECODE_REWRITE(*this,Product::post(home(*this),z,y,next_neg));
    }

    // Cancel one result occurrence when it is nonzero. If cancellation is
    // not yet possible, a remaining product that cannot be the required unit
    // makes zero the only solution for the result.
    int alias=-1;
    for (int i=0; i<x.size(); i++)
      if (x[i] == y) { alias=i; break; }
    if (alias >= 0) {
      if (y.assigned() && (y.val() == 0))
        return home.ES_SUBSUMED(*this);
      ViewArray<IntView> z(home,x.size()-1);
      for (int i=0, j=0; i<x.size(); i++)
        if (i != alias) z[j++]=x[i];
      const int unit=neg ? -1 : 1;
      if (!y.in(0)) {
        IntVar u(home,unit,unit);
        GECODE_REWRITE(*this,Product::post(home(*this),z,IntView(u)));
      }
      ProductInterval q=product_interval(z);
      if ((unit < q.min) || (unit > q.max)) {
        GECODE_ME_CHECK(y.eq(home,0));
        return home.ES_SUBSUMED(*this);
      }
    }

    // A nonzero result excludes a bounds-visible zero endpoint.
    if (!y.in(0))
      for (int i=0; i<x.size(); i++) {
        if ((x[i].min() == 0) && (x[i].max() > 0))
          GECODE_ME_CHECK(x[i].gq(home,1));
        else if ((x[i].max() == 0) && (x[i].min() < 0))
          GECODE_ME_CHECK(x[i].lq(home,-1));
      }

    // Zero can only arise from a zero factor.
    if (y.assigned() && (y.val() == 0)) {
      int zero=-1;
      for (int i=0; i<x.size(); i++)
        if (x[i].in(0)) {
          if (zero >= 0) { zero=-2; break; }
          zero=i;
        }
      if (zero == -1)
        return ES_FAILED;
      if (zero >= 0) {
        GECODE_ME_CHECK(x[zero].eq(home,0));
        return home.ES_SUBSUMED(*this);
      }
    }

    // Stable one-sided signs determine aggregate parity and zero possibility.
    bool stable=true, strict=true, negative=neg;
    for (int i=0; i<x.size(); i++) {
      if (x[i].max() <= 0) {
        negative=!negative;
        strict &= x[i].max() < 0;
      } else if (x[i].min() >= 0) {
        strict &= x[i].min() > 0;
      } else {
        stable=false; strict=false; break;
      }
    }
    if (stable) {
      if (negative)
        GECODE_ME_CHECK(y.lq(home,strict ? -1 : 0));
      else
        GECODE_ME_CHECK(y.gq(home,strict ? 1 : 0));
    }

    bool modified;
    do {
      modified = false;

      ProductInterval p = product_interval(x);
      {
        ModEvent me = neg ? y.lq(home,-p.min) : y.gq(home,p.min);
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }
      {
        ModEvent me = neg ? y.gq(home,-p.max) : y.lq(home,p.max);
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }

      // Group equal views as powers. Prefix and suffix products provide every
      // omitted group interval in linear time after grouping.
      Region r;
      int* representative=r.alloc<int>(x.size());
      int* exponent=r.alloc<int>(x.size());
      int groups=0;
      for (int i=0; i<x.size(); i++) {
        int g=0;
        while ((g < groups) && !(x[representative[g]] == x[i])) g++;
        if (g == groups) {
          representative[groups]=i; exponent[groups]=1; groups++;
        } else {
          exponent[g]++;
        }
      }
      ProductInterval* power=r.alloc<ProductInterval>(groups);
      ProductInterval* prefix = r.alloc<ProductInterval>(groups+1);
      ProductInterval* suffix = r.alloc<ProductInterval>(groups+1);
      prefix[0] = ProductInterval{1,1};
      for (int g=0; g<groups; g++) {
        power[g]=product_power_interval(x[representative[g]],exponent[g]);
        prefix[g+1]=product_interval_mul(prefix[g],power[g]);
      }
      suffix[groups] = ProductInterval{1,1};
      for (int g=groups; g--;)
        suffix[g]=product_interval_mul(suffix[g+1],power[g]);

      for (int g=0; g<groups; g++) {
        IntView xi=x[representative[g]];
        ProductInterval q=product_interval_mul(prefix[g],suffix[g+1]);
        if ((q.min <= 0) && (q.max >= 0) && y.in(0))
          continue;
        if ((q.min == 0) && (q.max == 0))
          return ES_FAILED;

        bool have=false;
        ProductInterval d = {Limits::max,Limits::min};
        if (q.min < 0) {
          ProductInterval n = product_quotient_interval
            (neg ? -y.max() : y.min(),neg ? -y.min() : y.max(),
             q.min,std::min(q.max,-1));
          d=n; have=true;
        }
        if (q.max > 0) {
          ProductInterval p = product_quotient_interval
            (neg ? -y.max() : y.min(),neg ? -y.min() : y.max(),
             std::max(q.min,1),q.max);
          if (have) {
            d.min=std::min(d.min,p.min); d.max=std::max(d.max,p.max);
          } else {
            d=p; have=true;
          }
        }
        if (!have || (d.min > d.max))
          return ES_FAILED;
        const int n=exponent[g];
        if (n == 1) {
          ModEvent me=xi.gq(home,d.min);
          if (me_failed(me)) return ES_FAILED;
          modified |= me_modified(me);
          me=xi.lq(home,d.max);
          if (me_failed(me)) return ES_FAILED;
          modified |= me_modified(me);
        } else if ((n & 1) != 0) {
          const int l = d.min < 0 ?
            -product_floor_root(-d.min,n) : product_ceil_root(d.min,n);
          const int u = d.max < 0 ?
            -product_ceil_root(-d.max,n) : product_floor_root(d.max,n);
          if (l > u) return ES_FAILED;
          ModEvent me=xi.gq(home,l);
          if (me_failed(me)) return ES_FAILED;
          modified |= me_modified(me);
          me=xi.lq(home,u);
          if (me_failed(me)) return ES_FAILED;
          modified |= me_modified(me);
        } else {
          if (d.max < 0) return ES_FAILED;
          const int lo=product_ceil_root(std::max(d.min,0),n);
          const int hi=product_floor_root(d.max,n);
          if (lo > hi) return ES_FAILED;
          bool have_negative=xi.min() <= -lo;
          bool have_positive=xi.max() >= lo;
          int l=Limits::max, u=Limits::min;
          if (have_negative) {
            l=std::max(xi.min(),-hi); u=std::min(xi.max(),-lo);
            have_negative=l <= u;
          }
          if (have_positive) {
            const int pl=std::max(xi.min(),lo);
            const int pu=std::min(xi.max(),hi);
            have_positive=pl <= pu;
            if (have_positive) {
              if (have_negative) { l=std::min(l,pl); u=std::max(u,pu); }
              else { l=pl; u=pu; }
            }
          }
          if (!have_negative && !have_positive) return ES_FAILED;
          ModEvent me=xi.gq(home,l);
          if (me_failed(me)) return ES_FAILED;
          modified |= me_modified(me);
          me=xi.lq(home,u);
          if (me_failed(me)) return ES_FAILED;
          modified |= me_modified(me);
        }
      }
    } while (modified);

    // Result propagation above can have excluded zero during this call.
    bool endpoint_modified=false;
    if (!y.in(0))
      for (int i=0; i<x.size(); i++) {
        ModEvent me=ME_INT_NONE;
        if ((x[i].min() == 0) && (x[i].max() > 0))
          me=x[i].gq(home,1);
        else if ((x[i].max() == 0) && (x[i].min() < 0))
          me=x[i].lq(home,-1);
        if (me_failed(me)) return ES_FAILED;
        endpoint_modified |= me_modified(me);
      }
    if (endpoint_modified)
      return ES_NOFIX;

    bool factors_assigned = true;
    for (int i=0; factors_assigned && (i<x.size()); i++)
      factors_assigned = x[i].assigned();
    if (factors_assigned) {
      Region r;
      int* v = r.alloc<int>(x.size());
      for (int i=0; i<x.size(); i++) v[i] = x[i].val();
      int p;
      if (!product_value(v,x.size(),p))
        return ES_FAILED;
      GECODE_ME_CHECK(y.eq(home,neg ? -p : p));
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

  template<ReifyMode rm>
  forceinline
  ReProduct<rm>::ReProduct(Home home, ViewArray<IntView>& z, IntView w,
                           BoolView c)
    : Propagator(home), x(z), y(w), b(c) {
    x.subscribe(home,*this,PC_INT_DOM);
    y.subscribe(home,*this,PC_INT_DOM);
    b.subscribe(home,*this,PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReProduct<rm>::post(Home home, ViewArray<IntView>& x, IntView y, BoolView b) {
    if (b.one() && (rm == RM_PMI)) return ES_OK;
    if (b.zero() && (rm == RM_IMP)) return ES_OK;
    if (x.size() == 0) {
      bool t = y.assigned() ? (y.val() == 1) : false;
      if (b.one() && (rm != RM_PMI)) {
        GECODE_ME_CHECK(y.eq(home,1)); return ES_OK;
      }
      if (b.zero() && (rm != RM_IMP)) {
        GECODE_ME_CHECK(y.nq(home,1)); return ES_OK;
      }
      if (!y.in(1)) {
        if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
        return ES_OK;
      }
      if (t) {
        if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
        return ES_OK;
      }
    }
    (void) new (home) ReProduct<rm>(home,x,y,b);
    return ES_OK;
  }

  template<ReifyMode rm>
  forceinline
  ReProduct<rm>::ReProduct(Space& home, ReProduct<rm>& p)
    : Propagator(home,p) {
    x.update(home,p.x); y.update(home,p.y); b.update(home,p.b);
  }

  template<ReifyMode rm>
  forceinline Actor*
  ReProduct<rm>::copy(Space& home) {
    return new (home) ReProduct<rm>(home,*this);
  }

  template<ReifyMode rm>
  forceinline PropCost
  ReProduct<rm>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::HI,x.size()+2);
  }

  template<ReifyMode rm>
  forceinline void
  ReProduct<rm>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_INT_DOM);
    y.reschedule(home,*this,PC_INT_DOM);
    b.reschedule(home,*this,PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReProduct<rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,Product::post(home(*this),x,y));
    }
    if (b.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      switch (product_status(x,y)) {
      case RT_TRUE: return ES_FAILED;
      case RT_FALSE: return home.ES_SUBSUMED(*this);
      case RT_MAYBE: return ES_FIX;
      default: GECODE_NEVER;
      }
    }
    switch (product_status(x,y)) {
    case RT_TRUE:
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one_none(home));
      return home.ES_SUBSUMED(*this);
    case RT_FALSE:
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero_none(home));
      return home.ES_SUBSUMED(*this);
    case RT_MAYBE:
      return ES_FIX;
    default: GECODE_NEVER;
    }
  }

  template<ReifyMode rm>
  forceinline size_t
  ReProduct<rm>::dispose(Space& home) {
    x.cancel(home,*this,PC_INT_DOM);
    y.cancel(home,*this,PC_INT_DOM);
    b.cancel(home,*this,PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}}
