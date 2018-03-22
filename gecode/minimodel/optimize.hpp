/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

namespace Gecode {

  forceinline
  IntMinimizeSpace::IntMinimizeSpace(void) {}

  forceinline
  IntMinimizeSpace::IntMinimizeSpace(IntMinimizeSpace& s)
    : Space(s) {}


  forceinline
  IntMaximizeSpace::IntMaximizeSpace(void) {}

  forceinline
  IntMaximizeSpace::IntMaximizeSpace(IntMaximizeSpace& s)
    : Space(s) {}


  forceinline
  IntLexMinimizeSpace::IntLexMinimizeSpace(void) {}

  forceinline
  IntLexMinimizeSpace::IntLexMinimizeSpace(IntLexMinimizeSpace& s)
    : Space(s) {}


  forceinline
  IntLexMaximizeSpace::IntLexMaximizeSpace(void) {}

  forceinline
  IntLexMaximizeSpace::IntLexMaximizeSpace(IntLexMaximizeSpace& s)
    : Space(s) {}


#ifdef GECODE_HAS_FLOAT_VARS

  forceinline
  FloatMinimizeSpace::FloatMinimizeSpace(FloatNum s)
    : step(s) {}

  forceinline
  FloatMinimizeSpace::FloatMinimizeSpace(FloatMinimizeSpace& s)
    : Space(s), step(s.step) {}


  forceinline
  FloatMaximizeSpace::FloatMaximizeSpace(FloatNum s)
    : step(s) {}

  forceinline
  FloatMaximizeSpace::FloatMaximizeSpace(FloatMaximizeSpace& s)
    : Space(s), step(s.step) {}

#endif

}

// STATISTICS: minimodel-search

