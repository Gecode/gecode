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

}

// STATISTICS: word-post
