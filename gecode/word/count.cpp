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

#include <gecode/word/count.hh>

namespace Gecode {

  void
  popcount(Home home, WordVar x, IntVar count) {
    GECODE_POST;
    GECODE_ES_FAIL((Word::Count::Popcount::post(
      home,Word::WordView(x),Int::IntView(count))));
  }

  void
  count_leading_zeros(Home home, WordVar x, IntVar count) {
    GECODE_POST;
    GECODE_ES_FAIL((Word::Count::ZeroCount<Word::Count::ZD_LEADING>::post(
      home,Word::WordView(x),Int::IntView(count))));
  }

  void
  count_trailing_zeros(Home home, WordVar x, IntVar count) {
    GECODE_POST;
    GECODE_ES_FAIL((Word::Count::ZeroCount<Word::Count::ZD_TRAILING>::post(
      home,Word::WordView(x),Int::IntView(count))));
  }

}

// STATISTICS: word-post
