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

    bool synchronize(void) {
      return synchronize_domain(width,kind,lo,hi,minimum,maximum);
    }
    bool range(WordValue min, WordValue max) {
      minimum=std::max(minimum,min);
      maximum=std::min(maximum,max);
      return (minimum <= maximum) && synchronize();
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
      if ((d->lo & ~d->hi) != 0 || !d->synchronize())
        return ME_WORD_FAILED;
      return assigned() ? ME_WORD_VAL : ME_WORD_DOM;
    }
    bool operator ==(const BoundLocalView& y) const { return d == y.d; }
    bool operator !=(const BoundLocalView& y) const { return d != y.d; }
  };

  template<class View>
  forceinline BoundLocalDomain
  bound_snapshot(View x) {
    return BoundLocalDomain{x.width(),x.domain_type(),x.lo(),x.hi(),
                            x.minimum(),x.maximum()};
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

  template<class View> struct BoundIsSigned { static const bool value=false; };
  template<> struct BoundIsSigned<SignedWordView> {
    static const bool value=true;
  };

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
    if (!BoundIsSigned<View>::value) {
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
    if (!BoundIsSigned<View>::value) {
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
    if (!BoundIsSigned<View>::value) {
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
    if (!BoundIsSigned<View>::value) {
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
    return BoundIsSigned<View>::value ? bound_mult_signed(x,y,z) :
      bound_mult_unsigned(x,y,z);
  }

  template<class View>
  forceinline bool
  bound_neg_ranges(BoundLocalDomain& x, BoundLocalDomain& z) {
    if (!BoundIsSigned<View>::value)
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

  template<class View, BoundArithmeticOperation op>
  class BoundArithmetic : public TernaryPropagator<View,PC_WORD_DOM> {
  protected:
    using TernaryPropagator<View,PC_WORD_DOM>::x0;
    using TernaryPropagator<View,PC_WORD_DOM>::x1;
    using TernaryPropagator<View,PC_WORD_DOM>::x2;
    BoundArithmetic(Home home, View x, View y, View z)
      : TernaryPropagator<View,PC_WORD_DOM>(home,x,y,z) {}
    BoundArithmetic(Space& home, BoundArithmetic& p)
      : TernaryPropagator<View,PC_WORD_DOM>(home,p) {}
    static ExecStatus narrow(Home home, View x, View y, View z) {
      BoundLocalDomain d[3]; BoundLocalView v[3];
      bound_alias_domains(x,y,z,d,v);
      for (;;) {
        const BoundLocalDomain old[3]={d[0],d[1],d[2]};
        if (op == BA_ADD) {
          unsigned int final;
          GECODE_ES_CHECK(add_narrow(home,v[0],v[1],v[2],3U,final));
        }
        if (op == BA_SUB) {
          unsigned int final;
          GECODE_ES_CHECK(sub_narrow(home,v[0],v[1],v[2],3U,final));
        }
        if (op == BA_MULT)
          GECODE_ES_CHECK(mult_narrow_views(home,v[0],v[1],v[2]));
        // Roles that alias share a local record; use the role mapping below.
        BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
        if (x.varimp() == y.varimp()) role[1]=role[0];
        if (x.varimp() == z.varimp()) role[2]=role[0];
        else if (y.varimp() == z.varimp()) role[2]=role[1];
        bool ok = op == BA_ADD ? bound_add_ranges<View>(*role[0],*role[1],*role[2]) :
          op == BA_SUB ? bound_sub_ranges<View>(*role[0],*role[1],*role[2]) :
          bound_mult_ranges<View>(*role[0],*role[1],*role[2]);
        if (!ok) return ES_FAILED;
        if ((d[0] == old[0]) && (d[1] == old[1]) && (d[2] == old[2]))
          break;
      }
      return bound_publish_distinct(home,x,y,z,d);
    }
  public:
    static bool numeric_regime(View x, View y) {
      const WordValue mask=width_mask(x.width());
      if (op == BA_ADD) {
        if (!BoundIsSigned<View>::value)
          return x.maximum() <= mask-y.maximum();
        const WordValue sign=sign_bit(x.width()); WordValue ignored;
        return bound_signed_add(x.minimum(),y.minimum(),sign,mask,ignored) &&
          bound_signed_add(x.maximum(),y.maximum(),sign,mask,ignored);
      }
      if (op == BA_SUB) {
        if (!BoundIsSigned<View>::value)
          return x.minimum() >= y.maximum();
        const WordValue sign=sign_bit(x.width()); WordValue ignored;
        return bound_signed_sub(x.minimum(),y.maximum(),sign,mask,ignored) &&
          bound_signed_sub(x.maximum(),y.minimum(),sign,mask,ignored);
      }
      if (!BoundIsSigned<View>::value)
        return (x.maximum() == 0U) ||
          (y.maximum() <= mask/x.maximum());
      const WordValue sign=sign_bit(x.width()); WordValue ignored;
      return bound_signed_product(x.minimum(),y.minimum(),sign,mask,ignored) &&
        bound_signed_product(x.minimum(),y.maximum(),sign,mask,ignored) &&
        bound_signed_product(x.maximum(),y.minimum(),sign,mask,ignored) &&
        bound_signed_product(x.maximum(),y.maximum(),sign,mask,ignored);
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundArithmetic(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,x0,x1,x2));
      return (x0.assigned() && x1.assigned() && x2.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, View x, View y, View z) {
      GECODE_ES_CHECK(narrow(home,x,y,z));
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
    static ExecStatus narrow(Home home, View x, View z) {
      BoundLocalDomain d[2]={bound_snapshot(x),bound_snapshot(z)};
      BoundLocalView vx(d[0]), vz((x.varimp() == z.varimp()) ? d[0] : d[1]);
      for (;;) {
        const BoundLocalDomain old[2]={d[0],d[1]};
        GECODE_ES_CHECK(neg_narrow(home,vx,vz));
        BoundLocalDomain& dz=(x.varimp() == z.varimp()) ? d[0] : d[1];
        if (!bound_neg_ranges<View>(d[0],dz)) return ES_FAILED;
        if ((d[0] == old[0]) && (d[1] == old[1])) break;
      }
      GECODE_ES_CHECK(bound_publish(home,x,d[0]));
      if (z.varimp() != x.varimp())
        GECODE_ES_CHECK(bound_publish(home,z,d[1]));
      return ES_OK;
    }
  public:
    static bool numeric_regime(View x) { return x.minimum() != 0U; }
    virtual Actor* copy(Space& home) { return new (home) BoundNeg(home,*this); }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,x0,x1));
      return (x0.assigned() && x1.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, View x, View z) {
      GECODE_ES_CHECK(narrow(home,x,z));
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
    static ExecStatus narrow(Home home, View x, View y, View z,
                             Int::BoolView flag) {
      BoundLocalDomain d[3]; BoundLocalView v[3];
      bound_alias_domains(x,y,z,d,v);
      unsigned int terminal=flag.one() ? 2U : flag.zero() ? 1U : 3U;
      for (;;) {
        const BoundLocalDomain old[3]={d[0],d[1],d[2]};
        const unsigned int old_terminal=terminal;
        unsigned int final;
        if (op == BA_ADD)
          GECODE_ES_CHECK(add_narrow(home,v[0],v[1],v[2],terminal,final));
        else
          GECODE_ES_CHECK(sub_narrow(home,v[0],v[1],v[2],terminal,final));
        terminal=final;

        BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
        if (x.varimp() == y.varimp()) role[1]=role[0];
        if (x.varimp() == z.varimp()) role[2]=role[0];
        else if (y.varimp() == z.varimp()) role[2]=role[1];
        const WordValue mask=width_mask(x.width());
        bool all_nonwrapping, all_wrapping;
        if (op == BA_ADD) {
          all_wrapping=role[0]->minimum > mask-role[1]->minimum;
          all_nonwrapping=role[0]->maximum <= mask-role[1]->maximum;
          if (all_wrapping)
            terminal &= 2U;
          if (all_nonwrapping)
            terminal &= 1U;
        } else {
          all_wrapping=role[0]->maximum < role[1]->minimum;
          all_nonwrapping=role[0]->minimum >= role[1]->maximum;
          if (all_wrapping)
            terminal &= 2U;
          if (all_nonwrapping)
            terminal &= 1U;
        }
        if (terminal == 0U) return ES_FAILED;
        if (terminal == 1U) {
          if (op == BA_ADD) {
            if (!role[0]->range(role[0]->minimum,
                                std::min(role[0]->maximum,
                                  mask-role[1]->minimum)) ||
                !role[1]->range(role[1]->minimum,
                                std::min(role[1]->maximum,
                                  mask-role[0]->minimum)))
              return ES_FAILED;
          } else {
            if (!role[0]->range(std::max(role[0]->minimum,
                                        role[1]->minimum),
                                role[0]->maximum) ||
                !role[1]->range(role[1]->minimum,
                                std::min(role[1]->maximum,
                                         role[0]->maximum)))
              return ES_FAILED;
          }
        }
        bool ok=true;
        if ((terminal == 2U) && all_wrapping) {
          WordValue minimum, maximum;
          if (op == BA_ADD) {
            minimum=role[0]->minimum-(mask-role[1]->minimum)-1U;
            maximum=role[0]->maximum-(mask-role[1]->maximum)-1U;
          } else {
            minimum=mask-(role[1]->maximum-role[0]->minimum)+1U;
            maximum=mask-(role[1]->minimum-role[0]->maximum)+1U;
          }
          ok=role[2]->range(minimum,maximum);
        } else if ((terminal != 2U) || all_nonwrapping) {
          ok = op == BA_ADD ?
            bound_add_ranges<View>(*role[0],*role[1],*role[2]) :
            bound_sub_ranges<View>(*role[0],*role[1],*role[2]);
        }
        if (!ok) return ES_FAILED;
        if ((d[0] == old[0]) && (d[1] == old[1]) &&
            (d[2] == old[2]) && (terminal == old_terminal))
          break;
      }
      GECODE_ES_CHECK(bound_publish_distinct(home,x,y,z,d));
      if (terminal == 1U) GECODE_ME_CHECK(flag.zero(home));
      else if (terminal == 2U) GECODE_ME_CHECK(flag.one(home));
      return ES_OK;
    }
  public:
    static bool numeric_regime(View x, View y, Int::BoolView flag) {
      if (flag.assigned()) return true;
      const WordValue mask=width_mask(x.width());
      return (op == BA_ADD) ?
        ((x.minimum() > mask-y.minimum()) ||
         (x.maximum() <= mask-y.maximum())) :
        ((x.maximum() < y.minimum()) || (x.minimum() >= y.maximum()));
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundFlagArithmetic(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
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
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,x0,x1,x2,flag));
      return (x0.assigned() && x1.assigned() && x2.assigned() &&
              flag.assigned()) ? home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, View x, View y, View z,
                           Int::BoolView flag) {
      GECODE_ES_CHECK(narrow(home,x,y,z,flag));
      if (!(x.assigned() && y.assigned() && z.assigned() && flag.assigned()))
        (void) new (home) BoundFlagArithmetic(home,x,y,z,flag);
      return ES_OK;
    }
  };

}}}

#endif
