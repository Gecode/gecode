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
 *
 */

#include <gecode/word/logic.hh>
#include <gecode/word/rel.hh>
#include <gecode/word/structure.hh>
#include <gecode/word/conditional.hh>

namespace Gecode {

  namespace {
    void check_widths(WordVar then_word, WordVar else_word, WordVar result) {
      if ((then_word.width() != else_word.width()) ||
          (then_word.width() != result.width()))
        throw Word::WidthMismatch("Word::ite");
    }

    void post_mask_ite(Home home, WordVar control, WordVar then_word,
                       WordVar else_word, WordVar result) {
      WordValue allowed[16] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
      };
      const WordValue mask = result.mask();
      for (unsigned int t=0; t<16; t++) {
        const unsigned int control_bit = t & 1U;
        const unsigned int then_bit = (t >> 1) & 1U;
        const unsigned int else_bit = (t >> 2) & 1U;
        const unsigned int result_bit = (t >> 3) & 1U;
        if (result_bit == (control_bit ? then_bit : else_bit))
          allowed[t] = mask;
      }
      const Word::WordView views[] = {
        Word::WordView(control), Word::WordView(then_word),
        Word::WordView(else_word), Word::WordView(result)
      };
      Word::Logic::post_table(home,views,4,allowed);
    }
  }

  void
  ite(Home home, WordVar control, WordVar then_word, WordVar else_word,
      WordVar result) {
    check_widths(then_word,else_word,result);
    if (control.width() != result.width())
      throw Word::WidthMismatch("Word::ite");
    GECODE_POST;
    if (Word::WordView(then_word) == Word::WordView(else_word)) {
      GECODE_ES_FAIL((Word::Rel::Eq<Word::WordView,Word::WordView>
                      ::post(home,Word::WordView(then_word),
                             Word::WordView(result))));
      return;
    }
    post_mask_ite(home,control,then_word,else_word,result);
  }

  void
  ite(Home home, BoolVar control, WordVar then_word, WordVar else_word,
      WordVar result) {
    check_widths(then_word,else_word,result);
    GECODE_POST;
    if (Word::WordView(then_word) == Word::WordView(else_word)) {
      GECODE_ES_FAIL((Word::Rel::Eq<Word::WordView,Word::WordView>
                      ::post(home,Word::WordView(then_word),
                             Word::WordView(result))));
      return;
    }
    GECODE_ES_FAIL((Word::Conditional::Ite<
      Word::WordView,Word::WordView,Word::WordView>::post(
        home,Int::BoolView(control),Word::WordView(then_word),
        Word::WordView(else_word),Word::WordView(result))));
  }

  void
  ite(Home home, WordVar control, unsigned int width, WordValue then_value,
      WordVar else_word, WordVar result) {
    Word::ConstWordView constant(width,then_value);
    if ((control.width() != width) || (else_word.width() != width) ||
        (result.width() != width))
      throw Word::WidthMismatch("Word::ite");
    GECODE_POST;
    WordVar then_word(home,width,constant.val(),constant.val());
    post_mask_ite(home,control,then_word,else_word,result);
  }

  void
  ite(Home home, WordVar control, WordVar then_word, unsigned int width,
      WordValue else_value, WordVar result) {
    Word::ConstWordView constant(width,else_value);
    if ((control.width() != width) || (then_word.width() != width) ||
        (result.width() != width))
      throw Word::WidthMismatch("Word::ite");
    GECODE_POST;
    WordVar else_word(home,width,constant.val(),constant.val());
    post_mask_ite(home,control,then_word,else_word,result);
  }

  void
  ite(Home home, BoolVar control, unsigned int width, WordValue then_value,
      WordVar else_word, WordVar result) {
    Word::ConstWordView constant(width,then_value);
    if ((else_word.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::ite");
    GECODE_POST;
    GECODE_ES_FAIL((Word::Conditional::Ite<
      Word::ConstWordView,Word::WordView,Word::WordView>::post(
        home,Int::BoolView(control),constant,Word::WordView(else_word),
        Word::WordView(result))));
  }

  void
  ite(Home home, BoolVar control, WordVar then_word, unsigned int width,
      WordValue else_value, WordVar result) {
    Word::ConstWordView constant(width,else_value);
    if ((then_word.width() != width) || (result.width() != width))
      throw Word::WidthMismatch("Word::ite");
    GECODE_POST;
    GECODE_ES_FAIL((Word::Conditional::Ite<
      Word::WordView,Word::ConstWordView,Word::WordView>::post(
        home,Int::BoolView(control),Word::WordView(then_word),constant,
        Word::WordView(result))));
  }

}

// STATISTICS: word-post
