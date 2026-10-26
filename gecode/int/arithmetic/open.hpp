/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
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

#ifndef GECODE_INT_ARITHMETIC_OPEN_HPP
#define GECODE_INT_ARITHMETIC_OPEN_HPP

#include <gecode/int/arithmetic.hh>
#include <gecode/int/open-propagator.hpp>

namespace Gecode { namespace Int { namespace Arithmetic {

  /// Track maxima; negated views implement min(x) = -max(-x)
  template<class View>
  class OpenMax : public OpenListPropagator<View> {
  protected:
    using OpenListPropagator<View>::list;
    using OpenListPropagator<View>::length;
    using OpenListPropagator<View>::processed;
    using OpenListPropagator<View>::closed;

    class Index : public Advisor {
    public:
      int i;

      Index(Space& home, Propagator& p, Council<Index>& c, int i0)
        : Advisor(home,p,c), i(i0) {}

      Index(Space& home, Index& a)
        : Advisor(home,a), i(a.i) {}
    };

    View result;
    IntPropLevel ipl;
    Council<Index> c;
    /// Maximum lower bound seen so far; bounds only increase
    int lower;
    /// Result upper bound last applied to all incorporated positions
    int upper;
    /// The result occurs in the list and supplies its own equality witness
    bool witness;

    OpenMax(Home home, OpenViewList<View> list,
               View result0, IntPropLevel ipl0)
      : OpenListPropagator<View>(home,list),
        result(result0), ipl(ipl0), c(home),
        lower(Limits::min), upper(result0.max()), witness(false) {
      result.subscribe(home,*this,PC_INT_BND);
    }

    OpenMax(Space& home, OpenMax& p)
      : OpenListPropagator<View>(home,p), ipl(p.ipl), c(home),
        lower(p.lower), upper(p.upper), witness(p.witness) {
      result.update(home,p.result);
      c.update(home,p.c);
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenMax(home,*this);
    }

    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      const int size = list.known_length();
      return PropCost::linear(PropCost::LO,
        ((closed() && !witness) || (result.max() < upper)) ?
        size : size-processed);
    }

    virtual ExecStatus
    advise(Space& home, Advisor& advisor, const Delta& d) {
      if (View::modevent(d) == ME_INT_DOM)
        return ES_FIX;
      Index& a = static_cast<Index&>(advisor);
      View x = list[a.i];
      lower = std::max(lower,x.min());
      if (x.assigned())
        a.dispose(home,c);
      return ((lower > result.min()) ||
              (witness && closed() && c.empty())) ? ES_NOFIX : ES_FIX;
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      if (result.max() < upper) {
        for (int i=0; i<processed; i++) {
          View x = list[i];
          GECODE_ME_CHECK(x.lq(home,result.max()));
        }
        upper = result.max();
      }
      for (int i=processed; i<size; i++) {
        View x = list[i];
        if (x == result) {
          witness = true;
          continue;
        }
        GECODE_ME_CHECK(x.lq(home,result.max()));
        lower = std::max(lower,x.min());
        if (!x.assigned())
          x.subscribe(home,*new (home) Index(home,*this,c,i));
      }
      processed = size;
      GECODE_ME_CHECK(result.gq(home,lower));
      // Future positions can supply the extremum. Enforce an equality witness
      // only once the list is closed, using the ordinary min/max propagator.
      if (!closed())
        return ES_FIX;
      if (size == 0)
        return ES_FAILED;
      if (witness)
        return (result.assigned() || c.empty()) ?
          home.ES_SUBSUMED(*this) : ES_FIX;
      ViewArray<View> variables(home,size);
      for (int i=0; i<size; i++)
        variables[i] = list[i];
      View y = result;
      if (vbd(ipl) == IPL_DOM)
        GECODE_REWRITE(*this,(Arithmetic::NaryMaxDom<View>
                             ::post(home(*this),variables,y)));
      GECODE_REWRITE(*this,(Arithmetic::NaryMaxBnd<View>
                           ::post(home(*this),variables,y)));
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator<View>::reschedule(home);
      result.reschedule(home,*this,PC_INT_BND);
    }

    virtual size_t
    dispose(Space& home) {
      if (!home.failed())
        for (Advisors<Index> as(c); as(); ++as) {
          View x = list[as.advisor().i];
          x.cancel(home,as.advisor());
        }
      c.dispose(home);
      result.cancel(home,*this,PC_INT_BND);
      OpenListPropagator<View>::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenViewList<View> list,
         View result, IntPropLevel ipl) {
      (void) new (home) OpenMax(home,list,result,ipl);
    }
  };

}}}

#endif

// STATISTICS: int-prop
