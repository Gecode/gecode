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

  const unsigned long long product_mod_support_limit = 200000ULL;

  inline bool
  product_mod_enumerable(const ViewArray<IntView>& x, const IntView& y) {
    unsigned long long n = y.size();
    for (int i=0; i<x.size(); i++) {
      const unsigned long long s = x[i].size();
      if ((s != 0ULL) && (n > product_mod_support_limit / s))
        return false;
      n *= s;
    }
    return n <= product_mod_support_limit;
  }

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

  inline bool
  product_mod_alias_ok(const ViewArray<IntView>& x, const IntView& y,
                       const int* v) {
    for (int i=0; i<x.size(); i++) {
      for (int j=0; j<i; j++)
        if ((x[i] == x[j]) && (v[i] != v[j]))
          return false;
      if ((x[i] == y) && (v[i] != v[x.size()]))
        return false;
    }
    return true;
  }

  inline ExecStatus
  product_mod_supports(Space& home, ViewArray<IntView>& x, IntView y, int m,
                       bool positive, bool filter,
                       bool& has_true, bool& has_false) {
    const int n = x.size();
    Region r;
    unsigned int* size = r.alloc<unsigned int>(n+1);
    unsigned int* pos = r.alloc<unsigned int>(n+1);
    int** values = r.alloc<int*>(n+1);
    unsigned long long cap64 = y.size();
    for (int i=0; i<n; i++)
      cap64 *= x[i].size();
    const unsigned int cap = static_cast<unsigned int>(cap64);
    int** support = filter ? r.alloc<int*>(n+1) : nullptr;

    for (int i=0; i<n; i++) {
      size[i] = x[i].size(); pos[i] = 0;
      values[i] = r.alloc<int>(size[i]);
      unsigned int k=0;
      for (ViewValues<IntView> vi(x[i]); vi(); ++vi)
        values[i][k++] = vi.val();
      if (filter)
        support[i] = r.alloc<int>(cap);
    }
    size[n] = y.size(); pos[n] = 0;
    values[n] = r.alloc<int>(size[n]);
    unsigned int k=0;
    for (ViewValues<IntView> vi(y); vi(); ++vi)
      values[n][k++] = vi.val();
    if (filter)
      support[n] = r.alloc<int>(cap);

    int* tuple = r.alloc<int>(n+1);
    unsigned int ns = 0;
    has_true = has_false = false;
    bool more = true;
    while (more) {
      for (int i=0; i<=n; i++)
        tuple[i] = values[i][pos[i]];
      if (product_mod_alias_ok(x,y,tuple)) {
        const bool relation =
          product_mod_value(tuple,n,m) == tuple[n];
        has_true |= relation; has_false |= !relation;
        if (filter && (relation == positive)) {
          for (int i=0; i<=n; i++)
            support[i][ns] = tuple[i];
          ns++;
        }
      }
      int i=n;
      while ((i >= 0) && (++pos[i] == size[i])) {
        pos[i]=0; i--;
      }
      more = i >= 0;
    }

    if (!filter)
      return ES_OK;
    if (ns == 0)
      return ES_FAILED;
    for (int i=0; i<=n; i++) {
      std::sort(support[i],support[i]+ns);
      unsigned int nu=1;
      for (unsigned int j=1; j<ns; j++)
        if (support[i][j] != support[i][nu-1])
          support[i][nu++] = support[i][j];
      Iter::Values::Array si(support[i],nu);
      if (i < n) {
        GECODE_ME_CHECK(x[i].inter_v(home,si,false));
      } else {
        GECODE_ME_CHECK(y.inter_v(home,si,false));
      }
    }
    return ES_OK;
  }

  forceinline
  ProductMod::ProductMod(Home home, ViewArray<IntView>& z, int modulus,
                         IntView w)
    : NaryOnePropagator<IntView,PC_INT_DOM>(home,z,w), m(modulus) {}

  inline ExecStatus
  ProductMod::post(Home home, ViewArray<IntView>& x, int m, IntView y) {
    GECODE_ME_CHECK(y.gq(home,0));
    GECODE_ME_CHECK(y.lq(home,m-1));
    if (x.size() == 0) {
      GECODE_ME_CHECK(y.eq(home,1 % m));
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
    return PropCost::linear(PropCost::HI,x.size()+1);
  }

  inline ExecStatus
  ProductMod::propagate(Space& home, const ModEventDelta&) {
    if (product_mod_enumerable(x,y)) {
      bool ht, hf;
      GECODE_ES_CHECK(product_mod_supports(home,x,y,m,true,true,ht,hf));
    }
    bool assigned = y.assigned();
    for (int i=0; assigned && (i<x.size()); i++)
      assigned = x[i].assigned();
    if (assigned)
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  inline bool
  product_mod_var_enumerable(const ViewArray<IntView>& x,
                             const IntView& m, const IntView& y) {
    unsigned long long n = static_cast<unsigned long long>(m.size()) * y.size();
    if (n > product_mod_support_limit)
      return false;
    for (int i=0; i<x.size(); i++) {
      const unsigned long long s = x[i].size();
      if ((s != 0ULL) && (n > product_mod_support_limit / s))
        return false;
      n *= s;
    }
    return n <= product_mod_support_limit;
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
      if (product_mod_enumerable(x,y)) {
        bool ht, hf;
        GECODE_ES_CHECK(product_mod_supports
                        (home,x,y,m,false,true,ht,hf));
        if (!ht)
          return home.ES_SUBSUMED(*this);
      }
      return ES_FIX;
    }
    if (product_mod_enumerable(x,y)) {
      bool ht, hf;
      GECODE_ES_CHECK(product_mod_supports
                      (home,x,y,m,true,false,ht,hf));
      if (!ht) {
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero_none(home));
        return home.ES_SUBSUMED(*this);
      }
      if (!hf) {
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one_none(home));
        return home.ES_SUBSUMED(*this);
      }
    }
    return ES_FIX;
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
