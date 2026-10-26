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

#ifndef GECODE_INT_PRECEDE_OPEN_HPP
#define GECODE_INT_PRECEDE_OPEN_HPP

#include <gecode/int/precede.hh>
#include <gecode/int/open-propagator.hpp>

namespace Gecode { namespace Int { namespace Precede {

  template<class View>
  class Open : public OpenListPropagator<View> {
  protected:
    using OpenListPropagator<View>::list;
    using OpenListPropagator<View>::length;
    using OpenListPropagator<View>::processed;
    using OpenListPropagator<View>::closed;

    /// Keep the view as well as its position for independent disposal
    class Index : public Advisor {
    public:
      View x;
      int i;

      Index(Space& home, Propagator& p, Council<Index>& c,
            View x0, int i0)
        : Advisor(home,p,c), x(x0), i(i0) {}

      Index(Space& home, Index& a)
        : Advisor(home,a), i(a.i) {
        x.update(home,a.x);
      }
    };

    Council<Index> council;
    int before;
    int after;
    /// First and second positions that can still take before
    int alpha;
    int beta;
    /// First position assigned after, or infinity while none is known
    int gamma;

    Open(Home home, OpenViewList<View> list,
                int before0, int after0)
      : OpenListPropagator<View>(home,list), council(home),
        before(before0), after(after0), alpha(0), beta(0),
        gamma(Limits::max) {}

    Open(Space& home, Open& p)
      : OpenListPropagator<View>(home,p), council(home),
        before(p.before), after(p.after),
        alpha(p.alpha), beta(p.beta), gamma(p.gamma) {
      council.update(home,p.council);
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,list.known_length()-alpha);
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) Open(home,*this);
    }

    virtual ExecStatus
    advise(Space& home, Advisor& a0, const Delta&) {
      Index& a = static_cast<Index&>(a0);
      const int i = a.i;
      bool changed = false;
      if (a.x.assigned()) {
        if ((a.x.val() == after) && (i < gamma)) {
          gamma = i;
          changed = true;
        }
        changed |= (i == alpha) || (i == beta);
        a.dispose(home,council);
      } else if ((i < alpha) || (i > gamma)) {
        a.x.cancel(home,a);
        a.dispose(home,council);
      } else {
        changed = ((i == alpha) || (i == beta)) && !a.x.in(before);
      }
      return changed ? ES_NOFIX : ES_FIX;
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      if (before == after) {
        for (int i=processed; i<size; i++)
          GECODE_ME_CHECK(list[i].nq(home,before));
        processed = size;
        return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
      }

      for (int i=processed; i<size; i++) {
        View x(list[i]);
        if (x.assigned()) {
          if (x.val() == after)
            gamma = std::min(gamma,i);
        } else if (i <= gamma) {
          x.subscribe(home,*new (home) Index(home,*this,council,x,i));
        }
      }
      processed = size;

      while ((alpha < size) && !list[alpha].in(before)) {
        GECODE_ME_CHECK(list[alpha].nq(home,after));
        alpha++;
      }
      if (alpha == size) {
        beta = size;
        return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
      }
      View first(list[alpha]);
      GECODE_ME_CHECK(first.nq(home,after));
      if (first.assigned())
        return home.ES_SUBSUMED(*this);

      // An exhausted cursor resumes at the previous end after growth.
      beta = std::max(beta,alpha+1);
      while ((beta < size) && !list[beta].in(before))
        beta++;
      if (beta > gamma) {
        GECODE_ME_CHECK(first.eq(home,before));
        return home.ES_SUBSUMED(*this);
      }
      return ES_FIX;
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator<View>::reschedule(home);
      View::schedule(home,*this,ME_INT_DOM);
    }

    virtual size_t
    dispose(Space& home) {
      for (Advisors<Index> as(council); as(); ++as)
        as.advisor().x.cancel(home,as.advisor());
      council.dispose(home);
      OpenListPropagator<View>::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenViewList<View> list, int before, int after) {
      (void) new (home) Open(home,list,before,after);
    }
  };

}}}

#endif

// STATISTICS: int-prop
