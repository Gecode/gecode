/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Claude-Guy Quimper
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Claude-Guy Quimper, 2006
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

#ifndef GECODE_INT_INTER_DISTANCE_HH
#define GECODE_INT_INTER_DISTANCE_HH

#include <gecode/int.hh>

/**
 * \namespace Gecode::Int::InterDistance
 * \brief Inter-distance propagator
 */
namespace Gecode { namespace Int { namespace InterDistance {

  /**
   * \brief Bounds-consistent inter-distance propagator
   *
   * Based on Claude-Guy Quimper, Alejandro Lopez-Ortiz, and Gilles Pesant,
   * A Quadratic Propagator for the Inter-Distance Constraint,
   * AAAI 2006, 123--128, and Quimper's 2006 implementation.
   *
   * Only the views and distance are kept across invocations. Quadratic
   * scratch storage is allocated in a region and is not copied on cloning.
   *
   * Requires \code #include <gecode/int/inter-distance.hh> \endcode
   * \ingroup FuncIntProp
   */
  class Bnd : public NaryPropagator<IntView,PC_INT_BND> {
  protected:
    /// Minimum distance
    int p;
    /// Constructor for posting
    Bnd(Home home, ViewArray<IntView>& x, int p);
    /// Constructor for cloning
    Bnd(Space& home, Bnd& b);
  public:
    /// Copy during cloning
    virtual Propagator* copy(Space& home);
    /// Quadratic propagation cost
    virtual PropCost cost(const Space&, const ModEventDelta&) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&);
    /// Cancel subscriptions and dispose
    virtual size_t dispose(Space& home);
    /// Post propagator
    static ExecStatus post(Home home, ViewArray<IntView>& x, int p);
  };

}}}

#endif

// STATISTICS: int-prop
