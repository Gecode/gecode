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

#include <gecode/word/channel.hh>

namespace Gecode { namespace Word { namespace Channel {

  Actor*
  Bit::copy(Space& home) {
    return new (home) Bit(home,*this);
  }

  ExecStatus
  Bit::propagate(Space& home, const ModEventDelta&) {
    if ((x0.lo() & bit_mask) != 0) {
      GECODE_ME_CHECK(x1.one(home));
    } else if ((x0.hi() & bit_mask) == 0) {
      GECODE_ME_CHECK(x1.zero(home));
    } else if (x1.one()) {
      GECODE_ME_CHECK(x0.narrow(home,x0.lo() | bit_mask,x0.hi()));
    } else if (x1.zero()) {
      GECODE_ME_CHECK(x0.narrow(home,x0.lo(),x0.hi() & ~bit_mask));
    } else {
      return ES_FIX;
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
