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

namespace Gecode { namespace Word { namespace Arithmetic {

  forceinline bool
  arithmetic_bit_in(WordView x, unsigned int bit, unsigned int value) {
    const WordValue mask = WordValue(1) << bit;
    return value != 0 ? (x.hi() & mask) != 0 : (x.lo() & mask) == 0;
  }

  forceinline
  Neg::Neg(Home home, WordView y0, WordView y1)
    : BinaryPropagator<WordView,PC_WORD_BITS>(home,y0,y1) {}

  forceinline
  Neg::Neg(Space& home, Neg& p)
    : BinaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline Actor*
  Neg::copy(Space& home) {
    return new (home) Neg(home,*this);
  }

  forceinline PropCost
  Neg::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  forceinline bool
  neg_transition(WordView x, WordView z, bool xz, unsigned int bit,
                 unsigned int carry, unsigned int xv, unsigned int zv,
                 unsigned int& next) {
    if (!arithmetic_bit_in(x,bit,xv) || !arithmetic_bit_in(z,bit,zv) ||
        (xz && (xv != zv)))
      return false;
    const unsigned int sum = (1U-xv)+carry;
    if (zv != (sum & 1U))
      return false;
    next = sum >> 1;
    return true;
  }

  forceinline ExecStatus
  Neg::narrow(Home home, WordView x, WordView z) {
    const unsigned int width = x.width();
    const bool xz = x == z;
    unsigned char forward[65] = {0};
    unsigned char backward[65] = {0};
    forward[0] = 2U;
    for (unsigned int bit=0; bit<width; bit++) {
      unsigned int states = 0;
      for (unsigned int carry=0; carry<2; carry++) {
        if ((forward[bit] & (1U << carry)) == 0)
          continue;
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int zv=0; zv<2; zv++) {
            unsigned int next;
            if (neg_transition(x,z,xz,bit,carry,xv,zv,next))
              states |= 1U << next;
          }
      }
      forward[bit+1] = static_cast<unsigned char>(states);
      if (states == 0)
        return ES_FAILED;
    }

    backward[width] = 3U;
    for (unsigned int bit=width; bit-- > 0;) {
      unsigned int states = 0;
      for (unsigned int carry=0; carry<2; carry++)
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int zv=0; zv<2; zv++) {
            unsigned int next;
            if (neg_transition(x,z,xz,bit,carry,xv,zv,next) &&
                ((backward[bit+1] & (1U << next)) != 0))
              states |= 1U << carry;
          }
      backward[bit] = static_cast<unsigned char>(states);
    }
    if ((backward[0] & 2U) == 0)
      return ES_FAILED;

    WordValue lo[2] = {0,0};
    WordValue hi[2] = {0,0};
    for (unsigned int bit=0; bit<width; bit++) {
      unsigned int support[2][2] = {{0,0},{0,0}};
      for (unsigned int carry=0; carry<2; carry++) {
        if ((forward[bit] & (1U << carry)) == 0)
          continue;
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int zv=0; zv<2; zv++) {
            unsigned int next;
            if (neg_transition(x,z,xz,bit,carry,xv,zv,next) &&
                ((backward[bit+1] & (1U << next)) != 0)) {
              support[0][xv] = support[1][zv] = 1U;
            }
          }
      }
      const WordValue mask = WordValue(1) << bit;
      for (int i=0; i<2; i++) {
        if (support[i][1] != 0)
          hi[i] |= mask;
        if (support[i][0] == 0)
          lo[i] |= mask;
      }
    }
    GECODE_ME_CHECK(x.narrow(home,lo[0],hi[0]));
    GECODE_ME_CHECK(z.narrow(home,lo[1],hi[1]));
    return ES_OK;
  }

  forceinline ExecStatus
  Neg::post(Home home, WordView x0, WordView x1) {
    GECODE_ES_CHECK(narrow(home,x0,x1));
    if (!(x0.assigned() && x1.assigned()))
      (void) new (home) Neg(home,x0,x1);
    return ES_OK;
  }

  forceinline ExecStatus
  Neg::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1));
    if (x0.assigned() && x1.assigned())
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  forceinline
  Sub::Sub(Home home, WordView y0, WordView y1, WordView y2)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,y0,y1,y2) {}

  forceinline
  Sub::Sub(Space& home, Sub& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline Actor*
  Sub::copy(Space& home) {
    return new (home) Sub(home,*this);
  }

  forceinline PropCost
  Sub::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  forceinline bool
  sub_transition(WordView x, WordView y, WordView z,
                 bool xy, bool xz, bool yz, unsigned int bit,
                 unsigned int borrow, unsigned int xv, unsigned int yv,
                 unsigned int zv, unsigned int& next) {
    if (!arithmetic_bit_in(x,bit,xv) || !arithmetic_bit_in(y,bit,yv) ||
        !arithmetic_bit_in(z,bit,zv) ||
        (xy && (xv != yv)) || (xz && (xv != zv)) ||
        (yz && (yv != zv)))
      return false;
    const int difference = static_cast<int>(xv) -
      static_cast<int>(yv) - static_cast<int>(borrow);
    if (zv != (static_cast<unsigned int>(difference) & 1U))
      return false;
    next = difference < 0 ? 1U : 0U;
    return true;
  }

  forceinline ExecStatus
  Sub::narrow(Home home, WordView x, WordView y, WordView z) {
    const unsigned int width = x.width();
    const bool xy = x == y;
    const bool xz = x == z;
    const bool yz = y == z;
    unsigned char forward[65] = {0};
    unsigned char backward[65] = {0};
    forward[0] = 1U;
    for (unsigned int bit=0; bit<width; bit++) {
      unsigned int states = 0;
      for (unsigned int borrow=0; borrow<2; borrow++) {
        if ((forward[bit] & (1U << borrow)) == 0)
          continue;
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int yv=0; yv<2; yv++)
            for (unsigned int zv=0; zv<2; zv++) {
              unsigned int next;
              if (sub_transition(x,y,z,xy,xz,yz,bit,borrow,
                                 xv,yv,zv,next))
                states |= 1U << next;
            }
      }
      forward[bit+1] = static_cast<unsigned char>(states);
      if (states == 0)
        return ES_FAILED;
    }

    backward[width] = 3U;
    for (unsigned int bit=width; bit-- > 0;) {
      unsigned int states = 0;
      for (unsigned int borrow=0; borrow<2; borrow++)
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int yv=0; yv<2; yv++)
            for (unsigned int zv=0; zv<2; zv++) {
              unsigned int next;
              if (sub_transition(x,y,z,xy,xz,yz,bit,borrow,
                                 xv,yv,zv,next) &&
                  ((backward[bit+1] & (1U << next)) != 0))
                states |= 1U << borrow;
            }
      backward[bit] = static_cast<unsigned char>(states);
    }
    if ((backward[0] & 1U) == 0)
      return ES_FAILED;

    WordValue lo[3] = {0,0,0};
    WordValue hi[3] = {0,0,0};
    for (unsigned int bit=0; bit<width; bit++) {
      unsigned int support[3][2] = {{0,0},{0,0},{0,0}};
      for (unsigned int borrow=0; borrow<2; borrow++) {
        if ((forward[bit] & (1U << borrow)) == 0)
          continue;
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int yv=0; yv<2; yv++)
            for (unsigned int zv=0; zv<2; zv++) {
              unsigned int next;
              if (sub_transition(x,y,z,xy,xz,yz,bit,borrow,
                                 xv,yv,zv,next) &&
                  ((backward[bit+1] & (1U << next)) != 0)) {
                support[0][xv] = support[1][yv] = support[2][zv] = 1U;
              }
            }
      }
      const WordValue mask = WordValue(1) << bit;
      for (int i=0; i<3; i++) {
        if (support[i][1] != 0)
          hi[i] |= mask;
        if (support[i][0] == 0)
          lo[i] |= mask;
      }
    }
    GECODE_ME_CHECK(x.narrow(home,lo[0],hi[0]));
    GECODE_ME_CHECK(y.narrow(home,lo[1],hi[1]));
    GECODE_ME_CHECK(z.narrow(home,lo[2],hi[2]));
    return ES_OK;
  }

  forceinline ExecStatus
  Sub::post(Home home, WordView x0, WordView x1, WordView x2) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    if (!(x0.assigned() && x1.assigned() && x2.assigned()))
      (void) new (home) Sub(home,x0,x1,x2);
    return ES_OK;
  }

  forceinline ExecStatus
  Sub::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    if (x0.assigned() && x1.assigned() && x2.assigned())
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
