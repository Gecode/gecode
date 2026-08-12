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

namespace Gecode { namespace Word { namespace Element {

  class IndexValues {
  protected:
    const Int::IdxView<WordView>* cur;
    const Int::IdxView<WordView>* end;
  public:
    IndexValues(const Int::IdxView<WordView>* begin,
                const Int::IdxView<WordView>* end0)
      : cur(begin), end(end0) {}
    bool operator ()(void) const { return cur < end; }
    void operator ++(void) { cur++; }
    int val(void) const { return cur->idx; }
  };

  forceinline
  View::View(Home home, Int::IdxViewArray<WordView>& x0,
             Int::IntView i0, WordView y0)
    : Propagator(home), x(x0), i(i0), y(y0) {
    x.subscribe(home,*this,PC_WORD_BITS);
    i.subscribe(home,*this,Int::PC_INT_DOM);
    y.subscribe(home,*this,PC_WORD_BITS);
  }

  forceinline
  View::View(Space& home, View& p) : Propagator(home,p) {
    x.update(home,p.x);
    i.update(home,p.i);
    y.update(home,p.y);
  }

  forceinline Actor*
  View::copy(Space& home) {
    return new (home) View(home,*this);
  }

  forceinline PropCost
  View::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size()+2);
  }

  forceinline void
  View::reschedule(Space& home) {
    x.reschedule(home,*this,PC_WORD_BITS);
    i.reschedule(home,*this,Int::PC_INT_DOM);
    y.reschedule(home,*this,PC_WORD_BITS);
  }

  forceinline size_t
  View::dispose(Space& home) {
    x.cancel(home,*this,PC_WORD_BITS);
    i.cancel(home,*this,Int::PC_INT_DOM);
    y.cancel(home,*this,PC_WORD_BITS);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  forceinline ExecStatus
  View::post(Home home, Int::IdxViewArray<WordView>& x,
             Int::IntView i, WordView y) {
    GECODE_ME_CHECK(i.gq(home,0));
    GECODE_ME_CHECK(i.lq(home,x.size()-1));
    if (i.assigned())
      return Rel::Eq<WordView,WordView>::post(home,x[i.val()].view,y);
    (void) new (home) View(home,x,i,y);
    return ES_OK;
  }

  forceinline ExecStatus
  View::propagate(Space& home, const ModEventDelta&) {
    int n = 0;
    for (int j=0; j<x.size(); j++) {
      const bool disjoint = ((x[j].view.lo() & ~y.hi()) != 0) ||
        ((y.lo() & ~x[j].view.hi()) != 0);
      if (!i.in(x[j].idx) || disjoint) {
        x[j].view.cancel(home,*this,PC_WORD_BITS);
      } else {
        x[n++] = x[j];
      }
    }
    if (n == 0)
      return ES_FAILED;
    if (n < x.size()) {
      x.size(n);
      IndexValues values(&x[0],&x[0]+x.size());
      GECODE_ME_CHECK(i.narrow_v(home,values,false));
    }
    if (x.size() == 1)
      GECODE_REWRITE(*this,(Rel::Eq<WordView,WordView>::post(
        home(*this),x[0].view,y)));

    WordValue lo = x[0].view.lo();
    WordValue hi = x[0].view.hi();
    for (int j=1; j<x.size(); j++) {
      lo &= x[j].view.lo();
      hi |= x[j].view.hi();
    }
    GECODE_ME_CHECK(y.narrow(home,lo,hi));
    if (y.assigned() && (lo == hi))
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
