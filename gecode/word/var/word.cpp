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

#include <gecode/word.hh>

namespace Gecode {
  WordVar::WordVar(Space& home, unsigned int width)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    if ((width == 0U) || (width > 64U))
      throw Word::OutOfLimits("WordVar::WordVar");
    _init(home,width,0,Word::width_mask(width));
  }
  WordVar::WordVar(Space& home, unsigned int width,
                   WordValue lo, WordValue hi)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    Word::check_domain(width,lo,hi,"WordVar::WordVar");
    _init(home,width,lo,hi);
  }

  void dom(Home home, WordVar x, WordValue lo, WordValue hi) {
    GECODE_POST;
    Word::WordView xv(x);
    GECODE_ME_FAIL(xv.narrow(home,lo,hi));
  }
  void dom(Home home, WordVar x, WordValue value) {
    GECODE_POST;
    Word::WordView xv(x);
    GECODE_ME_FAIL(xv.eq(home,value));
  }
}

// STATISTICS: word-var
