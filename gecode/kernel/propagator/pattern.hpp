/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
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
   * \defgroup TaskPropPat Propagator patterns
   *
   * \ingroup TaskActor
   */

  //@{
  /**
   * \brief Unary propagator
   *
   * Stores a single view of type \a View with propagation condition \a pc.
   *
   * If the propagation condition \a pc has the value PC_GEN_NONE, no
   * subscriptions are created.
   *
   */
  template<class View, PropCond pc>
  class UnaryPropagator : public Propagator {
  protected:
    /// Single view
    View x0;
    /// Constructor for cloning \a p
    UnaryPropagator(Space& home, UnaryPropagator& p);
    /// Constructor for rewriting \a p during cloning
    UnaryPropagator(Space& home, Propagator& p, View x0);
    /// Constructor for creation
    UnaryPropagator(Home home, View x0);
  public:
    /// Cost function (defined as PC_UNARY_LO)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Binary propagator
   *
   * Stores two views of type \a View with propagation condition \a pc.
   *
   * If the propagation condition \a pc has the value PC_GEN_NONE, no
   * subscriptions are created.
   *
   */
  template<class View, PropCond pc>
  class BinaryPropagator : public Propagator {
  protected:
    /// Two views
    View x0, x1;
    /// Constructor for cloning \a p
    BinaryPropagator(Space& home, BinaryPropagator& p);
    /// Constructor for creation
    BinaryPropagator(Home home, View x0, View x1);
    /// Constructor for rewriting \a p during cloning
    BinaryPropagator(Space& home, Propagator& p, View x0, View x1);
  public:
    /// Cost function (defined as low binary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Ternary propagator
   *
   * Stores three views of type \a View with propagation condition \a pc.
   *
   * If the propagation condition \a pc has the value PC_GEN_NONE, no
   * subscriptions are created.
   *
   */
  template<class View, PropCond pc>
  class TernaryPropagator : public Propagator {
  protected:
    /// Three views
    View x0, x1, x2;
    /// Constructor for cloning \a p
    TernaryPropagator(Space& home, TernaryPropagator& p);
    /// Constructor for creation
    TernaryPropagator(Home home, View x0, View x1, View x2);
    /// Constructor for rewriting \a p during cloning
    TernaryPropagator(Space& home, Propagator& p, View x0, View x1, View x2);
  public:
    /// Cost function (defined as low ternary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief n-ary propagator
   *
   * Stores array of views of type \a View with propagation condition \a pc.
   *
   * If the propagation condition \a pc has the value PC_GEN_NONE, no
   * subscriptions are created.
   *
   */
  template<class View, PropCond pc>
  class NaryPropagator : public Propagator {
  protected:
    /// Array of views
    ViewArray<View> x;
    /// Constructor for cloning \a p
    NaryPropagator(Space& home, NaryPropagator& p);
    /// Constructor for rewriting \a p during cloning
    NaryPropagator(Space& home, Propagator& p, ViewArray<View>& x);
    /// Constructor for creation
    NaryPropagator(Home home, ViewArray<View>& x);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief (n+1)-ary propagator
   *
   * Stores array of views and single view of type \a View with propagation
   * condition \a pc.
   *
   * If the propagation condition \a pc has the value PC_GEN_NONE, no
   * subscriptions are created.
   *
   */
  template<class View, PropCond pc>
  class NaryOnePropagator : public Propagator {
  protected:
    /// Array of views
    ViewArray<View> x;
    /// Single view
    View y;
    /// Constructor for cloning \a p
    NaryOnePropagator(Space& home, NaryOnePropagator& p);
    /// Constructor for rewriting \a p during cloning
    NaryOnePropagator(Space& home, Propagator& p, ViewArray<View>& x, View y);
    /// Constructor for creation
    NaryOnePropagator(Home home, ViewArray<View>& x, View y);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Mixed binary propagator
   *
   * Stores two views of type \a View0 and \a View1 with propagation
   * conditions \a pc0 and \a pc1.
   *
   * If the propagation conditions \a pc0 or \a pc1 have the values
   * PC_GEN_NONE, no subscriptions are created.
   *
   */
  template<class View0, PropCond pc0, class View1, PropCond pc1>
  class MixBinaryPropagator : public Propagator {
  protected:
    /// View of type \a View0
    View0 x0;
    /// View of type \a View1
    View1 x1;
    /// Constructor for cloning
    MixBinaryPropagator(Space& home, MixBinaryPropagator& p);
    /// Constructor for creation
    MixBinaryPropagator(Home home, View0 x0, View1 x1);
    /// Constructor for rewriting \a p during cloning
    MixBinaryPropagator(Space& home, Propagator& p, View0 x0, View1 x1);
  public:
    /// Cost function (defined as low binary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Mixed ternary propagator
   *
   * Stores three views of type \a View0, \a View1, and \a View2
   * with propagation conditions \a pc0, \a pc1, and \a pc2.
   *
   * If the propagation conditions \a pc0, \a pc1, \a pc2 have the values
   * PC_GEN_NONE, no subscriptions are created.
   *
   */
  template<class View0, PropCond pc0, class View1, PropCond pc1,
            class View2, PropCond pc2>
  class MixTernaryPropagator : public Propagator {
  protected:
    /// View of type \a View0
    View0 x0;
    /// View of type \a View1
    View1 x1;
    /// View of type \a View2
    View2 x2;
    /// Constructor for cloning
    MixTernaryPropagator(Space& home, MixTernaryPropagator& p);
    /// Constructor for creation
    MixTernaryPropagator(Home home, View0 x0, View1 x1, View2 x2);
    /// Constructor for rewriting \a p during cloning
    MixTernaryPropagator(Space& home, Propagator& p,
                         View0 x0, View1 x1, View2 x2);
  public:
    /// Cost function (defined as low ternary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Mixed (n+1)-ary propagator
   *
   * Stores array of views of type \a View0 with propagation condition \a pc0
   * and a single view of type \a View1 with propagation condition \a pc1.
   *
   * If the propagation conditions \a pc0 or \a pc1 have the values
   * PC_GEN_NONE, no subscriptions are created.
   *
   */
  template<class View0, PropCond pc0, class View1, PropCond pc1>
  class MixNaryOnePropagator : public Propagator {
  protected:
    /// Array of views
    ViewArray<View0> x;
    /// Single view
    View1 y;
    /// Constructor for cloning \a p
    MixNaryOnePropagator(Space& home, MixNaryOnePropagator& p);
    /// Constructor for creation
    MixNaryOnePropagator(Home home, ViewArray<View0>& x, View1 y);
    /// Constructor for rewriting \a p during cloning
    MixNaryOnePropagator(Space& home, Propagator& p,
                         ViewArray<View0>& x, View1 y);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };
  //@}


  /*
   * Unary propagators
   *
   */

  template<class View, PropCond pc>
  UnaryPropagator<View,pc>::UnaryPropagator(Home home, View y0)
    : Propagator(home), x0(y0) {
    if (pc != PC_GEN_NONE)
      x0.subscribe(home,*this,pc);
  }

  template<class View, PropCond pc>
  forceinline
  UnaryPropagator<View,pc>::UnaryPropagator
  (Space& home, UnaryPropagator<View,pc>& p)
    : Propagator(home,p) {
    x0.update(home,p.x0);
  }

  template<class View, PropCond pc>
  forceinline
  UnaryPropagator<View,pc>::UnaryPropagator
  (Space& home, Propagator& p, View y0)
    : Propagator(home,p) {
    x0.update(home,y0);
  }

  template<class View, PropCond pc>
  PropCost
  UnaryPropagator<View,pc>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::LO);
  }

  template<class View, PropCond pc>
  void
  UnaryPropagator<View,pc>::reschedule(Space& home) {
    if (pc != PC_GEN_NONE)
      x0.reschedule(home,*this,pc);
  }

  template<class View, PropCond pc>
  forceinline size_t
  UnaryPropagator<View,pc>::dispose(Space& home) {
    if (pc != PC_GEN_NONE)
      x0.cancel(home,*this,pc);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  /*
   * Binary propagators
   *
   */

  template<class View, PropCond pc>
  BinaryPropagator<View,pc>::BinaryPropagator(Home home, View y0, View y1)
    : Propagator(home), x0(y0), x1(y1) {
    if (pc != PC_GEN_NONE) {
      x0.subscribe(home,*this,pc);
      x1.subscribe(home,*this,pc);
    }
  }

  template<class View, PropCond pc>
  forceinline
  BinaryPropagator<View,pc>::BinaryPropagator
  (Space& home, BinaryPropagator<View,pc>& p)
    : Propagator(home,p) {
    x0.update(home,p.x0);
    x1.update(home,p.x1);
  }

  template<class View, PropCond pc>
  forceinline
  BinaryPropagator<View,pc>::BinaryPropagator
  (Space& home, Propagator& p, View y0, View y1)
    : Propagator(home,p) {
    x0.update(home,y0);
    x1.update(home,y1);
  }

  template<class View, PropCond pc>
  PropCost
  BinaryPropagator<View,pc>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<class View, PropCond pc>
  void
  BinaryPropagator<View,pc>::reschedule(Space& home) {
    if (pc != PC_GEN_NONE) {
      x0.reschedule(home,*this,pc);
      x1.reschedule(home,*this,pc);
    }
  }

  template<class View, PropCond pc>
  forceinline size_t
  BinaryPropagator<View,pc>::dispose(Space& home) {
    if (pc != PC_GEN_NONE) {
      x0.cancel(home,*this,pc);
      x1.cancel(home,*this,pc);
    }
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * Ternary propagators
   *
   */

  template<class View, PropCond pc>
  TernaryPropagator<View,pc>::TernaryPropagator
  (Home home, View y0, View y1, View y2)
    : Propagator(home), x0(y0), x1(y1), x2(y2) {
    if (pc != PC_GEN_NONE) {
      x0.subscribe(home,*this,pc);
      x1.subscribe(home,*this,pc);
      x2.subscribe(home,*this,pc);
    }
  }

  template<class View, PropCond pc>
  forceinline
  TernaryPropagator<View,pc>::TernaryPropagator
  (Space& home, TernaryPropagator<View,pc>& p)
    : Propagator(home,p) {
    x0.update(home,p.x0);
    x1.update(home,p.x1);
    x2.update(home,p.x2);
  }

  template<class View, PropCond pc>
  forceinline
  TernaryPropagator<View,pc>::TernaryPropagator
  (Space& home, Propagator& p, View y0, View y1, View y2)
    : Propagator(home,p) {
    x0.update(home,y0);
    x1.update(home,y1);
    x2.update(home,y2);
  }

  template<class View, PropCond pc>
  PropCost
  TernaryPropagator<View,pc>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);;
  }

  template<class View, PropCond pc>
  void
  TernaryPropagator<View,pc>::reschedule(Space& home) {
    if (pc != PC_GEN_NONE) {
      x0.reschedule(home,*this,pc);
      x1.reschedule(home,*this,pc);
      x2.reschedule(home,*this,pc);
    }
  }

  template<class View, PropCond pc>
  forceinline size_t
  TernaryPropagator<View,pc>::dispose(Space& home) {
    if (pc != PC_GEN_NONE) {
      x0.cancel(home,*this,pc);
      x1.cancel(home,*this,pc);
      x2.cancel(home,*this,pc);
    }
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * Nary propagators
   *
   */

  template<class View, PropCond pc>
  NaryPropagator<View,pc>::NaryPropagator
  (Home home, ViewArray<View>& y)
    : Propagator(home), x(y) {
    if (pc != PC_GEN_NONE)
      x.subscribe(home,*this,pc);
  }

  template<class View, PropCond pc>
  forceinline
  NaryPropagator<View,pc>::NaryPropagator
  (Space& home, NaryPropagator<View,pc>& p)
    : Propagator(home,p) {
    x.update(home,p.x);
  }

  template<class View, PropCond pc>
  forceinline
  NaryPropagator<View,pc>::NaryPropagator
  (Space& home, Propagator& p, ViewArray<View>& x0)
    : Propagator(home,p) {
    x.update(home,x0);
  }

  template<class View, PropCond pc>
  PropCost
  NaryPropagator<View,pc>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size());
  }

  template<class View, PropCond pc>
  void
  NaryPropagator<View,pc>::reschedule(Space& home) {
    if (pc != PC_GEN_NONE)
      x.reschedule(home,*this,pc);
  }

  template<class View, PropCond pc>
  forceinline size_t
  NaryPropagator<View,pc>::dispose(Space& home) {
    if (pc != PC_GEN_NONE)
      x.cancel(home,*this,pc);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * NaryOne (one additional variable) propagators
   *
   */

  template<class View, PropCond pc>
  NaryOnePropagator<View,pc>::NaryOnePropagator
  (Home home, ViewArray<View>& x0, View y0)
    : Propagator(home), x(x0), y(y0) {
    if (pc != PC_GEN_NONE) {
      x.subscribe(home,*this,pc);
      y.subscribe(home,*this,pc);
    }
  }

  template<class View, PropCond pc>
  forceinline
  NaryOnePropagator<View,pc>::NaryOnePropagator
  (Space& home, NaryOnePropagator<View,pc>& p)
    : Propagator(home,p) {
    x.update(home,p.x);
    y.update(home,p.y);
  }

  template<class View, PropCond pc>
  forceinline
  NaryOnePropagator<View,pc>::NaryOnePropagator
  (Space& home, Propagator& p, ViewArray<View>& x0, View y0)
    : Propagator(home,p) {
    x.update(home,x0);
    y.update(home,y0);
  }

  template<class View, PropCond pc>
  PropCost
  NaryOnePropagator<View,pc>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size()+1);
  }

  template<class View, PropCond pc>
  void
  NaryOnePropagator<View,pc>::reschedule(Space& home) {
    if (pc != PC_GEN_NONE) {
      x.reschedule(home,*this,pc);
      y.reschedule(home,*this,pc);
    }
  }

  template<class View, PropCond pc>
  forceinline size_t
  NaryOnePropagator<View,pc>::dispose(Space& home) {
    if (pc != PC_GEN_NONE) {
      x.cancel(home,*this,pc);
      y.cancel(home,*this,pc);
    }
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * Mixed binary propagators
   *
   */

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  MixBinaryPropagator<View0,pc0,View1,pc1>::MixBinaryPropagator
  (Home home, View0 y0, View1 y1)
    : Propagator(home), x0(y0), x1(y1) {
    if (pc0 != PC_GEN_NONE)
      x0.subscribe(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      x1.subscribe(home,*this,pc1);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  forceinline
  MixBinaryPropagator<View0,pc0,View1,pc1>::MixBinaryPropagator
  (Space& home, MixBinaryPropagator<View0,pc0,View1,pc1>& p)
    : Propagator(home,p) {
    x0.update(home,p.x0);
    x1.update(home,p.x1);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  forceinline
  MixBinaryPropagator<View0,pc0,View1,pc1>::MixBinaryPropagator
  (Space& home, Propagator& p, View0 y0, View1 y1)
    : Propagator(home,p) {
    x0.update(home,y0);
    x1.update(home,y1);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  PropCost
  MixBinaryPropagator<View0,pc0,View1,pc1>::cost(const Space&,
                                                 const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  void
  MixBinaryPropagator<View0,pc0,View1,pc1>::reschedule(Space& home) {
    if (pc0 != PC_GEN_NONE)
      x0.reschedule(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      x1.reschedule(home,*this,pc1);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  forceinline size_t
  MixBinaryPropagator<View0,pc0,View1,pc1>::dispose(Space& home) {
    if (pc0 != PC_GEN_NONE)
      x0.cancel(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      x1.cancel(home,*this,pc1);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * Mixed ternary propagators
   *
   */

  template<class View0, PropCond pc0, class View1, PropCond pc1,
            class View2, PropCond pc2>
  MixTernaryPropagator<View0,pc0,View1,pc1,View2,pc2>::
  MixTernaryPropagator(Home home, View0 y0, View1 y1, View2 y2)
    : Propagator(home), x0(y0), x1(y1), x2(y2) {
    if (pc0 != PC_GEN_NONE)
      x0.subscribe(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      x1.subscribe(home,*this,pc1);
    if (pc2 != PC_GEN_NONE)
      x2.subscribe(home,*this,pc2);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1,
            class View2, PropCond pc2>
  forceinline
  MixTernaryPropagator<View0,pc0,View1,pc1,View2,pc2>::
  MixTernaryPropagator(Space& home,
                         MixTernaryPropagator<View0,pc0,View1,pc1,
                         View2,pc2>& p)
    : Propagator(home,p) {
    x0.update(home,p.x0);
    x1.update(home,p.x1);
    x2.update(home,p.x2);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1,
            class View2, PropCond pc2>
  forceinline
  MixTernaryPropagator<View0,pc0,View1,pc1,View2,pc2>::MixTernaryPropagator
  (Space& home, Propagator& p, View0 y0, View1 y1, View2 y2)
    : Propagator(home,p) {
    x0.update(home,y0);
    x1.update(home,y1);
    x2.update(home,y2);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1,
            class View2, PropCond pc2>
  PropCost
  MixTernaryPropagator<View0,pc0,View1,pc1,View2,pc2>::
  cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1,
            class View2, PropCond pc2>
  void
  MixTernaryPropagator<View0,pc0,View1,pc1,View2,pc2>::reschedule(Space& home) {
    if (pc0 != PC_GEN_NONE)
      x0.reschedule(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      x1.reschedule(home,*this,pc1);
    if (pc2 != PC_GEN_NONE)
      x2.reschedule(home,*this,pc2);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1,
            class View2, PropCond pc2>
  forceinline size_t
  MixTernaryPropagator<View0,pc0,View1,pc1,View2,pc2>::dispose(Space& home) {
    if (pc0 != PC_GEN_NONE)
      x0.cancel(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      x1.cancel(home,*this,pc1);
    if (pc2 != PC_GEN_NONE)
      x2.cancel(home,*this,pc2);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * MixNaryOne (one additional variable) propagators
   *
   */

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  MixNaryOnePropagator<View0,pc0,View1,pc1>::MixNaryOnePropagator
  (Home home, ViewArray<View0>& x0, View1 y0)
    : Propagator(home), x(x0), y(y0) {
    if (pc0 != PC_GEN_NONE)
      x.subscribe(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      y.subscribe(home,*this,pc1);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  forceinline
  MixNaryOnePropagator<View0,pc0,View1,pc1>::MixNaryOnePropagator
  (Space& home, MixNaryOnePropagator<View0,pc0,View1,pc1>& p)
    : Propagator(home,p) {
    x.update(home,p.x);
    y.update(home,p.y);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  forceinline
  MixNaryOnePropagator<View0,pc0,View1,pc1>::MixNaryOnePropagator
  (Space& home, Propagator& p, ViewArray<View0>& x0, View1 y0)
    : Propagator(home,p) {
    x.update(home,x0);
    y.update(home,y0);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  PropCost
  MixNaryOnePropagator<View0,pc0,View1,pc1>::cost(const Space&,
                                                  const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size()+1);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  void
  MixNaryOnePropagator<View0,pc0,View1,pc1>::reschedule(Space& home) {
    if (pc0 != PC_GEN_NONE)
      x.reschedule(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      y.reschedule(home,*this,pc1);
  }

  template<class View0, PropCond pc0, class View1, PropCond pc1>
  forceinline size_t
  MixNaryOnePropagator<View0,pc0,View1,pc1>::dispose(Space& home) {
    if (pc0 != PC_GEN_NONE)
      x.cancel(home,*this,pc0);
    if (pc1 != PC_GEN_NONE)
      y.cancel(home,*this,pc1);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}

// STATISTICS: kernel-prop
