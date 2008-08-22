/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

namespace Gecode { namespace Set { namespace Element {

  forceinline
  ElementDisjoint::ElementDisjoint(Space& home,
                                   IdxViewArray<SetView>& iv0,
                                   SetView y1)
    : Propagator(home), iv(iv0), x1(y1) {

    x1.subscribe(home,*this, PC_SET_ANY);
    iv.subscribe(home,*this, PC_SET_ANY);
  }

  forceinline
  ElementDisjoint::ElementDisjoint(Space& home, bool share, ElementDisjoint& p)
    : Propagator(home,share,p) {
    x1.update(home,share,p.x1);
    iv.update(home,share,p.iv);
  }

  forceinline ExecStatus
  ElementDisjoint::post(Space& home, IdxViewArray<SetView>& xs,
                       SetView x1) {
    int n = xs.size();

    // s2 \subseteq {0,...,n-1}
    Iter::Ranges::Singleton s(0, n-1);
    GECODE_ME_CHECK(x1.intersectI(home,s));
    (void) new (home)
      ElementDisjoint(home,xs,x1);
    return ES_OK;
  }


}}}

// STATISTICS: set-prop
