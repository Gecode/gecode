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

#ifndef GECODE_INT_OPEN_PROPAGATOR_HPP
#define GECODE_INT_OPEN_PROPAGATOR_HPP

#include <gecode/int.hh>

namespace Gecode { namespace Int {

  /// Follow list growth and remember which positions have been incorporated
  template<class View>
  class OpenListPropagator : public Propagator {
  protected:
    OpenViewList<View> list;
    IntView length;
    int processed;

    OpenListPropagator(Home home, OpenViewList<View> list0)
      : Propagator(home), list(list0),
        length(list0.length()),
        processed(0) {
      length.subscribe(home,*this,PC_INT_BND);
      list.subscribe(home,*this);
    }

    OpenListPropagator(Space& home, OpenListPropagator& p)
      : Propagator(home,p), processed(p.processed) {
      list.update(home,p.list);
      length.update(home,p.length);
    }

    bool
    closed(void) const {
      return length.max() == list.known_length();
    }

    void
    dispose_base(Space& home) {
      length.cancel(home,*this,PC_INT_BND);
      list.cancel(home,*this);
      list.~OpenViewList();
      (void) Propagator::dispose(home);
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,list.known_length()-processed);
    }

    virtual void
    reschedule(Space& home) {
      length.reschedule(home,*this,PC_INT_BND);
      list.reschedule(home,*this);
    }
  };

}}

#endif

// STATISTICS: int-prop
