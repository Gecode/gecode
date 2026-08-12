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

  const unsigned long long product_support_limit = 200000ULL;

  /// Return whether the Cartesian product can be enumerated safely.
  inline bool
  product_enumerable(const ViewArray<IntView>& x, const IntView& y) {
    unsigned long long n = y.size();
    for (int i=0; i<x.size(); i++) {
      unsigned long long s = x[i].size();
      if ((s != 0ULL) && (n > product_support_limit / s))
        return false;
      n *= s;
    }
    return n <= product_support_limit;
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

  /// Check that one enumerated tuple respects all aliased views.
  inline bool
  product_alias_ok(const ViewArray<IntView>& x, const IntView& y,
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

  /** Enumerate supports. When \a filter is true, intersect every view with
   * its supports. Return whether satisfying and violating assignments exist.
   */
  inline ExecStatus
  product_supports(Space& home, ViewArray<IntView>& x, IntView y,
                   bool positive, bool filter,
                   bool& has_true, bool& has_false) {
    const int n = x.size();
    Region r;
    unsigned int* size = r.alloc<unsigned int>(n+1);
    unsigned int* pos = r.alloc<unsigned int>(n+1);
    int** values = r.alloc<int*>(n+1);
    unsigned long long cap64 = y.size();
    for (int i=0; i<n; i++) cap64 *= x[i].size();
    const unsigned int cap = static_cast<unsigned int>(cap64);
    int** support = filter ? r.alloc<int*>(n+1) : nullptr;

    for (int i=0; i<n; i++) {
      size[i] = x[i].size(); pos[i] = 0;
      values[i] = r.alloc<int>(size[i]);
      unsigned int k=0;
      for (ViewValues<IntView> vi(x[i]); vi(); ++vi)
        values[i][k++] = vi.val();
      if (filter) support[i] = r.alloc<int>(cap);
    }
    size[n] = y.size(); pos[n] = 0;
    values[n] = r.alloc<int>(size[n]);
    unsigned int k=0;
    for (ViewValues<IntView> vi(y); vi(); ++vi)
      values[n][k++] = vi.val();
    if (filter) support[n] = r.alloc<int>(cap);

    int* tuple = r.alloc<int>(n+1);
    unsigned int ns = 0;
    has_true = has_false = false;
    bool more = true;
    while (more) {
      for (int i=0; i<=n; i++) tuple[i] = values[i][pos[i]];
      if (product_alias_ok(x,y,tuple)) {
        int p;
        bool relation = product_value(tuple,n,p) && (p == tuple[n]);
        has_true |= relation; has_false |= !relation;
        if (filter && (relation == positive)) {
          for (int i=0; i<=n; i++) support[i][ns] = tuple[i];
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
  Product::Product(Home home, ViewArray<IntView>& z, IntView w)
    : NaryOnePropagator<IntView,PC_INT_DOM>(home,z,w) {}

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
    : NaryOnePropagator<IntView,PC_INT_DOM>(home,p) {}

  forceinline Actor*
  Product::copy(Space& home) {
    return new (home) Product(home,*this);
  }

  forceinline PropCost
  Product::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x.size()+1);
  }

  inline ExecStatus
  Product::propagate(Space& home, const ModEventDelta&) {
    if (product_enumerable(x,y)) {
      bool ht, hf;
      GECODE_ES_CHECK(product_supports(home,x,y,true,true,ht,hf));
    }
    bool assigned = y.assigned();
    for (int i=0; assigned && (i<x.size()); i++)
      assigned = x[i].assigned();
    if (assigned)
      return home.ES_SUBSUMED(*this);
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
      if (product_enumerable(x,y)) {
        bool ht, hf;
        GECODE_ES_CHECK(product_supports(home,x,y,false,true,ht,hf));
        if (!ht) return home.ES_SUBSUMED(*this);
      }
      return ES_FIX;
    }
    if (product_enumerable(x,y)) {
      bool ht, hf;
      GECODE_ES_CHECK(product_supports(home,x,y,true,false,ht,hf));
      if (!ht) {
        if (rm != RM_PMI) GECODE_ME_CHECK(b.zero_none(home));
        return home.ES_SUBSUMED(*this);
      }
      if (!hf) {
        if (rm != RM_IMP) GECODE_ME_CHECK(b.one_none(home));
        return home.ES_SUBSUMED(*this);
      }
    }
    return ES_FIX;
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
