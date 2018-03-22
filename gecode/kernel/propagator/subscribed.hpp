/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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
 */

namespace Gecode {

  /// Iterator over subscribed propagators
  class SubscribedPropagators {
  protected:
    /// Pointing at the current subscription (either propagator or advisor)
    ActorLink** c;
    /// End of propagator subscriptions
    ActorLink** ep;
    /// End of advisor subscriptions
    ActorLink** ea;
  public:
    /// Initialize
    template<class VIC>
    SubscribedPropagators(VarImp<VIC>& x);
    /// Initialize
    template<class View>
    SubscribedPropagators(ConstView<View>& x);
    /// Initialize
    template<class Var>
    SubscribedPropagators(VarImpView<Var>& x);
    /// Initialize
    template<class View>
    SubscribedPropagators(DerivedView<View>& x);
    /// Initialize
    template<class VIC>
    void init(VarImp<VIC>& x);
    /// Initialize
    template<class View>
    void init(ConstView<View>& x);
    /// Initialize
    template<class Var>
    void init(VarImpView<Var>& x);
    /// Initialize
    template<class View>
    void init(DerivedView<View>& x);
    /// Test whether there are propagators left
    bool operator ()(void) const;
    /// Move iterator to next propagator
    void operator ++(void);
    /// Return propagator
    Propagator& propagator(void) const;
  };

  template<class VIC>
  forceinline void
  SubscribedPropagators::init(VarImp<VIC>& x) {
    c = x.actor(0);
    ep = x.actorNonZero(x.pc_max+1); ea = x.b.base+x.entries;
  }
  template<class VIC>
  forceinline
  SubscribedPropagators::SubscribedPropagators(VarImp<VIC>& x) {
    init(x);
  }
  template<class View>
  forceinline void
  SubscribedPropagators::init(ConstView<View>&) {
    c = ep = ea = NULL;
  }
  template<class View>
  forceinline
  SubscribedPropagators::SubscribedPropagators(ConstView<View>& x) {
    init(x);
  }
  template<class Var>
  forceinline
  SubscribedPropagators::SubscribedPropagators(VarImpView<Var>& x) {
    init(*x.varimp());
  }
  template<class Var>
  forceinline void
  SubscribedPropagators::init(VarImpView<Var>& x) {
    init(*x.varimp());
  }
  template<class View>
  forceinline
  SubscribedPropagators::SubscribedPropagators(DerivedView<View>& x) {
    init(*x.varimp());
  }
  template<class View>
  forceinline void
  SubscribedPropagators::init(DerivedView<View>& x) {
    init(*x.varimp());
  }

  forceinline bool
  SubscribedPropagators::operator ()(void) const {
    return c<ea;
  }
  forceinline void
  SubscribedPropagators::operator ++(void) {
    c++;
  }
  forceinline Propagator&
  SubscribedPropagators::propagator(void) const {
    return (c < ep) ? *Propagator::cast(*c) : Advisor::cast(*c)->propagator();
  }

}

// STATISTICS: kernel-prop
