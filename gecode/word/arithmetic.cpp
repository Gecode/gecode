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

#include <gecode/word/logic.hh>
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

    /**
     * Wang et al.'s carry decomposition uses five ordinary intermediate
     * WordVars and six existing word-level propagators. The carry-in word has
     * bit zero fixed to zero; shifting the carry-out word discards overflow.
     */
    void post_add(Home home, WordVar x, WordVar y, WordVar result) {
      const unsigned int width = x.width();
      WordVar xor_xy(home,width);
      WordVar carry(home,width,0,Word::width_mask(width) & ~WordValue(1));
      WordVar both_one(home,width);
      WordVar carry_one(home,width);
      WordVar carry_out(home,width);
      rel(home,x,WOT_XOR,y,xor_xy);
      rel(home,xor_xy,WOT_XOR,carry,result);
      rel(home,x,WOT_AND,y,both_one);
      rel(home,carry,WOT_AND,xor_xy,carry_one);
      rel(home,both_one,WOT_OR,carry_one,carry_out);
      shift_left(home,carry_out,1,carry);
    }

    void post_neg(Home home, WordVar x, WordVar result) {
      WordVar complemented(home,x.width());
      complement(home,x,complemented);
      WordVar one(home,x.width(),1,1);
      post_add(home,complemented,one,result);
    }

    void post_sub(Home home, WordVar x, WordVar y, WordVar result) {
      WordVar negative_y(home,x.width());
      post_neg(home,y,negative_y);
      post_add(home,x,negative_y,result);
    }

    /**
     * The word-level schoolbook decomposition creates one conditional shifted
     * term per multiplier bit and folds the terms through modular addition.
     * Controls remain WordVars rather than a Boolean bit-blast. Each addition
     * contributes its five ordinary carry variables.
     */
    void post_mult(Home home, WordVar x, WordVar y, WordVar result) {
      const unsigned int width = x.width();
      WordVar zero(home,width,0,0);
      WordVar accumulator;
      for (unsigned int bit=0; bit<width; bit++) {
        WordVar control_bit(home,1);
        extract(home,y,bit,1,control_bit);
        WordVar control_mask(home,width);
        sign_extend(home,control_bit,width,control_mask);

        WordVar shifted(x);
        if (bit != 0) {
          shifted = WordVar(home,width);
          shift_left(home,x,bit,shifted);
        }

        WordVar term;
        if (width == 1)
          term = result;
        else
          term = WordVar(home,width);
        ite(home,control_mask,shifted,zero,term);

        if (bit == 0) {
          accumulator = term;
        } else if (bit+1 == width) {
          post_add(home,accumulator,term,result);
        } else {
          WordVar next(home,width);
          post_add(home,accumulator,term,next);
          accumulator = next;
        }
      }
    }

    void check_semantics(WordSemantics semantics, const char* location) {
      switch (semantics) {
      case WS_SMTLIB: return;
      default: throw Word::UnknownOperation(location);
      }
    }

    /**
     * Unsigned shift-subtract division compares the divisor with the
     * appropriately shifted current remainder. This avoids a width+1
     * temporary. A zero divisor selects every quotient bit while each
     * subtraction leaves the remainder unchanged, giving SMT-LIB's total
     * zero-divisor results without a separate case.
     */
    void post_divmod(Home home, WordVar x, WordVar y, WordVar result,
                     bool quotient) {
      const unsigned int width = x.width();
      WordVar remainder(x);
      for (unsigned int bit=width; bit-- > 0;) {
        WordVar shifted_remainder(remainder);
        if (bit != 0) {
          shifted_remainder = WordVar(home,width);
          logical_shift_right(home,remainder,bit,shifted_remainder);
        }

        BoolVar subtract(home,0,1);
        rel(home,y,WRT_ULQ,shifted_remainder,Reify(subtract,RM_EQV));
        if (quotient)
          channel(home,result,bit,subtract);

        WordVar shifted_divisor(y);
        if (bit != 0) {
          shifted_divisor = WordVar(home,width);
          shift_left(home,y,bit,shifted_divisor);
        }
        WordVar difference(home,width);
        post_sub(home,remainder,shifted_divisor,difference);

        WordVar next;
        if (!quotient && (bit == 0))
          next = result;
        else
          next = WordVar(home,width);
        ite(home,subtract,difference,remainder,next);
        remainder = next;
      }
    }

    void post_absolute(Home home, WordVar x, BoolVar negative,
                       WordVar magnitude) {
      WordVar negative_x(home,x.width());
      post_neg(home,x,negative_x);
      ite(home,negative,negative_x,x,magnitude);
    }

    void post_signed_divmod(Home home, WordVar x, WordVar y,
                            WordVar result, bool quotient) {
      const unsigned int width = x.width();
      BoolVar x_negative(home,0,1), y_negative(home,0,1);
      channel(home,x,width-1,x_negative);
      channel(home,y,width-1,y_negative);

      WordVar x_magnitude(home,width), y_magnitude(home,width);
      post_absolute(home,x,x_negative,x_magnitude);
      post_absolute(home,y,y_negative,y_magnitude);
      WordVar magnitude(home,width);
      post_divmod(home,x_magnitude,y_magnitude,magnitude,quotient);

      BoolVar result_negative(x_negative);
      if (quotient) {
        result_negative = BoolVar(home,0,1);
        rel(home,x_negative,BOT_XOR,y_negative,result_negative);
      }
      WordVar negative_magnitude(home,width);
      post_neg(home,magnitude,negative_magnitude);
      ite(home,result_negative,negative_magnitude,magnitude,result);
    }

    void post_signed_mod(Home home, WordVar x, WordVar y, WordVar result) {
      const unsigned int width = x.width();
      WordVar remainder(home,width);
      post_signed_divmod(home,x,y,remainder,false);

      BoolVar x_negative(home,0,1), y_negative(home,0,1);
      channel(home,x,width-1,x_negative);
      channel(home,y,width-1,y_negative);
      BoolVar signs_differ(home,0,1);
      rel(home,x_negative,BOT_XOR,y_negative,signs_differ);

      WordVar zero(home,width,0,0);
      BoolVar nonzero(home,0,1);
      rel(home,remainder,WRT_NQ,zero,Reify(nonzero,RM_EQV));
      BoolVar adjust(home,0,1);
      rel(home,signs_differ,BOT_AND,nonzero,adjust);

      WordVar adjusted(home,width);
      post_add(home,remainder,y,adjusted);
      ite(home,adjust,adjusted,remainder,result);
    }
  }

  void
  add(Home home, WordVar x, WordVar y, WordVar result) {
    check_widths(x,y,result,"Word::add");
    GECODE_POST;
    post_add(home,x,y,result);
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
