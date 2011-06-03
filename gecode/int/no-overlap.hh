/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2011
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

#ifndef __GECODE_INT_NO_OVERLAP_HH__
#define __GECODE_INT_NO_OVERLAP_HH__

#include <gecode/int.hh>

/**
 * \namespace Gecode::Int::NoOverlap
 * \brief %No-overlap propagators
 */

namespace Gecode { namespace Int { namespace NoOverlap {

  /**
   * \brief Dimension combining coordinate and integer size information
   */
  class IntDim {
  protected:
    /// Coordinate
    IntView c;
    /// Size
    int s;
    /// Modify smallest start coordinate
    ExecStatus ssc(Space& home, int n);
    /// Modify largest end coordinate
    ExecStatus lec(Space& home, int n);
    /// Dimension must not overlap with coordinates \a n to \a m
    ExecStatus nooverlap(Space& home, int n, int m);
  public:
    /// Default constructor
    IntDim(void);
    /// Constructor
    IntDim(IntView c, int s);

    /// Return smallest start coordinate
    int ssc(void) const;
    /// Return largest start coordinate
    int lsc(void) const;
    /// Return smallest end coordinate
    int sec(void) const;
    /// Return largest end coordinate
    int lec(void) const;

    /// Whether dimension is mandatory
    bool mandatory(void) const;
    /// Whether dimension is empty
    bool empty(void) const;

    /// Dimension must not overlap with \a d
    ExecStatus nooverlap(Space& home, IntDim& d);

    /// Update dimension during cloning
    void update(Space& home, bool share, IntDim& d);

    /// Subscribe propagator \a p to dimension
    void subscribe(Space& home, Propagator& p);
    /// Cancel propagator \a p from dimension
    void cancel(Space& home, Propagator& p);
  };

  /**
   * \brief Dimension combining coordinate and integer view size information
   */
  class ViewDim {
  protected:
    /// Coordinate
    IntView c;
    /// Size
    IntView s;
    /// Modify smallest start coordinate
    ExecStatus ssc(Space& home, int n);
    /// Modify largest end coordinate
    ExecStatus lec(Space& home, int n);
    /// Dimension must not overlap with coordinates \a n to \a m
    ExecStatus nooverlap(Space& home, int n, int m);
  public:
    /// Default constructor
    ViewDim(void);
    /// Constructor
    ViewDim(IntView c, IntView s);

    /// Return smallest start coordinate
    int ssc(void) const;
    /// Return largest start coordinate
    int lsc(void) const;
    /// Return smallest end coordinate
    int sec(void) const;
    /// Return largest end coordinate
    int lec(void) const;

    /// Whether dimension is mandatory
    bool mandatory(void) const;
    /// Whether dimension is empty
    bool empty(void) const;

    /// Dimension must not overlap with \a d
    ExecStatus nooverlap(Space& home, ViewDim& d);

    /// Update dimension during cloning
    void update(Space& home, bool share, ViewDim& d);

    /// Subscribe propagator \a p to dimension
    void subscribe(Space& home, Propagator& p);
    /// Subscribe advisor \a a to dimension
    void subscribe(Space& home, Advisor& a);
    /// Cancel propagator \a p from dimension
    void cancel(Space& home, Propagator& p);
    /// Cancel advisor \a a from dimension
    void cancel(Space& home, Advisor& a);
  };

}}}

#include <gecode/int/no-overlap/dim.hpp>

namespace Gecode { namespace Int { namespace NoOverlap {

  /**
   * \brief Box class
   */
  template<class Dim, int n>
  class Box {
  protected:
    /// Dimensions
    Dim d[n];
  public:
    /// Access to dimension \a i
    const Dim& operator [](int i) const;
    /// Access to dimension \a i
    Dim& operator [](int i);

    /// Whether box is mandatory
    bool mandatory(void) const;
    /// Whether box is empty
    bool empty(void) const;

    /// Check whether this box does not any longer overlap with \a b
    bool nooverlap(const Box<Dim,n>& b) const;
    /// Propagate that this box does not overlap with \a b
    ExecStatus nooverlap(Space& home, Box<Dim,n>& b);

    /// Update box during cloning
    void update(Space& home, bool share, Box<Dim,n>& r);

    /// Subscribe propagator \a p to box
    void subscribe(Space& home, Propagator& p);
    /// Subscribe advisor \a a to box
    void subscribe(Space& home, Advisor& a);
    /// Cancel propagator \a p from box
    void cancel(Space& home, Propagator& p);
    /// Cancel advisor \a a from box
    void cancel(Space& home, Advisor& a);
  };

}}}

#include <gecode/int/no-overlap/box.hpp>

namespace Gecode { namespace Int { namespace NoOverlap {

  /**
   * \brief Base class for no-overlap propagator
   *
   * Requires \code #include <gecode/int/no-overlap.hh> \endcode
   *
   * \ingroup FuncIntProp
   */
  template<class Dim, int d>
  class Base : public Propagator {
  protected:
    /// Boxes of dimension \a d
    Box<Dim,d>* b;
    /// Number of mandatory boxes: b[0] ... b[n-1]
    int n;
    /// Constructor for posting with \a n mandatory boxes
    Base(Home home, Box<Dim,d>* b, int n);
    /// Constructor for cloning \a p with \a m boxes
    Base(Space& home, bool share, Base<Dim,d>& p, int m);
    /**
     * \brief Partition \a n boxes \a b starting at position \a i
     *
     * Returns the number of mandatory boxes at the front of \a b.
     */
    static int partition(Box<Dim,d>* b, int i, int n);
  public:
    /// Cost function
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Destructor
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief No-overlap propagator for integer sized dimensions
   *
   * Requires \code #include <gecode/int/no-overlap.hh> \endcode
   *
   * \ingroup FuncIntProp
   */
  template<int d>
  class Int : public Base<IntDim,d> {
  protected:
    using Base<IntDim,d>::b;
    using Base<IntDim,d>::n;
    /// Constructor for posting
    Int(Home home, Box<IntDim,d>* b, int n);
    /// Constructor for cloning \a p
    Int(Space& home, bool share, Int<d>& p);
  public:
    /// Post propagator for boxes \a b
    static ExecStatus post(Home home, Box<IntDim,d>* b, int n);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Destructor
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief No-overlap propagator for integer view sized dimensions
   *
   * Requires \code #include <gecode/int/no-overlap.hh> \endcode
   *
   * \ingroup FuncIntProp
   */
  template<int d>
  class View : public Base<ViewDim,d> {
  protected:
    using Base<ViewDim,d>::b;
    using Base<ViewDim,d>::n;
    /// Number of optional boxes: b[n] ... b[n+m-1]
    int m;
    /// The advisor council
    Council<Advisor> c;
    /// What the propagator must do (controlled by advisor)
    enum {
      /// Nothing to do but propagation
      NOTHING,
      /// Check whether a box has become mandatory
      MANDATORY,
      /// Also check whether a box must be eliminated (as it is zero)
      ELIMINATE
    } todo;
    /// Constructor for posting
    View(Home home, Box<ViewDim,d>* b, int n, int m);
    /// Constructor for cloning \a p
    View(Space& home, bool share, View<d>& p);
    /// Return single advisor
    Advisor& advisor(void);
  public:
    /// Post propagator for boxes \a b
    static ExecStatus post(Home home, Box<ViewDim,d>* b, int n);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Destructor
    virtual size_t dispose(Space& home);
  };

}}}

#include <gecode/int/no-overlap/base.hpp>
#include <gecode/int/no-overlap/int.hpp>
#include <gecode/int/no-overlap/view.hpp>

#endif

// STATISTICS: int-prop

