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

#ifndef GECODE_INT_MIN_DISTANCE_HH
#define GECODE_INT_MIN_DISTANCE_HH

#include <gecode/int.hh>

#include <algorithm>

/**
 * \namespace Gecode::Int::MinDistance
 *
 * The forward-bound and greedy conflict-matching algorithms follow:
 *   M. Z. Lagerkvist, Propagation Algorithms for the Minimum-Distance
 *   Constraint over Selected Points, ModRef 2026.
 *   https://github.com/zayenz/modref2026-minimum-distance-propagators
 *
 * \brief Minimum-distance propagators over indexed sites
 */
namespace Gecode { namespace Int { namespace MinDistance {

  /// Pair maximum and its witness
  struct PairCache {
    int a;     ///< Site at the first endpoint
    int b;     ///< Site at the second endpoint
    int upper; ///< Maximum distance between the endpoint domains
  };

  /**
   * \brief Forward-bound actor for one selected-position pair
   *
   * Requires \code #include <gecode/int/min-distance.hh> \endcode
   * \ingroup FuncIntProp
   */
  class Pair
    : public MixTernaryPropagator<IntView,PC_INT_DOM,IntView,PC_INT_DOM,
                                 IntView,PC_INT_BND> {
  protected:
    /// Shared distances
    IntDistance d;
    /// Inclusive pair requirement
    int r;
    /// Constructor for posting
    Pair(Home home, IntView x0, IntView y0, IntView z0,
         const IntDistance& d0, int r0);
    /// Constructor for cloning
    Pair(Space& home, Pair& p);
  public:
    /// Post propagator
    static ExecStatus post(Home home, IntView x0, IntView x1, IntView x2,
                           const IntDistance& d, int r);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Cost function
    virtual PropCost cost(const Space&, const ModEventDelta&) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&);
    /// Cancel subscriptions and dispose shared data
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Equality finalizer and zero witness for the pairwise organization
   *
   * Requires \code #include <gecode/int/min-distance.hh> \endcode
   * \ingroup FuncIntProp
   */
  class Finalize
    : public MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND> {
  protected:
    /// Shared distances
    IntDistance d;
    /// Shared pair requirements
    IntSharedArray r;
    /// Constructor for posting
    Finalize(Home home, ViewArray<IntView>& x, IntView z,
             const IntDistance& d0, const IntSharedArray& r0);
    /// Constructor for cloning
    Finalize(Space& home, Finalize& p);
  public:
    /// Post propagator
    static ExecStatus post(Home home, ViewArray<IntView>& x, IntView z,
                           const IntDistance& d, const IntSharedArray& r);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Cost function
    virtual PropCost cost(const Space&, const ModEventDelta&) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&);
    /// Cancel subscriptions and dispose shared data
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Single forward-bound actor with domain advisors and pair witnesses
   *
   * Requires \code #include <gecode/int/min-distance.hh> \endcode
   * \ingroup FuncIntProp
   */
  class AdvisorForward : public Propagator {
  protected:
    /// Domain advisor for a selected position
    class XAdvisor : public Advisor {
    public:
      /// Position in the view array
      int index;
      /// Constructor for creation
      XAdvisor(Space& home, Propagator& p, Council<XAdvisor>& c, int i);
      /// Constructor for cloning
      XAdvisor(Space& home, XAdvisor& a);
    };
    /// Selected sites
    ViewArray<IntView> x;
    /// Minimum distance
    IntView z;
    /// Shared distances
    IntDistance d;
    /// Shared pair requirements
    IntSharedArray r;
    /// Domain advisors
    Council<XAdvisor> council;
    /// Pair maxima and witnesses
    PairCache* cache;
    /// Positions changed since the last fixpoint
    Support::BitSetBase dirty;
    /// Objective lower bound at the last fixpoint
    int last_min;
    /// Whether the objective shares a selected-site variable
    bool z_shared;
    /// Constructor for posting
    AdvisorForward(Home home, ViewArray<IntView>& x0, IntView z0,
                   const IntDistance& d0, const IntSharedArray& r0);
    /// Constructor for cloning
    AdvisorForward(Space& home, AdvisorForward& p);
  public:
    /// Post propagator
    static ExecStatus post(Home home, ViewArray<IntView>& x, IntView z,
                           const IntDistance& d, const IntSharedArray& r);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Cost function
    virtual PropCost cost(const Space&, const ModEventDelta&) const;
    /// Reschedule propagation
    virtual void reschedule(Space& home);
    /// Record a changed selected-site domain
    virtual ExecStatus advise(Space&, Advisor& a, const Delta&);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&);
    /// Cancel subscriptions and dispose stored data
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Independent matching-based objective upper bound
   *
   * Requires \code #include <gecode/int/min-distance.hh> \endcode
   * \ingroup FuncIntProp
   */
  class Matching
    : public MixNaryOnePropagator<IntView,PC_INT_DOM,IntView,PC_INT_BND> {
  protected:
    /// Shared distances
    IntDistance d;
    /// Constructor for posting
    Matching(Home home, ViewArray<IntView>& x, IntView z,
             const IntDistance& d0);
    /// Constructor for cloning
    Matching(Space& home, Matching& p);
    /// Whether a greedy conflict matching refutes the threshold
    bool refuted(int* sites, int n, bool* matched, int threshold) const;
  public:
    /// Post propagator
    static ExecStatus post(Home home, ViewArray<IntView>& x, IntView z,
                           const IntDistance& d);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Cost function
    virtual PropCost cost(const Space&, const ModEventDelta&) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&);
    /// Cancel subscriptions and dispose shared data
    virtual size_t dispose(Space& home);
  };

}}}

#include <gecode/int/min-distance/prop.hpp>

#endif

// STATISTICS: int-prop
