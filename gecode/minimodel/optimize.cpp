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

#include <gecode/minimodel.hh>

namespace Gecode {

  void
  IntMinimizeSpace::constrain(const Space& _best) {
    const IntMinimizeSpace* best =
      dynamic_cast<const IntMinimizeSpace*>(&_best);
    if (best == nullptr)
      throw DynamicCastFailed("IntMinimizeSpace::constrain");
    rel(*this, cost(), IRT_LE, best->cost().val());
  }


  void
  IntMaximizeSpace::constrain(const Space& _best) {
    const IntMaximizeSpace* best =
      dynamic_cast<const IntMaximizeSpace*>(&_best);
    if (best == nullptr)
      throw DynamicCastFailed("IntMaximizeSpace::constrain");
    rel(*this, cost(), IRT_GR, best->cost().val());
  }


  void
  IntLexMinimizeSpace::constrain(const Space& _best) {
    const IntLexMinimizeSpace* best =
      dynamic_cast<const IntLexMinimizeSpace*>(&_best);
    if (best == nullptr)
      throw DynamicCastFailed("IntLexMinimizeSpace::constrain");
    IntVarArgs cx(cost()), bx(best->cost());
    IntArgs bn(bx.size());
    for (int i=bn.size(); i--; )
      bn[i] = bx[i].val();
    rel(*this, cx, IRT_LE, bn);
  }

  void
  IntLexMaximizeSpace::constrain(const Space& _best) {
    const IntLexMaximizeSpace* best =
      dynamic_cast<const IntLexMaximizeSpace*>(&_best);
    if (best == nullptr)
      throw DynamicCastFailed("IntLexMaximizeSpace::constrain");
    IntVarArgs cx(cost()), bx(best->cost());
    IntArgs bn(bx.size());
    for (int i=bn.size(); i--; )
      bn[i] = bx[i].val();
    rel(*this, cx, IRT_GR, bn);
  }

#ifdef GECODE_HAS_FLOAT_VARS

  void
  FloatMinimizeSpace::constrain(const Space& _best) {
    const FloatMinimizeSpace* best =
      dynamic_cast<const FloatMinimizeSpace*>(&_best);
    if (best == nullptr)
      throw DynamicCastFailed("FloatMinimizeSpace::constrain");
    rel(*this, cost(), FRT_LE, best->cost().val()-step);
  }


  void
  FloatMaximizeSpace::constrain(const Space& _best) {
    const FloatMaximizeSpace* best =
      dynamic_cast<const FloatMaximizeSpace*>(&_best);
    if (best == nullptr)
      throw DynamicCastFailed("FloatMaximizeSpace::constrain");
    rel(*this, cost(), FRT_GR, best->cost().val()+step);
  }

#endif

}

// STATISTICS: minimodel-search

