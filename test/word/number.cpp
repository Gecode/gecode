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

#include "test/word.hh"
#include <gecode/word/arithmetic.hh>
#include <gecode/word/arithmetic/bounded.hpp>

#include <gecode/search.hh>

namespace Test { namespace Word { namespace Number {

  using Gecode::WordValue;

  static WordValue
  magnitude(WordValue value, unsigned int width, bool sign) {
    const WordValue mask=(WordValue(1) << width)-1U;
    if (!sign || ((value & (WordValue(1) << (width-1))) == 0U))
      return value;
    return (~value+1U)&mask;
  }

  static WordValue
  gcd_value(WordValue x, WordValue y) {
    while (y != 0U) {
      const WordValue r=x%y; x=y; y=r;
    }
    return x;
  }

  static bool
  progression_oracle(void) {
    using namespace Gecode;
    using namespace Gecode::Word::Arithmetic;
    for (unsigned int width=1; width<=5; width++) {
      const WordValue mask=(WordValue(1)<<width)-1U;
      for (unsigned int signed_order=0; signed_order<2; signed_order++) {
        const WordDomainType kind=signed_order ? WDT_SIGNED : WDT_UNSIGNED;
        for (WordValue hi=0U; hi<=mask; hi++) {
          for (WordValue lo=hi;; lo=(lo-1U)&hi) {
            BoundLocalDomain base={width,kind,lo,hi,0U,mask,false};
            if (!base.synchronize()) return false;
            const WordValue span=base.maximum-base.minimum;
            const WordValue interval_min[4]={
              base.minimum,base.minimum,base.minimum+span/2U,
              base.minimum+span/3U};
            const WordValue interval_max[4]={
              base.maximum,base.minimum+span/2U,base.maximum,
              base.maximum-span/3U};
            for (unsigned int interval=0; interval<4; interval++) {
              BoundLocalDomain narrowed=base;
              narrowed.minimum=interval_min[interval];
              narrowed.maximum=interval_max[interval];
              if (!narrowed.synchronize()) continue;
              WordValue ordered_lo, ordered_hi;
              Gecode::Word::ordered_cube(kind,width,narrowed.lo,narrowed.hi,
                                         ordered_lo,ordered_hi);
              for (WordValue step=1U; step<=mask; step++) {
                for (WordValue residue=0U; residue<step; residue++) {
                  bool found=false; WordValue first=0U, last=0U;
                  for (WordValue rank=narrowed.minimum;
                       rank<=narrowed.maximum; rank++) {
                    if (Gecode::Word::cube_contains(
                          ordered_lo,ordered_hi,rank,mask) &&
                        ((rank%step) == residue)) {
                      if (!found) first=rank;
                      last=rank; found=true;
                    }
                  }
                  BoundLocalDomain actual=narrowed;
                  bool success=true;
                  for (;;) {
                    const BoundLocalDomain old=actual;
                    actual.deferred=true;
                    success=bound_progression(actual,residue,step);
                    actual.deferred=false;
                    if (success) success=actual.synchronize();
                    if (!success || (actual == old)) break;
                  }
                  if (success != found) return false;
                  if (success && ((actual.minimum != first) ||
                                  (actual.maximum != last)))
                    return false;
                }
              }
            }
            if (lo == 0U) break;
          }
        }
      }
    }
    const WordValue maximum=~WordValue(0);
    BoundLocalDomain assigned={64,WDT_UNSIGNED,maximum,maximum,
                               maximum,maximum,true};
    if (!bound_progression(assigned,1U,maximum-1U) ||
        (assigned.minimum != maximum) || (assigned.maximum != maximum))
      return false;
    BoundLocalDomain incompatible={64,WDT_UNSIGNED,maximum,maximum,
                                   maximum,maximum,true};
    if (bound_progression(incompatible,0U,maximum-1U))
      return false;
    BoundLocalDomain overflow={64,WDT_UNSIGNED,0U,WordValue(1)<<63,
                               0U,maximum,true};
    if (!bound_progression(overflow,0U,maximum) ||
        (overflow.minimum != 0U) || (overflow.maximum != 0U))
      return false;

    const WordValue sign=WordValue(1)<<63;
    const WordValue negative_one_rank=sign-1U;
    const WordValue large_odd=(WordValue(1)<<62)-1U;
    BoundLocalDomain signed_odd={64,WDT_SIGNED,maximum,maximum,
                                 negative_one_rank,negative_one_rank,true};
    if (!bound_progression(
          signed_odd,negative_one_rank%large_odd,large_odd))
      return false;
    BoundLocalDomain signed_odd_bad={64,WDT_SIGNED,maximum,maximum,
                                     negative_one_rank,negative_one_rank,true};
    if (bound_progression(
          signed_odd_bad,(negative_one_rank%large_odd)+1U,large_odd))
      return false;

    const WordValue large_even=WordValue(1)<<62;
    BoundLocalDomain signed_even={64,WDT_SIGNED,0U,0U,sign,sign,true};
    if (!bound_progression(signed_even,0U,large_even))
      return false;
    BoundLocalDomain signed_even_bad={64,WDT_SIGNED,0U,0U,
                                      sign,sign,true};
    return !bound_progression(signed_even_bad,2U,large_even);
  }

  class GcdSpace : public Gecode::Space {
  public:
    Gecode::WordVar x, y, result;
    bool sign;
    GcdSpace(bool sign0, bool typed)
      : x(*this,3,typed ? (sign0 ? Gecode::WDT_SIGNED :
                           Gecode::WDT_UNSIGNED) : Gecode::WDT_CUBE),
        y(*this,3,typed ? (sign0 ? Gecode::WDT_SIGNED :
                           Gecode::WDT_UNSIGNED) : Gecode::WDT_CUBE),
        result(*this,3,typed ? Gecode::WDT_UNSIGNED : Gecode::WDT_CUBE),
        sign(sign0) {
      if (sign)
        Gecode::signed_gcd(*this,x,y,result);
      else
        Gecode::gcd(*this,x,y,result);
      Gecode::branch(*this,result,Gecode::WORD_VAL_LSB());
      Gecode::WordVarArgs words={x,y};
      Gecode::branch(*this,words,Gecode::WORD_VAR_SIZE_MIN(),
                     Gecode::WORD_VAL_LSB());
    }
    GcdSpace(GcdSpace& s) : Gecode::Space(s), sign(s.sign) {
      x.update(*this,s.x); y.update(*this,s.y); result.update(*this,s.result);
    }
    virtual Gecode::Space* copy(void) { return new GcdSpace(*this); }
  };

  class DividesSpace : public Gecode::Space {
  public:
    Gecode::WordVar divisor, dividend;
    Gecode::BoolVar b;
    bool sign;
    DividesSpace(bool sign0, bool typed)
      : divisor(*this,3,typed ? (sign0 ? Gecode::WDT_SIGNED :
                                 Gecode::WDT_UNSIGNED) : Gecode::WDT_CUBE),
        dividend(*this,3,typed ? (sign0 ? Gecode::WDT_SIGNED :
                                  Gecode::WDT_UNSIGNED) : Gecode::WDT_CUBE),
        b(*this,0,1), sign(sign0) {
      if (sign)
        Gecode::signed_divides(*this,divisor,dividend,Gecode::Reify(b));
      else
        Gecode::divides(*this,divisor,dividend,Gecode::Reify(b));
      Gecode::WordVarArgs words={divisor,dividend};
      Gecode::branch(*this,words,Gecode::WORD_VAR_SIZE_MIN(),
                     Gecode::WORD_VAL_LSB());
    }
    DividesSpace(DividesSpace& s) : Gecode::Space(s), sign(s.sign) {
      divisor.update(*this,s.divisor); dividend.update(*this,s.dividend);
      b.update(*this,s.b);
    }
    virtual Gecode::Space* copy(void) { return new DividesSpace(*this); }
  };

  class AliasGcdSpace : public Gecode::Space {
  public:
    Gecode::WordVar x, result;
    AliasGcdSpace(void)
      : x(*this,3,Gecode::WDT_UNSIGNED),
        result(*this,3,Gecode::WDT_UNSIGNED) {
      Gecode::gcd(*this,x,x,result);
      Gecode::WordVarArgs words={x,result};
      Gecode::branch(*this,words,Gecode::WORD_VAR_SIZE_MIN(),
                     Gecode::WORD_VAL_LSB());
    }
    AliasGcdSpace(AliasGcdSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x); result.update(*this,s.result);
    }
    virtual Gecode::Space* copy(void) { return new AliasGcdSpace(*this); }
  };

  class ReGcdSpace : public Gecode::Space {
  public:
    Gecode::WordVar x, y, result;
    Gecode::BoolVar b;
    ReGcdSpace(void)
      : x(*this,3), y(*this,3), result(*this,3), b(*this,0,1) {
      Gecode::gcd(*this,x,y,result,Gecode::Reify(b));
      Gecode::WordVarArgs words={x,y,result};
      Gecode::branch(*this,words,Gecode::WORD_VAR_SIZE_MIN(),
                     Gecode::WORD_VAL_LSB());
    }
    ReGcdSpace(ReGcdSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x); y.update(*this,s.y); result.update(*this,s.result);
      b.update(*this,s.b);
    }
    virtual Gecode::Space* copy(void) { return new ReGcdSpace(*this); }
  };

  class ResultAliasGcdSpace : public Gecode::Space {
  public:
    Gecode::WordVar x, y;
    ResultAliasGcdSpace(void)
      : x(*this,3,Gecode::WDT_UNSIGNED),
        y(*this,3,Gecode::WDT_UNSIGNED) {
      Gecode::gcd(*this,x,y,x);
      Gecode::WordVarArgs words={x,y};
      Gecode::branch(*this,words,Gecode::WORD_VAR_SIZE_MIN(),
                     Gecode::WORD_VAL_LSB());
    }
    ResultAliasGcdSpace(ResultAliasGcdSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x); y.update(*this,s.y);
    }
    virtual Gecode::Space* copy(void) {
      return new ResultAliasGcdSpace(*this);
    }
  };

  static bool
  exhaustive(void) {
    for (unsigned int mode=0; mode<4; mode++) {
      const bool sign=(mode & 1U) != 0;
      const bool typed=(mode & 2U) != 0;
      GcdSpace* root=new GcdSpace(sign,typed);
      Gecode::DFS<GcdSpace> search(root);
      delete root;
      unsigned int solutions=0;
      while (GcdSpace* s=search.next()) {
        const WordValue expected=gcd_value(magnitude(s->x.val(),3,sign),
          magnitude(s->y.val(),3,sign));
        const bool ok=s->result.assigned() &&
          (s->result.val() == expected) &&
          (Gecode::PropagatorGroup::all.size(*s) == 0);
        delete s;
        if (!ok) return false;
        solutions++;
      }
      if (solutions != 64U) return false;

      DividesSpace* droot=new DividesSpace(sign,typed);
      Gecode::DFS<DividesSpace> dsearch(droot);
      delete droot;
      solutions=0;
      while (DividesSpace* s=dsearch.next()) {
        const WordValue d=magnitude(s->divisor.val(),3,sign);
        const WordValue n=magnitude(s->dividend.val(),3,sign);
        const bool expected=(d == 0U) ? (n == 0U) : ((n%d) == 0U);
        const bool ok=s->b.assigned() &&
          (static_cast<bool>(s->b.val()) == expected) &&
          (Gecode::PropagatorGroup::all.size(*s) == 0);
        delete s;
        if (!ok) return false;
        solutions++;
      }
      if (solutions != 64U) return false;
    }

    AliasGcdSpace* root=new AliasGcdSpace;
    Gecode::DFS<AliasGcdSpace> search(root);
    delete root;
    unsigned int solutions=0;
    while (AliasGcdSpace* s=search.next()) {
      const bool ok=s->x.assigned() && s->result.assigned() &&
        (s->x.val() == s->result.val()) &&
        (Gecode::PropagatorGroup::all.size(*s) == 0);
      delete s;
      if (!ok) return false;
      solutions++;
    }
    if (solutions != 8U) return false;

    ResultAliasGcdSpace* aroot=new ResultAliasGcdSpace;
    Gecode::DFS<ResultAliasGcdSpace> asearch(aroot);
    delete aroot;
    solutions=0;
    while (ResultAliasGcdSpace* s=asearch.next()) {
      const bool ok=gcd_value(s->x.val(),s->y.val()) == s->x.val();
      delete s;
      if (!ok) return false;
      solutions++;
    }
    if (solutions != 24U) return false;

    ReGcdSpace* rroot=new ReGcdSpace;
    Gecode::DFS<ReGcdSpace> rsearch(rroot);
    delete rroot;
    solutions=0;
    while (ReGcdSpace* s=rsearch.next()) {
      const bool expected=s->result.val() == gcd_value(s->x.val(),s->y.val());
      const bool ok=s->b.assigned() &&
        (static_cast<bool>(s->b.val()) == expected) &&
        (Gecode::PropagatorGroup::all.size(*s) == 0);
      delete s;
      if (!ok) return false;
      solutions++;
    }
    if (solutions != 512U) return false;
    return true;
  }

  class BoundSpace : public Gecode::Space {
  public:
    Gecode::WordVar x, y, result;
    Gecode::BoolVar b;
    BoundSpace(void)
      : x(*this,8,Gecode::WDT_UNSIGNED,5U,20U),
        y(*this,8,Gecode::WDT_UNSIGNED,7U,25U),
        result(*this,8,Gecode::WDT_UNSIGNED,6U,6U), b(*this,1,1) {}
    BoundSpace(BoundSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x); y.update(*this,s.y); result.update(*this,s.result);
      b.update(*this,s.b);
    }
    virtual Gecode::Space* copy(void) { return new BoundSpace(*this); }
  };

  class SignedBoundSpace : public Gecode::Space {
  public:
    Gecode::WordVar divisor, dividend, minimum, zero, result;
    Gecode::BoolVar b;
    SignedBoundSpace(void)
      : divisor(*this,8,Gecode::WDT_SIGNED,250U,250U),
        dividend(*this,8,Gecode::WDT_SIGNED,236U,251U),
        minimum(*this,8,Gecode::WDT_SIGNED,128U,128U),
        zero(*this,8,Gecode::WDT_SIGNED,0U,0U),
        result(*this,8,Gecode::WDT_UNSIGNED,0U,255U), b(*this,1,1) {}
    SignedBoundSpace(SignedBoundSpace& s) : Gecode::Space(s) {
      divisor.update(*this,s.divisor); dividend.update(*this,s.dividend);
      minimum.update(*this,s.minimum); zero.update(*this,s.zero);
      result.update(*this,s.result);
      b.update(*this,s.b);
    }
    virtual Gecode::Space* copy(void) { return new SignedBoundSpace(*this); }
  };

  class SignedAliasGcdSpace : public Gecode::Space {
  public:
    Gecode::WordVar x, result;
    SignedAliasGcdSpace(WordValue lo, WordValue hi,
                        WordValue minimum, WordValue maximum)
      : x(*this,4,lo,hi,Gecode::WDT_SIGNED,minimum,maximum),
        result(*this,4,Gecode::WDT_UNSIGNED) {
      Gecode::signed_gcd(*this,x,x,result);
      Gecode::WordVarArgs words={x,result};
      Gecode::branch(*this,words,Gecode::WORD_VAR_SIZE_MIN(),
                     Gecode::WORD_VAL_LSB());
    }
    SignedAliasGcdSpace(SignedAliasGcdSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x); result.update(*this,s.result);
    }
    virtual Gecode::Space* copy(void) {
      return new SignedAliasGcdSpace(*this);
    }
  };

  class AlignedProgressionSpace : public Gecode::Space {
  public:
    Gecode::WordVar divisor, dividend, result, one, remainder;
    Gecode::IntVar modulus;
    Gecode::BoolVar divisible;
    AlignedProgressionSpace(void)
      : divisor(*this,8,Gecode::WDT_UNSIGNED,15U,15U),
        dividend(*this,8,0U,240U,Gecode::WDT_UNSIGNED,16U,240U),
        result(*this,8,Gecode::WDT_UNSIGNED,15U,15U),
        one(*this,8,Gecode::WDT_UNSIGNED,1U,1U),
        remainder(*this,8,Gecode::WDT_UNSIGNED,0U,0U),
        modulus(*this,15,15), divisible(*this,1,1) {
      Gecode::divides(*this,divisor,dividend,Gecode::Reify(divisible));
      Gecode::gcd(*this,divisor,dividend,result);
      Gecode::product_mod(*this,one,dividend,modulus,remainder);
    }
    AlignedProgressionSpace(AlignedProgressionSpace& s) : Gecode::Space(s) {
      divisor.update(*this,s.divisor); dividend.update(*this,s.dividend);
      result.update(*this,s.result); one.update(*this,s.one);
      remainder.update(*this,s.remainder); modulus.update(*this,s.modulus);
      divisible.update(*this,s.divisible);
    }
    virtual Gecode::Space* copy(void) {
      return new AlignedProgressionSpace(*this);
    }
  };

  class IncompatibleProgressionSpace : public Gecode::Space {
  public:
    Gecode::WordVar divisor, dividend;
    Gecode::BoolVar divisible;
    IncompatibleProgressionSpace(void)
      : divisor(*this,8,Gecode::WDT_UNSIGNED,6U,6U),
        dividend(*this,8,1U,255U,Gecode::WDT_UNSIGNED,1U,255U),
        divisible(*this,1,1) {
      Gecode::divides(*this,divisor,dividend,Gecode::Reify(divisible));
    }
    IncompatibleProgressionSpace(IncompatibleProgressionSpace& s)
      : Gecode::Space(s) {
      divisor.update(*this,s.divisor); dividend.update(*this,s.dividend);
      divisible.update(*this,s.divisible);
    }
    virtual Gecode::Space* copy(void) {
      return new IncompatibleProgressionSpace(*this);
    }
  };

  static bool
  signed_alias_results(WordValue lo, WordValue hi,
                       WordValue minimum, WordValue maximum,
                       WordValue expected) {
    SignedAliasGcdSpace* root=
      new SignedAliasGcdSpace(lo,hi,minimum,maximum);
    Gecode::DFS<SignedAliasGcdSpace> search(root);
    delete root;
    WordValue actual=0U;
    while (SignedAliasGcdSpace* s=search.next()) {
      if (!s->result.assigned() || (s->result.val() >= 16U)) {
        delete s;
        return false;
      }
      actual |= WordValue(1) << s->result.val();
      delete s;
    }
    return actual == expected;
  }

  static bool
  bounded(void) {
    if (!progression_oracle()) return false;
    const WordValue odd_magnitudes=(WordValue(1) << 1) |
      (WordValue(1) << 3) | (WordValue(1) << 5) |
      (WordValue(1) << 7);
    const WordValue zero_through_four=(WordValue(1) << 0) |
      (WordValue(1) << 1) | (WordValue(1) << 2) |
      (WordValue(1) << 3) | (WordValue(1) << 4);
    if (!signed_alias_results(1U,15U,9U,7U,odd_magnitudes) ||
        !signed_alias_results(0U,15U,13U,4U,zero_through_four) ||
        !signed_alias_results(1U,15U,9U,15U,odd_magnitudes) ||
        !signed_alias_results(1U,15U,1U,7U,odd_magnitudes))
      return false;

    AlignedProgressionSpace aligned;
    if ((aligned.status() == Gecode::SS_FAILED) ||
        !aligned.dividend.assigned() || (aligned.dividend.val() != 240U) ||
        (Gecode::PropagatorGroup::all.size(aligned) != 0))
      return false;
    IncompatibleProgressionSpace incompatible;
    if (incompatible.status() != Gecode::SS_FAILED)
      return false;

    BoundSpace gcd;
    Gecode::gcd(gcd,gcd.x,gcd.y,gcd.result);
    if ((gcd.status() == Gecode::SS_FAILED) ||
        (gcd.x.minimum() != 6U) || (gcd.x.maximum() != 18U) ||
        (gcd.y.minimum() != 12U) || (gcd.y.maximum() != 24U) ||
        ((gcd.x.hi() & 1U) != 0U) || ((gcd.y.hi() & 1U) != 0U))
      return false;

    BoundSpace divides;
    Gecode::dom(divides,divides.x,6U);
    Gecode::divides(divides,divides.x,divides.y,
                    Gecode::Reify(divides.b));
    if ((divides.status() == Gecode::SS_FAILED) ||
        (divides.y.minimum() != 12U) || (divides.y.maximum() != 24U) ||
        ((divides.y.hi() & 1U) != 0U))
      return false;

    SignedBoundSpace signed_divides;
    Gecode::signed_divides(signed_divides,signed_divides.divisor,
      signed_divides.dividend,Gecode::Reify(signed_divides.b));
    if ((signed_divides.status() == Gecode::SS_FAILED) ||
        (signed_divides.dividend.minimum() != 238U) ||
        (signed_divides.dividend.maximum() != 250U))
      return false;

    SignedBoundSpace signed_gcd;
    Gecode::signed_gcd(signed_gcd,signed_gcd.minimum,signed_gcd.zero,
                       signed_gcd.result);
    return (signed_gcd.status() != Gecode::SS_FAILED) &&
      signed_gcd.result.assigned() && (signed_gcd.result.val() == 128U);
  }

  class Lifecycle : public Base {
  public:
    Lifecycle(void) : Base("Word::Number::Lifecycle") {}
    virtual bool run(void) { return exhaustive() && bounded(); }
  };

  Lifecycle lifecycle;

}}}

// STATISTICS: test-word
