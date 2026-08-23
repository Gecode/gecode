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
#include <gecode/word/rel.hh>
#include <gecode/word/structure.hh>

namespace Gecode {

  namespace {
    void check_widths(WordVar x, WordVar y, WordVar result,
                      const char* location) {
      if ((x.width() != y.width()) || (x.width() != result.width()))
        throw Word::WidthMismatch(location);
    }

    WordValue checked_value(unsigned int width, WordValue value) {
      Word::ConstWordView c(width,value);
      return c.val();
    }

    bool matching_bounded(WordVar x, WordVar y, WordVar z,
                          WordDomainType& kind) {
      kind=x.domain_type();
      return (kind != WDT_CUBE) && (y.domain_type() == kind) &&
        (z.domain_type() == kind);
    }

    WordVar assigned_for(Home home, unsigned int width, WordValue value,
                         WordVar x, WordVar result) {
      const WordDomainType kind=(x.domain_type() == result.domain_type()) ?
        x.domain_type() : WDT_CUBE;
      return kind == WDT_CUBE ? WordVar(home,width,value,value) :
        WordVar(home,width,kind,value,value);
    }

    void post_add(Home home, WordVar x, WordVar y, WordVar result) {
      WordDomainType kind;
      if (matching_bounded(x,y,result,kind)) {
        if ((kind == WDT_UNSIGNED) &&
            Word::Arithmetic::BoundArithmetic<Word::UnsignedWordView,
              Word::Arithmetic::BA_ADD>::numeric_regime(
                Word::UnsignedWordView(x),Word::UnsignedWordView(y)))
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::UnsignedWordView,Word::Arithmetic::BA_ADD>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
        else if ((kind == WDT_SIGNED) &&
                 Word::Arithmetic::BoundArithmetic<Word::SignedWordView,
                   Word::Arithmetic::BA_ADD>::numeric_regime(
                     Word::SignedWordView(x),Word::SignedWordView(y)))
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::SignedWordView,Word::Arithmetic::BA_ADD>::post(
              home,Word::SignedWordView(x),Word::SignedWordView(y),
              Word::SignedWordView(result))));
        else
          goto cube_add;
        return;
      }
    cube_add:
      GECODE_ES_FAIL(Word::Arithmetic::Add::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result)));
    }

    void post_nary_add(Home home, const WordVarArgs& input,
                       WordVar result) {
      WordValue constant=0;
      ViewArray<Word::WordView> x(home,input.size());
      int n=0;
      for (int i=0; i<input.size(); i++) {
        Word::WordView next(input[i]);
        if (next.assigned())
          constant += next.val();
        else
          x[n++]=next;
      }
      constant &= result.mask();
      x.size(n);
      if (x.size() == 0) {
        GECODE_ME_FAIL(Word::WordView(result).eq(home,constant));
      } else if ((x.size() == 1) && (constant == 0)) {
        GECODE_ES_FAIL((Word::Rel::Eq<
          Word::WordView,Word::WordView>::post(
            home,x[0],Word::WordView(result))));
      } else {
        const WordDomainType kind=result.domain_type();
        bool bounded=(kind != WDT_CUBE);
        for (int i=0; bounded && (i<input.size()); i++)
          bounded = input[i].domain_type() == kind;
        if (bounded && (kind == WDT_UNSIGNED)) {
          WordValue maximum=0;
          for (int i=0; bounded && (i<input.size()); i++)
            if (input[i].assigned()) {
              Word::UnsignedWordView v(input[i]);
              bounded = v.val() <= result.mask()-maximum;
              if (bounded) maximum += v.val();
            }
          for (int i=0; bounded && (i<x.size()); i++) {
            Word::UnsignedWordView v(x[i].varimp());
            bounded = v.rank_maximum() <= result.mask()-maximum;
            if (bounded) maximum += v.rank_maximum();
          }
          if (bounded) {
            ViewArray<Word::UnsignedWordView> bx(home,x.size());
            for (int i=0; i<x.size(); i++)
              bx[i]=Word::UnsignedWordView(x[i].varimp());
            GECODE_ES_FAIL((Word::Arithmetic::BoundNaryAdd<
              Word::UnsignedWordView>::post(
                home,bx,Word::UnsignedWordView(result),constant)));
            return;
          }
        } else if (bounded && (kind == WDT_SIGNED)) {
          const WordValue sign=Word::sign_bit(result.width());
          WordValue minimum=sign, maximum=sign;
          for (int i=0; bounded && (i<input.size()); i++)
            if (input[i].assigned()) {
              Word::SignedWordView v(input[i]);
              WordValue next;
              bounded = Word::Arithmetic::bound_signed_add(
                minimum,Word::rank(kind,result.width(),v.val()),sign,
                result.mask(),next);
              if (bounded) minimum=maximum=next;
            }
          for (int i=0; bounded && (i<x.size()); i++) {
            Word::SignedWordView v(x[i].varimp());
            WordValue next_minimum, next_maximum;
            bounded = Word::Arithmetic::bound_signed_add(
              minimum,v.rank_minimum(),sign,result.mask(),next_minimum) &&
              Word::Arithmetic::bound_signed_add(
                maximum,v.rank_maximum(),sign,result.mask(),next_maximum);
            if (bounded) {
              minimum=next_minimum; maximum=next_maximum;
            }
          }
          if (bounded) {
            ViewArray<Word::SignedWordView> bx(home,x.size());
            for (int i=0; i<x.size(); i++)
              bx[i]=Word::SignedWordView(x[i].varimp());
            GECODE_ES_FAIL((Word::Arithmetic::BoundNaryAdd<
              Word::SignedWordView>::post(
                home,bx,Word::SignedWordView(result),constant)));
            return;
          }
        }
        GECODE_ES_FAIL(Word::Arithmetic::NaryAdd::post(
          home,x,Word::WordView(result),constant));
      }
    }

    void post_add_carry(Home home, WordVar x, WordVar y, WordVar result,
                        BoolVar carry) {
      if ((x.domain_type() == WDT_UNSIGNED) &&
          (y.domain_type() == WDT_UNSIGNED) &&
          (result.domain_type() == WDT_UNSIGNED)) {
        if (carry.zero()) {
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::UnsignedWordView,Word::Arithmetic::BA_ADD,
            Word::Arithmetic::BT_CLEAR>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
          return;
        }
        if (carry.one()) {
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::UnsignedWordView,Word::Arithmetic::BA_ADD,
            Word::Arithmetic::BT_SET>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
          return;
        }
        if (!Word::Arithmetic::BoundFlagArithmetic<
              Word::UnsignedWordView,
              Word::Arithmetic::BA_ADD>::numeric_regime(
                Word::UnsignedWordView(x),Word::UnsignedWordView(y),
                Int::BoolView(carry)))
          goto cube_add_carry;
        GECODE_ES_FAIL((Word::Arithmetic::BoundFlagArithmetic<
          Word::UnsignedWordView,Word::Arithmetic::BA_ADD>::post(
            home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
            Word::UnsignedWordView(result),Int::BoolView(carry))));
        return;
      }
    cube_add_carry:
      GECODE_ES_FAIL(Word::Arithmetic::AddCarry::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result),Int::BoolView(carry)));
    }

    void post_neg(Home home, WordVar x, WordVar result) {
      if ((x.domain_type() == WDT_SIGNED) &&
          (result.domain_type() == WDT_SIGNED) &&
          Word::Arithmetic::BoundNeg<Word::SignedWordView>::numeric_regime(
            Word::SignedWordView(x))) {
        GECODE_ES_FAIL((Word::Arithmetic::BoundNeg<
          Word::SignedWordView>::post(home,Word::SignedWordView(x),
                                     Word::SignedWordView(result))));
        return;
      }
      GECODE_ES_FAIL(Word::Arithmetic::Neg::post(
                       home,Word::WordView(x),Word::WordView(result)));
    }

    void post_sub(Home home, WordVar x, WordVar y, WordVar result) {
      WordDomainType kind;
      if (matching_bounded(x,y,result,kind)) {
        if ((kind == WDT_UNSIGNED) &&
            Word::Arithmetic::BoundArithmetic<Word::UnsignedWordView,
              Word::Arithmetic::BA_SUB>::numeric_regime(
                Word::UnsignedWordView(x),Word::UnsignedWordView(y)))
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::UnsignedWordView,Word::Arithmetic::BA_SUB>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
        else if ((kind == WDT_SIGNED) &&
                 Word::Arithmetic::BoundArithmetic<Word::SignedWordView,
                   Word::Arithmetic::BA_SUB>::numeric_regime(
                     Word::SignedWordView(x),Word::SignedWordView(y)))
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::SignedWordView,Word::Arithmetic::BA_SUB>::post(
              home,Word::SignedWordView(x),Word::SignedWordView(y),
              Word::SignedWordView(result))));
        else
          goto cube_sub;
        return;
      }
    cube_sub:
      GECODE_ES_FAIL(Word::Arithmetic::Sub::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result)));
    }

    void post_sub_borrow(Home home, WordVar x, WordVar y, WordVar result,
                         BoolVar borrow) {
      if ((x.domain_type() == WDT_UNSIGNED) &&
          (y.domain_type() == WDT_UNSIGNED) &&
          (result.domain_type() == WDT_UNSIGNED)) {
        if (borrow.zero()) {
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::UnsignedWordView,Word::Arithmetic::BA_SUB,
            Word::Arithmetic::BT_CLEAR>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
          return;
        }
        if (borrow.one()) {
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::UnsignedWordView,Word::Arithmetic::BA_SUB,
            Word::Arithmetic::BT_SET>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
          return;
        }
        if (!Word::Arithmetic::BoundFlagArithmetic<
              Word::UnsignedWordView,
              Word::Arithmetic::BA_SUB>::numeric_regime(
                Word::UnsignedWordView(x),Word::UnsignedWordView(y),
                Int::BoolView(borrow)))
          goto cube_sub_borrow;
        GECODE_ES_FAIL((Word::Arithmetic::BoundFlagArithmetic<
          Word::UnsignedWordView,Word::Arithmetic::BA_SUB>::post(
            home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
            Word::UnsignedWordView(result),Int::BoolView(borrow))));
        return;
      }
    cube_sub_borrow:
      GECODE_ES_FAIL(Word::Arithmetic::SubBorrow::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result),Int::BoolView(borrow)));
    }

    void post_mult(Home home, WordVar x, WordVar y, WordVar result) {
      WordDomainType kind;
      if (matching_bounded(x,y,result,kind)) {
        if ((kind == WDT_UNSIGNED) &&
            Word::Arithmetic::BoundArithmetic<Word::UnsignedWordView,
              Word::Arithmetic::BA_MULT>::numeric_regime(
                Word::UnsignedWordView(x),Word::UnsignedWordView(y)))
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::UnsignedWordView,Word::Arithmetic::BA_MULT>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
        else if ((kind == WDT_SIGNED) &&
                 Word::Arithmetic::BoundArithmetic<Word::SignedWordView,
                   Word::Arithmetic::BA_MULT>::numeric_regime(
                     Word::SignedWordView(x),Word::SignedWordView(y)))
          GECODE_ES_FAIL((Word::Arithmetic::BoundArithmetic<
            Word::SignedWordView,Word::Arithmetic::BA_MULT>::post(
              home,Word::SignedWordView(x),Word::SignedWordView(y),
              Word::SignedWordView(result))));
        else
          goto cube_mult;
        return;
      }
    cube_mult:
      GECODE_ES_FAIL(Word::Arithmetic::Mult::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result)));
    }

    void check_semantics(WordSemantics semantics, const char* location) {
      switch (semantics) {
      case WS_SMTLIB: return;
      default: throw Word::UnknownOperation(location);
      }
    }

    void post_divmod(Home home, WordVar x, WordVar y, WordVar result,
                     bool quotient) {
      if ((x.domain_type() == WDT_UNSIGNED) &&
          (y.domain_type() == WDT_UNSIGNED) &&
          (result.domain_type() == WDT_UNSIGNED) &&
          Word::Arithmetic::BoundUnsignedDivMod<
            Word::Arithmetic::BUD_DIV>::numeric_regime(
              Word::UnsignedWordView(y))) {
        if (quotient)
          GECODE_ES_FAIL((Word::Arithmetic::BoundUnsignedDivMod<
            Word::Arithmetic::BUD_DIV>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
        else
          GECODE_ES_FAIL((Word::Arithmetic::BoundUnsignedDivMod<
            Word::Arithmetic::BUD_MOD>::post(
              home,Word::UnsignedWordView(x),Word::UnsignedWordView(y),
              Word::UnsignedWordView(result))));
        return;
      }
      if (quotient)
        GECODE_ES_FAIL(Word::Arithmetic::Div::post(
          home,Word::WordView(x),Word::WordView(y),Word::WordView(result)));
      else
        GECODE_ES_FAIL(Word::Arithmetic::Mod::post(
          home,Word::WordView(x),Word::WordView(y),Word::WordView(result)));
    }

    void post_absolute(Home home, WordVar x, BoolVar negative,
                       WordVar magnitude) {
      WordVar negative_x(home,x.width());
      post_neg(home,x,negative_x);
      ite(home,negative,negative_x,x,magnitude);
    }

    void post_signed_divmod(Home home, WordVar x, WordVar y,
                            WordVar result, bool quotient) {
      if ((x.domain_type() == WDT_SIGNED) &&
          (y.domain_type() == WDT_SIGNED) &&
          (result.domain_type() == WDT_SIGNED) &&
          Word::Arithmetic::BoundSignedDivMod<
            Word::Arithmetic::SDO_DIV>::numeric_regime(
              Word::SignedWordView(y))) {
        if (quotient)
          GECODE_ES_FAIL((Word::Arithmetic::BoundSignedDivMod<
            Word::Arithmetic::SDO_DIV>::post(
              home,Word::SignedWordView(x),Word::SignedWordView(y),
              Word::SignedWordView(result))));
        else
          GECODE_ES_FAIL((Word::Arithmetic::BoundSignedDivMod<
            Word::Arithmetic::SDO_REM>::post(
              home,Word::SignedWordView(x),Word::SignedWordView(y),
              Word::SignedWordView(result))));
        return;
      }
      if (quotient)
        GECODE_ES_FAIL((Word::Arithmetic::SignedDivMod<
          Word::Arithmetic::SDO_DIV>::post(
            home,Word::WordView(x),Word::WordView(y),
            Word::WordView(result))));
      else
        GECODE_ES_FAIL((Word::Arithmetic::SignedDivMod<
          Word::Arithmetic::SDO_REM>::post(
            home,Word::WordView(x),Word::WordView(y),
            Word::WordView(result))));
    }

    void post_signed_mod(Home home, WordVar x, WordVar y, WordVar result) {
      if ((x.domain_type() == WDT_SIGNED) &&
          (y.domain_type() == WDT_SIGNED) &&
          (result.domain_type() == WDT_SIGNED) &&
          Word::Arithmetic::BoundSignedDivMod<
            Word::Arithmetic::SDO_MOD>::numeric_regime(
              Word::SignedWordView(y))) {
        GECODE_ES_FAIL((Word::Arithmetic::BoundSignedDivMod<
          Word::Arithmetic::SDO_MOD>::post(
            home,Word::SignedWordView(x),Word::SignedWordView(y),
            Word::SignedWordView(result))));
        return;
      }
      GECODE_ES_FAIL((Word::Arithmetic::SignedDivMod<
        Word::Arithmetic::SDO_MOD>::post(
          home,Word::WordView(x),Word::WordView(y),Word::WordView(result))));
    }

    void post_signed_add_overflow(Home home, WordVar x, WordVar y,
                                  BoolVar overflow) {
      const unsigned int width = x.width();
      WordVar result=(x.domain_type() == WDT_SIGNED &&
                      y.domain_type() == WDT_SIGNED) ?
        WordVar(home,width,WDT_SIGNED) : WordVar(home,width);
      post_add(home,x,y,result);
      BoolVar x_sign(home,0,1), y_sign(home,0,1), result_sign(home,0,1);
      channel(home,x,width-1,x_sign);
      channel(home,y,width-1,y_sign);
      channel(home,result,width-1,result_sign);
      BoolVar same_operands(home,0,1), changed_sign(home,0,1);
      rel(home,x_sign,BOT_EQV,y_sign,same_operands);
      rel(home,x_sign,BOT_XOR,result_sign,changed_sign);
      rel(home,same_operands,BOT_AND,changed_sign,overflow);
    }

    void post_unsigned_mult_overflow(Home home, WordVar x, WordVar y,
                                     BoolVar overflow) {
      const unsigned int width = x.width();
      const bool bounded=(x.domain_type() == WDT_UNSIGNED) &&
        (y.domain_type() == WDT_UNSIGNED);
      WordVar maximum=bounded ?
        WordVar(home,width,WDT_UNSIGNED,Word::width_mask(width),
                Word::width_mask(width)) :
        WordVar(home,width,Word::width_mask(width),Word::width_mask(width));
      WordVar quotient=bounded ? WordVar(home,width,WDT_UNSIGNED) :
        WordVar(home,width);
      post_divmod(home,maximum,x,quotient,true);
      rel(home,y,WRT_UGR,quotient,Reify(overflow,RM_EQV));
    }

    void post_signed_mult_overflow(Home home, WordVar x, WordVar y,
                                   BoolVar overflow) {
      const unsigned int width = x.width();
      const WordValue sign = WordValue(1) << (width-1);
      BoolVar x_negative(home,0,1), y_negative(home,0,1);
      channel(home,x,width-1,x_negative);
      channel(home,y,width-1,y_negative);
      WordVar x_magnitude(home,width), y_magnitude(home,width);
      post_absolute(home,x,x_negative,x_magnitude);
      post_absolute(home,y,y_negative,y_magnitude);

      BoolVar negative(home,0,1);
      rel(home,x_negative,BOT_XOR,y_negative,negative);
      WordVar positive_limit(home,width,sign-1,sign-1);
      WordVar negative_limit(home,width,sign,sign);
      WordVar limit(home,width);
      ite(home,negative,negative_limit,positive_limit,limit);
      WordVar quotient(home,width);
      post_divmod(home,limit,x_magnitude,quotient,true);
      rel(home,y_magnitude,WRT_UGR,quotient,Reify(overflow,RM_EQV));
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
      GECODE_ES_FAIL((Word::Rel::Eq<
        Word::WordView,Word::WordView>::post(
          home,Word::WordView(x[0]),Word::WordView(result))));
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
    if ((x.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::add");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar y=assigned_for(home,width,value,x,result);
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
    value = checked_value(width,value);
    GECODE_POST;
    WordVar x(home,width,value,value);
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
    if ((x.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::sub");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar y=assigned_for(home,width,value,x,result);
    post_sub(home,x,y,result);
  }

  void
  sub(Home home, unsigned int width, WordValue value, WordVar y,
      WordVar result) {
    if ((y.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::sub");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar x=assigned_for(home,width,value,y,result);
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
    if ((x.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::mult");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar y=assigned_for(home,width,value,x,result);
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
      WordVar result=(x.domain_type() == WDT_UNSIGNED &&
                      y.domain_type() == WDT_UNSIGNED) ?
        WordVar(home,x.width(),WDT_UNSIGNED) : WordVar(home,x.width());
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
    post_divmod(home,x,y,result,true);
  }

  void
  div(Home home, WordVar x, unsigned int width, WordValue value,
      WordVar result, WordSemantics semantics) {
    if ((x.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::div");
    check_semantics(semantics,"Word::div");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar y=assigned_for(home,width,value,x,result);
    post_divmod(home,x,y,result,true);
  }

  void
  div(Home home, unsigned int width, WordValue value, WordVar y,
      WordVar result, WordSemantics semantics) {
    if ((y.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::div");
    check_semantics(semantics,"Word::div");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar x=assigned_for(home,width,value,y,result);
    post_divmod(home,x,y,result,true);
  }

  void
  mod(Home home, WordVar x, WordVar y, WordVar result,
      WordSemantics semantics) {
    check_widths(x,y,result,"Word::mod");
    check_semantics(semantics,"Word::mod");
    GECODE_POST;
    post_divmod(home,x,y,result,false);
  }

  void
  mod(Home home, WordVar x, unsigned int width, WordValue value,
      WordVar result, WordSemantics semantics) {
    if ((x.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::mod");
    check_semantics(semantics,"Word::mod");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar y=assigned_for(home,width,value,x,result);
    post_divmod(home,x,y,result,false);
  }

  void
  mod(Home home, unsigned int width, WordValue value, WordVar y,
      WordVar result, WordSemantics semantics) {
    if ((y.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::mod");
    check_semantics(semantics,"Word::mod");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar x=assigned_for(home,width,value,y,result);
    post_divmod(home,x,y,result,false);
  }

  void
  divmod(Home home, WordVar dividend, WordVar divisor, WordVar quotient,
         WordVar remainder, WordSemantics semantics) {
    if ((dividend.width() != divisor.width()) ||
        (dividend.width() != quotient.width()) ||
        (dividend.width() != remainder.width()))
      throw Word::WidthMismatch("Word::divmod");
    check_semantics(semantics,"Word::divmod");
    GECODE_POST;
    if ((dividend.domain_type() == WDT_UNSIGNED) &&
        (divisor.domain_type() == WDT_UNSIGNED) &&
        (quotient.domain_type() == WDT_UNSIGNED) &&
        (remainder.domain_type() == WDT_UNSIGNED) &&
        Word::Arithmetic::BoundUnsignedDivModBoth::numeric_regime(
          Word::UnsignedWordView(divisor))) {
      GECODE_ES_FAIL(Word::Arithmetic::BoundUnsignedDivModBoth::post(
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
    post_signed_divmod(home,x,y,result,true);
  }

  void
  signed_div(Home home, WordVar x, unsigned int width, WordValue value,
             WordVar result, WordSemantics semantics) {
    if ((x.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::signed_div");
    check_semantics(semantics,"Word::signed_div");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar y=assigned_for(home,width,value,x,result);
    post_signed_divmod(home,x,y,result,true);
  }

  void
  signed_div(Home home, unsigned int width, WordValue value, WordVar y,
             WordVar result, WordSemantics semantics) {
    if ((y.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::signed_div");
    check_semantics(semantics,"Word::signed_div");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar x=assigned_for(home,width,value,y,result);
    post_signed_divmod(home,x,y,result,true);
  }

  void
  signed_rem(Home home, WordVar x, WordVar y, WordVar result,
             WordSemantics semantics) {
    check_widths(x,y,result,"Word::signed_rem");
    check_semantics(semantics,"Word::signed_rem");
    GECODE_POST;
    post_signed_divmod(home,x,y,result,false);
  }

  void
  signed_rem(Home home, WordVar x, unsigned int width, WordValue value,
             WordVar result, WordSemantics semantics) {
    if ((x.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::signed_rem");
    check_semantics(semantics,"Word::signed_rem");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar y=assigned_for(home,width,value,x,result);
    post_signed_divmod(home,x,y,result,false);
  }

  void
  signed_rem(Home home, unsigned int width, WordValue value, WordVar y,
             WordVar result, WordSemantics semantics) {
    if ((y.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::signed_rem");
    check_semantics(semantics,"Word::signed_rem");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar x=assigned_for(home,width,value,y,result);
    post_signed_divmod(home,x,y,result,false);
  }

  void
  signed_mod(Home home, WordVar x, WordVar y, WordVar result,
             WordSemantics semantics) {
    check_widths(x,y,result,"Word::signed_mod");
    check_semantics(semantics,"Word::signed_mod");
    GECODE_POST;
    post_signed_mod(home,x,y,result);
  }

  void
  signed_mod(Home home, WordVar x, unsigned int width, WordValue value,
             WordVar result, WordSemantics semantics) {
    if ((x.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::signed_mod");
    check_semantics(semantics,"Word::signed_mod");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar y=assigned_for(home,width,value,x,result);
    post_signed_mod(home,x,y,result);
  }

  void
  signed_mod(Home home, unsigned int width, WordValue value, WordVar y,
             WordVar result, WordSemantics semantics) {
    if ((y.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::signed_mod");
    check_semantics(semantics,"Word::signed_mod");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar x=assigned_for(home,width,value,y,result);
    post_signed_mod(home,x,y,result);
  }

}

// STATISTICS: word-post
