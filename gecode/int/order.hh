/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2019
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

#ifndef __GECODE_INT_ORDER_HH__
#define __GECODE_INT_ORDER_HH__

#include <gecode/int.hh>

/**
 * \namespace Gecode::Int::Order
 *
 * \brief %Int for ordering two tasks
 */

namespace Gecode { namespace Int { namespace Order {

  /**
   * \brief %Scheduling propagator for ordering two tasks
   *
   * Requires \code #include <gecode/int/order.hh> \endcode
   * \ingroup FuncIntProp
   */
  class OrderProp : public Propagator {
  protected:
    /// View for start times
    IntView s0, s1;
    /// Boolean view for order
    BoolView b;
    /// Processing times
    int p0, p1;
    /// Constructor for cloning
    OrderProp(Space& home, OrderProp& p);
    /// Constructor for creation
    OrderProp(Home home, IntView s0, int p0, IntView s1, int p1,
              BoolView b);
  public:
    /// Copy propagator during cloning
    GECODE_INT_EXPORT
    virtual Actor* copy(Space& home);
    /// Cost function (defined as low binary)
    GECODE_INT_EXPORT
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    GECODE_INT_EXPORT
    virtual void reschedule(Space& home);
    /// Perform propagation
    GECODE_INT_EXPORT
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post order propagator
    static ExecStatus post(Home home, IntView s0, int p0, IntView s1, int p1,
                           BoolView b);
    /// Delete propagator and return its size
    GECODE_INT_EXPORT
    virtual size_t dispose(Space& home);
  };

}}}

#include <gecode/int/order/propagate.hpp>

#endif

// STATISTICS: int-prop
