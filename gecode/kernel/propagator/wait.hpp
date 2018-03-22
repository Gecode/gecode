/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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
   * \brief Wait propagator for single view
   *
   * Requires \code #include <gecode/kernel/wait.hh> \endcode
   * \ingroup FuncKernelProp
   */
  template<class View>
  class UnaryWait : public Propagator {
  protected:
    /// View to wait for becoming assigned
    View x;
    /// Continuation to execute
    SharedData<std::function<void(Space& home)>> c;
    /// Constructor for creation
    UnaryWait(Home home, View x, std::function<void(Space& home)> c0);
    /// Constructor for cloning \a p
    UnaryWait(Space& home, UnaryWait& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home);
    /// Const function (defined as low unary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that waits until \a x becomes assigned and then executes \a c
    static ExecStatus post(Home home, View x,
                           std::function<void(Space& home)> c);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Wait propagator for several views
   *
   * Requires \code #include <gecode/kernel/wait.hh> \endcode
   * \ingroup FuncKernelProp
   */
  template<class View>
  class NaryWait : public Propagator {
  protected:
    /// Views to wait for becoming assigned
    ViewArray<View> x;
    /// Continuation to execute
    SharedData<std::function<void(Space& home)>> c;
    /// Constructor for creation
    NaryWait(Home home, ViewArray<View>& x,
             std::function<void(Space& home)> c0);
    /// Constructor for cloning \a p
    NaryWait(Space& home, NaryWait& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home);
    /// Const function (defined as high unary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that waits until \a x becomes assigned and then executes \a c
    static ExecStatus post(Home home, ViewArray<View>& x,
                           std::function<void(Space& home)> c);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };


  /*
   * Wait propagator for single view
   *
   */
  template<class View>
  forceinline
  UnaryWait<View>::UnaryWait(Home home, View x0,
                             std::function<void(Space& home)> c0)
    : Propagator(home), x(x0), c(c0) {
    x.subscribe(home,*this,PC_GEN_ASSIGNED);
    home.notice(*this,AP_DISPOSE);
  }
  template<class View>
  forceinline
  UnaryWait<View>::UnaryWait(Space& home, UnaryWait& p)
    : Propagator(home,p), c(p.c) {
    x.update(home,p.x);
  }
  template<class View>
  Actor*
  UnaryWait<View>::copy(Space& home) {
    return new (home) UnaryWait<View>(home,*this);
  }
  template<class View>
  PropCost
  UnaryWait<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::LO);
  }
  template<class View>
  void
  UnaryWait<View>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_GEN_ASSIGNED);
  }
  template<class View>
  ExecStatus
  UnaryWait<View>::propagate(Space& home, const ModEventDelta&) {
    assert(x.assigned());
    GECODE_VALID_FUNCTION(c());
    c()(home);
    return home.failed() ? ES_FAILED : home.ES_SUBSUMED(*this);
  }
  template<class View>
  forceinline ExecStatus
  UnaryWait<View>::post(Home home, View x,
                        std::function<void(Space& home)> c) {
    if (!c)
      throw InvalidFunction("UnaryWait::post");
    if (x.assigned()) {
      c(home);
      return home.failed() ? ES_FAILED : ES_OK;
    } else {
      (void) new (home) UnaryWait<View>(home,x,c);
      return ES_OK;
    }
  }
  template<class View>
  size_t
  UnaryWait<View>::dispose(Space& home) {
    x.cancel(home,*this,PC_GEN_ASSIGNED);
    home.ignore(*this,AP_DISPOSE);
    c.~SharedData<std::function<void(Space& home)>>();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  /*
   * Wait propagator for several views
   *
   */
  template<class View>
  forceinline
  NaryWait<View>::NaryWait(Home home, ViewArray<View>& x0,
                           std::function<void(Space& home)> c0)
    : Propagator(home), x(x0), c(c0) {
    assert(!x[0].assigned());
    x[0].subscribe(home,*this,PC_GEN_ASSIGNED);
    home.notice(*this,AP_DISPOSE);
  }
  template<class View>
  forceinline
  NaryWait<View>::NaryWait(Space& home, NaryWait& p)
    : Propagator(home,p), c(p.c) {
    x.update(home,p.x);
  }
  template<class View>
  Actor*
  NaryWait<View>::copy(Space& home) {
    assert(!x[0].assigned());
    for (int i=x.size()-1; i>0; i--)
      if (x[i].assigned())
        x.move_lst(i);
    assert(x.size() > 0);
    return new (home) NaryWait<View>(home,*this);
  }
  template<class View>
  PropCost
  NaryWait<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::HI);
  }
  template<class View>
  void
  NaryWait<View>::reschedule(Space& home) {
    x[0].reschedule(home,*this,PC_GEN_ASSIGNED);
  }
  template<class View>
  ExecStatus
  NaryWait<View>::propagate(Space& home, const ModEventDelta& ) {
    assert(x[0].assigned());
    for (int i=x.size()-1; i>0; i--)
      if (x[i].assigned())
        x.move_lst(i);
    assert(x.size() > 0);
    if (x.size() == 1) {
      x.size(0);
      GECODE_VALID_FUNCTION(c());
      c()(home);
      return home.failed() ? ES_FAILED : home.ES_SUBSUMED(*this);
    } else {
      // Create new subscription
      x.move_lst(0);
      assert(!x[0].assigned());
      x[0].subscribe(home,*this,PC_GEN_ASSIGNED,false);
      return ES_OK;
    }
  }
  template<class View>
  forceinline ExecStatus
  NaryWait<View>::post(Home home, ViewArray<View>& x,
                       std::function<void(Space& home)> c) {
    if (!c)
      throw InvalidFunction("NaryWait::post");
    for (int i=x.size(); i--; )
      if (x[i].assigned())
        x.move_lst(i);
    if (x.size() == 0) {
      c(home);
      return home.failed() ? ES_FAILED : ES_OK;
    } else {
      x.unique();
      if (x.size() == 1) {
        return UnaryWait<View>::post(home,x[0],c);
      } else {
        (void) new (home) NaryWait<View>(home,x,c);
        return ES_OK;
      }
    }
  }
  template<class View>
  size_t
  NaryWait<View>::dispose(Space& home) {
    if (x.size() > 0)
      x[0].cancel(home,*this,PC_GEN_ASSIGNED);
    home.ignore(*this,AP_DISPOSE);
    c.~SharedData<std::function<void(Space& home)>>();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}

// STATISTICS: kernel-prop
