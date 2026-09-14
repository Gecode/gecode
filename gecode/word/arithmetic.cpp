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

#include <gecode/word/arithmetic.hh>
#include <gecode/word/arithmetic/rewriting.hpp>
#include <gecode/word/rel.hh>
#include <gecode/word/structure.hh>

namespace Gecode {

  namespace {
    void check_widths(WordVar x, WordVar y, WordVar result,
                      const char* location) {
      const bool has_matching_widths=(x.width() == y.width()) &&
        (x.width() == result.width());
      if (!has_matching_widths)
        throw Word::WidthMismatch(location);
    }

    WordValue validate_constant(unsigned int width, WordValue value) {
      Word::ConstWordView c(width,value);
      return c.val();
    }

    WordDomainType select_bounded_kind(WordVar x, WordVar y, WordVar result) {
      const WordDomainType kind=x.domain_type();
      const bool has_matching_bounds=(kind != WDT_CUBE) &&
        (y.domain_type() == kind) && (result.domain_type() == kind);
      return has_matching_bounds ? kind : WDT_CUBE;
    }

    WordVar make_constant(Home home, unsigned int width, WordValue value,
                         WordVar x, WordVar result) {
      const WordDomainType kind=(x.domain_type() == result.domain_type()) ?
        x.domain_type() : WDT_CUBE;
      return kind == WDT_CUBE ? WordVar(home,width,value,value) :
        WordVar(home,width,kind,value,value);
    }

    /// Select a representation and post the concrete binary arithmetic actor.
    template<Word::Arithmetic::BoundArithmeticOperation op, class CubeActor>
    void post_binary(Home home, WordVar x, WordVar y, WordVar result) {
      const WordDomainType kind=select_bounded_kind(x,y,result);
      if (kind == WDT_UNSIGNED) {
        GECODE_ES_FAIL((Word::Arithmetic::RewritingArithmetic<
          Word::UnsignedWordView,op>::post(home,Word::UnsignedWordView(x),
            Word::UnsignedWordView(y),Word::UnsignedWordView(result))));
      } else if (kind == WDT_SIGNED) {
        GECODE_ES_FAIL((Word::Arithmetic::RewritingArithmetic<
          Word::SignedWordView,op>::post(home,Word::SignedWordView(x),
            Word::SignedWordView(y),Word::SignedWordView(result))));
      } else {
        GECODE_ES_FAIL(CubeActor::post(
          home,Word::WordView(x),Word::WordView(y),Word::WordView(result)));
      }
    }

    void post_add(Home home, WordVar x, WordVar y, WordVar result) {
      post_binary<Word::Arithmetic::BA_ADD,Word::Arithmetic::Add>(home,x,y,result);
    }

    /// Remaining Space-owned views, canonical constant and original kind match.
    struct NaryPreparationResult {
      ViewArray<Word::WordView> views;
      WordValue constant;
      WordDomainType kind;
    };

    /// Fold constants while retaining compatibility of every original input.
    NaryPreparationResult
    prepare_nary_operands(Home home, const WordVarArgs& input, WordVar result) {
      WordValue constant=0;
      ViewArray<Word::WordView> views(home,input.size());
      const WordDomainType kind=result.domain_type();
      bool has_matching_bounds=kind != WDT_CUBE;
      int n=0;
      for (int i=0; i<input.size(); i++) {
        Word::WordView next(input[i]);
        if (next.assigned()) constant += next.val();
        else views[n++]=next;
        if (has_matching_bounds)
          has_matching_bounds=input[i].domain_type() == kind;
      }
      views.size(n);
      return {views,constant & result.mask(),has_matching_bounds ? kind : WDT_CUBE};
    }

    /// Reduce prepared operands before selecting the existing n-ary actor.
    void post_nary_add(Home home, const WordVarArgs& input, WordVar result) {
      NaryPreparationResult operands=prepare_nary_operands(home,input,result);
      if (operands.views.size() == 0) {
        GECODE_ME_FAIL(Word::WordView(result).eq(home,operands.constant));
        return;
      }
      const bool is_identity=(operands.views.size() == 1) &&
        (operands.constant == 0);
      if (is_identity) {
        rel(home,WordVar(operands.views[0].varimp()),WRT_EQ,result);
        return;
      }
      if (operands.kind == WDT_UNSIGNED) {
        ViewArray<Word::UnsignedWordView> views(home,operands.views.size());
        for (int i=0; i<views.size(); i++)
          views[i]=Word::UnsignedWordView(operands.views[i].varimp());
        GECODE_ES_FAIL((Word::Arithmetic::RewritingNaryAdd<Word::UnsignedWordView>::post(
          home,views,Word::UnsignedWordView(result),operands.constant)));
      } else if (operands.kind == WDT_SIGNED) {
        ViewArray<Word::SignedWordView> views(home,operands.views.size());
        for (int i=0; i<views.size(); i++)
          views[i]=Word::SignedWordView(operands.views[i].varimp());
        GECODE_ES_FAIL((Word::Arithmetic::RewritingNaryAdd<Word::SignedWordView>::post(
          home,views,Word::SignedWordView(result),operands.constant)));
      } else {
        GECODE_ES_FAIL(Word::Arithmetic::NaryAdd::post(
          home,operands.views,Word::WordView(result),operands.constant));
      }
    }

    /// Keep carry/borrow arity separate from ordinary binary posting.
    template<Word::Arithmetic::BoundArithmeticOperation op, class CubeActor>
    void post_flagged(Home home, WordVar x, WordVar y, WordVar result,
                     BoolVar flag) {
      const bool has_unsigned_operands=
        select_bounded_kind(x,y,result) == WDT_UNSIGNED;
      if (has_unsigned_operands) {
        GECODE_ES_FAIL((Word::Arithmetic::RewritingFlagArithmetic<op>::post(
          home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
          Word::UnsignedWordView(result),Int::BoolView(flag))));
        return;
      }
      GECODE_ES_FAIL(CubeActor::post(home,Word::WordView(x),Word::WordView(y),
        Word::WordView(result),Int::BoolView(flag)));
    }

    void post_add_carry(Home home, WordVar x, WordVar y, WordVar result,
                        BoolVar carry) {
      post_flagged<Word::Arithmetic::BA_ADD,Word::Arithmetic::AddCarry>(
        home,x,y,result,carry);
    }

    void post_neg(Home home, WordVar x, WordVar result) {
      const bool has_signed_operands=(x.domain_type() == WDT_SIGNED) &&
        (result.domain_type() == WDT_SIGNED);
      if (has_signed_operands) {
        GECODE_ES_FAIL(Word::Arithmetic::RewritingNeg::post(
          home,Word::SignedWordView(x),Word::SignedWordView(result)));
        return;
      }
      GECODE_ES_FAIL(Word::Arithmetic::Neg::post(home,Word::WordView(x),Word::WordView(result)));
    }

    void post_sub(Home home, WordVar x, WordVar y, WordVar result) {
      post_binary<Word::Arithmetic::BA_SUB,Word::Arithmetic::Sub>(home,x,y,result);
    }

    void post_sub_borrow(Home home, WordVar x, WordVar y, WordVar result,
                         BoolVar borrow) {
      post_flagged<Word::Arithmetic::BA_SUB,Word::Arithmetic::SubBorrow>(
        home,x,y,result,borrow);
    }

    void post_mult(Home home, WordVar x, WordVar y, WordVar result) {
      post_binary<Word::Arithmetic::BA_MULT,Word::Arithmetic::Mult>(home,x,y,result);
    }

    void check_semantics(WordSemantics semantics, const char* location) {
      switch (semantics) {
      case WS_SMTLIB: return;
      default: throw Word::UnknownOperation(location);
      }
    }

    /// Select an unsigned division actor while preserving SMT zero divisors.
    template<Word::Arithmetic::BoundUnsignedDivModOperation op>
    void post_divmod(Home home, WordVar x, WordVar y, WordVar result) {
      const bool has_unsigned_operands=
        select_bounded_kind(x,y,result) == WDT_UNSIGNED;
      if (has_unsigned_operands) {
        GECODE_ES_FAIL((Word::Arithmetic::RewritingUnsignedDivMod<op>::post(
          home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
          Word::UnsignedWordView(result))));
        return;
      }
      if (op == Word::Arithmetic::BUD_DIV)
        GECODE_ES_FAIL(Word::Arithmetic::Div::post(
          home,Word::WordView(x),Word::WordView(y),Word::WordView(result)));
      else
        GECODE_ES_FAIL(Word::Arithmetic::Mod::post(
          home,Word::WordView(x),Word::WordView(y),Word::WordView(result)));
    }

    void post_absolute(Home home, WordVar x, BoolVar negative,
                       WordVar magnitude) {
      WordVar negative_x(home,x.width(),x.domain_type());
      post_neg(home,x,negative_x);
      ite(home,negative,negative_x,x,magnitude);
    }

    /// Post signed quotient, remainder or modulus with the selected semantics.
    template<Word::Arithmetic::SignedDivModOperation op>
    void post_signed_divmod(Home home, WordVar x, WordVar y, WordVar result) {
      const bool has_signed_operands=
        select_bounded_kind(x,y,result) == WDT_SIGNED;
      if (has_signed_operands) {
        GECODE_ES_FAIL((Word::Arithmetic::RewritingSignedDivMod<op>::post(
          home,Word::SignedWordView(x),Word::SignedWordView(y),
          Word::SignedWordView(result))));
        return;
      }
      GECODE_ES_FAIL((Word::Arithmetic::SignedDivMod<op>::post(
        home,Word::WordView(x),Word::WordView(y),Word::WordView(result))));
    }

    void post_signed_add_overflow(Home home, WordVar x, WordVar y,
                                  BoolVar overflow) {
      const unsigned int width = x.width();
      WordVar result(home,width,(x.domain_type() == y.domain_type()) ?
        x.domain_type() : WDT_CUBE);
      post_add(home,x,y,result);
      BoolVar x_sign(home,0,1), y_sign(home,0,1), result_sign(home,0,1);
      channel(home,x,width-1,x_sign);
      channel(home,y,width-1,y_sign);
      channel(home,result,width-1,result_sign);
      BoolVar has_same_sign(home,0,1), changed_sign(home,0,1);
      rel(home,x_sign,BOT_EQV,y_sign,has_same_sign);
      rel(home,x_sign,BOT_XOR,result_sign,changed_sign);
      rel(home,has_same_sign,BOT_AND,changed_sign,overflow);
    }

    void post_unsigned_mult_overflow(Home home, WordVar x, WordVar y,
                                     BoolVar overflow) {
      const unsigned int width = x.width();
      const bool are_bounded=x.bounded() && y.bounded();
      WordVar maximum=are_bounded ?
        WordVar(home,width,WDT_UNSIGNED,Word::width_mask(width),
                Word::width_mask(width)) :
        WordVar(home,width,Word::width_mask(width),Word::width_mask(width));
      WordVar quotient=are_bounded ? WordVar(home,width,WDT_UNSIGNED) :
        WordVar(home,width);
      post_divmod<Word::Arithmetic::BUD_DIV>(home,maximum,x,quotient);
      rel(home,y,WRT_UGR,quotient,Reify(overflow,RM_EQV));
    }

    /// Magnitudes and the modeled sign of their product, owned by the Space.
    struct SignedMagnitudeResult {
      WordVar x, y;
      BoolVar negative;
    };

    /// Channel signs and select magnitudes before constructing product limits.
    SignedMagnitudeResult
    prepare_signed_magnitudes(Home home, WordVar x, WordVar y) {
      const unsigned int width=x.width();
      BoolVar x_negative(home,0,1), y_negative(home,0,1);
      channel(home,x,width-1,x_negative);
      channel(home,y,width-1,y_negative);
      const bool are_bounded=x.bounded() && y.bounded();
      const WordDomainType kind=are_bounded ? WDT_UNSIGNED : WDT_CUBE;
      WordVar x_magnitude(home,width,kind), y_magnitude(home,width,kind);
      post_absolute(home,x,x_negative,x_magnitude);
      post_absolute(home,y,y_negative,y_magnitude);
      BoolVar negative(home,0,1);
      rel(home,x_negative,BOT_XOR,y_negative,negative);
      return {x_magnitude,y_magnitude,negative};
    }

    /// Compare magnitudes with the sign-dependent representable product limit.
    void post_signed_mult_overflow(Home home, WordVar x, WordVar y,
                                   BoolVar overflow) {
      const unsigned int width=x.width();
      const WordValue sign=WordValue(1) << (width-1);
      const SignedMagnitudeResult magnitudes=prepare_signed_magnitudes(home,x,y);
      const WordDomainType kind=magnitudes.x.domain_type();
      WordVar positive_limit=make_constant(home,width,sign-1,magnitudes.x,magnitudes.x);
      WordVar negative_limit=make_constant(home,width,sign,magnitudes.x,magnitudes.x);
      WordVar limit(home,width,kind);
      ite(home,magnitudes.negative,negative_limit,positive_limit,limit);
      WordVar quotient(home,width,kind);
      post_divmod<Word::Arithmetic::BUD_DIV>(home,limit,magnitudes.x,quotient);
      rel(home,magnitudes.y,WRT_UGR,quotient,Reify(overflow,RM_EQV));
    }
  }

  void
  add(Home home, WordVar x, WordVar y, WordVar result) {
    check_widths(x,y,result,"Word::add");
    GECODE_POST;
    post_add(home,x,y,result);
  }

  void
  add(Home home, const WordVarArgs& x, WordVar result) {
    for (int i=0; i<x.size(); i++)
      if (x[i].width() != result.width())
        throw Word::WidthMismatch("Word::add");
    GECODE_POST;
    if (x.size() == 0) {
      GECODE_ME_FAIL(Word::WordView(result).eq(home,0));
    } else if (x.size() == 1) {
      rel(home,x[0],WRT_EQ,result);
    } else {
      post_nary_add(home,x,result);
    }
  }

  void
  add(Home home, WordVar x, WordVar y, WordVar result, BoolVar carry) {
    check_widths(x,y,result,"Word::add");
    GECODE_POST;
    post_add_carry(home,x,y,result,carry);
  }

  void
  add(Home home, WordVar x, unsigned int width, WordValue value,
      WordVar result) {
    const bool has_matching_widths=(x.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::add");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar y=make_constant(home,width,value,x,result);
    post_add(home,x,y,result);
  }

  void
  neg(Home home, WordVar x, WordVar result) {
    if (x.width() != result.width())
      throw Word::WidthMismatch("Word::neg");
    GECODE_POST;
    post_neg(home,x,result);
  }

  void
  neg(Home home, unsigned int width, WordValue value, WordVar result) {
    if (result.width() != width)
      throw Word::WidthMismatch("Word::neg");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar x=make_constant(home,width,value,result,result);
    post_neg(home,x,result);
  }

  void
  sub(Home home, WordVar x, WordVar y, WordVar result) {
    check_widths(x,y,result,"Word::sub");
    GECODE_POST;
    post_sub(home,x,y,result);
  }

  void
  sub(Home home, WordVar x, WordVar y, WordVar result, BoolVar borrow) {
    check_widths(x,y,result,"Word::sub");
    GECODE_POST;
    post_sub_borrow(home,x,y,result,borrow);
  }

  void
  sub(Home home, WordVar x, unsigned int width, WordValue value,
      WordVar result) {
    const bool has_matching_widths=(x.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::sub");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar y=make_constant(home,width,value,x,result);
    post_sub(home,x,y,result);
  }

  void
  sub(Home home, unsigned int width, WordValue value, WordVar y,
      WordVar result) {
    const bool has_matching_widths=(y.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::sub");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar x=make_constant(home,width,value,y,result);
    post_sub(home,x,y,result);
  }

  void
  mult(Home home, WordVar x, WordVar y, WordVar result) {
    check_widths(x,y,result,"Word::mult");
    GECODE_POST;
    post_mult(home,x,y,result);
  }

  void
  mult(Home home, WordVar x, unsigned int width, WordValue value,
       WordVar result) {
    const bool has_matching_widths=(x.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::mult");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar y=make_constant(home,width,value,x,result);
    post_mult(home,x,y,result);
  }

  void
  product_mod(Home home, WordVar x, WordVar y, IntVar modulus,
              WordVar result) {
    check_widths(x,y,result,"Word::product_mod");
    GECODE_POST;
    GECODE_ES_FAIL(Word::Arithmetic::post_product_mod(
      home,Word::WordView(x),Word::WordView(y),Int::IntView(modulus),
      Word::WordView(result)));
  }

  void
  product_mod(Home home, WordVar x, WordVar y, IntVar modulus,
              WordVar result, Reify r) {
    check_widths(x,y,result,"Word::product_mod");
    GECODE_POST;
    Word::WordView xv(x), yv(y), rv(result);
    Int::IntView mv(modulus);
    Int::BoolView bv(r.var());
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Word::Arithmetic::ReProductMod<RM_EQV>::post(
        home,xv,yv,mv,rv,bv)));
      break;
    case RM_IMP:
      GECODE_ES_FAIL((Word::Arithmetic::ReProductMod<RM_IMP>::post(
        home,xv,yv,mv,rv,bv)));
      break;
    case RM_PMI:
      GECODE_ES_FAIL((Word::Arithmetic::ReProductMod<RM_PMI>::post(
        home,xv,yv,mv,rv,bv)));
      break;
    default:
      throw Word::UnknownReifyMode("Word::product_mod");
    }
  }

  void
  gcd(Home home, WordVar x, WordVar y, WordVar result, IntPropLevel) {
    check_widths(x,y,result,"Word::gcd");
    GECODE_POST;
    GECODE_ES_FAIL((Word::Arithmetic::post_gcd<false>(
      home,Word::WordView(x),Word::WordView(y),Word::WordView(result))));
  }

  void
  gcd(Home home, WordVar x, WordVar y, WordVar result, Reify r,
      IntPropLevel) {
    check_widths(x,y,result,"Word::gcd");
    GECODE_POST;
    Word::WordView xv(x), yv(y), rv(result);
    Int::BoolView b(r.var());
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Word::Arithmetic::ReGcd<RM_EQV,false>::post(
        home,xv,yv,rv,b))); break;
    case RM_IMP:
      GECODE_ES_FAIL((Word::Arithmetic::ReGcd<RM_IMP,false>::post(
        home,xv,yv,rv,b))); break;
    case RM_PMI:
      GECODE_ES_FAIL((Word::Arithmetic::ReGcd<RM_PMI,false>::post(
        home,xv,yv,rv,b))); break;
    default: throw Word::UnknownReifyMode("Word::gcd");
    }
  }

  void
  signed_gcd(Home home, WordVar x, WordVar y, WordVar result,
             IntPropLevel) {
    check_widths(x,y,result,"Word::signed_gcd");
    GECODE_POST;
    GECODE_ES_FAIL((Word::Arithmetic::post_gcd<true>(
      home,Word::WordView(x),Word::WordView(y),Word::WordView(result))));
  }

  void
  signed_gcd(Home home, WordVar x, WordVar y, WordVar result, Reify r,
             IntPropLevel) {
    check_widths(x,y,result,"Word::signed_gcd");
    GECODE_POST;
    Word::WordView xv(x), yv(y), rv(result);
    Int::BoolView b(r.var());
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Word::Arithmetic::ReGcd<RM_EQV,true>::post(
        home,xv,yv,rv,b))); break;
    case RM_IMP:
      GECODE_ES_FAIL((Word::Arithmetic::ReGcd<RM_IMP,true>::post(
        home,xv,yv,rv,b))); break;
    case RM_PMI:
      GECODE_ES_FAIL((Word::Arithmetic::ReGcd<RM_PMI,true>::post(
        home,xv,yv,rv,b))); break;
    default: throw Word::UnknownReifyMode("Word::signed_gcd");
    }
  }

  void
  divides(Home home, WordVar divisor, WordVar dividend, Reify r,
          IntPropLevel) {
    if (divisor.width() != dividend.width())
      throw Word::WidthMismatch("Word::divides");
    GECODE_POST;
    Word::WordView dv(divisor), nv(dividend); Int::BoolView b(r.var());
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Word::Arithmetic::ReDivides<RM_EQV,false>::post(
        home,dv,nv,b))); break;
    case RM_IMP:
      GECODE_ES_FAIL((Word::Arithmetic::ReDivides<RM_IMP,false>::post(
        home,dv,nv,b))); break;
    case RM_PMI:
      GECODE_ES_FAIL((Word::Arithmetic::ReDivides<RM_PMI,false>::post(
        home,dv,nv,b))); break;
    default: throw Word::UnknownReifyMode("Word::divides");
    }
  }

  void
  signed_divides(Home home, WordVar divisor, WordVar dividend, Reify r,
                 IntPropLevel) {
    if (divisor.width() != dividend.width())
      throw Word::WidthMismatch("Word::signed_divides");
    GECODE_POST;
    Word::WordView dv(divisor), nv(dividend); Int::BoolView b(r.var());
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Word::Arithmetic::ReDivides<RM_EQV,true>::post(
        home,dv,nv,b))); break;
    case RM_IMP:
      GECODE_ES_FAIL((Word::Arithmetic::ReDivides<RM_IMP,true>::post(
        home,dv,nv,b))); break;
    case RM_PMI:
      GECODE_ES_FAIL((Word::Arithmetic::ReDivides<RM_PMI,true>::post(
        home,dv,nv,b))); break;
    default: throw Word::UnknownReifyMode("Word::signed_divides");
    }
  }

  void
  overflow(Home home, WordVar x, WordOverflowType wot, BoolVar b,
           WordSemantics semantics) {
    check_semantics(semantics,"Word::overflow");
    if (wot != WOF_NEG_SIGNED)
      throw Word::UnknownOperation("Word::overflow");
    GECODE_POST;
    const WordValue minimum = WordValue(1) << (x.width()-1);
    rel(home,x,WRT_EQ,x.width(),minimum,Reify(b,RM_EQV));
  }

  void
  overflow(Home home, WordVar x, WordOverflowType wot, WordVar y, BoolVar b,
           WordSemantics semantics) {
    if (x.width() != y.width())
      throw Word::WidthMismatch("Word::overflow");
    check_semantics(semantics,"Word::overflow");
    GECODE_POST;
    switch (wot) {
    case WOF_ADD_UNSIGNED: {
      WordVar result(home,x.width(),(x.domain_type() == y.domain_type()) ?
        x.domain_type() : WDT_CUBE);
      post_add_carry(home,x,y,result,b);
      break;
    }
    case WOF_ADD_SIGNED:
      post_signed_add_overflow(home,x,y,b);
      break;
    case WOF_MULT_UNSIGNED:
      post_unsigned_mult_overflow(home,x,y,b);
      break;
    case WOF_MULT_SIGNED:
      post_signed_mult_overflow(home,x,y,b);
      break;
    case WOF_DIV_SIGNED: {
      const unsigned int width = x.width();
      BoolVar minimum(home,0,1), minus_one(home,0,1);
      rel(home,x,WRT_EQ,width,WordValue(1) << (width-1),
          Reify(minimum,RM_EQV));
      rel(home,y,WRT_EQ,width,Word::width_mask(width),
          Reify(minus_one,RM_EQV));
      rel(home,minimum,BOT_AND,minus_one,b);
      break;
    }
    default:
      throw Word::UnknownOperation("Word::overflow");
    }
  }

  void
  div(Home home, WordVar x, WordVar y, WordVar result,
      WordSemantics semantics) {
    check_widths(x,y,result,"Word::div");
    check_semantics(semantics,"Word::div");
    GECODE_POST;
    post_divmod<Word::Arithmetic::BUD_DIV>(home,x,y,result);
  }

  void
  div(Home home, WordVar x, unsigned int width, WordValue value,
      WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(x.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::div");
    check_semantics(semantics,"Word::div");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar y=make_constant(home,width,value,x,result);
    post_divmod<Word::Arithmetic::BUD_DIV>(home,x,y,result);
  }

  void
  div(Home home, unsigned int width, WordValue value, WordVar y,
      WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(y.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::div");
    check_semantics(semantics,"Word::div");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar x=make_constant(home,width,value,y,result);
    post_divmod<Word::Arithmetic::BUD_DIV>(home,x,y,result);
  }

  void
  mod(Home home, WordVar x, WordVar y, WordVar result,
      WordSemantics semantics) {
    check_widths(x,y,result,"Word::mod");
    check_semantics(semantics,"Word::mod");
    GECODE_POST;
    post_divmod<Word::Arithmetic::BUD_MOD>(home,x,y,result);
  }

  void
  mod(Home home, WordVar x, unsigned int width, WordValue value,
      WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(x.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::mod");
    check_semantics(semantics,"Word::mod");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar y=make_constant(home,width,value,x,result);
    post_divmod<Word::Arithmetic::BUD_MOD>(home,x,y,result);
  }

  void
  mod(Home home, unsigned int width, WordValue value, WordVar y,
      WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(y.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::mod");
    check_semantics(semantics,"Word::mod");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar x=make_constant(home,width,value,y,result);
    post_divmod<Word::Arithmetic::BUD_MOD>(home,x,y,result);
  }

  void
  divmod(Home home, WordVar dividend, WordVar divisor, WordVar quotient,
         WordVar remainder, WordSemantics semantics) {
    const bool has_matching_widths=(dividend.width() == divisor.width()) &&
      (dividend.width() == quotient.width()) &&
      (dividend.width() == remainder.width());
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::divmod");
    check_semantics(semantics,"Word::divmod");
    GECODE_POST;
    const bool has_unsigned_operands=(dividend.domain_type() == WDT_UNSIGNED) &&
      (divisor.domain_type() == WDT_UNSIGNED) &&
      (quotient.domain_type() == WDT_UNSIGNED) &&
      (remainder.domain_type() == WDT_UNSIGNED);
    if (has_unsigned_operands) {
      GECODE_ES_FAIL(Word::Arithmetic::RewritingUnsignedDivModBoth::post(
        home,Word::UnsignedWordView(dividend),
        Word::UnsignedWordView(divisor),Word::UnsignedWordView(quotient),
        Word::UnsignedWordView(remainder)));
      return;
    }
    GECODE_ES_FAIL(Word::Arithmetic::DivModBoth::post(
      home,Word::WordView(dividend),Word::WordView(divisor),
      Word::WordView(quotient),Word::WordView(remainder)));
  }

  void
  signed_div(Home home, WordVar x, WordVar y, WordVar result,
             WordSemantics semantics) {
    check_widths(x,y,result,"Word::signed_div");
    check_semantics(semantics,"Word::signed_div");
    GECODE_POST;
    post_signed_divmod<Word::Arithmetic::SDO_DIV>(home,x,y,result);
  }

  void
  signed_div(Home home, WordVar x, unsigned int width, WordValue value,
             WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(x.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::signed_div");
    check_semantics(semantics,"Word::signed_div");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar y=make_constant(home,width,value,x,result);
    post_signed_divmod<Word::Arithmetic::SDO_DIV>(home,x,y,result);
  }

  void
  signed_div(Home home, unsigned int width, WordValue value, WordVar y,
             WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(y.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::signed_div");
    check_semantics(semantics,"Word::signed_div");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar x=make_constant(home,width,value,y,result);
    post_signed_divmod<Word::Arithmetic::SDO_DIV>(home,x,y,result);
  }

  void
  signed_rem(Home home, WordVar x, WordVar y, WordVar result,
             WordSemantics semantics) {
    check_widths(x,y,result,"Word::signed_rem");
    check_semantics(semantics,"Word::signed_rem");
    GECODE_POST;
    post_signed_divmod<Word::Arithmetic::SDO_REM>(home,x,y,result);
  }

  void
  signed_rem(Home home, WordVar x, unsigned int width, WordValue value,
             WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(x.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::signed_rem");
    check_semantics(semantics,"Word::signed_rem");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar y=make_constant(home,width,value,x,result);
    post_signed_divmod<Word::Arithmetic::SDO_REM>(home,x,y,result);
  }

  void
  signed_rem(Home home, unsigned int width, WordValue value, WordVar y,
             WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(y.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::signed_rem");
    check_semantics(semantics,"Word::signed_rem");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar x=make_constant(home,width,value,y,result);
    post_signed_divmod<Word::Arithmetic::SDO_REM>(home,x,y,result);
  }

  void
  signed_mod(Home home, WordVar x, WordVar y, WordVar result,
             WordSemantics semantics) {
    check_widths(x,y,result,"Word::signed_mod");
    check_semantics(semantics,"Word::signed_mod");
    GECODE_POST;
    post_signed_divmod<Word::Arithmetic::SDO_MOD>(home,x,y,result);
  }

  void
  signed_mod(Home home, WordVar x, unsigned int width, WordValue value,
             WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(x.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::signed_mod");
    check_semantics(semantics,"Word::signed_mod");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar y=make_constant(home,width,value,x,result);
    post_signed_divmod<Word::Arithmetic::SDO_MOD>(home,x,y,result);
  }

  void
  signed_mod(Home home, unsigned int width, WordValue value, WordVar y,
             WordVar result, WordSemantics semantics) {
    const bool has_matching_widths=(y.width() == width) &&
      (result.width() == width);
    if (!has_matching_widths)
      throw Word::WidthMismatch("Word::signed_mod");
    check_semantics(semantics,"Word::signed_mod");
    value = validate_constant(width,value);
    GECODE_POST;
    WordVar x=make_constant(home,width,value,y,result);
    post_signed_divmod<Word::Arithmetic::SDO_MOD>(home,x,y,result);
  }

}

// STATISTICS: word-post
