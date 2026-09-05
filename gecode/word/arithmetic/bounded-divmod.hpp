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
 */

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_DIVMOD_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_DIVMOD_HPP

namespace Gecode { namespace Word { namespace Arithmetic {

  enum BoundUnsignedDivModOperation {
    BUD_DIV,
    BUD_MOD
  };

  forceinline WordValue
  bound_sat_add(WordValue x, WordValue y, WordValue mask) {
    return (x > mask-y) ? mask : x+y;
  }

  forceinline WordValue
  bound_sat_mult(WordValue x, WordValue y, WordValue mask) {
    return ((x != 0) && (y > mask/x)) ? mask : x*y;
  }

  forceinline bool
  bound_unsigned_div_ranges(BoundLocalDomain& a, BoundLocalDomain& b,
                            BoundLocalDomain& q) {
    const WordValue mask=width_mask(a.width);
    if (b.maximum == 0)
      return q.range(mask,mask);
    assert(b.minimum != 0);
    if (!q.range(a.minimum/b.maximum,a.maximum/b.minimum))
      return false;
    const WordValue amin=bound_sat_mult(q.minimum,b.minimum,mask);
    WordValue amax=bound_sat_mult(q.maximum,b.maximum,mask);
    amax=bound_sat_add(amax,b.maximum-1,mask);
    if (!a.range(amin,amax)) return false;
    WordValue bmin=b.minimum, bmax=b.maximum;
    if (q.maximum != mask)
      bmin=std::max(bmin,a.minimum/(q.maximum+1)+1);
    if (q.minimum != 0)
      bmax=std::min(bmax,a.maximum/q.minimum);
    return b.range(bmin,bmax);
  }

  forceinline bool
  bound_unsigned_mod_ranges(BoundLocalDomain& a, BoundLocalDomain& b,
                            BoundLocalDomain& r) {
    if (b.maximum == 0) {
      const WordValue minimum=std::max(a.minimum,r.minimum);
      const WordValue maximum=std::min(a.maximum,r.maximum);
      return a.range(minimum,maximum) && r.range(minimum,maximum);
    }
    assert(b.minimum != 0);
    if (!r.range(0,std::min(a.maximum,b.maximum-1))) return false;
    if (!a.range(r.minimum,a.maximum)) return false;
    return b.range(std::max(b.minimum,r.minimum+1),b.maximum);
  }

  template<BoundUnsignedDivModOperation op>
  class BoundUnsignedDivMod
    : public TernaryPropagator<UnsignedWordView,PC_WORD_DOM> {
  protected:
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x0;
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x1;
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x2;
    BoundUnsignedDivMod(Home home, UnsignedWordView a,
                       UnsignedWordView b, UnsignedWordView r)
      : TernaryPropagator<UnsignedWordView,PC_WORD_DOM>(home,a,b,r) {}
    BoundUnsignedDivMod(Space& home, BoundUnsignedDivMod& p)
      : TernaryPropagator<UnsignedWordView,PC_WORD_DOM>(home,p) {}
    static ExecStatus narrow(Home home, UnsignedWordView a,
                             UnsignedWordView b, UnsignedWordView result) {
      BoundLocalDomain d[3]; BoundLocalView v[3];
      bound_alias_domains(a,b,result,d,v);
      BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
      if (a.varimp() == b.varimp()) role[1]=role[0];
      if (a.varimp() == result.varimp()) role[2]=role[0];
      else if (b.varimp() == result.varimp()) role[2]=role[1];
      const WordValue mask=a.mask();
      const BoundLocalDomain initial[3]={d[0],d[1],d[2]};
      for (;;) {
        const BoundLocalDomain old[3]={d[0],d[1],d[2]};
        for (unsigned int i=0; i<3; i++) d[i].deferred=true;
        if (v[0] == v[1]) {
          const WordValue expected=(op == BUD_MOD) ? 0 :
            ((role[0]->maximum == 0) ? mask : 1);
          if (me_failed(v[2].narrow(home,expected,expected)) ||
              !role[2]->range(expected,expected))
            return ES_FAILED;
        }
        if (role[1]->maximum == 0) {
          if (op == BUD_DIV) {
            if (me_failed(v[2].narrow(home,mask,mask))) return ES_FAILED;
            if (!role[2]->range(mask,mask)) return ES_FAILED;
          } else {
            const WordValue lo=v[0].lo()|v[2].lo();
            const WordValue hi=v[0].hi()&v[2].hi();
            if (me_failed(v[0].narrow(home,lo,hi)) ||
                me_failed(v[2].narrow(home,lo,hi))) return ES_FAILED;
          }
        } else {
          assert(role[1]->minimum != 0);
          const bool ok=(op == BUD_DIV) ?
            bound_unsigned_div_ranges(*role[0],*role[1],*role[2]) :
            bound_unsigned_mod_ranges(*role[0],*role[1],*role[2]);
          if (!ok) return ES_FAILED;
          if ((op == BUD_MOD) && v[1].assigned()) {
            const WordValue divisor=v[1].val();
            if ((divisor & (divisor-1)) == 0) {
              const WordValue low=divisor-1;
              if (me_failed(v[2].narrow(home,v[0].lo()&low,
                                        v[0].hi()&low)) ||
                  me_failed(v[0].narrow(home,
                    v[0].lo()|(v[2].lo()&low),
                    (v[0].hi()&~low)|(v[2].hi()&low))))
                return ES_FAILED;
            }
          }
        }
        if (v[0].assigned() && v[1].assigned()) {
          const WordValue expected=(v[1].val() == 0) ?
            ((op == BUD_DIV) ? mask : v[0].val()) :
            ((op == BUD_DIV) ? v[0].val()/v[1].val() :
                               v[0].val()%v[1].val());
          if (me_failed(v[2].narrow(home,expected,expected)))
            return ES_FAILED;
          if (!role[2]->range(expected,expected)) return ES_FAILED;
        }
        for (unsigned int i=0; i<3; i++) d[i].deferred=false;
        for (unsigned int i=0; i<3; i++) {
          bool distinct=true;
          for (unsigned int j=0; j<i; j++)
            if (role[i] == role[j]) { distinct=false; break; }
          if (distinct && !role[i]->synchronize()) return ES_FAILED;
        }
        if ((d[0] == old[0]) && (d[1] == old[1]) && (d[2] == old[2]))
          break;
      }
      if (!(d[0] == initial[0])) GECODE_ES_CHECK(bound_publish(home,a,d[0]));
      if ((b.varimp() != a.varimp()) && !(d[1] == initial[1]))
        GECODE_ES_CHECK(bound_publish(home,b,d[1]));
      if ((result.varimp() != a.varimp()) &&
          (result.varimp() != b.varimp()) && !(d[2] == initial[2]))
        GECODE_ES_CHECK(bound_publish(home,result,d[2]));
      return ES_OK;
    }
  public:
    static bool numeric_regime(UnsignedWordView b) {
      return (b.rank_maximum() == 0) || (b.rank_minimum() != 0);
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundUnsignedDivMod(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,x0,x1,x2));
      return (x0.assigned() && x1.assigned() && x2.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView a,
                           UnsignedWordView b, UnsignedWordView result) {
      GECODE_ES_CHECK(narrow(home,a,b,result));
      if (a.varimp() == b.varimp())
        return ES_OK;
      if (!(a.assigned() && b.assigned() && result.assigned()))
        (void) new (home) BoundUnsignedDivMod(home,a,b,result);
      return ES_OK;
    }
  };

  class BoundUnsignedDivModBoth : public Propagator {
  protected:
    UnsignedWordView a, b, q, r;
    BoundUnsignedDivModBoth(Home home, UnsignedWordView a0,
                            UnsignedWordView b0, UnsignedWordView q0,
                            UnsignedWordView r0)
      : Propagator(home), a(a0), b(b0), q(q0), r(r0) {
      a.subscribe(home,*this,PC_WORD_DOM); b.subscribe(home,*this,PC_WORD_DOM);
      q.subscribe(home,*this,PC_WORD_DOM); r.subscribe(home,*this,PC_WORD_DOM);
    }
    BoundUnsignedDivModBoth(Space& home, BoundUnsignedDivModBoth& p)
      : Propagator(home,p) {
      a.update(home,p.a); b.update(home,p.b); q.update(home,p.q); r.update(home,p.r);
    }
    static ExecStatus narrow(Home home, UnsignedWordView a,
                             UnsignedWordView b, UnsignedWordView q,
                             UnsignedWordView r) {
      UnsignedWordView x[4]={a,b,q,r};
      BoundLocalDomain d[4]={bound_snapshot(a),bound_snapshot(b),
                             bound_snapshot(q),bound_snapshot(r)};
      BoundLocalDomain* role[4]={&d[0],&d[1],&d[2],&d[3]};
      BoundLocalView v[4];
      for (unsigned int i=0; i<4; i++) {
        for (unsigned int j=0; j<i; j++)
          if (x[i].varimp() == x[j].varimp()) { role[i]=role[j]; break; }
        v[i]=BoundLocalView(*role[i]);
      }
      const WordValue mask=a.mask();
      const BoundLocalDomain initial[4]={d[0],d[1],d[2],d[3]};
      for (;;) {
        const BoundLocalDomain old[4]={d[0],d[1],d[2],d[3]};
        for (unsigned int i=0; i<4; i++) d[i].deferred=true;
        if (v[0] == v[1]) {
          const WordValue qv=(role[0]->maximum == 0) ? mask : 1;
          if (me_failed(v[2].narrow(home,qv,qv)) ||
              me_failed(v[3].narrow(home,0,0)) ||
              !role[2]->range(qv,qv) || !role[3]->range(0,0))
            return ES_FAILED;
        }
        if (role[1]->maximum == 0) {
          if (me_failed(v[2].narrow(home,mask,mask)) ||
              !role[2]->range(mask,mask)) return ES_FAILED;
          const WordValue lo=v[0].lo()|v[3].lo();
          const WordValue hi=v[0].hi()&v[3].hi();
          if (me_failed(v[0].narrow(home,lo,hi)) ||
              me_failed(v[3].narrow(home,lo,hi))) return ES_FAILED;
          const WordValue minimum=std::max(role[0]->minimum,role[3]->minimum);
          const WordValue maximum=std::min(role[0]->maximum,role[3]->maximum);
          if (!role[0]->range(minimum,maximum) ||
              !role[3]->range(minimum,maximum)) return ES_FAILED;
        } else {
          if (!bound_unsigned_div_ranges(*role[0],*role[1],*role[2]) ||
              !bound_unsigned_mod_ranges(*role[0],*role[1],*role[3]))
            return ES_FAILED;
          if (v[1].assigned()) {
            const WordValue divisor=v[1].val();
            if ((divisor & (divisor-1)) == 0) {
              const WordValue low=divisor-1;
              if (me_failed(v[3].narrow(home,v[0].lo()&low,
                                        v[0].hi()&low)) ||
                  me_failed(v[0].narrow(home,
                    v[0].lo()|(v[3].lo()&low),
                    (v[0].hi()&~low)|(v[3].hi()&low))))
                return ES_FAILED;
            }
          }
          WordValue amin=bound_sat_mult(role[2]->minimum,
                                        role[1]->minimum,mask);
          amin=bound_sat_add(amin,role[3]->minimum,mask);
          WordValue amax=bound_sat_mult(role[2]->maximum,
                                        role[1]->maximum,mask);
          amax=bound_sat_add(amax,role[3]->maximum,mask);
          if (!role[0]->range(amin,amax)) return ES_FAILED;
        }
        if (v[0].assigned() && v[1].assigned()) {
          const WordValue qv=(v[1].val()==0) ? mask : v[0].val()/v[1].val();
          const WordValue rv=(v[1].val()==0) ? v[0].val() : v[0].val()%v[1].val();
          if (me_failed(v[2].narrow(home,qv,qv)) ||
              me_failed(v[3].narrow(home,rv,rv)) ||
              !role[2]->range(qv,qv) || !role[3]->range(rv,rv))
            return ES_FAILED;
        }
        for (unsigned int i=0; i<4; i++) d[i].deferred=false;
        for (unsigned int i=0; i<4; i++) {
          bool distinct=true;
          for (unsigned int j=0; j<i; j++)
            if (role[i] == role[j]) { distinct=false; break; }
          if (distinct && !role[i]->synchronize()) return ES_FAILED;
        }
        if ((d[0] == old[0]) && (d[1] == old[1]) &&
            (d[2] == old[2]) && (d[3] == old[3])) break;
      }
      for (unsigned int i=0; i<4; i++) {
        bool distinct=true;
        for (unsigned int j=0; j<i; j++)
          if (x[i].varimp() == x[j].varimp()) { distinct=false; break; }
        if (distinct && !(d[i] == initial[i]))
          GECODE_ES_CHECK(bound_publish(home,x[i],d[i]));
      }
      return ES_OK;
    }
  public:
    static bool numeric_regime(UnsignedWordView b) {
      return (b.rank_maximum() == 0) || (b.rank_minimum() != 0);
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundUnsignedDivModBoth(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,a.width());
    }
    virtual void reschedule(Space& home) {
      a.reschedule(home,*this,PC_WORD_DOM); b.reschedule(home,*this,PC_WORD_DOM);
      q.reschedule(home,*this,PC_WORD_DOM); r.reschedule(home,*this,PC_WORD_DOM);
    }
    virtual size_t dispose(Space& home) {
      a.cancel(home,*this,PC_WORD_DOM); b.cancel(home,*this,PC_WORD_DOM);
      q.cancel(home,*this,PC_WORD_DOM); r.cancel(home,*this,PC_WORD_DOM);
      (void) Propagator::dispose(home); return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,a,b,q,r));
      return (a.assigned() && b.assigned() && q.assigned() && r.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView a, UnsignedWordView b,
                           UnsignedWordView q, UnsignedWordView r) {
      GECODE_ES_CHECK(narrow(home,a,b,q,r));
      if (a.varimp() == b.varimp())
        return ES_OK;
      if (!(a.assigned() && b.assigned() && q.assigned() && r.assigned()))
        (void) new (home) BoundUnsignedDivModBoth(home,a,b,q,r);
      return ES_OK;
    }
  };

  template<SignedDivModOperation op>
  class BoundSignedDivMod
    : public TernaryPropagator<SignedWordView,PC_WORD_DOM> {
  protected:
    using TernaryPropagator<SignedWordView,PC_WORD_DOM>::x0;
    using TernaryPropagator<SignedWordView,PC_WORD_DOM>::x1;
    using TernaryPropagator<SignedWordView,PC_WORD_DOM>::x2;
    BoundSignedDivMod(Home home, SignedWordView a, SignedWordView b,
                      SignedWordView r)
      : TernaryPropagator<SignedWordView,PC_WORD_DOM>(home,a,b,r) {}
    BoundSignedDivMod(Space& home, BoundSignedDivMod& p)
      : TernaryPropagator<SignedWordView,PC_WORD_DOM>(home,p) {}
    static ExecStatus narrow(Home home, SignedWordView a, SignedWordView b,
                             SignedWordView result) {
      BoundLocalDomain d[3]; BoundLocalView v[3];
      bound_alias_domains(a,b,result,d,v);
      BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
      if (a.varimp() == b.varimp()) role[1]=role[0];
      if (a.varimp() == result.varimp()) role[2]=role[0];
      else if (b.varimp() == result.varimp()) role[2]=role[1];
      const WordValue mask=a.mask(), sign=sign_bit(a.width());
      const BoundLocalDomain initial[3]={d[0],d[1],d[2]};
      for (;;) {
        const BoundLocalDomain old[3]={d[0],d[1],d[2]};
        for (unsigned int i=0; i<3; i++) d[i].deferred=true;
        if (v[0] == v[1]) {
          const WordValue expected=(op == SDO_DIV) ?
            ((role[0]->minimum == sign && role[0]->maximum == sign) ?
              mask : 1) : 0;
          if (me_failed(v[2].narrow(home,expected,expected)) ||
              !role[2]->range(expected^sign,expected^sign))
            return ES_FAILED;
        }
        if ((role[1]->minimum == sign) && (role[1]->maximum == sign)) {
          if (op == SDO_DIV) {
            const WordValue negative_rank=WordValue(1)^sign;
            const WordValue positive_rank=mask^sign;
            WordValue minimum=std::min(negative_rank,positive_rank);
            WordValue maximum=std::max(negative_rank,positive_rank);
            if (role[0]->maximum < sign)
              minimum=maximum=negative_rank;
            else if (role[0]->minimum >= sign)
              minimum=maximum=positive_rank;
            if (!role[2]->range(minimum,maximum)) return ES_FAILED;
            if ((minimum == maximum) &&
                me_failed(v[2].narrow(home,minimum^sign,minimum^sign)))
              return ES_FAILED;
            if ((minimum != maximum) &&
                me_failed(v[2].narrow(home,WordValue(1),mask)))
              return ES_FAILED;
          } else {
            const WordValue lo=v[0].lo()|v[2].lo();
            const WordValue hi=v[0].hi()&v[2].hi();
            if (me_failed(v[0].narrow(home,lo,hi)) ||
                me_failed(v[2].narrow(home,lo,hi))) return ES_FAILED;
            const WordValue minimum=std::max(role[0]->minimum,role[2]->minimum);
            const WordValue maximum=std::min(role[0]->maximum,role[2]->maximum);
            if (!role[0]->range(minimum,maximum) ||
                !role[2]->range(minimum,maximum)) return ES_FAILED;
          }
        } else {
          const WordValue av[2]={role[0]->minimum^sign,
                                 role[0]->maximum^sign};
          const WordValue bv[2]={role[1]->minimum^sign,
                                 role[1]->maximum^sign};
          if (op == SDO_DIV) {
            const WordValue minus_one_rank=mask^sign;
            const bool wraps_at_minimum=
              (role[0]->minimum == 0) && (role[0]->maximum != 0) &&
              (role[1]->minimum <= minus_one_rank) &&
              (role[1]->maximum >= minus_one_rank);
            if (!wraps_at_minimum) {
              WordValue minimum=mask, maximum=0;
              for (unsigned int i=0; i<2; i++)
                for (unsigned int j=0; j<2; j++) {
                  const WordValue rank=SignedDivModSupport::evaluate<SDO_DIV>(
                    av[i],bv[j],sign,mask)^sign;
                  minimum=std::min(minimum,rank);
                  maximum=std::max(maximum,rank);
                }
              if (!role[2]->range(minimum,maximum)) return ES_FAILED;
            }
            if (v[1].assigned()) {
              const WordValue divisor_rank=v[1].val()^sign;
              WordValue p0, p1;
              if (bound_signed_product(role[2]->minimum,divisor_rank,
                                       sign,mask,p0) &&
                  bound_signed_product(role[2]->maximum,divisor_rank,
                                       sign,mask,p1)) {
                WordValue amin=std::min(p0,p1), amax=std::max(p0,p1);
                const WordValue magnitude=SignedDivModSupport::magnitude(
                  v[1].val(),sign,mask);
                const WordValue delta=sign+(magnitude-1);
                WordValue expanded;
                if (amin <= sign)
                  amin=bound_signed_sub(amin,delta,sign,mask,expanded) ?
                    expanded : 0;
                if (amax >= sign)
                  amax=bound_signed_add(amax,delta,sign,mask,expanded) ?
                    expanded : mask;
                if (!role[0]->range(amin,amax)) return ES_FAILED;
              }
            }
          } else {
            WordValue magnitude=0;
            for (unsigned int j=0; j<2; j++)
              magnitude=std::max(magnitude,
                SignedDivModSupport::magnitude(bv[j],sign,mask));
            const WordValue limit=magnitude-1;
            WordValue minimum, maximum;
            BoundLocalDomain& source=(op == SDO_REM) ? *role[0] : *role[1];
            if (source.maximum < sign) {
              minimum=sign-limit; maximum=sign;
            } else if (source.minimum >= sign) {
              minimum=sign; maximum=sign+limit;
            } else {
              minimum=sign-limit; maximum=sign+limit;
            }
            if (!role[2]->range(minimum,maximum)) return ES_FAILED;
            WordValue required=0;
            if (role[2]->maximum < sign)
              required=sign-role[2]->maximum;
            else if (role[2]->minimum > sign)
              required=role[2]->minimum-sign;
            if (required != 0) {
              if (required >= sign) return ES_FAILED;
              required++;
              if (role[1]->maximum < sign) {
                if (!role[1]->range(role[1]->minimum,
                                    std::min(role[1]->maximum,
                                             sign-required)))
                  return ES_FAILED;
              } else if (role[1]->minimum > sign) {
                if (!role[1]->range(std::max(role[1]->minimum,
                                             sign+required),
                                    role[1]->maximum))
                  return ES_FAILED;
              }
            }
          }
        }
        if (v[0].assigned() && v[1].assigned()) {
          const WordValue expected=SignedDivModSupport::evaluate<op>(
            v[0].val(),v[1].val(),sign,mask);
          if (me_failed(v[2].narrow(home,expected,expected)) ||
              !role[2]->range(expected^sign,expected^sign))
            return ES_FAILED;
        }
        for (unsigned int i=0; i<3; i++) d[i].deferred=false;
        for (unsigned int i=0; i<3; i++) {
          bool distinct=true;
          for (unsigned int j=0; j<i; j++)
            if (role[i] == role[j]) { distinct=false; break; }
          if (distinct && !role[i]->synchronize()) return ES_FAILED;
        }
        if ((d[0] == old[0]) && (d[1] == old[1]) && (d[2] == old[2]))
          break;
      }
      if (!(d[0] == initial[0])) GECODE_ES_CHECK(bound_publish(home,a,d[0]));
      if ((b.varimp() != a.varimp()) && !(d[1] == initial[1]))
        GECODE_ES_CHECK(bound_publish(home,b,d[1]));
      if ((result.varimp() != a.varimp()) &&
          (result.varimp() != b.varimp()) && !(d[2] == initial[2]))
        GECODE_ES_CHECK(bound_publish(home,result,d[2]));
      return ES_OK;
    }
  public:
    static bool numeric_regime(SignedWordView b) {
      const WordValue sign=sign_bit(b.width());
      return ((b.rank_minimum() == sign) &&
              (b.rank_maximum() == sign)) ||
        (b.rank_maximum() < sign) || (b.rank_minimum() > sign);
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundSignedDivMod(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,x0,x1,x2));
      return (x0.assigned() && x1.assigned() && x2.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, SignedWordView a, SignedWordView b,
                           SignedWordView result) {
      GECODE_ES_CHECK(narrow(home,a,b,result));
      if (a.varimp() == b.varimp())
        return ES_OK;
      if (!(a.assigned() && b.assigned() && result.assigned()))
        (void) new (home) BoundSignedDivMod(home,a,b,result);
      return ES_OK;
    }
  };

}}}

#endif

// STATISTICS: word-prop
