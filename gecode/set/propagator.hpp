/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004, 2005
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

namespace Gecode {

  /**
   * \defgroup TaskPropSetPat Patterns for set propagators
   *
   * The optional last Boolean argument to the constructor for creation
   * defines whether deletion must be forced.
   * \ingroup TaskActor
   */

  //@{


  /**
   * \brief Set/Int connection propagator
   *
   * Stores single view of type \a View with propagation condition \a pcs
   * and an integer variable with propagation condition \a pci.
   */
  template <class View, PropCond pcs, PropCond pci>
  class IntSetPropagator : public Propagator {
  protected:
    View x0;
    Gecode::Int::IntView x1;
    /// Constructor for cloning
    IntSetPropagator(Space& home,bool,IntSetPropagator&);
    /// Constructor for creation
    IntSetPropagator(Space& home,View,Gecode::Int::IntView);
  public:
    /// Cost function (defined as low binary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Set/Int reified connection propagator
   *
   * Stores a single view of type \a View with propagation condition \a pcs
   * and an integer variable with propagation condition \a pci.
   */
  template <class View, PropCond pcs, PropCond pci>
  class IntSetRePropagator : public Propagator {
  protected:
    View x0;
    Gecode::Int::IntView x1;
    Gecode::Int::BoolView b;
    /// Constructor for cloning
    IntSetRePropagator(Space& home,bool,IntSetRePropagator&);
    /// Constructor for creation
    IntSetRePropagator(Space& home,View,Gecode::Int::IntView,
                       Gecode::Int::BoolView);
  public:
    /// Cost function (defined as low ternary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  //@}

  template <class View, PropCond pcs, PropCond pci>
  IntSetPropagator<View,pcs,pci>::IntSetPropagator
  (Space& home, View y0, Gecode::Int::IntView y1)
    : Propagator(home), x0(y0), x1(y1) {
    x0.subscribe(home,*this,pcs);
    x1.subscribe(home,*this,pci);
  }

  template <class View, PropCond pcs, PropCond pci>
  forceinline
  IntSetPropagator<View,pcs,pci>::IntSetPropagator
  (Space& home, bool share, IntSetPropagator<View,pcs,pci>& p)
    : Propagator(home,share,p) {
    x0.update(home,share,p.x0);
    x1.update(home,share,p.x1);
  }

  template <class View, PropCond pcs, PropCond pci>
  PropCost
  IntSetPropagator<View,pcs,pci>::cost(const Space&, 
                                       const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template <class View, PropCond pcs, PropCond pci>
  size_t
  IntSetPropagator<View,pcs,pci>::dispose(Space& home) {
    if (!home.failed()) {
      x0.cancel(home,*this,pcs);
      x1.cancel(home,*this,pci);
    }
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template <class View, PropCond pcs, PropCond pci>
  IntSetRePropagator<View,pcs,pci>::IntSetRePropagator
  (Space& home, View y0, Gecode::Int::IntView y1, Gecode::Int::BoolView b2)
    : Propagator(home), x0(y0), x1(y1), b(b2) {
    x0.subscribe(home,*this,pcs);
    x1.subscribe(home,*this,pci);
    b.subscribe(home,*this,Gecode::Int::PC_INT_VAL);
  }

  template <class View, PropCond pcs, PropCond pci>
  forceinline
  IntSetRePropagator<View,pcs,pci>::IntSetRePropagator
  (Space& home, bool share, IntSetRePropagator<View,pcs,pci>& p)
    : Propagator(home,share,p) {
    x0.update(home,share,p.x0);
    x1.update(home,share,p.x1);
    b.update(home,share,p.b);
  }

  template <class View, PropCond pcs, PropCond pci>
  PropCost
  IntSetRePropagator<View,pcs,pci>
  ::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template <class View, PropCond pcs, PropCond pci>
  size_t
  IntSetRePropagator<View,pcs,pci>::dispose(Space& home) {
    if (!home.failed()) {
      x0.cancel(home,*this,pcs);
      x1.cancel(home,*this,pci);
      b.cancel(home,*this,Gecode::Int::PC_INT_VAL);
    }
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}

// STATISTICS: set-prop

