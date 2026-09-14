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

namespace Gecode { namespace Word { namespace Logic {

  template<BinaryOperation op>
  forceinline
  Binary<op>::Binary(Home home, WordView x, WordView y, WordView z)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,x,y,z) {}

  template<BinaryOperation op>
  forceinline
  Binary<op>::Binary(Space& home, Binary& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  /// Three proposed cube masks, in the actor's x/y/z publication order.
  struct BinaryLogicMasks {
    WordValue xlo, xhi, ylo, yhi, zlo, zhi;
  };

  /// Local binary deductions, consumed only when all cubes retain support.
  struct BinaryLogicResult {
    bool has_support;
    BinaryLogicMasks masks;
  };

  /// Computes a binary deduction pass without publishing any domain change.
  template<BinaryOperation op>
  forceinline BinaryLogicResult
  compute_binary_masks(BinaryLogicMasks masks, WordValue mask) {
    WordValue& xlo=masks.xlo; WordValue& xhi=masks.xhi;
    WordValue& ylo=masks.ylo; WordValue& yhi=masks.yhi;
    WordValue& zlo=masks.zlo; WordValue& zhi=masks.zhi;
    if (op == BO_OR) {
      zlo |= xlo|ylo;
      zhi &= xhi|yhi;
      xhi &= zhi;
      yhi &= zhi;
      xlo |= zlo&~yhi&mask;
      ylo |= zlo&~xhi&mask;
    } else {
      const WordValue known_yz=~((yhi^ylo)|(zhi^zlo))&mask;
      const WordValue known_xz=~((xhi^xlo)|(zhi^zlo))&mask;
      const WordValue known_xy=~((xhi^xlo)|(yhi^ylo))&mask;
      const WordValue value_x=(ylo^zlo)&known_yz;
      const WordValue value_y=(xlo^zlo)&known_xz;
      const WordValue value_z=(xlo^ylo)&known_xy;
      xlo |= value_x; xhi &= value_x|~known_yz;
      ylo |= value_y; yhi &= value_y|~known_xz;
      zlo |= value_z; zhi &= value_z|~known_xy;
    }
    const bool has_support=((xlo&~xhi) == 0) && ((ylo&~yhi) == 0) &&
      ((zlo&~zhi) == 0);
    return {has_support,masks};
  }

  template<BinaryOperation op>
  ExecStatus
  Binary<op>::narrow(Home home, WordView x, WordView y, WordView z) {
    const WordValue mask=x.mask();
    const bool has_bounded_view=x.bounded() || y.bounded() || z.bounded();
    for (;;) {
      const BinaryLogicMasks before={x.lo(),x.hi(),y.lo(),y.hi(),z.lo(),z.hi()};
      const BinaryLogicResult result=compute_binary_masks<op>(before,mask);
      if (!result.has_support)
        return ES_FAILED;
      const BinaryLogicMasks& masks=result.masks;
      const bool has_x_change=(masks.xlo != x.lo()) || (masks.xhi != x.hi());
      if (has_x_change)
        GECODE_ME_CHECK(x.narrow(home,masks.xlo,masks.xhi));
      const bool has_y_change=(masks.ylo != y.lo()) || (masks.yhi != y.hi());
      if (has_y_change)
        GECODE_ME_CHECK(y.narrow(home,masks.ylo,masks.yhi));
      const bool has_z_change=(masks.zlo != z.lo()) || (masks.zhi != z.hi());
      if (has_z_change)
        GECODE_ME_CHECK(z.narrow(home,masks.zlo,masks.zhi));
      const bool is_closed=!has_bounded_view ||
        ((masks.xlo == x.lo()) && (masks.xhi == x.hi()) &&
         (masks.ylo == y.lo()) && (masks.yhi == y.hi()) &&
         (masks.zlo == z.lo()) && (masks.zhi == z.hi()));
      if (is_closed)
        return ES_OK;
    }
  }

  template<BinaryOperation op>
  ExecStatus
  Binary<op>::post(Home home, WordView x, WordView y, WordView z) {
    GECODE_ES_CHECK(narrow(home,x,y,z));
    const bool is_assigned=x.assigned() && y.assigned() && z.assigned();
    if (!is_assigned)
      (void) new (home) Binary(home,x,y,z);
    return ES_OK;
  }

  template<BinaryOperation op>
  forceinline Actor*
  Binary<op>::copy(Space& home) {
    return new (home) Binary(home,*this);
  }

  template<BinaryOperation op>
  forceinline PropCost
  Binary<op>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<BinaryOperation op>
  ExecStatus
  Binary<op>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    const bool is_assigned=x0.assigned() && x1.assigned() && x2.assigned();
    return is_assigned ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
