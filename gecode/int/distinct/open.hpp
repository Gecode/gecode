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

#ifndef GECODE_INT_DISTINCT_OPEN_HPP
#define GECODE_INT_DISTINCT_OPEN_HPP

#include <gecode/int/distinct.hh>
#include <gecode/int/open-propagator.hpp>
#include <gecode/int/growable-int-set.hh>
#include <limits>

namespace Gecode { namespace Int { namespace Distinct {

  template<class View>
  class Open : public OpenListPropagator<View> {
  protected:
    using OpenListPropagator<View>::list;
    using OpenListPropagator<View>::length;
    using OpenListPropagator<View>::processed;
    using OpenListPropagator<View>::closed;

    /// Variables whose assigned values have not yet been propagated
    View* active;
    int active_count;
    int active_capacity;
    /// Assigned values to exclude from appended variables
    GrowableIntSet assigned;

    Open(Home home, OpenViewList<View> list)
      : OpenListPropagator<View>(home,list),
        active(nullptr), active_count(0), active_capacity(0) {
      home.notice(*this,AP_DISPOSE);
    }

    Open(Space& home, Open& p)
      : OpenListPropagator<View>(home,p),
        active((p.active_count == 0) ? nullptr :
               home.alloc<View>(p.active_count)),
        active_count(p.active_count), active_capacity(p.active_count) {
      for (int i=0; i<active_count; i++)
        active[i].update(home,p.active[i]);
      assigned.update(home,p.assigned);
    }

    /// Grow geometrically and copy only active views
    void
    reserve_active(Space& home, int required) {
      if (required <= active_capacity)
        return;
      const int limit = static_cast<int>(std::min(
        static_cast<size_t>(std::numeric_limits<int>::max()),
        static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max()) /
          sizeof(View)));
      if (required > limit)
        throw MemoryExhausted();
      const int extra = std::min(active_capacity/2+1,limit-active_capacity);
      const int capacity = std::max(required,active_capacity+extra);
      View* views = home.alloc<View>(capacity);
      for (int i=0; i<active_count; i++)
        views[i] = active[i];
      if (active != nullptr)
        home.free<View>(active,active_capacity);
      active = views;
      active_capacity = capacity;
    }

    /// Exclude a batch from the selected suffix and collect all assignments
    template<class I>
    ExecStatus
    exclude_values(Space& home, I excluded, int first,
                   int* values, int& count) {
      count = 0;
      for (int i=active_count; i--; ) {
        if ((i >= first) && !active[i].assigned()) {
          I ranges(excluded);
          GECODE_ME_CHECK(active[i].minus_r(home,ranges,false));
        }
        if (active[i].assigned()) {
          values[count++] = active[i].val();
          active[i] = active[--active_count];
        }
      }
      return ES_OK;
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,
        active_count+(list.known_length()-processed));
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) Open(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      reserve_active(home,active_count+(size-processed));
      const int first_new = active_count;
      for (int i=processed; i<size; i++) {
        View x(list[i]);
        if (x.assigned() && assigned.in(x.val()))
          return ES_FAILED;
        active[active_count++] = x;
        if (!x.assigned())
          x.subscribe(home,*this,PC_INT_VAL,false);
      }
      processed = size;

      Region region;
      const int capacity = active_count;
      int* current = region.alloc<int>(capacity);
      int* next = region.alloc<int>(capacity);
      int count;
      GrowableIntSetRanges history(assigned);
      GECODE_ES_CHECK(exclude_values(home,history,first_new,current,count));

      typedef Iter::Values::ToRanges<Iter::Values::Array> BatchRanges;
      while (count > 0) {
        Support::quicksort(current,count);
        for (int i=1; i<count; i++)
          if (current[i-1] == current[i])
            return ES_FAILED;

        Iter::Values::Array values(current,count);
        BatchRanges batch(values);
        int next_count;
        GECODE_ES_CHECK(exclude_values(home,batch,0,next,next_count));
        assigned.add_r(home,batch);
        std::swap(current,next);
        count = next_count;
      }

      if (closed()) {
        ViewArray<View> x(home,active_count);
        for (int i=0; i<x.size(); i++)
          x[i] = active[i];
        GECODE_REWRITE(*this,Distinct::Val<View>::post(home(*this),x));
      }
      return ES_FIX;
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator<View>::reschedule(home);
      for (int i=0; i<active_count; i++)
        if (!active[i].assigned())
          active[i].reschedule(home,*this,PC_INT_VAL);
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      for (int i=0; i<active_count; i++)
        if (!active[i].assigned())
          active[i].cancel(home,*this,PC_INT_VAL);
      if (active != nullptr)
        home.free<View>(active,active_capacity);
      assigned.dispose(home);
      OpenListPropagator<View>::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenViewList<View> list) {
      (void) new (home) Open(home,list);
    }
  };

}}}

#endif

// STATISTICS: int-prop
