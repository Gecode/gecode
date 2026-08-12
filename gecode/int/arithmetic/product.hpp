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

  /// Compute product bounds, optionally omitting one factor.
  inline ProductInterval
  product_interval(const ViewArray<IntView>& x, int omit=-1) {
    ProductInterval r = {1,1};
    for (int i=0; i<x.size(); i++)
      if (i != omit)
        r = product_interval_mul(r,x[i].min(),x[i].max());
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
  Product::Product(Home home, ViewArray<IntView>& z, IntView w)
    : NaryOnePropagator<IntView,PC_INT_BND>(home,z,w) {}

  inline ExecStatus
  Product::post(Home home, ViewArray<IntView>& x, IntView y) {
    if (x.size() == 0) {
      GECODE_ME_CHECK(y.eq(home,1));
      return ES_OK;
    }
    (void) new (home) Product(home,x,y);
    return ES_OK;
  }

  forceinline
  Product::Product(Space& home, Product& p)
    : NaryOnePropagator<IntView,PC_INT_BND>(home,p) {}

  forceinline Actor*
  Product::copy(Space& home) {
    return new (home) Product(home,*this);
  }

  forceinline PropCost
  Product::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::LO,x.size()+1);
  }

  inline ExecStatus
  Product::propagate(Space& home, const ModEventDelta&) {
    bool modified;
    do {
      modified = false;

      ProductInterval p = product_interval(x);
      {
        ModEvent me = y.gq(home,p.min);
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }
      {
        ModEvent me = y.lq(home,p.max);
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }

      for (int i=0; i<x.size(); i++) {
        ProductInterval q = product_interval(x,i);
        if ((q.min > 0) || (q.max < 0)) {
          long long int c[4] = {
            ceil_div_xx(static_cast<long long int>(y.min()),
                        static_cast<long long int>(q.min)),
            ceil_div_xx(static_cast<long long int>(y.min()),
                        static_cast<long long int>(q.max)),
            ceil_div_xx(static_cast<long long int>(y.max()),
                        static_cast<long long int>(q.min)),
            ceil_div_xx(static_cast<long long int>(y.max()),
                        static_cast<long long int>(q.max))
          };
          long long int f[4] = {
            floor_div_xx(static_cast<long long int>(y.min()),
                         static_cast<long long int>(q.min)),
            floor_div_xx(static_cast<long long int>(y.min()),
                         static_cast<long long int>(q.max)),
            floor_div_xx(static_cast<long long int>(y.max()),
                         static_cast<long long int>(q.min)),
            floor_div_xx(static_cast<long long int>(y.max()),
                         static_cast<long long int>(q.max))
          };
          long long int l = *std::min_element(c,c+4);
          long long int u = *std::max_element(f,f+4);
          l = std::max(l,static_cast<long long int>(Limits::min));
          u = std::min(u,static_cast<long long int>(Limits::max));
          if (l > u)
            return ES_FAILED;
          {
            ModEvent me = x[i].gq(home,static_cast<int>(l));
            if (me_failed(me)) return ES_FAILED;
            modified |= me_modified(me);
          }
          {
            ModEvent me = x[i].lq(home,static_cast<int>(u));
            if (me_failed(me)) return ES_FAILED;
            modified |= me_modified(me);
          }
        }
      }
    } while (modified);

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
      GECODE_ME_CHECK(y.eq(home,p));
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
    return PropCost::linear(PropCost::HI,x.size()+2);
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
