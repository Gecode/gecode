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

#include <gecode/word/structure.hh>

namespace Gecode {

  namespace {
    void check_extract(unsigned int input_width, unsigned int first,
                       unsigned int width, unsigned int output_width,
                       const char* location) {
      if ((width == 0U) || (first >= input_width) ||
          (width > input_width-first))
        throw Word::OutOfLimits(location);
      if (output_width != width)
        throw Word::WidthMismatch(location);
    }

    void check_concat(unsigned int high_width, unsigned int low_width,
                      unsigned int result_width, const char* location) {
      if ((high_width > 64U-low_width) ||
          (result_width != high_width+low_width))
        throw Word::WidthMismatch(location);
    }

    void check_repeat(unsigned int input_width, unsigned int count,
                      unsigned int result_width, const char* location) {
      if ((count == 0U) || (count > 64U/input_width))
        throw Word::OutOfLimits(location);
      if (result_width != input_width*count)
        throw Word::WidthMismatch(location);
    }

    void check_extend(unsigned int input_width, unsigned int result_width,
                      unsigned int actual_width, const char* location) {
      if ((result_width == 0U) || (result_width > 64U) ||
          (result_width < input_width))
        throw Word::OutOfLimits(location);
      if (actual_width != result_width)
        throw Word::WidthMismatch(location);
    }

    void check_same_width(unsigned int input_width,
                          unsigned int result_width,
                          const char* location) {
      if (input_width != result_width)
        throw Word::WidthMismatch(location);
    }

    template<class View0, class View1>
    void post_fixed(Home home, View0 x, View1 result,
                    Word::Structure::FixedOp op,
                    unsigned int a=0, unsigned int b=0) {
      GECODE_ES_FAIL((Word::Structure::Fixed<View0,View1>
                      ::post(home,x,result,op,a,b)));
    }

    template<class View>
    bool post_bounded_shift_left(Home home, WordVar x, WordVar result,
                                 unsigned int amount) {
      View xv(x), rv(result);
      if (!Word::Structure::BoundedShiftLeft<View>::numeric_regime(
            xv,amount))
        return false;
      if (Word::Structure::BoundedShiftLeft<View>::post(
            home,xv,rv,amount) == ES_FAILED)
        home.fail();
      return true;
    }

    template<class View0, class View1, class View2>
    void post_concat(Home home, View0 high, View1 low, View2 result) {
      GECODE_ES_FAIL((Word::Structure::Concat<View0,View1,View2>
                      ::post(home,high,low,result)));
    }

    void post_fixed_word(Home home, WordVar x, unsigned int amount,
                         WordVar result, Word::Structure::FixedOp op) {
      Word::WordView xv(x), rv(result);
      if (xv == rv) {
        if (amount == 0U)
          return;
        WordVar temporary(home,x.width());
        post_fixed(home,xv,Word::WordView(temporary),op,amount);
        rel(home,temporary,WRT_EQ,result);
        return;
      }
      if ((x.domain_type() == WDT_UNSIGNED) &&
          (result.domain_type() == WDT_UNSIGNED) &&
          (Word::WordView(x) != Word::WordView(result)) &&
          (op == Word::Structure::FO_SHIFT_LEFT) &&
          post_bounded_shift_left<Word::UnsignedWordView>(
            home,x,result,amount))
        return;
      post_fixed(home,xv,rv,op,amount);
    }

    void post_variable_shift(Home home, WordVar x, WordVar amount,
                             WordVar result,
                             Word::Structure::FixedOp op) {
      Word::WordView xv(x), av(amount), rv(result);
      if ((xv == av) || (xv == rv) || (av == rv)) {
        WordVar x_proxy(home,x.width());
        WordVar amount_proxy(home,x.width());
        WordVar result_proxy(home,x.width());
        rel(home,x,WRT_EQ,x_proxy);
        rel(home,amount,WRT_EQ,amount_proxy);
        rel(home,result,WRT_EQ,result_proxy);
        GECODE_ES_FAIL((Word::Structure::VariableShift::post(
          home,Word::WordView(x_proxy),Word::WordView(amount_proxy),
          Word::WordView(result_proxy),op)));
        return;
      }
      if ((op == Word::Structure::FO_SHIFT_LEFT) &&
          (x.domain_type() == WDT_UNSIGNED) &&
          (amount.domain_type() == WDT_UNSIGNED) &&
          (result.domain_type() == WDT_UNSIGNED) &&
          (Word::WordView(x) != Word::WordView(amount)) &&
          (Word::WordView(x) != Word::WordView(result)) &&
          (Word::WordView(amount) != Word::WordView(result))) {
        Word::UnsignedWordView bx(x), ba(amount), br(result);
        if (Word::Structure::VariableShiftLeftBounds::numeric_regime(bx,ba)) {
          GECODE_ES_FAIL((Word::Structure::VariableShift::post(
            home,xv,av,rv,op)));
          GECODE_ES_FAIL((Word::Structure::VariableShiftLeftBounds::post(
            home,bx,ba,br)));
          return;
        }
      }
      GECODE_ES_FAIL((Word::Structure::VariableShift::post(
        home,xv,av,rv,op)));
    }
  }

  void
  extract(Home home, WordVar x, unsigned int first, unsigned int width,
          WordVar y) {
    check_extract(x.width(),first,width,y.width(),"Word::extract");
    GECODE_POST;
    if ((Word::WordView(x) == Word::WordView(y)) && (first == 0U))
      return;
    post_fixed(home,Word::WordView(x),Word::WordView(y),
               Word::Structure::FO_EXTRACT,first,width);
  }

  void
  extract(Home home, unsigned int input_width, WordValue value,
          unsigned int first, unsigned int width, WordVar y) {
    Word::ConstWordView x(input_width,value);
    check_extract(input_width,first,width,y.width(),"Word::extract");
    GECODE_POST;
    post_fixed(home,x,Word::WordView(y),Word::Structure::FO_EXTRACT,
               first,width);
  }

  void
  concat(Home home, WordVar high, WordVar low, WordVar result) {
    check_concat(high.width(),low.width(),result.width(),"Word::concat");
    GECODE_POST;
    if (Word::WordView(high) == Word::WordView(low)) {
      repeat(home,high,2,result);
      return;
    }
    post_concat(home,Word::WordView(high),Word::WordView(low),
                Word::WordView(result));
  }

  void
  concat(Home home, unsigned int high_width, WordValue high,
         WordVar low, WordVar result) {
    Word::ConstWordView h(high_width,high);
    check_concat(high_width,low.width(),result.width(),"Word::concat");
    GECODE_POST;
    post_concat(home,h,Word::WordView(low),Word::WordView(result));
  }

  void
  concat(Home home, WordVar high, unsigned int low_width, WordValue low,
         WordVar result) {
    Word::ConstWordView l(low_width,low);
    check_concat(high.width(),low_width,result.width(),"Word::concat");
    GECODE_POST;
    post_concat(home,Word::WordView(high),l,Word::WordView(result));
  }

  void
  repeat(Home home, WordVar x, unsigned int count, WordVar result) {
    check_repeat(x.width(),count,result.width(),"Word::repeat");
    GECODE_POST;
    if ((Word::WordView(x) == Word::WordView(result)) && (count == 1U))
      return;
    post_fixed(home,Word::WordView(x),Word::WordView(result),
               Word::Structure::FO_REPEAT,count,x.width());
  }

  void
  repeat(Home home, unsigned int input_width, WordValue value,
         unsigned int count, WordVar result) {
    Word::ConstWordView x(input_width,value);
    check_repeat(input_width,count,result.width(),"Word::repeat");
    GECODE_POST;
    post_fixed(home,x,Word::WordView(result),Word::Structure::FO_REPEAT,
               count,input_width);
  }

  void
  zero_extend(Home home, WordVar x, unsigned int result_width,
              WordVar result) {
    check_extend(x.width(),result_width,result.width(),"Word::zero_extend");
    GECODE_POST;
    if (Word::WordView(x) == Word::WordView(result))
      return;
    post_fixed(home,Word::WordView(x),Word::WordView(result),
               Word::Structure::FO_ZERO_EXTEND);
  }

  void
  zero_extend(Home home, unsigned int input_width, WordValue value,
              unsigned int result_width, WordVar result) {
    Word::ConstWordView x(input_width,value);
    check_extend(input_width,result_width,result.width(),"Word::zero_extend");
    GECODE_POST;
    post_fixed(home,x,Word::WordView(result),
               Word::Structure::FO_ZERO_EXTEND);
  }

  void
  sign_extend(Home home, WordVar x, unsigned int result_width,
              WordVar result) {
    check_extend(x.width(),result_width,result.width(),"Word::sign_extend");
    GECODE_POST;
    if (Word::WordView(x) == Word::WordView(result))
      return;
    post_fixed(home,Word::WordView(x),Word::WordView(result),
               Word::Structure::FO_SIGN_EXTEND);
  }

  void
  sign_extend(Home home, unsigned int input_width, WordValue value,
              unsigned int result_width, WordVar result) {
    Word::ConstWordView x(input_width,value);
    check_extend(input_width,result_width,result.width(),"Word::sign_extend");
    GECODE_POST;
    post_fixed(home,x,Word::WordView(result),
               Word::Structure::FO_SIGN_EXTEND);
  }

  void
  shift_left(Home home, WordVar x, unsigned int amount, WordVar result) {
    check_same_width(x.width(),result.width(),"Word::shift_left");
    GECODE_POST;
    post_fixed_word(home,x,amount,result,Word::Structure::FO_SHIFT_LEFT);
  }

  void
  shift_left(Home home, WordVar x, WordVar amount, WordVar result) {
    if ((x.width() != amount.width()) ||
        (x.width() != result.width()))
      throw Word::WidthMismatch("Word::shift_left");
    GECODE_POST;
    post_variable_shift(home,x,amount,result,
                        Word::Structure::FO_SHIFT_LEFT);
  }

  void
  shift_left(Home home, unsigned int width, WordValue value,
             unsigned int amount, WordVar result) {
    Word::ConstWordView x(width,value);
    check_same_width(width,result.width(),"Word::shift_left");
    GECODE_POST;
    post_fixed(home,x,Word::WordView(result),
               Word::Structure::FO_SHIFT_LEFT,amount);
  }

  void
  logical_shift_right(Home home, WordVar x, unsigned int amount,
                      WordVar result) {
    check_same_width(x.width(),result.width(),
                     "Word::logical_shift_right");
    GECODE_POST;
    post_fixed_word(home,x,amount,result,
                    Word::Structure::FO_LOGICAL_SHIFT_RIGHT);
  }

  void
  logical_shift_right(Home home, WordVar x, WordVar amount,
                      WordVar result) {
    if ((x.width() != amount.width()) ||
        (x.width() != result.width()))
      throw Word::WidthMismatch("Word::logical_shift_right");
    GECODE_POST;
    post_variable_shift(home,x,amount,result,
                        Word::Structure::FO_LOGICAL_SHIFT_RIGHT);
  }

  void
  logical_shift_right(Home home, unsigned int width, WordValue value,
                      unsigned int amount, WordVar result) {
    Word::ConstWordView x(width,value);
    check_same_width(width,result.width(),"Word::logical_shift_right");
    GECODE_POST;
    post_fixed(home,x,Word::WordView(result),
               Word::Structure::FO_LOGICAL_SHIFT_RIGHT,amount);
  }

  void
  arithmetic_shift_right(Home home, WordVar x, unsigned int amount,
                         WordVar result) {
    check_same_width(x.width(),result.width(),
                     "Word::arithmetic_shift_right");
    GECODE_POST;
    post_fixed_word(home,x,amount,result,
                    Word::Structure::FO_ARITHMETIC_SHIFT_RIGHT);
  }

  void
  arithmetic_shift_right(Home home, WordVar x, WordVar amount,
                         WordVar result) {
    if ((x.width() != amount.width()) ||
        (x.width() != result.width()))
      throw Word::WidthMismatch("Word::arithmetic_shift_right");
    GECODE_POST;
    post_variable_shift(home,x,amount,result,
                        Word::Structure::FO_ARITHMETIC_SHIFT_RIGHT);
  }

  void
  arithmetic_shift_right(Home home, unsigned int width, WordValue value,
                         unsigned int amount, WordVar result) {
    Word::ConstWordView x(width,value);
    check_same_width(width,result.width(),
                     "Word::arithmetic_shift_right");
    GECODE_POST;
    post_fixed(home,x,Word::WordView(result),
               Word::Structure::FO_ARITHMETIC_SHIFT_RIGHT,amount);
  }

  void
  rotate_left(Home home, WordVar x, unsigned int amount, WordVar result) {
    check_same_width(x.width(),result.width(),"Word::rotate_left");
    amount %= x.width();
    GECODE_POST;
    post_fixed_word(home,x,amount,result,Word::Structure::FO_ROTATE_LEFT);
  }

  void
  rotate_left(Home home, unsigned int width, WordValue value,
              unsigned int amount, WordVar result) {
    Word::ConstWordView x(width,value);
    check_same_width(width,result.width(),"Word::rotate_left");
    amount %= width;
    GECODE_POST;
    post_fixed(home,x,Word::WordView(result),
               Word::Structure::FO_ROTATE_LEFT,amount);
  }

  void
  rotate_right(Home home, WordVar x, unsigned int amount, WordVar result) {
    check_same_width(x.width(),result.width(),"Word::rotate_right");
    amount %= x.width();
    GECODE_POST;
    post_fixed_word(home,x,amount,result,Word::Structure::FO_ROTATE_RIGHT);
  }

  void
  rotate_right(Home home, unsigned int width, WordValue value,
               unsigned int amount, WordVar result) {
    Word::ConstWordView x(width,value);
    check_same_width(width,result.width(),"Word::rotate_right");
    amount %= width;
    GECODE_POST;
    post_fixed(home,x,Word::WordView(result),
               Word::Structure::FO_ROTATE_RIGHT,amount);
  }

}

// STATISTICS: word-post
