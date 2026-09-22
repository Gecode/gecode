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

  /// Multiply intervals exactly, rejecting overflow rather than clipping.
  inline bool
  product_mod_interval_mul(ProductInterval a, ProductInterval b,
                           ProductInterval& p) {
    long long int q[4] = {
      static_cast<long long int>(a.min)*b.min,
      static_cast<long long int>(a.min)*b.max,
      static_cast<long long int>(a.max)*b.min,
      static_cast<long long int>(a.max)*b.max
    };
    const long long int l=*std::min_element(q,q+4);
    const long long int u=*std::max_element(q,q+4);
    if (!Limits::valid(l) || !Limits::valid(u))
      return false;
    p.min=static_cast<int>(l); p.max=static_cast<int>(u);
    return true;
  }

  /// Return exact representable product bounds, or false on overflow.
  inline bool
  product_mod_interval(const ViewArray<IntView>& x, ProductInterval& p,
                       int a=1) {
    p.min=a; p.max=a;
    for (int i=0; i<x.size(); i++) {
      ProductInterval q={x[i].min(),x[i].max()};
      if (!product_mod_interval_mul(p,q,p))
        return false;
    }
    return true;
  }

  /// Build all cofactor bounds in linear time, without modular saturation.
  inline bool
  product_mod_intervals(const ViewArray<IntView>& x,
                        ProductInterval* prefix, ProductInterval* suffix,
                        int a=1) {
    prefix[0]=ProductInterval{a,a};
    suffix[x.size()]=ProductInterval{1,1};
    for (int i=0; i<x.size(); i++) {
      ProductInterval q={x[i].min(),x[i].max()};
      if (!product_mod_interval_mul(prefix[i],q,prefix[i+1]))
        return false;
    }
    for (int i=x.size(); i--;) {
      ProductInterval q={x[i].min(),x[i].max()};
      if (!product_mod_interval_mul(q,suffix[i+1],suffix[i]))
        return false;
    }
    return true;
  }

  /// Reduce assigned factors and count the remaining factor occurrences.
  inline int
  product_mod_coefficient(const ViewArray<IntView>& x, int m,
                          int& free_count) {
    long long int p=1 % m;
    free_count=0;
    for (int i=0; i<x.size(); i++) {
      if (!x[i].assigned()) {
        free_count++;
      } else {
        long long int q=static_cast<long long int>(x[i].val()) % m;
        if (q < 0) q+=m;
        p=(p*q) % m;
      }
    }
    return static_cast<int>(p);
  }

  /// Fold assigned occurrences into modular and representable exact coefficients.
  inline void
  product_mod_fold(ViewArray<IntView>& x, int m, int& c, int& a) {
    for (int i=x.size(); i--;)
      if (x[i].assigned()) {
        const int v=x[i].val();
        c=static_cast<int>((static_cast<long long int>(c)*v) % m);
        if (c < 0) c+=m;
        const long long int p=static_cast<long long int>(a)*v;
        // Zero marks an unavailable exact coefficient; c remains exact modulo m.
        a=Limits::valid(p) ? static_cast<int>(p) : 0;
        x.move_lst(i);
      }
  }

  /// Drop assigned units without changing a product with an unknown modulus.
  inline void
  product_mod_units(ViewArray<IntView>& x) {
    for (int i=x.size(); i--;)
      if (x[i].assigned() && (x[i].val() == 1))
        x.move_lst(i);
  }

  /// A nonzero residue excludes bounds-visible zero factors.
  inline ExecStatus
  product_mod_nonzero(Space& home, ViewArray<IntView>& x, bool& modified) {
    for (int i=0; i<x.size(); i++) {
      ModEvent me=ME_INT_NONE;
      if (x[i].min() == 0)
        me=x[i].gq(home,1);
      else if (x[i].max() == 0)
        me=x[i].lq(home,-1);
      if (me_failed(me)) return ES_FAILED;
      modified |= me_modified(me);
    }
    return ES_OK;
  }

  /// Determine the fixed-modulus relation algebraically.
  inline RelTest
  product_mod_status(const ViewArray<IntView>& x, const IntView& y, int m) {
    if ((y.max() < 0) || (y.min() >= m))
      return RT_FALSE;
    int free_count;
    const int c=product_mod_coefficient(x,m,free_count);
    if ((c == 0) || (free_count == 0)) {
      if (!y.in(c)) return RT_FALSE;
      return y.assigned() ? RT_TRUE : RT_MAYBE;
    }
    const int g=gcd_value(c,m);
    const long long int l=ceil_div_xx
      (static_cast<long long int>(std::max(0,y.min())),
       static_cast<long long int>(g))*g;
    if (l > std::min(m-1,y.max()))
      return RT_FALSE;
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

  /// Extreme solutions to c*x = y modulo m, or an empty interval.
  inline ProductInterval
  product_mod_congruence(ProductInterval x, int c, int m, int y) {
    const int g=gcd_value(c,m);
    if ((y % g) != 0)
      return ProductInterval{1,0};
    const long long int step=m/g;
    long long int r=0;
    if (step > 1) {
      const long long int a=c/g, b=y/g;
      r=(product_mod_inverse(a % step,step)*b) % step;
      if (r < 0) r+=step;
    }
    const long long int l=r + ceil_div_xx
      (static_cast<long long int>(x.min)-r,step)*step;
    const long long int u=r + floor_div_xx
      (static_cast<long long int>(x.max)-r,step)*step;
    if (l > u)
      return ProductInterval{1,0};
    // Nonempty bounds lie inside x, so narrowing to int is safe.
    return ProductInterval{static_cast<int>(l),static_cast<int>(u)};
  }

  forceinline
  ProductMod::ProductMod(Home home, ViewArray<IntView>& z, int modulus,
                         IntView w, int c0, int a0)
    : NaryOnePropagator<IntView,PC_INT_BND>(home,z,w),
      m(modulus), c(c0), a(a0) {}

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
    int c=1, a=1;
    product_mod_fold(x,m,c,a);
    if ((c == 0) || (x.size() == 0)) {
      GECODE_ME_CHECK(y.eq(home,c));
      return ES_OK;
    }
    (void) new (home) ProductMod(home,x,m,y,c,a);
    return ES_OK;
  }

  forceinline
  ProductMod::ProductMod(Space& home, ProductMod& p)
    : NaryOnePropagator<IntView,PC_INT_BND>(home,p),
      m(p.m), c(p.c), a(p.a) {}

  forceinline Actor*
  ProductMod::copy(Space& home) {
    return new (home) ProductMod(home,*this);
  }

  forceinline PropCost
  ProductMod::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x.size()+1);
  }

  forceinline size_t
  ProductMod::dispose(Space& home) {
    (void) NaryOnePropagator<IntView,PC_INT_BND>::dispose(home);
    return sizeof(*this);
  }

  inline ExecStatus
  ProductMod::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(y.gq(home,0));
    GECODE_ME_CHECK(y.lq(home,m-1));

    Region region;
    ProductInterval* prefix=NULL;
    ProductInterval* suffix=NULL;
    bool modified;
    do {
      modified=false;
      product_mod_fold(x,m,c,a);
      if ((c == 0) || (x.size() == 0)) {
        GECODE_ME_CHECK(y.eq(home,c));
        return home.ES_SUBSUMED(*this);
      }
      const unsigned int old_size=y.size();
      GECODE_ES_CHECK(gcd_multiple_bounds(home,y,gcd_value(c,m)));
      modified |= old_size != y.size();
      if (y.min() > 0)
        GECODE_ES_CHECK(product_mod_nonzero(home,x,modified));

      // One remaining occurrence gives a linear congruence.
      if ((x.size() == 1) && y.assigned()) {
        const ProductInterval bounds={x[0].min(),x[0].max()};
        const ProductInterval solutions=
          product_mod_congruence(bounds,c,m,y.val());
        if (solutions.min > solutions.max)
          return ES_FAILED;
        ModEvent me=x[0].gq(home,solutions.min);
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
        me=x[0].lq(home,solutions.max);
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }
      ProductInterval p;
      if ((a != 0) && product_mod_interval(x,p,a)) {
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

          // Allocate cofactors only when interval inversion can contribute.
          if (prefix == NULL) {
            prefix=region.alloc<ProductInterval>(x.size()+1);
            suffix=region.alloc<ProductInterval>(x.size()+1);
          }
          if (!product_mod_intervals(x,prefix,suffix,a))
            continue;

          // Within one quotient band, invert the exact-product interval.
          const long long int tmin=static_cast<long long int>(y.min())+shift;
          const long long int tmax=static_cast<long long int>(y.max())+shift;
          for (int i=0; i<x.size(); i++) {
            ProductInterval q;
            if (product_mod_interval_mul(prefix[i],suffix[i+1],q) &&
                ((q.min > 0) || (q.max < 0))) {
              const ProductWideInterval d=
                product_quotient_bounds(tmin,tmax,q.min,q.max);
              const long long int l=std::max
                (d.min,static_cast<long long int>(Limits::min));
              const long long int u=std::min
                (d.max,static_cast<long long int>(Limits::max));
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

    return ES_FIX;
  }

  /// Whether the modulus occurs among the factors.
  inline bool
  product_mod_var_mod_factor(const ViewArray<IntView>& x, const IntView& m) {
    for (int i=0; i<x.size(); i++)
      if (x[i] == m)
        return true;
    return false;
  }

  /// Compute the exact assigned product when it fits in a signed 64-bit value.
  inline bool
  product_mod_var_exact(const ViewArray<IntView>& x, long long int& p) {
    long long int q=1;
    for (int i=0; i<x.size(); i++) {
      if (!x[i].assigned())
        return false;
      const long long int v=x[i].val();
      if (Limits::overflow_mul(q,v))
        return false;
      q *= v;
    }
    p=q;
    return true;
  }

  /// Tighten divisor bounds for a nonzero difference without enumeration.
  inline bool
  product_mod_var_divisor_bounds(int lower, int upper, long long int d,
                                 int& least, int& greatest) {
    const unsigned long long int ad = d < 0
      ? static_cast<unsigned long long int>(-(d+1))+1ULL
      : static_cast<unsigned long long int>(d);
    assert(ad > 0);

    least=lower;
    greatest=upper;

    // A positive divisor cannot exceed the absolute difference.
    if (ad <= static_cast<unsigned long long int>(Limits::max))
      greatest=std::min(greatest,static_cast<int>(ad));
    if (least > greatest)
      return false;

    // If floor(ad/m) is fixed throughout the remaining interval, there is
    // at most one possible divisor. This is constant-time quotient reasoning;
    // it does not inspect the values in the modulus domain.
    const unsigned long long int q0 =
      ad / static_cast<unsigned long long int>(greatest);
    const unsigned long long int q1 =
      ad / static_cast<unsigned long long int>(least);
    if (q0 == q1) {
      if ((q0 == 0) || ((ad % q0) != 0))
        return false;
      const unsigned long long int candidate=ad/q0;
      if ((candidate < static_cast<unsigned long long int>(least)) ||
          (candidate > static_cast<unsigned long long int>(greatest)) ||
          (candidate > static_cast<unsigned long long int>(Limits::max)))
        return false;
      least=greatest=static_cast<int>(candidate);
    }
    return true;
  }

  /// Propagate a nonnegative representable product with a variable modulus.
  inline ExecStatus
  product_mod_var_ranges(Home home, ViewArray<IntView>& x, IntView m,
                         IntView y) {
    Region region;
    ProductInterval* prefix=NULL;
    ProductInterval* suffix=NULL;
    bool modified;
    do {
      modified=false;
      if (y.min() > 0)
        GECODE_ES_CHECK(product_mod_nonzero(home,x,modified));
      for (int i=0; i<x.size(); i++)
        if (x[i].min() < 0)
          return ES_OK;
      ProductInterval p;
      if (!product_mod_interval(x,p))
        return ES_OK;

      {
        ModEvent me=y.lq(home,std::min(p.max,m.max()-1));
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }

      const long long int kmin =
        static_cast<long long int>(p.min) / m.max();
      const long long int kmax =
        static_cast<long long int>(p.max) / m.min();
      if (kmin != kmax)
        continue;

      const long long int k=kmin;
      const long long int rmin =
        static_cast<long long int>(p.min)-k*m.max();
      const long long int rmax =
        static_cast<long long int>(p.max)-k*m.min();
      {
        ModEvent me=y.gq(home,static_cast<int>(rmin));
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }
      {
        ModEvent me=y.lq(home,static_cast<int>(rmax));
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }
      {
        ModEvent me=m.gq(home,y.min()+1);
        if (me_failed(me)) return ES_FAILED;
        modified |= me_modified(me);
      }

      if (prefix == NULL) {
        prefix=region.alloc<ProductInterval>(x.size()+1);
        suffix=region.alloc<ProductInterval>(x.size()+1);
      }
      if (!product_mod_intervals(x,prefix,suffix))
        continue;

      const long long int tmin=k*m.min()+y.min();
      const long long int tmax=k*m.max()+y.max();
      for (int i=0; i<x.size(); i++) {
        ProductInterval q;
        if (!product_mod_interval_mul(prefix[i],suffix[i+1],q))
          continue;
        if (q.max == 0) {
          if (tmin > 0)
            return ES_FAILED;
          continue;
        }
        const long long int candidate_min=ceil_div_xx(tmin,
          static_cast<long long int>(q.max));
        const long long int candidate_max=(q.min == 0) ? x[i].max() :
          floor_div_xx(tmax,static_cast<long long int>(q.min));
        const long long int l=std::max(
          candidate_min,static_cast<long long int>(x[i].min()));
        const long long int u=std::min(
          candidate_max,static_cast<long long int>(x[i].max()));
        if (l > u)
          return ES_FAILED;
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
    } while (modified);
    return ES_OK;
  }

  /// Determine algebraic status; evaluate the residue only when fully assigned.
  inline RelTest
  product_mod_var_status(const ViewArray<IntView>& x, const IntView& m,
                         const IntView& y) {
    if ((m == y) || (m.max() <= 0) || (y.max() < 0) ||
        (y.min() >= m.max()))
      return RT_FALSE;
    if (m.assigned())
      return product_mod_status(x,y,m.val());
    bool zero=product_mod_var_mod_factor(x,m);
    for (int i=0; !zero && (i<x.size()); i++)
      zero=x[i].assigned() && (x[i].val() == 0);
    if (zero) {
      if (!y.in(0)) return RT_FALSE;
      if ((m.min() > 0) && y.assigned()) return RT_TRUE;
      return RT_MAYBE;
    }
    if (x.size() == 0) {
      const bool zero=m.in(1) && y.in(0);
      const bool one=(m.max() >= 2) && y.in(1);
      if (!zero && !one) return RT_FALSE;
      if ((m.min() >= 2) && y.assigned() && (y.val() == 1))
        return RT_TRUE;
      return RT_MAYBE;
    }
    if (y.assigned()) {
      long long int p;
      if (product_mod_var_exact(x,p) &&
          !Limits::overflow_sub(p,static_cast<long long int>(y.val()))) {
        const long long int d=p-y.val();
        if (d == 0)
          return (y.val() >= 0) && (m.min() > y.val())
            ? RT_TRUE : RT_MAYBE;
        int least, greatest;
        if (!product_mod_var_divisor_bounds
            (std::max(m.min(),y.val()+1),m.max(),d,least,greatest))
          return RT_FALSE;
        if ((least == greatest) && !m.in(least))
          return RT_FALSE;
      }
    }
    return RT_MAYBE;
  }

  forceinline
  ProductModVar::ProductModVar(Home home, ViewArray<IntView>& z,
                               IntView modulus, IntView w)
    : Propagator(home), x(z), m(modulus), y(w) {
    home.notice(*this,AP_WEAKLY);
    x.subscribe(home,*this,PC_INT_BND);
    m.subscribe(home,*this,PC_INT_BND);
    y.subscribe(home,*this,PC_INT_BND);
  }

  inline ExecStatus
  ProductModVar::post(Home home, ViewArray<IntView>& x, IntView m, IntView y) {
    if (m == y)
      return ES_FAILED;
    GECODE_ME_CHECK(m.gq(home,1));
    GECODE_ME_CHECK(y.gq(home,0));
    GECODE_ME_CHECK(y.lq(home,m.max()-1));
    GECODE_ME_CHECK(m.gq(home,y.min()+1));
    product_mod_units(x);
    if (product_mod_var_mod_factor(x,m)) {
      GECODE_ME_CHECK(y.eq(home,0));
      return ES_OK;
    }
    if (x.size() == 0) {
      GECODE_ME_CHECK(y.lq(home,1));
      if (!y.in(0)) {
        GECODE_ME_CHECK(m.gq(home,2));
        GECODE_ME_CHECK(y.eq(home,1));
      } else if (!y.in(1)) {
        GECODE_ME_CHECK(m.eq(home,1));
        GECODE_ME_CHECK(y.eq(home,0));
      }
    }
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
    x.reschedule(home,*this,PC_INT_BND);
    m.reschedule(home,*this,PC_INT_BND);
    y.reschedule(home,*this,PC_INT_BND);
  }

  inline ExecStatus
  ProductModVar::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(m.gq(home,1));
    GECODE_ME_CHECK(y.gq(home,0));
    GECODE_ME_CHECK(y.lq(home,m.max()-1));
    GECODE_ME_CHECK(m.gq(home,y.min()+1));
    product_mod_units(x);
    if (product_mod_var_mod_factor(x,m)) {
      GECODE_ME_CHECK(y.eq(home,0));
      return home.ES_SUBSUMED(*this);
    }
    for (int i=0; i<x.size(); i++)
      if (x[i].assigned() && (x[i].val() == 0)) {
        GECODE_ME_CHECK(y.eq(home,0));
        return home.ES_SUBSUMED(*this);
      }
    if (x.size() == 0) {
      GECODE_ME_CHECK(y.lq(home,1));
      if (!y.in(0)) {
        GECODE_ME_CHECK(m.gq(home,2));
        GECODE_ME_CHECK(y.eq(home,1));
        return home.ES_SUBSUMED(*this);
      }
      if (!y.in(1)) {
        GECODE_ME_CHECK(m.eq(home,1));
        GECODE_ME_CHECK(y.eq(home,0));
        return home.ES_SUBSUMED(*this);
      }
      if (m.min() >= 2) {
        GECODE_ME_CHECK(y.eq(home,1));
        return home.ES_SUBSUMED(*this);
      }
      if (!m.in(1)) {
        GECODE_ME_CHECK(y.eq(home,1));
        return home.ES_SUBSUMED(*this);
      }
    }
    if (m.assigned())
      GECODE_REWRITE(*this,ProductMod::post(home(*this),x,m.val(),y));

    GECODE_ES_CHECK(product_mod_var_ranges(home,x,m,y));
    if (m.assigned())
      GECODE_REWRITE(*this,ProductMod::post(home(*this),x,m.val(),y));

    // With an assigned product and result, m must divide product-result.
    const unsigned int modulus_size=m.size();
    long long int p;
    if (y.assigned() && product_mod_var_exact(x,p)) {
      if (!Limits::overflow_sub(p,static_cast<long long int>(y.val()))) {
        const long long int d=p-y.val();
        if (d == 0)
          return home.ES_SUBSUMED(*this);
        int least, greatest;
        if (!product_mod_var_divisor_bounds
            (m.min(),m.max(),d,least,greatest))
          return ES_FAILED;
        GECODE_ME_CHECK(m.gq(home,least));
        GECODE_ME_CHECK(m.lq(home,greatest));
      }
    }
    if (m.assigned())
      GECODE_REWRITE(*this,ProductMod::post(home(*this),x,m.val(),y));
    // A sparse-domain update can expose a new fixed quotient band.
    return (modulus_size != m.size()) ? ES_NOFIX : ES_FIX;
  }

  forceinline size_t
  ProductModVar::dispose(Space& home) {
    x.cancel(home,*this,PC_INT_BND);
    m.cancel(home,*this,PC_INT_BND);
    y.cancel(home,*this,PC_INT_BND);
    home.ignore(*this,AP_WEAKLY);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<ReifyMode rm>
  forceinline
  ReProductModVar<rm>::ReProductModVar(Home home, ViewArray<IntView>& z,
                                       IntView modulus, IntView w, BoolView c)
    : Propagator(home), x(z), m(modulus), y(w), b(c) {
    home.notice(*this,AP_WEAKLY);
    x.subscribe(home,*this,PC_INT_BND);
    m.subscribe(home,*this,PC_INT_BND);
    y.subscribe(home,*this,PC_INT_BND);
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
    x.reschedule(home,*this,PC_INT_BND);
    m.reschedule(home,*this,PC_INT_BND);
    y.reschedule(home,*this,PC_INT_BND);
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
      const RelTest rt=product_mod_var_status(x,m,y);
      if (rt == RT_TRUE)
        return ES_FAILED;
      if (rt == RT_FALSE)
        return home.ES_SUBSUMED(*this);
      return ES_FIX;
    }

    const RelTest rt=product_mod_var_status(x,m,y);
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
    x.cancel(home,*this,PC_INT_BND);
    m.cancel(home,*this,PC_INT_BND);
    y.cancel(home,*this,PC_INT_BND);
    b.cancel(home,*this,PC_BOOL_VAL);
    home.ignore(*this,AP_WEAKLY);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<ReifyMode rm>
  forceinline
  ReProductMod<rm>::ReProductMod(Home home, ViewArray<IntView>& z,
                                 int modulus, IntView w, BoolView c)
    : Propagator(home), x(z), y(w), b(c), m(modulus) {
    home.notice(*this,AP_WEAKLY);
    x.subscribe(home,*this,PC_INT_BND);
    y.subscribe(home,*this,PC_INT_BND);
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
    if (x.size() == 0)
      return Rel::ReEqDomInt<IntView,BoolView,rm>::post(home,y,1 % m,b);
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
    x.reschedule(home,*this,PC_INT_BND);
    y.reschedule(home,*this,PC_INT_BND);
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
    x.cancel(home,*this,PC_INT_BND);
    y.cancel(home,*this,PC_INT_BND);
    b.cancel(home,*this,PC_BOOL_VAL);
    home.ignore(*this,AP_WEAKLY);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}}
