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
