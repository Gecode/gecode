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

  /// Compute the canonical residue without signed overflow.
  inline int
  product_mod_value(const int* v, int n, int m) {
    long long int p = 1 % m;
    for (int i=0; i<n; i++) {
      long long int q = static_cast<long long int>(v[i]) % m;
      if (q < 0)
        q += m;
      // Both factors are smaller than Limits::max, so this fits in int64.
      p = (p*q) % m;
    }
    return static_cast<int>(p);
  }

  /// Return exact representable product bounds, or false on saturation.
  inline bool
  product_mod_interval(const ViewArray<IntView>& x, ProductInterval& p,
                       int omit=-1) {
    p.min=1; p.max=1;
    for (int i=0; i<x.size(); i++)
      if (i != omit) {
        long long int q[4] = {
          static_cast<long long int>(p.min) * x[i].min(),
          static_cast<long long int>(p.min) * x[i].max(),
          static_cast<long long int>(p.max) * x[i].min(),
          static_cast<long long int>(p.max) * x[i].max()
        };
        const long long int l = *std::min_element(q,q+4);
        const long long int u = *std::max_element(q,q+4);
        if (!Limits::valid(l) || !Limits::valid(u))
          return false;
        p.min=static_cast<int>(l); p.max=static_cast<int>(u);
      }
    return true;
  }

  /// Return whether an assigned factor makes the residue identically zero.
  inline bool
  product_mod_zero(const ViewArray<IntView>& x, int m) {
    for (int i=0; i<x.size(); i++)
      if (x[i].assigned() && ((x[i].val() % m) == 0))
        return true;
    return false;
  }

  /// Return whether all factors are assigned and compute their residue.
  inline bool
  product_mod_assigned(const ViewArray<IntView>& x, int m, int& residue) {
    Region r;
    int* v = r.alloc<int>(x.size());
    for (int i=0; i<x.size(); i++) {
      if (!x[i].assigned())
        return false;
      v[i]=x[i].val();
    }
    residue=product_mod_value(v,x.size(),m);
    return true;
  }

  /// Determine the fixed-modulus relation algebraically.
  inline RelTest
  product_mod_status(const ViewArray<IntView>& x, const IntView& y, int m) {
    if ((y.max() < 0) || (y.min() >= m))
      return RT_FALSE;
    const bool zero = (m == 1) || product_mod_zero(x,m);
    if (zero) {
      if (!y.in(0)) return RT_FALSE;
      return y.assigned() ? RT_TRUE : RT_MAYBE;
    }
    int residue;
    if (product_mod_assigned(x,m,residue)) {
      if (!y.in(residue)) return RT_FALSE;
      return y.assigned() ? RT_TRUE : RT_MAYBE;
    }
    ProductInterval p;
    if (product_mod_interval(x,p)) {
      const long long int kl = floor_div_xx
        (static_cast<long long int>(p.min),static_cast<long long int>(m));
      const long long int ku = floor_div_xx
        (static_cast<long long int>(p.max),static_cast<long long int>(m));
      if (kl == ku) {
        const long long int shift=kl*m;
        if ((static_cast<long long int>(p.max)-shift < y.min()) ||
            (static_cast<long long int>(p.min)-shift > y.max()))
          return RT_FALSE;
      }
    }
    return RT_MAYBE;
  }

  /// Extended Euclid for a modular inverse (arguments are coprime).
  inline long long int
  product_mod_inverse(long long int a, long long int m) {
    long long int old_r=a, r=m, old_s=1, s=0;
    while (r != 0) {
      const long long int q=old_r/r;
      const long long int nr=old_r-q*r; old_r=r; r=nr;
      const long long int ns=old_s-q*s; old_s=s; s=ns;
    }
    old_s %= m;
    return old_s < 0 ? old_s+m : old_s;
  }

  inline long long int
  product_mod_gcd(long long int a, long long int b) {
    while (b != 0) {
      const long long int r=a%b; a=b; b=r;
    }
    return a < 0 ? -a : a;
  }

  forceinline
  ProductMod::ProductMod(Home home, ViewArray<IntView>& z, int modulus,
                         IntView w)
    : NaryOnePropagator<IntView,PC_INT_DOM>(home,z,w), m(modulus) {}

  inline ExecStatus
  ProductMod::post(Home home, ViewArray<IntView>& x, int m, IntView y) {
    GECODE_ME_CHECK(y.gq(home,0));
    GECODE_ME_CHECK(y.lq(home,m-1));
    if (m == 1) {
      GECODE_ME_CHECK(y.eq(home,0));
      return ES_OK;
    }
    if (x.size() == 0) {
      GECODE_ME_CHECK(y.eq(home,1 % m));
      return ES_OK;
    }
    if (product_mod_zero(x,m)) {
      GECODE_ME_CHECK(y.eq(home,0));
      return ES_OK;
    }
    int residue;
    if (product_mod_assigned(x,m,residue)) {
      GECODE_ME_CHECK(y.eq(home,residue));
      return ES_OK;
    }
    (void) new (home) ProductMod(home,x,m,y);
    return ES_OK;
  }

  forceinline
  ProductMod::ProductMod(Space& home, ProductMod& p)
    : NaryOnePropagator<IntView,PC_INT_DOM>(home,p), m(p.m) {}

  forceinline Actor*
  ProductMod::copy(Space& home) {
    return new (home) ProductMod(home,*this);
  }

  forceinline PropCost
  ProductMod::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::LO,x.size()+1);
  }

  inline ExecStatus
  ProductMod::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(y.gq(home,0));
    GECODE_ME_CHECK(y.lq(home,m-1));

    if (product_mod_zero(x,m)) {
      GECODE_ME_CHECK(y.eq(home,0));
      return home.ES_SUBSUMED(*this);
    }

    int residue;
    if (product_mod_assigned(x,m,residue)) {
      GECODE_ME_CHECK(y.eq(home,residue));
      return home.ES_SUBSUMED(*this);
    }

    bool modified;
    do {
      modified=false;
      ProductInterval p;
      if (product_mod_interval(x,p)) {
        const long long int kl = floor_div_xx
          (static_cast<long long int>(p.min),static_cast<long long int>(m));
        const long long int ku = floor_div_xx
          (static_cast<long long int>(p.max),static_cast<long long int>(m));
        if (kl == ku) {
          const long long int shift=kl*m;
          {
            const long long int l=static_cast<long long int>(p.min)-shift;
            ModEvent me=y.gq(home,static_cast<int>(l));
            if (me_failed(me)) return ES_FAILED;
            modified |= me_modified(me);
          }
          {
            const long long int u=static_cast<long long int>(p.max)-shift;
            ModEvent me=y.lq(home,static_cast<int>(u));
            if (me_failed(me)) return ES_FAILED;
            modified |= me_modified(me);
          }

          // Within one quotient band, invert the exact-product interval.
          const long long int tmin=static_cast<long long int>(y.min())+shift;
          const long long int tmax=static_cast<long long int>(y.max())+shift;
          for (int i=0; i<x.size(); i++) {
            ProductInterval q;
            if (product_mod_interval(x,q,i) &&
                ((q.min > 0) || (q.max < 0))) {
              long long int c[4] = {
                ceil_div_xx(tmin,static_cast<long long int>(q.min)),
                ceil_div_xx(tmin,static_cast<long long int>(q.max)),
                ceil_div_xx(tmax,static_cast<long long int>(q.min)),
                ceil_div_xx(tmax,static_cast<long long int>(q.max))
              };
              long long int f[4] = {
                floor_div_xx(tmin,static_cast<long long int>(q.min)),
                floor_div_xx(tmin,static_cast<long long int>(q.max)),
                floor_div_xx(tmax,static_cast<long long int>(q.min)),
                floor_div_xx(tmax,static_cast<long long int>(q.max))
              };
              long long int l=*std::min_element(c,c+4);
              long long int u=*std::max_element(f,f+4);
              l=std::max(l,static_cast<long long int>(Limits::min));
              u=std::min(u,static_cast<long long int>(Limits::max));
              if (l > u) return ES_FAILED;
              {
                ModEvent me=x[i].gq(home,static_cast<int>(l));
                if (me_failed(me)) return ES_FAILED;
                modified |= me_modified(me);
              }
              {
                ModEvent me=x[i].lq(home,static_cast<int>(u));
                if (me_failed(me)) return ES_FAILED;
                modified |= me_modified(me);
              }
            }
          }
        }
      }
    } while (modified);

    // If only one factor is free and the result is assigned, solve the
    // resulting linear congruence and tighten to its extreme solutions.
    bool congruence_modified=false;
    if (y.assigned()) {
      int free=-1;
      long long int cofactor=1 % m;
      for (int i=0; i<x.size(); i++) {
        if (x[i].assigned()) {
          long long int r=static_cast<long long int>(x[i].val()) % m;
          if (r < 0) r += m;
          cofactor=(cofactor*r) % m;
        } else if (free < 0) {
          free=i;
        } else {
          free=-2; break;
        }
      }
      if (free >= 0) {
        const long long int g=product_mod_gcd(cofactor,m);
        if ((y.val() % g) != 0)
          return ES_FAILED;
        const long long int step=m/g;
        long long int r=0;
        if (step > 1) {
          const long long int a=cofactor/g;
          const long long int b=y.val()/g;
          r=(product_mod_inverse(a % step,step) * b) % step;
          if (r < 0) r += step;
        }
        const long long int l=r + ceil_div_xx
          (static_cast<long long int>(x[free].min())-r,step)*step;
        const long long int u=r + floor_div_xx
          (static_cast<long long int>(x[free].max())-r,step)*step;
        if (l > u) return ES_FAILED;
        {
          ModEvent me=x[free].gq(home,static_cast<int>(l));
          if (me_failed(me)) return ES_FAILED;
          congruence_modified |= me_modified(me);
        }
        {
          ModEvent me=x[free].lq(home,static_cast<int>(u));
          if (me_failed(me)) return ES_FAILED;
          congruence_modified |= me_modified(me);
        }
      }
    }

    if (product_mod_assigned(x,m,residue)) {
      GECODE_ME_CHECK(y.eq(home,residue));
      return home.ES_SUBSUMED(*this);
    }
    return congruence_modified ? ES_NOFIX : ES_FIX;
  }

  inline bool
  product_mod_var_enumerable(const ViewArray<IntView>& x,
                             const IntView& m, const IntView& y) {
    const unsigned long long support_limit=200000ULL;
    unsigned long long n = static_cast<unsigned long long>(m.size()) * y.size();
    if (n > support_limit)
      return false;
    for (int i=0; i<x.size(); i++) {
      const unsigned long long s = x[i].size();
      if ((s != 0ULL) && (n > support_limit / s))
        return false;
      n *= s;
    }
    return n <= support_limit;
  }

  inline bool
  product_mod_var_alias_ok(const ViewArray<IntView>& x,
                           const IntView& m, const IntView& y,
                           const int* v) {
    const int mi = x.size();
    const int yi = mi+1;
    if ((m == y) && (v[mi] != v[yi]))
      return false;
    for (int i=0; i<x.size(); i++) {
      for (int j=0; j<i; j++)
        if ((x[i] == x[j]) && (v[i] != v[j]))
          return false;
      if ((x[i] == m) && (v[i] != v[mi]))
        return false;
      if ((x[i] == y) && (v[i] != v[yi]))
        return false;
    }
    return true;
  }

  inline ExecStatus
  product_mod_var_supports(Space& home, ViewArray<IntView>& x,
                           IntView m, IntView y, bool positive=true,
                           bool filter=true, bool* has_true_out=nullptr,
                           bool* has_false_out=nullptr) {
    const int n = x.size();
    const int nv = n+2;
    Region r;
    unsigned int* size = r.alloc<unsigned int>(nv);
    unsigned int* pos = r.alloc<unsigned int>(nv);
    int** values = r.alloc<int*>(nv);
    unsigned long long cap64 = static_cast<unsigned long long>(m.size()) *
      y.size();
    for (int i=0; i<n; i++)
      cap64 *= x[i].size();
    const unsigned int cap = static_cast<unsigned int>(cap64);
    int** support = filter ? r.alloc<int*>(nv) : nullptr;

    for (int i=0; i<n; i++) {
      size[i] = x[i].size(); pos[i] = 0;
      values[i] = r.alloc<int>(size[i]);
      unsigned int k=0;
      for (ViewValues<IntView> vi(x[i]); vi(); ++vi)
        values[i][k++] = vi.val();
      if (filter) support[i] = r.alloc<int>(cap);
    }
    size[n] = m.size(); pos[n] = 0;
    values[n] = r.alloc<int>(size[n]);
    unsigned int k=0;
    for (ViewValues<IntView> vi(m); vi(); ++vi)
      values[n][k++] = vi.val();
    if (filter) support[n] = r.alloc<int>(cap);
    size[n+1] = y.size(); pos[n+1] = 0;
    values[n+1] = r.alloc<int>(size[n+1]);
    k=0;
    for (ViewValues<IntView> vi(y); vi(); ++vi)
      values[n+1][k++] = vi.val();
    if (filter) support[n+1] = r.alloc<int>(cap);

    int* tuple = r.alloc<int>(nv);
    unsigned int ns = 0;
    bool has_true = false, has_false = false;
    bool more = true;
    while (more) {
      for (int i=0; i<nv; i++)
        tuple[i] = values[i][pos[i]];
      if (product_mod_var_alias_ok(x,m,y,tuple)) {
        const bool relation = (tuple[n] > 0) && (tuple[n+1] >= 0) &&
          (tuple[n+1] < tuple[n]) &&
          (product_mod_value(tuple,n,tuple[n]) == tuple[n+1]);
        has_true |= relation; has_false |= !relation;
        if (filter && (relation == positive)) {
          for (int i=0; i<nv; i++)
            support[i][ns] = tuple[i];
          ns++;
        }
      }
      int i=nv-1;
      while ((i >= 0) && (++pos[i] == size[i])) {
        pos[i]=0; i--;
      }
      more = i >= 0;
    }

    if (has_true_out != nullptr) *has_true_out = has_true;
    if (has_false_out != nullptr) *has_false_out = has_false;
    if (!filter)
      return ES_OK;
    if (ns == 0)
      return ES_FAILED;
    for (int i=0; i<nv; i++) {
      std::sort(support[i],support[i]+ns);
      unsigned int nu=1;
      for (unsigned int j=1; j<ns; j++)
        if (support[i][j] != support[i][nu-1])
          support[i][nu++] = support[i][j];
      Iter::Values::Array si(support[i],nu);
      if (i < n) {
        GECODE_ME_CHECK(x[i].inter_v(home,si,false));
      } else if (i == n) {
        GECODE_ME_CHECK(m.inter_v(home,si,false));
      } else {
        GECODE_ME_CHECK(y.inter_v(home,si,false));
      }
    }
    return ES_OK;
  }

  forceinline
  ProductModVar::ProductModVar(Home home, ViewArray<IntView>& z,
                               IntView modulus, IntView w)
    : Propagator(home), x(z), m(modulus), y(w) {
    x.subscribe(home,*this,PC_INT_DOM);
    m.subscribe(home,*this,PC_INT_DOM);
    y.subscribe(home,*this,PC_INT_DOM);
  }

  inline ExecStatus
  ProductModVar::post(Home home, ViewArray<IntView>& x, IntView m, IntView y) {
    if (m == y)
      return ES_FAILED;
    GECODE_ME_CHECK(m.gq(home,1));
    GECODE_ME_CHECK(y.gq(home,0));
    GECODE_ME_CHECK(y.lq(home,m.max()-1));
    GECODE_ME_CHECK(m.gq(home,y.min()+1));
    if (m.assigned())
      return ProductMod::post(home,x,m.val(),y);
    (void) new (home) ProductModVar(home,x,m,y);
    return ES_OK;
  }

  forceinline
  ProductModVar::ProductModVar(Space& home, ProductModVar& p)
    : Propagator(home,p) {
    x.update(home,p.x); m.update(home,p.m); y.update(home,p.y);
  }

  forceinline Actor*
  ProductModVar::copy(Space& home) {
    return new (home) ProductModVar(home,*this);
  }

  forceinline PropCost
  ProductModVar::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x.size()+2);
  }

  forceinline void
  ProductModVar::reschedule(Space& home) {
    x.reschedule(home,*this,PC_INT_DOM);
    m.reschedule(home,*this,PC_INT_DOM);
    y.reschedule(home,*this,PC_INT_DOM);
  }

  inline ExecStatus
  ProductModVar::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(m.gq(home,1));
    GECODE_ME_CHECK(y.gq(home,0));
    GECODE_ME_CHECK(y.lq(home,m.max()-1));
    GECODE_ME_CHECK(m.gq(home,y.min()+1));
    if (m.assigned())
      GECODE_REWRITE(*this,ProductMod::post(home(*this),x,m.val(),y));
    if (product_mod_var_enumerable(x,m,y))
      GECODE_ES_CHECK(product_mod_var_supports(home,x,m,y));
    if (m.assigned())
      GECODE_REWRITE(*this,ProductMod::post(home(*this),x,m.val(),y));
    bool assigned = m.assigned() && y.assigned();
    for (int i=0; assigned && (i<x.size()); i++)
      assigned = x[i].assigned();
    if (assigned)
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  forceinline size_t
  ProductModVar::dispose(Space& home) {
    x.cancel(home,*this,PC_INT_DOM);
    m.cancel(home,*this,PC_INT_DOM);
    y.cancel(home,*this,PC_INT_DOM);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<ReifyMode rm>
  forceinline
  ReProductModVar<rm>::ReProductModVar(Home home, ViewArray<IntView>& z,
                                       IntView modulus, IntView w, BoolView c)
    : Propagator(home), x(z), m(modulus), y(w), b(c) {
    x.subscribe(home,*this,PC_INT_DOM);
    m.subscribe(home,*this,PC_INT_DOM);
    y.subscribe(home,*this,PC_INT_DOM);
    b.subscribe(home,*this,PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReProductModVar<rm>::post(Home home, ViewArray<IntView>& x, IntView m,
                            IntView y, BoolView b) {
    if (b.one()) {
      if (rm == RM_PMI) return ES_OK;
      return ProductModVar::post(home,x,m,y);
    }
    if (b.zero() && (rm == RM_IMP))
      return ES_OK;
    (void) new (home) ReProductModVar<rm>(home,x,m,y,b);
    return ES_OK;
  }

  template<ReifyMode rm>
  forceinline
  ReProductModVar<rm>::ReProductModVar(Space& home,
                                       ReProductModVar<rm>& p)
    : Propagator(home,p) {
    x.update(home,p.x); m.update(home,p.m); y.update(home,p.y);
    b.update(home,p.b);
  }

  template<ReifyMode rm>
  forceinline Actor*
  ReProductModVar<rm>::copy(Space& home) {
    return new (home) ReProductModVar<rm>(home,*this);
  }

  template<ReifyMode rm>
  forceinline PropCost
  ReProductModVar<rm>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x.size()+3);
  }

  template<ReifyMode rm>
  forceinline void
  ReProductModVar<rm>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_INT_DOM);
    m.reschedule(home,*this,PC_INT_DOM);
    y.reschedule(home,*this,PC_INT_DOM);
    b.reschedule(home,*this,PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReProductModVar<rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI)
        return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,ProductModVar::post(home(*this),x,m,y));
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return home.ES_SUBSUMED(*this);
      if (product_mod_var_enumerable(x,m,y)) {
        bool ht, hf;
        GECODE_ES_CHECK(product_mod_var_supports
                        (home,x,m,y,false,true,&ht,&hf));
        if (!ht)
          return home.ES_SUBSUMED(*this);
      } else if ((m == y) || (m.max() <= 0) || (y.max() < 0) ||
                 (y.min() >= m.max())) {
        return home.ES_SUBSUMED(*this);
      }
      return ES_FIX;
    }

    RelTest rt;
    if (m == y) {
      rt = RT_FALSE;
    } else if ((m.max() <= 0) || (y.max() < 0) || (y.min() >= m.max())) {
      rt = RT_FALSE;
    } else if (product_mod_var_enumerable(x,m,y)) {
      bool ht, hf;
      GECODE_ES_CHECK(product_mod_var_supports
                      (home,x,m,y,true,false,&ht,&hf));
      rt = !ht ? RT_FALSE : (!hf ? RT_TRUE : RT_MAYBE);
    } else {
      rt = RT_MAYBE;
    }
    switch (rt) {
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
  ReProductModVar<rm>::dispose(Space& home) {
    x.cancel(home,*this,PC_INT_DOM);
    m.cancel(home,*this,PC_INT_DOM);
    y.cancel(home,*this,PC_INT_DOM);
    b.cancel(home,*this,PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<ReifyMode rm>
  forceinline
  ReProductMod<rm>::ReProductMod(Home home, ViewArray<IntView>& z,
                                 int modulus, IntView w, BoolView c)
    : Propagator(home), x(z), y(w), b(c), m(modulus) {
    x.subscribe(home,*this,PC_INT_DOM);
    y.subscribe(home,*this,PC_INT_DOM);
    b.subscribe(home,*this,PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReProductMod<rm>::post(Home home, ViewArray<IntView>& x, int m, IntView y,
                         BoolView b) {
    if (b.one() && (rm == RM_PMI))
      return ES_OK;
    if (b.zero() && (rm == RM_IMP))
      return ES_OK;
    if (x.size() == 0) {
      const int identity = 1 % m;
      const bool t = y.assigned() ? (y.val() == identity) : false;
      if (b.one() && (rm != RM_PMI)) {
        GECODE_ME_CHECK(y.eq(home,identity)); return ES_OK;
      }
      if (b.zero() && (rm != RM_IMP)) {
        GECODE_ME_CHECK(y.nq(home,identity)); return ES_OK;
      }
      if (!y.in(identity)) {
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero(home));
        return ES_OK;
      }
      if (t) {
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one(home));
        return ES_OK;
      }
    }
    switch (product_mod_status(x,y,m)) {
    case RT_TRUE:
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    case RT_FALSE:
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    case RT_MAYBE:
      break;
    default: GECODE_NEVER;
    }
    (void) new (home) ReProductMod<rm>(home,x,m,y,b);
    return ES_OK;
  }

  template<ReifyMode rm>
  forceinline
  ReProductMod<rm>::ReProductMod(Space& home, ReProductMod<rm>& p)
    : Propagator(home,p), m(p.m) {
    x.update(home,p.x); y.update(home,p.y); b.update(home,p.b);
  }

  template<ReifyMode rm>
  forceinline Actor*
  ReProductMod<rm>::copy(Space& home) {
    return new (home) ReProductMod<rm>(home,*this);
  }

  template<ReifyMode rm>
  forceinline PropCost
  ReProductMod<rm>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x.size()+2);
  }

  template<ReifyMode rm>
  forceinline void
  ReProductMod<rm>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_INT_DOM);
    y.reschedule(home,*this,PC_INT_DOM);
    b.reschedule(home,*this,PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReProductMod<rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI)
        return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,ProductMod::post(home(*this),x,m,y));
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return home.ES_SUBSUMED(*this);
      switch (product_mod_status(x,y,m)) {
      case RT_TRUE: return ES_FAILED;
      case RT_FALSE: return home.ES_SUBSUMED(*this);
      case RT_MAYBE: return ES_FIX;
      default: GECODE_NEVER;
      }
    }
    switch (product_mod_status(x,y,m)) {
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
  ReProductMod<rm>::dispose(Space& home) {
    x.cancel(home,*this,PC_INT_DOM);
    y.cancel(home,*this,PC_INT_DOM);
    b.cancel(home,*this,PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}}
