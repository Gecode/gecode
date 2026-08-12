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

#include <gecode/word/element.hh>

namespace Gecode {

  void
  element(Home home, const WordVarArgs& x, IntVar i, WordVar y) {
    if (x.size() == 0)
      throw Word::TooFewArguments("Word::element");
    const unsigned int width = y.width();
    for (int j=0; j<x.size(); j++)
      if (x[j].width() != width)
        throw Word::WidthMismatch("Word::element");
    GECODE_POST;
    Int::IdxViewArray<Word::WordView> views(home,x.size());
    for (int j=0; j<x.size(); j++) {
      views[j].idx = j;
      views[j].view = Word::WordView(x[j]);
    }
    GECODE_ES_FAIL((Word::Element::View::post(
      home,views,Int::IntView(i),Word::WordView(y))));
  }

}

// STATISTICS: word-post
