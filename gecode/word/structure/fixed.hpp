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

namespace Gecode { namespace Word { namespace Structure {

  forceinline WordValue
  low_mask(unsigned int n) {
    return (n == 0U) ? 0 : ((WordValue(1) << n) - 1);
  }

  forceinline WordValue
  rotate_left_value(WordValue value, unsigned int width,
                    unsigned int amount) {
    amount %= width;
    if (amount == 0U)
      return value;
    const WordValue mask = width_mask(width);
    return ((value << amount) | (value >> (width-amount))) & mask;
  }

  forceinline WordValue
  rotate_right_value(WordValue value, unsigned int width,
                     unsigned int amount) {
    amount %= width;
    if (amount == 0U)
      return value;
    const WordValue mask = width_mask(width);
    return ((value >> amount) | (value << (width-amount))) & mask;
  }

  template<class View0, class View1>
  forceinline
  Fixed<View0,View1>::Fixed(Home home, View0 y0, View1 y1,
                           FixedOp op0, unsigned int a0, unsigned int b0)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,y0,y1),
      op(op0), a(a0), b(b0) {}

  template<class View0, class View1>
  forceinline
  Fixed<View0,View1>::Fixed(Space& home, Fixed& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,p),
      op(p.op), a(p.a), b(p.b) {}

  template<class View0, class View1>
  ExecStatus
  Fixed<View0,View1>::narrow(Home home, View0 x0, View1 x1,
                            FixedOp op, unsigned int a, unsigned int b) {
    switch (op) {
    case FO_EXTRACT: {
      const WordValue field = x1.mask() << a;
      WordValue lo0 = x0.lo() | (x1.lo() << a);
      WordValue hi0 = x0.hi() & ~((~x1.hi() & x1.mask()) << a);
      GECODE_ME_CHECK(x0.narrow(home,lo0,hi0));
      GECODE_ME_CHECK(x1.narrow(home,(x0.lo() & field) >> a,
                                (x0.hi() & field) >> a));
      break;
    }
    case FO_REPEAT: {
      WordValue lo0 = x0.lo();
      WordValue hi0 = x0.hi();
      for (unsigned int i=0; i<a; i++) {
        lo0 |= (x1.lo() >> (i*b)) & x0.mask();
        hi0 &= (x1.hi() >> (i*b)) & x0.mask();
      }
      GECODE_ME_CHECK(x0.narrow(home,lo0,hi0));
      WordValue lo1 = 0;
      WordValue hi1 = 0;
      for (unsigned int i=0; i<a; i++) {
        lo1 |= x0.lo() << (i*b);
        hi1 |= x0.hi() << (i*b);
      }
      GECODE_ME_CHECK(x1.narrow(home,lo1,hi1));
      break;
    }
    case FO_ZERO_EXTEND: {
      GECODE_ME_CHECK(x0.narrow(home,x0.lo() | (x1.lo() & x0.mask()),
                                x0.hi() & x1.hi()));
      GECODE_ME_CHECK(x1.narrow(home,x0.lo(),x0.hi()));
      break;
    }
    case FO_SIGN_EXTEND: {
      const WordValue sign = WordValue(1) << (x0.width()-1);
      const WordValue extension = x1.mask() & ~x0.mask();
      WordValue lo0 = x0.lo() | (x1.lo() & x0.mask());
      WordValue hi0 = x0.hi() & x1.hi();
      if ((x1.lo() & extension) != 0)
        lo0 |= sign;
      if ((x1.hi() & extension) != extension)
        hi0 &= ~sign;
      GECODE_ME_CHECK(x0.narrow(home,lo0,hi0));
      WordValue lo1 = x0.lo();
      WordValue hi1 = x0.hi();
      if ((x0.lo() & sign) != 0)
        lo1 |= extension;
      if ((x0.hi() & sign) != 0)
        hi1 |= extension;
      GECODE_ME_CHECK(x1.narrow(home,lo1,hi1));
      break;
    }
    case FO_SHIFT_LEFT: {
      if (a >= x0.width()) {
        GECODE_ME_CHECK(x1.narrow(home,0,0));
        break;
      }
      const WordValue relevant = x0.mask() >> a;
      GECODE_ME_CHECK(x0.narrow(home,x0.lo() | (x1.lo() >> a),
                                x0.hi() & ((x1.hi() >> a) |
                                           (x0.mask() & ~relevant))));
      GECODE_ME_CHECK(x1.narrow(home,(x0.lo() << a) & x1.mask(),
                                (x0.hi() << a) & x1.mask()));
      break;
    }
    case FO_LOGICAL_SHIFT_RIGHT: {
      if (a >= x0.width()) {
        GECODE_ME_CHECK(x1.narrow(home,0,0));
        break;
      }
      const WordValue lost = low_mask(a);
      GECODE_ME_CHECK(x0.narrow(home,x0.lo() | (x1.lo() << a),
                                x0.hi() & ((x1.hi() << a) | lost)));
      GECODE_ME_CHECK(x1.narrow(home,x0.lo() >> a,x0.hi() >> a));
      break;
    }
    case FO_ARITHMETIC_SHIFT_RIGHT: {
      const unsigned int width = x0.width();
      const WordValue sign = WordValue(1) << (width-1);
      const WordValue mapped = (a >= width) ? 0 : (x1.mask() >> a);
      const WordValue extension = x1.mask() & ~mapped;
      WordValue lo0 = x0.lo();
      WordValue hi0 = x0.hi();
      if (a < width) {
        lo0 |= (x1.lo() & mapped) << a;
        hi0 &= ((x1.hi() & mapped) << a) | low_mask(a);
      }
      if ((x1.lo() & extension) != 0)
        lo0 |= sign;
      if ((x1.hi() & extension) != extension)
        hi0 &= ~sign;
      GECODE_ME_CHECK(x0.narrow(home,lo0,hi0));
      WordValue lo1 = (a >= width) ? 0 : (x0.lo() >> a);
      WordValue hi1 = (a >= width) ? 0 : (x0.hi() >> a);
      if ((x0.lo() & sign) != 0)
        lo1 |= extension;
      if ((x0.hi() & sign) != 0)
        hi1 |= extension;
      GECODE_ME_CHECK(x1.narrow(home,lo1,hi1));
      break;
    }
    case FO_ROTATE_LEFT: {
      GECODE_ME_CHECK(x0.narrow(home,
        rotate_right_value(x1.lo(),x1.width(),a),
        rotate_right_value(x1.hi(),x1.width(),a)));
      GECODE_ME_CHECK(x1.narrow(home,
        rotate_left_value(x0.lo(),x0.width(),a),
        rotate_left_value(x0.hi(),x0.width(),a)));
      break;
    }
    case FO_ROTATE_RIGHT: {
      GECODE_ME_CHECK(x0.narrow(home,
        rotate_left_value(x1.lo(),x1.width(),a),
        rotate_left_value(x1.hi(),x1.width(),a)));
      GECODE_ME_CHECK(x1.narrow(home,
        rotate_right_value(x0.lo(),x0.width(),a),
        rotate_right_value(x0.hi(),x0.width(),a)));
      break;
    }
    default:
      GECODE_NEVER;
    }
    return ES_OK;
  }

  template<class View0, class View1>
  forceinline bool
  Fixed<View0,View1>::done(View0 x0, View1 x1, FixedOp op,
                           unsigned int a, unsigned int) {
    if (op == FO_EXTRACT)
      return x1.assigned() &&
        ((x0.unknown() & (x1.mask() << a)) == 0);
    if (op == FO_SHIFT_LEFT) {
      const WordValue relevant = (a >= x0.width()) ? 0 : (x0.mask() >> a);
      return x1.assigned() && ((x0.unknown() & relevant) == 0);
    }
    if (op == FO_LOGICAL_SHIFT_RIGHT) {
      const WordValue relevant = (a >= x0.width()) ? 0 :
        (x0.mask() & ~low_mask(a));
      return x1.assigned() && ((x0.unknown() & relevant) == 0);
    }
    if (op == FO_ARITHMETIC_SHIFT_RIGHT) {
      const WordValue relevant = (a >= x0.width()) ?
        (WordValue(1) << (x0.width()-1)) :
        (x0.mask() & ~low_mask(a));
      return x1.assigned() && ((x0.unknown() & relevant) == 0);
    }
    return x0.assigned() && x1.assigned();
  }

  template<class View0, class View1>
  ExecStatus
  Fixed<View0,View1>::post(Home home, View0 x0, View1 x1,
                          FixedOp op, unsigned int a, unsigned int b) {
    GECODE_ES_CHECK(narrow(home,x0,x1,op,a,b));
    if (!done(x0,x1,op,a,b))
      (void) new (home) Fixed(home,x0,x1,op,a,b);
    return ES_OK;
  }

  template<class View0, class View1>
  Actor*
  Fixed<View0,View1>::copy(Space& home) {
    return new (home) Fixed(home,*this);
  }

  template<class View0, class View1>
  ExecStatus
  Fixed<View0,View1>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,op,a,b));
    return done(x0,x1,op,a,b) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
