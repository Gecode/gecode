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

#ifndef GECODE_INT_REL_OPEN_HPP
#define GECODE_INT_REL_OPEN_HPP

#include <gecode/int/rel.hh>
#include <gecode/int/open-propagator.hpp>

namespace Gecode { namespace Int { namespace Rel {

  /// Propagate a relation over an open chain in one actor
  template<class View, IntRelType irt>
  class Open : public OpenListPropagator<View> {
  protected:
    using OpenListPropagator<View>::list;
    using OpenListPropagator<View>::processed;
    using OpenListPropagator<View>::closed;

    /// Identify the position whose domain changed
    class Position : public Advisor {
    public:
      int index;

      Position(Space& home, Propagator& p, Council<Position>& council,
               int index0)
        : Advisor(home,p,council), index(index0) {}

      Position(Space& home, Position& a)
        : Advisor(home,a), index(a.index) {}

      void
      dispose(Space& home, Council<Position>& council) {
        Advisor::dispose(home,council);
      }
    };

    Council<Position> council;
    /// Inclusive range of positions needing propagation
    int first;
    int last;
    /// Whether equality propagates holes as well as bounds
    bool domain;

    Open(Home home, OpenViewList<View> list, IntPropLevel ipl)
      : OpenListPropagator<View>(home,list), council(home),
        first(Int::Limits::max), last(-1),
        domain((vbd(ipl) == IPL_DOM) || (vbd(ipl) == IPL_DEF)) {}

    Open(Space& home, Open& p)
      : OpenListPropagator<View>(home,p), council(home),
        first(p.first), last(p.last), domain(p.domain) {
      council.update(home,p.council);
    }

    void
    changed(int index) {
      first = std::min(first,index);
      last = std::max(last,index);
    }

    /// Read decreasing relations in increasing order
    View
    ordered(int index, int size) const {
      return list[((irt == IRT_GQ) || (irt == IRT_GR))
                  ? size-1-index : index];
    }

    /// Accumulate the domain intersection in the first position
    ExecStatus
    propagate_equal(Space& home, int begin, int end, int size) {
      View common(list[0]);
      const unsigned int old_size = common.size();
      for (int i=begin; i<=end; i++) {
        View x(list[i]);
        if (x == common)
          continue;
        if (domain) {
          ViewRanges<View> ranges(x);
          GECODE_ME_CHECK(common.inter_r(home,ranges,false));
        } else {
          GECODE_ME_CHECK(common.gq(home,x.min()));
          GECODE_ME_CHECK(common.lq(home,x.max()));
        }
      }
      // If the common domain is unchanged, update only the changed positions.
      if ((begin == 0) || (common.size() != old_size)) {
        begin = 1;
        end = size-1;
      }
      for (int i=begin; i<=end; i++) {
        View x(list[i]);
        if (x == common)
          continue;
        if (domain) {
          ViewRanges<View> ranges(common);
          GECODE_ME_CHECK(x.inter_r(home,ranges,false));
        } else {
          GECODE_ME_CHECK(x.gq(home,common.min()));
          GECODE_ME_CHECK(x.lq(home,common.max()));
        }
      }
      return ES_OK;
    }

    /// Propagate assignments to their immediate neighbors
    ExecStatus
    propagate_different(Space& home, int begin, int end, int size) {
      for (int i=begin; i<=end; i++) {
        View x(list[i]);
        if (!x.assigned())
          continue;
        if (i > 0)
          GECODE_ME_CHECK(list[i-1].nq(home,x.val()));
        if (i+1 < size)
          GECODE_ME_CHECK(list[i+1].nq(home,x.val()));
      }
      return ES_OK;
    }

    /// Sweep lower bounds forward and upper bounds backward from changed positions
    ExecStatus
    propagate_order(Space& home, int begin, int end, int size) {
      const int offset = ((irt == IRT_LE) || (irt == IRT_GR)) ? 1 : 0;
      if constexpr ((irt == IRT_GQ) || (irt == IRT_GR)) {
        const int old_begin = begin;
        begin = size-1-end;
        end = size-1-old_begin;
      }
      for (int i=std::max(1,begin); i<size; i++) {
        ModEvent me = ordered(i,size).gq(home,ordered(i-1,size).min()+offset);
        if (me_failed(me))
          return ES_FAILED;
        if ((i > end) && !me_modified(me))
          break;
      }
      for (int i=std::min(size-2,end); i>=0; i--) {
        ModEvent me = ordered(i,size).lq(home,ordered(i+1,size).max()-offset);
        if (me_failed(me))
          return ES_FAILED;
        if ((i < begin) && !me_modified(me))
          break;
      }
      return ES_OK;
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) Open(home,*this);
    }

    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,list.known_length());
    }

    virtual ExecStatus
    advise(Space& home, Advisor& advisor, const Delta& delta) {
      Position& a = static_cast<Position&>(advisor);
      View x(list[a.index]);
      if constexpr (irt == IRT_NQ) {
        if (!x.assigned())
          return ES_FIX;
      } else if (((irt != IRT_EQ) || !domain) &&
                 (View::modevent(delta) == ME_INT_DOM)) {
        return ES_FIX;
      }
      changed(a.index);
      if (x.assigned())
        return home.ES_NOFIX_DISPOSE(council,a);
      return ES_NOFIX;
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      if (processed < size) {
        changed(std::max(0,processed-1));
        changed(size-1);
      }
      for (int i=processed; i<size; i++) {
        View x(list[i]);
        if (((irt == IRT_NQ) || (irt == IRT_LE) || (irt == IRT_GR)) &&
            (i > 0) && (x == list[i-1]))
          return ES_FAILED;
        if (!x.assigned())
          x.subscribe(home,*new (home) Position(home,*this,council,i));
      }
      processed = size;

      // Advisors record further changes caused by sparse bounds or aliases.
      while (first <= last) {
        const int begin = first;
        const int end = last;
        first = Int::Limits::max;
        last = -1;
        if constexpr (irt == IRT_EQ) {
          GECODE_ES_CHECK(propagate_equal(home,begin,end,size));
        } else if constexpr (irt == IRT_NQ) {
          GECODE_ES_CHECK(propagate_different(home,begin,end,size));
        } else {
          GECODE_ES_CHECK(propagate_order(home,begin,end,size));
        }
      }
      return (closed() && ((size < 2) || council.empty()))
        ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator<View>::reschedule(home);
      first = 0;
      last = list.known_length()-1;
      View::schedule(home,*this,ME_INT_DOM);
    }

    virtual size_t
    dispose(Space& home) {
      if (!home.failed())
        for (Advisors<Position> a(council); a(); ++a)
          list[a.advisor().index].cancel(home,a.advisor());
      council.dispose(home);
      OpenListPropagator<View>::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenViewList<View> list, IntPropLevel ipl) {
      (void) new (home) Open(home,list,ipl);
    }
  };

}}}

#endif

// STATISTICS: int-prop
