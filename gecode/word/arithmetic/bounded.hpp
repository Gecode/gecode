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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_HPP

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  struct BoundLocalDomain {
    unsigned int width;
    WordDomainType kind;
    WordValue lo;
    WordValue hi;
    WordValue minimum;
    WordValue maximum;
    /// Whether cube/interval synchronization is batched by the current phase
    bool deferred;

    bool synchronize(void) {
      return synchronize_domain(width,kind,lo,hi,minimum,maximum);
    }
    bool range(WordValue min, WordValue max) {
      minimum=std::max(minimum,min);
      maximum=std::min(maximum,max);
      return (minimum <= maximum) && (deferred || synchronize());
    }
  };

  forceinline bool
  operator ==(const BoundLocalDomain& x, const BoundLocalDomain& y) {
    return (x.lo == y.lo) && (x.hi == y.hi) &&
      (x.minimum == y.minimum) && (x.maximum == y.maximum);
  }

  class BoundLocalView {
  private:
    BoundLocalDomain* d;
  public:
    BoundLocalView(void) : d(nullptr) {}
    explicit BoundLocalView(BoundLocalDomain& d0) : d(&d0) {}
    unsigned int width(void) const { return d->width; }
    WordValue mask(void) const { return width_mask(d->width); }
    WordValue lo(void) const { return d->lo; }
    WordValue hi(void) const { return d->hi; }
    WordValue unknown(void) const { return d->hi & ~d->lo; }
    bool assigned(void) const {
      return (d->lo == d->hi) && (d->minimum == d->maximum);
    }
    WordValue val(void) const { assert(assigned()); return d->lo; }
    ModEvent narrow(Space&, WordValue lo, WordValue hi) {
      d->lo |= lo;
      d->hi &= hi;
      if ((d->lo & ~d->hi) != 0 ||
          (!d->deferred && !d->synchronize()))
        return ME_WORD_FAILED;
      return assigned() ? ME_WORD_VAL : ME_WORD_DOM;
    }
    bool operator ==(const BoundLocalView& y) const { return d == y.d; }
    bool operator !=(const BoundLocalView& y) const { return d != y.d; }
  };

  forceinline bool
  bound_signed_add(WordValue x, WordValue y, WordValue sign,
                   WordValue mask, WordValue& result);
  forceinline bool
  bound_signed_sub(WordValue x, WordValue y, WordValue sign,
                   WordValue mask, WordValue& result);

  template<class View>
  forceinline BoundLocalDomain
  bound_snapshot(View x) {
    static_assert(View::supports_bounds,
                  "bounded arithmetic requires a bounded Word view");
    return BoundLocalDomain{x.width(),x.domain_type(),x.lo(),x.hi(),
                            x.rank_minimum(),x.rank_maximum(),false};
  }

  template<class View>
  forceinline ExecStatus
  bound_publish(Home home, View x, const BoundLocalDomain& d) {
    return me_failed(x.narrow_domain(home,d.lo,d.hi,
                                     d.minimum,d.maximum)) ?
      ES_FAILED : ES_OK;
  }

  template<class View>
  forceinline void
  bound_alias_domains(View x0, View x1, View x2,
                      BoundLocalDomain (&d)[3], BoundLocalView (&v)[3]) {
    d[0]=bound_snapshot(x0);
    d[1]=bound_snapshot(x1);
    d[2]=bound_snapshot(x2);
    v[0]=BoundLocalView(d[0]);
    v[1]=BoundLocalView((x1.varimp() == x0.varimp()) ? d[0] : d[1]);
    if (x2.varimp() == x0.varimp())
      v[2]=BoundLocalView(d[0]);
    else if (x2.varimp() == x1.varimp())
      v[2]=v[1];
    else
      v[2]=BoundLocalView(d[2]);
  }

  template<class View>
  forceinline ExecStatus
  bound_publish_distinct(Home home, View x0, View x1, View x2,
                         BoundLocalDomain (&d)[3]) {
    GECODE_ES_CHECK(bound_publish(home,x0,d[0]));
    if (x1.varimp() != x0.varimp())
      GECODE_ES_CHECK(bound_publish(home,x1,d[1]));
    if ((x2.varimp() != x0.varimp()) &&
        (x2.varimp() != x1.varimp()))
      GECODE_ES_CHECK(bound_publish(home,x2,d[2]));
    return ES_OK;
  }

  template<class View>
  forceinline bool
  bound_nary_add_ranges(BoundLocalDomain** input, int n,
                        BoundLocalDomain& result, WordValue constant) {
    Region region;
    WordValue* prefix_min=region.alloc<WordValue>(n+1);
    WordValue* prefix_max=region.alloc<WordValue>(n+1);
    WordValue* suffix_min=region.alloc<WordValue>(n+1);
    WordValue* suffix_max=region.alloc<WordValue>(n+1);
    const WordValue mask=width_mask(result.width);
    const WordValue identity=View::signed_order ? sign_bit(result.width) : 0;
    prefix_min[0]=prefix_max[0]=
      Word::rank(result.kind,result.width,constant);
    suffix_min[n]=suffix_max[n]=identity;
    for (int i=0; i<n; i++) {
      if (!View::signed_order) {
        if ((input[i]->minimum > mask-prefix_min[i]) ||
            (input[i]->maximum > mask-prefix_max[i]))
          return false;
        prefix_min[i+1]=prefix_min[i]+input[i]->minimum;
        prefix_max[i+1]=prefix_max[i]+input[i]->maximum;
      } else {
        const WordValue sign=sign_bit(result.width);
        if (!bound_signed_add(prefix_min[i],input[i]->minimum,sign,mask,
                              prefix_min[i+1]) ||
            !bound_signed_add(prefix_max[i],input[i]->maximum,sign,mask,
                              prefix_max[i+1]))
          return false;
      }
    }
    if (!result.range(prefix_min[n],prefix_max[n]))
      return false;
    for (int i=n; i-- > 0;) {
      if (!View::signed_order) {
        if ((input[i]->minimum > mask-suffix_min[i+1]) ||
            (input[i]->maximum > mask-suffix_max[i+1]))
          return false;
        suffix_min[i]=input[i]->minimum+suffix_min[i+1];
        suffix_max[i]=input[i]->maximum+suffix_max[i+1];
      } else {
        const WordValue sign=sign_bit(result.width);
        if (!bound_signed_add(input[i]->minimum,suffix_min[i+1],sign,mask,
                              suffix_min[i]) ||
            !bound_signed_add(input[i]->maximum,suffix_max[i+1],sign,mask,
                              suffix_max[i]))
          return false;
      }
    }
    for (int i=0; i<n; i++) {
      WordValue other_min, other_max, minimum, maximum;
      if (!View::signed_order) {
        if ((suffix_min[i+1] > mask-prefix_min[i]) ||
            (suffix_max[i+1] > mask-prefix_max[i]))
          return false;
        other_min=prefix_min[i]+suffix_min[i+1];
        other_max=prefix_max[i]+suffix_max[i+1];
        if (result.maximum < other_min)
          return false;
        minimum=(result.minimum >= other_max) ?
          result.minimum-other_max : 0;
        maximum=result.maximum-other_min;
      } else {
        const WordValue sign=sign_bit(result.width);
        if (!bound_signed_add(prefix_min[i],suffix_min[i+1],sign,mask,
                              other_min) ||
            !bound_signed_add(prefix_max[i],suffix_max[i+1],sign,mask,
                              other_max))
          return false;
        if (!bound_signed_sub(result.minimum,other_max,sign,mask,minimum))
          minimum=0;
        if (!bound_signed_sub(result.maximum,other_min,sign,mask,maximum))
          maximum=mask;
      }
      if (!input[i]->range(std::max(input[i]->minimum,minimum),
                           std::min(input[i]->maximum,maximum)))
        return false;
    }
    return true;
  }

  forceinline bool
  bound_signed_add(WordValue x, WordValue y, WordValue sign,
                   WordValue mask, WordValue& result) {
    if (y >= sign) {
      const WordValue delta=y-sign;
      if (x > mask-delta) return false;
      result=x+delta;
    } else {
      const WordValue delta=sign-y;
      if (x < delta) return false;
      result=x-delta;
    }
    return true;
  }

  forceinline bool
  bound_signed_sub(WordValue x, WordValue y, WordValue sign,
                   WordValue mask, WordValue& result) {
    if (y >= sign) {
      const WordValue delta=y-sign;
      if (x < delta) return false;
      result=x-delta;
    } else {
      const WordValue delta=sign-y;
      if (x > mask-delta) return false;
      result=x+delta;
    }
    return true;
  }

  template<class View>
  forceinline bool
  bound_add_ranges(BoundLocalDomain& x, BoundLocalDomain& y,
                   BoundLocalDomain& z) {
    const WordValue mask=width_mask(x.width);
    WordValue zmin, zmax, xmin, xmax, ymin, ymax;
    if (!View::signed_order) {
      if (x.maximum > mask-y.maximum)
        return true;
      zmin=x.minimum+y.minimum; zmax=x.maximum+y.maximum;
    } else {
      const WordValue sign=sign_bit(x.width);
      if (!bound_signed_add(x.minimum,y.minimum,sign,mask,zmin) ||
          !bound_signed_add(x.maximum,y.maximum,sign,mask,zmax))
        return true;
    }
    if (!z.range(zmin,zmax)) return false;
    const BoundLocalDomain old_x=x, old_y=y;
    if (!View::signed_order) {
      xmin=(z.minimum >= old_y.maximum) ? z.minimum-old_y.maximum : 0;
      xmax=z.maximum-old_y.minimum;
      ymin=(z.minimum >= old_x.maximum) ? z.minimum-old_x.maximum : 0;
      ymax=z.maximum-old_x.minimum;
    } else {
      const WordValue sign=sign_bit(x.width);
      if (!bound_signed_sub(z.minimum,old_y.maximum,sign,mask,xmin)) xmin=0;
      if (!bound_signed_sub(z.maximum,old_y.minimum,sign,mask,xmax)) xmax=mask;
      if (!bound_signed_sub(z.minimum,old_x.maximum,sign,mask,ymin)) ymin=0;
      if (!bound_signed_sub(z.maximum,old_x.minimum,sign,mask,ymax)) ymax=mask;
    }
    return x.range(std::max(old_x.minimum,xmin),
              std::min(old_x.maximum,xmax)) &&
      y.range(std::max(old_y.minimum,ymin),
              std::min(old_y.maximum,ymax));
  }

  template<class View>
  forceinline bool
  bound_sub_ranges(BoundLocalDomain& x, BoundLocalDomain& y,
                   BoundLocalDomain& z) {
    const WordValue mask=width_mask(x.width);
    WordValue zmin, zmax, xmin, xmax, ymin, ymax;
    if (!View::signed_order) {
      if (x.minimum < y.maximum)
        return true;
      zmin=x.minimum-y.maximum; zmax=x.maximum-y.minimum;
    } else {
      const WordValue sign=sign_bit(x.width);
      if (!bound_signed_sub(x.minimum,y.maximum,sign,mask,zmin) ||
          !bound_signed_sub(x.maximum,y.minimum,sign,mask,zmax))
        return true;
    }
    if (!z.range(zmin,zmax)) return false;
    const BoundLocalDomain old_x=x, old_y=y;
    if (!View::signed_order) {
      xmin=z.minimum+old_y.minimum;
      xmax=(z.maximum > mask-old_y.maximum) ?
        mask : z.maximum+old_y.maximum;
      ymin=(old_x.minimum >= z.maximum) ? old_x.minimum-z.maximum : 0;
      ymax=old_x.maximum-z.minimum;
    } else {
      const WordValue sign=sign_bit(x.width);
      if (!bound_signed_add(z.minimum,old_y.minimum,sign,mask,xmin)) xmin=0;
      if (!bound_signed_add(z.maximum,old_y.maximum,sign,mask,xmax)) xmax=mask;
      if (!bound_signed_sub(old_x.minimum,z.maximum,sign,mask,ymin)) ymin=0;
      if (!bound_signed_sub(old_x.maximum,z.minimum,sign,mask,ymax)) ymax=mask;
    }
    return x.range(std::max(old_x.minimum,xmin),
              std::min(old_x.maximum,xmax)) &&
      y.range(std::max(old_y.minimum,ymin),
              std::min(old_y.maximum,ymax));
  }

  forceinline bool
  bound_mult_unsigned(BoundLocalDomain& x, BoundLocalDomain& y,
                      BoundLocalDomain& z) {
    const WordValue mask=width_mask(x.width);
    if ((x.maximum != 0) && (y.maximum > mask/x.maximum))
      return true;
    const WordValue zmin=x.minimum*y.minimum;
    const WordValue zmax=x.maximum*y.maximum;
    if (!z.range(zmin,zmax)) return false;
    WordValue xmin=x.minimum, xmax=x.maximum;
    WordValue ymin=y.minimum, ymax=y.maximum;
    if (y.maximum != 0)
      xmin=std::max(xmin,z.minimum/y.maximum+
                    ((z.minimum%y.maximum) != 0));
    if (y.minimum != 0) xmax=std::min(xmax,z.maximum/y.minimum);
    if (x.maximum != 0)
      ymin=std::max(ymin,z.minimum/x.maximum+
                    ((z.minimum%x.maximum) != 0));
    if (x.minimum != 0) ymax=std::min(ymax,z.maximum/x.minimum);
    return x.range(xmin,xmax) && y.range(ymin,ymax);
  }

  forceinline bool
  bound_signed_product(WordValue x, WordValue y, WordValue sign,
                       WordValue mask, WordValue& result) {
    const bool xn=x < sign, yn=y < sign;
    const WordValue xm=xn ? sign-x : x-sign;
    const WordValue ym=yn ? sign-y : y-sign;
    const bool negative=xn != yn;
    const WordValue limit=negative ? sign : sign-1;
    if ((xm != 0) && (ym > limit/xm)) return false;
    const WordValue magnitude=xm*ym;
    result=negative ? sign-magnitude : sign+magnitude;
    return result <= mask;
  }

  forceinline bool
  bound_mult_signed(BoundLocalDomain& x, BoundLocalDomain& y,
                    BoundLocalDomain& z) {
    const WordValue mask=width_mask(x.width), sign=sign_bit(x.width);
    const WordValue xs[2]={x.minimum,x.maximum};
    const WordValue ys[2]={y.minimum,y.maximum};
    WordValue minimum=mask, maximum=0;
    for (unsigned int i=0; i<2; i++)
      for (unsigned int j=0; j<2; j++) {
        WordValue value;
        if (!bound_signed_product(xs[i],ys[j],sign,mask,value))
          return true;
        minimum=std::min(minimum,value);
        maximum=std::max(maximum,value);
      }
    return z.range(minimum,maximum);
  }

  template<class View>
  forceinline bool
  bound_mult_ranges(BoundLocalDomain& x, BoundLocalDomain& y,
                    BoundLocalDomain& z) {
    return View::signed_order ? bound_mult_signed(x,y,z) :
      bound_mult_unsigned(x,y,z);
  }

  template<class View>
  forceinline bool
  bound_neg_ranges(BoundLocalDomain& x, BoundLocalDomain& z) {
    if (!View::signed_order)
      return true;
    if (x.minimum == 0)
      return true;
    const WordValue mask=width_mask(x.width);
    const WordValue zmin=mask-x.maximum+1;
    const WordValue zmax=mask-x.minimum+1;
    if (!z.range(zmin,zmax)) return false;
    const WordValue xmin=mask-z.maximum+1;
    const WordValue xmax=mask-z.minimum+1;
    return x.range(xmin,xmax);
  }

  enum BoundArithmeticOperation {
    BA_ADD,
    BA_SUB,
    BA_MULT
  };

  enum BoundTerminal {
    BT_ANY=3U,
    BT_CLEAR=1U,
    BT_SET=2U
  };

  forceinline WordValue
  bound_terminal_add_sat(WordValue x, WordValue y, WordValue mask) {
    return (x > mask-y) ? mask : x+y;
  }

  forceinline bool
  bound_add_clear(BoundLocalDomain& x, BoundLocalDomain& y,
                  BoundLocalDomain& z, WordValue mask) {
    if (!x.range(x.minimum,std::min(x.maximum,mask-y.minimum)) ||
        !y.range(y.minimum,std::min(y.maximum,mask-x.minimum)))
      return false;
    const BoundLocalDomain old_x=x, old_y=y;
    if (!z.range(old_x.minimum+old_y.minimum,
                 bound_terminal_add_sat(old_x.maximum,old_y.maximum,mask)))
      return false;
    if ((z.maximum < old_y.minimum) || (z.maximum < old_x.minimum))
      return false;
    const WordValue xmin=(z.minimum > old_y.maximum) ?
      z.minimum-old_y.maximum : 0U;
    const WordValue ymin=(z.minimum > old_x.maximum) ?
      z.minimum-old_x.maximum : 0U;
    return x.range(std::max(old_x.minimum,xmin),
                   std::min(old_x.maximum,z.maximum-old_y.minimum)) &&
      y.range(std::max(old_y.minimum,ymin),
              std::min(old_y.maximum,z.maximum-old_x.minimum));
  }

  forceinline bool
  bound_add_set(BoundLocalDomain& x, BoundLocalDomain& y,
                BoundLocalDomain& z, WordValue mask) {
    if ((x.maximum <= mask-y.maximum) ||
        !x.range(std::max(x.minimum,mask-y.maximum+1U),x.maximum) ||
        !y.range(std::max(y.minimum,mask-x.maximum+1U),y.maximum))
      return false;
    const BoundLocalDomain old_x=x, old_y=y;
    const WordValue zmin=(old_x.minimum > mask-old_y.minimum) ?
      old_x.minimum-(mask-old_y.minimum)-1U : 0U;
    const WordValue zmax=old_x.maximum-(mask-old_y.maximum)-1U;
    if (!z.range(zmin,zmax)) return false;
    const WordValue xbase=mask-old_y.maximum+1U;
    const WordValue ybase=mask-old_x.maximum+1U;
    if ((z.minimum > mask-xbase) || (z.minimum > mask-ybase))
      return false;
    const WordValue xmin=xbase+z.minimum;
    const WordValue ymin=ybase+z.minimum;
    const WordValue xmax=bound_terminal_add_sat(
      mask-old_y.minimum+1U,z.maximum,mask);
    const WordValue ymax=bound_terminal_add_sat(
      mask-old_x.minimum+1U,z.maximum,mask);
    return x.range(std::max(old_x.minimum,xmin),
                   std::min(old_x.maximum,xmax)) &&
      y.range(std::max(old_y.minimum,ymin),
              std::min(old_y.maximum,ymax));
  }

  forceinline bool
  bound_sub_clear(BoundLocalDomain& x, BoundLocalDomain& y,
                  BoundLocalDomain& z, WordValue mask) {
    if (!x.range(std::max(x.minimum,y.minimum),x.maximum) ||
        !y.range(y.minimum,std::min(y.maximum,x.maximum)))
      return false;
    const BoundLocalDomain old_x=x, old_y=y;
    const WordValue zmin=(old_x.minimum > old_y.maximum) ?
      old_x.minimum-old_y.maximum : 0U;
    if (!z.range(zmin,old_x.maximum-old_y.minimum)) return false;
    const WordValue xmin=bound_terminal_add_sat(
      z.minimum,old_y.minimum,mask);
    const WordValue xmax=bound_terminal_add_sat(
      z.maximum,old_y.maximum,mask);
    const WordValue ymin=(old_x.minimum > z.maximum) ?
      old_x.minimum-z.maximum : 0U;
    return x.range(std::max(old_x.minimum,xmin),
                   std::min(old_x.maximum,xmax)) &&
      y.range(std::max(old_y.minimum,ymin),
              std::min(old_y.maximum,old_x.maximum-z.minimum));
  }

  forceinline bool
  bound_sub_set(BoundLocalDomain& x, BoundLocalDomain& y,
                BoundLocalDomain& z, WordValue mask) {
    if ((x.minimum >= y.maximum) ||
        !x.range(x.minimum,std::min(x.maximum,y.maximum-1U)) ||
        !y.range(std::max(y.minimum,x.minimum+1U),y.maximum))
      return false;
    const BoundLocalDomain old_x=x, old_y=y;
    const WordValue zmin=mask-(old_y.maximum-old_x.minimum)+1U;
    const WordValue zmax=(old_x.maximum < old_y.minimum) ?
      mask-(old_y.minimum-old_x.maximum)+1U : mask;
    if (!z.range(zmin,zmax)) return false;
    const WordValue gap_min=mask-z.maximum+1U;
    const WordValue gap_max=mask-z.minimum+1U;
    if ((old_y.maximum < gap_min) ||
        (old_x.minimum > mask-gap_min)) return false;
    const WordValue xmin=(old_y.minimum > gap_max) ?
      old_y.minimum-gap_max : 0U;
    const WordValue xmax=old_y.maximum-gap_min;
    const WordValue ymin=old_x.minimum+gap_min;
    const WordValue ymax=bound_terminal_add_sat(
      old_x.maximum,gap_max,mask);
    return x.range(std::max(old_x.minimum,xmin),
                   std::min(old_x.maximum,xmax)) &&
      y.range(std::max(old_y.minimum,ymin),
              std::min(old_y.maximum,ymax));
  }

  template<class View, BoundArithmeticOperation op>
  forceinline bool
  bound_terminal_ranges(BoundLocalDomain* (&role)[3],
                        unsigned int terminal) {
    const WordValue mask=width_mask(role[0]->width);
    if (op == BA_ADD)
      return (terminal == BT_CLEAR) ?
        bound_add_clear(*role[0],*role[1],*role[2],mask) :
        bound_add_set(*role[0],*role[1],*role[2],mask);
    return (terminal == BT_CLEAR) ?
      bound_sub_clear(*role[0],*role[1],*role[2],mask) :
      bound_sub_set(*role[0],*role[1],*role[2],mask);
  }

  template<class View>
  forceinline
  BoundNaryAdd<View>::BoundNaryAdd(Home home, ViewArray<View>& x0, View y0,
                                   WordValue c, bool a)
    : MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_DOM>(home,x0,y0),
      constant(c), aliased(a) {}

  template<class View>
  forceinline
  BoundNaryAdd<View>::BoundNaryAdd(Space& home, BoundNaryAdd& p)
    : MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_DOM>(home,p),
      constant(p.constant), aliased(p.aliased) {}

  template<class View>
  Actor*
  BoundNaryAdd<View>::copy(Space& home) {
    return new (home) BoundNaryAdd(home,*this);
  }

  template<class View>
  PropCost
  BoundNaryAdd<View>::cost(const Space&, const ModEventDelta& med) const {
    if (View::me(med) == ME_WORD_BND)
      return aliased ? PropCost::quadratic(PropCost::LO,x.size()+1) :
        PropCost::linear(PropCost::LO,static_cast<unsigned int>(x.size()+1));
    return PropCost::linear(PropCost::HI,
                            static_cast<unsigned int>(x.size())*y.width());
  }

  template<class View>
  ExecStatus
  BoundNaryAdd<View>::narrow(Home home, ViewArray<View>& input, View result,
                             WordValue c, bool cube) {
    Region region;
    const int n=input.size();
    BoundLocalDomain* d=region.alloc<BoundLocalDomain>(n+1);
    int* representative=region.alloc<int>(n+1);
    ViewArray<BoundLocalView> local(region,n);
    for (int i=0; i<n+1; i++) {
      View current=(i<n) ? input[i] : result;
      representative[i]=i;
      for (int j=0; j<i; j++) {
        View previous=(j<n) ? input[j] : result;
        if (current.varimp() == previous.varimp()) {
          representative[i]=representative[j];
          break;
        }
      }
      if (representative[i] == i)
        d[i]=bound_snapshot(current);
      if (i<n)
        local[i]=BoundLocalView(d[representative[i]]);
    }
    BoundLocalView local_result(d[representative[n]]);
    BoundLocalDomain** roles=region.alloc<BoundLocalDomain*>(n);
    BoundLocalDomain* old=region.alloc<BoundLocalDomain>(n+1);
    for (int i=0; i<n; i++)
      roles[i]=&d[representative[i]];

    for (;;) {
      for (int i=0; i<n+1; i++)
        if (representative[i] == i) {
          old[i]=d[i];
          d[i].deferred=true;
        }
      if (cube)
        GECODE_ES_CHECK(NaryAdd::narrow(
          home,local,local_result,c,true));
      if (!bound_nary_add_ranges<View>(roles,n,d[representative[n]],c))
        return ES_FAILED;
      for (int i=0; i<n+1; i++)
        if (representative[i] == i) {
          d[i].deferred=false;
          if (!d[i].synchronize()) return ES_FAILED;
        }
      bool changed=false;
      for (int i=0; i<n+1; i++)
        if ((representative[i] == i) && !(d[i] == old[i]))
          changed=true;
      cube=false;
      if (!changed) break;
    }
    for (int i=0; i<n; i++)
      if (representative[i] == i)
        GECODE_ES_CHECK(bound_publish(home,input[i],d[i]));
    if (representative[n] == n)
      GECODE_ES_CHECK(bound_publish(home,result,d[n]));
    return ES_OK;
  }

  template<class View>
  ExecStatus
  BoundNaryAdd<View>::narrow_bounds(Home home, ViewArray<View>& input,
                                    View result, WordValue c, bool& bits) {
    WordValue before_lo=result.lo(), before_hi=result.hi();
    Region region;
    WordValue* lo=region.alloc<WordValue>(input.size());
    WordValue* hi=region.alloc<WordValue>(input.size());
    for (int i=0; i<input.size(); i++) {
      lo[i]=input[i].lo(); hi[i]=input[i].hi();
    }
    GECODE_ES_CHECK(narrow(home,input,result,c,false));
    bits=(before_lo != result.lo()) || (before_hi != result.hi());
    for (int i=0; i<input.size(); i++)
      bits |= (lo[i] != input[i].lo()) || (hi[i] != input[i].hi());
    return ES_OK;
  }

  template<class View>
  ExecStatus
  BoundNaryAdd<View>::propagate(Space& home, const ModEventDelta& med) {
    if (View::me(med) == ME_WORD_BND) {
      bool bits;
      GECODE_ES_CHECK(narrow_bounds(home,x,y,constant,bits));
      bool assigned=y.assigned();
      for (int i=0; i<x.size(); i++) assigned &= x[i].assigned();
      if (assigned) return home.ES_SUBSUMED(*this);
      return bits ? home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS)) :
        ES_FIX;
    }
    GECODE_ES_CHECK(narrow(home,x,y,constant,true));
    bool assigned=y.assigned();
    for (int i=0; i<x.size(); i++) assigned &= x[i].assigned();
    return assigned ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View>
  ExecStatus
  BoundNaryAdd<View>::post(Home home, ViewArray<View>& input, View result,
                           WordValue c) {
    const bool a=shared(input) || shared(input,result);
    GECODE_ES_CHECK(narrow(home,input,result,c,true));
    bool assigned=result.assigned();
    for (int i=0; i<input.size(); i++) assigned &= input[i].assigned();
    if (!assigned)
      (void) new (home) BoundNaryAdd(home,input,result,c,a);
    return ES_OK;
  }

  /*
   * Bounded actors with an O(width) cube algorithm use two propagation
   * stages. A bound-only event first runs the constant-cost numeric rules and
   * synchronizes each local role once. If that synchronization fixes cube
   * bits, ES_NOFIX_PARTIAL schedules the cube algorithm separately at its
   * honest linear cost. Combined/bit events run both stages locally.
   */
  template<class View, BoundArithmeticOperation op,
           BoundTerminal terminal=BT_ANY>
  class BoundArithmetic : public TernaryPropagator<View,PC_WORD_DOM> {
  protected:
    using TernaryPropagator<View,PC_WORD_DOM>::x0;
    using TernaryPropagator<View,PC_WORD_DOM>::x1;
    using TernaryPropagator<View,PC_WORD_DOM>::x2;
    BoundArithmetic(Home home, View x, View y, View z)
      : TernaryPropagator<View,PC_WORD_DOM>(home,x,y,z) {}
    BoundArithmetic(Space& home, BoundArithmetic& p)
      : TernaryPropagator<View,PC_WORD_DOM>(home,p) {}
    static ExecStatus narrow_bounds(Home home, View x, View y, View z,
                                    bool& bits) {
      BoundLocalDomain d[3]; BoundLocalView v[3];
      bound_alias_domains(x,y,z,d,v);
      const WordValue initial_lo[3]={d[0].lo,d[1].lo,d[2].lo};
      const WordValue initial_hi[3]={d[0].hi,d[1].hi,d[2].hi};
      for (;;) {
        const BoundLocalDomain old[3]={d[0],d[1],d[2]};
        for (unsigned int i=0; i<3; i++) d[i].deferred=true;
        BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
        if (x.varimp() == y.varimp()) role[1]=role[0];
        if (x.varimp() == z.varimp()) role[2]=role[0];
        else if (y.varimp() == z.varimp()) role[2]=role[1];
        const bool ok = (terminal != BT_ANY) ?
          bound_terminal_ranges<View,op>(role,terminal) :
          op == BA_ADD ?
          bound_add_ranges<View>(*role[0],*role[1],*role[2]) :
          op == BA_SUB ?
          bound_sub_ranges<View>(*role[0],*role[1],*role[2]) :
          bound_mult_ranges<View>(*role[0],*role[1],*role[2]);
        if (!ok) return ES_FAILED;
        for (unsigned int i=0; i<3; i++) {
          d[i].deferred=false;
          if (!d[i].synchronize()) return ES_FAILED;
        }
        if ((d[0] == old[0]) && (d[1] == old[1]) && (d[2] == old[2]))
          break;
      }
      bits=false;
      for (unsigned int i=0; i<3; i++)
        bits |= (d[i].lo != initial_lo[i]) || (d[i].hi != initial_hi[i]);
      return bound_publish_distinct(home,x,y,z,d);
    }
    static ExecStatus narrow(Home home, View x, View y, View z,
                             bool cube) {
      BoundLocalDomain d[3]; BoundLocalView v[3];
      bound_alias_domains(x,y,z,d,v);
      for (;;) {
        const BoundLocalDomain old[3]={d[0],d[1],d[2]};
        for (unsigned int i=0; i<3; i++) d[i].deferred=true;
        if (cube && (op == BA_ADD)) {
          unsigned int final;
          GECODE_ES_CHECK(add_narrow(home,v[0],v[1],v[2],terminal,final));
        }
        if (cube && (op == BA_SUB)) {
          unsigned int final;
          GECODE_ES_CHECK(sub_narrow(home,v[0],v[1],v[2],terminal,final));
        }
        if (cube && (op == BA_MULT))
          GECODE_ES_CHECK(mult_narrow_views(home,v[0],v[1],v[2]));
        // Roles that alias share a local record; use the role mapping below.
        BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
        if (x.varimp() == y.varimp()) role[1]=role[0];
        if (x.varimp() == z.varimp()) role[2]=role[0];
        else if (y.varimp() == z.varimp()) role[2]=role[1];
        const WordValue before_lo[3]={d[0].lo,d[1].lo,d[2].lo};
        const WordValue before_hi[3]={d[0].hi,d[1].hi,d[2].hi};
        bool ok = (terminal != BT_ANY) ?
          bound_terminal_ranges<View,op>(role,terminal) :
          op == BA_ADD ? bound_add_ranges<View>(*role[0],*role[1],*role[2]) :
          op == BA_SUB ? bound_sub_ranges<View>(*role[0],*role[1],*role[2]) :
          bound_mult_ranges<View>(*role[0],*role[1],*role[2]);
        if (!ok) return ES_FAILED;
        for (unsigned int i=0; i<3; i++) {
          d[i].deferred=false;
          if (!d[i].synchronize()) return ES_FAILED;
        }
        cube=false;
        for (unsigned int i=0; i<3; i++)
          cube |= (d[i].lo != before_lo[i]) ||
            (d[i].hi != before_hi[i]);
        if ((d[0] == old[0]) && (d[1] == old[1]) && (d[2] == old[2]))
          break;
      }
      return bound_publish_distinct(home,x,y,z,d);
    }
  public:
    static bool numeric_regime(View x, View y) {
      static_assert(View::supports_bounds,
                    "bounded arithmetic requires a bounded Word view");
      static_assert((terminal == BT_ANY) || !View::signed_order,
                    "terminal carry/borrow is an unsigned Word property");
      static_assert((terminal == BT_ANY) || (op != BA_MULT),
                    "multiplication has no terminal carry actor");
      if (terminal != BT_ANY)
        return true;
      const WordValue mask=width_mask(x.width());
      if (op == BA_ADD) {
        if (!View::signed_order)
          return x.rank_maximum() <= mask-y.rank_maximum();
        const WordValue sign=sign_bit(x.width()); WordValue ignored;
        return bound_signed_add(x.rank_minimum(),y.rank_minimum(),sign,mask,
                                ignored) &&
          bound_signed_add(x.rank_maximum(),y.rank_maximum(),sign,mask,
                           ignored);
      }
      if (op == BA_SUB) {
        if (!View::signed_order)
          return x.rank_minimum() >= y.rank_maximum();
        const WordValue sign=sign_bit(x.width()); WordValue ignored;
        return bound_signed_sub(x.rank_minimum(),y.rank_maximum(),sign,mask,
                                ignored) &&
          bound_signed_sub(x.rank_maximum(),y.rank_minimum(),sign,mask,
                           ignored);
      }
      if (!View::signed_order)
        return (x.rank_maximum() == 0U) ||
          (y.rank_maximum() <= mask/x.rank_maximum());
      const WordValue sign=sign_bit(x.width()); WordValue ignored;
      return bound_signed_product(x.rank_minimum(),y.rank_minimum(),sign,mask,
                                  ignored) &&
        bound_signed_product(x.rank_minimum(),y.rank_maximum(),sign,mask,
                             ignored) &&
        bound_signed_product(x.rank_maximum(),y.rank_minimum(),sign,mask,
                             ignored) &&
        bound_signed_product(x.rank_maximum(),y.rank_maximum(),sign,mask,
                             ignored);
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundArithmetic(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return (View::me(med) == ME_WORD_BND) ?
        PropCost::ternary(PropCost::LO) :
        PropCost::linear(PropCost::HI,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (View::me(med) == ME_WORD_BND) {
        bool bits;
        GECODE_ES_CHECK(narrow_bounds(home,x0,x1,x2,bits));
        if (x0.assigned() && x1.assigned() && x2.assigned())
          return home.ES_SUBSUMED(*this);
        if (bits)
          return home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS));
        return ES_FIX;
      }
      GECODE_ES_CHECK(narrow(home,x0,x1,x2,true));
      return (x0.assigned() && x1.assigned() && x2.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, View x, View y, View z) {
      GECODE_ES_CHECK(narrow(home,x,y,z,true));
      if (!(x.assigned() && y.assigned() && z.assigned()))
        (void) new (home) BoundArithmetic(home,x,y,z);
      return ES_OK;
    }
  };

  template<class View>
  class BoundNeg : public BinaryPropagator<View,PC_WORD_DOM> {
  protected:
    using BinaryPropagator<View,PC_WORD_DOM>::x0;
    using BinaryPropagator<View,PC_WORD_DOM>::x1;
    BoundNeg(Home home, View x, View z)
      : BinaryPropagator<View,PC_WORD_DOM>(home,x,z) {}
    BoundNeg(Space& home, BoundNeg& p)
      : BinaryPropagator<View,PC_WORD_DOM>(home,p) {}
    static ExecStatus narrow_bounds(Home home, View x, View z, bool& bits) {
      BoundLocalDomain d[2]={bound_snapshot(x),bound_snapshot(z)};
      const bool aliased=x.varimp() == z.varimp();
      const WordValue initial_lo[2]={d[0].lo,d[1].lo};
      const WordValue initial_hi[2]={d[0].hi,d[1].hi};
      for (;;) {
        const BoundLocalDomain old[2]={d[0],d[1]};
        d[0].deferred=d[1].deferred=true;
        BoundLocalDomain& dz=aliased ? d[0] : d[1];
        if (!bound_neg_ranges<View>(d[0],dz)) return ES_FAILED;
        for (unsigned int i=0; i<2; i++) {
          d[i].deferred=false;
          if (!d[i].synchronize()) return ES_FAILED;
        }
        if ((d[0] == old[0]) && (d[1] == old[1])) break;
      }
      bits=(d[0].lo != initial_lo[0]) || (d[0].hi != initial_hi[0]) ||
        (!aliased && ((d[1].lo != initial_lo[1]) ||
                      (d[1].hi != initial_hi[1])));
      GECODE_ES_CHECK(bound_publish(home,x,d[0]));
      if (!aliased) GECODE_ES_CHECK(bound_publish(home,z,d[1]));
      return ES_OK;
    }
    static ExecStatus narrow(Home home, View x, View z, bool cube) {
      BoundLocalDomain d[2]={bound_snapshot(x),bound_snapshot(z)};
      BoundLocalView vx(d[0]), vz((x.varimp() == z.varimp()) ? d[0] : d[1]);
      for (;;) {
        const BoundLocalDomain old[2]={d[0],d[1]};
        d[0].deferred=d[1].deferred=true;
        if (cube) GECODE_ES_CHECK(neg_narrow(home,vx,vz));
        BoundLocalDomain& dz=(x.varimp() == z.varimp()) ? d[0] : d[1];
        const WordValue before_lo[2]={d[0].lo,d[1].lo};
        const WordValue before_hi[2]={d[0].hi,d[1].hi};
        if (!bound_neg_ranges<View>(d[0],dz)) return ES_FAILED;
        for (unsigned int i=0; i<2; i++) {
          d[i].deferred=false;
          if (!d[i].synchronize()) return ES_FAILED;
        }
        cube=false;
        for (unsigned int i=0; i<2; i++)
          cube |= (d[i].lo != before_lo[i]) ||
            (d[i].hi != before_hi[i]);
        if ((d[0] == old[0]) && (d[1] == old[1])) break;
      }
      GECODE_ES_CHECK(bound_publish(home,x,d[0]));
      if (z.varimp() != x.varimp())
        GECODE_ES_CHECK(bound_publish(home,z,d[1]));
      return ES_OK;
    }
  public:
    static bool numeric_regime(View x) { return x.rank_minimum() != 0U; }
    virtual Actor* copy(Space& home) { return new (home) BoundNeg(home,*this); }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return (View::me(med) == ME_WORD_BND) ?
        PropCost::binary(PropCost::LO) :
        PropCost::linear(PropCost::HI,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (View::me(med) == ME_WORD_BND) {
        bool bits;
        GECODE_ES_CHECK(narrow_bounds(home,x0,x1,bits));
        if (x0.assigned() && x1.assigned())
          return home.ES_SUBSUMED(*this);
        if (bits)
          return home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS));
        return ES_FIX;
      }
      GECODE_ES_CHECK(narrow(home,x0,x1,true));
      return (x0.assigned() && x1.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, View x, View z) {
      GECODE_ES_CHECK(narrow(home,x,z,true));
      if (!(x.assigned() && z.assigned()))
        (void) new (home) BoundNeg(home,x,z);
      return ES_OK;
    }
  };

  template<class View, BoundArithmeticOperation op>
  class BoundFlagArithmetic : public Propagator {
  protected:
    View x0;
    View x1;
    View x2;
    Int::BoolView flag;
    BoundFlagArithmetic(Home home, View x, View y, View z, Int::BoolView b)
      : Propagator(home), x0(x), x1(y), x2(z), flag(b) {
      x0.subscribe(home,*this,PC_WORD_DOM);
      x1.subscribe(home,*this,PC_WORD_DOM);
      x2.subscribe(home,*this,PC_WORD_DOM);
      flag.subscribe(home,*this,Int::PC_BOOL_VAL);
    }
    BoundFlagArithmetic(Space& home, BoundFlagArithmetic& p)
      : Propagator(home,p) {
      x0.update(home,p.x0); x1.update(home,p.x1); x2.update(home,p.x2);
      flag.update(home,p.flag);
    }
    static bool narrow_ranges(BoundLocalDomain* (&role)[3],
                              unsigned int& terminal) {
      const WordValue mask=width_mask(role[0]->width);
      const bool all_wrapping=(op == BA_ADD) ?
        role[0]->minimum > mask-role[1]->minimum :
        role[0]->maximum < role[1]->minimum;
      const bool all_nonwrapping=(op == BA_ADD) ?
        role[0]->maximum <= mask-role[1]->maximum :
        role[0]->minimum >= role[1]->maximum;
      if (all_wrapping) terminal &= BT_SET;
      if (all_nonwrapping) terminal &= BT_CLEAR;
      if (terminal == 0U) return false;
      return bound_terminal_ranges<View,op>(role,
        static_cast<BoundTerminal>(terminal));
    }
    static ExecStatus publish(Home home, View x, View y, View z,
                              Int::BoolView flag,
                              BoundLocalDomain (&d)[3],
                              unsigned int terminal) {
      GECODE_ES_CHECK(bound_publish_distinct(home,x,y,z,d));
      if (terminal == 1U) GECODE_ME_CHECK(flag.zero(home));
      else if (terminal == 2U) GECODE_ME_CHECK(flag.one(home));
      return ES_OK;
    }
    static ExecStatus narrow_bounds(Home home, View x, View y, View z,
                                    Int::BoolView flag, bool& cube) {
      BoundLocalDomain d[3]; BoundLocalView v[3];
      bound_alias_domains(x,y,z,d,v);
      const WordValue initial_lo[3]={d[0].lo,d[1].lo,d[2].lo};
      const WordValue initial_hi[3]={d[0].hi,d[1].hi,d[2].hi};
      unsigned int terminal=flag.one() ? 2U : flag.zero() ? 1U : 3U;
      const unsigned int initial_terminal=terminal;
      BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
      if (x.varimp() == y.varimp()) role[1]=role[0];
      if (x.varimp() == z.varimp()) role[2]=role[0];
      else if (y.varimp() == z.varimp()) role[2]=role[1];
      for (;;) {
        const BoundLocalDomain old[3]={d[0],d[1],d[2]};
        const unsigned int old_terminal=terminal;
        for (unsigned int i=0; i<3; i++) d[i].deferred=true;
        if (!narrow_ranges(role,terminal)) return ES_FAILED;
        for (unsigned int i=0; i<3; i++) {
          d[i].deferred=false;
          if (!d[i].synchronize()) return ES_FAILED;
        }
        if ((d[0] == old[0]) && (d[1] == old[1]) &&
            (d[2] == old[2]) && (terminal == old_terminal))
          break;
      }
      cube=terminal != initial_terminal;
      for (unsigned int i=0; i<3; i++)
        cube |= (d[i].lo != initial_lo[i]) || (d[i].hi != initial_hi[i]);
      return publish(home,x,y,z,flag,d,terminal);
    }
    static ExecStatus narrow(Home home, View x, View y, View z,
                             Int::BoolView flag, bool cube) {
      BoundLocalDomain d[3]; BoundLocalView v[3];
      bound_alias_domains(x,y,z,d,v);
      unsigned int terminal=flag.one() ? 2U : flag.zero() ? 1U : 3U;
      BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
      if (x.varimp() == y.varimp()) role[1]=role[0];
      if (x.varimp() == z.varimp()) role[2]=role[0];
      else if (y.varimp() == z.varimp()) role[2]=role[1];
      for (;;) {
        const BoundLocalDomain old[3]={d[0],d[1],d[2]};
        const unsigned int old_terminal=terminal;
        for (unsigned int i=0; i<3; i++) d[i].deferred=true;
        if (cube) {
          unsigned int final;
          if (op == BA_ADD)
            GECODE_ES_CHECK(add_narrow(home,v[0],v[1],v[2],terminal,final));
          else
            GECODE_ES_CHECK(sub_narrow(home,v[0],v[1],v[2],terminal,final));
          terminal=final;
        }
        const WordValue before_lo[3]={d[0].lo,d[1].lo,d[2].lo};
        const WordValue before_hi[3]={d[0].hi,d[1].hi,d[2].hi};
        const unsigned int before_terminal=terminal;
        if (!narrow_ranges(role,terminal)) return ES_FAILED;
        for (unsigned int i=0; i<3; i++) {
          d[i].deferred=false;
          if (!d[i].synchronize()) return ES_FAILED;
        }
        cube=terminal != before_terminal;
        for (unsigned int i=0; i<3; i++)
          cube |= (d[i].lo != before_lo[i]) ||
            (d[i].hi != before_hi[i]);
        if ((d[0] == old[0]) && (d[1] == old[1]) &&
            (d[2] == old[2]) && (terminal == old_terminal))
          break;
      }
      return publish(home,x,y,z,flag,d,terminal);
    }
  public:
    static bool numeric_regime(View x, View y, Int::BoolView flag) {
      if (flag.assigned()) return true;
      const WordValue mask=width_mask(x.width());
      return (op == BA_ADD) ?
        ((x.rank_minimum() > mask-y.rank_minimum()) ||
         (x.rank_maximum() <= mask-y.rank_maximum())) :
        ((x.rank_maximum() < y.rank_minimum()) ||
         (x.rank_minimum() >= y.rank_maximum()));
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundFlagArithmetic(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return (View::me(med) == ME_WORD_BND) ?
        PropCost::ternary(PropCost::LO) :
        PropCost::linear(PropCost::HI,x0.width());
    }
    virtual void reschedule(Space& home) {
      x0.reschedule(home,*this,PC_WORD_DOM);
      x1.reschedule(home,*this,PC_WORD_DOM);
      x2.reschedule(home,*this,PC_WORD_DOM);
      flag.reschedule(home,*this,Int::PC_BOOL_VAL);
    }
    virtual size_t dispose(Space& home) {
      x0.cancel(home,*this,PC_WORD_DOM);
      x1.cancel(home,*this,PC_WORD_DOM);
      x2.cancel(home,*this,PC_WORD_DOM);
      flag.cancel(home,*this,Int::PC_BOOL_VAL);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (View::me(med) == ME_WORD_BND) {
        bool cube;
        GECODE_ES_CHECK(narrow_bounds(home,x0,x1,x2,flag,cube));
        if (x0.assigned() && x1.assigned() && x2.assigned() &&
            flag.assigned())
          return home.ES_SUBSUMED(*this);
        if (flag.zero())
          GECODE_REWRITE(*this,(BoundArithmetic<View,op,BT_CLEAR>::post(
            home(*this),x0,x1,x2)));
        if (flag.one())
          GECODE_REWRITE(*this,(BoundArithmetic<View,op,BT_SET>::post(
            home(*this),x0,x1,x2)));
        if (cube)
          return home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS));
        return ES_FIX;
      }
      GECODE_ES_CHECK(narrow(home,x0,x1,x2,flag,true));
      if (x0.assigned() && x1.assigned() && x2.assigned() && flag.assigned())
        return home.ES_SUBSUMED(*this);
      if (flag.zero())
        GECODE_REWRITE(*this,(BoundArithmetic<View,op,BT_CLEAR>::post(
          home(*this),x0,x1,x2)));
      if (flag.one())
        GECODE_REWRITE(*this,(BoundArithmetic<View,op,BT_SET>::post(
          home(*this),x0,x1,x2)));
      return ES_FIX;
    }
    static ExecStatus post(Home home, View x, View y, View z,
                           Int::BoolView flag) {
      GECODE_ES_CHECK(narrow(home,x,y,z,flag,true));
      if (!(x.assigned() && y.assigned() && z.assigned() && flag.assigned()))
        (void) new (home) BoundFlagArithmetic(home,x,y,z,flag);
      return ES_OK;
    }
  };

}}}

#endif
