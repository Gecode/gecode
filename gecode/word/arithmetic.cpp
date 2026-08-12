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

    void post_add(Home home, WordVar x, WordVar y, WordVar result) {
      GECODE_ES_FAIL(Word::Arithmetic::Add::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result)));
    }

    void post_add_carry(Home home, WordVar x, WordVar y, WordVar result,
                        BoolVar carry) {
      GECODE_ES_FAIL(Word::Arithmetic::AddCarry::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result),Int::BoolView(carry)));
    }

    void post_neg(Home home, WordVar x, WordVar result) {
      GECODE_ES_FAIL(Word::Arithmetic::Neg::post(
                       home,Word::WordView(x),Word::WordView(result)));
    }

    void post_sub(Home home, WordVar x, WordVar y, WordVar result) {
      GECODE_ES_FAIL(Word::Arithmetic::Sub::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result)));
    }

    void post_sub_borrow(Home home, WordVar x, WordVar y, WordVar result,
                         BoolVar borrow) {
      GECODE_ES_FAIL(Word::Arithmetic::SubBorrow::post(
                       home,Word::WordView(x),Word::WordView(y),
                       Word::WordView(result),Int::BoolView(borrow)));
    }

    void post_mult(Home home, WordVar x, WordVar y, WordVar result) {
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
      GECODE_ES_FAIL((Word::Arithmetic::SignedDivMod<
        Word::Arithmetic::SDO_MOD>::post(
          home,Word::WordView(x),Word::WordView(y),Word::WordView(result))));
    }

    void post_signed_add_overflow(Home home, WordVar x, WordVar y,
                                  BoolVar overflow) {
      const unsigned int width = x.width();
      WordVar result(home,width);
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
      WordVar maximum(home,width,Word::width_mask(width),
                      Word::width_mask(width));
      WordVar quotient(home,width);
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
    WordVar y(home,width,value,value);
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
    WordVar y(home,width,value,value);
    post_sub(home,x,y,result);
  }

  void
  sub(Home home, unsigned int width, WordValue value, WordVar y,
      WordVar result) {
    if ((y.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::sub");
    value = checked_value(width,value);
    GECODE_POST;
    WordVar x(home,width,value,value);
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
    WordVar y(home,width,value,value);
    post_mult(home,x,y,result);
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
      WordVar result(home,x.width());
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
    WordVar y(home,width,value,value);
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
    WordVar x(home,width,value,value);
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
    WordVar y(home,width,value,value);
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
    WordVar x(home,width,value,value);
    post_divmod(home,x,y,result,false);
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
    WordVar y(home,width,value,value);
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
    WordVar x(home,width,value,value);
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
    WordVar y(home,width,value,value);
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
    WordVar x(home,width,value,value);
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
    WordVar y(home,width,value,value);
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
    WordVar x(home,width,value,value);
    post_signed_mod(home,x,y,result);
  }

}

// STATISTICS: word-post
