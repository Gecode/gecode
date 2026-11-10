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
    WordVar constant_for(Home home, WordValue value, WordVar result) {
      const unsigned int width=result.width();
      const WordDomainType kind=
        Word::intermediate_domain_type(result.domain_type());
      return kind == WDT_CUBE ?
        WordVar(home,width,{value}) :
        WordVar(home,width,kind,value,value);
    }

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

    /// Post \a result = \a word, using bounds in integer order \a order
    void post_same_branches(Home home, Word::IntegerRankOrder order,
                            WordVar word, WordVar result) {
      switch (order) {
      case Word::IntegerRankOrder::UNSIGNED:
        GECODE_ES_FAIL((Word::Rel::BoundEq<
          Word::UnsignedWordView,Word::UnsignedWordView>::post(
            home,Word::UnsignedWordView(word),
            Word::UnsignedWordView(result))));
        break;
      case Word::IntegerRankOrder::SIGNED:
        GECODE_ES_FAIL((Word::Rel::BoundEq<
          Word::SignedWordView,Word::SignedWordView>::post(
            home,Word::SignedWordView(word),
            Word::SignedWordView(result))));
        break;
      case Word::IntegerRankOrder::NONE:
      case Word::IntegerRankOrder::NON_INTEGER:
        GECODE_ES_FAIL((Word::Rel::Eq<Word::WordView,Word::WordView>
                        ::post(home,Word::WordView(word),
                               Word::WordView(result))));
        break;
      }
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
    const Word::WordView t(then_word), e(else_word), r(result);
    const Word::IntegerRankOrder order=Word::common_integer_rank_order(
      then_word.domain_type(),else_word.domain_type(),result.domain_type());
    if (t == e) {
      post_same_branches(home,order,then_word,result);
      return;
    }
    switch (order) {
    case Word::IntegerRankOrder::UNSIGNED:
      GECODE_ES_FAIL((Word::Conditional::BoundIte<
        Word::UnsignedWordView>::post(
          home,Int::BoolView(control),Word::UnsignedWordView(then_word),
          Word::UnsignedWordView(else_word),Word::UnsignedWordView(result))));
      break;
    case Word::IntegerRankOrder::SIGNED:
      GECODE_ES_FAIL((Word::Conditional::BoundIte<
        Word::SignedWordView>::post(
          home,Int::BoolView(control),Word::SignedWordView(then_word),
          Word::SignedWordView(else_word),Word::SignedWordView(result))));
      break;
    case Word::IntegerRankOrder::NONE:
    case Word::IntegerRankOrder::NON_INTEGER:
      GECODE_ES_FAIL((Word::Conditional::Ite<
        Word::WordView,Word::WordView,Word::WordView>::post(
          home,Int::BoolView(control),t,e,r)));
      break;
    }
  }

  void
  ite(Home home, WordVar control, WordValue then_value, WordVar else_word,
      WordVar result) {
    const unsigned int width=result.width();
    if ((control.width() != width) || (else_word.width() != width))
      throw Word::WidthMismatch("Word::ite");
    Word::ConstWordView constant(width,then_value);
    GECODE_POST;
    WordVar then_word=constant_for(home,constant.val(),result);
    post_mask_ite(home,control,then_word,else_word,result);
  }

  void
  ite(Home home, WordVar control, WordVar then_word, WordValue else_value,
      WordVar result) {
    const unsigned int width=result.width();
    if ((control.width() != width) || (then_word.width() != width))
      throw Word::WidthMismatch("Word::ite");
    Word::ConstWordView constant(width,else_value);
    GECODE_POST;
    WordVar else_word=constant_for(home,constant.val(),result);
    post_mask_ite(home,control,then_word,else_word,result);
  }

  void
  ite(Home home, BoolVar control, WordValue then_value, WordVar else_word,
      WordVar result) {
    if (else_word.width() != result.width())
      throw Word::WidthMismatch("Word::ite");
    Word::ConstWordView constant(result.width(),then_value);
    GECODE_POST;
    if ((else_word.domain_type() == result.domain_type()) &&
        Word::has_integer_rank_order(result.domain_type())) {
      ite(home,control,constant_for(home,constant.val(),result),
          else_word,result);
      return;
    }
    GECODE_ES_FAIL((Word::Conditional::Ite<
      Word::ConstWordView,Word::WordView,Word::WordView>::post(
        home,Int::BoolView(control),constant,Word::WordView(else_word),
        Word::WordView(result))));
  }

  void
  ite(Home home, BoolVar control, WordVar then_word, WordValue else_value,
      WordVar result) {
    if (then_word.width() != result.width())
      throw Word::WidthMismatch("Word::ite");
    Word::ConstWordView constant(result.width(),else_value);
    GECODE_POST;
    if ((then_word.domain_type() == result.domain_type()) &&
        Word::has_integer_rank_order(result.domain_type())) {
      ite(home,control,then_word,
          constant_for(home,constant.val(),result),result);
      return;
    }
    GECODE_ES_FAIL((Word::Conditional::Ite<
      Word::WordView,Word::ConstWordView,Word::WordView>::post(
        home,Int::BoolView(control),Word::WordView(then_word),constant,
        Word::WordView(result))));
  }

}

// STATISTICS: word-post
